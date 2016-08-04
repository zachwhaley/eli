#ifndef __ELI_BUFFER_H__
#define __ELI_BUFFER_H__

#include "line.h"

#include <stdbool.h>

typedef struct Buffer_ {
    char *name;
    struct Buffer_ *prev, *next;
    Line *line, *beg, *end;
    size_t size;
    size_t row, col;
} Buffer;

Buffer * buf_new();
void buf_free(Buffer *buf);
bool buf_read(Buffer *buf);
bool buf_write(Buffer *buf);
void buf_pushfront(Buffer *buf, Line *line);
void buf_pushback(Buffer *buf, Line *line);
void buf_insert(Buffer *buf, Line *dst, Line *line);
void buf_erase(Buffer *buf, Line *line);
void buf_clear(Buffer *buf);

#endif /* __ELI_BUFFER_H__ */
