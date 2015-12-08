#include "file.h"

void begofline(File *file, int key)
{
    file->col = 0;
}

void endofline(File *file, int key)
{
    file->col = strlen(file->pos->str);
}

void nextchar(File *file, int key)
{
    if (file->col + 1 <= strlen(file->pos->str)) {
        file->col++;
    }
    else if (file->pos->next) {
        nextline(file, key);
        begofline(file, key);
    }
}

void prevchar(File *file, int key)
{
    if (file->col > 0) {
        file->col--;
    }
    else if (file->pos->prev) {
        prevline(file, key);
        endofline(file, key);
    }
}

void nextline(File *file, int key)
{
    Line *nline = file->pos->next;
    if (nline) {
        file->pos = nline;
        file->row++;
    }
    if (file->col > strlen(file->pos->str))
        endofline(file, key);
}

void prevline(File *file, int key)
{
    Line *pline = file->pos->prev;
    if (pline) {
        file->pos = pline;
        file->row--;
    }
    if (file->col > strlen(file->pos->str))
        endofline(file, key);
}

void newline(File *file, int key)
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
    nextline(file, key);
    begofline(file, key);
}

void backchar(File *file, int key)
{
    Line *l = file->pos;
    prevchar(file, key);
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

void addchar(File *file, int key)
{
    line_insert(file->pos, file->col, key);
    nextchar(file, key);
}
