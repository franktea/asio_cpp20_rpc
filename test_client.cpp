#include <iostream>
#include "asio_rpc.hpp"

using namespace asio_rpc;

asio::awaitable<void> TestCall(asio::io_context& ioc) {
    Client client(ioc);
    co_await client.Connect();
    int ret = co_await client.Call<int>("test_function", 3, 5);
    std::cout<<"get result: "<<ret<<"\n";
    co_return;
}

int main() {
    asio::io_context io_context;
    asio::co_spawn(io_context, TestCall(io_context), asio::detached);
    io_context.run();
}