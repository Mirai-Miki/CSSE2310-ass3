/* board.h
 *
 * Author: Michael Bossner
 *
 * board.h header file for board.c
 */

#ifndef BOARD_H
#define BOARD_H

#include "lib.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define MAX_MARKETS 8

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * Sets up the board to be empty
 *
 * state: Contains all information needed to keep track of the game
 */
void init_board(GameState* state);

/*
 * Checks if there are any markets set up on the board
 *
 * state: Contains all information needed to keep track of the game
 *
 * returns: returns false if the board has 0 markets set up else returns true
 */
bool is_board_empty(GameState* state);

/*
 * Frees all markets set up on the board from memory. Does not free the
 * cards contained in the market.
 *
 * state: Contains all information needed to keep track of the game
 */
void free_board(GameState* state);

/*
 * Removes a market from the board and frees it from memory. The card contained
 * in the market is returned for use.
 *
 *
 * state: Contains all information needed to keep track of the game
 *
 * boardIndex: Index of the market to be removed and card collected
 *
 * return: If there is a market set up at the boardIndex the card contained
 *         is returned else if there is no market set up a NULL pointer
 *         is returned.
 */
Card* purchase_card(GameState* state, int boardIndex);

/*
 * prints all cards currently on the board in the format of 
 * "Card C:Bonus/Score/TP,TB,TY,TR"
 * C:       card index
 * Bonus:   card colour
 * score:   card points
 * TP:      Purple cost
 * TB:      Brown cost
 * TY:      Yellow cost
 * TR:      Red cost
 *
 * state: Contains all information needed to keep track of the game
 *
 * stream: output to be printed to
 */
void print_board(GameState* state, FILE* stream);

/*
 * Sets up a market for the board if there is a card left in the deck to be 
 * sold && there is a free market spot. Adds the card to the new market.
 *
 * state: Contains all information needed to keep track of the game
 *
 * card: card to be sold in the new market
 *
 * return: Returns 0 if the there are no free market spots || there are no
 *         cards left in the deck to be sold else returns 1
 */
int add_to_board(GameState* state, Card* card);

/*
 * Returns a pointer to the card contained in the market without changing 
 * anything or removing anything from memory.
 *
 * state: Contains all information needed to keep track of the game
 *
 * boardIndex: Index of the card to be looked at
 *
 * return: Returns a pointer to the card contained in the market at the 
 *         boardIndex. If no market exists at that boardIndex a NULL pointer
 *         is returned.
 */
Card* check_market_card(GameState* state, int boardIndex);

#endif