//===--------------------------------------------------------------------------------------------===
// memory.h - Memory functions
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

static inline void *safe_malloc(size_t size) {
    void *mem = malloc(size);
    assert(mem);
    return mem;
}

static inline void *safe_calloc(size_t count, size_t size) {
    void *mem = calloc(count, size);
    assert(mem);
    return mem;
}

static inline void *safe_realloc(void *mem, size_t size) {
    mem = realloc(mem, size);
    assert(!size || mem);
    return mem;
}

static inline void safe_free(void *mem) {
    if(mem) {
        free(mem);
    }
}

static char *safe_strdup(const char *str) {
    size_t len = strlen(str);
    char *new_str = safe_calloc(len+1, sizeof(char));
    strcpy(new_str, str);
    new_str[len] = '\0';
    return new_str;
}

#endif /* end of include guard: MEMORY_H */
