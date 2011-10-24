/*
-- SOURCE FILE: client.c
--
-- PROGRAM: Super File Transfer
--
-- FUNCTIONS:
-- void processCommand(int* controlSocket);
-- void receiveFile(int port, const char* fileName);
-- void sendFile(int port, const char* fileName);
-- int initConnection(int port, const char* ip);
-- void initalizeServer(int* port, int* socket);
-- void printHelp(); 
-- int getPort(int* socket);
-- void printProgressBar(int fileSize, int tBytesRead);
-- static void systemFatal(const char* message);
--
-- DATE: March 12, 2011
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- NOTES:
-- This file contains the necessary functions that will connect to the server
-- and process the different commands that the user specifies.
*/

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
#include <string.h>

#include "client.h"

#define USAGE		"Usage: %s -i [ip address]\n"
#define DEF_DIR 	"./share/"

/*
-- FUNCTION: main
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: int main(int argc, char** argv)
--				argc - number of arguments
--				argv - the arguments
--
-- RETURNS: int - 0
--
-- NOTES:
-- This is the main function where the arguments are parsed and proper 
-- preparations are done. These preparations include initializing sockets.
*/
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
-- r - receive a file from the server
-- s - send a file to the server
-- f - show local files
-- h - show a list of available commands
*/
void processCommand(int* controlSocket)
{
	FILE* temp = NULL;
	char* cmd = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
	int port = 7009;//getPort(controlSocket);
	
	// Print help
	printHelp();
	printf("$ ");
	
	while(TRUE) {		
		cmd[0] = getc(stdin);
		
		switch(cmd[0]) {
		case 'e': // exit
			closeSocket(controlSocket);
			exit(EXIT_SUCCESS);
		case 'r': // receive file
			cmd[0] = (char)0;
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
			cmd[0] = (char)1;
			printf("Enter Filename: ");
			scanf("%s", cmd + 1);
			// Send Command and file name
			if(sendData(controlSocket, cmd, BUFFER_LENGTH) == -1) {
				systemFatal("Error sending send command.");
			}
			if((temp = fopen(cmd + 1, "r"))== NULL) {
				fprintf(stderr, "%s does not exist\n", cmd + 1);
				continue;
			}
			fclose(temp);
			closeSocket(controlSocket);
			sendFile(port, cmd + 1);
			exit(EXIT_SUCCESS);
		case 'h': // show commands
			printHelp();
			printf("$ ");
			break;
		case 'f':
			system("ls");
			printf("$ ");
			break;
		}
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
-- INTERFACE: void receiveFile(int port, const char* fileName)
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
	char* fileNamePath = (char*)malloc(sizeof(char) * FILENAME_MAX);
	
	initalizeServer(&port, &transferSocket);
	
	// Get Size of file
	readData(&transferSocket, buffer, BUFFER_LENGTH);
	memmove((void*)&fileSize, buffer, sizeof(off_t));
	printf("Size of File: %d\n", (int)fileSize);
	
	// Create file path
	sprintf(fileNamePath, "%s%s", DEF_DIR, fileName);
    printf("Save Path: %s\n", fileNamePath);
	
	// Opening file for writing
	if((file = fopen(fileNamePath, "wb")) == NULL) {
		fprintf(stderr, "Error opening file: %s\n", fileName);
		return;
	}
	
	// Hide Cursor
	fprintf(stderr, "\033[?25l");
	
	// Start Reading from socket
	while (count < (fileSize - BUFFER_LENGTH)) {
        bytesRead = readData(&transferSocket, buffer, BUFFER_LENGTH);
        fwrite(buffer, sizeof(char), bytesRead, file);
        count += bytesRead;
        printProgressBar(fileSize, count);
    }
	
	bytesRead = readData(&transferSocket, buffer, fileSize - count);
	printProgressBar(fileSize, count + bytesRead);
    fwrite(buffer, sizeof(char), bytesRead, file);
	// End Reading from socket
	
	// Show Cursor
	fprintf(stderr, "\033[?25h\n");
	
	// Close file
	fclose(file);
	
	chmod(fileName, 00400 | 00200 | 00100);
    
    // Free memory allocated for buffer
    free(buffer);
    
    // Print Success message
    printf("Transfer Complete!\n");
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
-- INTERFACE: void sendFile(int port, const char* fileName)
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
	char *buffer = (char*)calloc(BUFFER_LENGTH, sizeof(char));
	int file = 0;
    int transferSocket = 0;
	
	initalizeServer(&port, &transferSocket);
	
	if ((file = open(fileName, O_RDONLY)) == -1) {
        systemFatal("Unable To Open File");
	}
	
	// Get size of file
	if (fstat(file, &statBuffer) == -1) {
        systemFatal("Error Getting File Information");
    }
    memmove(buffer, (void*)&statBuffer.st_size, sizeof(off_t));
    
    printf("Connected to server and sending %s\n", fileName);
    
    // Send file size
    if (sendData(&transferSocket, buffer, BUFFER_LENGTH) == -1) {
        systemFatal("Send Failed");
    }
    
    // Send the file to the client
    if (sendfile(transferSocket, file, NULL, statBuffer.st_size) == -1) {
        fprintf(stderr, "Error sending %s\n", fileName);
    }
    
    // Close the file
    close(file);
    free(buffer);
    
    // Print Success message
    printf("Transfer Complete!\n");
}

/*
-- FUNCTION: initConnection
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: int initConnection(int port, const char* ip) 
--				port - the port the client will connect to to the server
--				ip - ip address of the server
--
-- RETURNS: int - the new socket created
--
-- NOTES:
-- This function will create the socket, set reuse and connect to the server.
*/
int initConnection(int port, const char* ip) 
{
	int socket;

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

/*
-- FUNCTION: printHelp
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void printHelp()
--
-- RETURNS: void
--
-- NOTES:
-- Prints ouf the list of commands available.
*/
void printHelp()
{
	system("clear");
	printf("Super File Transfer\n");
	printf("r - receive file\n");
	printf("s - send file\n");
	printf("f - list local files\n");
	printf("h - help\n");
	printf("e - exit\n");
}

/*
-- FUNCTION: initializeServer
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void initalizeServer(int* port, int* socket)
--				port - the port the client will listen on
--				socket - the socket that will hold the new socket
--
-- RETURNS: void
--
-- NOTES:
-- This function will create the socket, set reuse and listen for any incoming
-- connections from the server.
*/
void initalizeServer(int* port, int* socket)
{
    int sock = 0;

    // Create a TCP socket
    if ((sock = tcpSocket()) == -1) {
        systemFatal("Cannot Create Socket!");
    }
    
    // Allow the socket to be reused immediately after exit
    if (setReuse(&sock) == -1) {
        systemFatal("Cannot Set Socket To Reuse");
    }
    
    // Bind an address to the socket
    if (bindAddress(port, &sock) == -1) {
        systemFatal("Cannot Bind Address To Socket");
    }
    
    // Set the socket to listen for connections
    if (setListen(&sock) == -1) {
        systemFatal("Cannot Listen On Socket");
    }
    
    if((*socket = acceptConnection(&sock)) == -1) {
    	systemFatal("Cannot Accept on Socket");
    }
    close(sock);
}

/*
-- FUNCTION: getPort
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: int getPort(int* socket)
--				socket - the socket where the port will be determined.
--
-- RETURNS: void
--
-- NOTES:
-- This function will get the port that the socket is bound to.
*/
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
-- FUNCTION: printProgressBar
--
-- DATE: September 23, 2011
--
-- REVISIONS:
--
-- DESIGNER: Karl Castillo
--
-- PROGRAMMER: Karl Castillo
--
-- INTERFACE: void printProgressBar(int fileSize, int tBytesRead)
--				fileSize - the total size of the file.
--				tBytesRead - the number of bytes currently read
--
-- RETURNS: void
--
-- NOTES:
-- This function will produce a progress bar-like animation.
*/
void printProgressBar(int fileSize, int tBytesRead) 
{
	char t = '=';
	int perc = ((double)tBytesRead / fileSize) * 100;
	int step = perc/2;
	int i;
	
	fprintf(stderr, "%d%c[", perc, '%');
	for(i = 0; i < step; i++) {
		fprintf(stderr, "%c", t);
	}
	fprintf(stderr, ">\r");
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

