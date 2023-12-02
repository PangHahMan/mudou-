#pragma once
#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>
#define BUFFER_DEFAULT_SIZE 1024

class Buffer {
private:
    std::vector<char> _buffer;//使用vector进行内存空间管理
    uint64_t _read_offset;    //读偏移
    uint64_t _write_offset;   //写偏移
public:
    Buffer()
        : _read_offset(0), _write_offset(0), _buffer(BUFFER_DEFAULT_SIZE) {}
    // 拷贝构造函数
    Buffer(const Buffer &other)
        : _buffer(other._buffer),
          _read_offset(other._read_offset),
          _write_offset(other._write_offset) {
    }
    //赋值重载
    Buffer &operator=(const Buffer &other) {
        if (this != &other) {
            _buffer = other._buffer;
            _read_offset = other._read_offset;
            _write_offset = other._write_offset;
        }
        return *this;
    }
    //获取buffer首地址
    char *begin() {
        return &(*_buffer.begin());//begin是迭代器 需要解引用后在取地址
    }
    //获取当前写入起始地址，_buffer的空间起始地址，加上写偏移量
    char *get_write_start_address() {
        return begin() + _write_offset;
    }
    //获取当前读取起始地址，_buffer的空间起始地址，加上读偏移量
    char *get_read_start_address() {
        return begin() + _read_offset;
    }
    //获取缓冲区末尾空闲空间大小--写偏移之后的空闲空间，总体空间大小减去写偏移
    uint64_t get_remaining_write_space() {
        return _buffer.size() - _write_offset;
    }
    //获取缓冲区起始空闲空间大小--读偏移之前的空闲空间
    uint64_t get_remaining_read_space() {
        return _read_offset;
    }
    //获取可读数据大小 = 写偏移 - 读偏移
    uint64_t get_readable_data_size() {
        return _write_offset - _read_offset;
    }
    //将读偏移向后移动
    void move_read_offset_forward(uint64_t len) {
        //读偏移向后移动的大小，必须小于可读数据的大小
        assert(len <= get_readable_data_size());
        _read_offset += len;
    }
    //将写偏移向后移动
    void move_write_offset_forward(uint64_t len) {
        //写偏移向后移动的大小，必须小于当前后边空闲的大小
        assert(len <= get_remaining_write_space());
        _write_offset += len;
    }
    //确保可写空间足够(整体空闲空间够了就移动数据，否则就扩容)
    void ensure_sufficient_writable_space(uint64_t len) {
        //如果末尾空间大小足够，直接返回
        if (get_remaining_write_space() >= len) {
            return;
        }
        //末尾空间大小不够，则判断加上起始位置的空间大小是否足够，够了就将数据移动到起始位置
        if (len <= get_remaining_read_space() + get_remaining_write_space()) {
            //将数据移动到起始位置
            uint64_t rsz = get_readable_data_size();
            std::copy(get_read_start_address(), get_write_start_address() + rsz, begin());
            _read_offset = 0;
            _write_offset = rsz;
        } else {
            // 不够则扩容空间，通常扩展到所需大小的两倍，以避免频繁扩容
            uint64_t new_capacity = std::max(_buffer.size() * 2, _write_offset + len);
            _buffer.resize(new_capacity);
        }
    }
    //写入数据
    void write(const void *data, uint64_t len) {
        //1.保证有足够空间
        //2.拷贝数据进去
        ensure_sufficient_writable_space(len);
        const char *d = (const char *) data;
        //拷贝len字节数据，从d到d+len
        std::copy(d, d + len, get_write_start_address());
    }
    //写入数据+更改偏移量
    void write_push(const void *data, uint64_t len) {
        write(data, len);
        move_write_offset_forward(len);//写偏移
    }
    //写入字符串数据
    void write_as_string(const std::string &data) {
        return write(data.c_str(), data.size());
    }
    //写入字符串数据 + 偏移量
    void write_as_string_push(const std::string &data) {
        write_as_string(data);
        move_write_offset_forward(data.size());
    }
    //从另一个Buffer中写入数据
    void write_buffer(Buffer &data) {
        return write(data.get_read_start_address(), data.get_readable_data_size());
    }
    //从另一个Buffer中写入数据+更改偏移量
    void write_buffer_push(Buffer &data) {
        write_buffer(data);
        //移动的写偏移量就是Buffer中可读数据的大小
        move_write_offset_forward(data.get_readable_data_size());
    }
    //读取数据
    void read(void *buf, uint64_t len) {
        //要求要获取的数据大小必须小于可读数据的大小
        assert(len <= get_readable_data_size());
        //将可读的首地址到len长度数据读取到buf中
        std::copy(get_read_start_address(), get_read_start_address() + len, (char *) buf);
    }
    //读取数据后增加读偏移
    void read_pop(void *buf, uint64_t len) {
        read(buf, len);
        move_read_offset_forward(len);
    }
    //读取的数据作为string返回
    std::string read_as_string(uint64_t len) {
        //要求要获取的数据大小必须小于可读数据大小
        assert(len <= get_readable_data_size());
        std::string str;
        str.resize(len);
        read(&str[0], len);//这里Read的参数是非const类型的，而str.cstr()是const类型的，所以要采用这种方式
        return str;
    }
    //读取后增加偏移量
    std::string read_as_string_pop(uint64_t len) {
        std::string str = read_as_string(len);
        move_read_offset_forward(len);
        return str;
    }
    //找换行符第一次出现地址
    char *find_line() {
        char *res = (char *) memchr(get_read_start_address(), '\n', get_readable_data_size());
        return res;
    }
    //获取一行数据
    std::string get_Line_data() {
        char *pos = find_line();
        if (pos == nullptr) {
            return "";
        }
        // +1是为了把换行字符也取出来。
        return read_as_string(pos - get_read_start_address() + 1);
    }
    std::string get_Line_data_pop() {
        std::string str = get_Line_data();
        move_read_offset_forward(str.size());
        return str;
    }
    //清空缓冲区
    void clear() {
        //只需要清零两个偏移量
        _read_offset = 0;
        _write_offset = 0;
    }
};