#include "server.h"

//====== GLOBAL VARIABLES ====== 
BoardState *curr; 
BoardState *proposed; 
LetterBag *bag; 

Player *players[MAX_PLAYERS];
int socketfd;

int player_count = 0;  
char *letters = "AAAAAAAAABBCCDDDDEEEEEEEEEEEEEEEEEEEEFFGGGHHIIIIIIIIIIIIJJKKLLLLLMMMNNNNNNNNOOOOOOOOPPQRRRRRRRRRSSSSSSSSTTTTTTTTUUUUVVWWXYYZ"; //maybe modify this based on number of players

//====== MAIN ====== 
int main(int argc, char* argv[]) { 
	//====== INITIALIZATION ======
	signal(SIGINT, sighandler);
	generate_table("server/words.txt");

	memset(players, 0, sizeof(players));
	curr = calloc(sizeof(BoardState), 1);
	proposed = calloc(sizeof(BoardState), 1);  
	
	bag = calloc(sizeof(LetterBag), 1); 
	strcpy(bag->letters, letters);
	bag->num_letters = strlen(bag->letters);
	
	// wait for players to connect
	socketfd = initialize_socket(PORT); //maybe change this to a command line argument later

	printf("Waiting for players to join...\n");
	while(player_count < MAX_PLAYERS) { //waits for all players to join
		//accept player
		Player *new_player = accept_player(socketfd);
		if(new_player == NULL) {
			printf("Error accepting player\n");
			return 1;
		}
		players[player_count] = new_player;
		write(new_player->socket, &player_count, sizeof(int)); //send player number to player
		player_count++;
	}

	printf("Game has started\n");
	game_loop();
	return 0;
}

//====== GAME LOOP ====== 
void game_loop() {
	GameState state; 
	int score;
	state.turn = 0; 
	state.connected_players = player_count; 
	for(int i = 0; i < player_count; i++) {
		Player *current_player = &(state.players[i]); 
		memcpy(current_player->name, players[i]->name, sizeof(players[i]->name));
		current_player->score = 0; 
		memset(current_player->hand, 0, sizeof(current_player->hand));
		draw_tiles(current_player);
	}
	usleep(500);
	send_board();
	usleep(500);
	send_state(state); 
	while(1) {   
		int turn = state.turn % player_count;
		read(players[turn]->socket, proposed, sizeof(BoardState));
		score = authenticate_move(proposed); 
		write(players[turn]->socket, &score, sizeof(int)); //send score to player
		if(score > 0) { //if move is valid
			//update board
			memcpy(curr, proposed, sizeof(BoardState));
			send_board();
			
			//give player the points 
			players[turn]->score += score;  

			// edit state, then send to all players
			state.players[turn].score = players[turn]->score;
			draw_tiles(&(state.players[turn])); //redraw tiles after move
			state.turn++;
			usleep(500);
			send_state(state);
		}
	}
	
}

void send_state(GameState state) {
	//send state to all players 
	for(int i = 0; i < MAX_PLAYERS; i++) {
		if(players[i] != NULL) {
			write(players[i]->socket, &state, sizeof(GameState));
		}
	}
}

void send_board() {
	//send board to all players 
	for(int i = 0; i < MAX_PLAYERS; i++) {
		if(players[i] != NULL) {
			write(players[i]->socket, curr, sizeof(BoardState));
		}
	}
}


//====== FUNCTIONS ======
// connection functions:
int initialize_socket(char *port)
{
	struct sockaddr_in server_address;
	int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == -1)
	{
		printf("socket creation failed...\n");
		printf("Error: %s\n", strerror(errno));
		return -1;
	}

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(atoi(port)); // port number

	if ((bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address))) != 0)
	{
		printf("socket bind failed...\n");
		printf("Error: %s\n", strerror(errno));
		return -1;
	}
	
	if ((listen(listen_socket, MAX_PLAYERS)) != 0)
	{
		printf("Listen failed...\n");
		printf("Error: %s\n", strerror(errno));
		return -1;
	}

	return listen_socket;
}

