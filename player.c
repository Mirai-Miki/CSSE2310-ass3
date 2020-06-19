/* player.c
 *
 * Author: Michael Bossner
 *
 * player.c contains functions related to player processes
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "player.h"
#include "lib.h"
#include "comms.h"
#include "board.h"
#include "card.h"
#include "token.h"

/////////////////////////////////// Defines ///////////////////////////////////

#define ARGV_THIS_PLAYER 2
#define ARGV_TOTAL_PLAYER 1
#define TOKEN_START 6
#define CARD_START 7
#define WILD_START 4
#define PURCH_START 9
#define TOOK_START 4
#define A_CHAR 65
#define MAX_PLAYERS 26
#define MIN_PLAYERS 2
#define ARG_COUNT 3
#define FLAGGED 1
#define MIN_TOKENS 3
#define EOG 3
#define DO_WHAT_MES 6
#define PURCHASED_MIN 22
#define NEW_CARD_MIN 18
#define TOKENS_MIN 7
#define WILD_MIN 5
#define TOOK_MIN 13
#define TOKENS_AND_WILD 5
#define WILD_INDEX 4

/**/
enum End {
    GAME_OVER = 0,
    WRONG_NUM_ARGS = 1,
    INVALID_NUM_PLAYER = 2,
    INVALID_ID = 3,
    COMMS_ERR = 6,
};

/**/
enum ActionList {
    NO_ACTION,
    END_OF_GAME,
    DO_WHAT,
    PURCHASED,
    NEW_CARD,
    TOOK,
    TOKENS,
    WILD,
};

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * prints the game over message for players and prints the winners.
 * then begins shut down of the program
 *
 * state: Contains all information needed to keep track of the game
 */
static void end_of_game(GameState* state);

/*
 * handles shutting down the player and printed any error messages
 *
 * state: Contains all information needed to keep track of the game
 *
 * exitStatus: reason the player is closing
 *
 * Error 1: Wrong number of arguments.
 *
 * Error 2: Invalid player count
 *
 * Error 3: Invalid player ID
 *
 * Error 6: Communication Error
 */
static void end_player(GameState* state, int exitStatus);

/*
 * parses the message to check if it is the end of game message
 *
 * return: Returns true if message is "eog\n" else false
 */
static bool is_eog(char* message);

/*
 * parses the message to check if it is the dowhat message
 *
 * return: returns true is the message is "dowhat\n" else false
 */
static bool is_dowhat(char* message);

/*
 * parses the message to check if it is the purchased message
 *
 * return: returns true if the first 9 character are "purchased" else false
 *         note does not check line length         
 */
static bool is_purchased(char* message);

/*
 * parses the message to check if it is the newcard message
 *
 * return: returns true if the first 7 character are "newcard" else false
 *         note does not check line length 
 */
static bool is_newcard(char* message);

/*
 * parses the message to check if it is the took message
 *
 * return: returns true if the first 4 character are "took" else false
 *         note does not check line length   
 */
static bool is_took(char* message);

/*
* parses the message to check if it is the tokens message
 *
 * return: returns true if the first 6 character are "tokens" else false
 *         note does not check line length   
 */
static bool is_tokens(char* message);

/*
 * parses the message to check if it is the wild message
 *
 * return: returns true if the first 4 character are "wild" else false
 *         note does not check line length   
 */
static bool is_wild(char* message);

/*
 * performs the purchased action by parsing the second half of the message
 * and making sure it is of the correct format if it is the game state is
 * updated. message format should be in
 *
 * "P:C:TP,TB,TY,TR,TW\n"
 * P:  index of the player buying the card
 * C:  index of the card to be purchased
 * TP: number of purple tokens used to buy
 * TB: number of brown tokens used to buy
 * TY: number of yellow tokens used to buy
 * TR: number of red tokens used to buy
 * TW: number of wild tokens used to buy
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: purchased message to be parsed
 *
 * Error 6: Communication Error. Invalid message
 */
static void purchased(GameState* state, char* message);

