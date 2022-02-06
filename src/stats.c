//===--------------------------------------------------------------------------------------------===
// stats.c - Stats loader/exporter and handler
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "game.h"
#include "memory.h"
#include <stdio.h>
#include <unistd.h>

typedef struct {
    unsigned        seq;
    unsigned        guess_count;
    bool            has_won;
} history_entry_t;

typedef struct {
    unsigned        count;
    unsigned        capacity;
    history_entry_t *entries;
} history_t;


typedef struct {
    unsigned wins;
    unsigned total;
    unsigned streak;
    unsigned long_streak;
    
    unsigned freq[MAX_GUESSES];
} stats_t;

static void add_history_entry(history_t *stats, const history_entry_t *entry, bool safe) {
    if(safe) {
        for(int i = 0; i < stats->count; ++i) {
            if(stats->entries[i].seq == entry->seq) {
                stats->entries[i] = *entry;
                return;
            }
        }
    }
    
    if(stats->count + 1 > stats->capacity) {
        stats->capacity = stats->capacity ? stats->capacity * 2 : 32;
    }
    stats->entries = safe_realloc(stats->entries, stats->capacity * 2);
    stats->entries[stats->count] = *entry;
    stats->count += 1;
}

int load_history(history_t *stats, const char *path) {
    (void)safe_strdup;
    stats->entries = NULL;
    stats->count = 0;
    stats->capacity = 0;
    
    FILE *in = fopen(path, "rb");
    if(!in) return 0;
    
    char *line = NULL;
    size_t cap = 0;
    
    while(getline(&line, &cap, in) != -1) {
        
        unsigned seq = 0;
        unsigned attempts = 0;
        
        if(sscanf(line, "%u: %u", &seq, &attempts) != 2) continue;
        
        bool has_won = attempts > 0 && attempts <= MAX_GUESSES;
        history_entry_t entry = {
            .seq = seq,
            .guess_count = has_won ? attempts : 0,
            .has_won = has_won
        };
        add_history_entry(stats, &entry, false);
    }
    
    safe_free(line);
    fclose(in);
    return stats->count;
}

bool save_history(const history_t *stats, const char *path) {
    FILE *out = fopen(path, "wb");
    if(!out) return false;
    
    for(unsigned i = 0; i < stats->count; ++i) {
        const history_entry_t *entry = &stats->entries[i];
        fprintf(out, "%u: %u\n", entry->seq, entry->has_won ? entry->guess_count : 0);
    }
    
    fclose(out);
    return true;
}

static const char* history_path() {
    const char* home = getenv("HOME");
    if(!home) return ".wordle_history";

    static char path[4096];
    snprintf(path, 4096, "%s/.wordle_history", home);
    return path;
}


static void compute_stats(const history_t *history, stats_t *stats) {
    stats->wins = 0;
    stats->total = history->count;
    stats->streak = 0;
    stats->long_streak = 0;
    
    for(int i = 0; i < MAX_GUESSES; ++i) {
        stats->freq[i] = 0;
    }
    
    for(int i = 0; i < history->count; ++i) {
        const history_entry_t *entry = &history->entries[i];
        
        if(entry->guess_count > MAX_GUESSES) continue;
        if(entry->has_won) {
            stats->wins += 1;
            stats->streak += 1;
            
            if(stats->streak > stats->long_streak) {
                stats->long_streak = stats->long_streak;
            }
        } else {
            stats->streak = 0;
        }
        stats->freq[entry->guess_count - 1] += 1;
    }
}


void game_stats(const game_t *game) {
    const char *path = history_path();
    history_t history = {};
    load_history(&history, path);
    
    history_entry_t new_entry = {
        .seq = game->seq,
        .has_won = game->won,
        .guess_count = game->guess_count
    };
    add_history_entry(&history, &new_entry, true);
    
    stats_t stats = {};
    compute_stats(&history, &stats);
    
    printf("------\n");
    printf("played:  %u\n", stats.total);
    printf("won:     %.0f%%\n", 100 * (double)stats.wins/(double)stats.total);
    printf("streak:  %u\n", stats.streak);
    printf("longest: %u\n", stats.long_streak);
    printf("------\n");
    
    save_history(&history, path);
}