int accept_connection(int socket)
{
	socklen_t sock_size;
	struct sockaddr_storage client_address;
	sock_size = sizeof(client_address);

	return accept(socket, (struct sockaddr *)&client_address, &sock_size);
}

struct player *accept_player(int socket)
{
	struct player *new_player = malloc(sizeof(Player)); 
	memset(new_player, 0, sizeof(Player)); //clears the memory

	new_player->socket = accept_connection(socket); 
	new_player->score = 0; 

	if (new_player->socket == -1)
	{
		printf("Error accepting connection\n");
		printf("Error: %s\n", strerror(errno));
		return NULL;
	}   

	read(new_player->socket, new_player->name, sizeof(new_player->name)); 
	printf("Player %s connected\n", new_player->name); 
	return new_player;
}


void disconnect_all()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (players[i] != NULL)
		{
			close(players[i]->socket);
			free(players[i]); 
		}
	}
}

//game functions:
void load_game(char* file_name) {
	printf("Loading game from file %s\n", file_name);
	// load game from file
	int fd = open(file_name, O_RDONLY);
	if (fd == -1)
	{
		printf("Error opening file %s\n", file_name);
		printf("Error: %s\n", strerror(errno));
	}
	read(fd, curr, sizeof(BoardState));
	close(fd);
}

int generate_table(char* file) { 
	ENTRY item;
	if(!hcreate(DICT_SIZE)) {
		printf("not enough space for the entire scrabble dict, sorry bub\n");
		return -1;
	}
	char word[SMALL_BUFFER];
	FILE *f = fopen(file, "r");
	while(fgets(word, SMALL_BUFFER, f)) {
		item.key = strdup(word);
		if(!hsearch(item, ENTER)) {
			fclose(f);
			return -1;
		}
	}
	fclose(f);
	return 0;
}

int search(char* entry) { 
	char word[SMALL_BUFFER];
	strcpy(word, entry);
	strcat(word, "\n");
	for(int i = 0; i < strlen(word); i++) {
		word[i] = toupper(word[i]);
	}
	ENTRY item;
	item.key = word;
	if(hsearch(item, FIND)) {
		return 1;
	}
	return 0;
} 

static void sighandler(int signo){
	if (signo == SIGINT){
		usleep(100);
		save_game();
		disconnect_all();
		close(socketfd);
		printf("Exiting...\n");
		exit(0);
	}
}

int save_game() {
	printf("Do you want to save the game? (y/n)\n");
	char c = getchar();
	if(c == 'y') {
		printf("Enter filename to save to: ");
		char buffer[SMALL_BUFFER];
		char file[SMALL_BUFFER] = "output/";
		fgets(buffer, SMALL_BUFFER, stdin);
		strcat(file, buffer);
		int fd = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0644); 
		if (fd == -1) {
			printf("Error opening file %s\n", file);	
			printf("Error: %s\n", strerror(errno)); 
			return -1; 
		} 
		write(fd, curr, sizeof(BoardState)); 
		close(fd); 
		printf("Saved to %s\n", file);
	}
	else {
		printf("Game not saved\n");
	}
	return 0;
}

int score_letter(char letter) {
	if(letter == 'A' || letter == 'E' || letter == 'I' || letter == 'O' || letter == 'U' || letter == 'L' || letter == 'N' || letter == 'S' || letter == 'T' || letter == 'R') {
		return 1;
	}
	if(letter == 'D' || letter == 'G') {
		return 2;
	}
	if(letter == 'B' || letter == 'C' || letter == 'M' || letter == 'P') {
		return 3;
	}
	if(letter == 'F' || letter == 'H' || letter == 'V' || letter == 'W' || letter == 'Y') {
		return 4;
	}
	if(letter == 'K') {
		return 5;
	}
	if(letter == 'J' || letter == 'X') {
		return 8;
	}
	if(letter == 'Q' || letter == 'Z') {
		return 10;
	}
}

