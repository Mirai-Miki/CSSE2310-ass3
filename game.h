/* game.h
 *
 * Author: Michael Bossner
 *
 * game.h header file for game.c
 */

#ifndef GAME_H
#define GAME_H

#include "lib.h"

////////////////////////////// Global Variables ///////////////////////////////

/* Container for all signal handling flags and information */
SigStore sigStore;

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * The game loop for the entire game. After the game starts game signal flags
 * are checked then game over is checked. Then the players turn will happen
 * if all succeeds the current player will be incremented and it will all start
 * again
 * 
 * state: Contains all information needed to keep track of the game
 *
 * Error 5: Bad Start. A player failed to start correctly
 *
 * Error 6: player closed before end of game message was sent
 *
 * Error 7: Protocol error by player
 *
 * Error 10: Received SIGINT
 */
void game_loop(GameState* state);

#endif