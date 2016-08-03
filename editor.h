#ifndef __ELI_EDITOR_H__
#define __ELI_EDITOR_H__

#include "buffer.h"
#include "mode.h"
#include "window.h"

typedef struct {
    Window titlewin;
    Window textwin;
    Buffer *buf, *beg, *end;
    Mode mode;
} Editor;

void setmode(Editor *e, MODE m);
bool readfile(void *ctx, int key);
bool writefile(void *ctx, int key);
bool begofline(void *ctx, int key);
bool endofline(void *ctx, int key);
bool nextchar(void *ctx, int key);
bool prevchar(void *ctx, int key);
bool nextword(void *ctx, int key);
bool prevword(void *ctx, int key);
bool nextline(void *ctx, int key);
bool prevline(void *ctx, int key);
bool newline(void *ctx, int key);
bool backchar(void *ctx, int key);
bool delchar(void *ctx, int key);
bool addchar(void *ctx, int key);
bool newbuf(void *ctx, int key);
bool begofbuf(void *ctx, int key);

#endif /* __ELI_EDITOR_H__ */