/*
 * performs the newcard action by parsing the message received and making
 * sure it is of the correct format and updating the game state if it is.
 * message format should be
 *
 * "D:V:TP,TB,TY,TR\n"
 * D:  The colour of the card
 * V:  the number of points the card provides
 * TP: number of purple tokens the card costs
 * TB: number of brown tokens the card cost
 * TY: number of yellow tokens the card cost
 * TR: number of red tokens the card cost
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: newcard message to be parsed
 *
 * Error 6: Communication Error. Invalid message
 */
static void new_card(GameState* state, char* message);

/*
 * performs the took action by parsing the message received and making
 * sure it is of the correct format and updating the game state if it is.
 * message format should be
 *
 * "P:TP,TB,TY,TR\n"
 * TP: number of purple tokens taken
 * TB: number of brown tokens taken
 * TY: number of yellow tokens taken
 * TR: number of red tokens taken
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: took message to be parsed
 *
 * Error 6: Communication Error. Invalid message
 */
static void took(GameState* state, char* message);

/*
 * performs the tokens action by parsing the message received and making
 * sure it is of the correct format and updating the game state if it is.
 * message format should be
 *
 * "T\n"
 * T: number of tokens in each non-wild pile
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: tokens message to be parsed
 *
 * Error 6: Communication Error. Invalid message
 */
static void tokens(GameState* state, char* message);

/*
 * performs the wild action by parsing the message received and making
 * sure it is of the correct format and updating the game state if it is.
 * message format should be
 *
 * "P\n"
 * P: Player who submits the action
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: wild message to be parsed
 *
 * Error 6: Communication Error. Invalid message
 */
static void wild(GameState* state, char* message);

/*
 * prints the state of the board and the state of all players
 *
 * state: Contains all information needed to keep track of the game
 *
 * stream: output for the messages to be sent
 */
static void print_state(GameState* state, FILE* stream);

/*
 * prints the state of all players
 * 
 * state: Contains all information needed to keep track of the game
 *
 * stream: output for the message to be sent
 */
static void print_player_state(GameState* state, FILE* stream);

/*
 * checks that a players name is valid
 *
 * state: Contains all information needed to keep track of the game
 *
 * message: players name to be checked
 *
 * return: Returns true if a players name is a char between 'A' to 'Z'
 *         else returns false
 */
static bool player_parse(GameState* state, char* message);

/*
 * checks if a message is a valid action
 *
 * message: message to be checked
 *
 * return: Returns 0 if the message is not a valid action
 *         Returns 1 if the message is "eog"
 *         Returns 2 if the message is "dowhat"
 *         Returns 3 if the message starts with "purchased"...
 *         Returns 4 if the message starts with "newcard"...
 *         Returns 5 if the message starts with "took"...
 *         Returns 6 if the message starts with "tokens"...
 *         Returns 7 if the message starts with "wild"...
 */
static int parse_message(char* message);

/*
 * counts the total number of tokens a card costs
 *
 * state: Contains all information needed to keep track of the game
 *
 * return: returns the number of tokens the card costs
 */
static int card_cost_count(GameState* state, Card* card);

/*
 * Frees all cards that are being sold by markets on the board
 *
 * state: Contains all information needed to keep track of the game
 */
static void free_player_card(GameState* state);

////////////////////////////////// Functions //////////////////////////////////

void is_args_valid(GameState* state, int argc, char** argv) {
    if (argc != ARG_COUNT) {
        end_player(state, WRONG_NUM_ARGS);
    } 

    int totalPlayers = is_str_pos_number(argv[ARGV_TOTAL_PLAYER]);
    int thisPlayer = is_str_pos_number(argv[ARGV_THIS_PLAYER]);

    
    if ((totalPlayers < MIN_PLAYERS) || (totalPlayers > MAX_PLAYERS)) {
        end_player(state, INVALID_NUM_PLAYER);
    } else if ((thisPlayer < 0) || (thisPlayer > MAX_PLAYERS - 1) || 
            (totalPlayers <= thisPlayer)) {
        end_player(state, INVALID_ID);
    }
}

