//===--------------------------------------------------------------------------------------------===
// printing.c - File out printing for wordle
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2022 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include "game.h"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <term/colors.h>

static void print_green(char c, FILE *out) {
    term_set_fg(out, TERM_GREEN);
    fputc(c, out);
    term_set_fg(out, TERM_DEFAULT);
}

static void print_yellow(char c, FILE *out) {
    term_set_fg(out, TERM_YELLOW);
    fputc(c, out);
    term_set_fg(out, TERM_DEFAULT);
}

// static void print_magenta(char c, FILE *out) {
//     fprintf(out, "\033[35m%c\033[0m", c);
// }

static void print_norm(char c, FILE *out) {
    term_set_fg(out, TERM_DEFAULT);
    fputc(c, out);
}

static void print_alphabet_line(const letter_state_t alphabet[ALPHABET_SIZE], unsigned line, FILE *out) {
    const unsigned letters_per_line = ceil((double)ALPHABET_SIZE/(double)MAX_GUESSES);
    
    unsigned start = letters_per_line * line;
    unsigned end = start + letters_per_line;
    if(end > ALPHABET_SIZE) end = ALPHABET_SIZE;

    term_set_bold(out, true);
    for(unsigned i = start; i < end; ++i) {
        char letter = i + 'A';
        switch(alphabet[i]) {
        case GAME_LETTER_UNUSED:
            print_norm(letter, out);
            break;
            
        case GAME_LETTER_NO:
            // print_norm(tolower(letter), out);
            print_norm(' ', out);
            break;
            
        case GAME_LETTER_MISPLACED:
            print_yellow(letter, out);
            break;
            
        case GAME_LETTER_RIGHT:
            print_green(letter, out);
            break;
        }
        
        if(i < end-1) {
            fprintf(out, " ");
        }
    }
    term_style_reset(out);
}

static void print_guess(const guess_t *guess, FILE *out) {
    term_set_bold(out, true);
    term_reverse(out);
    for(unsigned i = 0; i < WORD_SIZE; ++i) {
        char c = toupper(guess->word[i]);
        switch(guess->check[i]) {
        case GAME_LETTER_RIGHT:
            print_green(c, out);
            break;
            
        case GAME_LETTER_MISPLACED:
            print_yellow(c, out);
            break;
            
        case GAME_LETTER_UNUSED:
        case GAME_LETTER_NO:
            print_norm(c, out);
            break;
        }
    }
    term_style_reset(out);
}

static void print_emoji_guess(const guess_t *guess, FILE *out) {
    for(unsigned i = 0; i < WORD_SIZE; ++i) {
        switch(guess->check[i]) {
        case GAME_LETTER_RIGHT:
            fprintf(out, "🟩");
            break;
            
        case GAME_LETTER_MISPLACED:
            fprintf(out, "🟨");
            break;
            
        case GAME_LETTER_UNUSED:
        case GAME_LETTER_NO:
            fprintf(out, "⬜️");
            break;
        }
    }
    printf("\n");
}

static void print_empty(FILE *out) {
    for(int i = 0; i < WORD_SIZE; ++i) {
        fprintf(out, ".");
    }
}

static void print_emoji_empty(FILE *out) {
    for(int i = 0; i < WORD_SIZE; ++i) {
        fprintf(out, "⬜️");
    }
}

void print_board(const game_t *game, bool show_emoji, FILE *out) {
    fprintf(out, "\n----------\n");
    for(unsigned i = 0; i < MAX_GUESSES; ++i) {
        bool is_empty = i >= game->guess_count;
        const guess_t *guess = &game->guesses[i];
        
        if(show_emoji) {
            if(is_empty) {
                print_emoji_empty(out);
            } else {
                print_emoji_guess(guess, out);
            }
            fprintf(out, "\t");
        }
        
        if(is_empty) {
            print_empty(out);
        } else {
            print_guess(&game->guesses[i], out);
        }
        
        fprintf(out, "\t");
        print_alphabet_line(game->alphabet, i, out);
        fprintf(out, "\n");
    }
    fprintf(out, "\n");
}

void print_share_sheet(const game_t *game, FILE *out) {
    fprintf(out, "Wordle %u %u/%u\n\n", game->seq, game->guess_count, MAX_GUESSES);
    for(unsigned i = 0; i < game->guess_count; ++i) {
        print_emoji_guess(&game->guesses[i], out);
    }
}

