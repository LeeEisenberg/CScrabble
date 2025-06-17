#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/*
Prompts user to choose between join, host, and load game
join: execvp client.c
host: execvp server.c
load: execvp server.c with arguments to load game from user-specified file
*/ 
#define PORT "25595"

int main(int argc, char *argv[]) {
    char c[5];
    printf("Welcome to Scrabble!\n");
    printf("Please choose an option:\n");
    printf("1. Join a game\n");
    printf("2. Host a game\n");
    printf("3. Load a game\n");
    fgets(c, sizeof(c), stdin);

    int choice = atoi(c);
    if(choice == 1) {
        printf("Joining a game...\n");
        printf("Please enter the address of the host:\n");
        char host[100];
        fgets(host, sizeof(host), stdin);
        host[strlen(host) - 1] = '\0'; //remove newline
        printf("Using port 25595\n");
        char *args[] = {"output/client.out", host, PORT, NULL};
        execvp(args[0], args); 
    } 
    else if(choice == 2) {
        printf("Hosting a game...\n"); 
        if(fork()) {
            printf("Starting server...\n");
            char *args[] = {"output/server.out", NULL};
            execvp(args[0], args);
        }
        else {
            sleep(1);
            printf("Joining own server...\n");
            char *args[] = {"output/client.out", "127.0.0.1", PORT, NULL}; //add args for host and port
            execvp(args[0], args);
        }
    }
    else if(choice == 3) { 
        char filename[100];
        printf("Please enter the name of the file you would like to load:\n");
        fgets(filename, sizeof(filename), stdin); 
        filename[strlen(filename) - 1] = '\0'; //remove newline  
        
        printf("Loading a game...\n"); 
        if (fork()) {
            printf("Starting server...\n");
            char *args[] = {"output/server.out", strdup(filename), NULL};
            execvp(args[0], args);
        }
        else {
            printf("Joining own server...\n");
            char *args[] = {"output/client.out", "127.0.0.1", PORT, NULL}; //add args for host and port
            execvp(args[0], args);
        }
    }
    else {
        printf("Invalid choice. Please try again.\n");
    } 
    return 0;
} 

