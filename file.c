#include "file.h"

#include <string.h>

static void readfile(const char *fname, Buffer *buf);
static void writefile(const char *fname, const Buffer *buf);

void file_begofline(File *file, int key)
{
    file->col = 0;
}

void file_endofline(File *file, int key)
{
    file->col = strlen(file->pos->str);
}

void file_nextchar(File *file, int key)
{
    if (file->col + 1 <= strlen(file->pos->str)) {
        file->col++;
    }
    else if (file->pos->next) {
        file_nextline(file, key);
        file_begofline(file, key);
    }
}

void file_prevchar(File *file, int key)
{
    if (file->col > 0) {
        file->col--;
    }
    else if (file->pos->prev) {
        file_prevline(file, key);
        file_endofline(file, key);
    }
}

void file_nextline(File *file, int key)
{
    Line *nline = file->pos->next;
    if (nline) {
        file->pos = nline;
        file->row++;
    }
    if (file->col > strlen(file->pos->str))
        file_endofline(file, key);
}

void file_prevline(File *file, int key)
{
    Line *pline = file->pos->prev;
    if (pline) {
        file->pos = pline;
        file->row--;
    }
    if (file->col > strlen(file->pos->str))
        file_endofline(file, key);
}

void file_newline(File *file, int key)
{
    char *split = file->pos->str + file->col;
    size_t len = strlen(split);
    Line *l = line_new(split, len);
    memset(split, '\0', len);
    if (file->pos->next) {
        buf_insert(&file->buf, file->pos->next, l);
    }
    else {
        buf_pushback(&file->buf, l);
    }
    file_nextline(file, key);
    file_begofline(file, key);
}

void file_backchar(File *file, int key)
{
    Line *l = file->pos;
    file_prevchar(file, key);
    // If we moved to the previous line, we need to bring what was left of the line below to
    // our current line
    if (file->pos == l->prev) {
        line_pushback(file->pos, l->str, strlen(l->str));
        buf_erase(&file->buf, l);
    }
    else {
        line_erase(file->pos, file->col);
    }
}

void file_delchar(File *file, int key)
{
    line_erase(file->pos, file->col);
}

void file_addchar(File *file, int key)
{
    line_insert(file->pos, file->col, key);
    file_nextchar(file, key);
}

void file_read(File *file)
{
    readfile(file->name, &file->buf);
}

void file_write(File *file)
{
    writefile(file->name, &file->buf);
}

// Static Functions
static void readfile(const char *fname, Buffer *buf)
{
    FILE *fp = fopen(fname, "r");
    if (fp) {
        char in[BUFSIZ];
        while (fgets(in, BUFSIZ, fp) != NULL) {
            Line *l = line_new(in, strlen(in) - 1);
            buf_pushback(buf, l);
        }
        fclose(fp);
    }
}

static void writefile(const char *fname, const Buffer *buf)
{
    FILE *fp = fopen(fname, "w");
    if (fp) {
        for (Line *l = buf->beg; l != NULL; l = l->next) {
            fprintf(fp, "%s\n", l->str);
        }
        fclose(fp);
    }
}

