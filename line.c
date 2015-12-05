#include "line.h"

#include <stdlib.h>
#include <string.h>

Line * line_new(const char *str, size_t len)
{
    Line *l = (Line *)calloc(1, sizeof(Line));
    if (str) {
        strncpy(l->str, str, len);
    }
    return l;
}

void line_free(Line *line)
{
    line->str[0] = '\0';
    line->next = line->prev = NULL;
    free(line);
}

void line_pushback(Line *line, const char *str, size_t cnt)
{
    strncat(line->str, str, cnt);
}

void line_insert(Line *line, size_t ndx, char chr)
{
    if (ndx < strlen(line->str)) {
        char *split = line->str + ndx;
        memmove(split + 1, split, strlen(split));
    }
    line->str[ndx] = chr;
}

void line_erase(Line *line, size_t ndx)
{
    if (ndx < strlen(line->str)) {
        char *split = line->str + ndx;
        memmove(split, split + 1, strlen(split));
    }
}
