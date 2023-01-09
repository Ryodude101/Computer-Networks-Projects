/* Client
 * 04.28.22
 * Ryan Colon
 * This program is the client side for project 3 for Computer Networks
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXMSG 11

int socket_desc; //The socket handler
struct sockaddr_in server; //The handler for connecting to the server
char* filename;
char fileBuffer[1000]; //Used to store and format strings to be printed to the logfile
FILE* out;

#pragma pack(1)
typedef struct packet{
	uint32_t Version;
	uint32_t messageType;
	uint32_t messageLength;
	char message[MAXMSG];
}packet;
#pragma pack(0)

//Call this function to close the file and socket, needs to know if the socket has been made
void endSession(bool socketActive){
	if(socketActive)
		shutdown(socket_desc, SHUT_RDWR);

	fputs("Ending session.", out);
	fclose(out);
}

//Call this to "begin the session" (Create a logfile, claim the socket, and connect to the server)
bool beginSession(){
	//Begin logfile
	out = fopen(filename, "w");
	fputs("Beginning session.\n", out);

	//Initialize the socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_desc == -1){
		fputs("Couldn't create socket.\n", out);
		endSession(false);
		return false;
	}
	
	//Connect
	if(connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0){
		fputs("Error connecting to server.\n", out);
		endSession(true);
		return false;
	}

	return true;
}

int main(int argc, char* argv[]){
	//Default the values to what they're supposed to be in case the user fudges them
	filename = "Logfile";
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("34.125.14.243");
	server.sin_port = htons(8001);

	//This for loop handles all the flags that are present when the program is called
	for(int i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
				case 's':
					++i;
					server.sin_addr.s_addr = inet_addr(argv[i]);
					printf("Server: %s\n", argv[i]);
					server.sin_family = AF_INET;
					break;
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
	
	//Start the session
	if(!beginSession())
		return 1;

	packet toSend;
	//send the hello message
	toSend.Version = 17;
	toSend.messageType = 0;
	toSend.messageLength = 5;
	toSend.message[0] = 'H';
	toSend.message[1] = 'E';
	toSend.message[2] = 'L';
	toSend.message[3] = 'L';
	toSend.message[4] = 'O';
	toSend.message[5] = '\0';

	send(socket_desc, &toSend, sizeof(toSend), 0);

	//Log the message being sent
	fputs("SENDING HELLO MESSAGE\n", out);
	
	//get the server reply
	packet fromServer;
	recv(socket_desc, &fromServer, sizeof(fromServer), 0);

	//handle version mismatch
	if(fromServer.Version != 17){
		fputs("VERSION MISMATCH\n", out);
		endSession(true);
		return 0;
	}

	fputs("VERSION ACCEPTED\n", out);

	//Send Command
	fputs("Sending LIGHTON.\n", out);
	toSend.messageType = 1;
	toSend.messageLength = 7;
	toSend.message[0] = 'L';
	toSend.message[1] = 'I';
	toSend.message[2] = 'G';
	toSend.message[3] = 'H';
	toSend.message[4] = 'T';
	toSend.message[5] = 'O';
	toSend.message[6] = 'N';
	toSend.message[7] = '\0';

	
	send(socket_desc, &toSend, sizeof(toSend), 0);

	//get the reply
	recv(socket_desc, &fromServer, sizeof(fromServer), 0);

	//handle version mismatch
	if(fromServer.Version != 17){
		fputs("VERSION MISMATCH\n", out);
		endSession(true);
		return 0;
	}

	//log response
	char fileBuffer[2000];
	sprintf(fileBuffer, "RECEIVED REPLY: \"%s\"\n", fromServer.message);
	fputs(fileBuffer, out);

	fputs("DISCONNECTING.\n", out);

	//disconnect
	toSend.messageType = 0;
	toSend.messageLength = 10;
	toSend.message[0] = 'D';
	toSend.message[1] = 'I';
	toSend.message[2] = 'S';
	toSend.message[3] = 'C';
	toSend.message[4] = 'O';
	toSend.message[5] = 'N';
	toSend.message[6] = 'N';
	toSend.message[7] = 'E';
	toSend.message[8] = 'C';
	toSend.message[9] = 'T';
	toSend.message[10] = '\0';

	send(socket_desc, &toSend, sizeof(toSend), 0);

	//End the program
	endSession(true);
	return 0;
}
