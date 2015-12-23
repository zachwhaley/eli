#ifndef __ELI_FILE_H__
#define __ELI_FILE_H__

#include "buffer.h"
#include "line.h"

#include <stdlib.h>

typedef struct {
    const char *name;
    Buffer buf;
    Line *pos;
    size_t row, col;
} File;

void file_read(File *file);
void file_write(File *file);

// File actions
typedef void (*action_func)(File *, int);

void file_begofline(File *file, int key);
void file_endofline(File *file, int key);
void file_nextchar(File *file, int key);
void file_prevchar(File *file, int key);
void file_nextline(File *file, int key);
void file_prevline(File *file, int key);
void file_newline(File *file, int key);
void file_backchar(File *file, int key);
void file_addchar(File *file, int key);

#endif /* __ELI_FILE_H__ */
