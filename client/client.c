#include "client.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <strings.h>
#include <time.h>

#define USAGE		"Usage: %s -i [ip address]\n"
#define RECEIVE		0
#define SEND		1

int main(int argc, char** argv)
{
	char* ipAddr = 0;
	int option = 0;
	int controlSocket = 0;

	if(argc < 3) {
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

	printf("Connecting to control Server: %s\n", ipAddr);
	controlSocket = initConnection(DEF_PORT, ipAddr);
	processCommand(&controlSocket);

	return 0;
}

/*
-- FUNCTION: processCommand
--
-- DATE: September 23, 2011
--
-- REVISIONS: 
-- September 25, 2011 - removed socket as part of the arguments and moved the 
-- creation of the socket inside.
-- September 27, 2011 - moved the creation of the socket to a helper function
-- September 27, 2011 - changed arguments to controlSocket and transferSocket
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void processCommand(int* controlSocket, int* transferSocket);
--				controlSocket - pointer to the controlSocket
--				transferSocket - pointer to the transferSocket
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
-- 0 - receive a file from the server
-- 1 - send a file to the server
-- h - show a list of available commands
*/
void processCommand(int* controlSocket)
{
	char* cmd = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
	int port = getPort(controlSocket);
	
	printf("Port: %d", port); 
	
	// Print help
	printHelp();
	
	while(TRUE) {
		printf("$ ");
		if(scanf("%c", &(cmd[0])) == EOF) { 
			cmd[0] = 'e';
		}
		
		switch(cmd[0]) {
		case 'e': // exit
			closeSocket(controlSocket);
			exit(EXIT_SUCCESS);
		case 'l': // list files
			if(sendData(controlSocket, cmd, BUFFER_LENGTH) == -1) {
				systemFatal("Error sending list command");
			}
			closeSocket(controlSocket);
			listFile(port);
			break;
		case 'r': // receive file
			printf("Enter Filename: ");
			scanf("%s", cmd + 1);
			// Send Command and file name
			if(sendData(controlSocket, cmd, BUFFER_LENGTH) == -1) {
				systemFatal("Error sending receive command");
			}
			closeSocket(controlSocket);
			receiveFile(port, cmd + 1);
			exit(EXIT_SUCCESS);
		case 's': // send file
			printf("Enter Filename: ");
			scanf("%s", cmd + 1);
			// Send Command and file name
			if(sendData(controlSocket, cmd, BUFFER_LENGTH) == -1) {
				systemFatal("Error sending send command.");
			}
			closeSocket(controlSocket);
			sendFile(port, cmd + 1);
			exit(EXIT_SUCCESS);
		case 'h': // show commands
			printHelp();
			break;
		case 'f':
			system("ls");
			break;
		default:
			printf("Invalid Command. Type h for a list of commands.\n");
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
void listFile(int port)
{	
	char files[BUFFER_LENGTH];
	int dataRead;
	int transferSocket = 0;
	
	initalizeServer(&transferSocket, &port);
	
	system("clear");
	
	while(TRUE) {
		// Receive File List
		dataRead = readData(&transferSocket, files, FILENAME_MAX);
		if(dataRead <= 0) {
			break;
		}
		// Print File List
		printf("%s", files);
	}
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
void receiveFile(int port, const char* fileName)
{
	char* buffer = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
	FILE* file = NULL;
	off_t fileSize = 0;
	int bytesRead = 0;
	int count = 0;
	int transferSocket = 0;
	
	initalizeServer(&transferSocket, &port);
	
	readData(&transferSocket, buffer, BUFFER_LENGTH);
	bcopy(buffer + 1, (void*)fileSize, sizeof(off_t));
	
	if((file = fopen(fileName, "wb")) == NULL) {
		fprintf(stderr, "Error opening file: %s\n", fileName);
		return;
	}
	
	while (count < (fileSize - BUFFER_LENGTH)) {
        bytesRead = readData(&transferSocket, buffer, BUFFER_LENGTH);
        fwrite(buffer, sizeof(char), bytesRead, file);
        count += bytesRead;
    }
	
	bytesRead = readData(&transferSocket, buffer, fileSize - count);
    fwrite(buffer, sizeof(char), bytesRead, file);
	
	fclose(file);
	
	chmod(fileName, 00400 | 00200 | 00100);
    
    free(buffer);
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
void sendFile(int port, const char* fileName)
{
	struct stat statBuffer;
	char* buffer = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
	int file = 0;
    off_t offset = 0;
    int transferSocket = 0;
	
	initalizeServer(&transferSocket, &port);
	
	if ((file = open(fileName, O_RDONLY)) == -1) {
		fprintf(stderr, "Error opening file: %s\n", fileName);
		return;
	}
	
	if (fstat(file, &statBuffer) == -1) {
        systemFatal("Error Getting File Information");
    }

    bcopy((void*)&statBuffer.st_size, buffer, sizeof(off_t));
    sendData(&transferSocket, buffer, BUFFER_LENGTH);
    
    // Send the file to the client
    if (sendfile(transferSocket, file, &offset, statBuffer.st_size) == -1) {
        fprintf(stderr, "Error sending %s\n", fileName);
    }
    
    // Close the file
    close(file);
    free(buffer);
}

int initConnection(int port, const char* ip) 
{
	int socket;
	
	printf("Port: %d\n", port);
	
	// Creating Socket
	if((socket = tcpSocket()) == -1) {
		systemFatal("Error Creating Socket");
	}

	// Setting Socket to Reuse
	if(setReuse(&socket) == -1) {
		systemFatal("Error Set Socket Reuse");
	}
	
	// Connect to transfer server
	if(connectToServer(&port, &socket, ip) == -1) {
		systemFatal("Cannot Connect to server");
	}
	
	return socket;
}

void printHelp()
{
	system("clear");
	printf("Super File Transfer\n");
	printf("l - list transferable files\n");
	printf("r - receive file\n");
	printf("s - send file\n");
	printf("h - help\n");
	printf("e - exit\n");
}

void initalizeServer(int* socket, int* port)
{
    // Create a TCP socket
    if ((*socket = tcpSocket()) == -1) {
        systemFatal("Cannot Create Socket!");
    }
    
    // Allow the socket to be reused immediately after exit
    if (setReuse(&(*socket)) == -1) {
        systemFatal("Cannot Set Socket To Reuse");
    }
    
    // Bind an address to the socket
    if (bindAddress(port, socket) == -1) {
        systemFatal("Cannot Bind Address To Socket");
    }
    
    // Set the socket to listen for connections
    if (setListen(&(*socket)) == -1) {
        systemFatal("Cannot Listen On Socket");
    }
    
    if(acceptConnection(&(*socket)) == -1) {
    	systemFatal("Cannot Accept on Socket");
    }
}

int getPort(int* socket)
{
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(*socket, (struct sockaddr *)&sin, &len) == -1) {
		systemFatal("getsockname Error");
	}
	return ntohs(sin.sin_port);
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

