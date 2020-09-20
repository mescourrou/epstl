#pragma once

#include <functional>

#include <deque>
#include <semaphore.h>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace epstl
{
/**
 * @brief Base class for the template class process.
 */
class abstract_process
{
  public:
    /**
     * @brief Default constructor.
     */
    abstract_process() = default;
    /**
     * @brief Default destructor.
     */
    virtual ~abstract_process() = default;

    /**
     * @brief Execute the process.
     * @param in_object Object to give to the process.
     * @return Return of the procecss, as a void*.
     */
    virtual void* operator()(void* in_object) = 0;
};

/**
 * @brief Basic implementation of processes. Can be inherited for more complex usages.
 *
 * To use, simply give the callback in the constructor parameter. To call it, use the operator() method.
 *
 * About the transmission between threads:
 * - The types need to be copyable, they are copied between threads (or moved if possible).
 * - The types are not verified at compile time, be sure that the types are castable.
 *
 * @code
 * process<std::string, double> p([&](std::string i)
 * {
 *      std::cout << "Process get " << i << std::endl;
 *      return std::atof(i.c_str());
 * });
 * @endcode
 *
 * @warning The return type 'void' is supported only with c++17 and after. The entry type 'void' is not supported yet.
 * @warning There is no type verification due to the use of void*. Be careful !
 *
 * @tparam type_in Type taken as parameter of the callback.
 * @tparam type_out Type returned by the callback.
 */
template <typename type_in, typename type_out>
class process : public abstract_process
{
  public:
    /**
     * @brief Create a process with the given callback.
     * @param func Function called by the process.
     */
    process(std::function<type_out(type_in)> func) : m_func(func) {}
    /**
     * @brief Default destructor.
     */
    ~process() override = default;

    void* operator()(void* in_object) override final;

  protected:
    /**
     * @brief Default constructor. Can only be used for derived classes.
     */
    process() = default;
    /**
     * @brief exec Execute the process. The conversion of types is done in operator().
     * @param in_object Object given as arguments.
     * @return Return the return of the function.
     */
    virtual type_out exec(type_in in_object)
    {
        return m_func(in_object);
    }

  private:
    /// Process body function
    std::function<type_out(type_in)> m_func;
};

/**
 * @brief Multithread pipeline class.
 *
 * @code
 * pipeline<std::string> pip;
 * pip.add_process(new process<std::string, double>([&](std::string i)
 * {
 *      std::cout << "Process 1 get " << i << std::endl;
 *      std::this_thread::sleep_for(std::chrono::milliseconds(100));
 *      return std::atof(i.c_str());
 * }));
 * pip.add_process(new process<double, std::string>([&](double i)
 * {
 *      std::cout << "Process 2 get " << i << std::endl;
 *      std::this_thread::sleep_for(std::chrono::milliseconds(200));
 *      return std::to_string(i);
 * }));
 * pip.feed("1");
 * pip.feed("2");
 * pip.wait_end();
 * @endcode
 *
 * @tparam T Type of the entry point.
 */
template <class T>
class pipeline
{
  public:
    /**
     * @brief Default constructor.
     */
    pipeline() = default;
    ~pipeline();

    void add_process(abstract_process* p);
    void feed(T first_element);
    void stop();

    void wait_end();
  private:
    void execute_process(abstract_process* p, const size_t process_id);

    /// Waiting list for fed jobs.
    std::deque<T> m_waiting_list;
    /// Mutex to access m_waiting_list.
    std::mutex m_waiting_list_access;
    /// List of the process pointers.
    std::vector<abstract_process*> m_process;
    /// List of threads.
    std::vector<std::thread> m_threads;
    /// Conditions variables to communicate between threads.
    std::vector<std::condition_variable*> m_semaphores;
    /// Arguments passed between process.
    std::vector<void*> m_passing_arguments;
    /// Thread mutex protection.
    std::vector<std::mutex*> m_mutexes;
    /// End of pipeline trigger.
    std::condition_variable m_end_cv;
    /// Continue the threads.
    bool m_continue = true;
    /// Count of the jobs in the pipeline and the waiting list.
    std::atomic_size_t m_jobs_in_pipeline = 0;
    /// Number of processes.
    size_t m_process_count = 0;
};

/**
 * @brief Destructor, stoping the pipeline.
 */
template<class T>
pipeline<T>::~pipeline()
{
    stop();
    for (abstract_process* p : m_process)
        delete p;
}

/**
 * @brief Add a process to the pipeline.
 * @warning There is not type verification, the type out of the previous process must be convertible into the type in of the new process.
 * @param p New process. The process is own by the pipeline (will be destroyed in the destructor).
 */
template<class T>
void pipeline<T>::add_process(abstract_process* p)
{
    m_process.push_back(p);
    m_semaphores.push_back(new std::condition_variable());
    m_passing_arguments.push_back(nullptr);
    m_mutexes.push_back(new std::mutex);
    m_process_count++;
    m_threads.push_back(std::thread(&pipeline::execute_process, this, p,
                                    m_process_count - 1));

}

/**
 * @brief Feed the pipeline with a new element. The element will be submitted to the first process when available.
 * @warning If the feeding frequency is too high regarding the time treatment of the slowest process, the waiting list will grow indefinitely.
 *
 * @param first_element Element to be given to the first process.
 */
template<class T>
void pipeline<T>::feed(T first_element)
{
    m_jobs_in_pipeline++;
    std::lock_guard<std::mutex> lock_waiting_list(m_waiting_list_access);
    m_waiting_list.push_back(first_element);
    if (m_semaphores.size() > 0)
    {
        m_semaphores.front()->notify_all();
    }
}

/**
 * @brief Stop the pipeline.
 */
template<class T>
void pipeline<T>::stop()
{
    m_continue = false;
    for (auto* s : m_semaphores)
    {
        s->notify_all();
    }
    for (auto& t : m_threads)
    {
        if (t.joinable())
            t.join();
    }
    for (auto* m : m_mutexes)
    {
        m->lock();
        m->unlock();
    }
}

/**
 * @brief Wait the end of the pipeline (empty waiting list and all element treated).
 */
template<class T>
void pipeline<T>::wait_end()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    while (m_jobs_in_pipeline != 0)
    {
        m_end_cv.wait(lock);
    }
    stop();
}

