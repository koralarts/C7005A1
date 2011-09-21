# GCC flags
GCC = gcc
FLAGS = -W -Wall

# Directories
CDIR = ./client
SDIR = ./server
NDIR = ./network
ODIR = ./object
BDIR = ./bin

# client
client: client.o network.o
	$(GCC) $(FLAGS) -o $(BDIR)/client $(ODIR)/client.o $(ODIR)/network.o

# server
server: server.o network.o
	$(GCC) $(FLAGS) -o $(BDIR)/server $(ODIR)/server.o $(ODIR)/network.o

# Clean
clean:
	rm -f $(BDIR)/* $(ODIR)/*

# Object
network.o: $(NDIR)/network.c
	$(GCC) $(FLAGS) -o $(ODIR)/network.o -c $(NDIR)/network.c

client.o: $(CDIR)/client.c
	$(GCC) $(FLAGS) -o $(ODIR)/client.o -c $(CDIR)/client.c

server.o: $(SDIR)/server.c
	$(GCC) $(FLAGS) -o $(ODIR)/server.o -c $(SDIR)/server.c
