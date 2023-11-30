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

int main() {
    using Task = std::function<void()>;
    vector<Task> array;

    array.push_back(bind(print, "hello", 10));
    array.push_back(bind(print, "world", 20));

    for (auto &f: array) {
        f();
    }
    return 0;
}