void init_player(GameState* state, int argc, char** argv) {
    THIS_PLAYER = is_str_pos_number(argv[ARGV_THIS_PLAYER]);
    state->player.count = is_str_pos_number(argv[ARGV_TOTAL_PLAYER]);
    
    for (int player = 0; player < state->player.count; player++) {
        for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
            state->player.tokens[player][colour] = 0;
            state->player.discountList[player][colour] = 0;
        }
        state->player.scoreCard[player] = 0;
        state->player.wildPile[player] = 0;
        state->deck.size = INT_MAX;
        state->deck.deckIndex = 0;
    }
    init_board(state);
}

void player_loop(GameState* state, void (*doWhat)(GameState* state)) {
    char* message;
    int streamEnd = 0;
    int action;

    FOREVER {
        if(!(message = rec_message(stdin, &streamEnd)) || 
                streamEnd == FLAGGED) { // EOF received
            end_player(state, COMMS_ERR);
        } else {
            action = parse_message(message);

            switch (action) {
                case NO_ACTION:
                    end_player(state, COMMS_ERR);
                case END_OF_GAME:
                    free(message);
                    end_of_game(state);
                    break;
                case DO_WHAT:
                    fprintf(stderr, "Received dowhat\n");
                    fflush(stderr);
                    doWhat(state);
                    break;
                case PURCHASED:
                    purchased(state, &message[PURCH_START]);
                    break;
                case NEW_CARD:
                    new_card(state, &message[CARD_START]);
                    break;
                case TOOK:
                    took(state, &message[TOOK_START]);
                    break;
                case TOKENS:
                    tokens(state, &message[TOKEN_START]);
                    break;
                case WILD:
                    wild(state, &message[WILD_START]);
                    break;
            }
            free(message);
        }
    }
}

int can_buy_card(GameState* state, int* cardIndexs, int pointMin, int player) {
    int count = 0;
    int boardIndex = 0;
    long totalWild;
    Market* market = state->board.oldest;

    while (market != NULL) {
        totalWild = state->player.wildPile[player];
        for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
            if ((state->player.tokens[player][i] + 
                    state->player.discountList[player][i] +
                    totalWild) >= 
                    market->card->cost[i]) { // can purchase

                if ((state->player.tokens[player][i] + 
                        state->player.discountList[player][i]) < 
                        market->card->cost[i]) { // must use wild tokens
                    totalWild -= (market->card->cost[i] - 
                            (state->player.tokens[player][i] + 
                            state->player.discountList[player][i]));
                }               
            } else {
                break;
            }
            if (i == (MAX_TOKEN_COLOUR - 1)) { // can purchase
                if (market->card->points < pointMin) { //not above the pointMin
                    break;
                } else { // adding to the index
                    cardIndexs[count] = boardIndex;
                    count++;
                }
            }
        }
        boardIndex++;
        market = market->next;
    }
    return count;
}

void take_wild(void) {
    fprintf(stdout, "wild\n");
    fflush(stdout);
}

bool can_take_tokens(GameState* state) {
    int count = 0;

    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
        if (state->tokenPile.pile[i] > 0) {
            count++;
        }
    }

    if (count >= MIN_TOKENS) {
        return true;
    } else {
        return false;
    }
}

void take(long* tokens) {
    fprintf(stdout, "take%ld,%ld,%ld,%ld\n",
            tokens[PURPLE],
            tokens[BROWN],
            tokens[YELLOW],
            tokens[RED]);
    fflush(stdout);
}

void purchase(int cardNum, long* tokens, long wild) {
    fprintf(stdout, "purchase%d:%ld,%ld,%ld,%ld,%ld\n",
            cardNum,
            tokens[PURPLE],
            tokens[BROWN],
            tokens[YELLOW],
            tokens[RED],
            wild);
    fflush(stdout);
}

