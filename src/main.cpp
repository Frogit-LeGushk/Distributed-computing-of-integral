#include <iostream>

#include "calculate.hpp"
#include "client.hpp"
#include "server.hpp"


int main(int n, char **v)
{
    if(n == 2 && v[1][0] == 's') start_server();
    if(n == 2 && v[1][0] == 'c') start_client();

    std::cout << "Wrong cmd line args, exit..." << std::endl;
    return 0;
}
