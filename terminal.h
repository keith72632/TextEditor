#include <termios.h>
#include <time.h>
#include <stdarg.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define TAB_STOP 4
#define MARY_VERSION "0.0.1"

//editor row. pointer to specific row
typedef struct erow {
	int size;
	int rsize;
	char *chars;
	char *render; // Contains the actual characters to be rendered on screen
} erow;

struct editorConfig {
	//cx and cy are indexes to chars
	int cx, cy;
	//index to render field. If no tabs (render), then rx same as cx
	int rx;
	int rowoff;
	int coloff;
	int screenrows;
 	int screencols;
	int numrows;
	//array of erow struct for multiple lines. Needs initialized to NULL later
	erow *row;
	char *filename;
	char statusmsg[80];
	time_t statusmsg_time;
  	struct termios orig_termios;
};

// Map keypress to number high enough it doesn't conflict with character type
enum editorKey {
	ARROW_LEFT  = 1000,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
	DEL_KEY,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN
};

#ifndef TERMINAL_H
#define TERMINAL_H

void die(const char *s);

void disableRawMode();

void enableRawMode();

int editorReadKey();

int getCursorPosition(int *rows, int *cols);

int getWindowSize(int *rows, int *cols);

void initEditor();

#endif