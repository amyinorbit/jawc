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
#include <term/arg.h>
#include <term/printing.h>
#include "game.h"

#define COUNTOF(arr) (sizeof(arr) / sizeof(arr[0]))

static game_t game;
static const term_param_t params[] = {
    {'w', 0, "wordle", TERM_ARG_VALUE, "play a specific past problem"},
    {'s', 0, "no-stats", TERM_ARG_OPTION, "do not save results to the stats file"},
};

static const char *uses[] = {
    "[--no-stats]",
    "--wordle WORDLE_NUMBER"
};

#define WEBSITE "https://github.com/amyinorbit/jawc"
#define EMAIL "amy@amyparent.com"

static void print_prompt(const char *name) {
    printf("%s %u/%u> ", name, game.guess_count+1, MAX_GUESSES);
}

int main(int argc, const char **argv) {
    term_arg_parser_t args;
    term_arg_parser_init(&args, argc, argv);
    
    int wordle = -1;
    bool do_stats = true;
    
    term_arg_result_t r = term_arg_parse(&args, params, COUNTOF(params));
    while(r.name != TERM_ARG_DONE) {
        switch(r.name) {
        case TERM_ARG_HELP:
            term_print_usage(stdout, "jawc", uses, 2);
            term_print_help(stdout, params, COUNTOF(params));
            return 0;
            
        case TERM_ARG_ERROR:
            term_error("jawc", 1, "%s", args.error);
            break;
            
        case TERM_ARG_VERSION:
            printf("jawc version 1.0r1 (" __DATE__ ")\n");
            printf("Built by Amy Parent. Wordle (c) 2021-2022 Jason Wardle\n");
            return 0;
        
        case 'w':
            do_stats = false;
            wordle = atoi(r.value);
            break;
        case 's':
            do_stats = false;
            break;
        }
        r = term_arg_parse(&args, params, COUNTOF(params));
    }
    
    game_init(&game, wordle);
    
    line_t *editor = line_new(&(line_functions_t){.print_prompt = print_prompt});
    line_set_prompt(editor, "wordle");
    
    printf("Playing Wordle #%u\n\n", game.seq);
    
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
    
    if(do_stats) game_stats(&game);
    print_share_sheet(&game, stdout);
    
    game_fini(&game);
    return 0;
}


