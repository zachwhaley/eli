#ifndef __ELI_BUFFER_H__
#define __ELI_BUFFER_H__

#include <stdbool.h>
#include <stddef.h>

struct Line;

struct Buffer {
    char *name;
    struct Buffer *prev, *next;
    struct Line *line, *beg, *end;
    size_t size;
    size_t row, col;
};

struct Buffer* buf_new(const char *name);
void buf_free(struct Buffer *buf);
bool buf_read(struct Buffer *buf, const char *name);
bool buf_write(struct Buffer *buf, const char *name);
void buf_pushfront(struct Buffer *buf, struct Line *line);
void buf_pushback(struct Buffer *buf, struct Line *line);
void buf_insert(struct Buffer *buf, struct Line *dst, struct Line *line);
void buf_erase(struct Buffer *buf, struct Line *line);
void buf_clear(struct Buffer *buf);

#endif /* __ELI_BUFFER_H__ */
