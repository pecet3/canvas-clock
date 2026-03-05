#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

typedef struct
{
    uint8_t *mem;
    size_t size;
    size_t offset;
} arena_t;

char *arena_strdup(arena_t *a, const char *s)
{
    size_t len = strlen(s) + 1;
    char *dst = arena_alloc(a, len);
    if (!dst)
    {
        return NULL;
    }
    memcpy(dst, s, len);
    return dst;
}