#pragma once

#include <vector>
#include <string>
#include <functional>
#include <tuple>
#include <system_error>
#include <unordered_map>
#include <msgpack/msgpack.hpp>
#include <asio.hpp>
#include <stdint.h>

namespace asio_rpc {

template<typename... Args>
std::vector<uint8_t> serialize(const std::string& function_name, Args&&... arguments) {
    using ArgsTuple = std::tuple<typename std::decay_t<std::remove_reference_t<Args>>...>;
    ArgsTuple tuple = std::make_tuple(arguments...);
    msgpack::Packer packer{};
    packer(function_name); // package function name, then pack all param values
    [&]<typename Tuple, std::size_t... I>(Tuple&& tuple, std::index_sequence<I...>) {
        (packer(std::get<I>(tuple)), ...);
    } (std::forward<ArgsTuple>(tuple),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<ArgsTuple>>>{});
    return packer.vector();
}

template<typename R, typename... Args>
std::vector<uint8_t> call_proxy(std::function<R(Args...)>&& func, msgpack::Unpacker& unpacker) {
    using ArgsTuple = std::tuple<typename std::decay_t<std::remove_reference_t<Args>>...>;
    ArgsTuple tuple;
    R result = [&]<typename Tuple, std::size_t... I>(Tuple&& tuple, std::index_sequence<I...>) -> R {
        (unpacker(std::get<I>(tuple)), ...);
        return func(std::get<I>(std::forward<Tuple>(tuple))...);        
    }(std::forward<ArgsTuple>(tuple), std::make_index_sequence<std::tuple_size_v<ArgsTuple>>{});
    
    msgpack::Packer packer{};
    packer(result);
    return packer.vector();
}

inline std::unordered_map<std::string, std::function<std::vector<uint8_t>(msgpack::Unpacker&)>> functions;

template<typename R, typename... Args>
bool register_function(const std::string function_name,
    R(*function)(Args...)) {
    auto [it, b] = functions.insert(std::make_pair(function_name, [function](msgpack::Unpacker& unpacker){
            std::function<R(Args...)> f = function;
            return call_proxy(std::move(f), unpacker);
    }));
    return b;
}

class Client {
public:
    Client(asio::io_context& ioc): ioc_(ioc), socket_(ioc_) {}
    asio::awaitable<void> Connect() {
        asio::ip::tcp::endpoint end_point(asio::ip::address::from_string("127.0.0.1"), 5555);
        co_await socket_.async_connect(end_point, asio::use_awaitable);
        co_return;
    }

    template<typename R, typename... Args>
    asio::awaitable<R> Call(std::string func, Args... args) {
        std::vector<uint8_t> buff = serialize(func, std::forward<Args>(args)...);
        size_t len = buff.size();
        co_await asio::async_write(socket_, asio::buffer(&len, sizeof(len)), asio::use_awaitable);
        co_await asio::async_write(socket_, asio::buffer(buff), asio::use_awaitable);
        co_await asio::async_read(socket_, asio::buffer(&len, sizeof(len)), asio::use_awaitable);
        buff.resize(len);
        co_await asio::async_read(socket_, asio::buffer(buff), asio::use_awaitable);
        msgpack::Unpacker unpacker(&buff[0], buff.size());
        R result;
        unpacker(result);
        co_return result;
    }
private:
    asio::io_context& ioc_;
    asio::ip::tcp::socket socket_;
};

class Server {
    Server(asio::io_context& ioc): ioc_(ioc) {}
    void Run() {

    }
private:
    asio::io_context& ioc_;
};

} // end of namespace asio_rpc