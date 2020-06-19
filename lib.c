/* lib.c
 *
 * Author: Michael Bossner
 *
 * lib.c contains general function useful for many situations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define NAME_INT_CONV(player) (player + 65)
#define NAME_CHAR_CONV(player) (player - 65)

////////////////////////////////// Functions //////////////////////////////////

bool is_num_args_valid(int argc, int minArgs, int maxArgs) {
    if (argc < minArgs || argc > maxArgs) {
        return false;
    } else {
        return true;
    }
}

int is_str_pos_number(char* string) {
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] < ZERO || string[i] > NINE) {
            return INVALID;
        }
    }
    return atoi(string);
}

char player_int_to_char(int player) {
    return (char)NAME_INT_CONV(player);
}

int player_char_to_int(char player) {
    return (int)NAME_CHAR_CONV(player);
}