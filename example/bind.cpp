#include <functional>
#include <iostream>
#include <string>
#include <vector>

using std::bind;
using std::cout;
using std::endl;
using std::vector;

void print(const std::string &str, int num) {
    cout << str << " " << num << endl;
}

void test1() {
    auto func = bind(print, "hello", std::placeholders::_1);
    func(5);
}

void test2() {
    using Task = std::function<void()>;
    vector<Task> array;

    array.push_back(bind(print, "hello", 10));
    array.push_back(bind(print, "world", 20));

    for (auto &f: array) {
        f();
    }
}

int main() {
    test2();
    return 0;
}