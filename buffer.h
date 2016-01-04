#ifndef __ELI_BUFFER_H__
#define __ELI_BUFFER_H__

#include "line.h"

typedef struct {
    Line *beg, *end;
} Buffer;

void buf_pushfront(Buffer *buf, Line *line);
void buf_pushback(Buffer *buf, Line *line);
void buf_insert(Buffer *buf, Line *dst, Line *line);
void buf_erase(Buffer *buf, Line *line);
void buf_clear(Buffer *buf);

#endif /* __ELI_BUFFER_H__ */
