#include <iostream>
#include <stdint.h>
#include <algorithm>
#include <memory>

class Foo
{
public:
    Foo(std::size_t size, uint8_t* data)
    : data_(new uint8_t[size]),
      size_(size)
    {
        std::cout << "Foo(std::size_t, uint8_t*). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
        std::copy_n(data, size, data_);
    }

    Foo(std::size_t size)
    : data_(new uint8_t[size]),
      size_(size)
    {
        std::cout << "Foo(std::size_t). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
    }

    Foo(Foo const& other)
    : size_(other.size_),
      data_(new uint8_t[size_])
    {
        std::cout << "Foo(Foo const&). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
        std::copy_n(other.data_, other.size_, data_);
    }

    Foo(Foo&& other)
    : data_(other.data_),
      size_(other.size_)
    {
        std::cout << "Foo(Foo&&). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
        other.data_ = nullptr;
        other.size_ = 0;
    }

    ~Foo()
    {
        std::cout << "~Foo(). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
        delete[] data_;
        data_ = nullptr;
        size_ = 0;
    }


private:
    uint8_t* data_;
    std::size_t size_;
};

class Bar
{
public:
    Bar()
    : foo_(nullptr)
    {
        std::cout << "Bar(). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
    }

    Bar(std::size_t size)
    : foo_(new Foo(size))
    {
        std::cout << "Bar(std::size_t). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
    }
    Bar(Bar const& other)
    : foo_(new Foo(other.foo_ != nullptr ? *other.foo_ : 0))
    {
        std::cout << "Bar(Bar const&). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
    }
    Bar(Bar&& other)
    : foo_(other.foo_)
    {
        std::cout << "Bar(Bar&&). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << std::endl;
        other.foo_ = nullptr;
    }
    ~Bar()
    {
        std::cout << "~Bar(). this = 0x" << std::hex << reinterpret_cast<uint32_t>(this) << ". foo_ = 0x" << reinterpret_cast<uint32_t>(foo_) << std::endl;
        delete foo_;
    }
private:
    Foo* foo_;
};

int main(int, char**)
{
    {
        std::vector<Bar> bar_vec;
        Bar bar(10);
        bar_vec.push_back(bar);
    }
    std::cout << "-= move semantics =-" << std::endl;
    {
        std::vector<Bar> bar_vec;
        bar_vec.push_back(Bar(10));
    }
    std::cout << "-= emplace_back =-" << std::endl;
    {
        std::vector<Bar> bar_vec;
        bar_vec.emplace_back(10);
    }
    std::cout << "-= std::move & unique_ptr =-" << std::endl;
    {
        std::vector<std::unique_ptr<Bar>> bar_vec;
        std::unique_ptr<Bar> bar(new Bar(10));
        bar_vec.push_back(std::move(bar));

    }
    return 0;
}
