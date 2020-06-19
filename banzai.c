/* banzai.c
 *
 * Author: Michael Bossner
 *
 * banzai.c is the main file for the Banzai player program
 */

#include <stdio.h>
#include <signal.h>

#include "player.h"
#include "board.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define BANZAI 1
#define POINT_MIN 1
#define TOKEN_MIN 3

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Controls which action banzai will take next dependent on the current game 
 * state and then performs the action he decides to take.
 *
 * state: Contains all information needed to keep track of the game
 *
 * Error 6: Communication Error. Pipe closed or invalid message received.
 */
static void do_what(GameState* state);

////////////////////////////////// Functions //////////////////////////////////

int main(int argc, char** argv) {
    GameState state;
    state.victoryPoints = BANZAI;

    struct sigaction sigAct;
    sigAct.sa_handler = SIG_IGN;

    sigaction(SIGPIPE, &sigAct, NULL);  

    is_args_valid(&state, argc, argv);
    init_player(&state, argc, argv);
    player_loop(&state, do_what);
    
    return 0; // will never happen
}

////////////////////////////// Private Functions //////////////////////////////
//
static void do_what(GameState* state) {
    int cardIndexs[MAX_MARKETS];
    int canPurch;
    long tokens[MAX_TOKEN_COLOUR];
    long numTokens = state->player.wildPile[THIS_PLAYER];
    long wild;

    for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) { // count tokens
        numTokens += state->player.tokens[THIS_PLAYER][colour];
    }
    if (numTokens < TOKEN_MIN && can_take_tokens(state)) { // 1. take tokens
        int tokenCount = 0;
        int colour = YELLOW;
        while (colour != RED) {
            if (colour < PURPLE) {
                colour = RED;
            } 
            if ((state->tokenPile.pile[colour] > 0) && 
                    (tokenCount < TOKEN_MIN)) {
                tokens[colour] = 1;
                tokenCount++;
            } else {
                tokens[colour] = 0;
            }
            if (colour != RED) {
                colour--;
            }
        }
        take(tokens);
    } else if ((canPurch = can_buy_card(state, cardIndexs, POINT_MIN, 
            THIS_PLAYER))) { // 2. Purchase card
        if (canPurch == 1) { // can only buy one
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        } else if (find_highest_cost(state, &canPurch, cardIndexs)) { 
        // most expensive total cost
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        } else if (highest_wild_cost(state, &canPurch, cardIndexs)) {
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        } else { // oldest card
            load_tokens(state, cardIndexs[0], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        }
    } else { // 3. take wild
        take_wild();
    }
}