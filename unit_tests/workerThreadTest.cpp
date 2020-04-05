#include "workerThreadTest.hpp"
namespace epstl
{

/*
 * Test adding new work to the worker thread
 */
TEST_F(WorkerThreadTest, NewWork)
{
    mutex.lock();
    unsigned int cbCalls = 0;
    auto cb = [&cbCalls]()
    {
        cbCalls++;
        usleep(500);
    };

    worker_thread::new_work(cb); // 1
    worker_thread::new_work(cb); // 2
    worker_thread::new_work(cb); // 3
    worker_thread::new_work(cb); // 4
    worker_thread::new_work(cb); // 5
    worker_thread::new_work(cb); // 6
    worker_thread::new_work(cb); // 7
    worker_thread::new_work(cb); // 8
    worker_thread::new_work(cb); // 9
    worker_thread::new_work(cb); // 10
    worker_thread::new_work(cb); // 11
    worker_thread::new_work(cb); // 12
    worker_thread::new_work(cb); // 13
    worker_thread::new_work(cb); // 14
    worker_thread::new_work(cb); // 15

    usleep(100000);

    EXPECT_EQ(cbCalls, 15);
    mutex.unlock();
}

/*
 * Test waiting for all threads to finish
 */
TEST_F(WorkerThreadTest, WaitForJoin)
{
    mutex.lock();
    bool active = false;
    auto cb = [&]()
    {
        active = true;
        usleep(1000);
        active = false;
    };

    worker_thread::new_work(cb);
    usleep(50);
    EXPECT_TRUE(active);
    worker_thread::wait_for_join();
    EXPECT_FALSE(active);
    mutex.unlock();
}

}


