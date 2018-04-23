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
    size_t size;
};

struct Eli {
    struct Window cmdwin, titlewin, textwin;
    struct Buffer *buf, *beg, *end;
    struct Mode mode;
    int key;
};

struct Eli* eli_init(int ac, const char *av[]);
void eli_run(struct Eli *eli);
void eli_term(struct Eli *eli);

#endif /* __ELI_EDITOR_H__ */
