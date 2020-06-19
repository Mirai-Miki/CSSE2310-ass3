/* comms.h
 *
 * Author: Michael Bossner
 *
 * comms.h header file for comms.c
 */

#ifndef COMMS_H
#define COMMS_H

#include "lib.h"

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * receives a message from the stream and puts it into a string.
 *
 * stream: place we are trying to get the message from
 *
 * streamEnd: Signals that EOF was received but there is a message waiting
 *
 * return: returns a pointer to the message that was received.
 *         returns a NULL pointer if EOF was received and there is no message
 */
char* rec_message(FILE* stream, int* streamEnd);

/*
 * checks if a purchase message is of a valid format and stores the information
 * in a more manageable structure
 * format for a purchase message should be:
 *
 * "C:TP,TB,TY,TR,TW"
 * C:  card index
 * TP: Number Purple to use
 * TB: Number Brown to use
 * TY: Number Yellow to use
 * TR: Number Red to use
 * TW: Number Wild to use
 *
 * tokens: stores the token information from the message
 *
 * mesIndex: message to be parsed
 *
 * boardIndex: storage for the index of the card from the message
 *
 * returns: true if message is of correct format else false
 */
bool is_valid_purchase(long* tokens, char* mesIndex, int* boardIndex);

/*
 * checks if a take message is of valid format and stores the information
 * in a more manageable structure
 * format for the take message should be:
 *
 * "TP,TB,TY ,TR"
 * TP: Number Purple to take
 * TB: Number Brown to take
 * TY: Number Yellow to take
 * TR: Number Red to take
 *
 * tokens: stores the token information from the message
 *
 * mesIndex: message to be parsed
 *
 * return: true if message is of correct format else false
 */
bool is_valid_take(long* tokens, char* mesIndex);

/*
 * prints all players with the highest score. If there are more then 1 it
 * will print the players in order of name.
 * print will be of the format
 *
 * ""message"name, name, name..."
 * name: names of the winning players. Number of names will change depending
 *         on how many players win
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: message to print
 *
 * stream: output to send message to
 */
void print_winners(GameState* state, char* message, FILE* stream);

#endif