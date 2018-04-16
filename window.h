#ifndef __ELI_WINDOW_H__
#define __ELI_WINDOW_H__

#include <ncurses.h>

struct Window {
    WINDOW *win;
    size_t cols, lines;
    size_t top, bot;
    size_t cur_y, cur_x;
};

#endif /* __ELI_WINDOW_H__ */
