#ifndef __ELI_BUFFER_H__
#define __ELI_BUFFER_H__

#include "line.h"

typedef struct {
    const char *name;
    Line *pos, *beg, *end;
    size_t row, col;
} Buffer;

Buffer * buf_new();
void buf_free(Buffer *buf);
void buf_read(Buffer *buf);
void buf_write(Buffer *buf);
void buf_pushfront(Buffer *buf, Line *line);
void buf_pushback(Buffer *buf, Line *line);
void buf_insert(Buffer *buf, Line *dst, Line *line);
void buf_erase(Buffer *buf, Line *line);
void buf_clear(Buffer *buf);

#endif /* __ELI_BUFFER_H__ */
