/* Socket Tutorial
 * 03.06.22
 * Ryan Colon
 * This program is the project 1 for Computer Networks
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int socket_desc; //The socket handler
struct sockaddr_in server; //The handler for connecting to the server
char* filename;
char fileBuffer[1000]; //Used to store and format strings to be printed to the logfile
FILE* out;

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

	//Get from the user the message we want to send to the server
	puts("Enter message to send to server:");
	char message[256];
	scanf("%s", message);

	//Log the message being sent
	sprintf(fileBuffer, "Sending message to server: \"%s\"\n", message);
	fputs(fileBuffer, out);

	//Send the user entered message to the server
	if(send(socket_desc, message, strlen(message), 0) < 0){
		fputs("Failed to send message.\n", out);
		endSession(true);
		return 1;
	}
	
	//Get the reply from the server
	char server_reply[2000];
	if(recv(socket_desc, server_reply, 2000, 0) < 0){
		fputs("Failed to receive response.\n", out);
		endSession(true);
		return 1;
	}

	//Log the server response	
	sprintf(fileBuffer, "Received message from server: \"%s\"\n", server_reply);
	fputs(fileBuffer, out);
	printf("%s", fileBuffer);

	//End the program
	endSession(true);
	return 0;
}
