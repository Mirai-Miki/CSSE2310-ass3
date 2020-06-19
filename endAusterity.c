/* endAusterity.c 
 *
 * Author: Michael Bossner
 *
 * endAusterity.c contains functions related to ending austerity
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "endAusterity.h"
#include "deck.h"
#include "board.h"
#include "lib.h"
#include "game.h"
#include "comms.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define SLEEP 2

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Game end because a player reached the victory score or no more cards
 * can be purchased
 *
 * state: Contains all information needed to keep track of the game
 */
static void game_over(GameState* state);

/*
 * The wrong number of arguments have been provided to start the game
 * 5 arguments min are required
 *
 * 1. Number of non-wild tokens in each pile
 * 2. Number of points required to end the game
 * 3. Name of the deckFile to be used
 * 4. player
 * 5. player
 *
 * Maximum of 26 players are allowed
 * Prints a message explaining
 */
static void wrong_num_args(void);

/*
 * The arguments that have been provided are invalid
 * prints a message explaining
 */
static void invalid_args(void);

/*
 * The deck file provided cannot be accessed
 * prints a message explaining
 */
static void cannot_access_deck(void);

/*
 * the contents of the deck file file are invalid
 * prints a message explaining
 */
static void invalid_deck(void);

/*
 * Failed to start a player process
 * kills all player processes and prints a message explaining
 *
 * state: Contains all information needed to keep track of the game
 */
static void bad_start(GameState* state);

/*
 * A player closed communication with the hub
 * kills all player processes running and prints a message explaining
 *
 * state: Contains all information needed to keep track of the game
 */
static void client_disconnected(GameState* state);

/*
 * A player sent 2 invalid messages in a row
 * kills all player processes running and prints a message explaining
 *
 * state: Contains all information needed to keep track of the game
 */
static void protocol_error(GameState* state);

/*
 * SIGINT received
 * kills all player processes running and prints a message explaining
 *
 * state: Contains all information needed to keep track of the game
 */
static void sigint_caught(GameState* state);

/*
 * sends the end of game message to all players.
 * if players have not ended within 2 seconds SIGKILL will be sent
 *
 * state: Contains all information needed to keep track of the game
 */
static void kill_children(GameState* state);

/*
 * closes all communications with players
 *
 * state: Contains all information needed to keep track of the game
 */
static void close_pipes(GameState* state);

/*
 * prints the exit status of all children if the status is not 0
 *
 * state: Contains all information needed to keep track of the game
 */
static void print_status(GameState* state);

////////////////////////////////// Functions //////////////////////////////////

void end_austerity(GameState* state, int exitStatus) {
    switch (exitStatus) {
        case GAME_OVER:
            game_over(state);
            break;
        case WRONG_NUM_ARGS:
            wrong_num_args();
            break;
        case INVALID_ARG:
            invalid_args();
            break;
        case CANNOT_OPEN_DECK:
            cannot_access_deck();
            break;
        case INVALID_DECK:
            invalid_deck();
            break;
        case BAD_START:
            bad_start(state);
            break;
        case CLIENT_DISCONNECT:
            client_disconnected(state);
            break;
        case PROTOCOL_ERR:
            protocol_error(state);
            break;
        case SIGINT_CAUGHT:
            sigint_caught(state);
            break;
    }
    
    // free malloced memory
    free_board(state);  
    free_deck(state);   
    close_pipes(state);

    exit(exitStatus);
}

////////////////////////////// Private Functions //////////////////////////////
//
static void game_over(GameState* state) {
    char* winners = "Winner(s) ";
    kill_children(state);
    print_status(state);
    print_winners(state, winners, stdout);
    fflush(stderr);
}

//
static void wrong_num_args(void) {
    fprintf(stderr, "%s\n", "Usage: austerity tokens points deck player"
            " player [player ...]");
    fflush(stderr);
}

//
static void invalid_args(void) {
    fprintf(stderr, "%s\n", "Bad argument");
    fflush(stderr);
}

//
static void cannot_access_deck(void) {
    fprintf(stderr, "%s\n", "Cannot access deck file");
    fflush(stderr);
}

//
static void invalid_deck(void) {
    fprintf(stderr, "%s\n", "Invalid deck file contents");
    fflush(stderr);
}

//
static void bad_start(GameState* state) {
    kill_children(state);
    fprintf(stderr, "%s\n", "Bad start");
    fflush(stderr);
}

//
static void client_disconnected(GameState* state) {
    kill_children(state);       
    print_status(state);
    fprintf(stdout, "%s\n", "Game ended due to disconnect");
    fflush(stdout);
    fprintf(stderr, "%s\n", "Client disconnected");
    fflush(stderr);
}

//
static void protocol_error(GameState* state) {
    kill_children(state);
    print_status(state);
    fprintf(stderr, "%s\n", "Protocol error by client");
    fflush(stderr);
}

//
static void sigint_caught(GameState* state) {
    kill_children(state);
    fprintf(stderr, "%s\n", "SIGINT caught");
    fflush(stderr);
}

//
static void kill_children(GameState* state) {
    int sleepTime = SLEEP;
    for (int player = 0; player < state->player.count; player++) {
        fprintf(state->player.commsList[player][WRITE], "eog\n");
        fflush(state->player.commsList[player][WRITE]);
    }

    while (sleepTime > 0 && (sigStore.index != state->player.count)) {
        sleepTime = sleep(sleepTime);
    }

    for (int player = 0; player < state->player.count; player++) {
        for (int dead = 0; dead < sigStore.index; dead++) {
            if (state->player.pidList[player] == sigStore.children[dead]) {
                break;
            } else if (dead == (sigStore.index - 1)) {
                kill(state->player.pidList[player], SIGKILL);
            }
        }
    }
}

//
static void close_pipes(GameState* state) {
    for (int player = 0; player < state->player.count; player++) {      
        fclose(state->player.commsList[player][READ]);
        fclose(state->player.commsList[player][WRITE]);
    }
}

//
static void print_status(GameState* state) {
    for (int player = 0; player < state->player.count; player++) {
        for (int i = 0; i < state->player.count; i++) {
            if (sigStore.children[i] == state->player.pidList[player]) {
                if (sigStore.status[i] != 0) {
                    if (sigStore.childSignaled) {
                        fprintf(stderr, "Player %c shutdown after receiving " 
                                "signal %d\n", 
                                player_int_to_char(player),
                                sigStore.status[i]);
                        fflush(stderr);
                    } else {
                        fprintf(stderr, "Player %c ended with status %d\n", 
                                player_int_to_char(player),
                                sigStore.status[i]);
                        fflush(stderr);
                    }
                }
            }
        }
    }
}