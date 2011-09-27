#ifndef CLIENT_H
#define CLIENT_H

#include "../network/network.h"

#define MAX_PORT_SIZE 	11
#define TRUE 			1

#ifdef __cplusplus
extern "C" {
#endif
int processParent(const char* ipAddr);
void processChild(const char* ip, int port);
void listFile(int* socket);
void receiveFile(int* socket, const char* fileName);
void sendFile(int* socket, const char* fileName);

// Helper functions
int initConnection(int port, const char* ip);
void printHelp();
static void systemFatal(const char* message);
#ifdef __cplusplus
}
#endif
#endif
