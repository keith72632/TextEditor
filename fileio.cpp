#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include "terminal.h"
extern struct editorConfig E;

int editorRowCxToRx(erow *row, int cx)
{
    int rx = 0;
    int j;
    for(j = 0; j < cx; j++)
    {
        if(row->chars[j] == '\t')
            rx += (TAB_STOP -1 ) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

void editorUpdateRow(erow *row)
{
    int tabs = 0;
    int j;
    int idx = 0;

    for(j = 0; j < row->size; j++)
    {
        if(row->chars[j] == '\t') tabs++;
    }

    free(row->render);
    row->render = (char*)malloc(row->size + tabs*(TAB_STOP - 1) + 1);
    
    for(j = 0; j < row->size; j++)
    {
        if(row->chars[j] == '\t')
        {
            row->render[idx++] =' ';
            while(idx % TAB_STOP != 0) row->render[idx++] = ' ';
        } else {
            row->render[idx++] = row->chars[j];
        }
    }
    row->render[idx] = '\0';
    row->rsize = idx;
}

void editorAppendRow(char *s, size_t len)
{
    //allocate number of bytes in each erow times number of erow we want
    E.row = (erow*)realloc(E.row, sizeof(erow) * (E.numrows + 1));

    int at = E.numrows;
    E.row[at].size = len;
    E.row[at].chars = (char*)malloc(len +1);
    memcpy(E.row[at].chars, s, len);
    E.row[at].chars[len] = '\0';

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]);

    E.numrows++;
}

void editorRowInsertChar(erow *row, int at, int c)
{
    //at is the index where we want to insert character
    if(at < 0 || at > row->size) at = row->size;
    //allocate one more byte for the chars of the erow(add 2 because of nullbute)
    row->chars = (char*)realloc(row->chars, row->size + 2);
    //make room for new character
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
    //update size of row and update render and rsize
    row->size++;
    row->chars[at] = c;
    editorUpdateRow(row);
}

void editorOpen(char *filename)
{
    free(E.filename);
    //strdup returns a pointer to a null terminated byte string. Used here to point E.filename to open file
    E.filename = strdup(filename);

    FILE *fp = fopen(filename, "r");
    if(!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0; //size_t is useful for function that can return either signed or unsigned
    ssize_t linelen;

    //getline is useful for reading lines from a file and don't know how much memory to allocate
    //take NULL line pointer to read line to, and sets linecap which is set to zero for new line to read
    //Returns length of line read. -1 or for EOF
    while((linelen = getline(&line, &linecap, fp)) != -1){
        //strips carriage and newline off since we already know each erow represents one line
        while(linelen > 0 && (line[linelen -1] == '\n' ||
                              line[linelen -1] == '\r'))
            linelen--;

        editorAppendRow(line, linelen);
    }
    free(line);
    fclose(fp);
}