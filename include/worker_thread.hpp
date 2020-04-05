#pragma once

//STL
#include <list>
#include <thread>
#include <mutex>
#include <functional>
#include <memory>

#include "work.hpp"

#ifdef EPSTL_BUILD_TEST
#include <gtest/gtest.h>
#endif

namespace epstl
{

#ifdef EPSTL_BUILD_TEST
class WorkerThreadTest;
#endif

/**
 * @brief Manage Work and allocate them to separated threads.
 *
 * The number of thread is limited by the number of thread of the device.
 */
class worker_thread
{
#ifdef EPSTL_BUILD_TEST
    friend class epstl::WorkerThreadTest;
#endif
  public:
    /**
     * @brief Destructor : wait for all threads to finish
     */
    ~worker_thread()
    {
        wait_for_join();
    }
    worker_thread(const worker_thread&) = delete;
    worker_thread(worker_thread&&) = delete;
    worker_thread& operator=(const worker_thread&) = delete;
    worker_thread& operator=(worker_thread&&) = delete;
    /**
     * @fn void WorkerThread::newWork(std::function<void(Args...)> work, Args... arguments)
     * @brief Add a new work
     *
     * @param work Function to call for work
     * @param arguments Arguments of the function
     */
    template<typename ...Args>
    static void new_work(const std::function<void(Args...)>& w,
                         Args... arguments)
    {
        m_mutex.lock();
        if (m_active_threads >= max_threads)
            m_waiting_list.push_back(std::make_shared<work<Args...>>(work<Args...>(w,
                                     arguments...)));
        else
        {
            m_active_threads++;
            getInstance().m_workers.push_back(std::thread(worker,
                                              std::make_shared<work<Args...>>(work<Args...>(w, arguments...))));
        }
        m_mutex.unlock();
    }
    static void new_work(const std::function<void()>& w)
    {
        m_mutex.lock();
        if (m_active_threads >= max_threads)
            m_waiting_list.push_back(std::make_shared<work<void>>(work<void>(w)));
        else
        {
            m_active_threads++;
            getInstance().m_workers.push_back(std::thread(worker,
                                              std::make_shared<work<void>>(work<void>(w))));
        }
        m_mutex.unlock();
    }

    template<typename I, typename M, typename ...Args>
    static void new_work(I* object_instance, M func, Args... arguments)
    {
        new_work([object_instance, func, arguments...]()
        {
            std::bind(func, object_instance, arguments...)();
        });
    }
    template<typename I, typename M>
    static void new_work(I* objectInstance, M func)
    {
        new_work([objectInstance, func]()
        {
            std::bind(func, objectInstance)();
        });
    }

    /**
     * @brief Wait for all threads to finish
     */
    static void wait_for_join()
    {
        while (getInstance().m_workers.size() != 0)
        {
            getInstance().m_workers.back().join();
            getInstance().m_workers.pop_back();
        }
    }

  private:
    static worker_thread& getInstance()
    {
        static worker_thread instance;
        return instance;
    }
    /**
     * @brief Worker main function.
     *
     * Start the firstWork and then start the works of the waiting list
     * @param [in] firstWork First work to start
     */
    worker_thread() = default;
    /**
     * @brief Add a new work to the working list. Start it immedialty if it is possible
     * @param [in] work Work to add
     */
    static void worker(std::shared_ptr<abstract_work_t> first_work)
    {
        first_work->run();
        m_mutex.lock();
        while (m_waiting_list.size() > 0)
        {
            auto work = m_waiting_list.front();
            m_waiting_list.pop_front();
            m_mutex.unlock();
            work->run();
            work.reset();
            m_mutex.lock();
        }
        m_active_threads--;
        m_mutex.unlock();
    }

    /**
     * @brief Work waiting list.
     *
     * It is only used if there is not enough thread available at the moment
     */
    static inline std::list<std::shared_ptr<abstract_work_t>> m_waiting_list;

    /// Thread list
    std::vector<std::thread> m_workers;
    /// Number of active threads
    static inline unsigned int m_active_threads = 0;
    /// Maximum number of threads
    static const inline unsigned int max_threads =
        std::thread::hardware_concurrency();
    /// Time to wait before closing a inactive thread
    static const inline unsigned int m_expiration_time_ms = 1000;
    /// Mutex
    static inline std::mutex m_mutex;
};


} // namespace epstl

