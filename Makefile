CC=g++
CFLAGS=-c -std=gnu++14
LDFLAGS=-pthread

MOVE_ALERTS_TO_TRASH=2>/dev/null

SRC=main.cpp calculate.cpp client.cpp server.cpp FUNCTION.cpp
OBJ=$(SRC:.cpp=.o)
DEL=$(SRC:.cpp=.o) server client

run_server: server
	./server s

run_client: client
	./client c

server: $(SRC)
	$(CC) $(CFLAGS) $(SRC)
	$(CC) $(OBJ) $(LDFLAGS) -o server

client: $(SRC)
	$(CC) $(CFLAGS) $(SRC)
	$(CC) $(OBJ) $(LDFLAGS) -o client

clean:
	-rm $(DEL) $(MOVE_ALERTS_TO_TRASH)

