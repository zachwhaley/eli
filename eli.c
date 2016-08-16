#include "eli.h"

#include "action.h"
#include "buffer.h"

#define COUNT(x) (sizeof (x) / sizeof *(x))
#define CTRL(chr) (chr & 037)

// Modal Actions
static struct Action normal_actions[] = {
    { CTRL('s'), writefile, NORMAL },
    { CTRL('o'), readfile,  NORMAL },
    { CTRL('t'), newbuf,    NORMAL },
    { CTRL('w'), delbuf,    NORMAL },
    { CTRL('n'), nextbuf,   NORMAL },
    { CTRL('p'), prevbuf,   NORMAL },
    { 'i',       NULL,      INSERT },
    { 'g',       begofbuf,  NORMAL },
    { 'G',       endofbuf,  NORMAL },
    { 'a',       nextchar,  INSERT },
    { 'h',       prevchar,  NORMAL },
    { 'j',       nextline,  NORMAL },
    { 'k',       prevline,  NORMAL },
    { 'l',       nextchar,  NORMAL },
    { 'w',       nextword,  NORMAL },
    { 'b',       prevword,  NORMAL },
    { 'x',       delchar,   NORMAL },
    { ':',       command,   NORMAL },
    // Default Action
    { '\0',      NULL,      NORMAL },
};
static const struct Mode normal_mode = {
    .exit_key = CTRL('q'),
    .actions = normal_actions,
    .count = COUNT(normal_actions),
};

static struct Action insert_actions[] = {
    { CTRL('s'),     writefile, INSERT },
    { KEY_HOME,      begofline, INSERT },
    { KEY_END,       endofline, INSERT },
    { KEY_UP,        prevline,  INSERT },
    { KEY_DOWN,      nextline,  INSERT },
    { KEY_RIGHT,     nextchar,  INSERT },
    { KEY_LEFT,      prevchar,  INSERT },
    { '\n',          newline,   INSERT },
    { '\r',          newline,   INSERT },
    { KEY_BACKSPACE, backchar,  INSERT },
    { KEY_DC,        delchar,   INSERT },
    { CTRL('c'),     NULL,      NORMAL },
    // Default Action
    { '\0',          addchar,   INSERT },
};
static const struct Mode insert_mode = {
    .exit_key = CTRL('q'),
    .actions = insert_actions,
    .count = COUNT(insert_actions),
};

void setmode(struct Eli *e, enum MODE m)
{
    switch (m) {
        case INSERT:
            e->mode = insert_mode;
            break;
        case NORMAL:
            e->mode = normal_mode;
            break;
    }
}
