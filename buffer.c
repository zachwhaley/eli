#include "buffer.h"
#include "line.h"

#include <stdlib.h>

Buffer * buf_new()
{
    Buffer *b = (Buffer *)calloc(1, sizeof(Buffer));
    return b;
}

void buf_free(Buffer *buf)
{
    buf_clear(buf);
    free(buf);
}

void buf_pushback(Buffer *buf, Line *line)
{
    if (buf->end) {
        line->prev = buf->end;
        line->prev->next = line;
        buf->end = line;
    }
    else {
        buf->beg = buf->end = line;
    }
}

void buf_pushfront(Buffer *buf, Line *line)
{
    if (buf->beg) {
        line->next = buf->beg;
        line->next->prev = line;
        buf->beg = line;
    }
    else {
        buf->beg = buf->end = line;
    }
}

void buf_insert(Buffer *buf, Line *dst, Line *line)
{
    line->next = dst;
    line->prev = dst->prev;
    if (line->next)
        line->next->prev = line;
    if (line->prev)
        line->prev->next = line;
}

void buf_erase(Buffer *buf, Line *line)
{
    if (line == buf->beg)
        buf->beg = line->next;
    else
        line->prev->next = line->next;

    if (line == buf->end)
        buf->end = line->prev;
    else
        line->next->prev = line->prev;

    free(line);
}

void buf_clear(Buffer *buf)
{
    Line *l = buf->beg;
    while (l) {
        Line *nl = l->next;
        line_free(l);
        l = nl;
    }
    buf->beg = buf->end = NULL;
}
