#include "eli.h"

#include "line.h"
#include "action.h"
#include "buffer.h"

#include <ncurses.h>

#include <stdlib.h>
#include <string.h>

#define COUNT(x) (sizeof (x) / sizeof *(x))
#define CTRL(chr) (chr & 037)

// Modal Actions
static struct Action normal_actions[] = {
    { CTRL('s'), writefile, NORMAL },
    { CTRL('o'), readfile,  NORMAL },
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
    { 'H',       topofwin,  NORMAL },
    { 'M',       midofwin,  NORMAL },
    { 'L',       botofwin,  NORMAL },
    { ':',       command,   NORMAL },
    // Default Action
    { '\0',      NULL,      NORMAL },
};
static const struct Mode normal_mode = {
    .exit_key = CTRL('q'),
    .actions = normal_actions,
    .count = COUNT(normal_actions),
};

static struct Action insert_actions[] = {
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
static const struct Mode insert_mode = {
    .exit_key = CTRL('q'),
    .actions = insert_actions,
    .count = COUNT(insert_actions),
};

static void setmode(struct Eli *e, enum MODE m);
static void drawscreen(struct Eli *eli);

struct Eli* eli_init(int ac, const char *av[])
{
    initscr();
    raw();
    noecho();

    const size_t cols = COLS, lines = LINES;

    struct Eli *eli = calloc(1, sizeof(struct Eli));

    eli->cmdwin.win = newwin(1, cols, lines - 1, 0);
    eli->cmdwin.cols = cols;
    eli->cmdwin.lines = 1;

    eli->titlewin.win = newwin(1, cols, lines - 2, 0);
    eli->titlewin.cols = cols;
    eli->titlewin.lines = 1;
    wattron(eli->titlewin.win, A_REVERSE);

    eli->textwin.win = newwin(lines - 2, cols, 0, 0);
    eli->textwin.cols = cols;
    eli->textwin.lines = lines - 2;
    eli->textwin.bot = eli->textwin.lines - 1;
    keypad(eli->textwin.win, true);

    for (int i = ac - 1; i > 0; i--) {
        struct Buffer *buf = buf_new(av[i]);
        if (!eli->end) {
            eli->buf = eli->end = buf;
        }
        else {
            eli->buf->prev = buf;
            buf->next = eli->buf;
            eli->buf = buf;
        }
    }
    eli->beg = eli->buf;
    eli->key = 0;
    setmode(eli, NORMAL);

    return eli;
}

void eli_run(struct Eli *eli)
{
    while (true) {
        // Make sure we always have a buffer
        if (!eli->buf) {
            eli->buf = buf_new(NULL);
            eli->beg = eli->end = eli->buf;
        }
        drawscreen(eli);

        eli->key = wgetch(eli->textwin.win);
        // Should we exit?
        if (eli->key == eli->mode.exit_key)
            return;

        // Find an action for the key
        struct Action action = {};
        for (int ndx = 0; ndx < eli->mode.count; ndx++) {
            action = eli->mode.actions[ndx];
            if (action.key == eli->key) {
                if (action.func)
                    action.func(eli);
                goto done;
            }
        }
        // If no action found, resort to the default action
        if (action.func) {
            action.func(eli);
        }
done:
        setmode(eli, action.nextmode);
    }
}

void eli_term(struct Eli *eli)
{
    struct Buffer *buf = eli->beg;
    while (buf) {
        struct Buffer *nbuf = buf->next;
        buf_free(buf);
        buf = nbuf;
    }
    delwin(eli->cmdwin.win);
    delwin(eli->titlewin.win);
    delwin(eli->textwin.win);
    free(eli);

    endwin();
}

static void setmode(struct Eli *e, enum MODE m)
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

static void drawscreen(struct Eli *eli)
{
    // Adjust window top and bottom
    if (eli->buf->row > eli->textwin.bot) {
        eli->textwin.top += eli->buf->row - eli->textwin.bot;
        eli->textwin.bot = eli->buf->row;
    }
    else if (eli->buf->row < eli->textwin.top) {
        eli->textwin.bot -= eli->textwin.top - eli->buf->row;
        eli->textwin.top = eli->buf->row;
    }

    // Refresh text window
    size_t winrow;
    struct Line *wline = eli->buf->beg;
    for (winrow = 0; winrow < eli->textwin.top && wline; winrow++) {
        wline = wline->next;
    }
    for (winrow = 0; winrow < eli->textwin.lines; winrow++) {
        wmove(eli->textwin.win, winrow, 0);
        for (size_t col = 0; col < eli->textwin.cols; col++) {
            if (wline && col < strlen(wline->str))
                waddch(eli->textwin.win, wline->str[col]);
            else
                waddch(eli->textwin.win, ' ');
        }
        if (wline)
            wline = wline->next;
    }

    // Set cursor File
    size_t col = eli->buf->col;
    size_t len = line_len(eli->buf->line);
    eli->textwin.cur_y = eli->buf->row - eli->textwin.top;
    eli->textwin.cur_x = (col < len)? col : len;
    wmove(eli->textwin.win, eli->textwin.cur_y, eli->textwin.cur_x);

    // Refresh title window
    char title[eli->titlewin.cols];
    struct Buffer *buf = eli->buf;
    int ndx = 0;
    do {
        ndx += snprintf(&title[ndx], sizeof(title) - ndx, " %s :", buf->name ?: "[No Name]");
        buf = buf->next ?: eli->beg;
    } while (buf != eli->buf);
    snprintf(&title[ndx], sizeof(title) - ndx, " %lu,%lu", eli->textwin.cur_y, eli->textwin.cur_x);
    wclear(eli->titlewin.win);
    mvwaddstr(eli->titlewin.win, 0, 0, title);
    for (size_t col = strlen(title); col < eli->titlewin.cols; col++) {
        waddch(eli->titlewin.win, ' ');
    }

    wnoutrefresh(eli->titlewin.win);
    wnoutrefresh(eli->textwin.win);
    doupdate();
}
