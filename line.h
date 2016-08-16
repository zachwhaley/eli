#ifndef __ELI_LINE_H__
#define __ELI_LINE_H__

#include <stdio.h>

struct Line {
    char str[BUFSIZ];
    struct Line *prev, *next;
};

struct Line* line_new(const char *str, size_t len);
void line_free(struct Line *line);
void line_pushback(struct Line *line, const char *str, size_t cnt);
void line_insert(struct Line *line, size_t ndx, char c);
void line_erase(struct Line *line, size_t ndx);

#endif /* __ELI_LINE_H__ */
