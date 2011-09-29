#ifndef NETWORK_H
#define NETWORK_H

#define DEF_PORT 		7001
#define BUFFER_LENGTH 	275
#define FILE_SIZE		3

// Function Prototypes
#ifdef __cplusplus
extern "C" {
#endif
int tcpSocket();
int setReuse(int* socket);
int bindAddress(int *port, int *socket);
int setListen(int *socket);
int acceptConnection(int *listenSocket);
int acceptConnectionIp(int *listenSocket, char* ip);
int acceptConnectionPort(int *listenSocket, unsigned short *port);
int readData(int *socket, char *buffer, int bytesToRead);
int sendData(int *socket, const char *buffer, int bytesToSend);
int closeSocket(int *socket);
int connectToServer(int *port, int *socket, const char *ip);
int makeSocketNonBlocking(int *socket);
#ifdef __cplusplus
}
#endif
#endif


