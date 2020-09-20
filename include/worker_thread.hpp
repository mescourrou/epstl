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
 * @brief Manage work and allocate them to separated threads.
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
     * @brief Destructor : wait for all threads to finish.
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
     * @brief Add a new work.
     *
     * @param w Function to call for work.
     * @param arguments Arguments of the function.
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
    /**
     * @brief Add a new work (no arguments).
     *
     * @param w Function to call for work.
     */
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
    /**
     * @brief Add a new work (method version).
     *
     * @param object_instance Instance object to be used when calling the method.
     * @param method Method of object_instance to be called for work.
     * @param arguments Arguments of the function.
     */
    template<typename I, typename M, typename ...Args>
    static void new_work(I* object_instance, M method, Args... arguments)
    {
        new_work([object_instance, method, arguments...]()
        {
            std::bind(method, object_instance, arguments...)();
        });
    }
    /**
     * @brief Add a new work (method version, no arguments).
     *
     * @param object_instance Instance object to be used when calling the method.
     * @param method Method of object_instance to be called for work.
     */
    template<typename I, typename M>
    static void new_work(I* objectInstance, M method)
    {
        new_work([objectInstance, method]()
        {
            std::bind(method, objectInstance)();
        });
    }

    /**
     * @brief Wait for all threads to finish.
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
    /**
     * @brief Singleton instance getter.
     * @return Return the singleton instance.
     */
    static worker_thread& getInstance()
    {
        static worker_thread instance;
        return instance;
    }

    /**
     * @brief Default constructor.
     */
    worker_thread() = default;

    /**
     * @brief Worker main function.
     *
     * Start the first_work and then start the works of the waiting list.
     * @param [in] first_work First work to start.
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
     * It is only used if there is not enough thread available at the moment.
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

