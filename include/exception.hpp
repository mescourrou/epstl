#pragma once

#ifdef USE_CUSTOM_STL
#define BASE_STL_EXCEPTION
#else
#include <exception>
#define BASE_STL_EXCEPTION : public std::exception
#endif

namespace epstl
{
/**
 * @brief Base exception for epstl.
 */
class exception BASE_STL_EXCEPTION
{
  public:
    /**
     * @brief Default constructor.
     */
    explicit exception() noexcept = default;

    /**
     * @brief Construct an exeption with a what string.
     * @param what String explaining the exception.
     */
    explicit exception(const char* what) noexcept : m_what(what) {}

    /**
     * @brief Get the what string.
     */
    virtual const char* what() const noexcept final
    {
        return m_what;
    }
  private:
    const char* m_what =
        ""; ///< String containing the explaination of the exception.
};

/**
 * @brief Error due to a bad implementation.
 *
 * For example, when a case should not occurs.
 */
class implementation_exception : public exception
{
  public:
    /**
     * @brief Default constructor.
     */
    explicit implementation_exception() noexcept = default;
    /**
     * @brief Constructor of exception with explanation.
     */
    explicit implementation_exception(const char* what) noexcept :
        exception(what) {}
};

}
