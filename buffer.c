#include "buffer.h"

#include "line.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Buffer* buf_new(const char *name)
{
    struct Buffer *buf = calloc(1, sizeof(struct Buffer));
    if (name)
        buf->name = strdup(name);
    buf_read(buf, name);
    return buf;
}

void buf_free(struct Buffer *buf)
{
    buf_clear(buf);
    buf->next = buf->prev = NULL;
    free(buf->name);
    buf->name = '\0';
    free(buf);
}

bool buf_read(struct Buffer *buf, const char *name)
{
    buf_clear(buf);
    FILE *fp = fopen(name ?: buf->name, "r");
    if (fp) {
        char in[BUFSIZ];
        while (fgets(in, BUFSIZ, fp) != NULL) {
            struct Line *l = line_new(in, strlen(in) - 1);
            buf_pushback(buf, l);
        }
        buf->line = buf->beg;
        fclose(fp);
        return true;
    }
    else {
        struct Line *l = line_new(NULL, 0);
        buf_pushback(buf, l);
        buf->line = l;
    }
    return false;
}

bool buf_write(struct Buffer *buf, const char *name)
{
    FILE *fp = fopen(name ?: buf->name, "w");
    if (fp) {
        for (struct Line *l = buf->beg; l != NULL; l = l->next) {
            fprintf(fp, "%s\n", l->str);
        }
        fclose(fp);
        return true;
    }
    return false;
}

void buf_pushback(struct Buffer *buf, struct Line *line)
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

void buf_pushfront(struct Buffer *buf, struct Line *line)
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

void buf_insert(struct Buffer *buf, struct Line *dst, struct Line *line)
{
    line->next = dst;
    line->prev = dst->prev;
    if (line->next)
        line->next->prev = line;
    if (line->prev)
        line->prev->next = line;
    buf->size++;
}

void buf_erase(struct Buffer *buf, struct Line *line)
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

void buf_clear(struct Buffer *buf)
{
    struct Line *l = buf->beg;
    while (l) {
        struct Line *nl = l->next;
        line_free(l);
        l = nl;
    }
    buf->size = 0;
    buf->row = buf->col = 0;
    buf->line = buf->beg = buf->end = NULL;
}
