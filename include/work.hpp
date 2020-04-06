#pragma once

#include <functional>
#include <tuple>

#ifdef EPSTL_BUILD_TEST
#include <gtest/gtest.h>
#endif

namespace epstl
{

#ifdef EPSTL_BUILD_TEST
class WorkTest;
#endif

/**
 * @brief Base class for all templates implementation of Work
 */
class abstract_work_t
{
#ifdef EPSTL_BUILD_TEST
    friend class events::WorkTest;
#endif
  public:
    /// @brief Constructor
    abstract_work_t() = default;
    /// @brief Destructor
    virtual ~abstract_work_t() = default;

    /**
     * @brief Start the work
     */
    virtual void run() = 0;
};

/**
 * @class Work
 * @brief Template class for creating a work
 *
 * The template arguments take the type of the function arguments.
 *
 * Example:
 * @code
 * auto callback = [](std::string word, double number) {
 *     std::cout << word << " => " << number << std::endl;
 * };
 * Work<std::string, double> myWork(callback, "bla", 4.2);
 * myWork.run();
 * @endcode
 *
 * Output:
 * @code
 * bla => 4.2
 * @endcode
 */
#if __cplusplus >= 201702L
template<typename ...Args>
class work : public abstract_work_t
{
  public:
    /**
     * @brief Construct a work with the given function
     * @param func Function to do
     * @param arguments Arguments of the function
     */
    explicit work(const std::function<void(Args...)>& func,
                  Args... arguments) : m_func(func),
        m_arguments(arguments...) {}
    ~work() override = default;

    /**
     * @brief Run the job
     */
    void run() override
    {
        std::apply(m_func, m_arguments);
    }

  private:
    std::function<void(Args...)> m_func; ///< Function to activate in run method
    std::tuple<Args...> m_arguments; ///< Arguments of the function


};
#else
template<typename arg_t>
class work : public abstract_work_t
{
  public:
    /**
     * @brief Construct a work with the given function
     * @param func Function to do
     * @param arguments Arguments of the function
     */
    explicit work(const std::function<void(arg_t)>& func,
                  arg_t argument) : m_func(func),
        m_argument(argument) {}
    ~work() override = default;

    /**
     * @brief Run the job
     */
    void run() override
    {
        m_func(m_argument);
    }

  private:
    std::function<void(arg_t)> m_func; ///< Function to activate in run method
    arg_t m_argument; ///< Arguments of the function


};
#endif

/**
 * @brief Specialization of Work to manage functions without arguments.
 */
template<>
class work<void> : public abstract_work_t
{
  public:
    /**
     * @brief Construct a work with the given function
     * @param func Function to do
     */
    explicit work(const std::function<void(void)>& func) : m_func(func) {}
    ~work() override = default;
    /**
     * @brief Run the job
     */
    void run() override
    {
        m_func();
    }

  private:
    std::function<void(void)> m_func; ///< Function to activate in run method.
};

} // namespace epstl

