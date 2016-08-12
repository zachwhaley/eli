#ifndef __ELI_ACTION_H__
#define __ELI_ACTION_H__

#include <stdbool.h>

struct Eli_;

bool command(struct Eli_ *e, int key);
bool readfile(struct Eli_ *e, int key);
bool writefile(struct Eli_ *e, int key);
bool begofline(struct Eli_ *e, int key);
bool endofline(struct Eli_ *e, int key);
bool nextchar(struct Eli_ *e, int key);
bool prevchar(struct Eli_ *e, int key);
bool nextword(struct Eli_ *e, int key);
bool prevword(struct Eli_ *e, int key);
bool nextline(struct Eli_ *e, int key);
bool prevline(struct Eli_ *e, int key);
bool newline(struct Eli_ *e, int key);
bool backchar(struct Eli_ *e, int key);
bool delchar(struct Eli_ *e, int key);
bool addchar(struct Eli_ *e, int key);
bool newbuf(struct Eli_ *e, int key);
bool delbuf(struct Eli_ *e, int key);
bool nextbuf(struct Eli_ *e, int key);
bool prevbuf(struct Eli_ *e, int key);
bool begofbuf(struct Eli_ *e, int key);
bool endofbuf(struct Eli_ *e, int key);

#endif /* __ELI_ACTION_H__ */
