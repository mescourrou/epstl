#include "pipelineTest.hpp"
#include "pipeline.hpp"

#include <iostream>
#include <string>

namespace epstl
{

TEST_F(pipelineTest, Test)
{
    std::mutex mtx1;
    std::mutex mtx2;
    std::mutex mtx3;
    std::mutex mtx_exec_times;
    std::array<int, 3> exec_times{0, 0, 0};
    pipeline<std::string> pip;
    pip.add_process(new process<std::string, double>([&](std::string i)
    {
        EXPECT_TRUE(mtx1.try_lock());
        std::cout << "Process 1 get " << i << std::endl;
        {
            std::lock_guard<std::mutex> lock(mtx_exec_times);
            exec_times.at(0)++;

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        mtx1.unlock();
        return std::atof(i.c_str());
    }));
    pip.add_process(new process<double, std::string>([&](double i)
    {
        EXPECT_TRUE(mtx2.try_lock());
        std::cout << "Process 2 get " << i << std::endl;
        {
            std::lock_guard<std::mutex> lock(mtx_exec_times);
            exec_times.at(1)++;

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        mtx2.unlock();
        return std::to_string(i);
    }));
    pip.add_process(new process<std::string, void>([&](std::string i)
    {
        EXPECT_TRUE(mtx3.try_lock());
        std::cout << "Process 3 get " << i << std::endl;
        {
            std::lock_guard<std::mutex> lock(mtx_exec_times);
            exec_times.at(2)++;

        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        mtx3.unlock();
    }));

    pip.feed("1");
    pip.feed("2");
    pip.feed("3");
    pip.feed("4");
    pip.wait_end();
    std::cout << "END" << std::endl;
    std::lock_guard<std::mutex> lock(mtx_exec_times);
    for (unsigned int i = 0; i < exec_times.size(); i++)
    {
        SCOPED_TRACE(std::string("process number : ").append(std::to_string(i + 1)));
        EXPECT_EQ(exec_times.at(i), 4);
    }
    //    std::this_thread::sleep_for(std::chrono::milliseconds(30));

}

} // namespace epstl
