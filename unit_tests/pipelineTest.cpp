#include "pipelineTest.hpp"
#include "pipeline.hpp"

#include <iostream>
#include <string>

namespace epstl
{

TEST_F(pipelineTest, Test)
{
    try
    {
        pipeline<int> pip;
        pip.add_process(new process<int, int>([](int i)
        {
            std::cout << "Process 1 get " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            return i;
        }));
        pip.add_process(new process<int, int>([](int i)
        {
            std::cout << "Process 2 get " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            return i;
        }));
        pip.add_process(new process<int, int>([](int i)
        {
            std::cout << "Process 3 get " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return i;
        }));

        pip.feed(1);
        pip.feed(2);
        pip.feed(3);
        pip.wait_end();
        std::cout << "END" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

}

} // namespace epstl
