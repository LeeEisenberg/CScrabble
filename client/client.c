#include "client.h"

#define isLetter(c) (c >= 'A' && c <= 'Z') 

//====== GLOBAL VARIABLES ====== 
int sockfd; 
struct addrinfo *servinfo; 
BoardState *curr; 
BoardState *proposal; 

GameState *state;
GameState *prev_state; 
int player_num;
int coords[2] = {0, 0};
int score = 0;

//====== MAIN ======
int main(int argc, char* argv[]){
    signal(SIGINT, sighandler);
    printf("Welcome to Scrabble!\n");
    
    if (argc != 3)
    {
        printf("Usage: ./client <host> <port>\n");
        return 1;
    }

    sockfd = connect_to_server(argv[1], argv[2]); //I add the servinfo here so we can use freeaddrinfo later 
    if (sockfd == -1)
    {
        printf("Connection failed\n");
        return 1;
    } 
    printf("Connection successful\n");

    //wait for game to start 
    printf("Waiting for game to start...\n"); 
    sleep(1);   

    game_loop(); 

    //program ends 
    freeaddrinfo(servinfo); 
    close(sockfd); 
    return 0; 
}

void game_loop() {
    WINDOW *w = initscr(); //initialize ncurses window

    curr = malloc(sizeof(BoardState));
    read(sockfd, curr, sizeof(BoardState)); // read board state from server
    
    proposal = malloc(sizeof(BoardState));
    BoardState *toprint = malloc(sizeof(BoardState)); //waste of memory but this was Zawad's idea and I'm too lazy to change it
    memcpy(proposal, curr, sizeof(BoardState)); 

    state = malloc(sizeof(GameState)); 
    memset(state, 0, sizeof(GameState)); 
    read(sockfd, state, sizeof(GameState)); //read game state from server 
    
    prev_state = malloc(sizeof(GameState)); 
    memset(prev_state, 0, sizeof(GameState)); 
    memcpy(prev_state, state, sizeof(GameState)); 

    print_hand(); 

    for(int i = 0; i < state->connected_players; i++) {
        state->players[i].score = 0; //glitch with read 
    }

    while(1) {
        int temp = state->turn % state->connected_players;
        if(temp == player_num) { //if it is our turn 
            while(score <= 0) {
                input(w);
                memcpy(toprint, proposal, sizeof(BoardState)); 
                toprint->board[coords[0]][coords[1]] = '|';
                clear();
                printw("It is your turn!\n"); //makes sure this is always on top
                display(toprint);
            }
            clear();
            score = 0;
            read(sockfd, curr, sizeof(BoardState));
            read(sockfd, state, sizeof(GameState));
        }
        else {
            clear();
            printw("Waiting for player %s to make a move...\n", state->players[temp].name); 
            display(curr);
            
            //wait to read from server once moves are being made
            read(sockfd, curr, sizeof(BoardState)); //read board state from server 
            clear();
            printw("Player %s made a move!\n", state->players[temp].name);  
            read(sockfd, state, sizeof(GameState)); //read game state from server
            display(curr); //display board state after changes
            memcpy(proposal, curr, sizeof(BoardState));
            memcpy(prev_state, state, sizeof(GameState));
        }
    }
}

//====== FUNCTIONS ====== 
int connect_to_server(char *host, char *port) { //I add the servinfo here so we can use freeaddrinfo later
    struct addrinfo hints; 
    memset(&hints, 0, sizeof(hints)); 
    
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 

    if (getaddrinfo(host, port, &hints, &servinfo) != 0) {
        perror("getaddrinfo");
        return -1;
    }

    int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // allow reuse of port 

    //prompt for name
    char name[NAME_LEN + 1]; 
    usleep(100000); //wait for server to start
    printf("Enter your name: "); 
    fgets(name, 20, stdin); 
    name[strlen(name) - 1] = '\0'; 

    //connect to server
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        perror("connect");
        return -1;
    } 

    //server expects name to be sent first
    write(sockfd, name, sizeof(name)); //write name to server 
    read(sockfd, &player_num, sizeof(player_num)); //read player number from server
    return sockfd; //might have to freeaddrinfo somewhere idk 
} 

static void sighandler(int signo){
    if (signo == SIGINT){
        clear();
        endwin();
        printf("Exiting...\n");
        close(sockfd);
        exit(0);
    }
}

void input(WINDOW *w) {
	keypad(w, true);
    int ch = wgetch(w);
	switch(ch) { //perish in the [REDACTED] istab
		case KEY_UP: 
			if(--coords[0] < 0) {
				coords[0] = BOARD_LEN - 1;
			}
			break; 
		case KEY_DOWN:
			if (++coords[0] > BOARD_LEN - 1) {
				coords[0] = 0;
			}
			break; 
		case KEY_RIGHT:
			if (++coords[1] > BOARD_LEN - 1) {
				coords[1] = 0;
			}
			break; 
		case KEY_LEFT:
			if (--coords[1] < 0) {
				coords[1] = BOARD_LEN - 1;
			}
			break; 
        case KEY_BACKSPACE:
            memcpy(proposal, curr, sizeof(BoardState));
            memcpy(state, prev_state, sizeof(GameState));
            break; 
        case 10: //enter
            write(sockfd, proposal, sizeof(BoardState));
            read(sockfd, &score, sizeof(int)); 
            break;
        default:
            ch = (char) toupper((int) ch);
			if(isLetter(ch) && curr->board[coords[0]][coords[1]] == 0 && inHand(ch)) { //is it a letter and is the space empty?
				proposal->board[coords[0]][coords[1]] = ch;
                removeTile(ch);
			}
			break; 
	}
}

void display(BoardState *board)
{
    print_players(); 
    print_board(board);
    print_hand();
} 


void print_board(BoardState *board) {
	for(int i = 0; i < BOARD_LEN; i++) {
		for(int j = 0; j < BOARD_LEN; j++) {
			if(board->board[i][j]){
				printw("%c ", board->board[i][j]);
			}
			else {
				printw("_ ");
			}
		}
		printw("\n");
	}
    refresh();
}  

void print_players() {
    printw("\nPlayers: "); 
    for(int i = 0; i < state->connected_players; i++) { 
        printw("%s : %d", state->players[i].name, state->players[i].score); 
        if(i != state->connected_players - 1) {
            printw(" | ");
        }
    } 
    printw("\n\n");
} 

void print_hand() {
    printw("\n\nYour hand: "); 
    Player *current = &(state->players[player_num]);
    printw("[ ");
    for(int i = 0; i < 7; i++) {  
        if (current->hand[i] == 0) {
            printw("_ ");
        } 
        else {
            printw("%c ", current->hand[i]); 
        } 
        if (i != 6) {
            printw(", ");
        }
    }
    printw("]");
    printw("\n\n");
    refresh();
} 

int inHand(char ch) {
    Player *current = &state->players[player_num];
    for(int i = 0; i < 7; i++) {
        if (current->hand[i] == ch) {
            return 1;
        }
    }
    return 0;
} 

void removeTile(char ch) {
    Player *current = &state->players[player_num];
    for(int i = 0; i < 7; i++) {
        if (current->hand[i] == ch) {
            current->hand[i] = 0;
            return;
        }
    }
}