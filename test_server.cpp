#include <iostream>
#include "asio_rpc.hpp"

using namespace asio_rpc;

int test_function(int a, int b) { return a + b; }

int main() {
    register_function("test_function", test_function);
    
    asio::io_context ioc;
    Server server(ioc);
    server.Run();
}