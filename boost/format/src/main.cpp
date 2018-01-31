#include <iostream>
#include <boost/format.hpp>
#include <stdint.h>

int main(int, char**)
{
    uint32_t ip = 0xC0A8C864;
    std::cout << "ip: " << std::hex << std::showbase << ip << ". string: " 
              << boost::str(boost::format("%d.%d.%d.%d") % (ip >> 24) % ((ip & 0x00FF0000) >> 16) % ((ip & 0x0000FF00) >> 8) % (ip & 0x000000FF)) << std::endl;
    return 0;
}
