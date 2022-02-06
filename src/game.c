//===--------------------------------------------------------------------------------------------===
// game.c - JAWC game implementation
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "game.h"
#include "memory.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define XOR_KEY     0x5a


static void xor_string(char *str, uint8_t key) {
    while(*str) {
        *str = (*str) ^ key;
        str++;
    }
}

char **load_answer_list(const char *path, hset_t *set) {
    assert(path);
    
    FILE *in = fopen(path, "rb");
    if(!in) return NULL;
    
    char **list = NULL;
    size_t cap = 0;
    size_t size = 0;
    
    char *word = NULL;
    size_t word_cap = 0;
    
    while(getline(&word, &word_cap, in) != -1) {
        size_t len = strlen(word);
        if(len && word[len-1] == '\n') {
            word[len-1] = '\0';
            len -= 1;
        }
        if(len != WORD_SIZE) continue;
        
        if(size + 2 > cap) {
            cap = cap ? cap * 2 : 128;
            list = safe_realloc(list, cap * sizeof(char *));
        }
        xor_string(word, XOR_KEY);
        hset_insert(set, word);
        list[size] = safe_strdup(word);
        size += 1;
    }
    safe_free(word);
    
    fclose(in);
    
    if(list) {
        list[size] = NULL;
    }
    return list;
}

bool load_word_list(const char *path, hset_t *set) {
    assert(path);
    assert(set);
    
    FILE *in = fopen(path, "rb");
    if(!in) return false;
    
    char *word = NULL;
    size_t word_cap = 0;
    
    while(getline(&word, &word_cap, in) != -1) {
        size_t len = strlen(word);
        if(len && word[len-1] == '\n') {
            word[len-1] = '\0';
            len -= 1;
        }
        if(len != WORD_SIZE) continue;
        hset_insert(set, word);
    }
    safe_free(word);
    
    return true;
}

unsigned get_wordle_seq() {
    static const uint64_t day_seconds = (24 * 60 * 60);
    
    struct tm epoch_tm = {
        .tm_year = (2021 - 1900),
        .tm_mon = 5,
        .tm_mday = 19,
        .tm_hour = 0,
        .tm_min = 0,
        .tm_sec = 1,
    };
    time_t epoch = mktime(&epoch_tm);
    time_t now = time(NULL);
    
    return (now - epoch) / day_seconds;
}


void game_init(game_t *game, const char *word_list, const char *answers_list) {
    assert(game);
    memset(game, 0, sizeof(*game));
    game->won = false;
    game->guess_count = 0;
    game->seq = get_wordle_seq();
    hset_init(&game->words);
    game->answers = load_answer_list(answers_list, &game->words);
    load_word_list(word_list, &game->words);
    strncpy(game->answer, game->answers[game->seq], WORD_SIZE);
    
    for(int i = 0; i < ALPHABET_SIZE; ++i) {
        game->alphabet[i] = GAME_LETTER_UNUSED;
    }
}

void game_fini(game_t *game) {
    assert(game);
    hset_fini(&game->words);
    if(game->answers) {
        for(size_t i = 0; game->answers[i]; ++i) {
            free(game->answers[i]);
        }
        free(game->answers);
    }
}

static letter_state_t mark_letter(letter_state_t existing, letter_state_t guess) {
    switch(existing) {
        case GAME_LETTER_UNUSED: return guess;
        case GAME_LETTER_NO: return GAME_LETTER_NO;
        case GAME_LETTER_MISPLACED: return guess;
        case GAME_LETTER_RIGHT: return GAME_LETTER_RIGHT;
    }
    return GAME_LETTER_NO;
};

static bool check(guess_t *guess, const char *word, letter_state_t state[ALPHABET_SIZE]) {
    unsigned noice_count = 0;
    
    char answer[WORD_SIZE+1];
    strncpy(answer, word, WORD_SIZE);
    answer[WORD_SIZE] = '\0';
    
    for(unsigned i = 0; i < WORD_SIZE; ++i) {
        guess->check[i] = GAME_LETTER_NO;
    }
    
    for(unsigned i = 0; i < WORD_SIZE; ++i) {
        if(guess->word[i] == answer[i]) {
            guess->check[i] = GAME_LETTER_RIGHT;
            answer[i] = '_';
            noice_count += 1;
        }
    }
    if(noice_count == WORD_SIZE) return true;
    
    for(unsigned i = 0; i < WORD_SIZE; ++i) {
        if(guess->check[i] == GAME_LETTER_RIGHT) continue;
        char *pos = strchr(answer, guess->word[i]);
        if(pos != NULL) {
            *pos = '_';
            guess->check[i] = GAME_LETTER_MISPLACED;
        } else {
            guess->check[i] = GAME_LETTER_NO;
        }
    }
    
    for(unsigned i = 0; i < WORD_SIZE; ++i) {
        unsigned idx = guess->word[i]-'a';
        state[idx] = mark_letter(state[idx], guess->check[i]);
    }
    
    return noice_count == WORD_SIZE;
}

static bool check_already_guessed(const game_t *game, const char *word) {
    for(unsigned i = 0; i < game->guess_count; ++i) {
        if(!strcmp(game->guesses[i].word, word)) return true;
    }
    return false;
}

static void trim_whitespace(char *word) {
    while(*word) {
        if(*word == '\n' || *word == '\t' || *word == ' ' || *word == '\r') {
            *word = '\0';
            return;
        }
        word++;
    }
}

result_t game_submit(game_t *game, const char *word, const guess_t **out) {
    assert(game);
    assert(word);
    assert(out);
    
    if(game->guess_count >= MAX_GUESSES) return GAME_RESULT_LOST;
    
    guess_t *guess = &game->guesses[game->guess_count];
    strncpy(guess->word, word, WORD_SIZE);
    trim_whitespace(guess->word);
    for(int i = 0; i < WORD_SIZE; ++i) {
        guess->word[i] = tolower(guess->word[i]);
    }
    
    if(strlen(guess->word) != WORD_SIZE) return GAME_RESULT_NOT_A_WORD;
    if(check_already_guessed(game, guess->word)) return GAME_RESULT_ALREADY_GUESSED;
    if(!hset_contains(&game->words, guess->word)) return GAME_RESULT_NOT_A_WORD;

    game->guess_count += 1;
    *out = guess;
    
    if(check(guess, game->answer, game->alphabet)) {
        game->won = true;
        return GAME_RESULT_WON;
    }
    return game->guess_count < MAX_GUESSES ? GAME_RESULT_AGAIN : GAME_RESULT_LOST;
}


unsigned game_get_guess_count(const game_t *game) {
    assert(game);
    return game->guess_count;
}

const guess_t *game_get_guess(const game_t *game, unsigned idx) {
    assert(game);
    assert(idx < game->guess_count);
    return &game->guesses[idx];
}

