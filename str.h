#ifndef __ELI_STR_H__
#define __ELI_STR_H__

#include <ctype.h>
#include <string.h>
#include <stdbool.h>

char* strtrim(char *str)
{
    // Trim leading space
    while (isspace(*str))
        str++;
    // Return if all spaces
    if (*str == '\0')
        return str;
    // Trim trailing space
    char *end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;
    // Write NULL terminator
    *(end + 1) = '\0';
    return str;
}

bool streq(const char *l, const char *r)
{
    return strcmp(l, r) == 0;
}

#endif /* __ELI_STR_H__ */
