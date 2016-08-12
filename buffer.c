#include "buffer.h"
#include "line.h"

#include <stdlib.h>
#include <string.h>

Buffer * buf_new(const char *name)
{
    Buffer *buf = calloc(1, sizeof(Buffer));
    if (name)
        buf->name = strdup(name);
    buf_read(buf, name);
    return buf;
}

void buf_free(Buffer *buf)
{
    buf_clear(buf);
    buf->next = buf->prev = NULL;
    free(buf->name);
    buf->name = '\0';
    free(buf);
}

bool buf_read(Buffer *buf, const char *name)
{
    buf_clear(buf);
    FILE *fp = fopen(name ?: buf->name, "r");
    if (fp) {
        char in[BUFSIZ];
        while (fgets(in, BUFSIZ, fp) != NULL) {
            Line *l = line_new(in, strlen(in) - 1);
            buf_pushback(buf, l);
        }
        buf->line = buf->beg;
        fclose(fp);
        return true;
    }
    else {
        Line *l = line_new(NULL, 0);
        buf_pushback(buf, l);
        buf->line = l;
    }
    return false;
}

bool buf_write(Buffer *buf, const char *name)
{
    FILE *fp = fopen(name ?: buf->name, "w");
    if (fp) {
        for (Line *l = buf->beg; l != NULL; l = l->next) {
            fprintf(fp, "%s\n", l->str);
        }
        fclose(fp);
        return true;
    }
    return false;
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
    buf->size++;
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
    buf->size++;
}

void buf_insert(Buffer *buf, Line *dst, Line *line)
{
    line->next = dst;
    line->prev = dst->prev;
    if (line->next)
        line->next->prev = line;
    if (line->prev)
        line->prev->next = line;
    buf->size++;
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
    buf->size--;
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
    buf->size = 0;
    buf->row = buf->col = 0;
    buf->line = buf->beg = buf->end = NULL;
}
