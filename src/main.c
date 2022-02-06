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
#include <unistd.h>
#include "game.h"

int main(int argc, const char **argv) {
    (void)argc;
    (void)argv;
    
    game_t game;
    game_init(&game, "allowed-guesses.txt", "answers.txt");

    char *word = NULL;
    size_t size = 0;
    bool done = false;

    while(!done) {

        printf("wordle:%u/%u> ", game.guess_count+1, MAX_GUESSES);
        if(getline(&word, &size, stdin) == -1) break;
        
        const guess_t *guess = NULL;
        result_t result = game_submit(&game, word, &guess);
        print_board(&game, false, stdout);
        
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
    
    game_stats(&game);
    
    print_share_sheet(&game, stdout);
    
    free(word);
    game_fini(&game);
    return 0;
}


