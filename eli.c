#include "eli.h"
#include "str.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#define COUNT(x) (sizeof (x) / sizeof *(x))
#define CTRL(chr) (chr & 037)

static bool command(void *ctx, int key);
static bool readfile(void *ctx, int key);
static bool writefile(void *ctx, int key);
static bool begofline(void *ctx, int key);
static bool endofline(void *ctx, int key);
static bool nextchar(void *ctx, int key);
static bool prevchar(void *ctx, int key);
static bool nextword(void *ctx, int key);
static bool prevword(void *ctx, int key);
static bool nextline(void *ctx, int key);
static bool prevline(void *ctx, int key);
static bool newline(void *ctx, int key);
static bool backchar(void *ctx, int key);
static bool delchar(void *ctx, int key);
static bool addchar(void *ctx, int key);
static bool newbuf(void *ctx, int key);
static bool delbuf(void *ctx, int key);
static bool nextbuf(void *ctx, int key);
static bool prevbuf(void *ctx, int key);
static bool begofbuf(void *ctx, int key);
static bool endofbuf(void *ctx, int key);

static void getinput(Eli *e, char *input, const char *cmd);
static bool runcmd(Eli *e, const char *cmd, const char *arg);

// Modal Actions
static Action normal_actions[] = {
    { CTRL('s'), writefile, NORMAL },
    { CTRL('r'), readfile,  NORMAL },
    { CTRL('t'), newbuf,    NORMAL },
    { CTRL('w'), delbuf,    NORMAL },
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
    { ':',       command,   NORMAL },
    // Default Action
    { '\0',      NULL,      NORMAL },
};
static const Mode normal_mode = {
    .exit_key = CTRL('q'),
    .actions = normal_actions,
    .count = COUNT(normal_actions),
};

static Action insert_actions[] = {
    { CTRL('s'),     writefile, INSERT },
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
    // Default Action
    { '\0',          addchar,   INSERT },
};
static const Mode insert_mode = {
    .exit_key = CTRL('q'),
    .actions = insert_actions,
    .count = COUNT(insert_actions),
};

void setmode(Eli *e, MODE m)
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

static bool command(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    char input[BUFSIZ] = {};
    getinput(e, input, NULL);
    if (strlen(input) == 0)
        return false;

    char *cmd = strtrim(input);
    char *arg = strchr(cmd, ' ');
    if (arg) {
        *arg = '\0';
        arg = arg + 1;
    }
    return runcmd(e, cmd, arg);
}

static bool readfile(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    return buf_read(e->buf, NULL);
}

static bool writefile(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    return buf_write(e->buf, NULL);
}

static bool begofline(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    e->buf->col = 0;
    return true;
}

static bool endofline(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    e->buf->col = strlen(e->buf->line->str);
    return true;
}

static bool nextchar(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool prevchar(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool nextword(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool prevword(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool nextline(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool prevline(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool newline(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool backchar(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
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

static bool delchar(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    line_erase(e->buf->line, e->buf->col);
    return true;
}

static bool addchar(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    line_insert(e->buf->line, e->buf->col, key);
    nextchar(e, key);
    return true;
}

static bool newbuf(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    Buffer *buf = buf_new(NULL);
    buf->next = e->buf;
    buf->prev = e->buf->prev;
    if (buf->next)
        buf->next->prev = buf;
    if (buf->prev)
        buf->prev->next = buf;
    if (e->beg == e->buf)
        e->beg = buf;
    e->buf = buf;
    return true;
}

static bool delbuf(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    Buffer *buf = e->buf;
    if (buf == e->beg)
        e->beg = buf->next;
    else
        buf->prev->next = buf->next;
    if (buf == e->end)
        e->end = buf->prev;
    else
        buf->next->prev = buf->prev;
    e->buf = buf->next ?: e->beg;
    buf_free(buf);
    return true;
}

static bool nextbuf(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    if (e->buf->next) {
        e->buf = e->buf->next;
    }
    else {
        e->buf = e->beg;
    }
    return true;
}

static bool prevbuf(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    if (e->buf->prev) {
        e->buf = e->buf->prev;
    }
    else {
        e->buf = e->end;
    }
    return true;
}

static bool begofbuf(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    e->buf->line = e->buf->beg;
    e->buf->row = 0;
    begofline(e, key);
    return true;
}

static bool endofbuf(void *ctx, int key)
{
    Eli *e = (Eli *)ctx;
    e->buf->line = e->buf->end;
    e->buf->row = e->buf->size - 1;
    endofline(e, key);
    return true;
}

static void getinput(Eli *e, char *input, const char *cmd)
{
    // Command prompt
    mvwaddstr(e->cmdwin.win, 0, 0, ":");
    wrefresh(e->cmdwin.win);

    // Get user input
    echo();
    wgetstr(e->cmdwin.win, input);
    noecho();

    // Clear command prompt
    wclear(e->cmdwin.win);
    wrefresh(e->cmdwin.win);
}

static bool runcmd(Eli *e, const char *cmd, const char *arg)
{
    if (streq(cmd, "write")) {
        return buf_write(e->buf, arg);
    }
    return false;
}
