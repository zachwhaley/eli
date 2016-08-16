#include "action.h"

#include "eli.h"
#include "str.h"
#include "line.h"
#include "buffer.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

static void getinput(struct Eli *e, char *input, const char *cmd);
static bool runcmd(struct Eli *e, const char *cmd, const char *arg);

bool command(struct Eli *e, int key)
{
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

bool readfile(struct Eli *e, int key)
{
    return buf_read(e->buf, NULL);
}

bool writefile(struct Eli *e, int key)
{
    return buf_write(e->buf, NULL);
}

bool begofline(struct Eli *e, int key)
{
    e->buf->col = 0;
    return true;
}

bool endofline(struct Eli *e, int key)
{
    e->buf->col = strlen(e->buf->line->str);
    return true;
}

bool nextchar(struct Eli *e, int key)
{
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

bool prevchar(struct Eli *e, int key)
{
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

bool nextword(struct Eli *e, int key)
{
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

bool prevword(struct Eli *e, int key)
{
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

bool nextline(struct Eli *e, int key)
{
    struct Line *nline = e->buf->line->next;
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

bool prevline(struct Eli *e, int key)
{
    struct Line *pline = e->buf->line->prev;
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

bool newline(struct Eli *e, int key)
{
    char *split = e->buf->line->str + e->buf->col;
    size_t len = strlen(split);
    struct Line *l = line_new(split, len);
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

bool backchar(struct Eli *e, int key)
{
    struct Line *l = e->buf->line;
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

bool delchar(struct Eli *e, int key)
{
    line_erase(e->buf->line, e->buf->col);
    return true;
}

bool addchar(struct Eli *e, int key)
{
    line_insert(e->buf->line, e->buf->col, key);
    nextchar(e, key);
    return true;
}

bool newbuf(struct Eli *e, int key)
{
    struct Buffer *buf = buf_new(NULL);
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

bool delbuf(struct Eli *e, int key)
{
    struct Buffer *buf = e->buf;
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

bool nextbuf(struct Eli *e, int key)
{
    if (e->buf->next) {
        e->buf = e->buf->next;
    }
    else {
        e->buf = e->beg;
    }
    return true;
}

bool prevbuf(struct Eli *e, int key)
{
    if (e->buf->prev) {
        e->buf = e->buf->prev;
    }
    else {
        e->buf = e->end;
    }
    return true;
}

bool begofbuf(struct Eli *e, int key)
{
    e->buf->line = e->buf->beg;
    e->buf->row = 0;
    begofline(e, key);
    return true;
}

bool endofbuf(struct Eli *e, int key)
{
    e->buf->line = e->buf->end;
    e->buf->row = e->buf->size - 1;
    endofline(e, key);
    return true;
}

static void getinput(struct Eli *e, char *input, const char *cmd)
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

static bool runcmd(struct Eli *e, const char *cmd, const char *arg)
{
    if (streq(cmd, "save")) {
        return buf_write(e->buf, arg);
    }
    if (streq(cmd, "open")) {
        if (arg) {
            free(e->buf->name);
            e->buf->name = strdup(arg);
        }
        return buf_read(e->buf, arg);
    }
    return false;
}
