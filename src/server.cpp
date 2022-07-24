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
#include <signal.h>

#include "server.hpp"
#include "speenlock.hpp"


static Lock                 __LOCK;
static bool                 __IS_CALCULATE = false;
static std::vector<int> *   __PULL_CONNECTIONS;


struct WaitFunctionArg {
    int sock;
    std::vector<int> * pool_connections;
};
static void * wait_fn(void * arg) {
    WaitFunctionArg * wait_fn_arg = static_cast<WaitFunctionArg *>(arg);
    while(1)
    {
        int conn = accept(wait_fn_arg->sock, NULL, NULL);
        if(conn != -1)
        {
            __LOCK.lock();
            wait_fn_arg->pool_connections->push_back(conn);
            __LOCK.unlock();
        }
    }
    return nullptr;
};

static double broadcast_calc_req(double x_start, double x_end, int cnt_threads, const std::vector<int> & pool_connections)
{
    double total_sum = 0;
    double all_range = x_end - x_start;
    double range = all_range / pool_connections.size();
    double arr_sum[pool_connections.size()];
    char buffer[BUFSIZ+1];

    for(int i = 0; i < pool_connections.size(); i++)
    {
        sprintf(buffer, "%lf %lf %d", x_start + range * i, x_start + range * (i+1), cnt_threads);
        send(pool_connections[i], buffer, strlen(buffer), 0);
    }

    for(int i = 0; i < pool_connections.size(); i++)
    {
        int readed = recv(pool_connections[i], buffer, BUFSIZ, 0);
        buffer[readed] = '\0';
        sscanf(buffer, "%lf", &arr_sum[i]);
    }

    for(int i = 0; i < pool_connections.size(); i++)
        total_sum += arr_sum[i];

    return total_sum;
}
static bool broadcast_check_req(std::vector<int> & pool_connections)
{
    bool isReady = true;
    char ready_symbol_req = 'R';
    char ready_rep_buffer[BUFSIZ];
    std::vector<bool> ready_items;
    ready_items.assign(pool_connections.size(), true);

    struct timeval tv_4sec = {4, 0};
    struct timeval tv_inf  = {0, 0};

    /* set waiting 4 sec */
    for(int i = 0; i < pool_connections.size(); i++)
        setsockopt(pool_connections[i], SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_4sec, sizeof(tv_4sec));

    /* check clients */
    for(int i = 0; i < pool_connections.size(); i++)
    {
        send(pool_connections[i], &ready_symbol_req, 1, 0);
        int readed = recv(pool_connections[i], ready_rep_buffer, BUFSIZ, 0);
        if((readed != 1) || (ready_rep_buffer[0] != ready_symbol_req))
            ready_items[i] = false;
    }

    /* delete don't ready clients */
    for(int i = pool_connections.size() - 1; (i >= 0 && pool_connections.size()); i--)
    {
        if(!ready_items[i])
        {
            pool_connections.erase(pool_connections.begin()+i);
            isReady = false;
        }
    }

    /* set waiting as infinite */
    for(int i = 0; i < pool_connections.size(); i++)
        setsockopt(pool_connections[i], SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_inf, sizeof(tv_inf));

    return (isReady && pool_connections.size());
}

/* correct terminate of program */
static void signal_handler(int sig)
{
    if(sig == SIGINT)
    {
        char quite_symbol_rep = 'Q';
        for(int i = 0; i < __PULL_CONNECTIONS->size(); i++)
            send((*__PULL_CONNECTIONS)[i], &quite_symbol_rep, 1, 0);
        exit(EXIT_SUCCESS);
    }
}

/* start main loop, which wait commands for calculate integral sum  */
void start_server(void)
{
    const std::string filename  = "IPv4.txt";
    const std::string command   = "hostname -I | awk '{print $1}' >> " + filename;
    std::string serverAddr;
    system(command.c_str());

    std::ifstream input_file(filename);
    input_file >> serverAddr;
    serverAddr += ":8080";
    input_file.close();
    std::remove(filename.c_str());

    std::cout << "Server info [IPv4:PORT]: " << "[" << serverAddr << "]" << std::endl;
    std::string IPv4 = serverAddr.substr(0, serverAddr.find(':'));
    std::string PORT = serverAddr.substr(serverAddr.find(':')+1, serverAddr.size());

    bool correct_start = true;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    std::perror("socket\t\t");
    if(sock == -1) correct_start = false;

    const int optval = 1;
    int sockopt = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    std::perror("setsockopt\t");
    if(sockopt == -1) correct_start = false;

    struct sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IPv4.c_str());
    addr.sin_port = htons(std::stoi(PORT));

    int bind_ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    std::perror("bind\t\t");
    if(bind_ret == -1) correct_start = false;

    int listen_ret = listen(sock, SOMAXCONN);
    std::perror("listen\t\t");
    if(listen_ret == -1) correct_start = false;

    if(!correct_start)
    {
        std::cerr << "Start error, exit..." << std::endl;
        exit(EXIT_FAILURE);
    }

    pthread_t wait_thread;
    std::vector<int> pool_connections;
    __PULL_CONNECTIONS = &pool_connections;

    struct WaitFunctionArg wait_fn_arg = {sock, &pool_connections};
    assert(pthread_create(&wait_thread, nullptr, wait_fn, &wait_fn_arg) == 0);

    signal(SIGINT, signal_handler);
    while(1)
    {
        std::cout << "Enter [x_start, x_end, cnt_threads]: ";
        double x_start, x_end;
        int cnt_threads;
        std::cin >> x_start >> x_end >> cnt_threads;

        __LOCK.lock();
        __IS_CALCULATE = true;

        while(broadcast_check_req(pool_connections) == false && pool_connections.size() > 0);

        if(pool_connections.size())
        {
            std::cout << "Waiting... " << std::endl;
            std::cout << "Result: " << broadcast_calc_req(x_start, x_end, cnt_threads, pool_connections);
            std::cout << std::endl;
            __IS_CALCULATE = false;
            __LOCK.unlock();
            continue;
        }

        std::cout << "No working clients, repeat lately" << std::endl;
        __IS_CALCULATE = false;
        __LOCK.unlock();
    }
}
