#include "file.h"

#include <ncurses.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COUNT(x) (sizeof (x) / sizeof *(x))
#define CTRL(chr) (chr & 037)

typedef void (*action_func)(File *, int);

typedef struct {
    WINDOW *win;
    size_t cols, lines;
    size_t top, bot;
} Window;

typedef enum {
    NORMAL,
    INSERT,
} MODE;

typedef struct {
    int key;
    action_func func;
    MODE nextmode;
} Action;

typedef struct {
    int exit_key;
    action_func default_action;
    Action *actions;
    size_t count;
} Mode;

static void init(int ac, const char *av[]);
static void term();
static void writefile(File *, int);
static char * getinput(const char *msg, char *input);
static void setmode(MODE m);
static void display();
static void edit();

Action normal_actions[] = {
    { CTRL('w'), writefile,     NORMAL },
    { 'i',       NULL,          INSERT },
    { 'a',       file_nextchar, INSERT },
    { 'h',       file_prevchar, NORMAL },
    { 'j',       file_nextline, NORMAL },
    { 'k',       file_prevline, NORMAL },
    { 'l',       file_nextchar, NORMAL },
};
Mode normal_mode = {
    .exit_key = CTRL('q'),
    .default_action = NULL,
    .actions = normal_actions,
    .count = COUNT(normal_actions),
};

Action insert_actions[] = {
    { CTRL('w'),     writefile,      INSERT },
    { KEY_HOME,      file_begofline, INSERT },
    { KEY_END,       file_endofline, INSERT },
    { KEY_UP,        file_prevline,  INSERT },
    { KEY_DOWN,      file_nextline,  INSERT },
    { KEY_RIGHT,     file_nextchar,  INSERT },
    { KEY_LEFT,      file_prevchar,  INSERT },
    { '\n',          file_newline,   INSERT },
    { '\r',          file_newline,   INSERT },
    { KEY_BACKSPACE, file_backchar,  INSERT },
    { CTRL('i'),     NULL,           NORMAL },
};
Mode insert_mode = {
    .exit_key = CTRL('q'),
    .default_action = file_addchar,
    .actions = insert_actions,
    .count = COUNT(insert_actions),
};

Mode mode = {};
Window titlewin = {};
Window textwin = {};
File file = {};

static void init(int ac, const char *av[])
{
    const size_t cols = COLS, lines = LINES;
    titlewin.win = newwin(1, cols, lines - 1, 0);
    titlewin.cols = cols;
    titlewin.lines = 1;
    wattron(titlewin.win, A_REVERSE);

    textwin.win = newwin(lines - 1, cols, 0, 0);
    textwin.cols = cols;
    textwin.lines = lines - 1;
    textwin.bot = textwin.lines - 1;
    keypad(textwin.win, true);

    if (ac > 1) {
        file.name = av[1];
        file_read(&file);
    }
    if (!file.buf.beg) {
        Line *l = line_new(NULL, 0);
        buf_pushback(&file.buf, l);
    }
    file.pos = file.buf.beg;

    setmode(NORMAL);
}

static void term()
{
    buf_clear(&file.buf);
}

// TODO: Need a better way to get user input for an action
static void writefile(File *file, int key)
{
    if (!file->name) {
        char input[FILENAME_MAX];
        file->name = getinput("Save as:", input);
    }
    file_write(file);
}

static char * getinput(const char *msg, char *input)
{
    // Title message
    wclear(titlewin.win);
    mvwaddstr(titlewin.win, 0, 0, msg);

    // User input area
    wattroff(titlewin.win, A_REVERSE);
    waddch(titlewin.win, ' ');
    wrefresh(titlewin.win);

    // Get user input
    echo();
    wgetstr(titlewin.win, input);
    noecho();

    // Make the title window white again
    wattron(titlewin.win, A_REVERSE);
    return input;
}

static void setmode(MODE m)
{
    switch (m) {
        case INSERT:
            mode = insert_mode;
            break;
        case NORMAL:
            mode = normal_mode;
            break;
    }
}

static void display()
{
    // Refresh title window
    char title[titlewin.cols];
    snprintf(title, sizeof(title), " %s %lu,%lu",
             (file.name) ? file.name : "#No File#", file.row, file.col);
    wclear(titlewin.win);
    mvwaddstr(titlewin.win, 0, 0, title);
    for (size_t col = strlen(title); col < titlewin.cols; col++) {
        waddch(titlewin.win, ' ');
    }

    // Adjust window top and bottom
    if (file.row > textwin.bot) {
        textwin.top += file.row - textwin.bot;
        textwin.bot = file.row;
    }
    else if (file.row < textwin.top) {
        textwin.bot -= textwin.top - file.row;
        textwin.top = file.row;
    }
    // Refresh text window
    size_t wline;
    Line *fline = file.buf.beg;
    for (wline = 0; wline < textwin.top && fline; wline++) {
        fline = fline->next;
    }
    for (wline = 0; wline < textwin.lines; wline++) {
        wmove(textwin.win, wline, 0);
        for (size_t col = 0; col < textwin.cols; col++) {
            if (fline && col < strlen(fline->str))
                waddch(textwin.win, fline->str[col]);
            else
                waddch(textwin.win, ' ');
        }
        if (fline)
            fline = fline->next;
    }
    // Set cursor File
    int cur_y = file.row - textwin.top;
    int cur_x = file.col;
    wmove(textwin.win, cur_y, cur_x);

    wnoutrefresh(titlewin.win);
    wnoutrefresh(textwin.win);
    doupdate();
}

static void edit()
{
    while (true) {
        display();

        int key = wgetch(textwin.win);
        // Should we exit?
        if (key == mode.exit_key)
            return;

        // Find an action for the key
        int ndx;
        for (ndx = 0; ndx < mode.count; ndx++) {
            Action action = mode.actions[ndx];
            if (action.key == key) {
                if (action.func)
                    action.func(&file, key);
                setmode(action.nextmode);
                break;
            }
        }
        // If no action found, resort to the default action
        if (ndx == mode.count && mode.default_action)
            mode.default_action(&file, key);
    }
}

int main(int argc, const char *argv[])
{
    initscr();
    raw();
    noecho();

    init(argc, argv);
    edit();
    term();

    endwin();
    return 0;
}
