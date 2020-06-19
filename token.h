/* token.h
 *
 * Author: Michael Bossner
 *
 * token.h header file for token.c
 */

#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

#include "lib.h"

///////////////////////// Public Functions Prototypes /////////////////////////

/*
 * Initializes the tokenPile
 *
 * state: Contains all information needed to keep track of the game
 */
void init_tokens(GameState* state);

/*
 * adds a discount to the player in the game state
 *
 * state: Contains all information needed to keep track of the game
 *
 * discount: discount to be added must be either 'P' || 'B' || 'Y' || 'R'
 *           else nothing will happen
 *
 * player: player index to add the discount to
 */
void add_discount(GameState* state, char discount, int player);

/*
 * checks that a purchase action is legal in the current game state
 * purchase cannot use tokens the player does not have
 * purchase cannot buy a card they cannot afford
 *
 * state: Contains all information needed to keep track of the game
 *
 * tokens: the tokens the player is trying to use to buy the card
 *
 * card: the card the player is trying to purchase
 *
 * return: Returns false if the sanity check fails
 *         Returns true if the purchase is legal
 */
bool purchase_sanity_check(GameState* state, long* tokens, Card* card);

/*
 * checks that a take action is legal in the current state of the game
 * take must be for exactly 3 tokens
 * take cannot take from piles of tokens that have 0 tokens
 * take can only ask for 1 of each colour
 *
 * state: Contains all information needed to keep track of the game
 *
 * tokens: tokens the player is asking to take
 *
 * return: Returns false if the take action fails the sanity checks
 *         Returns true if the take action is legal
 */
bool take_sanity_check(GameState* state, long* tokens);

#endif