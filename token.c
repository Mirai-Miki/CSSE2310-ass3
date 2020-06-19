/* token.c
 *
 * Author: Michael Bossner
 *
 * token.c contains functions related to tokens
 */

#include <stdio.h>
#include <stdbool.h>

#include "token.h"
#include "lib.h"

////////////////////////////////// Functions //////////////////////////////////

void init_tokens(GameState* state) {
    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
        state->tokenPile.pile[i] = state->tokenPile.maxTokens;
    }
}

void add_discount(GameState* state, char discount, int player) {
    switch (discount) {
        case 'P':
            state->player.discountList[player][PURPLE] += 1;
            break;
        case 'B':
            state->player.discountList[player][BROWN] += 1;
            break;
        case 'Y':
            state->player.discountList[player][YELLOW] += 1;
            break;
        case 'R':
            state->player.discountList[player][RED] += 1;
            break;
    }
}

bool purchase_sanity_check(GameState* state, long* tokens, Card* card) {
    long playerWild = state->player.wildPile[state->currentPlayer];

    for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
        if ((state->player.tokens[state->currentPlayer][colour] + 
                state->player.discountList[state->currentPlayer][colour] +
                playerWild) >= card->cost[colour]) { //player can afford colour

            if ((state->player.tokens[state->currentPlayer][colour] + 
                    state->player.discountList[state->currentPlayer][colour]) <
                    card->cost[colour]) { // player needs to use wild to buy

                playerWild -= (card->cost[colour] - 
                        (state->player.tokens[state->currentPlayer][colour] + 
                        (state->player.discountList
                        [state->currentPlayer][colour])));
            }

            if (state->player.tokens[state->currentPlayer][colour] <
                    tokens[colour]) { // player does not have that many tokens
                return false;
            }

            if (playerWild < 0) { // player does not have that many wild 
                return false;
            }
        } else { // player can not afford the card
            return false;
        }
    }
    return true;
}

bool take_sanity_check(GameState* state, long* tokens) {
    int count = 0;
    for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
        if (tokens[colour] != 1 && tokens[colour] != 0) { // not valid
            return false;
        } else if (tokens[colour] > state->tokenPile.pile[colour]) {
            // trying to take more then there is
            return false;
        } else if (tokens[colour] == 1) { // count tokens
            count++;
        } 
        if (colour == (MAX_TOKEN_COLOUR - 1) && count != 3) {
            // can only take 3 tokens
            return false;
        }
    }
    return true;
}