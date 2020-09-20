#pragma once

#include <type_traits>
#include <exception.hpp>

namespace epstl
{
/**
 * @brief Exception class for wrong value.
 */
class value_exception : public exception
{
  public:
    explicit value_exception() noexcept = default;
    explicit value_exception(const char* what) noexcept :
        exception(what) {}
};

/**
 * @brief Returns the absolute value of the number given.
 * @param nb Number from which getting the absolute value.
 * @tparam T Type of the parameters.
 */
template <typename T>
T abs(T nb)
{
    return nb > 0 ? nb : -nb;
}


/**
 * @brief Get the max of the two elements.
 *
 * The > operator need to be implemented.
 *
 * @param a First element.
 * @param b Second element.
 * @return Return the max between a and b.
 */
template <typename T>
T max(T a, T b)
{
    return a > b ? a : b;
}

/**
 * @brief Get the max of a list of elements.
 *
 * The > operator need to be implemented.
 *
 * @param a First element.
 * @param b Second element.
 * @param others Others elements.
 * @return Return the max all elements.
 */
#if __cplusplus >= 201702L
template < typename T, typename ...Args,
           std::enable_if_t < (std::is_same_v<T, Args>&& ...), int > = 0 >
#else
template < typename T, typename ...Args>
#endif
T max(T a, T b, Args... others)
{
    return max(a, max(b, others...));
}

/**
 * @brief Returns the modulo value of the number. In other words, it keep the value between zero and @a mod.
 *
 * To keep the value between two other values, use modulo2.
 * @tparam T Type of the values.
 * @param nb Number to keep between the bounds.
 * @param mod Upper bound.
 */
template<typename T>
T modulo(T nb, T mod)
{
    bool negative = false;
    mod = abs(mod);
    if (nb < 0)
    {
        nb = -nb;
        negative = true;
    }

    while (nb > mod)
    {
        nb -= mod;
    }

    if (negative)
        return -nb;
    else
        return nb;
}

/**
 * @brief Keeps the value of @a nb between the two bounds.
 *
 * @tparam T Type of the values.
 * @param nb Number to keep between the bounds.
 * @param modLow Lower bound.
 * @param modHigh Upper bound.
 */
template<typename T>
T modulo2(T nb, T modLow, T modHigh)
{
    if (modLow >= modHigh)
        throw epstl::value_exception("Lower bound of the modulo2 needs to be lower than the higher bound");
    if (nb < modLow)
    {
        while (nb < modLow)
        {
            nb += modHigh - modLow;
        }
        return nb;
    }
    else
    {
        while (nb >= modHigh)
        {
            nb -= modHigh - modLow;
        }
        return nb;
    }
}

} // namespace epstl
