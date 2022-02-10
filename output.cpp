#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "terminal.h"
#include "common.h"
#include "fileio.h"

extern struct editorConfig E;

/*****************************************************************
 *                            output                             *
 *****************************************************************/
void editorScroll()
{
	E.rx = 0;
	if(E.cy < E.numrows)
		E.rx = editorRowCxToRx(&E.row[E.cy], E.cx);

	if(E.cy < E.rowoff)
		E.rowoff = E.cy;

	if(E.cy >= E.rowoff + E.screenrows)
		E.rowoff = E.cy - E.screenrows + 1;

	if(E.rx < E.coloff) E.coloff = E.rx;

	if(E.rx > E.coloff + E.screencols)
		E.coloff = E.rx - E.screencols + 1;
}
void editorDrawRows(struct abuf *ab) 
{
	int y;
	for (y = 0; y < E.screenrows; y++) {
		int filerow = y + E.rowoff;
		if(filerow >= E.numrows)
		{
			//if no file, print welcome message
			if (E.numrows == 0 && y == E.screenrows / 3) {
				char welcome[80];
				int welcomelen = snprintf(welcome, sizeof(welcome),
					"Mary editor -- version %s", MARY_VERSION);
				if (welcomelen > E.screencols) welcomelen = E.screencols;
				int padding = (E.screencols - welcomelen) / 2;
				if (padding) {
					abAppend(ab, "~", 1);
					padding--;
				}

				while (padding--) abAppend(ab, " ", 1);
				abAppend(ab, welcome, welcomelen);

			} else {
				abAppend(ab, "~", 1);
			}
		} else {
			int len = E.row[filerow].rsize - E.coloff;
			if(len < 0) len = 0;
			if(len > E.screencols) len = E.screencols;
			abAppend(ab, &E.row[filerow].render[E.coloff], len);
		}

			abAppend(ab, "\x1b[K", 3);

			abAppend(ab, "\r\n", 2);
	}
}

void editorDrawMessageBar(struct abuf *ab) {
	abAppend(ab, "\x1b[K", 3);
	int msglen = strlen(E.statusmsg);
	if (msglen > E.screencols) msglen = E.screencols;
	if (msglen && time(NULL) - E.statusmsg_time < 5)
		abAppend(ab, E.statusmsg, msglen);
}

//\x1b[7m inverts colors, \x1b[m undoes invert
//\x1b[m takes arguments 1 for bold, 4 for underscore, 5 for blink and 7 for inverted colors. Ex. \x1b[1;4;5;7m
void editorDrawStatusBar(struct abuf *ab)
{
	abAppend(ab, "\x1b[7m", 4);
	char status[80], rstatus[80];
	int len = snprintf(status, sizeof(status), "%.20s - %d lines",
		E.filename ? E.filename : "[No File Name]", E.numrows);
	int rlen = snprintf(rstatus, sizeof(rstatus), "%d%d",
		E.cy + 1, E.numrows);

	if(len > E.screencols) len = E.screencols;
	abAppend(ab, status, len);

	while(len < E.screencols)
	{
		if(E.screencols - len ==  rlen)
		{
			abAppend(ab, rstatus, rlen);
			break;
		} else {
			abAppend(ab, " ", 1);
			len++;
		}
	}
	abAppend(ab, "\x1b[m", 3);
	abAppend(ab, "\r\n", 2);
}

void editorRefreshScreen() 
{
	editorScroll();

	struct abuf ab = ABUF_INIT;
    // 4 bytes to terminal. First byte = \x1b(escape(27)) escape characters always start with 27 followed by [
	// Cursor repainting. h (set) l(reset)
	abAppend(&ab, "\x1b[?25l", 6);
	//J command is Erase in Display. Takes argument before commands. 2 = entire screen. 1 clears screen up to cursor
	// Using VT100 escape sequences. ncurses is another library used for this

    // H command is cursor position. Takes two arguments: row number and column number
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
	editorDrawStatusBar(&ab);
	editorDrawMessageBar(&ab);

    char buf[32];
	//H is escape char for curso postion
  	snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    abAppend(&ab, buf, strlen(buf));


	abAppend(&ab, "\x1b[?25h", 6);
	write(STDOUT_FILENO, ab.b, ab.len);
	abFree(&ab);
}

void editorStatusMsg(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap);
	va_end(ap);
	E.statusmsg_time = time(NULL);
}
