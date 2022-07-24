DIR=./src/
SRC=$(DIR)main.cpp $(DIR)calculate.cpp $(DIR)client.cpp $(DIR)server.cpp FUNCTION.cpp
OBJ=main.o calculate.o client.o server.o FUNCTION.o

server: $(SRC)
	g++ -c main.cpp 		-std=gnu++14 -o main.o
	g++ -c calculate.cpp 	-std=gnu++14 -o calculate.o
	g++ -c FUNCTION.cpp 	-std=gnu++14 -o FUNCTION.o
	g++ -c client.cpp 		-std=gnu++14 -o client.o
	g++ -c server.cpp 		-std=gnu++14 -o server.o
	g++ $(OBJ) -pthread -std=gnu++14 -o exe
	./exe s

client: main.cpp calculate.cpp client.cpp server.cpp FUNCTION.cpp
	g++ -c main.cpp 		-std=gnu++14 -o main.o
	g++ -c calculate.cpp 	-std=gnu++14 -o calculate.o
	g++ -c FUNCTION.cpp 	-std=gnu++14 -o FUNCTION.o
	g++ -c client.cpp 		-std=gnu++14 -o client.o
	g++ -c server.cpp 		-std=gnu++14 -o server.o
	g++ main.o calculate.o client.o server.o FUNCTION.o -pthread -std=gnu++14 -o exe
	./exe c

clean:
	-rm *.o 2>/dev/null
	-rm *.out 2>/dev/null
	-rm exe 2>/dev/null
	-rm *.txt 2>/dev/null
