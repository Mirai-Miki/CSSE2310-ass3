/* game.c 
 *
 * Author: Michael Bossner
 *
 * game.c contains functions related to the game loop
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "game.h"
#include "lib.h"
#include "board.h"
#include "deck.h"
#include "endAusterity.h"
#include "comms.h"
#include "card.h"
#include "token.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define PROTOCOL_ERR_MAX 2
#define TOKENS_WILD 5
#define TAKE_MIN 11

/* All actions the hub can receive from the player*/
enum DoWhat {
    INVALID_MESSAGE,
    WILD,
    PURCHASE,
    TAKE,
};

/* Used for indexing into a wild action message*/
enum Wild {
    WILD_W,
    WILD_I,
    WILD_L,
    WILD_D,
    WILD_START,
};

/* Used for indexing into a purchase action message*/
enum Purchase {
    PUR_P,
    PUR_U,
    PUR_R,
    PUR_C,
    PUR_H,
    PUR_A,
    PUR_S,
    PUR_E,
    PUR_START,
};

/* Used for indexing into a take action message*/
enum Take {
    TAKE_T,
    TAKE_A,
    TAKE_K,
    TAKE_E,
    TAKE_START,
};

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Starts the game by sending the tokens message then drawing up to 8 cards
 * to the board from the deck. May be less if the deck size is smaller than 8.
 *
 * state: Contains all information needed to keep track of the game
 */
static void game_start(GameState* state);

/*
 * Checks to see if any signals have been flagged
 *
 * state: Contains all information needed to keep track of the game
 *
 * Error 5: Bad start. failed to start all players
 *
 * Error 10: SIGINT caught
 */
static void check_flags(GameState* state);

/*
 * Takes the next card in the deck and places in on the board.
 * Informs all players of the new card
 *
 * state: Contains all information needed to keep track of the game
 */
static void new_card(GameState* state);

/*
 * Checks if the message received is a valid action
 * Valid actions are
 * "take"
 * "wild"
 * "purchase"
 *
 * state: Contains all information needed to keep track of the game
 *
 * action: messaged to be checked
 *
 * return: returns 0 if action is not valid
 *         returns 1 if action is "wild"
 *         returns 2 if action is "purchase"
 *         returns 3 if action is "take"
 *
 * Error 6: Client disconnected. EOF received from the player
 */
static int is_action_valid(GameState* state, char* action);

/*
 * Sends the doWhat message to the player who's turn it is and waits for a
 * reply. Parses the reply and takes the corresponding action if the message
 * is valid. If an invalid messaged is received doWhat will be sent again
 * and will wait for another reply. If this message is also invalid the game
 * will exit on a protocol error.
 * Valid messages are
 *
 * "purchaseC:TP,TB,TY,TR,TW\n"
 * "takeTP,TB,TY,TR\n"
 * "wild\n"
 *
 * C:  Card index to be purchased. Must be an integer from 0 -> 7
 * TP: Purple token to be used in purchase or taken. Must be a positive int
 * TB: Brown token to be used in purchase or taken. Must be a positive int
 * TY: Yellow token to be used in purchase or taken. Must be a positive int
 * TR: Red token to be used in purchase or taken. Must be a positive int
 * TW: wild token to be used in purchase. Must be a positive int
 *
 * state: Contains all information needed to keep track of the game
 *
 * Error 6: Client disconnected. EOF received from the player
 *
 * Error 7: Protocol Error. 2 invalid messages received in a row
 */
static void do_what(GameState* state);

/*
 * Informs all players that the current player took a wild token
 *
 * state: Contains all information needed to keep track of the game
 */
static void took_wild(GameState* state);

/*
 * Checks if the purchase is of a valid format
 * Also does sanity checks against the current state of the game to check
 * for cheating or purchases of cards that are not on the board.
 * If the purchase is accepted card will be removed from the board and
 * the state will be updated and all players will be informed of the purchase
 * if there is another card on the deck it will be drawn to the board. 
 * format for the purchase must be
 *
 * "C:TP,TB,TY,TR,TW\n"
 *
 * C:  Card index to be purchased. Must be an integer from 0 -> 7
 * TP: Purple token to be used in purchase. Must be a positive int
 * TB: Brown token to be used in purchase. Must be a positive int
 * TY: Yellow token to be used in purchase. Must be a positive int
 * TR: Red token to be used in purchase. Must be a positive int
 * TW: wild token to be used in purchase. Must be a positive int
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: message to be parsed for the purchase
 *
 * return: Returns 0 if the message is not of the correct format
 *         Returns 0 if there is no card on the board where the index asks for
 *         Returns 0 if the player trys to buy a card with less tokens then
 *         they have
 *         Returns 1 if the purchase succeeds
 */
static int purchased(GameState* state, char* message);

