#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define USAGE		"Usage: %s -i [ip address]\n"

int main(int argc, char** argv)
{
	char* ipAddr = 0;
	int port = 0;
	int option = 0;

	if(argc != 3) {
		fprintf(stderr, "Not Enough Arguments\n");
		fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
	}

	while((option = getopt(argc, argv, ":i:")) != -1)
    {
        switch(option)
        {
        case 'i':
            ipAddr = optarg;
            break;
        default:
            fprintf(stderr, USAGE, argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    
    if(ipAddr == 0) {
    	fprintf(stderr, "-i is a required switch\n");
    	fprintf(stderr, USAGE, argv[0]);
        exit(EXIT_FAILURE);
    }

	// Connecting to server
	if((port = processParent(ipAddr)) == 0) {
		systemFatal("Invalid Port\n");
	}
	
	processChild(ipAddr, port);

	return 0;
}

/*
-- FUNCTION: processParent
--
-- DATE: September 23, 2011
--
-- REVISIONS: 
-- September 25, 2011 - removed socket and port as part of the arguments and 
-- moved the creation of the socket inside.
-- September 27, 2011 - moved the creation of the socket to a helper function.
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: int processParent(const char* ip);
--				ip - ip address of the server
--
-- RETURNS: int > 0 - the dynamically created port
--				== 0 - invalid port
--
-- NOTES:
-- This function sets up the required client connections to connect to the
-- main server, such as creating a socket, setting the socket to reuse mode,
-- binding it to an address, and setting it to listen. If an error occurs, the
-- function calls "systemFatal" with an error message.
*/
int processParent(const char* ip) 
{
	int socket;
	char* reply = (char*)malloc(sizeof(char) * MAX_PORT_SIZE);
	
	socket = initConnection(DEF_PORT, ip);
	
	// Wait for port respond
	if(readData(&socket, reply, MAX_PORT_SIZE) == -1) {
		systemFatal("Error Reading from Socket\n");
	}
	
	// Convert reply to int
	return atoi(reply);
}

/*
-- FUNCTION: processChild
--
-- DATE: September 23, 2011
--
-- REVISIONS: 
-- September 25, 2011 - removed socket as part of the arguments and moved the 
-- creation of the socket inside.
-- September 27, 2011 - moved the creation of the socket to a helper function
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void processChild(const char* ip, int port);
--				ip - ip address of the server
--				port - the port to connect to
--
-- RETURNS: void
--
-- NOTES:
-- This function sets up the required client connections to connect to the
-- transfer server, such as creating a socket, setting the socket to reuse mode,
-- binding it to an address, and setting it to listen. If an error occurs, the
-- function calls "systemFatal" with an error message.
--
-- A menu will be printed with the available commands. Each command will produce
-- a different effect on the server.
--
-- Commands:
-- e - exit the program
-- l - list the available files on the server
-- r - receive a file from the server
-- s - send a file to the server
-- h - show a list of available commands
*/
void processChild(const char* ip, int port)
{
	int socket, cmd, space, numRead;
	char fileName[FILENAME_MAX];
	
	socket = initConnection(port, ip);
	
	// Print help
	printHelp();
	
	while(TRUE) {
		memset(fileName, '\0', FILENAME_MAX);
		printf("$ ");
		
		if((numRead = scanf("%d%d%s", &cmd, &space, fileName)) == EOF) { 
			cmd = 'e';
		}
		
		switch(cmd) {
		case 'e': // exit
			closeSocket(&socket);
			exit(EXIT_SUCCESS);
		case 'l': // list files
			listFile(&socket);
			break;
		case 'r': // receive file
			if(numRead != 3) {
				perror("Invalid Command. h for help\n");
			} else {
				receiveFile(&socket, fileName);
			}
			break;
		case 's': // send file
			if(numRead != 3) {
				perror("Invalid Command. h for help\n");
			} else {
				sendFile(&socket, fileName);
			}
			break;
		case 'h': // show commands
			printHelp();
		}
	}
	
}

/*
-- FUNCTION: listFile
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void listFile(int* socket)
--				socket - the current socket the client is connected as
--
-- RETURNS: void
--
-- NOTES:
-- This function sends the list command and waits for the reply of the transfer
-- server. The transfer server will reply with the list of files available in
-- the transfer directory on the server.
-- 
-- Once the reply is received, it will be printed in order of receiving.
*/
void listFile(int* socket)
{	
	char cmd[2] = "l";
	char files[FILENAME_MAX];
	
	// Send Command
	if(sendData(socket, cmd, sizeof(cmd)) == -1) {
		systemFatal("Error sending list command\n");
	}
	
	// Receive File List
	if(readData(socket, files, FILENAME_MAX) == -1) {
		systemFatal("Error receiving file list\n");
	}
	
	// Print File List
	system("clear");
	printf("File List\n%s", files);
}


/*
-- FUNCTION: receiveFile
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void receiveFile(int* socket, const char* fileName)
--				socket - the current socket the client is connected as
--				fileName - the name of the file to be received/downloaded
--
-- RETURNS: void
--
-- NOTES:
-- This function sends the receive command and waits for the reply of the 
-- transfer server. The transfer server will reply with the contents of the
-- file. If the file is not present, the server will return an error message.
-- This error message will be printed out.
--
-- Once all the contents of the file are received and written to a file, the
-- program will print out a success message.
*/
void receiveFile(int* socket, const char* fileName)
{
	char cmd[2] = "r";
	FILE* file;
	
	// Send Command
	if(sendData(socket, cmd, sizeof(cmd)) == -1) {
		systemFatal("Error sending receive command\n");
	}
	
	// Send file name to receive
	if(sendData(socket, fileName, FILENAME_MAX) == -1) {
		systemFatal("Error sending receive filename\n");
	}
	
	if((file = fopen(fileName, "w")) == NULL) {
		fprintf(stderr, "Error opening file: %s\n", fileName);
		return;
	}
	
	while(TRUE) {
		
	}
	
}


/*
-- FUNCTION: sendFile
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void sendFile(int* socket, const char* fileName)
--				socket - the current socket the client is connected as
--				fileName - the name of the file to be received/downloaded
--
-- RETURNS: void
--
-- NOTES:
-- This function sends the receive command and waits for the reply of the 
-- transfer server. The transfer server will reply with the contents of the
-- file. If the file is not present, the server will return an error message.
-- This error message will be printed out.
--
-- Once all the contents of the file are received and written to a file, the
-- program will print out a success message.
*/
void sendFile(int* socket, const char* fileName){}

int initConnection(int port, const char* ip) 
{
	int socket;
	// Creating Socket
	if((socket = tcpSocket()) == -1) {
		systemFatal("Error Creating Socket\n");
	}

	// Setting Socket to Reuse
	if(setReuse(&socket) == -1) {
		systemFatal("Error Set Socket Reuse\n");
	}
	
	// Connect to transfer server
	if(connectToServer(&port, &socket, ip) == -1) {
		systemFatal("Cannot Connect to server\n");
	}
	
	return socket;
}

void printHelp()
{
	system("clear");
	printf("Super File Transfer\n");
	printf("l - list transferable files\n");
	printf("r [[file name]] - receive file\n");
	printf("s [[file name]] - send file\n");
	printf("h - help\n");
	printf("e - exit\n");
}

/*
-- FUNCTION: systemFatal
--
-- DATE: March 12, 2011
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: static void systemFatal(const char* message);
--
-- RETURNS: void
--
-- NOTES:
-- This function displays an error message and shuts down the program.
*/
static void systemFatal(const char* message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