void load_tokens(GameState* state, int boardIndex, long* tokens, long* wild) {
    // assumes we can afford the card
    *wild = 0;
    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
        tokens[i] = 0;
        if ((state->player.tokens[THIS_PLAYER][i] + 
                state->player.discountList[THIS_PLAYER][i]) < 
                check_market_card(state, boardIndex)->cost[i]) { // need wild

            tokens[i] = (state->player.tokens[THIS_PLAYER][i]);
            *wild += ((check_market_card(state, boardIndex)->cost[i]) -
                    (state->player.tokens[THIS_PLAYER][i] + 
                    state->player.discountList[THIS_PLAYER][i]));
        } else { // dont need wild
            tokens[i] = ((check_market_card(state, boardIndex)->cost[i]) -
                    state->player.discountList[THIS_PLAYER][i]);
            if (tokens[i] < 0) { // more discounts than cost of the colour
                tokens[i] = 0;
            }
        }
    }
}

int find_highest_index(GameState* state, int* cardIndexs, int* canPurch) {
    // find highest point card or if there are more then 1
    long highest = 0;
    int tempIndexs[MAX_MARKETS];
    int tempCanPurch;

    for (int i = 0; i < *canPurch; i++) {       
        if (highest < 
                check_market_card(state, cardIndexs[i])->points) { // highest
            highest = 
                    (check_market_card(state, cardIndexs[i])->points);
            tempCanPurch = 1;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];
        } else if (highest == 
                check_market_card(state, cardIndexs[i])->points) { // another
            tempCanPurch++;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];
        }
    }
    *canPurch = tempCanPurch;
    for (int i = 0; i < tempCanPurch; i++) { // updates cardIndexs
        cardIndexs[i] = tempIndexs[i];
    }
    return tempCanPurch;
}

int find_lowest_cost(GameState* state, int* canPurch, int* cardIndexs) {
    int lowestCount = INT_MAX;
    int tempIndexs[MAX_MARKETS];
    int tempCanPurch;
    int count;
    Card* card;

    for (int i = 0; i < *canPurch; i++) {
        card = check_market_card(state, cardIndexs[i]);
        count = card_cost_count(state, card);
        
        if (count < lowestCount) { // lowest cost
            lowestCount = count;
            tempCanPurch = 1;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];         
        } else if (lowestCount == count) { // another lowest
            tempCanPurch++;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];
        }
    }
    *canPurch = tempCanPurch;
    for (int i = 0; i < tempCanPurch; i++) { // updates card indexes
        cardIndexs[i] = tempIndexs[i];
    }
    return tempCanPurch;
}

int find_highest_cost(GameState* state, int* canPurch, int* cardIndexs) {
    int highestCount = 0;
    int tempIndexs[MAX_MARKETS];
    int tempCanPurch;
    int count;
    Card* card;

    for (int i = 0; i < *canPurch; i++) { 
        card = check_market_card(state, cardIndexs[i]);
        count = card_cost_count(state, card);

        if (count > highestCount) { // highest cost
            highestCount = count;
            tempCanPurch = 1;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];         
        } else if (highestCount == count) { // another highest
            tempCanPurch++;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];
        }
        *canPurch = tempCanPurch;
        for (int i = 0; i < tempCanPurch; i++) { // updates card indexes
            cardIndexs[i] = tempIndexs[i];
        }
    }
    return tempCanPurch;
}

int highest_wild_cost(GameState* state, int* canPurch, int* cardIndexs) {
    long tokens[MAX_TOKEN_COLOUR];
    long wild;
    int tempIndexs[MAX_MARKETS];
    int tempCanPurch;
    int highestWild = 0;

    for (int i = 0; i < *canPurch; i++) {
        load_tokens(state, cardIndexs[i], tokens, &wild);

        if (wild > highestWild) { // highest wild cost
            highestWild = wild;
            tempCanPurch = 1;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];         
        } else if (highestWild == wild) { // another highest
            tempCanPurch++;
            tempIndexs[(tempCanPurch - 1)] = cardIndexs[i];
        }
        *canPurch = tempCanPurch;
        for (int i = 0; i < tempCanPurch; i++) { // updates card cost
            cardIndexs[i] = tempIndexs[i];
        }
    }
    return tempCanPurch;
}

