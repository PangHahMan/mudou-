#pragma once
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#define LISTEM_MAX 1024

class Socket {
private:
    int _sockfd;

public:
    Socket()
        : _sockfd(-1) {}
    Socket(int fd)
        : _sockfd(fd) {}
    ~Socket() {
        close(_sockfd);
    }
    //创建套接字
    bool Create() {
        //int socket(int domain, int type, int protocol);
        // AF_INET表示IPV4,SOCK_STREAM面向字节流， protocol为0: 表示选择默认的协议。根据 domain 和 type 参数选择适当的协议。
        _sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_sockfd < 0) {
            std::cout << "socket create failed" << std::endl;
            return false;
        }
        return true;
    }
    //绑定地址信息
    bool Bind(const std::string &ip, uint16_t port) {
        //sockaddr_in 用于表示IPV4地址，sockaddr表示任何类型,bind的接口参数是sockaddr,需要强转
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        //inet_addr  是将IP地址转换为网络字节序的二进制形式的函数
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        //htons 将16位无符号整数从主机字节序转换为网络字节序
        addr.sin_port = htons(port);
        socklen_t len = sizeof(struct sockaddr_in);
        int ret = bind(_sockfd, (struct sockaddr *) &addr, len);
        if (ret < 0) {
            std::cout << "socket bind failed" << std::endl;
            return false;
        }
        return true;
    }
    //开始监听
    bool Listen(int backlog = LISTEM_MAX) {
        int ret = listen(_sockfd, backlog);
        if (ret < 0) {
            std::cout << "socket listen failed" << std::endl;
            return false;
        }
        return true;
    }
    //向服务器发起连接
    bool Connect(const std::string &ip, uint16_t port) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip.c_str());
        addr.sin_port = htons(port);
        socklen_t len = sizeof(struct sockaddr_in);
        int ret = connect(_sockfd, (struct sockaddr *) &addr, len);
        if (ret < 0) {
            std::cout << "socket connect failed" << std::endl;
            return false;
        }
        return true;
    }
    //获取新连接
    int Accept() {
        //用于接受一个新的连接请求，并创建一个新的套接字来处理该连接。
        struct sockaddr_in addr;
        socklen_t len = sizeof(sockaddr_in);
        //addr是用于存储连接请求的客户端的地址信息。
        int connfd = accept(_sockfd, (struct sockaddr *) &addr, &len);
        if (connfd < 0) {
            std::cout << "socket accept failed" << std::endl;
            return -1;
        }
        return connfd;
    }
    //接收数据
    ssize_t Recv(void *buf, size_t len, int flag = 0) {
        //从_sockfd中接收数据到buf中
        ssize_t ret = recv(_sockfd, buf, len, flag);
        if (ret <= 0) {
            //EAGAIN 当前socket的接收缓冲区中没有数据了，在非阻塞的情况下才会有这个错误
            //EINTR  表示当前socket的阻塞等待，被信号打断了
            if (errno == EAGAIN || errno == EINTR) {
                return 0;//表示这次接收没有接收到数据
            }
            std::cout << "socket recv failed" << std::endl;
            return -1;
        }
        return ret;
    }
    ssize_t Recv_non_block(void *buf, size_t len) {
        return Recv(buf, len, MSG_DONTWAIT);//MSG_DONTWAIT 表示当前接收为非阻塞。
    }
    //发送数据
    ssize_t Send(const void *buf, size_t len, int flag = 0) {
        //将buf中的数据发送到_sockfd
        ssize_t ret = send(_sockfd, buf, len, flag);
        if (ret < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                return 0;
            }
            std::cout << "socket send failed" << std::endl;
            return -1;
        }
        return ret;//实际发送的数据长度
    }
    ssize_t Send_non_block(void *buf, size_t len) {
        if (len == 0) {
            return 0;
        }
        return Send(buf, len, MSG_DONTWAIT);
    }
    //关闭套接字
    void Close() {
        if (_sockfd != -1) {
            close(_sockfd);
            _sockfd = -1;
        }
    }
    //设置套接字选项--开启地址端口重用- 允许多个套接字访问一个端口号
    void Reuse_address() {
        int val = 1;//1表示开启SO_REUSEADDR  ,0表示关闭
        //SO_REUSEADDR  表示允许重复使用本地地址 SO_REUSEPORT  表示允许重复使用本地端口
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &val, sizeof(int));
        setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, (void *) &val, sizeof(int));
    }
    //设置套接字阻塞属性 -- 设置为非阻塞
    void Non_block() {
        int flag = fcntl(_sockfd, F_GETFL, 0);
        fcntl(_sockfd, F_SETFL, flag | O_NONBLOCK);
    }
    //创建一个服务端连接
    bool Create_server(uint16_t port, const std::string &ip = "0.0.0.0", bool block_flag = false) {
        //1.创建套接字
        if (Create() == false) {
            return false;
        }
        //2.设置非阻塞
        if (block_flag) {
            Non_block();
        }
        //3.绑定套接字
        if (Bind(ip, port) == false) {
            return false;
        }
        //4.监听套接字
        if (Listen() == false) {
            return false;
        }
        //5.开启地址端口重用
        Reuse_address();
        return true;
    }
    //创建一个客户端连接
    bool Create_client(uint16_t port, const std::string &ip) {
        //1. 创建套接字
        if (Create() == false) {
            return false;
        }
        //2.指向连接服务器
        if (Connect(ip, port) == false) {
            return false;
        }
        return true;
    }
};