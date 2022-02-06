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
#include "dict.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define XOR_KEY     0x5a


static void xor_string(char *str, uint8_t key) {
    while(*str) {
        *str = (*str) ^ key;
        str++;
    }
}

void load_answer_list(hset_t *set) {
    
    for(unsigned i = 0; i < answers_size; ++i) {
        const char *answer = answers[i];
        char word[WORD_SIZE+1];
        strncpy(word, answer, WORD_SIZE);
        word[WORD_SIZE] = '\0';
        xor_string(word, XOR_KEY);
        
        size_t len = strlen(word);
        if(len && word[len-1] == '\n') {
            word[len-1] = '\0';
            len -= 1;
        }
        if(len != WORD_SIZE) continue;
        hset_insert(set, word);
    }
}

void load_word_list(hset_t *set) {
    assert(set);
    
    for(unsigned i = 0; i < words_size; ++i) {
        const char *word = words[i];
        hset_insert(set, word);
    }
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


void game_init(game_t *game) {
    assert(game);
    
    (void)safe_strdup; // Because clang.
    
    memset(game, 0, sizeof(*game));
    game->won = false;
    game->guess_count = 0;
    game->seq = get_wordle_seq();
    hset_init(&game->words);
    
    load_answer_list(&game->words);
    load_word_list(&game->words);
    
    strncpy(game->answer, answers[game->seq], WORD_SIZE);
    xor_string(game->answer, XOR_KEY);
    
    for(int i = 0; i < ALPHABET_SIZE; ++i) {
        game->alphabet[i] = GAME_LETTER_UNUSED;
    }
}

void game_fini(game_t *game) {
    assert(game);
    hset_fini(&game->words);
}

static letter_state_t mark_letter(letter_state_t existing, letter_state_t guess) {
    switch(existing) {
        case GAME_LETTER_UNUSED: return guess;
        case GAME_LETTER_NO: return GAME_LETTER_NO;
        case GAME_LETTER_MISPLACED: return guess;
        case GAME_LETTER_RIGHT: return GAME_LETTER_RIGHT;
    }
    return GAME_LETTER_NO;
}

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

