#ifndef __ELI_WINDOW_H__
#define __ELI_WINDOW_H__

#include <ncurses.h>

typedef struct {
    WINDOW *win;
    size_t cols, lines;
    size_t top, bot;
} Window;

#endif /* __ELI_WINDOW_H__ */
