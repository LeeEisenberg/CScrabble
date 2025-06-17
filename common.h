#ifndef COMMON_H 
#define COMMON_H 

//=====REQUIRED LIBRARIES=====
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h>

#include <netdb.h> 
#include <sys/types.h> 
#include <sys/socket.h>

#include <signal.h> 
#include <errno.h> 
//====== DEFINES ======
#define PORT "25595"
#define BOARD_LEN 15
#define NAME_LEN 20
#define MAX_PLAYERS 2 // max number of players, increase to 10 later

//====== STRUCTS ======

/**
 * char **board stores the current game-state in characters
 * char **specials stores the special spaces on the board
 * int round stores the current round
 * int turn stores the number of the player whose turn it is
 * int *player_scores stores the scores of each player
 */
struct board_state
{
    char board[BOARD_LEN][BOARD_LEN];
    char specials[BOARD_LEN][BOARD_LEN];
};

typedef struct board_state BoardState; 

struct player
{
    int socket;
    int score;
    char name[NAME_LEN + 1];
    char hand[7];
};

typedef struct player Player;

struct game_state
{
    int turn;
    int connected_players;
    Player players[MAX_PLAYERS];
}; 

typedef struct game_state GameState; 

struct letter_bag {
    char letters[256];
    int num_letters;
}; 

typedef struct letter_bag LetterBag;

#endif