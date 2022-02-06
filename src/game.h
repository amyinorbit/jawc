//===--------------------------------------------------------------------------------------------===
// game.h - JAWC - just another wordle clone
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#ifndef JAWC_GAME_H
#define JAWC_GAME_H

#include "set.h"
#include <stdio.h>

#define MAX_GUESSES     (6)
#define WORD_SIZE       (5)
#define ALPHABET_SIZE   (26)

typedef enum {
    GAME_LETTER_UNUSED,
    GAME_LETTER_NO,
    GAME_LETTER_MISPLACED,
    GAME_LETTER_RIGHT,
} letter_state_t;

typedef enum {
    GAME_RESULT_ALREADY_GUESSED,
    GAME_RESULT_NOT_A_WORD,         // Janet is that you?
    GAME_RESULT_WON,
    GAME_RESULT_LOST,
    GAME_RESULT_AGAIN
} result_t;

typedef struct {
    char            word[WORD_SIZE+1];
    letter_state_t  check[WORD_SIZE];
} guess_t;

typedef struct {
    hset_t          words;
    char            **answers;
    
    bool            won;
    unsigned        seq;
    unsigned        guess_count;
    char            answer[WORD_SIZE+1];
    guess_t         guesses[MAX_GUESSES];
    letter_state_t  alphabet[ALPHABET_SIZE];
    // result_t    last_result;
} game_t;

void game_init(game_t *game, const char *words, const char *answers);
void game_fini(game_t *game);

result_t game_submit(game_t *game, const char *guess, const guess_t **out);

void print_board(const game_t *game, bool show_emoji, FILE *out);
void print_share_sheet(const game_t *game, FILE *out);

void game_stats(const game_t *game);
// unsigned game_get_guess_count(const game_t *game);
// const guess_t *game_get_guess(const game_t *game, unsigned idx);

#endif /* end of include guard: JAWC_GAME_H */
