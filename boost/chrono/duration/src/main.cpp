#include <iostream>
#include <boost/chrono.hpp>
#include <boost/thread.hpp>

#include <stdint.h>

static uint16_t value = 5;

int main(int, char**)
{
    //auto start = boost::chrono::system_clock::now();
    auto start = boost::chrono::system_clock::from_time_t(time(NULL));
    std::cout << "Sleep " << value << " seconds..." << std::endl;
    boost::this_thread::sleep_for(boost::chrono::seconds(value) + boost::chrono::milliseconds(123));

    auto end = boost::chrono::system_clock::now();

    auto duration = end - start;

    std::cout << "duration: " << boost::chrono::duration_cast<boost::chrono::milliseconds>(end - start) << std::endl;

    if(duration > boost::chrono::seconds(value + 1))
        std::cout << "TRUE!" << std::endl;
    else
        std::cout << "FALSE!" << std::endl;

    return 0;
}
