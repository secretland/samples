#include <iostream>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

boost::mutex mutex;
boost::condition_variable cv;

void foo()
{
    std::cout  << __FUNCTION__ << " : " << __LINE__ << ". thread id: " << std::hex << boost::this_thread::get_id() << std::endl;
    boost::lock_guard<boost::mutex> lock(mutex);
    //boost::mutex::scoped_lock lock(mutex);
    std::cout << __FUNCTION__ << ". thread sleep..." << std::endl;
    boost::this_thread::sleep_for(boost::chrono::seconds(5));
    std::cout << __FUNCTION__ << ". thread wake up" << std::endl;
    cv.notify_one();
}

void bar()
{
    std::cout  << __FUNCTION__ << " : " << __LINE__ << ". thread id: " << std::hex << boost::this_thread::get_id() << std::endl;
    boost::mutex::scoped_lock lock(mutex);
    std::cout << __FUNCTION__ << ". thread waiting notify other thread..." << std::endl;
    cv.wait(lock);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(1000));
    std::cout << __FUNCTION__ << ". notify received" << std::endl;
}

int main(int, char**)
{
    std::cout << __FUNCTION__ << ". create threads" << std::endl;
    boost::thread_group thr_gr;
    thr_gr.create_thread(bar);
    boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
    thr_gr.create_thread(foo);
    thr_gr.join_all();
    //std::cout << __FUNCTION__ << ". threads end" << std::endl;
    std::cout << "application end" << std::endl;
    return 0;
}
