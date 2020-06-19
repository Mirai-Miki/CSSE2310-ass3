/* shenzi.c
 *
 * Author: Michael Bossner
 *
 * shenzi.c is the main file for the Shenzi player program
 */

#include <stdio.h>
#include <signal.h>

#include "player.h"
#include "board.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define SHENZI 0
#define POINT_MIN 0

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Controls which action shenzi will take next dependent on the current game 
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

    struct sigaction sigAct;
    sigAct.sa_handler = SIG_IGN; // Ignore

    sigaction(SIGPIPE, &sigAct, NULL);

    state.victoryPoints = SHENZI;

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
    long wild;
    
    if ((canPurch = can_buy_card(state, cardIndexs, POINT_MIN, THIS_PLAYER))) {
    // 1. buy card      
        if (canPurch == 1) { // can only buy one
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        } else if (find_highest_index(state, cardIndexs, &canPurch) 
                == 1) { // highest points
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        } else if (find_lowest_cost(state, &canPurch, cardIndexs) == 1) {
            // lowest cost
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        } else { // youngest
            load_tokens(state, cardIndexs[(canPurch - 1)], tokens, &wild);
            purchase(cardIndexs[(canPurch - 1)], tokens, wild);
        }
    } else if (can_take_tokens(state)) { // 2. take tokens
        int tokenCount = 0;
        for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
            if ((state->tokenPile.pile[i] > 0) && (tokenCount < 3)) {
                tokens[i] = 1;
                tokenCount++;
            } else {
                tokens[i] = 0;
            }
        }
        take(tokens);
    } else { // 3. take wild
        take_wild();
    }
}