#include <stdio.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdlib.h>

#include "server.h"
#include "../network/network.h"

/* Might not need this
typedef union epoll_data
{
    void *ptr;
    int fd;
    _uint32_t u32;
    _unit64_t u64;
} epoll_data_t;

struct epoll_event
{
    // Epoll events
    _uint32_t events;
    // User data
    epoll_data_t data;
};
*/

void initializeServer(int *listenSocket, int *port);
static void systemFatal(const char* message);

void server(int port, size_t maxClients)
{
    int listenSocket = 0;
    int epollServer = 0;
    struct epoll_event event;
    struct epoll_event *events;
    
    // Set up the server
    initializeServer(&listenSocket, &port);
    
    // Create the epoll server
    epollServer = epoll_create1(0);
    
    if (epollServer == -1)
    {
        systemFatal("Unable To Create Epoll Server");
    }

    // Set up the event
    event.data.fd = listenSocket;
    event.events = EPOLLIN | EPOLLET;

    // Attach the event to the epoll server, along with the socket
    if (epoll_ctl(epollServer, EPOLL_CTL_ADD, listenSocket, &event) == -1)
    {
        systemFatal("Unable To Set Up Epoll Server With Socket");
    }

    // Buffer where the events are returned
    events = calloc(maxClients, sizeof event);
    
    printf("Server Closing!\n");
}

/*
-- FUNCTION: initializeServer
--
-- DATE: March 12, 2011
--
-- REVISIONS: September 22, 2011 - Added some extra comments about failure and
-- a function call to set the socket into non blocking mode.
--
-- DESIGNER: Luke Queenan
--
-- PROGRAMMER: Luke Queenan
--
-- INTERFACE: void initializeServer(int *listenSocket, int *port);
--
-- RETURNS: void
--
-- NOTES:
-- This function sets up the required server connections, such as creating a
-- socket, setting the socket to reuse mode, binding it to an address, and
-- setting it to listen. If an error occurs, the function calls "systemFatal"
-- with an error message.
*/
void initializeServer(int *listenSocket, int *port)
{
    // Create a TCP socket
    if ((*listenSocket = tcpSocket()) == -1)
    {
        systemFatal("Cannot Create Socket!");
    }
    
    // Allow the socket to be reused immediately after exit
    if (setReuse(&(*listenSocket)) == -1)
    {
        systemFatal("Cannot Set Socket To Reuse");
    }
    
    // Bind an address to the socket
    if (bindAddress(&(*port), &(*listenSocket)) == -1)
    {
        systemFatal("Cannot Bind Address To Socket");
    }
    
    // Make the socket non blocking
    if (makeSocketNonBlocking(&(*listenSocket)) == -1)
    {
        systemFatal("Cannot Set Socket To Non Blocking");
    }
    
    // Set the socket to listen for connections
    if (setListen(&(*listenSocket)) == -1)
    {
        systemFatal("Cannot Listen On Socket");
    }
}

/*
-- FUNCTION: systemFatal
--
-- DATE: March 12, 2011
--
-- REVISIONS: (Date and Description)
--
-- DESIGNER: Aman Abdulla
--
-- PROGRAMMER: Luke Queenan
--
-- INTERFACE: static void systemFatal(const char* message);
--
-- RETURNS: void
--
-- NOTES:
-- This function displays an error message and shuts down the program.
*/
static void systemFatal(const char* message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

