#include <iostream>
#include <regex>
#include <string>

int main() {
    std::string str = "/numbers/1234";
    //匹配以 /numbers/起始，后边跟了一个或多个数字字符的字符串
    std::regex e("/numbers/(\\d+)");
    std::smatch matches;

    bool ret = std::regex_match(str, matches, e);
    if (ret == false) {
        return -1;
    }

    for (auto &s: matches) {
        std::cout << s << std::endl;
    }

    return 0;
}
