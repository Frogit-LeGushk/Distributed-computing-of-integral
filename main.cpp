#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>



struct ThreadArg {
    double      start_x         ;
    double      end_x           ;
    double      diff_x          ;
    double      (*f)(double x)  ;
    double *    result          ;
};
void * thread_entry_point(void * arg)
{
    ThreadArg * thread_arg = static_cast<ThreadArg *>(arg);
    double result = 0;

    double xi = thread_arg->start_x + (thread_arg->diff_x)/2;
    while(xi < thread_arg->end_x)
    {
        result += thread_arg->f(xi) * thread_arg->diff_x;
        xi += thread_arg->diff_x;
    }

    *(thread_arg->result) = result;
    return nullptr;
}
double calc_function(double x)
{
    return exp(4*sin(x));
}
double result_parallel_calc(double x_start, double x_end, int cnt_threads)
{
    assert(x_start < x_end);

    /* 0) initialize handle structures */
    const double    ALL_RANGE   = x_end - x_start;
    const int       CNT_THREADS = cnt_threads;
    const double    RANGE       = ALL_RANGE / cnt_threads;
    const long      CNT_CUTS    = RANGE * 100'000;
    const double    EPS         = RANGE/CNT_CUTS;
    pthread_t       arr_tid[CNT_THREADS];
    ThreadArg       arr_arg[CNT_THREADS];
    double          arr_res[CNT_THREADS];

    /* 1) initialize thread arguments */
    for(int i = 0; i < CNT_THREADS; i++)
    {
        arr_arg[i].start_x  = x_start + i * RANGE;
        arr_arg[i].end_x    = x_start + (i+1) * RANGE;
        arr_arg[i].diff_x   = EPS;
        arr_arg[i].f        = calc_function;
        arr_arg[i].result   = &arr_res[i];
    }

    /* 2) start threads */
    for(int i = 0; i < CNT_THREADS; i++)
        pthread_create(&arr_tid[i], NULL, thread_entry_point, &arr_arg[i]);

    /* 3) wait threads */
    for(int i = 0; i < CNT_THREADS; i++)
        pthread_join(arr_tid[i], nullptr);

    /* 4) sum all results */
    double total_res = 0;
    for(int i = 0; i < CNT_THREADS; i++)
        total_res += arr_res[i];

    return total_res;
}
struct WaitFunctionArg {
    int sock;
    std::vector<int> * pool_connections;
};
void * wait_fn(void * arg) {
    WaitFunctionArg * wait_fn_arg = static_cast<WaitFunctionArg *>(arg);
    while(1)
    {
        int conn = accept(wait_fn_arg->sock, NULL, NULL);
        if(conn != -1) wait_fn_arg->pool_connections->push_back(conn);
    }
    return nullptr;
};
double broadcast_calc_req(double x_start, double x_end, int cnt_threads, const std::vector<int> & pool_connections)
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
void start_server(const std::string serverAddr = "127.0.0.1:8080")
{
    std::string IPv4 = serverAddr.substr(0, serverAddr.find(':'));
    std::string PORT = serverAddr.substr(serverAddr.find(':')+1, serverAddr.size());

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    std::perror("socket\t\t");

    const int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    std::perror("setsockopt\t");

    struct sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IPv4.c_str());
    addr.sin_port = htons(std::stoi(PORT));

    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    std::perror("bind\t\t");

    listen(sock, SOMAXCONN);
    std::perror("listen\t\t");

    pthread_t wait_thread;
    std::vector<int> pool_connections;

    struct WaitFunctionArg wait_fn_arg = {sock, &pool_connections};
    pthread_create(&wait_thread, nullptr, wait_fn, &wait_fn_arg);

    while(1)
    {
        std::cout << "Enter [x_start, x_end, cnt_threads]: ";
        double x_start, x_end;
        int cnt_threads;
        std::cin >> x_start >> x_end >> cnt_threads;

        if(pool_connections.size())
        {
            std::cout << "Waiting... " << std::endl;
            std::cout << "Result: " << broadcast_calc_req(x_start, x_end, cnt_threads, pool_connections);
            std::cout << std::endl;
            continue;
        }

        std::cout << "No working clients, repeat lately" << std::endl;
    }
}
void start_client(const std::string serverAddr = "127.0.0.1:8080")
{
    std::string IPv4 = serverAddr.substr(0, serverAddr.find(':'));
    std::string PORT = serverAddr.substr(serverAddr.find(':')+1, serverAddr.size());

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    std::perror("socket\t\t");

    struct sockaddr_in  addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(IPv4.c_str());
    addr.sin_port = htons(std::stoi(PORT));

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    std::perror("connect\t\t");

    char buffer[BUFSIZ+1];
    while(1)
    {
        int readed = recv(sock, buffer, BUFSIZ, 0);
        buffer[readed] = '\0';
        std::cout << "Got data: " << buffer << std::endl;

        double x_start, x_end;
        int cnt_threads;
        sscanf(buffer, "%lf %lf %d", &x_start, &x_end, &cnt_threads);

        double result = result_parallel_calc(x_start, x_end, cnt_threads);
        sprintf(buffer, "%lf", result);
        send(sock, buffer, strlen(buffer), 0);
    }
}
int main(int n, char **v)
{
    std::string serverAddr = "192.168.1.8:8080";

    if(n == 2 && v[1][0] == 's') start_server(serverAddr);
    if(n == 2 && v[1][0] == 'c') start_client(serverAddr);

    return 0;
}
