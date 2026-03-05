#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct
{
    uint8_t *mem;
    size_t size;
    size_t offset;
} arena_t;