/*
 * Checks if the take is of a valid format
 * Also does sanity checks against the current state of the game to check
 * for cheating or asking to take from empty piles.
 * If the take is accepted the coins will be removed from the token pile and
 * given to the player. All players will then be informed of the take action. 
 * format for the take must be
 *
 * "TP,TB,TY,TR\n"
 *
 * TP: Purple tokens to be taken. Must be a positive int
 * TB: Brown tokens to be taken. Must be a positive int
 * TY: Yellow tokens to be taken. Must be a positive int
 * TR: Red tokens to be taken. Must be a positive int
 *
 * cannot take more than 3 coins || if the token pile has less than 3 tokens
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: message to be parsed for the take
 *
 * return: Returns 0 if the message is not of the correct format or if the
 *         action fails a sanity check.
 *         Returns 1 if the take succeeds
 */
static int took(GameState* state, char* message);

/*
 * Prints the tokens message to all players
 *
 * state: Contains all information needed to keep track of the game
 */
static void tokens(GameState* state);

/*
 * checks if the game is over either by the board running out of card for
 * purchase or the victory score has been reached and all player have had
 * there turn. if true austerity will begin shut down.
 *
 * state: Contains all information needed to keep track of the game
 */
static void is_game_over(GameState* state);

/*
 * Prints the purchased message to all players
 *
 * state: Contains all information needed to keep track of the game
 *
 * boardIndex: Index of the card being purchased
 *
 * tokens: Number of tokens in each colour being used to buy
 */
static void print_purchased(GameState* state, int boardIndex, long* tokens);

/*
 * Prints the take message to all players
 *
 * state: Contains all information needed to keep track of the game
 *
 * tokens: Number of tokens in each colour being taken
 */
static void print_take(GameState* state, long* tokens);

////////////////////////////////// Functions //////////////////////////////////

void game_loop(GameState* state) {
    check_flags(state);
    game_start(state);
    FOREVER {
        check_flags(state);
        is_game_over(state);

        do_what(state);

        if (state->currentPlayer == state->player.count - 1) {
            state->currentPlayer = 0;
        } else {
            state->currentPlayer++;
        }
    }
}

////////////////////////////// Private Functions //////////////////////////////
//
static void game_start(GameState* state) {
    tokens(state);
    for (int i = 0; i < MAX_MARKETS; i++) {
        new_card(state);
    }
}

//
static void check_flags(GameState* state) {
    if (sigStore.badStart) { // child failed to exec
        end_austerity(state, BAD_START);
    } else if (sigStore.sigIntCaught) { // SIGINT fired
        end_austerity(state, SIGINT_CAUGHT);
    }
}

//
static void new_card(GameState* state) {
    Card* card = state->deck.cardPile[state->deck.deckIndex];

    if (add_to_board(state, card)) {
        state->deck.deckIndex++;
    } else { // no cards can be added
        return;
    }

    for (int player = 0; player < state->player.count; player++) {
        fprintf(state->player.commsList[player][WRITE], 
                "newcard%c:%ld:%ld,%ld,%ld,%ld\n",
                card->discount,
                card->points,
                card->cost[PURPLE],
                card->cost[BROWN],
                card->cost[YELLOW],
                card->cost[RED]);
        fflush(state->player.commsList[player][WRITE]);
    }
    printf("New card = Bonus %c, worth %ld, costs %ld,%ld,%ld,%ld\n", 
            card->discount,
            card->points,
            card->cost[PURPLE],
            card->cost[BROWN],
            card->cost[YELLOW],
            card->cost[RED]);
    fflush(stdout);
}

//
static int is_action_valid(GameState* state, char* action) {
    if (action == NULL) {
        end_austerity(state, CLIENT_DISCONNECT);
    }
    int messageSize = strlen(action);

    if (messageSize == WILD_START) { // wild
        if (action[WILD_W] == 'w' && action[WILD_I] == 'i' && 
                action[WILD_L] == 'l' && action[WILD_D] == 'd') {
            return WILD;
        }
    } else if (messageSize >= TAKE_MIN) { // other action
        if ((action[PUR_P] == 'p' && action[PUR_U] == 'u' && 
                action[PUR_R] == 'r' && action[PUR_C] == 'c' &&
                action[PUR_H] == 'h' && action[PUR_A] == 'a' &&
                action[PUR_S] == 's' && action[PUR_E] == 'e')) {
            return PURCHASE;
        } else if (action[TAKE_T] == 't' && action[TAKE_A] == 'a' &&
                action[TAKE_K] == 'k' && action[TAKE_E] == 'e') {
            return TAKE;
        }
    }
    return FAIL;
}

