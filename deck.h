/* deck.h
 *
 * Author: Michael Bossner
 *
 * deck.h header file for deck.c
 */

#ifndef DECK_H
#define DECK_H

#include <stdio.h>

#include "lib.h"

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * loads all cards out of a file into memory for game play
 *
 * Minimum amount of cards in a deck file should be at least 1.
 * Format should be each card on it's own line with EOF on it's own line:
 *
 * "D:V:P,B,Y,R\n"
 * "D:V:P,B,Y,R\n"
 * "EOF"
 *
 * D: Colour of the card either 'B' || 'Y' || 'P' || 'R'
 * v: number of points the card gives. Must be a positive number
 * P: number of Purple tokens the card costs. Must be a positive number
 * B: number of Brown tokens the card costs. Must be a positive number
 * Y: number of Yellow tokens the card costs. Must be a positive number
 * R: number of Red tokens the card costs. Must be a positive number
 *
 * deckFile: Name of the file that contains the deck
 *
 * state: Contains all information needed to keep track of the game
 *
 * Error 3: Cannot access deck file.
 *
 * Error 4: Invalid deck file contents. deckFile does not meet the format
 */
void load_deck_file(char* deckFileName, GameState* state);

/*
 * frees the entire deck pile from memory
 *
 * state: Contains all information needed to keep track of the game
 */
void free_deck(GameState* state);

#endif