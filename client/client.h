#ifndef CLIENT_H
#define CLIENT_H

#include "../network/network.h"

#define MAX_PORT_SIZE 	11
#define TRUE 			1

#ifdef __cplusplus
extern "C" {
#endif
int processParent(int* socket, const char* ipAddr);
void processChild(int* socket, const char* ip, const int port);
void listFile(int* socket);
void receiveFile(int* socket, const char* fileName);
void sendFile(int* socket, const char* fileName);

// Helper functions
void printHelp();
#ifdef __cplusplus
}
#endif
#endif