//
static void do_what(GameState* state) {
    int streamEnd = 0;
    char* message;
    int action;

    for (int protocolError = 0; protocolError < PROTOCOL_ERR_MAX; 
            protocolError++) {

        fprintf(state->player.commsList[state->currentPlayer][WRITE], 
                "dowhat\n");
        fflush(state->player.commsList[state->currentPlayer][WRITE]);

        message = rec_message( // wait for message from player
                state->player.commsList[state->currentPlayer][READ], 
                &streamEnd);
        action = is_action_valid(state, message);
        
        switch (action) {
            case INVALID_MESSAGE:               
                break;
            case WILD:
                took_wild(state);
                free(message);
                return;
            case PURCHASE:
                if(purchased(state, &message[PUR_START])) {
                    free(message);
                    return;
                } else {
                    break;
                }
            case TAKE:
                if(took(state, &message[TAKE_START])) {
                    free(message);
                    return;
                } else {
                    break;
                }
        }
        free(message);
    }
    end_austerity(state, PROTOCOL_ERR);
}

//
static void took_wild(GameState* state) {
    state->player.wildPile[state->currentPlayer]++;

    for (int player = 0; player < state->player.count; player++) {
        fprintf(state->player.commsList[player][WRITE], 
                "wild%c\n", player_int_to_char(state->currentPlayer));
        fflush(state->player.commsList[player][WRITE]);
    }

    printf("Player %c took a wild\n", 
            player_int_to_char(state->currentPlayer));
    fflush(stdout);
}

//
static int purchased(GameState* state, char* message) {
    Card* card;
    long tokens[TOKENS_WILD];
    char* mesIndex = message;
    int boardIndex;

    if(!is_valid_purchase(tokens, mesIndex, &boardIndex)) {
        return FAIL;
    } else if ((card = check_market_card(state, boardIndex)) == NULL) {
        return FAIL; // no card at board index
    } else {

        if (!purchase_sanity_check(state, tokens, card)) {
            return FAIL; // not a legal purchase
        }

        purchase_card(state, boardIndex);
        // update state
        for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
            state->player.tokens[state->currentPlayer][colour] -= 
                    tokens[colour];
            state->tokenPile.pile[colour] += tokens[colour];
        }
        state->player.wildPile[state->currentPlayer] -= tokens[WILD_START];
        add_discount(state, card->discount, state->currentPlayer);
        state->player.scoreCard[state->currentPlayer] += card->points;
    }

    print_purchased(state, boardIndex, tokens); 
    new_card(state);

    return VALID;
}

//
static void print_purchased(GameState* state, int boardIndex, long* tokens) {
    for (int player = 0; player < state->player.count; player++) {
        fprintf(state->player.commsList[player][WRITE], 
                "purchased%c:%d:%ld,%ld,%ld,%ld,%ld\n", 
                player_int_to_char(state->currentPlayer),
                boardIndex,
                tokens[PURPLE],
                tokens[BROWN],
                tokens[YELLOW],
                tokens[RED],
                tokens[WILD_START]);
        fflush(state->player.commsList[player][WRITE]);
    }

    printf("Player %c purchased %d using %ld,%ld,%ld,%ld,%ld\n", 
            player_int_to_char(state->currentPlayer),
            boardIndex,
            tokens[PURPLE],
            tokens[BROWN],
            tokens[YELLOW],
            tokens[RED],
            tokens[WILD_START]);
    fflush(stdout);
}

//
static int took(GameState* state, char* message) {
    long tokens[MAX_TOKEN_COLOUR];
    char* mesIndex = message;

    if (!is_valid_take(tokens, mesIndex)) {
        return FAIL;
    } else if (!take_sanity_check(state, tokens)) {
        return FAIL; // not a legal take
    } else { // update state
        for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
            state->tokenPile.pile[colour] -= tokens[colour];
            state->player.tokens[state->currentPlayer][colour] += 
                    tokens[colour];
        }
    }

    print_take(state, tokens);  
    return VALID;
}

//
static void print_take(GameState* state, long* tokens) {
    for (int player = 0; player < state->player.count; player++) {
        fprintf(state->player.commsList[player][WRITE], 
                "took%c:%ld,%ld,%ld,%ld\n", 
                player_int_to_char(state->currentPlayer),
                tokens[PURPLE],
                tokens[BROWN],
                tokens[YELLOW],
                tokens[RED]);
        fflush(state->player.commsList[player][WRITE]);
    }

    printf("Player %c drew %ld,%ld,%ld,%ld\n", 
            player_int_to_char(state->currentPlayer),
            tokens[PURPLE],
            tokens[BROWN],
            tokens[YELLOW],
            tokens[RED]);
    fflush(stdout);
}

//
static void tokens(GameState* state) {
    for (int player = 0; player < state->player.count; player++) {

        fprintf(state->player.commsList[player][WRITE], 
                "tokens%d\n", state->tokenPile.maxTokens);
        fflush(state->player.commsList[player][WRITE]);
    }
}

//
static void is_game_over(GameState* state) {
    if (is_board_empty(state)) {
        end_austerity(state, GAME_OVER);
    }
    for (int player = 0; player < state->player.count; player++) {
        if (state->player.scoreCard[player] >= state->victoryPoints && 
                state->currentPlayer == 0) { // score reached && player 0
            end_austerity(state, GAME_OVER);
        }
    }
}