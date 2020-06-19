/* ed.c
 *
 * Author: Michael Bossner
 *
 * ed.c is the main file for the Ed player program
 */

#include <stdio.h>
#include <signal.h>

#include "player.h"
#include "board.h"

#define ED 2
#define POINT_MIN 0
#define IDENTIFIED 0
#define UNIDENTIFIED -1

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Controls which action ed will take next dependent on the current game 
 * state and then performs the action he decides to take.
 *
 * state: Contains all information needed to keep track of the game
 *
 * Error 6: Communication Error. Pipe closed or invalid message received.
 */
static void do_what(GameState* state);

/*
 * selects which tokens ed would like to pick up for a take action
 *
 * state: Contains all information needed to keep track of the game
 *
 * tokens: the tokens that ed would like to pick up
 *
 * neededTokens: the tokens ed need to pick up to buy his card
 */
static void select_tokens(GameState* state, long* tokens, long* neededTokens);

/*
 * finds out if ed can buy his identified card
 *
 * state: Contains all information needed to keep track of the game
 *
 * identifiedCard: index of the card Ed would like to purchase
 *
 * neededTokens: the tokens that needs in order to buy his card
 *
 * canBuy: flag saying if he can buy the card or not
 */
static void can_buy_identified(GameState* state, int identifiedCard, 
        long* neededTokens, bool* canBuy);

////////////////////////////////// Functions //////////////////////////////////

int main(int argc, char** argv) {
    GameState state;
    state.victoryPoints = ED;

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
    int canPurch[MAX_PLAYERS];
    int cardIndexs[MAX_PLAYERS][MAX_MARKETS];
    int highest = 0;
    int identifiedCard = UNIDENTIFIED;
    long tokens[MAX_TOKEN_COLOUR];
    long neededTokens[MAX_TOKEN_COLOUR];
    bool canBuy = true;
    long wild;

    for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) { // inits tokens
        neededTokens[colour] = 0;
        tokens[colour] = 0;
    }
    for (int player = (THIS_PLAYER + 1); ; player++) {
        if (player >= (state->player.count)) { // wrap back to first player
            player = 0;
        } 
        if (player == THIS_PLAYER) { // check everyone
            break;
        }
        if ((canPurch[player] = 
                can_buy_card(state, cardIndexs[player], POINT_MIN, player))) {
        // finds all cards player can buy
            find_highest_index(state, cardIndexs[player], &canPurch[player]);
            // finds the highest point cards. If more then 1 picks the oldest
            if (highest < (check_market_card
                    (state, cardIndexs[player][0])->points)) {
            // found the current highest point card
                highest = (check_market_card
                        (state, cardIndexs[player][0])->points);
                identifiedCard = cardIndexs[player][0];
            }
        }
    }   
    if (identifiedCard >= IDENTIFIED) {
        can_buy_identified(state, identifiedCard, neededTokens, &canBuy);
    }
    if (canBuy && (identifiedCard >= IDENTIFIED)) { // 1. purchase card
        load_tokens(state, identifiedCard, tokens, &wild);
        purchase(identifiedCard, tokens, wild);
    } else if (can_take_tokens(state)) { // 2. take tokens
        select_tokens(state, tokens, neededTokens);
        take(tokens);
    } else { // 3. take wild
        take_wild();
    }
}

//
static void select_tokens(GameState* state, long* tokens, long* neededTokens) {
    int count = 0;

    for (int i = 0; i < 2; i++) { // goes for neededTokens first
        if (neededTokens[YELLOW] >= 1 || i == 1) {
            if (state->tokenPile.pile[YELLOW] > 0 && tokens[YELLOW] == 0 &&
                    count < 3) {
                tokens[YELLOW]++;
                count++;
            }
        }
        if (neededTokens[RED] >= 1 || i == 1) {
            if (state->tokenPile.pile[RED] > 0 && tokens[RED] == 0 && 
                    count < 3) {
                tokens[RED]++;
                count++;
            }
        }
        if (neededTokens[BROWN] >= 1 || i == 1) {
            if (state->tokenPile.pile[BROWN] > 0 && tokens[BROWN] == 0 && 
                    count < 3) {
                tokens[BROWN]++;
                count++;
            }
        }
        if (neededTokens[PURPLE] >= 1 || i == 1) {
            if (state->tokenPile.pile[PURPLE] > 0 && tokens[PURPLE] == 0 && 
                    count < 3) {
                tokens[PURPLE]++;
                count++;
            }
        }
    }
}

//
static void can_buy_identified(GameState* state, int identifiedCard, 
        long* neededTokens, bool* canBuy) {
    long tempWild = state->player.wildPile[THIS_PLAYER];
    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
        if ((state->player.tokens[THIS_PLAYER][i] + 
                state->player.discountList[THIS_PLAYER][i] + tempWild) >= 
                (check_market_card(state, identifiedCard)->cost[i])) {
            // can afford colour
            if ((state->player.tokens[THIS_PLAYER][i] + 
                    state->player.discountList[THIS_PLAYER][i]) < 
                    (check_market_card(state, 
                    identifiedCard)->cost[i])) { // must use wild tokens
                        
                tempWild -= ((check_market_card(state, 
                        identifiedCard)->cost[i]) - 
                        (state->player.tokens[THIS_PLAYER][i] + 
                        state->player.discountList[THIS_PLAYER][i]));
                        
                neededTokens[i] = (check_market_card(state, 
                        identifiedCard)->cost[i]);
            }       
        } else { // cannot buy
            *canBuy = false;
            neededTokens[i] = (check_market_card(state, 
                    identifiedCard)->cost[i]);
        }
    }
}