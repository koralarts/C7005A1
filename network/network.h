#ifndef NETWORK_H
#define NETWORK_H

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
int readData(int *socket, char *buffer, int bytesToRead);
int sendData(int *socket, const char *buffer, int bytesToSend);
int closeSocket(int *socket);
int connectToServer(int *port, int *socket, const char *ip);
#ifdef __cplusplus
}
#endif
#endif


