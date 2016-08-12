#ifndef __ELI_EDITOR_H__
#define __ELI_EDITOR_H__

#include "buffer.h"
#include "mode.h"
#include "window.h"

typedef struct {
    Window cmdwin, titlewin, textwin;
    Buffer *buf, *beg, *end;
    Mode mode;
} Eli;

void setmode(Eli *e, MODE m);

#endif /* __ELI_EDITOR_H__ */
