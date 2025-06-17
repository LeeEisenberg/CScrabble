#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h> 
#include <arpa/inet.h>


// First way to initialize socket 

int initialize_socket(char *port) // memory leaks here
{
    struct addrinfo hints, *results;
    memset(&hints, '\0', sizeof(hints)); // make sure the struct is empty

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;           // TCP socket
    hints.ai_flags = AI_PASSIVE;               // only needed on server

    char *ip = NULL;
    getaddrinfo(ip, port, &hints, &results); // get address info

    // create socket
    int listen_socket = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
    if(listen_socket == -1) {
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    } 
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // allow reuse of port
    bind(listen_socket, results->ai_addr, results->ai_addrlen);
    listen(listen_socket, 3); // 10 clients can wait to be processed

    return listen_socket;
}

// Second way to initialize socket

/*
int initialize_socket(char *port) {
    struct sockaddr_in server_address; 
    int listen_socket = socket(AF_INET, SOCK_STREAM, 0); 
    if (listen_socket == -1) {
        printf("socket creation failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully created..\n"); 
    } 
    memset(&server_address, 0, sizeof(server_address)); 
    server_address.sin_family = AF_INET;
    // server's ip address
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(atoi(port));  // port number


    if ((bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } else {
        printf("Socket successfully binded..\n"); 
    }

    if ((listen(listen_socket, 3)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } else {
        printf("Server listening..\n"); 
    } 

    return listen_socket;
}
*/ 

int accept_connection(int socket)
{
    socklen_t sock_size;
    struct sockaddr_storage client_address;
    sock_size = sizeof(client_address);

    return accept(socket, (struct sockaddr *)&client_address, &sock_size);
} 

int main() {
    int listen_socket = initialize_socket("25565");
    int client_socket = accept_connection(listen_socket); 

    char buffer[1024];
    read(client_socket, buffer, 1024); 
    write(client_socket, buffer, strlen(buffer));
    
    printf("successful connection\n");
}

