#include <iostream>

int main(int, char**)
{
    auto x = [](std::string const& text, int value) { std::cout << text << value << std::endl; };
    auto value = 10;
    x("text value: ", value);
    return 0;
}
