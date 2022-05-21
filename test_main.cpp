#include <iostream>
#include "asio_rpc.hpp"

using namespace asio_rpc;

int test_function(int a, int b) { return a + b; }

int main() {
    register_function("test_function", test_function);

    std::vector<uint8_t> bytes = serialize("test_function", 3, 5);
    msgpack::Unpacker unpacker(&bytes[0], bytes.size());
    std::string function_name;
    unpacker(function_name);
    std::cout<<function_name<<"\n";

    bytes = functions[function_name](unpacker);
    unpacker = {&bytes[0], bytes.size()};
    int result;
    unpacker(result);
    std::cout<<"result is: "<<result<<"\n";
}