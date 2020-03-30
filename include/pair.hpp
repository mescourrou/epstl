#pragma once

#include <utility>

namespace epstl
{
#ifdef USE_CUSTOM_STL

/**
 * @brief Standard pair structure
 */
template <typename T1, typename T2 = T1>
struct pair
{
    /**
     * @brief Default constructor
     */
    pair() = default;
    /**
     * @brief Construct the pair with the given values
     * @param first First value
     * @param second Second value
     */
    pair(T1 first, T2 second) : first(first), second(second) {}

#if __cplusplus >= 201702L
    // Tuple conversion
    template<std::size_t I>
    auto& get()
    {
        if constexpr (I == 0)
            return first;
        if constexpr (I == 1)
            return second;
    }

    template<std::size_t I>
    auto get() const
    {
        if constexpr (I == 0)
            return first;
        if constexpr (I == 1)
            return second;
    }
#endif

    T1 first;   ///< First value
    T2 second;  ///< Second value

};

} // namespace epstl

namespace std
{

template <class T1, class T2>
struct tuple_size<::epstl::pair<T1, T2>> : integral_constant<std::size_t, 2> {};

template<class T1, class T2>
struct tuple_element<0, ::epstl::pair<T1, T2>>
{
    using type = T1;
};

template<class T1, class T2>
struct tuple_element<1, ::epstl::pair<T1, T2>>
{
    using type = T2;
};
#else
template <class T1, class T2 = T1>
using pair = std::pair<T1, T2>;
#endif

}

