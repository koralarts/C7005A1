#ifndef SERVER_H
#define SERVER_H

#define DEF_PORT 7001

// Function Prototypes
#ifdef __cplusplus
extern "C" {
#endif
void server (int port, size_t maxClients);
#ifdef __cplusplus
}
#endif
#endif


