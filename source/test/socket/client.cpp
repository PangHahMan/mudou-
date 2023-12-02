#include "../../server.hpp"

int main() {
    Socket client_sock;
    client_sock.Create_client(8888, "127.0.0.1");
    std::string str = "Hello World!";
    client_sock.Send(str.c_str(), str.size());  //发送给服务器
    char buffer[1024] = {0};
    client_sock.Recv(buffer, 1023);   //从服务器接收回来
    std::cout << buffer << std::endl;
    return 0;
}