#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "client.hpp"
#include "calculate.hpp"


/* start main loop with server connection  */
void start_client(void)
{
    std::string broadcast_addr;
    std::cout << "Enter IPv4 address of server: ";
    std::cin >> broadcast_addr;
    broadcast_addr += ":8080";

    std::string IPv4 = broadcast_addr.substr(0, broadcast_addr.find(':'));
    std::string PORT = broadcast_addr.substr(broadcast_addr.find(':')+1, broadcast_addr.size());

    bool correct_start = true;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    std::perror("socket\t\t");
    if(sock == -1) correct_start = false;

    struct sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IPv4.c_str());
    addr.sin_port = htons(std::stoi(PORT));

    int conn_ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    std::perror("connect\t\t");
    if(conn_ret == -1) correct_start = false;

    if(!correct_start)
    {
        std::cerr << "Start error, exit..." << std::endl;
        exit(EXIT_FAILURE);
    }

    char buffer[BUFSIZ+1];
    char ready_symbol_req = 'R';
    char quite_symbol_rep = 'Q';
    while(1)
    {
        int readed = recv(sock, buffer, BUFSIZ, 0);
        if(readed == -1)
        {
            std::perror("recv\t\t");
            exit(EXIT_FAILURE);
        }

        buffer[readed] = '\0';
        std::cout << "Got data: " << buffer << std::endl;

        if((readed == 1) && (buffer[0] == ready_symbol_req))
        {
            send(sock, buffer, 1, 0);
            continue;
        }
        if((readed == 1) && (buffer[0] == quite_symbol_rep))
            exit(EXIT_SUCCESS);

        double x_start, x_end;
        int cnt_threads;
        assert(sscanf(buffer, "%lf %lf %d", &x_start, &x_end, &cnt_threads) == 3);

        double result = result_parallel_calc(x_start, x_end, cnt_threads);
        sprintf(buffer, "%lf", result);
        send(sock, buffer, strlen(buffer), 0);
    }
}
