#include "../../server.hpp"

int main() {
    // 创建一个 Socket 对象用于监听连接
    Socket listen_sock;
    listen_sock.Create_server(8888);

    while (true) {
        // 接受一个新的连接
        int newfd = listen_sock.Accept();
        if (newfd < 0) {
            continue;
        }

        // 创建一个 Socket 对象用于与客户端通信
        Socket client_sock(newfd);

        // 创建一个缓冲区用于接收客户端发送的数据
        char buffer[1024] = {0};

        // 从newfd接收客户端发送的数据
        int ret = client_sock.Recv(buffer, 1023);
        if (ret < 0) {
            // 如果接收数据出错，关闭与客户端的连接
            client_sock.Close();
            continue;
        }

        // 将接收到的数据发送回客户端
        client_sock.Send(buffer, ret);

        // 关闭与客户端的连接
        client_sock.Close();
    }

    // 关闭监听连接的 Socket
    listen_sock.Close();

    return 0;
}