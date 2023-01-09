//#include <iostream>
//#include <cctype>
#include <stdlib.h>
#include <stdbool.h>
//#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
//#include <vector>
//#include <sstream>
#include <unistd.h>
//#include <string>
#include <errno.h>

//using namespace std;

#define MAXLINE 256
#define MAXSIZE 1024

#define ACK                   101
#define REQUESTFILE           102
#define SENDINGFILE           103
#define RECEIVEFILE           104
#define NOTACK                401
#define INVALIDCOMMAND        402
#define NOFILE                403

#define DEFAULT_PORT 9002

char* filename = "Logfile";
FILE* out;

bool handleConnection(int clisock);

int main(int argc, char* argv[]) 
{
	// Declare variables to set up socket
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket == -1){
		printf("Error creating socket.\tErrno: %d\n%s\n", errno, strerror(errno));
		return 0;
	}

	struct sockaddr_in server;  //The handler for the server
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(DEFAULT_PORT);
	
	// Check the input from argument in runtime or from user interface
	if(argc > 1){
		for(int i = 1; i < argc; i++){
			if(argv[i][0] == '-'){
				switch(argv[i][1]){
					case 'p':
						++i;
						server.sin_port = htons(atoi(argv[i]));
						printf("Port: %s\n", argv[i]);
						break;
					case 'l':
						++i;
						filename = argv[i];
						printf("Logfile: %s\n", argv[i]);
						break;
					default:
						puts("Unrecognized flag: \"");
						puts(argv[i]);
						puts("\"\n");
						return 0;
				}
			}
			else{
				printf("Unrecognized flag: \"%s\"\n", argv[i]);
				return 0;
			}
		}
	}
	else
		printf("No input flags detected, defaulting server values.\n\tPort: %d\n\tLogfile: %s\n", DEFAULT_PORT, filename);
	
	// Bind the socket to the server address and port.
	// check for error and print error message
	bind(server_socket,(struct sockaddr*) &server, sizeof(server));
	if(server_socket == -1){
		printf("Error binding socket.\tErrno: %d\n%s\n", errno, strerror(errno));
		return 0;
	}
	
	// Clear the server address structure.
	//Open logfile
	out = fopen(filename, "w");
	fputs("Beginning session.\n", out);


	// Listen on the socket, queue 5 incoming connections.
	listen(server_socket, 5);
	fputs("Listening...\n", out);
	fclose(out);
	
	// Loop forever, handling connections.
	while(1) 
	{
		// accept connection from the client
		// Exit with an error messgae if there is an error
		int clisock = -1;
		clisock = accept(server_socket, NULL, NULL);
		if(clisock == -1){
			printf("Error accepting connection.\tErrno: %d\n%s\n", errno, strerror(errno));
			return 0;
		}
		else{	// handleConnection;
			if(!handleConnection(clisock)){
				out = fopen(filename, "a");
				fputs("Ending session.\n", out);
				fclose(out);
				return 0;
			}
		}
	}
	return 0;
}

bool handleConnection(int clisock) 
{
	// Declare variables, File pointers as needed
	FILE* quoteFile;

	//Open and grab quotes, each quote takes a single line
	quoteFile = fopen("quotes.txt", "r");
	int count = 0;
	char quotes[MAXLINE][MAXSIZE] = {'\0'};
	while(fgets(quotes[count], MAXSIZE, quoteFile) != NULL){
		quotes[count][strlen(quotes[count]) - 1] = '\0';
		++count;
	}

	fclose(quoteFile);

	int quotePos = rand() % count; // Determine the random quote
	
	// receive the message into the buffer use "recv"
	// print error message if there is any error
	char message[2000];
	for(int i = 0; i < 2000; i++)
		message[2000] = '\0';
	if(recv(clisock, message, 2000, 0) < 0){
		fputs("Failed to receive message.\n", out);
		out = fopen(filename, "a");
		printf("Error receiving client message\n");
		fclose(out);
		return false;
	}

	// Print the message into the console
	// Print error message or the message from the client to the log file
	char filebuffer[2000];
        for(int i = 0; i < 2000; i++)
		filebuffer[i] = '\0';	
	sprintf(filebuffer, "Received message: %s\n", message);
	out = fopen(filename, "a");
	fputs(filebuffer, out);
	fclose(out);
	printf("Received message: %s\n", message);

	
	// Check for the "network" keyword in the message from client
	// Print the quote message to be sent to the client to the log file
	// If the key word exist pull a random quote and send to the receiver
	if(strstr(message, "networks") == NULL){
		out = fopen(filename, "a");
		fputs("Message did not contain 'networks'. Not responding.\n", out);
		fclose(out);
		printf("Message did not contain 'networks'. Not responding\n.");
	}
	else{
		sprintf(filebuffer, "Sending message: %s\n", quotes[quotePos]);
		out = fopen(filename, "a");
		fputs(filebuffer, out);
		fclose(out);
		printf("Sending message: %s\n", quotes[quotePos]);
		send(clisock, quotes[quotePos], strlen(quotes[quotePos]) + 1, 0);
	}
	
	//close connection to the client
	close(clisock);
	return true;
}
