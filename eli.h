#ifndef __ELI_EDITOR_H__
#define __ELI_EDITOR_H__

#include "buffer.h"
#include "window.h"

struct Eli_;

typedef enum {
    NORMAL,
    INSERT,
} MODE;

typedef bool (*action_func)(struct Eli_ *, int);
typedef struct {
    int key;
    action_func func;
    MODE nextmode;
} Action;

typedef struct {
    int exit_key;
    Action *actions;
    size_t count;
} Mode;

typedef struct Eli_ {
    Window cmdwin, titlewin, textwin;
    Buffer *buf, *beg, *end;
    Mode mode;
} Eli;

void setmode(Eli *e, MODE m);

#endif /* __ELI_EDITOR_H__ */
