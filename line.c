#include "line.h"

#include <stdlib.h>
#include <string.h>

struct Line* line_new(const char *str, size_t len)
{
    struct Line *l = calloc(1, sizeof(struct Line));
    if (str) {
        strncpy(l->str, str, len);
    }
    return l;
}

void line_free(struct Line *line)
{
    line->str[0] = '\0';
    line->next = line->prev = NULL;
    free(line);
}

void line_pushback(struct Line *line, const char *str, size_t cnt)
{
    strncat(line->str, str, cnt);
}

void line_insert(struct Line *line, size_t ndx, char chr)
{
    if (ndx < strlen(line->str)) {
        char *split = line->str + ndx;
        memmove(split + 1, split, strlen(split));
    }
    line->str[ndx] = chr;
}

void line_erase(struct Line *line, size_t ndx)
{
    if (ndx < strlen(line->str)) {
        char *split = line->str + ndx;
        memmove(split, split + 1, strlen(split));
    }
}
