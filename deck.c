/* deck.c 
 *
 * Author: Michael Bossner
 *
 * deck.c contains functions related to the games deck
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "deck.h"
#include "lib.h"
#include "endAusterity.h"
#include "comms.h"
#include "card.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define MIN_CARDS 1
#define INVALID -1
#define FILE_END 0
#define CARD_END 1

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Adds a new card to the deck pile if it is of a valid format
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
 * return: Returns -1 if EOF is received at the start of the line
 *         Returns 0 if the deck file is invalid
 *         Returns 1 if card was added successfully
 *
 * Error 4: Invalid deck file contents. deckFile does not meet the format
 */
static int add_card(FILE* deckFile, GameState* state);

////////////////////////////////// Functions //////////////////////////////////

void load_deck_file(char* deckFileName, GameState* state) {
    FILE* deckFile = fopen(deckFileName, "r");
    if (deckFile == NULL) {
        end_austerity(state, CANNOT_OPEN_DECK);
    }

    state->deck.size = 0;
    state->deck.deckIndex = 0;
    // Creates storage for a single card
    state->deck.cardPile = malloc(sizeof(Card*) * MIN_CARDS);
    // Adds all cards to the pile and keeps count
    while (add_card(deckFile, state) > 0) {
        state->deck.size++;
        state->deck.cardPile = realloc(state->deck.cardPile, 
                sizeof(Card*) * (state->deck.size + 1));
    }
    fclose(deckFile);
}

void free_deck(GameState* state) {
    for (int i = 0; i < state->deck.size; i++) {
        free(state->deck.cardPile[i]);
    }
    free(state->deck.cardPile);
}

////////////////////////////// Private Functions //////////////////////////////
//
static int add_card(FILE* deckFile, GameState* state) {

    Card* card = malloc(sizeof(Card) * 1);
    char* message;
    int status;
    int streamEnd;
    if ((message = rec_message(deckFile, &streamEnd)) == NULL) { // EOF line
        status = INVALID;
    } else if (streamEnd) { // message 'EOF' terminated
        status = FAIL;
    } else { // normal message '\n' terminated
        status = unwrap_card(message, card);
    }

    switch (status) {
        case FAIL:
            free(card);
            free(message);
            fclose(deckFile);
            end_austerity(state, INVALID_DECK);
        case INVALID:
            free(card);     
            if (state->deck.size == 0) { // Only fails if there are no cards
                fclose(deckFile);
                end_austerity(state, INVALID_DECK);
            }
            break;
        default:
            free(message);
            state->deck.cardPile[state->deck.size] = card;
    }
    return status;
}