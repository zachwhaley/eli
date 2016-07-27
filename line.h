#ifndef __ELI_LINE_H__
#define __ELI_LINE_H__

#include <stdio.h>

typedef struct Line_ {
    char str[BUFSIZ];
    struct Line_ *prev, *next;
} Line;

Line * line_new(const char *str, size_t len);
void line_free(Line *line);
void line_pushback(Line *line, const char *str, size_t cnt);
void line_insert(Line *line, size_t ndx, char c);
void line_erase(Line *line, size_t ndx);

#endif /* __ELI_LINE_H__ */
