#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argv, char* argc[])
{
	int socket;
	int port;
	char* ipAddr;

	if(argv < 2) {
		perror("Not enough arguments. Usage: client [[ip address]]\n");
		return 1;
	} else {
		ipAddr = argc[1];
	}

	if((socket = tcpSocket()) == -1) {
		perror("Cannot create socket\n");
		return 1;
	}

	if(setReuse(&socket) == -1) {
		perror("Cannot set reuse\n");
		return 1;
	}

	if((port = processParent(&socket, ipAddr)) == 0) {
		perror("Invalid Port");
		return 1;
	}
	
	processChild(&socket, ipAddr, port);

	return 0;
}

int processParent(int* socket, const char* ip) 
{
	char* reply = (char*)malloc(sizeof(char) * MAX_PORT_SIZE);
	
	printf("Connecting to server %s...\n", ip);
	// Connect to server
	if(connectToServer((int*)DEF_PORT, socket, ip) == -1) {
		perror("Cannot Connect to server\n");		
		exit(1);
	}
	printf("Connected to server.\n");
	
	// Wait for port respond
	if(readData(socket, reply, MAX_PORT_SIZE) == -1) {
		printf("Error reading from socket.\n");
		exit(1);
	}
	
	// Convert reply to int
	return atoi(reply);
}

void processChild(int* socket, const char* ip, int port)
{
	int cmd, space, numRead;
	char fileName[FILENAME_MAX];
	
	printf("Connecting to transfer server...\n");
	// Connect to transfer server
	if(connectToServer(&port, socket, ip) == -1) {
		perror("Cannot Connect to server\n");		
		exit(1);
	}
	printf("Connected to transfer server...\n");
	
	// Print help
	printHelp();
	
	while(TRUE) {
		memset(fileName, '\0', FILENAME_MAX);
		printf("Enter command: ");
		
		if((numRead = scanf("%d%d%s", &cmd, &space, fileName)) == EOF) { 
			closeSocket(socket);
			exit(0);
		}
		
		switch(cmd) {
		case 'x': // exit
			closeSocket(socket);
			exit(0);
		case 'l': // list files
			listFile(socket);
			break;
		case 'r': // receive file
			if(numRead != 3) {
				perror("Invalid Command. h for help");
			} else {
				receiveFile(socket, fileName);
			}
			break;
		case 's': // send file
			if(numRead != 3) {
				perror("Invalid Command. h for help");
			} else {
				sendFile(socket, fileName);
			}
			break;
		case 'h': // show commands
			printHelp();
		}
	}
	
}

void listFile(int* socket)
{	
	char cmd[2] = "l";
	char files[FILENAME_MAX];
	
	// Send Command
	if(sendData(socket, cmd, sizeof(cmd)) == -1) {
		perror("Error sending list command");
		exit(1);
	}
	
	// Receive File List
	if(readData(socket, files, FILENAME_MAX) == -1) {
		perror("Error receiving file list");
		exit(1);
	}
	
	// Print File List
	system("clear");
	printf("File List\n%s", files);
}

void receiveFile(int* socket, const char* fileName)
{
	char cmd[2] = "r";
	char file[FILENAME_MAX];
	
	// Send Command
	if(sendData(socket, cmd, sizeof(cmd)) == -1) {
		perror("Error sending receive command");
		exit(1);
	}
	
	// Send file name to receive
	//if(
	
}

void sendFile(int* socket, const char* fileName){}

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
