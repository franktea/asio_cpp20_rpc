#include <iostream>
#include "asio_rpc.hpp"

using namespace asio_rpc;

asio::awaitable<void> Rpc(asio::ip::tcp::socket socket) {
    std::vector<uint8_t> buff;
    try {
        for(;;) {
            size_t len;
            co_await asio::async_read(socket, asio::buffer(&len, sizeof(size_t)), asio::use_awaitable);
            buff.resize(len);
            co_await asio::async_read(socket, asio::buffer(buff), asio::use_awaitable);
            msgpack::Unpacker unpacker(&buff[0], buff.size());
            std::string function_name;
            unpacker(function_name);
            buff = functions[function_name](unpacker);
            len = buff.size();
            co_await asio::async_write(socket, asio::buffer(&len, sizeof(len)), asio::use_awaitable);
            co_await asio::async_write(socket, asio::buffer(buff), asio::use_awaitable);
        }
    } catch (std::exception& e) {
        std::cout<<"exception: "<<e.what()<<"\n";
    }
}

asio::awaitable<void> Listen(asio::io_context& ioc) {
    asio::ip::tcp::acceptor acceptor(ioc, {asio::ip::tcp::v4(), 5555});
    for(;;) {
        std::cout<<"listen...\n";
        asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(ioc, Rpc(std::move(socket)), asio::detached);
    }
}

int test_function(int a, int b) { return a + b; }

int main() {
    register_function("test_function", test_function);
    
    asio::io_context ioc;
    Server server(ioc);
    server.Run();
}