int score_word(char *str) {
	int res = 0;
	for(int i = 0; i < strlen(str); i++) {
		res+=score_letter(str[i]);
	}
	return res;
}

int authenticate_move(BoardState *board) {
	int score = 0;
	char* word = calloc(SMALL_BUFFER, sizeof(char));
	char* sideWord = calloc(SMALL_BUFFER, sizeof(char));
	int k = 0;

	for(int i = 0; i < BOARD_LEN; i++) {
		for(int j = 0; j < BOARD_LEN; j++) {
			if(board->board[i][j] != curr->board[i][j]){
				//they are different here
				if (j < BOARD_LEN-1 && board->board[i][j+1] != curr->board[i][j+1]) {
					//horizontal word, increment j until you hit a space / end of board
					while (j > 0 && board->board[i][j-1] != 0) {
						j--; //move j to beginning of word
					}
					while (j < BOARD_LEN && board->board[i][j] != 0) { 
						word[strlen(word)] = toupper(board->board[i][j]); //construct word
						if (board->board[i][j] != curr->board[i][j]) {
							sideWord = calloc(SMALL_BUFFER, sizeof(char));
							k = i;
							while (k > 0 && board->board[k-1][j] != 0) { //prepare to construct side word
								k--;
							}
							while (k < BOARD_LEN && board->board[k][j] != 0) {
								sideWord[strlen(sideWord)] = toupper(board->board[k][j]); //construct side word
								k++;
							}
							if (strlen(sideWord) > 1) {
								sideWord[strlen(sideWord)] = '\0';
								if (search(sideWord) == 0){
									return 0;
								}
								score += score_word(sideWord);
							}
						}
						j++;
					}
					if (strlen(word) > 1) {
						word[strlen(word)] = '\0';
						if (search(word) == 0){
							return 0;
						}
						score += score_word(word);
					}
					return score;
				} else{
					//vertical word, increment i until you hit a space / end of board
					while (i > 0 && board->board[i-1][j] != 0) {
						i--; //move i to beginning of word
					}
					while (i < BOARD_LEN && board->board[i][j] != 0) {
						word[strlen(word)] = toupper(board->board[i][j]); //construct word
						if (board->board[i][j] != curr->board[i][j]) {
							sideWord = calloc(SMALL_BUFFER, sizeof(char));
							k = j;
							while (k > 0 && board->board[i][k-1] != 0) { //prepare to construct side word
								k--;
							}
							while (k < BOARD_LEN && board->board[i][k] != 0) {
								sideWord[strlen(sideWord)] = toupper(board->board[i][k]); //construct side word
								k++;
							}
							if (strlen(sideWord) > 1) {
								sideWord[strlen(sideWord)] = '\0';
								if (search(sideWord) == 0) {
									return 0; //side word not in dictionary
								}
								score += score_word(sideWord);
							}
						}
						i++;
					}
					if (strlen(word) > 1) {
						word[strlen(word)] = '\0';
						if (search(word) == 0) {
							return 0; //word not in dictionary
						}
						score += score_word(word);
					}
					return score;
				}
			}
		}
	}
	return 0; //bro u literally did not submit anything :skull:
} 

void draw_tiles(Player *p) {
	for(int i = 0; i < 7 && bag->num_letters > 0; i++) {
		if (p->hand[i] != 0) {
			continue;
		}

		do {
			int index = rand() % bag->num_letters; 
			p->hand[i] = bag->letters[index]; 
			bag->letters[index] = bag->letters[bag->num_letters-1]; 
			bag->letters[bag->num_letters-1] = 0; 
		} 
		while(p->hand[i] == 0); 
		bag->num_letters--;
	}
}

//test function 
void print_hand(Player *current)
{
	printf("[ ");
	for (int i = 0; i < 7; i++)
	{
		if (current->hand[i] == 0)
		{
			printf("_ ");
		}
		else
		{
			printf("%c ", current->hand[i]);
		}
		if (i != 6)
		{
			printf(", ");
		}
	}
	printf("]\n");
}