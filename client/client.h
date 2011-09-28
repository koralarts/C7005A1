#ifndef CLIENT_H
#define CLIENT_H

#include "../network/network.h"

#define MAX_PORT_SIZE 	5
#define TRUE 			1

#ifdef __cplusplus
extern "C" {
#endif
void processCommand(int* controlSocket, int* transferSocket);
void listFile(int* controlSocket, int* transferSocket);
void receiveFile(int* controlSocket, int* transferSocket, const char* fileName);
void sendFile(int* controlSocket, int* transferSocket, const char* fileName);

// Helper functions
int initConnection(int port, const char* ip);
void printHelp();
static void systemFatal(const char* message);
#ifdef __cplusplus
}
#endif
#endif
