//===--------------------------------------------------------------------------------------------===
// set.c - hash set implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "set.h"
#include "memory.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY (16)

uint32_t hash_str(const char *str) {
    assert(str);
    //Fowler-Noll-Vo 1a hash
    //http://create.stephan-brumme.com/fnv-hash/
    uint32_t hash = 0x811C9DC5;
    size_t length = strlen(str);
    for(size_t i = 0; i < length; ++i) {
        hash = (hash ^ str[i]) * 0x01000193;
    }
    return hash;
}

void hset_init(hset_t *hset) {
    assert(hset);
    hset->size = 0;
    hset->capacity = 0;
    hset->entries = NULL;
}

void hset_fini(hset_t *hset) {
    assert(hset);
    
    for(size_t i = 0; i < hset->capacity; ++i) {
        if(hset->entries[i]) safe_free(hset->entries[i]);
    }
    
    safe_free(hset->entries);
    hset_init(hset);
}

static bool do_insert(char **entries, size_t cap, char *entry) {
    size_t idx = hash_str(entry) % cap;
    size_t start_idx = idx;
    do {
        if(!entries[idx]) {
            entries[idx] = entry;
            return true;
        }
        if(!strcmp(entries[idx], entry)) return false;
        idx = (idx + 1) % cap;
    } while(idx != start_idx);
    return false;
}

static void grow(hset_t *hset) {
    size_t new_cap = hset->capacity ? hset->capacity * 2 : DEFAULT_CAPACITY;
    char **new_entries = safe_calloc(new_cap, sizeof(char *));
    
    for(size_t i = 0; i < hset->capacity; ++i) {
        if(hset->entries[i]) {
            do_insert(new_entries, new_cap, hset->entries[i]);
        }
    }
    
    safe_free(hset->entries);
    hset->entries = new_entries;
    hset->capacity = new_cap;
}

void hset_insert(hset_t *hset, const char *str) {
    assert(hset);
    assert(str);
    if(hset->size + 1 > hset->capacity * 0.7) {
        grow(hset);
    }
    char *copy = safe_strdup(str);
    if(do_insert(hset->entries, hset->capacity, copy)) {
        hset->size += 1;
    } else {
        safe_free(copy);
    }
}

bool hset_contains(const hset_t *hset, const char *str) {
    assert(hset);
    assert(str);
    
    uint32_t hash = hash_str(str);
    size_t idx = hash % hset->capacity;
    size_t start_idx = idx;
    
    while(hset->entries[idx]) {
        uint32_t t_hash = hash_str(hset->entries[idx]);
        if(t_hash == hash && !strcmp(str, hset->entries[idx])) {
            return true;
        }
        idx = (idx + 1) % hset->capacity;
        if(idx == start_idx) break;
    }
    return false;
}