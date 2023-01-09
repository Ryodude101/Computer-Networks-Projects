/* Server
 * 04.28.22
 * Ryan Colon
 * This program is the server side for project 3 for Computer Networks
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <sys/un.h>
#include <arpa/inet.h>

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
#define MAXMSG 11

bool handleConnection(int clisock);

char* filename = "Logfile";
FILE* out;

#pragma pack(1)
typedef struct packet{
	uint32_t Version;
	uint32_t messageType;
	uint32_t messageLength;
	char message[MAXMSG];
}packet;
#pragma pack(0)

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
	
	//Open logfile
	out = fopen(filename, "w");
	fputs("BEGINNING SESSION.\n", out);
	fputs("LISTENING.\n", out);
	fclose(out);

	// Listen on the socket, queue 5 incoming connections.
	listen(server_socket, 5);

	// Loop forever, handling connections.
	while(1) 
	{
		// accept connection from the client
		// Exit with an error messgae if there is an error
		int clisock = -1;
		struct sockaddr_in client_addr;
		socklen_t clen;

		clisock = accept(server_socket, (struct sockaddr *)&client_addr, &clen);

		char filebuffer[2000];
		sprintf(filebuffer, "Received connection from: <%s, %d>\n", inet_ntoa(client_addr.sin_addr), (int) ntohs(client_addr.sin_port));
		out = fopen(filename, "a");
		fputs(filebuffer, out);
		fclose(out);

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
	packet recvd; //received packet

	bool exit = false;

	//accept message until disconnect
	while(!exit){
		recv(clisock, &recvd, sizeof(recvd), 0);

		//Check for version mismatch
		if(recvd.Version != 17){
			//Log Error
			out = fopen(filename, "a");
			fputs("ERROR: VERSION MISMATCH.\n", out);
			fclose(out);

			exit = true;
		}
		//else handle the message
		else{
			//check message type and act accordingly
			packet response;
			switch(recvd.messageType){
				case 1:
					//log
					out = fopen(filename, "a");
					fputs("EXECUTED SUPPORTED COMMAND: LIGHTON.\n", out);
					fclose(out);

					//construct and send response
					response.Version = 17;
					response.messageType = 0;
					response.message[0] = 'S';
					response.message[1] = 'U';
					response.message[2] = 'C';
					response.message[3] = 'C';
					response.message[4] = 'E';
					response.message[5] = 'S';
					response.message[6] = 'S';
					response.message[7] = '\0';
					response.messageLength = 7;

					send(clisock, &response, sizeof(response), 0);
					break;
				case 2:
					//log
					out = fopen(filename, "a");
					fputs("EXECUTED SUPPORTED COMMAND: LIGHTOFF.\n", out);
					fclose(out);

					//construct and send response
					response.Version = 17;
					response.messageType = 0;
					response.message[0] = 'S';
					response.message[1] = 'U';
					response.message[2] = 'C';
					response.message[3] = 'C';
					response.message[4] = 'E';
					response.message[5] = 'S';
					response.message[6] = 'S';
					response.message[7] = '\0';
					response.messageLength = 7;

					send(clisock, &response, sizeof(response), 0);
					break;
				default:
					if(strcmp(recvd.message, "HELLO") == 0){
						//log
						out = fopen(filename, "a");
						fputs("SENDING HELLO.\n", out);
						fclose(out);

						//construct and send response
						packet response;
						response.Version = 17;
						response.messageType = 0;
						response.message[0] = 'H';
						response.message[1] = 'E';
						response.message[2] = 'L';
						response.message[3] = 'L';
						response.message[4] = 'O';
						response.message[5] = '\0';
						response.messageLength = 5;

						send(clisock, &response, sizeof(response), 0);
					}
					else if(strcmp(recvd.message, "DISCONNECT") == 0){
						exit = true;
					}
					
			}
		}
	}

	out = fopen(filename, "a");
	fputs("DISCONNECTING\n", out);
	fclose(out);

	//close connection to the client
	close(clisock);
	return true;
}
