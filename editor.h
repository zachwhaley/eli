#ifndef __ELI_EDITOR_H__
#define __ELI_EDITOR_H__

#include "buffer.h"
#include "mode.h"
#include "window.h"

typedef struct {
    Window titlewin;
    Window textwin;
    Buffer *buf;
    Mode mode;
} Editor;

void setmode(Editor *e, MODE m);
void * readfile(void *ctx, int key);
void * writefile(void *ctx, int key);
void * begofline(void *ctx, int key);
void * endofline(void *ctx, int key);
void * nextchar(void *ctx, int key);
void * prevchar(void *ctx, int key);
void * nextline(void *ctx, int key);
void * prevline(void *ctx, int key);
void * newline(void *ctx, int key);
void * backchar(void *ctx, int key);
void * delchar(void *ctx, int key);
void * addchar(void *ctx, int key);

#endif /* __ELI_EDITOR_H__ */
