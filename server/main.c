#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

#define DEFAULT_PORT 8929

int main(int argc, char **argv);

int main(int argc, char **argv)
{
    // Initialize port and give default option in case of no user input
    int port = DEFAULT_PORT;
    int option = 0;

    // Parse command line parameters using getopt
    while ((option = getopt(argc, argv, "p:")) != -1)
    {
        switch (option)
        {
            case 'p':
                port = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s -p [port]\n", argv[0]);
                return 0;
        }
    }
    
    // Start server
    server(port);
    
    return 0;
}

