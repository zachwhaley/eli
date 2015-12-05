#include "file.h"

#include <ncurses.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CTRL(chr) (chr & 037)

typedef struct {
    WINDOW *win;
    size_t cols, lines;
    size_t top, bot;
} Window;

Window titlewin = {};
Window textwin = {};

File file = {};

void init(int ac, const char *av[]);
void term();
Buffer * readfile(const char *fname);
void writefile(const char *fname);
char * getinput(const char *msg, char *input);
void display();
void edit();

void init(int ac, const char *av[])
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
        file.buf = readfile(file.name);
    }
    if (!file.buf) {
        file.buf = buf_new();
        Line *l = line_new(NULL, 0);
        buf_pushback(file.buf, l);
    }
    file.pos = file.buf->beg;
}

void term()
{
    buf_free(file.buf);
}

Buffer * readfile(const char *fname)
{
    Buffer *buf = NULL;
    FILE *fp = fopen(fname, "r");
    if (fp) {
        buf = buf_new();
        char fl[BUFSIZ];
        while (fgets(fl, BUFSIZ, fp) != NULL) {
            Line *l = line_new(fl, strlen(fl) - 1);
            buf_pushback(buf, l);
        }
        fclose(fp);
    }
    return buf;
}

void writefile(const char *fname)
{
    FILE *fp = fopen(fname, "w");
    if (fp) {
        for (Line *l = file.buf->beg; l != NULL; l = l->next) {
            fprintf(fp, "%s\n", l->str);
        }
        fclose(fp);
    }
}

char * getinput(const char *msg, char *input)
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

void display()
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
    Line *fline = file.buf->beg;
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

void edit()
{
    while (true) {
        display();

        int ch = wgetch(textwin.win);
        switch (ch) {
            case CTRL('q'):
                return;
            case CTRL('w'):
                if (!file.name) {
                    char input[FILENAME_MAX];
                    file.name = getinput("Save as:", input);
                }
                writefile(file.name);
                break;
            case KEY_HOME:
                begofline(&file, ch);
                break;
            case KEY_END:
                endofline(&file, ch);
                break;
            case KEY_UP:
                prevline(&file, ch);
                break;
            case KEY_DOWN:
                nextline(&file, ch);
                break;
            case KEY_RIGHT:
                nextchar(&file, ch);
                break;
            case KEY_LEFT:
                prevchar(&file, ch);
                break;
            case '\n':
            case '\r':
                newline(&file, ch);
                break;
            case KEY_BACKSPACE:
                backchar(&file, ch);
                break;
            default:
                addchar(&file, ch);
        }
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
