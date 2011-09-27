#include <stdio.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "server.h"
#include "../network/network.h"

#define MAX_EVENTS 64
#define BUFFER_LENGTH 512
#define PORT_RANGE 10000

void initializeServer(int *listenSocket, int *port);
int getFreePort(char *buffer, int startingPort);
static void systemFatal(const char* message);

void server(int port)
{
    int listenSocket = 0;
    int socket = 0;
    int epollServer = 0;
    int numberOfReadyEvents = 0;
    int count = 0;
    char *ports = (char*)calloc(PORT_RANGE, sizeof(char));
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
    events = calloc(MAX_EVENTS, sizeof event);
    
    // Loop to monitor the epoll server
    while (1)
    {
        // Wait indefinitely for an event (connection) to occur
        numberOfReadyEvents = epoll_wait(epollServer, events, MAX_EVENTS, -1);
        
        // If we get here, we have some events, so iterate through the events
        for (count = 0; count < numberOfReadyEvents; count++)
        {
            if ((events[count].events & EPOLLERR) ||
                (events[count].events & EPOLLHUP) ||
                (!events[count].events & EPOLLIN))
            {
                // A non fatal error has occured, continue through the list
                close(events[count].data.fd);
                continue;
            }
            else if (listenSocket == events[count].data.fd)
            {
                // We have a connection waiting on the listening socket
                while (1)
                {
                    /*
                    if ((socket = acceptConnection(&listenSocket)) == -1)
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            // We are done processing connections
                            break;
                        }
                        else
                        {
                            // Other error when trying to accept the connection
                            // TODO: error message here
                            break;
                        }
                    }
                    */
                    // TODO: FORK HERE?
                    
                    // Create a mini server to deal with the client
                    socket = getFreePort(&(*ports), port);
                    initializeServer(&socket, socket);
                    
                    // Make the socket non blocking
                    if (makeSocketNonBlocking(&socket) == -1)
                    {
                        systemFatal("Cannot Set Socket To Non Blocking");
                    }
                    
                    // Add the socket to list of file descriptors to monitor
                    event.data.fd = socket;
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(listenSocket, EPOLL_CTL_ADD, socket, &event)
                        == -1)
                    {
                        systemFatal("Unable To Add Socket To Epoll Server");
                    }
                    
                    // Send a new port to the client
                }
                continue;
            }
            else
            {
                // We have a file transfer waiting to happen, spawn a process
            }
        }
    }
    
    printf("Server Closing!\n");
}

int getFreePort(char *buffer, int startingPort)
{
    int count = 0;
    for (count = 0; count < PORT_RANGE; count++)
    {
        if (buffer[count] == 0)
        {
            buffer[count] = 1;
            return startingPort + count;
        }
    }
    return -1;
}

void setFreePort(char *buffer, int startingPort, int port)
{
    buffer[port - startingPort] = 0;
}

void processConnection(int socket)
{
    int done = 0;
    int bytesRead = 0;
    char *buffer = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
    
    // Read data from the client
    while (1)
    {
        bytesRead = readData(&socket, &(*buffer), BUFFER_LENGTH);
        
        switch (buffer[0])
        {
        
        }
    }
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

