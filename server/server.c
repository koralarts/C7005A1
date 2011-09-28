#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <strings.h>

#include "server.h"
#include "../network/network.h"

#define GET_FILE 0
#define SEND_FILE 1
#define REQUEST_LIST 2
#define FILE_SIZE 3

void initializeServer(int *listenSocket, int *port);
void processConnection(int socket);
void getFile(int socket, char *fileName);
void sendFile(int socket, char *fileName);
static void systemFatal(const char* message);

void server(int port)
{
    int listenSocket = 0;
    int socket = 0;
    int processId = 0;
    
    // Set up the server
    initializeServer(&listenSocket, &port);
    
    // Loop to monitor the server socket
    while (1)
    {
        // Block here and wait for new connections
        if ((socket = acceptConnection(&listenSocket)) == -1)
        {
            systemFatal("Can't Accept Client");
        }
        printf("New Client!");
        // Spawn process to deal with client
        if (processId == 0)
        {
            close(listenSocket);
            // Process the child connection
            processConnection(socket);
            // Once we are done, exit
            close(socket);
            return;
        }
        else if (processId > 0)
        {
            // Since I am the parent, keep on going
            close(socket);
            continue;
        }
        else
        {
            // Fork failed, should shut down as this is a serious issue
            systemFatal("Fork Failed To Create Child To Deal With Client");
        }
    }
    
    printf("Server Closing!\n");
}

void processConnection(int socket)
{
    char *buffer = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
    
    // Read data from the client
    while (1)
    {
        readData(&socket, buffer, BUFFER_LENGTH);

        switch (buffer[0])
        {
        case GET_FILE:
            // Add 1 to buffer to move past the control byte
            getFile(socket, buffer + 1);
            break;
        case SEND_FILE:
            // Add 1 to buffer to move past the control byte
            sendFile(socket, buffer + 1);
            break;
        case REQUEST_LIST:
            break;
        }
    }
}

void getFile(int socket, char *fileName)
{
    char *buffer = (char*)malloc(sizeof(char) * BUFFER_LENGTH);
    int count = 0;
    int bytesRead = 0;
    off_t fileSize = 0;
    FILE *file = NULL;
    
    // Get the control packet with the file size
    readData(&socket, buffer, BUFFER_LENGTH);
    
    // Retrieve file size from the buffer
    bcopy(buffer + 1, (void*)fileSize, sizeof(off_t));
    
    // Open the file
    file = fopen(fileName, "wb");
    if (file == NULL)
    {
        systemFatal("Unable To Create File");
    }
    
    // Read from the socket and write the file to disk
    while (count < (fileSize - BUFFER_LENGTH))
    {
        bytesRead = readData(&socket, buffer, BUFFER_LENGTH);
        fwrite(buffer, sizeof(char), bytesRead, file);
        count += bytesRead;
    }
    
    // Retrieve any left over data and write it out
    bytesRead = readData(&socket, buffer, fileSize - count);
    fwrite(buffer, sizeof(char), bytesRead, file);

    // Close the file
    fclose(file);
    
    // Change the permission on the new file to allow access
    chmod(fileName, 00400 | 00200 | 00100);
    
    free(buffer);
}

void sendFile(int socket, char *fileName)
{
    int file = 0;
    struct stat statBuffer;
    off_t offset = 0;
    char *buffer = (char*)calloc(BUFFER_LENGTH, sizeof(char));
    
    // Open the file for reading
    if ((file = open(fileName, O_RDONLY)) == -1)
    {
        systemFatal("Problem Opening File");
    }
    
    // Ensure that the were able to get the file statistics
    if (fstat(file, &statBuffer) == -1)
    {
        systemFatal("Problem Getting File Information");
    }
    
    // Send a control message with the size of the file
    *buffer = FILE_SIZE;
    bcopy((void*)statBuffer.st_size, buffer + 1, sizeof(off_t));
    sendData(&socket, buffer, BUFFER_LENGTH);
    
    // Send the file to the client
    if (sendfile(socket, file, &offset, statBuffer.st_size) == -1)
    {
        systemFatal("Unable To Send File");
    }
    
    // Close the file
    close(file);
    free(buffer);
}


void getFileList(char *buffer)
{
    int index = 0;
    DIR *mydir = opendir(".");
    struct dirent *entry = NULL;
    
    while ((entry = readdir(mydir)))
    {
        break;
    }
    
    closedir(mydir);
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

