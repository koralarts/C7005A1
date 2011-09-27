# GCC flags
GCC = gcc
FLAGS = -W -Wall

# Directories
CDIR = ./client
SDIR = ./server
NDIR = ./network
ODIR = ./object
BDIR = ./bin
DDIR = ./debug


# Release
release: client server

# Debug
debug: client-d server-d

# client
client: network.o client.o
	$(GCC) $(FLAGS) -o $(BDIR)/client $(ODIR)/client.o $(ODIR)/network.o

# client debug
client-d: network.o client.o
	$(GCC) $(FLAGS) -g -o $(DDIR)/client $(ODIR)/client.o $(ODIR)/network.o

# server
server: network.o server.o main.o
	$(GCC) $(FLAGS) -o $(BDIR)/server $(ODIR)/server.o $(ODIR)/main.o $(ODIR)/network.o
	
# server debug
server-d: network.o server.o main.o
	$(GCC) $(FLAGS) -g -o $(DDIR)/server $(ODIR)/server.o $(ODIR)/main.o $(ODIR)/network.o

# mkDir
dir:
	mkdir -p $(BDIR) && mkdir -p $(ODIR) && mkdir -p $(DDIR)

# Clean
clean:
	rm -f $(BDIR)/* $(ODIR)/* $(DDIR)/*

# Object
network.o: dir
	$(GCC) $(FLAGS) -o $(ODIR)/network.o -c $(NDIR)/network.c

client.o:
	$(GCC) $(FLAGS) -o $(ODIR)/client.o -c $(CDIR)/client.c

server.o:
	$(GCC) $(FLAGS) -o $(ODIR)/server.o -c $(SDIR)/server.c
	
main.o:
	$(GCC) $(FLAGS) -o $(ODIR)/main.o -c $(SDIR)/main.c

