/*===--------------------------------------------------------------------------------------------===
 * main.c - JAWC entry point implementation
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * NOTICE:  All information contained herein is, and remains the property
 * of Amy Alex Parent. The intellectual and technical concepts contained
 * herein are proprietary to Amy Alex Parent and may be covered by U.S. and
 * Foreign Patents, patents in process, and are protected by trade secret
 * or copyright law. Dissemination of this information or reproduction of
 * this material is strictly forbidden unless prior written permission is
 * obtained from Amy Alex Parent.
 *===--------------------------------------------------------------------------------------------===
*/
#include <stdio.h>
#include <stdlib.h>
#include <term/line.h>
#include <term/colors.h>
#include "game.h"

game_t game;

void print_prompt(const char *name) {
    printf("%s %u/%u> ", name, game.guess_count+1, MAX_GUESSES);
}

int main(int argc, const char **argv) {
    (void)argc;
    (void)argv;
    
    game_init(&game);
    
    line_t *editor = line_new(&(line_functions_t){.print_prompt = print_prompt});
    line_set_prompt(editor, "wordle");
    bool done = false;
    while(!done) {
        
        char *word = line_get(editor);
        if(!word) return 1;
        
        const guess_t *guess = NULL;
        result_t result = game_submit(&game, word, &guess);
        print_board(&game, false, stdout);
        free(word);
        
        switch(result) {
        case GAME_RESULT_ALREADY_GUESSED:
            printf("already guessed!\n\n");
            break;
        case GAME_RESULT_NOT_A_WORD:
            printf("not a word!\n\n");
            break;
        case GAME_RESULT_WON:
            printf("Well done!\n\n");
            done = true;
            break;
        case GAME_RESULT_LOST:
            printf("You lose: %s\n\n", game.answer);
            done = true;
            break;
        case GAME_RESULT_AGAIN:
            printf("Not quite!\n\n");
            break;
        }
    }
    line_destroy(editor);
    
    game_stats(&game);
    print_share_sheet(&game, stdout);
    
    game_fini(&game);
    return 0;
}


