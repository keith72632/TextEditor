#include <unistd.h>
#include <stdlib.h>
#include "terminal.h"


/******************************************************************
 *                            INPUT                               *
 ******************************************************************/

extern struct editorConfig E;

void editorMoveCursor(int key)
{
    //checks to see if cursor is on valid line
    erow *row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    switch(key){
        case ARROW_LEFT:
            if(E.cx != 0)
                E.cx--;
            //Moves cursor to end of previous line if cursor is at 0 on x-axis of current line
            else if(E.cy > 0)
            {
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if(row && E.cx < row->size)
                E.cx++;
            //move cursor to beginning of next line if at the end of current line
            else if(row && E.cx == row->size)
            {
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_DOWN:
            if(E.cy < E.numrows)
                E.cy++;
            break;
        case ARROW_UP:
            if(E.cy != 0)
                E.cy--;
            break;
    }

    //this sets cursor to the end of the line when moving down along y-axis
    row = (E.cy >= E.numrows) ? NULL : &E.row[E.cy];
    int rowlen = row ? row->size : 0;
    if(E.cx > rowlen)
        E.cx = rowlen;
}

void editorProcessKeypress() {
	int c = editorReadKey();

	switch (c) {
		case CTRL_KEY('q'):
		    write(STDOUT_FILENO, "\x1b[2J", 4);
		    write(STDOUT_FILENO, "\x1b[H", 3);
		    exit(0);
		    break;

        case HOME_KEY:
            E.cx = 0;
            break;
        
        case END_KEY:
            if(E.cy = E.numrows)
                E.cx = E.row[E.cy].size;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if(c == PAGE_UP)
                {
                    E.cy = E.rowoff;
                } else if (c == PAGE_DOWN) {
                    E.cy = E.rowoff + E.screenrows - 1;
                    if(E.cy > E.numrows) E.cy = E.numrows;
                }
                int times = E.screenrows;
                while(times--)
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
            }
            break;

        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editorMoveCursor(c);
            break;
        
        case CTRL_KEY('k'):
            E.cy = 0;
            break;
        case CTRL_KEY('j'):
            E.cy = E.screenrows - 1;
            break;
        case CTRL_KEY('h'):
            E.cx = 0;
            break;
        case CTRL_KEY('l'):
            E.cx = E.screencols -1;
            break;
	}
}