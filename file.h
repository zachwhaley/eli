#ifndef __ELI_FILE_H__
#define __ELI_FILE_H__

#include "buffer.h"
#include "line.h"

#include <string.h>

typedef struct {
    const char *name;
    Buffer buf;
    Line *pos;
    size_t row, col;
} File;

typedef void (*action)(File *, int);

void begofline(File *file, int key);
void endofline(File *file, int key);
void nextchar(File *file, int key);
void prevchar(File *file, int key);
void nextline(File *file, int key);
void prevline(File *file, int key);
void newline(File *file, int key);
void backchar(File *file, int key);
void addchar(File *file, int key);

#endif /* __ELI_FILE_H__ */
