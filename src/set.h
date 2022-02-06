//===--------------------------------------------------------------------------------------------===
// set.h - simple linear-probed hash set
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef SET_H
#define SET_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t size;
    size_t capacity;
    char **entries;
} hset_t;

uint32_t hash_str(const char *str);

void hset_init(hset_t *hset);
void hset_fini(hset_t *hset);

void hset_insert(hset_t *hset, const char *str);
bool hset_contains(const hset_t *hset, const char *str);

#endif /* end of include guard: SET_H */

