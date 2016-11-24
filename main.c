#include "eli.h"
#include "line.h"
#include "buffer.h"

#include <ncurses.h>

#include <string.h>

static void eli_init(struct Eli *eli, int ac, const char *av[]);
static void eli_edit(struct Eli *eli);
static void eli_display(struct Eli *eli);
static void eli_term(struct Eli *eli);

static void eli_init(struct Eli *eli, int ac, const char *av[])
{
    const size_t cols = COLS, lines = LINES;
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

    setmode(eli, NORMAL);
}

static void eli_term(struct Eli *eli)
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
}

static void eli_display(struct Eli *eli)
{
    // Refresh title window
    char title[eli->titlewin.cols];
    struct Buffer *buf = eli->buf;
    int ndx = 0;
    do {
        ndx += snprintf(&title[ndx], sizeof(title) - ndx, " %s :", buf->name ?: "[No Name]");
        buf = buf->next ?: eli->beg;
    } while (buf != eli->buf);
    snprintf(&title[ndx], sizeof(title) - ndx, " %lu,%lu", eli->buf->row, eli->buf->col);
    wclear(eli->titlewin.win);
    mvwaddstr(eli->titlewin.win, 0, 0, title);
    for (size_t col = strlen(title); col < eli->titlewin.cols; col++) {
        waddch(eli->titlewin.win, ' ');
    }

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
    int cur_y = eli->buf->row - eli->textwin.top;
    int cur_x = eli->buf->col;
    wmove(eli->textwin.win, cur_y, cur_x);

    wnoutrefresh(eli->titlewin.win);
    wnoutrefresh(eli->textwin.win);
    doupdate();
}

static void eli_edit(struct Eli *eli)
{
    while (true) {
        // Make sure we always have a buffer
        if (!eli->buf) {
            eli->buf = buf_new(NULL);
            eli->beg = eli->end = eli->buf;
        }
        eli_display(eli);

        int key = wgetch(eli->textwin.win);
        // Should we exit?
        if (key == eli->mode.exit_key)
            return;

        // Find an action for the key
        int ndx;
        struct Action action = {};
        for (ndx = 0; ndx < eli->mode.count; ndx++) {
            action = eli->mode.actions[ndx];
            if (action.key == key) {
                if (action.func)
                    action.func(eli, key);
                goto done;
            }
        }
        // If no action found, resort to the default action
        if (action.func) {
            action.func(eli, key);
        }
done:
        setmode(eli, action.nextmode);
    }
}

int main(int argc, const char *argv[])
{
    initscr();
    raw();
    noecho();

    struct Eli eli = {};
    eli_init(&eli, argc, argv);
    eli_edit(&eli);
    eli_term(&eli);

    endwin();
    return 0;
}
