/* endAusterity.h
 *
 * Author: Michael Bossner
 *
 * endAusterity.h header file for endAusterity.c
 */

#ifndef END_AUSTERITY_H
#define END_AUSTERITY_H

#include "lib.h"

/////////////////////////////////// Defines ///////////////////////////////////

/* All exit statuses the hub can take */
enum End {
    GAME_OVER = 0,
    WRONG_NUM_ARGS = 1,
    INVALID_ARG = 2,
    CANNOT_OPEN_DECK = 3,
    INVALID_DECK = 4,
    BAD_START = 5,
    CLIENT_DISCONNECT = 6,
    PROTOCOL_ERR = 7,
    SIGINT_CAUGHT = 10,
    BAD_CHILD = 11, // used for children that failed to start
};

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * Controls the ending of the Austerity program
 *
 * state: Contains all information needed to keep track of the game
 *
 * exitStatus: reason austerity is closing
 *
 * Error 1: Wrong number of arguments provided
 *
 * Error 2: Invalid command line arguments
 *
 * Error 3: Deck file cannot be read
 *
 * Error 4: Deck file is incorrect
 *
 * Error 5: Failure starting any of the player processes
 *
 * Error 6: player closed before end of game message was sent
 *
 * Error 7: Protocol error by player
 *
 * Error 10: Received SIGINT
 *
 * Error 11: Failed to exec
 */
void end_austerity(GameState* state, int exitStatus);

#endif