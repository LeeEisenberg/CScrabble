#ifndef CLIENT_H
#define CLIENT_H

#include "../common.h"

//====== REQUIRED LIBRARIES ======
#include <ctype.h>
#include <curses.h>
#include <search.h>

//====== DEFINES ====== 


//====== Functions to implement ====== 

/**
 * Connects to the server. 
 * 
 * @param host: the host IP
 * @param port: the port (socket) to connect to 
 * Returns:
 * -1 on ERROR 
 * 1 on SUCCESS 
 * 0 if game is already in session
*/
int connect_to_server(char *host, char *port); 

/**
 * Proposes a new game-state based on the player's move. 
 * 
 * @param change: the change to propose
 * Returns: 
 * 0 if proposal is accepted.
 * -1 if proposal is rejected. 
*/ 
int propose(BoardState *change);  

/**
 * Wrapper for print_board. 
 * Here just n case we wanted to use SDL for graphics or smthn. 
 * 
 * @param board: the board to display 
*/
void display(BoardState *board);

/**
 * Clears the previous board and displays the game-state to the player.
 *
 * @param board: the board to display
 */
void print_board();

/**
 * literally sighandler what do u want
 */
static void sighandler(int signo);
void input(WINDOW *w);
char *toUpper(char *str); 


void game_loop(); 
void print_players();
void print_hand();
int inHand(char ch); 
void removeTile(char ch);

#endif 