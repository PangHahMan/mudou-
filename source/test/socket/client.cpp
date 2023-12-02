#include "../../server.hpp"

int main() {
    Socket client_sock;
    client_sock.Create_client(8888, "127.0.0.1");
    std::string str = "Hello World!";
    client_sock.Send(str.c_str(), str.size());
    char buffer[1024] = {0};
    client_sock.Recv(buffer, 1023);
    std::cout << buffer << std::endl;
    return 0;
}