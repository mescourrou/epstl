#pragma once

#include <functional>

#include <deque>
#include <semaphore.h>
#include <thread>
#include <condition_variable>
#include <atomic>

namespace epstl
{

class abstract_process
{
  public:
    abstract_process() = default;
    virtual ~abstract_process() = default;

    virtual void* operator()(void* in_object) = 0;
  protected:
    abstract_process* next_process = nullptr;
};

template <typename type_in, typename type_out>
class process : public abstract_process
{
  public:
    process() = default;
    process(std::function<type_out(type_in)> func) : m_func(func) {}
    ~process() override = default;

    void* operator()(void* in_object) override final;

  protected:
    virtual type_out exec(type_in in_object)
    {
        return m_func(in_object);
    }

  private:
    std::function<type_out(type_in)> m_func;
};

template <class T>
class pipeline
{
    struct process_list
    {
        abstract_process* p = nullptr;
        process_list* next_p = nullptr;
    };

  public:
    pipeline() = default;
    ~pipeline();

    void add_process(abstract_process* p)
    {
        if (!m_pipeline)
        {
            m_pipeline = new process_list{p, nullptr};
            m_lastProcess = m_pipeline;
        }
        else
        {
            process_list* new_process = new process_list{p, nullptr};
            m_lastProcess->next_p = new_process;
            m_lastProcess = new_process;
        }
        m_semaphores.push_back(new std::condition_variable());
        m_passing_arguments.push_back(nullptr);
        m_mutexes.push_back(new std::mutex);
        m_process_count++;
        m_threads.push_back(std::thread(&pipeline::execute_process, this, p,
                                        m_process_count - 1));

    }
    void feed(T first_element)
    {
        m_jobs_in_pipeline++;
        std::lock_guard<std::mutex> lock_waiting_list(m_waiting_list_access);
        m_waiting_list.push_back(first_element);
        if (m_semaphores.size() > 0)
        {
            m_semaphores.front()->notify_all();
        }
    }
    void stop()
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

    void wait_end()
    {
        std::mutex mtx;
        std::unique_lock<std::mutex> lock(mtx);
        while (m_jobs_in_pipeline != 0)
        {
            m_end_cv.wait(lock);
        }
        stop();
    }
  private:
    void execute_process(abstract_process* p, const size_t process_id);

    std::deque<T> m_waiting_list;
    std::mutex m_waiting_list_access;
    std::vector<std::thread> m_threads;
    std::vector<std::condition_variable*> m_semaphores;
    std::vector<void*> m_passing_arguments;
    std::vector<std::mutex*> m_mutexes;
    std::condition_variable m_end_cv;
    bool m_continue = true;
    std::atomic_size_t m_jobs_in_pipeline = 0;
    size_t m_process_count = 0;
    process_list* m_pipeline = nullptr;
    process_list* m_lastProcess = nullptr;
};

template<class T>
pipeline<T>::~pipeline()
{
    stop();
}

template<class T>
void pipeline<T>::execute_process(abstract_process* p, const size_t process_id)
{
    while (m_continue)
    {
        void* arg = nullptr;
        std::unique_lock<std::mutex> lock(*(m_mutexes.at(process_id)));
        if (process_id > 0)
        {
            if(!m_semaphores.at(process_id))
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
            if(m_passing_arguments.at(process_id) == nullptr)
                throw std::runtime_error("Argument disapeared");
            arg = m_passing_arguments.at(process_id);

        }
        else
        {
            if(!m_semaphores.front())
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
