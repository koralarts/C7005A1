#ifndef CLIENT_H
#define CLIENT_H

#include "../network/network.h"

#define MAX_PORT_SIZE 	5
#define TRUE 			1

#ifdef __cplusplus
extern "C" {
#endif
void processCommand(int* controlSocket);
void listFile(int port);
void receiveFile(int port, const char* fileName);
void sendFile(int port, const char* fileName);

// Helper functions
int initConnection(int port, const char* ip);
void initalizeServer(int *socket, int *port);
void printHelp();
int getPort(int* socket);
static void systemFatal(const char* message);
#ifdef __cplusplus
}
#endif
#endif
