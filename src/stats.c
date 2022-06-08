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

#define JSMN_STRICT
#define JSMN_NEXT_SIBLING
#include "jsmn.h"

typedef struct {
    unsigned won;
    unsigned played;
    unsigned cur_streak;
    unsigned max_streak;
    
    unsigned last_won;
    
    unsigned guesses[MAX_GUESSES];
} stats_t;

static void write_json_i(FILE *out, const char *key, int num) {
    fprintf(out, "\"%s\": %d", key, num);
}

static void write_json_next(FILE *out) {
    fprintf(out, ",");
}

static void write_json_vi(FILE *out, const char *key, const int *num, unsigned count) {
    fprintf(out, "\"%s\": [", key);
    for(unsigned i = 0; i < count; ++i) {
        fprintf(out, "%d", num[i]);
        if(i != count-1) {
            write_json_next(out);
        }
    }
    fprintf(out, "]");
}

typedef struct {
    const char      *text;
    const jsmntok_t   *tokens;
    unsigned        count;
} json_t;

static bool jsoneq(const char *json, const jsmntok_t *tok, const char *s, unsigned len) {
  if ((tok->type & JSMN_STRING) && len == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return true;
  }
  return false;
}

static const jsmntok_t *json_get(const json_t *json, const char *path) {
    const char *next_path = NULL;
    size_t len = strlen(path);
    const char *slash = strchr(path, '/');
    if(slash) {
        next_path = slash + 1;
        len -= strlen(slash);
    }
    
    for(unsigned i = 0; i < json->count;) {
        const jsmntok_t *key = &json->tokens[i];
        const jsmntok_t *val = &json->tokens[i+1];
        unsigned obj_start = i + 2;
        if(val->type & JSMN_CONTAINER) {
            i = val->next_sibling;
        } else {
            i += 2;
        }
        if(!jsoneq(json->text, key, path, (int)len)) continue;
        
        if(!next_path) return val;
        if(!(val->type & JSMN_OBJECT)) return NULL;
        
        json_t nested = {
            .text = json->text,
            .tokens = json->tokens + obj_start,
            .count = (val->next_sibling - obj_start)
        };
        return json_get(&nested, next_path);
    }
    return NULL;
}

static bool check_int(const char *text, const jsmntok_t *tok, int *out) {
    if(!(tok->type & JSMN_PRIMITIVE)) return false;
    const char *val = text + tok->start;
    if(val[0] != '-' && (val[0] < '0' || val[0] > '9')) return false;
    *out = atoi(val);
    return true;
}

static bool json_get_i(const json_t *json, const char *path, int *out) {
    const jsmntok_t *tok = json_get(json, path);
    if(!tok) {
        fprintf(stderr, "missing value at json path '%s'\n", path);
        return false;
    }
    if(check_int(json->text, tok, out)) return true;
    fprintf(stderr, "invalid value at json path '%s'\n", path);
    return false;
}

static int json_get_vi(const json_t *json, const char *path, int *out, unsigned cap) {
    const jsmntok_t *tok = json_get(json, path);
    if(!tok) {
        fprintf(stderr, "missing array at json path '%s'\n", path);
        return -1;
    }
    if(!(tok->type & JSMN_ARRAY)) {
        fprintf(stderr, "invalid array at json path '%s'\n", path);
        return -1;
    }
        
    unsigned written = 0;
    for(unsigned i = 0; i < tok->size && written < cap; ++i) {
        const jsmntok_t *val = tok + (1 + i);
        if(!check_int(json->text, val, out + written)) {
            fprintf(stderr, "invalid interger array value at json path '%s'\n", path);
            return -1;
        }
        written += 1;
    }
    return written;
}