////////////////////////////// Private Functions //////////////////////////////
//
static void end_player(GameState* state, int exitStatus) {
    char* program[] = {"shenzi", "banzai", "ed"};

    switch (exitStatus) {
        case GAME_OVER:
            break;
        case WRONG_NUM_ARGS:
            fprintf(stderr, "Usage: %s pcount myid\n", 
                    program[state->victoryPoints]);
            break;
        case INVALID_NUM_PLAYER:
            fprintf(stderr, "Invalid player count\n");
            break;
        case INVALID_ID:
            fprintf(stderr, "Invalid player ID\n");
            break;
        case COMMS_ERR:
            fprintf(stderr, "Communication Error\n");
            break;
    }
    fflush(stderr);

    free_player_card(state);
    free_board(state);
    exit(exitStatus);
}

//
static int parse_message(char* message) {
    int len = strlen(message);

    if (len == EOG) {
        if (is_eog(message)) {
            return END_OF_GAME;
        }
    } else if (len == DO_WHAT_MES) {
        if (is_dowhat(message)) {
            return DO_WHAT;
        }
    }
    if (len >= PURCHASED_MIN) {
        if (is_purchased(message)) {
            return PURCHASED;
        }
    } 
    if (len >= NEW_CARD_MIN) {
        if (is_newcard(message)) {
            return NEW_CARD;
        }
    }
    if (len >= TOOK_MIN) {
        if (is_took(message)) {
            return TOOK;
        }
    }
    if (len >= TOKENS_MIN) {
        if (is_tokens(message)) {
            return TOKENS;
        }
    }
    if (len >= WILD_MIN) {
        if (is_wild(message)) {
            return WILD;
        }
    }
    return NO_ACTION;
}

//
static bool is_eog(char* message) {
    if (message[0] == 'e' && message[1] == 'o' && message[2] == 'g') {
        return true;
    } else {
        return false;
    }
}

//
static bool is_dowhat(char* message) {
    if (message[0] == 'd' && message[1] == 'o' && message[2] == 'w' &&
            message[3] == 'h' && message[4] == 'a' && message[5] == 't') {
        return true;
    } else {
        return false;
    }
}

//
static bool is_purchased(char* message) {
    if (message[0] == 'p' && message[1] == 'u' && message[2] == 'r' &&
            message[3] == 'c' && message[4] == 'h' && message[5] == 'a' &&
            message[6] == 's' && message[7] == 'e' && message[8] == 'd') {
        return true;
    } else {
        return false;
    }
}

//
static bool is_newcard(char* message) {
    if (message[0] == 'n' && message[1] == 'e' && message[2] == 'w' &&
            message[3] == 'c' && message[4] == 'a' && message[5] == 'r' &&
            message[6] == 'd') {
        return true;
    } else {
        return false;
    }
}

//
static bool is_took(char* message) {
    if (message[0] == 't' && message[1] == 'o' && message[2] == 'o' &&
            message[3] == 'k') {
        return true;
    } else {
        return false;
    }
}

//
static bool is_tokens(char* message) {
    if (message[0] == 't' && message[1] == 'o' && message[2] == 'k' &&
            message[3] == 'e' && message[4] == 'n' && message[5] == 's') {
        return true;
    } else {
        return false;
    }
}

//
static bool is_wild(char* message) {
    if (message[0] == 'w' && message[1] == 'i' && message[2] == 'l' &&
            message[3] == 'd') {
        return true;
    } else {
        return false;
    } 
}

//
static void end_of_game(GameState* state) {
    char* winners = "Game over. Winners are ";
    print_winners(state, winners, stderr);
    end_player(state, GAME_OVER);
}