/**
 * @brief Execute the process given, and manage the communication between threads.
 * @param p Process to execute.
 * @param process_id Id of the process in the pipeline.
 */
template<class T>
void pipeline<T>::execute_process(abstract_process* p, const size_t process_id)
{
    while (m_continue)
    {
        void* arg = nullptr;
        std::unique_lock<std::mutex> lock(*(m_mutexes.at(process_id)));
        if (process_id > 0)
        {
            if (!m_semaphores.at(process_id))
                throw std::runtime_error(std::string("Null semaphore #") + std::to_string(
                                             process_id));
            m_semaphores.at(process_id)->wait(lock, [this, process_id]()
            {
                return m_passing_arguments.at(process_id) != nullptr || !m_continue;
            });
            if (!m_continue)
            {
                return;
            }
            if (m_passing_arguments.at(process_id) == nullptr)
                throw std::runtime_error("Argument disappeared");
            arg = m_passing_arguments.at(process_id);

        }
        else
        {
            if (!m_semaphores.front())
                throw std::runtime_error("Null semaphore #0");
            m_semaphores.front()->wait(lock, [this]()
            {
                m_waiting_list_access.lock();
                if (m_waiting_list.size() > 0)
                    return true;
                m_waiting_list_access.unlock();
                return !m_continue;
            });
            if (!m_continue)
            {
                return;
            }
            else
            {
                arg = new T(m_waiting_list.front());
                m_waiting_list.pop_front();
                m_waiting_list_access.unlock();
            }

        }
        void* ret = (*p)(arg);
        if (process_id + 1 < m_process_count)
        {
            std::lock_guard<std::mutex> lockNext(*m_mutexes.at(process_id + 1));
            m_passing_arguments.at(process_id + 1) = ret;

            m_passing_arguments.at(process_id) = nullptr;
            lock.unlock();
            m_semaphores.at(process_id + 1)->notify_all();
        }
        else
        {
            m_jobs_in_pipeline--;
            m_passing_arguments.at(process_id) = nullptr;
            lock.unlock();
            m_end_cv.notify_all();
        }
    }
}

/**
 * @brief Execute the process and convert the void* into the types in and out.
 * @param in_object Object to give to the process.
 * @return Return the returns of the process.
 */
template < typename type_in, typename type_out>
void* process<type_in, type_out>::operator()(void* in_object)
{
    // If not in c++17, void type for type_out won't work
#if __cplusplus >= 201702L
    if constexpr (std::is_void_v<type_out>)
    {
        exec(std::move(type_in(*((type_in*)in_object))));
        return nullptr;
    }
    else
#endif
        return (void*)(new type_out(std::move(exec(type_in(*((type_in*)in_object))))));
}

} // namespace epstl
