
server: main.cpp
	g++ main.cpp -pthread -std=gnu++14 && ./a.out s

client: main.cpp
	g++ main.cpp -pthread -std=gnu++14 && ./a.out c

clean:
	-rm *.o 2>/dev/null
	-rm *.out 2>/dev/null
