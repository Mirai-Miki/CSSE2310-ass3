/* card.h
 *
 * Author: Michael Bossner
 *
 * card.h header file for card.c
 */

#ifndef CARD_H
#define CARD_H

#include "lib.h"

///////////////////////// Public Function Prototypes //////////////////////////

/*
 * prints all information contained in the card in for format of
 *
 * "Bonus/score/TP,TB,TY,TR"
 * Bonus:   card colour
 * score:   card points
 * TP:      Purple cost
 * TB:      Brown cost
 * TY:      Yellow cost
 * TR:      Red cost
 *
 * card: card to be printed
 *
 * stream: output to be printed to
 */
void print_card(Card* card, FILE* stream);

/*
 * takes a string and stores the information in a card
 * string must be of the following format to succeed
 *
 * "Bonus/score/TP,TB,TY,TR"
 * Bonus:   card colour
 * score:   card points
 * TP:      Purple cost
 * TB:      Brown cost
 * TY:      Yellow cost
 * TR:      Red cost
 *
 * cardString: string to be stored into the card
 *
 * card: where the information from the string will be stored
 *
 * return: returns 0 if cardString is not of the correct format else returns 1
 */
int unwrap_card(char* cardString, Card* card);

#endif