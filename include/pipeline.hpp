#pragma once

#include <functional>

#include <deque>
#include <semaphore.h>
#include <thread>
#include <condition_variable>

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

    void* operator()(void* in_object) override final
    {
        return (void*)(new type_out(exec(type_in(*((type_in*)in_object)))));
    }

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
        m_running.push_back(false);
        m_threads.push_back(std::thread(&pipeline::execute_process, this, p,
                                        m_process_count));
        m_process_count++;
        m_last_item_position = m_process_count;

    }
    void feed(T first_element)
    {
        m_waiting_list.push_back(first_element);
        m_last_item_position = 0;
        if (m_semaphores.size() > 0)
            m_semaphores.front()->notify_all();
    }
    void stop()
    {
        m_continue = false;
        for (auto* s : m_semaphores)
        {
            s->notify_all();
        }
    }

    void wait_end()
    {
        std::mutex mt;
        std::unique_lock<std::mutex> lock(mt);
        while (m_last_item_position != m_process_count)
        {
            m_end_cv.wait(lock);
        }
        stop();
    }
  private:
    void execute_process(abstract_process* p, const size_t process_id);

    std::deque<T> m_waiting_list;
    std::vector<std::thread> m_threads;
    std::vector<std::condition_variable*> m_semaphores;
    std::vector<void*> m_passing_arguments;
    std::vector<std::mutex*> m_mutexes;
    std::vector<bool> m_running;
    std::condition_variable m_end_cv;
    bool m_continue = true;
    size_t m_process_count = 0;
    size_t m_last_item_position = 0;
    process_list* m_pipeline = nullptr;
    process_list* m_lastProcess = nullptr;
};

template<class T>
pipeline<T>::~pipeline()
{
    for (auto& t : m_threads)
        t.join();
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
            m_semaphores.at(process_id)->wait(lock, [this, process_id]()
            {
                return m_passing_arguments.at(process_id) != nullptr || !m_continue;
            });
            if (!m_continue)
            {
                return;
            }
            arg = m_passing_arguments.at(process_id);
        }
        else
        {
            m_semaphores.at(process_id)->wait(lock, [this, process_id]()
            {
                return m_waiting_list.size() > 0 || !m_continue;
            });
            if (!m_continue)
            {
                return;
            }
            arg = new T(m_waiting_list.front());
            m_waiting_list.pop_front();
        }
        void* ret = (*p)(arg);
        if (process_id + 1 < m_process_count)
        {
            std::unique_lock<std::mutex> lockNext(*m_mutexes.at(process_id + 1));
            m_passing_arguments.at(process_id + 1) = ret;
            lockNext.unlock();
            m_passing_arguments.at(process_id) = nullptr;
            lock.unlock();
            m_semaphores.at(process_id + 1)->notify_all();
        }
        else
        {
            m_passing_arguments.at(process_id) = nullptr;
            lock.unlock();
        }
        if (m_last_item_position == process_id && m_waiting_list.empty())
        {
            m_last_item_position++;
            m_end_cv.notify_all();
        }
    }
}

} // namespace epstl
