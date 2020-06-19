/* comms.c 
 *
 * Author: Michael Bossner
 *
 * comms.c contains functions related to communication
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "comms.h"
#include "lib.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define ZERO 48
#define NINE 57
#define SEVEN 55
#define FLAGGED 1
#define BASE 10

////////////////////////////////// Functions //////////////////////////////////

char* rec_message(FILE* stream, int* streamEnd) {
    int next;
    int messageSize = 0;
    char* message = malloc(sizeof(char) * 1);   
    *streamEnd = 0;

    FOREVER {
        next = fgetc(stream);
        switch (next) {
            case EOF:
                if (messageSize != 0) { // there is a message but EOF was found
                    *streamEnd = FLAGGED;
                    message[messageSize] = '\0';
                    return message;
                } else {
                    free(message);
                    return NULL;
                }
            case '\n':
                message[messageSize] = '\0';
                return message;
            default:
                message = realloc(message, sizeof(char) * messageSize + 2);
                message[messageSize] = next;
                messageSize++;
        }
    }
}

bool is_valid_purchase(long* tokens, char* mesIndex, int* boardIndex) {

    if (mesIndex[0] < ZERO || mesIndex[0] > SEVEN || mesIndex[1] != ':') {
        return false;
    }
    *boardIndex = (mesIndex[0] - ZERO);
    mesIndex = &mesIndex[2];
    // also checks wild unlike is_valid_take()
    for (int i = 0; i <= MAX_TOKEN_COLOUR; i++) { // for each colour
        if (mesIndex[0] < ZERO || mesIndex[0] > NINE) { // not a number
            return false;
        } else {
            tokens[i] = strtol(&mesIndex[0], &mesIndex, BASE);
        }
        if (((mesIndex[0] != ',') && (i < MAX_TOKEN_COLOUR)) || 
                ((mesIndex[0] != '\0') && (i == MAX_TOKEN_COLOUR))) {
            return false;
        } else if (i < MAX_TOKEN_COLOUR) { // set index to skip to next num
            mesIndex = &mesIndex[1];
        }
    }
    return true;
}

bool is_valid_take(long* tokens, char* mesIndex) {

    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
        if (mesIndex[0] < ZERO || mesIndex[0] > NINE) {
            return false;
        } else {
            tokens[i] = strtol(mesIndex, &mesIndex, BASE);
        }
        if (((mesIndex[0] != ',') && (i < MAX_TOKEN_COLOUR - 1)) ||
                ((mesIndex[0] != '\0') && (i == (MAX_TOKEN_COLOUR - 1)))) {
            return false;
        } else if (i < (MAX_TOKEN_COLOUR - 1)) {
            mesIndex = &mesIndex[1];
        }
    }
    return true;
}

void print_winners(GameState* state, char* message, FILE* stream) {

    long highest = 0;
    int count = 0;

    fprintf(stream, "%s", message);
    fflush(stream);

    for (int i = 0; i < state->player.count; i++) { // checks the highest score
        if (highest < state->player.scoreCard[i]) {
            highest = state->player.scoreCard[i];
            count = 1;
        } else if (state->player.scoreCard[i] == highest) { // more than 1
            count++;
        }
    }
    for (int i = 0; i < state->player.count; i++) { // prints all winners
        if (state->player.scoreCard[i] == highest) {
            if (count > 1) {
                fprintf(stream, "%c,", player_int_to_char(i));
                fflush(stream);
                count--;
            } else {
                fprintf(stream, "%c\n", player_int_to_char(i));
                fflush(stream);
            }
        }
    }
}