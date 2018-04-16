#ifndef __ELI_EDITOR_H__
#define __ELI_EDITOR_H__

#include "window.h"

struct Eli;
struct Buffer;

enum MODE {
    NORMAL,
    INSERT,
};

struct Action {
    int key;
    bool (*func)(struct Eli*);
    enum MODE nextmode;
};

struct Mode {
    int exit_key;
    struct Action *actions;
    size_t count;
};

struct Eli {
    struct Window cmdwin, titlewin, textwin;
    struct Buffer *buf, *beg, *end;
    struct Mode mode;
    int key;
};

void setmode(struct Eli *e, enum MODE m);

#endif /* __ELI_EDITOR_H__ */
