/* austerity.c
 *
 * Author: Michael Bossner
 *
 * austerity.c is the main file for the Austerity Hub program
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>

#include "lib.h"
#include "token.h"
#include "deck.h"
#include "endAusterity.h"
#include "board.h"
#include "game.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define MIN_ARGS 6
#define MAX_ARGS 30
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define PIPE_FAIL -1
#define FLAGGED 1
#define BUFFER 10

/* Program argument indexes */
enum ArgIndex {
    TOKENS = 1,
    POINTS = 2,
    DECK_FILE = 3,
    PLAYER_START = 4,
};

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Initializes the state variables of the game.
 *
 * state: Contains all information needed to keep track of the game
 *
 */
static void init_state(GameState* state);

/*
 * Parses the tokens and points arguments and sets the values into the state
 *
 * tokens: Number of tokens in each non wild pile. Must be a positive number.
 *
 * points: Number of points needed to end the game. Must be a positive number.
 *
 * state: Contains all information needed to keep track of the game
 *
 * return: Returns 0 if tokens || points is not a positive number
 *         else returns 1.
 */
static int process_args(char* tokens, char* points, GameState* state);

/*
 * Sets up and starts all player processes for the game
 *
 * state: Contains all information needed to keep track of the game
 *
 * argv: number of arguments passed into austerity
 *
 * argc: all arguments passed into austerity including all player names
 *
 * Error 6: Bad start. Failed to set up a player process
 */
static void process_players(GameState* state, char** argv, int argc);

/*
 * Signal handler for SIGINT, SIGCHLD and SIGPIPE. Sets values to the 
 * global variable sigStore.
 *
 * sigNo: signal identifier
 */
static void handle_signals(int sigNo);

////////////////////////////////// Functions //////////////////////////////////

int main(int argc, char** argv) {
    GameState state;

    struct sigaction sigAct;
    sigAct.sa_handler = handle_signals;
    sigAct.sa_flags = SA_RESTART;

    // initialize the sigStore global variable
    sigStore.index = 0;
    sigStore.badStart = false;
    sigStore.sigIntCaught = false;
    sigStore.sigPipeCaught = false;
    sigStore.childSignaled = false;

    sigaction(SIGINT, &sigAct, NULL);
    sigaction(SIGCHLD, &sigAct, NULL);
    sigaction(SIGPIPE, &sigAct, NULL);
    
    if (!is_num_args_valid(argc, MIN_ARGS, MAX_ARGS)) {
        end_austerity(&state, WRONG_NUM_ARGS);
    }

    if (!process_args(argv[TOKENS], argv[POINTS], &state)) {
        end_austerity(&state, INVALID_ARG);
    }

    init_state(&state);

    load_deck_file(argv[DECK_FILE], &state);

    process_players(&state, argv, argc);    

    game_loop(&state);
    
    end_austerity(&state, GAME_OVER); // will never get to this line.
}

////////////////////////////// Private Functions //////////////////////////////
//
static void init_state(GameState* state) {
    init_board(state);
    init_tokens(state);

    state->currentPlayer = 0;

    for (int player = 0; player < MAX_PLAYERS; player++) {
        for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
            state->player.discountList[player][colour] = 0;
            state->player.tokens[player][colour] = 0;

        }
        state->player.scoreCard[player] = 0;
        state->player.wildPile[player] = 0;
    }
}

//
static int process_args(char* tokens, char* points, GameState* state) {
    int maxTokens = is_str_pos_number(tokens);
    int vicPoints = is_str_pos_number(points);
    if (maxTokens == INVALID || vicPoints == INVALID) {
        return FAIL;
    } else {
        state->tokenPile.maxTokens = maxTokens;     
        state->victoryPoints = vicPoints;
    }
    return VALID;
}

//
static void process_players(GameState* state, char** argv, int argc) {  
    const int playerCount = (argc - PLAYER_START);
    state->player.count = playerCount;

    for (int i = PLAYER_START; i < argc; i++) { // for each player
        const int currentPlayer = (i - PLAYER_START);
        int readWrite[READ_WRITE];
        int writeRead[READ_WRITE];
        if (pipe(readWrite) == PIPE_FAIL || (pipe(writeRead) == PIPE_FAIL)) {
            end_austerity(state, BAD_START);
        }   
        if ((state->player.pidList[currentPlayer] = fork()) == 0) { // Child
            int devNull = open("/dev/null", O_WRONLY);
            char tempCount[BUFFER];          
            sprintf(tempCount, "%d", playerCount); // setting up player args
            char tempPlayer[BUFFER];
            sprintf(tempPlayer, "%d", currentPlayer);
            dup2(readWrite[WRITE], STDOUT); // redirecting pipes
            dup2(writeRead[READ], STDIN);
            dup2(devNull, STDERR);
            close(devNull); // closing all unused files
            close(readWrite[READ]);
            close(readWrite[WRITE]);
            close(writeRead[READ]);
            close(writeRead[WRITE]);
            execlp(argv[i], argv[i], tempCount, tempPlayer, (char*) NULL);
            // exec failed. Gracefully exiting process
            free_board(state);
            free_deck(state);
            for (int i = 0; i < currentPlayer; i++) {// close all previous pipe
                fclose(state->player.commsList[i][READ]);
                fclose(state->player.commsList[i][WRITE]);
            }
            exit(BAD_CHILD);        
        } else if (state->player.pidList[currentPlayer] < 0) {  // Fork Failed
            end_austerity(state, BAD_START);
        } else {  // Parent
            close(readWrite[WRITE]);
            close(writeRead[READ]);
            state->player.commsList[currentPlayer][READ] = 
                    fdopen(readWrite[READ], "r");
            state->player.commsList[currentPlayer][WRITE] = 
                    fdopen(writeRead[WRITE], "w");
        }
    }   
    sleep(1); // Austerity was starting too quickly
}

//
static void handle_signals(int sigNo) {
    int status;
    switch (sigNo) {
        case SIGINT:
            sigStore.sigIntCaught = true;
            break;

        case SIGCHLD:
            sigStore.children[sigStore.index] = wait(&status);
            if (WIFEXITED(status)) {
                sigStore.status[sigStore.index] = WEXITSTATUS(status);
            } else {
                sigStore.status[sigStore.index] = WIFSIGNALED(status);
                sigStore.childSignaled = true;
            }
            if (sigStore.status[sigStore.index] == BAD_CHILD) {
                sigStore.badStart = true;
            }
            sigStore.index++;
            break;
        case SIGPIPE:
            sigStore.sigPipeCaught = true;
            break;
    }
}