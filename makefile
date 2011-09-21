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
client: network.o client.o
	$(GCC) $(FLAGS) -o $(BDIR)/client $(ODIR)/client.o $(ODIR)/network.o

# server
server: network.o server.o
	$(GCC) $(FLAGS) -o $(BDIR)/server $(ODIR)/server.o $(ODIR)/network.o

# mkDir
dir:
	mkdir -p $(BDIR) && mkdir -p $(ODIR)

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
