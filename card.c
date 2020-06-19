/* card.c
 *
 * Author: Michael Bossner
 *
 * card.c contains functions related to cards
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"
#include "lib.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define MIN_CARD_LEN 11
#define COLOUR 0
#define POINTS 2
#define BASE 10

////////////////////////////////// Functions //////////////////////////////////

void print_card(Card* card, FILE* stream) {
    fprintf(stream, "%c/%ld/%ld,%ld,%ld,%ld\n", 
            card->discount, 
            card->points,
            card->cost[PURPLE], 
            card->cost[BROWN], 
            card->cost[YELLOW], 
            card->cost[RED]);
    fflush(stream);
}

int unwrap_card(char* cardString, Card* card) {
    char* index;

    if (strlen(cardString) < MIN_CARD_LEN) { // not a valid card
        return FAIL;
    } else if ((cardString[COLOUR] != 'P' && cardString[COLOUR] != 'B' && 
            cardString[COLOUR] != 'Y' && cardString[COLOUR] != 'R') || 
            (cardString[1] != ':')) { // not a valid card
        return FAIL;
    } else {
        card->discount = cardString[0];
    }

    if (cardString[POINTS] < ZERO || cardString[POINTS] > NINE) { 
        // needs to be a number
        return FAIL;
    } else {
        card->points = strtol(&cardString[POINTS], &index, BASE);
    }

    if (index[0] != ':' && (index[1] < ZERO || index[1] > NINE)) {
        return FAIL;
    } else {
        card->cost[PURPLE] = strtol(&index[1], &index, BASE);
    }
    for (int colour = BROWN; colour <= RED; colour++) {
        if (index[0] != ',' && (index[1] < ZERO || index[1] > NINE)) {
            return FAIL;
        } else {
            card->cost[colour] = strtol(&index[1], &index, BASE);
        }
    }

    if (index[0] != '\0') { // there should not be anything else
        return FAIL;
    } else { // card is valid
        return VALID;
    }
}