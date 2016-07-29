#include "editor.h"

#include <string.h>

#define COUNT(x) (sizeof (x) / sizeof *(x))
#define CTRL(chr) (chr & 037)

// Modal Actions
static Action normal_actions[] = {
    { CTRL('w'), writefile, NORMAL },
    { 'i',       NULL,      INSERT },
    { 'a',       nextchar,  INSERT },
    { 'h',       prevchar,  NORMAL },
    { 'j',       nextline,  NORMAL },
    { 'k',       prevline,  NORMAL },
    { 'l',       nextchar,  NORMAL },
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

void readfile(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    buf_clear(e->buf);
    buf_read(e->buf);
}

void writefile(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (!e->buf->name) {
        char input[FILENAME_MAX];
        e->buf->name = getinput(e, input, "Save as:");
    }
    buf_write(e->buf);
}

void begofline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    e->buf->col = 0;
}

void endofline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    e->buf->col = strlen(e->buf->pos->str);
}

void nextchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (e->buf->col + 1 <= strlen(e->buf->pos->str)) {
        e->buf->col++;
    }
    else if (e->buf->pos->next) {
        nextline(e, key);
        begofline(e, key);
    }
}

void prevchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    if (e->buf->col > 0) {
        e->buf->col--;
    }
    else if (e->buf->pos->prev) {
        prevline(e, key);
        endofline(e, key);
    }
}

void nextline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Line *nline = e->buf->pos->next;
    if (nline) {
        e->buf->pos = nline;
        e->buf->row++;
    }
    if (e->buf->col > strlen(e->buf->pos->str))
        endofline(e, key);
}

void prevline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Line *pline = e->buf->pos->prev;
    if (pline) {
        e->buf->pos = pline;
        e->buf->row--;
    }
    if (e->buf->col > strlen(e->buf->pos->str))
        endofline(e, key);
}

void newline(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    char *split = e->buf->pos->str + e->buf->col;
    size_t len = strlen(split);
    Line *l = line_new(split, len);
    memset(split, '\0', len);
    if (e->buf->pos->next) {
        buf_insert(e->buf, e->buf->pos->next, l);
    }
    else {
        buf_pushback(e->buf, l);
    }
    nextline(e, key);
    begofline(e, key);
}

void backchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    Line *l = e->buf->pos;
    prevchar(e, key);
    // If we moved to the previous line, we need to bring what was left of the line below to
    // our current line
    if (e->buf->pos == l->prev) {
        line_pushback(e->buf->pos, l->str, strlen(l->str));
        buf_erase(e->buf, l);
    }
    else {
        line_erase(e->buf->pos, e->buf->col);
    }
}

void delchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    line_erase(e->buf->pos, e->buf->col);
}

void addchar(void *ctx, int key)
{
    Editor *e = (Editor *)ctx;
    line_insert(e->buf->pos, e->buf->col, key);
    nextchar(e, key);
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
