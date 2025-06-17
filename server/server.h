/* ===== WHAT WE CAME TO A CONSENSUS ON =====
*word detection on client
*send words to server
*server checks if words are valid
*if yes, send game state to both clients
*if no send rejection to client that submited words
*server must be either multi-threaded or forking to handle multiple clients 
*server must be able to save game state to file and load game state from file 
*server must be able to send game state to clients 
*server must be able to receive game state from clients 
*/

#ifndef SERVER_H
#define SERVER_H 

#include "../common.h"

//====== REQUIRED LIBRARIES ======
#include <ctype.h>
#include <search.h> 
#include <math.h>

//====== DEFINES ======
#define DICT_SIZE 178691 //size of dictionary
#define SMALL_BUFFER 256

/**
 * Literally sighandler what do u want
 */
static void sighandler(int signo); 

//====== CONNECTION FUNCTIONS ======

/**
 * Initializes the socket for the server to listen on
 * @warning If the port is not forwarded, the server will default to localhost
 * @param port The port number to listen on
 * @return The socket file descriptor, or -1 if an error occurred
 */
int initialize_socket(char *port);

/**
 * Accepts a connection to the server.
 *
 * @param socket The socket of the server
 * @returns The socket of the new connection
 */
int accept_connection(int socket);

/**
 * Accepts a new player and returns a pointer to a player struct with the player's name and socket
 * @param socket the socket to accept a player from
 * @return a pointer to a player struct with the player's name and socket
 */
Player *accept_player(int socket);

/**
 * Checks the connection to the player
 *
 * Returns:
 * 0: if no acknowledgement is recieved.
 * 1: Acknowledgement is recieved.
 */
int check_connection();

/**
 * Disconnects all players from the server.
 */
void disconnect_all();

//====== GAME FUNCTIONS ======

/**
 * Authenticates the proposed game-state by the player. 
 * 
 * Returns: 
 * -1 if the game-state is invalid 
 * Points given for the move if it is valid
 */
int authenticate_move(BoardState *board);

/**
 * Calculates the score of a letter
 * 
 * @param letter the letter to score
 * @returns the score of the letter
 */
int score_letter(char letter);

/**
 * Calculates the score of a word
 * 
 * @param word the word to score
 * @returns the score of the word
 */
int score_word(char *word);

/**
 * Creates a hash table from a dictionary file
 * 
 * @param file the name of the file to read from
 * @returns 0 on success, -1 on failure
 */
int generate_table(char *file); 

/**
 * Checks if a word is in the dictionary 
 * 
 * @param entry the word to check 
 * @returns 1 if the word is in the dictionary, 0 if not
 */
int search(char *entry);

/**
 * Saves the current game state to a file
 *
 * @return 0 on success, -1 on failure
 */
int save_game(); 

/**
 * Loads the game state from a file.
 * 
 * @param file_name the name of the file to read from 
*/
void load_game(char* file_name); 

/**
 * Game loop for the server
*/
void game_loop();

/**
 * Sends current scrabble board to all players
*/
void send_board();

/**
 * Sends current game state to all players
 *
 * @param state the current state object
*/
void send_state(GameState state);

void draw_tiles(Player *p);

void print_hand(Player *current);

#endif
