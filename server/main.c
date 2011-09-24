#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

#define DEFAULT_PORT 8929
#define MAX_CLIENTS 64

int main(int argc, char **argv);

int main(int argc, char **argv)
{
    // Initialize port and maxClients to default values in case of no user input
    int port = DEFAULT_PORT;
    size_t maxClients = MAX_CLIENTS;
    int option = 0;

    // Parse command line parameters using getopt
    while ((option = getopt(argc, argv, "p:c:")) != -1)
    {
        switch (option)
        {
            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                maxClients = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -p [port]\n", argv[0]);
                return 0;
        }
    }
    
    // Start server
    server(port, maxClients);
    
    return 0;
}

