// Assignment 4 Aditya Singh Cheema
// This code was inspired by server-multithreaded.c shared by CSSE2310
// teaching staff
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <csse2310a4.h>
#define MAX_SIZE 1024
#define BUFFER_SIZE 100
#define MIN_ARGS 2
#define MAX_ARGS 3
#define MIN_PORT 1024
#define MAX_PORT 65535

void* client_thread(void*);

/* exit_error()
 * ------------
 * Function to exit with stderr and exit code specified in argument error.
 *
 * error: Integer to store value to exit
 *
 * Returns: Void
 *
 * Errors: Exits with error 1 when error = 1 and prints to stcerr
 *	   when invalid commandline arguments are provided.
 *	   Exits with error 2 when error = 2 and prints to stcerr
 *	   when socket can not be opened for listening.
 **/
void exit_error(int error) {
    switch (error) {
	case 1: // Error = 1 when invalid commandline arguments are given
	    fprintf(stderr, "Usage: psserver connections [portnum]\n");
	    exit(1);
	case 2:	// Error = 2 When socket can't be opened for listening
	    fprintf(stderr, "psserver: unable to open socket");
	    fprintf(stderr, " for listening\n");
	    exit(2);
    }
}

/* listening()
 * -----------
 * Function to open socket for listening so multiple clients can connect.
 * It intitialises structure addrinfo to store connection data (IPv4, 
 * TCP). Uses port number to get address info, then creates socket and 
 * invokes bind() system call. It then listens on the port for number of
 * connections specified in connection integer. It then prints to stderr 
 * the port number.
 *
 * portNo: constant character array to store port number provided by user.
 * connection: Integer to store number of clients that can connect.
 *
 * Returns: listenFd which is the file descriptor that is opened for 
 *	    listening.
 *
 * Errors: Exits with error 0 when getaddrinfo() call fails and address
 *	   can not be obtained.
 *	   Calls exit_error(2) to exit with error and print to stderr when
 *	   - Socket can not be created
 *	   - bind() system call fails
 *	   - listen() system call fails
 **/
int listening(const char* portNo, int connection) {
    struct addrinfo* ai = 0;
    struct addrinfo hint;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    int error;
    if ((error = getaddrinfo(NULL, portNo, &hint, &ai))) {
	freeaddrinfo(ai);
	exit(0);
    }
    // Creating Socket
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    int temp;
    if (setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &temp,
	    sizeof(int)) < 0) {
	exit_error(2);
    }
    // Binding
    if (bind(listenFd, (struct sockaddr*)ai->ai_addr,
	    sizeof(struct sockaddr)) < 0) {
	exit_error(2);
    }
    if (listen(listenFd, connection) < 0) {
	exit_error(2);
    }
    fprintf(stderr, "%s\n", portNo);
    fflush(stderr);
    return listenFd;
}   

/* connections()
 * -----------
 * Function to repeteadly accept connections from clients using accept()
 * system call. It does so by calling accept() in an infinite loop.
 * If there is an error accepting from a client, it exits with
 * status 2. The function then creates a thread and client_thread is 
 * invoked.
 *
 * fdServer: Integer which stores the file descriptor of the socket that
 *	     is opened by listening().
 *
 * Returns: Void.
 *
 * Errors: Exits with error status 2 when accept() system call fails.
 **/
void connections(int fdServer) {
    int fd;
    struct sockaddr_in client;
    socklen_t clientSize;
    // Accepting connections repeatedly
    while (1) {
	clientSize = sizeof(struct sockaddr_in);
	// accept() to block and wait for new connection
	fd = accept(fdServer, (struct sockaddr*)&client, &clientSize);
	if (fd < 0) {
	    exit(2);
	}
	// Starting a thread
	int* fdPtr = malloc(sizeof(int));
	*fdPtr = fd;
	pthread_t thread;
	pthread_create(&thread, NULL, client_thread, fdPtr);
    }
}

/* request_validate()
 * ------------------
 * Function to validate commands given by client. It is invoked by 
 * client_request_handle() and takens in a 2D character array with stores
 * all words in the commandline. It also takes integer words to store the
 * number of words. If the command has wrong name, sub, unsub or pub
 * command given, it returns ":invalid\n".
 *
 * result: 2D character array to store all words in the commandline
 * words: Integer variable to store number of words.
 *
 * Returns: NULL 
 *
 * Errors: Invalid string if the command is invalid.
 **/
