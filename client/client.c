#include "client.h"
#include <stdio.h>

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
		return 2;
	}

	if(setReuse(&socket) == -1) {
		perror("Cannot set reuse\n");
		return 3;
	}

	if((port = conToServer(&socket, ipAddr)) == -1) {
		return 4;
	}

	return 0;
}

int conToServer(int* socket, const char* ip) 
{
	if(connectToServer((int*)DEF_PORT, socket, ip) == -1) {
		perror("Cannot Connect to server\n");		
		return -1;
	}
	
	return 0;
}