bool load_stats(stats_t *stats, const char *path) {
    FILE *in = fopen(path, "rb");
    if(!in) return false;
    fseek(in, 0, SEEK_END);
    size_t json_len = ftell(in);
    fseek(in, 0, SEEK_SET);
    
    char *json = safe_calloc(json_len+1, 1);
    fread(json, json_len, 1, in);
    json[json_len] = '\0';
    fclose(in);
    
    int count = 0;
    int cap = 0;
    jsmntok_t *tok = NULL;
    jsmn_parser parser;
    jsmn_init(&parser);
    
    do {
        cap = cap ? cap * 2 : 50;
        tok = safe_calloc(cap, sizeof(*tok));
        count = jsmn_parse(&parser, json, json_len, tok, cap);
    } while(count == JSMN_ERROR_NOMEM);
    
    if(count <= 0) {
        if(tok) free(tok);
        if(json) free(json);
        return false;
    }
    
    if(!(tok[0].type & JSMN_OBJECT)) goto errout;
    
    json_t dict = {
        .text = json,
        .tokens = tok + 1,
        .count = count - 1,
    };
    
    if(!json_get_i(&dict, "won", (int *)&stats->won)) goto errout;
    if(!json_get_i(&dict, "played",(int *) &stats->played)) goto errout;
    if(!json_get_i(&dict, "cur_streak", (int *)&stats->cur_streak)) goto errout;
    if(!json_get_i(&dict, "max_streak", (int *)&stats->max_streak)) goto errout;
    if(!json_get_i(&dict, "last_won", (int *)&stats->last_won)) goto errout;
    if(json_get_vi(&dict, "guesses", (int *)&stats->guesses, MAX_GUESSES) != MAX_GUESSES) goto errout;
    free(tok);
    free(json);
    return true;
    
errout:
    free(tok);
    free(json);
    return false;
}

bool save_stats(const stats_t *stats, const char *path) {
    FILE *out = fopen(path, "wb");
    if(!out) return false;
    
    fprintf(out, "{");
    write_json_i(out, "won", stats->won);                           write_json_next(out);
    write_json_i(out, "played", stats->played);                     write_json_next(out);
    write_json_i(out, "cur_streak", stats->cur_streak);             write_json_next(out);
    write_json_i(out, "max_streak", stats->max_streak);             write_json_next(out);
    write_json_i(out, "last_won", stats->last_won);                 write_json_next(out);
    write_json_vi(out, "guesses", (const int *)stats->guesses, MAX_GUESSES);
    fprintf(out, "}\n");
    
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





// static void compute_stats(const history_t *history, stats_t *stats) {
//     stats->wins = 0;
//     stats->total = history->count;
//     stats->streak = 0;
//     stats->long_streak = 0;
//
//     for(int i = 0; i < MAX_GUESSES; ++i) {
//         stats->freq[i] = 0;
//     }
//
//     for(unsigned i = 0; i < history->count; ++i) {
//         const history_entry_t *entry = &history->entries[i];
//
//         if(entry->guess_count > MAX_GUESSES) continue;
//         if(entry->has_won) {
//             stats->wins += 1;
//             stats->streak += 1;
//
//             if(stats->streak > stats->long_streak) {
//                 stats->long_streak = stats->streak;
//             }
//         } else {
//             stats->streak = 0;
//         }
//         stats->freq[entry->guess_count - 1] += 1;
//     }
// }
static void add_game_stats(stats_t *stats, const game_t *game) {
    stats->played += 1;
    if(game->won) {
        stats->won += 1;
        stats->guesses[game->guess_count-1] += 1;
        
        stats->cur_streak = game->seq == stats->last_won + 1 ? stats->cur_streak + 1 : 1;
        stats->last_won = game->seq;
        
        if(stats->cur_streak > stats->max_streak) {
            stats->max_streak = stats->cur_streak;
        }
    } else {
        stats->cur_streak = 0;
    }
}


void game_stats(const game_t *game) {
    (void)safe_strdup;
    const char *path = history_path();
    stats_t stats = {.won=0};
    load_stats(&stats, path);
    
    add_game_stats(&stats, game);
    // history_entry_t new_entry = {
    //     .seq = game->seq,
    //     .has_won = game->won,
    //     .guess_count = game->guess_count
    // };
    // add_history_entry(&history, &new_entry, true);
    
    // stats_t stats = {.wins=0};
    // compute_stats(&history, &stats);
    
    printf("------\n");
    printf("played:  %u\n", stats.played);
    printf("won:     %.0f%%\n", 100 * (double)stats.won/(double)stats.played);
    printf("streak:  %u\n", stats.cur_streak);
    printf("longest: %u\n", stats.max_streak);
    printf("------\n");
    
    save_stats(&stats, path);
}
