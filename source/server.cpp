#include "server.hpp"
void test_buffer1() {
    Buffer buf;
    std::string str = "Hello!";
    buf.write_as_string_push(str);

    Buffer buf1;
    buf1.write_buffer_push(buf);

    std::string tmp;
    tmp = buf1.read_as_string_pop(buf1.get_readable_data_size());

    std::cout << tmp << std::endl;                          //Hello!
    std::cout << buf.get_readable_data_size() << std::endl; //6
    std::cout << buf1.get_readable_data_size() << std::endl;//6
}
//测试扩容
void test_buffer2() {
    Buffer buf;
    //默认字节是1024 这里循环300次 一次str是9字节
    for (int i = 0; i < 300; i++) {
        std::string str = "Hello!" + std::to_string(i) + '\n';
        buf.write_as_string_push(str);
    }

    while (buf.get_readable_data_size() > 0) {
        std::string line = buf.get_Line_data_pop();
        std::cout << line << std::endl;
    }
}

int main() {
    test_buffer2();
    return 0;
}