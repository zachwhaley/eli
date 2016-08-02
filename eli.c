#include "editor.h"

#include <ncurses.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void eli_init(Editor *eli, int ac, const char *av[]);
static void eli_edit(Editor *eli);
static void eli_display(Editor *eli);
static void eli_term(Editor *eli);

static void eli_init(Editor *eli, int ac, const char *av[])
{
    const size_t cols = COLS, lines = LINES;
    eli->titlewin.win = newwin(1, cols, lines - 1, 0);
    eli->titlewin.cols = cols;
    eli->titlewin.lines = 1;
    wattron(eli->titlewin.win, A_REVERSE);

    eli->textwin.win = newwin(lines - 1, cols, 0, 0);
    eli->textwin.cols = cols;
    eli->textwin.lines = lines - 1;
    eli->textwin.bot = eli->textwin.lines - 1;
    keypad(eli->textwin.win, true);

    eli->buf = buf_new();
    if (ac > 1) {
        eli->buf->name = av[1];
        readfile(eli, 0);
    }
    eli->buf->line = eli->buf->beg;

    setmode(eli, NORMAL);
}

static void eli_term(Editor *eli)
{
    buf_free(eli->buf);
}

static void eli_display(Editor *eli)
{
    // Refresh title window
    char title[eli->titlewin.cols];
    snprintf(title, sizeof(title), " %s %lu,%lu", eli->buf->name ?: "#No File#", eli->buf->row, eli->buf->col);
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
    Line *wline = eli->buf->beg;
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

static void eli_edit(Editor *eli)
{
    while (true) {
        eli_display(eli);

        int key = wgetch(eli->textwin.win);
        // Should we exit?
        if (key == eli->mode.exit_key)
            return;

        // Find an action for the key
        int ndx;
        for (ndx = 0; ndx < eli->mode.count; ndx++) {
            Action action = eli->mode.actions[ndx];
            if (action.key == key) {
                if (action.func)
                    action.func(eli, key);
                setmode(eli, action.nextmode);
                break;
            }
        }
        // If no action found, resort to the default action
        if (ndx == eli->mode.count && eli->mode.default_action)
            eli->mode.default_action(eli, key);
    }
}

int main(int argc, const char *argv[])
{
    initscr();
    raw();
    noecho();

    Editor eli = {};
    eli_init(&eli, argc, argv);
    eli_edit(&eli);
    eli_term(&eli);

    endwin();
    return 0;
}
