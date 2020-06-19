/* lib.h
 *
 * Author: Michael Bossner
 *
 * lib.h header file for lib.c Contains many useful data structures
 */

#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

/////////////////////////////////// Defines ///////////////////////////////////

#define MAX_PLAYERS 26
#define FOREVER for (;;)
#define ZERO 48
#define NINE 57
#define READ_WRITE 2
#define ERROR_FLAGS 2
#define READ 0
#define WRITE 1
#define INVALID -1
#define FAIL 0
#define VALID 1

/* Indexes for token storage as well as max number of tokens */
enum TokenIndex {
    PURPLE,
    BROWN,
    YELLOW,
    RED,
    MAX_TOKEN_COLOUR
};

/* Container for the games token pile */
typedef struct {
    int maxTokens;
    long pile[MAX_TOKEN_COLOUR]; // games token pile
} TokenPile;

/* A Card contains all information stored in a card */
typedef struct {
    char discount; // Card colour
    long points; // victory points card provides
    long cost[MAX_TOKEN_COLOUR]; // cost to purchase the card
} Card;

/* A Deck stores all cards in the deck and deck information */
typedef struct {
    int size; // size of the deck
    int deckIndex; // current card the deck is up to
    Card** cardPile; // pile of cards in the deck
} Deck;

/* A market is set up in empty board spots and sells a single card
 * it also contains information about the markets directly next to it on
 * either side */
typedef struct Market Market;
struct Market {
    Card* card; // card that the market is selling

    Market* next; // a pointer to the market to the right of this market
    Market* prev; // a pointer to the market on the left of this market
};

/* A Board stores locations of the oldest (left most) market and youngest
 * (right most) markets on the board. The oldest can also be the youngest 
 * if there is only 1 market set up */
typedef struct {
    Market* oldest; // Right most market set up
    Market* youngest; // Left most market set up
} Board;

/* The Player contains all player related information */
typedef struct {
    int count; // Amount of players in the game
    long scoreCard[MAX_PLAYERS]; // Current player score for all players
    long tokens[MAX_PLAYERS][MAX_TOKEN_COLOUR]; // All player tokens
    long wildPile[MAX_PLAYERS]; // list of all wild tokens player have
    // All discounts that each player has
    int discountList[MAX_PLAYERS][MAX_TOKEN_COLOUR];
    pid_t pidList[MAX_PLAYERS]; // each players ID
    // list of each players communication streams
    FILE* commsList[MAX_PLAYERS][READ_WRITE];
} Player;

/* The GameState contains all information needed to keep track of the game */
typedef struct {    
    TokenPile tokenPile; // See TokenPile struct
    Deck deck; // See Deck struct
    Board board; // see Board struct
    Player player; // see Player struct
    int victoryPoints; // points needed to end the game 
    int currentPlayer; // index of the player who's turn it is
} GameState;

/* A SigStore contains all signal handling information */
typedef struct {
    int index; // Num of dead children. Used to index into status and children
    pid_t children[MAX_PLAYERS]; // list of dead children IDs
    int status[MAX_PLAYERS]; // list of dead children statuses
    bool childSignaled; // Flag to indicate a child was signaled
    bool sigIntCaught; // Flag to indicate a SIGINT was caught
    bool sigPipeCaught; // Flag to indicate a SIGPIPE was caught
    bool badStart; // Flag to indicate a child died from a bad start
} SigStore;

//////////////////////// Private Functions Prototypes /////////////////////////

/*
 * Tells you if the number of arguments is valid
 *
 * argc: number of arguments received
 *
 * minArgs: Minimum amount of arguments allowed
 *
 * maxArgs: Maximum amount of arguments allowed
 *
 * returns: Returns true false (argc < minAgrs) || (argc > maxArgs) else true
 */
bool is_num_args_valid(int argc, int minArgs, int maxArgs);

/*
 * Converts a string into a positive integer
 *
 * string: string to be converted
 *
 * return: Returns -1 if the number is negative or the string is not valid
 *         Returns the converted number if successful
 */
int is_str_pos_number(char* string);

/*
 * converts a players index into the players name
 * example: player index 0 would be converted to player 'A'
 *
 * player: index of player to be converted
 *
 * return: returns the players name
 */
char player_int_to_char(int player);

/*
 * converts a players name into the players index
 * example: player 'A' would be converted to player 0
 *
 * player: Name of the player to be converted
 *
 * return: index of the player
 */
int player_char_to_int(char player);

#endif