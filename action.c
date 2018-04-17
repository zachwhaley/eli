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

bool command(struct Eli *e)
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

bool readfile(struct Eli *e)
{
    return buf_read(e->buf, NULL);
}

bool writefile(struct Eli *e)
{
    return buf_write(e->buf, NULL);
}

bool begofline(struct Eli *e)
{
    e->buf->col = 0;
    return true;
}

bool endofline(struct Eli *e)
{
    e->buf->col = line_len(e->buf->line);
    return true;
}

bool nextline(struct Eli *e)
{
    return buf_moverow(e->buf, e->buf->row + 1);
}

bool prevline(struct Eli *e)
{
    return buf_moverow(e->buf, e->buf->row - 1);
}

bool nextchar(struct Eli *e)
{
    size_t col = e->buf->col;
    struct Line *line = e->buf->line;
    if (buf_movecol(e->buf, col + 1)) {
        return true;
    }
    else if (line->next) {
        nextline(e);
        begofline(e);
        return true;
    }
    return false;
}

bool prevchar(struct Eli *e)
{
    size_t col = e->buf->col;
    struct Line *line = e->buf->line;
    if (col > 0) {
        if (!buf_movecol(e->buf, col - 1))
            return endofline(e);
        return true;
    }
    else if (line->prev) {
        prevline(e);
        endofline(e);
        return true;
    }
    return false;
}

bool nextword(struct Eli *e)
{
    while (nextchar(e)) {
        size_t col = e->buf->col;
        char *str = e->buf->line->str;
        char ch = str[col];
        if (!isblank(ch) && ch != '\0') {
            if (col == 0) {
                return true;
            }
            else {
                char pchar = str[col - 1];
                if (isblank(pchar))
                    return true;
            }
        }
    }
    return false;
}

bool prevword(struct Eli *e)
{
    while (prevchar(e)) {
        size_t col = e->buf->col;
        char *str = e->buf->line->str;
        char ch = str[col];
        if (!isblank(ch) && ch != '\0') {
            if (col == 0) {
                return true;
            }
            else {
                char pchar = str[col - 1];
                if (isblank(pchar))
                    return true;
            }
        }
    }
    return false;
}

bool begofbuf(struct Eli *e)
{
    e->buf->line = e->buf->beg;
    e->buf->row = 0;
    begofline(e);
    return true;
}

bool endofbuf(struct Eli *e)
{
    e->buf->line = e->buf->end;
    e->buf->row = e->buf->size - 1;
    endofline(e);
    return true;
}

bool topofwin(struct Eli *e)
{
    return buf_moverow(e->buf, e->textwin.top);
}

bool midofwin(struct Eli *e)
{
    struct Window *win = &e->textwin;
    size_t mid = win->top + ((win->bot - win->top) / 2);
    return buf_moverow(e->buf, mid);
}

bool botofwin(struct Eli *e)
{
    if (!buf_moverow(e->buf, e->textwin.bot)) {
        return endofbuf(e);
    }
    return true;
}

bool newline(struct Eli *e)
{
    struct Line *line e->buf->line;
    char *split = line->str + e->buf->col;
    size_t len = strlen(split);
    struct Line *newln = line_new(split, len);
    memset(split, '\0', len);
    if (line->next) {
        buf_insert(e->buf, line->next, newln);
    }
    else {
        buf_pushback(e->buf, newln);
    }
    nextline(e);
    begofline(e);
    return true;
}

bool backchar(struct Eli *e)
{
    struct Line *line = e->buf->line;
    if (!prevchar(e))
        return false;

    // If we moved to the previous line, we need to bring what was left of the line below to
    // our current line
    if (e->buf->line == line->prev) {
        line_pushback(e->buf->line, line->str, line_len(line));
        buf_erase(e->buf, line);
    }
    else {
        delchar(e);
    }
    return true;
}

bool delchar(struct Eli *e)
{
    size_t len = line_len(e->buf->line);
    size_t ndx = (e->buf->col > len)? e->textwin.cur_x : e->buf->col;
    line_erase(e->buf->line, ndx);
    return true;
}

bool addchar(struct Eli *e)
{
    size_t len = line_len(e->buf->line);
    size_t ndx = (e->buf->col > len)? e->textwin.cur_x : e->buf->col;
    line_insert(e->buf->line, ndx, e->key);
    return buf_movecol(e->buf, ndx + 1);
}

bool newbuf(struct Eli *e)
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

bool delbuf(struct Eli *e)
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

bool nextbuf(struct Eli *e)
{
    if (e->buf->next)
        e->buf = e->buf->next;
    else
        e->buf = e->beg;
    return true;
}

bool prevbuf(struct Eli *e)
{
    if (e->buf->prev)
        e->buf = e->buf->prev;
    else
        e->buf = e->end;
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
