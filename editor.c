#include "editor.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define COUNT(x) (sizeof (x) / sizeof *(x))
#define CTRL(chr) (chr & 037)

// Modal Actions
static Action normal_actions[] = {
    { CTRL('w'), writefile, NORMAL },
    { CTRL('t'), newbuf,    NORMAL },
    { CTRL('n'), nextbuf,   NORMAL },
    { CTRL('p'), prevbuf,   NORMAL },
    { 'i',       NULL,      INSERT },
    { 'g',       begofbuf,  NORMAL },
    { 'G',       endofbuf,  NORMAL },
    { 'a',       nextchar,  INSERT },
    { 'h',       prevchar,  NORMAL },
    { 'j',       nextline,  NORMAL },
    { 'k',       prevline,  NORMAL },
    { 'l',       nextchar,  NORMAL },
    { 'w',       nextword,  NORMAL },
    { 'b',       prevword,  NORMAL },
    { 'x',       delchar,   NORMAL },
};
static const Mode normal_mode = {
    .exit_key = CTRL('q'),
    .default_action = NULL,
    .actions = normal_actions,
    .count = COUNT(normal_actions),
};

static Action insert_actions[] = {
    { CTRL('w'),     writefile, INSERT },
    { KEY_HOME,      begofline, INSERT },
    { KEY_END,       endofline, INSERT },
    { KEY_UP,        prevline,  INSERT },
    { KEY_DOWN,      nextline,  INSERT },
    { KEY_RIGHT,     nextchar,  INSERT },
    { KEY_LEFT,      prevchar,  INSERT },
    { '\n',          newline,   INSERT },
    { '\r',          newline,   INSERT },
    { KEY_BACKSPACE, backchar,  INSERT },
    { KEY_DC,        delchar,   INSERT },
    { CTRL('c'),     NULL,      NORMAL },
};
static const Mode insert_mode = {
    .exit_key = CTRL('q'),
    .default_action = addchar,
    .actions = insert_actions,
    .count = COUNT(insert_actions),
};

static char * getinput(Editor *e, char *input, const char *msg);

void setmode(Editor *e, MODE m)
{
    switch (m) {
        case INSERT:
            e->mode = insert_mode;
            break;
        case NORMAL:
            e->mode = normal_mode;
            break;
    }
}

bool readfile(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    buf_clear(e->buf);
    return buf_read(e->buf);
}

bool writefile(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (!e->buf->name) {
        char input[FILENAME_MAX];
        e->buf->name = getinput(e, input, "Save as:");
    }
    return buf_write(e->buf);
}

bool begofline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    e->buf->col = 0;
    return true;
}

bool endofline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    e->buf->col = strlen(e->buf->line->str);
    return true;
}

bool nextchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (e->buf->col + 1 <= strlen(e->buf->line->str)) {
        e->buf->col++;
        return true;
    }
    else if (e->buf->line->next) {
        nextline(e, key);
        begofline(e, key);
        return true;
    }
    return false;
}

bool prevchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (e->buf->col > 0) {
        e->buf->col--;
        return true;
    }
    else if (e->buf->line->prev) {
        prevline(e, key);
        endofline(e, key);
        return true;
    }
    return false;
}

bool nextword(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    while (nextchar(e, key)) {
        char ch = e->buf->line->str[e->buf->col];
        if (!isblank(ch) && ch != '\0') {
            if (e->buf->col == 0) {
                return true;
            }
            else {
                char pchar = e->buf->line->str[e->buf->col - 1];
                if (isblank(pchar))
                    return true;
            }
        }
    }
    return false;
}

bool prevword(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    while (prevchar(e, key)) {
        char ch = e->buf->line->str[e->buf->col];
        if (!isblank(ch) && ch != '\0') {
            if (e->buf->col == 0) {
                return true;
            }
            else {
                char pchar = e->buf->line->str[e->buf->col - 1];
                if (isblank(pchar))
                    return true;
            }
        }
    }
    return false;
}

bool nextline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Line *nline = e->buf->line->next;
    bool rval = false;
    if (nline) {
        e->buf->line = nline;
        e->buf->row++;
        rval = true;
    }
    if (e->buf->col > strlen(e->buf->line->str))
        endofline(e, key);
    return rval;
}

bool prevline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Line *pline = e->buf->line->prev;
    bool rval = false;
    if (pline) {
        e->buf->line = pline;
        e->buf->row--;
        rval = true;
    }
    if (e->buf->col > strlen(e->buf->line->str))
        endofline(e, key);
    return rval;
}

bool newline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    char *split = e->buf->line->str + e->buf->col;
    size_t len = strlen(split);
    Line *l = line_new(split, len);
    memset(split, '\0', len);
    if (e->buf->line->next) {
        buf_insert(e->buf, e->buf->line->next, l);
    }
    else {
        buf_pushback(e->buf, l);
    }
    nextline(e, key);
    begofline(e, key);
    return true;
}

bool backchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Line *l = e->buf->line;
    prevchar(e, key);
    // If we moved to the previous line, we need to bring what was left of the line below to
    // our current line
    if (e->buf->line == l->prev) {
        line_pushback(e->buf->line, l->str, strlen(l->str));
        buf_erase(e->buf, l);
    }
    else {
        line_erase(e->buf->line, e->buf->col);
    }
    return true;
}

bool delchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    line_erase(e->buf->line, e->buf->col);
    return true;
}

bool addchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    line_insert(e->buf->line, e->buf->col, key);
    nextchar(e, key);
    return true;
}

bool newbuf(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Buffer *buf = buf_new();
    buf->prev = e->buf;
    e->buf->next = buf;
    e->buf = buf;
    e->end = buf;
    return true;
}

bool nextbuf(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (e->buf->next) {
        e->buf = e->buf->next;
    }
    else {
        e->buf = e->beg;
    }
    begofbuf(e, key);
    return true;
}

bool prevbuf(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (e->buf->prev) {
        e->buf = e->buf->prev;
    }
    else {
        e->buf = e->end;
    }
    begofbuf(e, key);
    return true;
}

bool begofbuf(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    e->buf->line = e->buf->beg;
    e->buf->row = 0;
    begofline(e, key);
    return true;
}

bool endofbuf(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    e->buf->line = e->buf->end;
    e->buf->row = e->buf->size - 1;
    endofline(e, key);
    return true;
}

static char * getinput(Editor *e, char *input, const char *msg)
{
    // Title message
    wclear(e->titlewin.win);
    mvwaddstr(e->titlewin.win, 0, 0, msg);

    // User input area
    wattroff(e->titlewin.win, A_REVERSE);
    waddch(e->titlewin.win, ' ');
    wrefresh(e->titlewin.win);

    // Get user input
    echo();
    wgetstr(e->titlewin.win, input);
    noecho();

    // Make the title window white again
    wattron(e->titlewin.win, A_REVERSE);
    return input;
}
