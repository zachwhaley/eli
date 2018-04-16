#ifndef __ELI_ACTION_H__
#define __ELI_ACTION_H__

#include <stdbool.h>

struct Eli;

bool command(struct Eli *e);
bool readfile(struct Eli *e);
bool writefile(struct Eli *e);
bool begofline(struct Eli *e);
bool endofline(struct Eli *e);
bool nextchar(struct Eli *e);
bool prevchar(struct Eli *e);
bool nextword(struct Eli *e);
bool prevword(struct Eli *e);
bool nextline(struct Eli *e);
bool prevline(struct Eli *e);
bool newline(struct Eli *e);
bool backchar(struct Eli *e);
bool delchar(struct Eli *e);
bool addchar(struct Eli *e);
bool newbuf(struct Eli *e);
bool delbuf(struct Eli *e);
bool nextbuf(struct Eli *e);
bool prevbuf(struct Eli *e);
bool begofbuf(struct Eli *e);
bool endofbuf(struct Eli *e);
bool topofwin(struct Eli *e);
bool midofwin(struct Eli *e);
bool botofwin(struct Eli *e);

#endif /* __ELI_ACTION_H__ */