//
static void purchased(GameState* state, char* message) {
    Card* card;
    int player;
    int boardIndex;
    char* mesIndex = &message[2];
    long tokens[TOKENS_AND_WILD];

    if (!player_parse(state, message)) { // invalid player
        end_player(state, COMMS_ERR);
    }
    player = player_char_to_int(message[0]);

    if (!is_valid_purchase(tokens, mesIndex, &boardIndex)) { // invalid
        end_player(state, COMMS_ERR);
    }

    if ((card = purchase_card(state, boardIndex)) == NULL) { // invalid index
        end_player(state, COMMS_ERR);
    } // card now removed from board

    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) { // updates sate
        state->player.tokens[player][i] -= tokens[i];
        state->tokenPile.pile[i] += tokens[i];
    }   
    state->player.wildPile[player] -= tokens[WILD_INDEX];
    add_discount(state, card->discount, player);
    state->player.scoreCard[player] += card->points;    
    
    free(card);

    // send state to all
    print_state(state, stderr);
}

//
static bool player_parse(GameState* state, char* message) {
    if ((message[0] < A_CHAR) || 
            (message[0] > (A_CHAR + state->player.count - 1)) ||
            (message[1] != ':')) {
        return false;
    } else {
        return true;
    }
}

//
static void new_card(GameState* state, char* message) {
    Card* card = malloc(sizeof(Card) * 1);
    if (unwrap_card(message, card)) {
        if (!add_to_board(state, card)) {
            free(card);
        }
        print_state(state, stderr);
    } else {
        free(card);
        end_player(state, COMMS_ERR);
    }
}

//
static void took(GameState* state, char* message) {
    if (!player_parse(state, message)) {
        end_player(state, COMMS_ERR);
    }

    int player = player_char_to_int(message[0]);
    char* mesIndex = &message[2];
    long tokens[MAX_TOKEN_COLOUR];

    if(!is_valid_take(tokens, mesIndex)) {
        end_player(state, COMMS_ERR);
    }

    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) { // updates state
        state->player.tokens[player][i] += tokens[i];
        state->tokenPile.pile[i] -= tokens[i];
    }

    print_state(state, stderr);
}

//
static void tokens(GameState* state, char* message) {
    int maxTokens;

    if ((maxTokens = is_str_pos_number(message)) == INVALID) {
        end_player(state, COMMS_ERR);
    } else {
        for (int colour = 0; colour < MAX_TOKEN_COLOUR; colour++) {
            // state updates
            state->tokenPile.pile[colour] = maxTokens;
        }
        state->tokenPile.maxTokens = maxTokens;
        print_state(state, stderr);
    }
}

//
static void wild(GameState* state, char* message) {

    if ((message[0] < A_CHAR) || 
            (message[0] > (A_CHAR + state->player.count - 1))) { 
        // invalid player name
        end_player(state, COMMS_ERR);
    }
    // updates state
    int player = player_char_to_int(message[0]);
    state->player.wildPile[player] += 1;    
    print_state(state, stderr);
}

//
static void print_state(GameState* state, FILE* stream) {
    print_board(state, stream);
    print_player_state(state, stream);
}

//
static void print_player_state(GameState* state, FILE* stream) {
    for (int player = 0; player < state->player.count; player++) {
        fprintf(stream, 
                "Player %c:%ld:Discounts=%d,%d,%d,%d:"
                "Tokens=%ld,%ld,%ld,%ld,%ld\n",
                player_int_to_char(player),
                state->player.scoreCard[player],
                state->player.discountList[player][PURPLE],
                state->player.discountList[player][BROWN],
                state->player.discountList[player][YELLOW],
                state->player.discountList[player][RED],
                state->player.tokens[player][PURPLE],
                state->player.tokens[player][BROWN],
                state->player.tokens[player][YELLOW],
                state->player.tokens[player][RED],
                state->player.wildPile[player]);
        fflush(stream);
    }
}

//
static int card_cost_count(GameState* state, Card* card) {
    int count = 0;  
    
    for (int i = 0; i < MAX_TOKEN_COLOUR; i++) {
        count += card->cost[i];
        if (card->cost[i] > 0) {
            count -= state->player.discountList[THIS_PLAYER][i];
            if (count < 0) {
                count = 0;
            }
        }           
    }
    return count;
}

//
static void free_player_card(GameState* state) {
    Market* market = state->board.oldest;
    while (market != NULL) {
        free(market->card);
        market = market->next;
    }
}