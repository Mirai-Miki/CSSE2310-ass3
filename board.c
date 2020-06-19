/* 
 * board.c
 *
 * Author: Michael Bossner
 *
 * board.c is contains all functions related to the game board
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "board.h"
#include "deck.h"
#include "lib.h"
#include "card.h"

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Checks how many markets are set up on the board
 *
 * state: Contains all information needed to keep track of the game
 *
 * return: returns the number of markets set up on the board
 */
static int market_length(GameState* state);

////////////////////////////////// Functions //////////////////////////////////

void init_board(GameState* state) {
    state->board.oldest = NULL;
    state->board.youngest = NULL;
}

bool is_board_empty(GameState* state) {
    if (state->board.oldest == NULL) {
        return true;
    } else {
        return false;
    }
}

void free_board(GameState* state) {
    Market* temp;
    while (!is_board_empty(state)) {
        temp = state->board.oldest->next;
        free(state->board.oldest);
        state->board.oldest = temp;
    }
}

Card* purchase_card(GameState* state, int boardIndex) {
    Card* purchasedCard;
    if (boardIndex >= 0 && boardIndex < market_length(state)) {
        Market* temp = state->board.oldest;
        if (boardIndex == 0) { // card is the oldest card
            if (market_length(state) == 1) { // only 1 card on the board
                state->board.oldest = NULL;
                state->board.youngest = NULL;
            } else { // more than 1 card on the board
                temp->next->prev = NULL;
                state->board.oldest = temp->next;
            }
        } else if (boardIndex == market_length(state) - 1) {
            // card is the youngest card
            temp = state->board.youngest;
            temp->prev->next = NULL;
            state->board.youngest = temp->prev;
        } else { // card is a middle card
            for (int i = 0; i <= boardIndex; i++) {             
                if (i == boardIndex) {                  
                    temp->prev->next = temp->next;
                    temp->next->prev = temp->prev;
                } else {
                    temp = temp->next;
                }
            }
        }
        purchasedCard = temp->card;
        free(temp);
        return purchasedCard;
    } else {
        return NULL;
    }
}

void print_board(GameState* state, FILE* stream) {
    int cardCount = 0;
    Market* temp = state->board.oldest;
    while (temp != NULL) {
        fprintf(stream, "Card %d:", cardCount);
        fflush(stream);
        print_card(temp->card, stream);
        temp = temp->next;
        cardCount++;
    }
}

int add_to_board(GameState* state, Card* card) {
    if ((state->deck.deckIndex < state->deck.size) && 
            (market_length(state) < MAX_MARKETS)) { 
        // card available && market spot available
        Market* market = malloc(sizeof(Market) * 1);
        market->card = card;
        if (is_board_empty(state)) { // board is empty
            state->board.oldest = market;
            state->board.youngest = market;
            market->prev = NULL;
        } else { // board is not empty but not full
            market->prev = state->board.youngest;
            state->board.youngest->next = market;
            state->board.youngest = market;
        }
        market->next = NULL;
        return VALID;
    } else {
        return FAIL;
    }
}

Card* check_market_card(GameState* state, int boardIndex) {
    Market* temp = state->board.oldest;
    while (temp != NULL) {
        if (!boardIndex) {
            return temp->card;
        } else {
            temp = temp->next;
            boardIndex--;
        }
    }
    return NULL;
}

////////////////////////////// Private Functions //////////////////////////////
//
static int market_length(GameState* state) {
    int count = 0;
    Market* temp = state->board.oldest;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    return count;
}