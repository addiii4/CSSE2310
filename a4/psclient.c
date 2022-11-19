// Assignment 4 Aditya Singh Cheema
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFFER_SIZE 100

// Structure ThreadInfo to store name and socket file descriptor
typedef struct {
    char* name;
    int socket;
} ThreadInfo;

/* exit_error()
 * ------------
 * Function to exit with stderr and exit code specified in argument error
 *
 * error: Integer to store value to exit
 * port: Character array to store port number to be printed when error = 4.
 *
 * Returns: Void
 *
 * Errors: Exits with error 1 when error = 1 and prints to stderr 
 *	   when invalid commandline arguments are provided.
 *	   Exits with error 2 when error = 2 and prints to stderr 
 *	   when invalid name is provided.
 *	   Exits with error 2 when error = 3 and prints to stderr
 *	   when invalid topic is provided.
 *	   Exits with error 3 when error = 4 and prints to stderr 
 *	   the port number when unable to connect.
 *	   Exits with error 4 when error = 5 and prints to stderr 
 *	   when server is disconnected.
 * */
void exit_error(int error, char* port) {
    switch (error) {
	case 1: // Error = 1 when improper commandline arguments given
	    fprintf(stderr, "Usage: psclient portnum name");
	    fprintf(stderr, " [topic] ...\n");
	    exit(1);
	case 2: // Error = 2 when invalid name given
	    fprintf(stderr, "psclient: invalid name\n");
	    exit(2);
	case 3: // Error = 3 when invalid topic given
	    fprintf(stderr, "psclient: invalid topic\n");
	    exit(2);
	case 4: // Error = 4 when unable to connect to 'port' provided
	    fprintf(stderr, "psclient: unable to connect");
	    fprintf(stderr, " to port %s\n", port);
	    exit(3);
	case 5: // Error = 5 when server is disconnected
	    fprintf(stderr, "psclient: server connection terminated\n");
	    exit(4);
    }
}

/* connecting()
 * ------------
 * Function to connect to the server. Invokes connect() system call.
 *
 * socketFd: Integer to store socket File Descriptor
 * port: Character string to store port number provided by user
 * address: Structure to store sockaddr data for connecting to server
 * result: Integer to store value returned by connect() system call.
 *
 * Returns: Void
 *
 * Errors: Calls exit_error() to exit with error 3 when result < 0 and 
 *	   connection unsuccessful.
 * */
void connecting(int socketFd, char* port, struct sockaddr_in* address) {
    int result = connect(socketFd, (struct sockaddr*) address,
	    sizeof(*address));
    // Result < 0 signifies connection error
    if (result < 0) {
	exit_error(4, port);
    }
}

/* sending()
 * ---------
 * Function to write to the server using send() system call.
 * Stores name and topic list in a single buffer which is sent to server.
 * Takes in input by STDIN and sends to server till EOF is detected.
 *
 * name: Character string to store name provided by user.
 * topics: 2D array to store list of topics provided by user.
 * topicCount: Integer counter to store number of topics.
 * socketFd: Integer to store socket File Descriptor
 * address: Structure to store sockaddr data for connecting to server
 *
 * Returns: exits with status 0.
 *
 * Errors: None
 * */
void* sending(char* name, char** topics, int topicCount, int socketFd, 
	struct sockaddr_in* address) {
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    strcat(buffer, "name ");
    strcat(buffer, name);
    strcat(buffer, "\n");
    // If topics are present in arguments, they are added to buffer
    if (topicCount != 0) {
	strcat(buffer, "sub");
	for (int i = 0; i < topicCount; i++) {
	    strcat(buffer, " ");
	    strcat(buffer, topics[i]);
	}
	strcat(buffer, "\n");
    }
    // Sending commandline arguments to server
    send(socketFd, buffer, strlen(buffer), 0);
    memset(buffer, 0, strlen(buffer));
    // Loop to take STDIN and send to server
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
	send(socketFd, buffer, strlen(buffer), 0);
	memset(buffer, 0, strlen(buffer));
    }
    free(buffer);
    exit(0);
}

/* receiving()
 * -----------
 * Function to read from the server using recvfrom() system call.
 * Stores socket file descriptor from threadData. 
 * Receives from server using recvfrom in an infinite loop till server is
 * disconnected. Prints to STDOUT the buffer received from server.
 * 
 * threadData: Structure to store value of name and socket file descriptor
 *
 * Returns: exits with status 4.
 *
 * Errors: exits with status 4 when recvfrom returns a non-positive integer.
 * */
void* receiving(void* threadData) {
    int socketFd, result;
    char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
    ThreadInfo* data = (ThreadInfo*)threadData;
    socketFd = data->socket;
    memset(buffer, 0, BUFFER_SIZE);
    // Infinite loop to keep receiving from server till disconnected
    while (1) {
	result = recvfrom(socketFd, buffer, BUFFER_SIZE, 0, NULL, NULL);
	// If not able to receive from server socket
	if (result <= 0) {
	    exit_error(5, NULL);
	} else {
	    printf("%s", buffer);
	    fflush(stdout);
	}
	memset(buffer, 0, BUFFER_SIZE);
    }
    free(buffer);
    exit_error(5, NULL);
}

/* argument_validate()
 * -------------------
 * Function to validate commandline arguments name and topics.
 * Checks if name has space, ':' or empty string and if topic has space,
 * ':', '\n' or empty string. If present, argument is invalid and exits 
 * while printing to stderr.
 * 
 * name: Character array to store name provided by user.
 * topicCount: Integer counter to store number of topics.
 * topics: 2D character array to store topics provided by user.
 *
 * Returns: Void.
 *
 * Errors: calls exit_error(2) when name is invalid to exit with status 2.
 *	   calls exit_error(3) when topic is invalid to exit with status 2.
 * */
void argument_validate(char* name, int topicCount, char** topics) {
    // If name has space, ':' or empty string, argument is invalid
    if (strchr(name, ' ') || strchr(name, ':') || strcmp(name, "") == 0) {
	exit_error(2, NULL);
    }
    // If topic has space, ':', '\n' or empty string, argument is invalid
    for (int i = 0; i < topicCount; i++) {
	if (strchr(topics[i], ' ') || strchr(topics[i], ':') || 
		strchr(topics[i], '\n') || strcmp(topics[i], "") == 0) {
	    exit_error(3, NULL);
	}
    }
}

int main(int argc, char** argv) {
    // Validating commandline arguments
    int minArgs = 3;
    if (argc < minArgs) {
	exit_error(1, NULL);
    }
    char* name = malloc(sizeof(char) * BUFFER_SIZE);
    long portNumber = strtol(argv[1], NULL, 0);
    char** topic = malloc(sizeof(char*) * BUFFER_SIZE);
    name = argv[2];
    int topicCount = 0;	
    if (argc >= minArgs) {
	for (int i = 3; i < argc; i++, topicCount++) {
	    topic[topicCount] = argv[i];
	}
    }
    argument_validate(name, topicCount, topic);
    // Defining IPv4 and ready to bind to any IP address for TCP 
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portNumber);
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    // Calling funtion connecting to connect to server on port in argv[1] 
    connecting(socketFd, argv[1], &address);
    // Creating threads to handle sending and receiving
    pthread_t thread;
    ThreadInfo info;
    info.name = name; 
    info.socket = socketFd;
    //Thread for receiving message
    pthread_create(&thread, NULL, receiving, (void*)&info);
    //Thread for Sending message
    sending(name, topic, topicCount, socketFd, &address);
    close(socketFd);
    free(name);
    free(topic);
    pthread_exit(NULL);
    return 0;
}