char* request_validate(int words, char** result) {
    char* invalid = ":invalid\n";
    for (int i = 0; i < words; i++) {
	if (strcmp(result[i], "name") == 0) {
	    if (strchr(result[i + 1], ' ') || strchr(result[i + 1], ':') ||
		    strcmp(result[i + 1], "") == 0) {
		return invalid;
	    }
	    if (strcmp(result[i + 2], "") == 0) {
		return invalid;
	    }
	} else if (strcmp(result[i], "sub") == 0) {
	    if (strchr(result[i + 1], ' ') || strchr(result[i + 1], ':') ||
		    strcmp(result[i + 1], "") == 0) {
		return invalid;
	    }
	} else if (strcmp(result[i], "pub") == 0) {
	    if (strchr(result[i + 1], ' ') || strchr(result[i + 1], ':') ||
		    strcmp(result[i + 1], "") == 0) {
		return invalid;
	    }
	    if (strcmp(result[i + 2], "") == 0) {
		return invalid;
	    }
	} else if (strcmp(result[i], "unsub") == 0) {
	    if (strchr(result[i + 1], ' ') || strchr(result[i + 1], ':') ||
		    strcmp(result[i + 1], "") == 0) {
		return invalid;
	    }
	} else {
	    return invalid;
	}
    }
    return NULL;
}

/* client_request_handle()
 * -----------------------
 * Function to handle requests provided by the client. It uses strtok()
 * function to split the array into tokens and number of words is stored
 * in wordCount integer. The new array is stored in result 2D array which
 * is passed to request_validate() and the result is stored in temp array.
 *
 * array: Character array which stores the commands given by client
 * length: Integer to store the number of bytes read.
 *
 * Returns: temp if temp != NULL otherwise if temp == NULL, returns array
 *
 * Errors: None.
 **/
char* client_request_handle(char* array, int length) {
    char** result = malloc(sizeof(char*) * length);
    // Loop to initialise result 2D array
    for (int i = 0; i < length; i++) {
	result[i] = malloc(sizeof(char) * length);   
    }
    char* temp = malloc(sizeof(char) * length);
    temp = NULL;
    char* token = strtok(array, " ");
    int wordCount = 0;
    while (token != NULL) {
	strcpy(result[wordCount], token);
	wordCount++;
	// To move to the next word
	token = strtok(NULL, " ");
    }
    temp = request_validate(wordCount, result);
    free(result);
    free(token);
    if (temp != NULL) {
	return temp;
    } else {
	return array;
    }
}

/* client_thread()
 * ---------------
 * Thread function that is invoked by connections(). Takes in argument arg
 * of void* data type which is the file descriptor of the socket connected
 * to a particular client. It then reads what is sent by the client from 
 * the socket and stores it in buffer. 
 * This buffer is passed to client_request_handle() to validate the 
 * commands. The result is then written back to the client.
 *
 * arg: Of type void* but stores file descriptor which is type casted to
 *	intger and stored in integer fd.
 *
 * Returns: calls pthread_exit(NULL) to close the thread.
 *
 * Errors: If read() system call fails, exits with error status 2.
 **/
void* client_thread(void* arg) {
    char buffer[MAX_SIZE];
    char* result = malloc(sizeof(char) * BUFFER_SIZE);
    ssize_t bytesRead;
    int fd = *(int*)arg;
    free(arg);
    // Reading data from client and sending back to server
    memset(buffer, 0, MAX_SIZE);
    while((bytesRead = read(fd, buffer, MAX_SIZE)) > 0) {
	result = client_request_handle(buffer, bytesRead);
	write(fd, result, strlen(result));
    }
    if (bytesRead < 0) {
	exit(2);
    }
    close(fd);
    pthread_exit(NULL);
}

int main(int argc, char** argv) {
    int fdServer;
    // Validating commandline arguments
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
	exit_error(1);
    }
    int portNumber = 0;
    if (argv[2] != NULL) {
	portNumber = atoi(argv[2]);
	// Port number needs to be in the valid range 1024 - 65535
	if (portNumber < MIN_PORT || portNumber > MAX_PORT) {
	    exit_error(1);
	}
    }
    const int connectNumber = atoi(argv[1]);
    char* port = argv[2];
    for (int i = 0; i < strlen(argv[1]); i++) {
	if (!isdigit(argv[1][i])) {
	    exit_error(1);
	}
    }
    // Connecting server
    fdServer = listening(port, connectNumber);
    connections(fdServer);
    free(port);
    return 0;
}
