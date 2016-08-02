#ifndef __ELI_MODE_H__
#define __ELI_MODE_H__

#include <stdbool.h>

typedef bool (*action_func)(void *, int);

typedef enum {
    NORMAL,
    INSERT,
} MODE;

typedef struct {
    int key;
    action_func func;
    MODE nextmode;
} Action;

typedef struct {
    int exit_key;
    action_func default_action;
    Action *actions;
    size_t count;
} Mode;

#endif /* __ELI_MODE_H__ */
