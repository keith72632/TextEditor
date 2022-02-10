#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/ioctl.h>

#include "terminal.h"


extern struct editorConfig E;

/*******************************************************
 *                   Terminal                          *
 *******************************************************/

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
  	write(STDOUT_FILENO, "\x1b[H", 3);

	// perror from stdio.h
  	perror(s);
  	exit(1);
}

void disableRawMode() {
  	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    	die("tcsetattr");
}

void enableRawMode() {
  	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");
	// from stdlib.h
  	atexit(disableRawMode);

    // ECHO == 00000000000000000000000000001000b. Each key typed printed to terminal. Needs to be disabled
    // ICANON. "Canonical input". disabling this kills process when pressing q without enter
    // ISIG enables signals. Disabling allows Ctrl-c and Ctrl-z to be printed
    // IXON Enables start and stop output flow. Disable Ctrl-s and Ctrl-q. Pause and resum transmission
    // IEXTEN enable input character processing. Disable to print Ctrl-V
    // ICRNL translates carriage returns. Disable to print Ctrl-m
    // OPOST post process output. Disable to trun off all output processing features
    // BRKINT when enabled, break condition will cause signal to be sent to program like pressing Ctrl-C
    // INPCK enables parity checking
    // ISTRIP causes the 8th but of each input bute to be stripped to 0
    // CS8 not a flag, but a bit mask. 
	struct termios raw = E.orig_termios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

  	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
		if (nread == -1 && errno != EAGAIN) die("read");
	}

	// if an escape key is read, read two more bytes into seq buffer. If times out, assumes \x1b is all that is returned
	if(c == '\x1b'){
		char seq[3];
		
		if(read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if(read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

		//Escape key arguments after [. A = up, B = down, C = right, D = left
		if(seq[0] == '['){
			if(seq[1] >= '0' && seq[1] <= '9')
			{
				if(read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';

				// certain keys like page up and page down are followed by ~
				if(seq[2] == '~')
				{
					switch(seq[1])
					{
						case '1': return HOME_KEY;
						case '3': return DEL_KEY;
						case '4': return END_KEY;
						case '5': return PAGE_UP;
						case '6': return PAGE_DOWN;
						case '7': return HOME_KEY;
						case '8': return END_KEY;
					}
				}
			} else {
				switch(seq[1]){
					case 'A': return ARROW_UP;
					case 'B': return ARROW_DOWN;
					case 'C': return ARROW_RIGHT;
					case 'D': return ARROW_LEFT;
					case 'H': return HOME_KEY;
					case 'F': return END_KEY;
				}
			}
		} else if (seq[0] == 'O')
		{
			switch(seq[1])
			{
				case 'H': return HOME_KEY;
				case 'F': return END_KEY;
			}
		}

		return '\x1b';
	} else {
		return c;
	}
}


int getCursorPosition(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;

	//\x1b[6n is a device status report
	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
	while (i < sizeof(buf) - 1) {
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		i++;
	}

	buf[i] = '\0';
	if (buf[0] != '\x1b' || buf[1] != '[') return -1;
	if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

	return 0;
}

int getWindowSize(int *rows, int *cols) {
  	struct winsize ws;
	//The ioctl number(TIOCGWINSZ) encodes the major device number, the type of the ioctl, the command, and the type of the parameter.
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		// B and C escape characters move the cursor 
    	if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    	return getCursorPosition(rows, cols);
  	} else {
    	*cols = ws.ws_col;
    	*rows = ws.ws_row;
 	}
   	return 0;
}

/*** init ***/

void initEditor() {
	E.cx = 0;
	E.cy = 0;
	E.rx = 0;
	E.rowoff = 0;
	E.coloff = 0;
	E.numrows = 0;
	E.row = NULL;
	E.filename = NULL;
	E.statusmsg[0] = '\0';
	E.statusmsg_time = 0;
  	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
	E.screenrows -= 2;
}