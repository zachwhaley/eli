#ifndef __ELI_ACTION_H__
#define __ELI_ACTION_H__

#include <stdbool.h>

struct Eli;

bool command(struct Eli *e, int key);
bool readfile(struct Eli *e, int key);
bool writefile(struct Eli *e, int key);
bool begofline(struct Eli *e, int key);
bool endofline(struct Eli *e, int key);
bool nextchar(struct Eli *e, int key);
bool prevchar(struct Eli *e, int key);
bool nextword(struct Eli *e, int key);
bool prevword(struct Eli *e, int key);
bool nextline(struct Eli *e, int key);
bool prevline(struct Eli *e, int key);
bool newline(struct Eli *e, int key);
bool backchar(struct Eli *e, int key);
bool delchar(struct Eli *e, int key);
bool addchar(struct Eli *e, int key);
bool newbuf(struct Eli *e, int key);
bool delbuf(struct Eli *e, int key);
bool nextbuf(struct Eli *e, int key);
bool prevbuf(struct Eli *e, int key);
bool begofbuf(struct Eli *e, int key);
bool endofbuf(struct Eli *e, int key);

#endif /* __ELI_ACTION_H__ */
