#pragma once
#include <functional>
#include <iostream>
#include <sys/epoll.h>

class Channel {
    using EventCallback = std::function<void()>;

private:
    int _fd;
    uint32_t _events;             //当前需要监控的事件
    uint32_t _revents;            //当前连接触发的事件
    EventCallback _read_callback; //可读事件被触发的回调函数
    EventCallback _write_callback;//可写事件被触发的回调函数
    EventCallback _error_callback;//错误事件被触发的回调函数
    EventCallback _close_callback;//连接关闭事件被触发的回调函数
    EventCallback _event_callback;//任意事件被触发的回调函数
public:
    Channel(int fd)
        : _fd(fd), _events(0), _revents(0) {}
    //获取文件描述符
    int Fd() {
        return _fd;
    }
    //设置events
    void set_events(uint32_t events) {
        _events = events;
    }
    void set_read_callback(const EventCallback &callback) {
        _read_callback = callback;
    }
    void set_write_callback(const EventCallback &callback) {
        _write_callback = callback;
    }
    void set_error_callback(const EventCallback &callback) {
        _error_callback = callback;
    }
    void set_close_callback(const EventCallback &callback) {
        _close_callback = callback;
    }
    void set_event_callback(const EventCallback &callback) {
        _event_callback = callback;
    }
    //当前是否监控了可读
    bool read_able() {
        return _revents & EPOLLIN;  //& 检查_revents变量是否设置了EPOLLIN标志
    }
    //当前是否监控了可写
    bool write_able(){
        return _revents & EPOLLOUT;
    }  ewwwwwww.eeew3qrfew3qrfew3qeqasdsassssssadasdrerqwerwewrewrre

    void enable_read();  //启动读事件监控
   11 void enable_write(); //启动写事件监控
    void disable_read(); //停止读事件监控
    void333.....................0.............................................                                        vcxvxcxvvvvvdddddxcf c disable_write();//停止写事件监控
    void disable_all();  //停止所有事件监控
    void remove();       //移除所有事件监控
    void handle_event(); //处理事件,一旦连接触发了事件，就调用这个函数
};