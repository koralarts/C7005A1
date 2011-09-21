#ifndef CLIENT_H
#define CLIENT_H

#include "../network/network.h"

#ifdef __cplusplus
extern "C" {
#endif
int conToServer(int* socket, const char* ipAddr);
#ifdef __cplusplus
}
#endif
#endif
