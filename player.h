/* player.h
 *
 * Author: Michael Bossner
 *
 * player.h header file for player.c
 */

#ifndef PLAYER_H
#define PLAYER_H

#include "lib.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define THIS_PLAYER state->currentPlayer

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * Checks the arguments for validity
 * there must be 2 arguments passed to the player and they must be both
 * a positive integer. Arugment 1 should not be less then 2 or greater than 26
 * argument 2 must not be less then 0 or greater than 25 and must not greater
 * than or equal to argument 1
 *
 * state: Contains all information needed to keep track of the game
 *
 * argc: number of arguments
 *
 * argv: list of arguments passed
 *
 * Error 1: Wrong number of arguments. Must be 2 arguments passed in
 *
 * Error 2: Invalid player count. (argument1 > 2 || < 26)
 *
 * Error 3: Invalid ID. (argument2 (> 0 || < 25) && <= argument1)
 */
void is_args_valid(GameState* state, int argc, char** argv);

/*
 * Initializes the players game state
 *
 * state: Contains all information needed to keep track of the game
 *
 * argc: number of arguments
 *
 * argv: list of arguments passed
 */
void init_player(GameState* state, int argc, char** argv);

/*
 * The players game loop. Waits for a messages then parses it. if it is a
 * valid message the appropriate action is then taken and the loop repeats
 *
 * state: Contains all information needed to keep track of the game
 *
 * doWhat: the function that will respond to the dowhat message
 *
 * Error 6: Communication Error. pipe closed before the end of the game or an
 *          Invalid message was received.
 */
void player_loop(GameState* state, void (*doWhat)(GameState* state));

/*
 * checks if there are any card on the board that the player can purchase
 *
 * state: Contains all information needed to keep track of the game
 *
 * cardIndexs: list of card indexes to be provided that can be purchased
 *
 * pointMin: specifies a minimum amount of points a card must have to be added
 *
 * player: player to be checked what cards they can purchase
 *
 * return: Returns the amount of cards that can be purchased
 */
int can_buy_card(GameState* state, int* cardIndexs, int pointMin, int player);

/*
 * sends the wild message to standard out
 */
void take_wild(void);

/*
 * checks if the player calling the function can take tokens
 *
 * state: Contains all information needed to keep track of the game
 *
 * return: returns true if they can take tokens or false if they cannot
 */
bool can_take_tokens(GameState* state);

/*
 * prints the take message to standard out
 *
 * tokens: the amount of tokens to be taken by the player
 */
void take(long* tokens);

/*
 * prints the purchase message to standard out
 *
 * cardNum: the card index requested to be purchased
 *
 * tokens: the tokens the player wishes to use to buy the card
 *
 * wild: the number of wild tokens the player wishes to use to buy the card
 */
void purchase(int cardNum, long* tokens, long wild);

/*
 * will load the amount of tokens needed to purchase the card requested
 * into storage
 *
 * state: Contains all information needed to keep track of the game
 *
 * boardIndex: index of the card the player wants to buy
 *
 * tokens: storage for the amount of tokens required to buy will be loaded to
 *
 * wild: storage for the amount of wild tokens needed to buy will be loaded to
 */
void load_tokens(GameState* state, int boardIndex, long* tokens, long* wild);

/*
 * finds the card with the highest amount of points. Can be more then one card
 *
 * state: Contains all information needed to keep track of the game
 *
 * cardIndexs: indexes of the card to be checked. will mutate to be only the
 *             highest point cards left
 *
 * canPurch: number of card indexes given in cardIndexs. will mutate to be the
 *           number of highest point cards
 *
 * return: returns the number of highest point cards available it will be the 
 *         same as what canPurch is at the end of the function
 */
int find_highest_index(GameState* state, int* cardIndexs, int* canPurch);

/*
 * finds the index of the card with the lowest cost for purchase out of the
 * list provided
 *
 * state: Contains all information needed to keep track of the game
 *
 * canPurch: number of card indexes given in cardIndexs. will mutate to be the
 *           number of lowest cost cards at the end.
 *
 * cardIndexs: indexes of the cards to be checked. will mutate to end up being
 *             only the lowest costing cards out of the list given
 *
 * return: returns the number of lowest cost cards
 */
int find_lowest_cost(GameState* state, int* canPurch, int* cardIndexs);

/*
 * finds the index of the card with the highest cost for purchase out of the
 * list provided
 *
 * state: Contains all information needed to keep track of the game
 *
 * canPurch: number of card indexes given in cardIndexs. will mutate to be the
 *           number of highest cost cards at the end.
 *
 * cardIndexs: indexes of the cards to be checked. will mutate to end up being
 *             only the highest costing cards out of the list given
 *
 * return: returns the number of highest costing cards
 */
int find_highest_cost(GameState* state, int* canPurch, int* cardIndexs);

/*
 * finds the index of the card with the highest amount of wild tokens that
 * the card will require to buy
 *
 * state: Contains all information needed to keep track of the game
 *
 * canPurch: number of card indexes given in cardIndexs. will mutate to be the
 *           number of highest wild cost cards at the end.
 *
 * cardIndexs: indexes of the cards to be checked. will mutate to end up being
 *             only the highest wild costing cards out of the list given
 *
 * return: returns the number of highest wild costing cards
 */
int highest_wild_cost(GameState* state, int* canPurch, int* cardIndexs);

#endif