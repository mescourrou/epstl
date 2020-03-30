#pragma once

#include <initializer_list>
#include <type_traits>
#include <_types.hpp>

namespace epstl
{
#if __cplusplus >= 201702L
template<size_t k_size, typename k_type = double, typename = typename std::enable_if_t<(k_size > 0)
         >>
#else
template<size_t k_size, typename k_type = double, typename = typename std::enable_if<(k_size > 0)>::type
>
#endif
class Point
{
  public:
    Point() = default;
    Point(std::initializer_list<k_type> coordinates)
    {
        // Too many coordinates given in initializer_list
        assert(coordinates.size() <= k_size);
        size_t i = 0;
        for (auto& c : coordinates)
        {
            m_coordinates[i] = c;
            i++;
        }
    }

    k_type& operator[](size_t i)
    {
        return m_coordinates[i];
    }

    const k_type& operator[](size_t i) const
    {
        return m_coordinates[i];
    }

    constexpr size_t size() const noexcept
    {
        return k_size;
    }

    /// @brief Get and modify the x coordinate (0)
#if __cplusplus >= 201702L
    template<typename = std::enable_if_t<(k_size >= 1) >>
#endif
    k_type & x() noexcept
    {
        return m_coordinates[0];
    }
    /// @brief Get and modify the y coordinate (1)
#if __cplusplus >= 201702L
    template<typename = std::enable_if_t<(k_size >= 2) >>
#endif
    k_type & y() noexcept
    {
        return m_coordinates[1];
    }
    /// @brief Get and modify the z coordinate (2)
#if __cplusplus >= 201702L
    template<typename = std::enable_if_t<(k_size >= 3) >>
#endif
    k_type & z() noexcept
    {
        return m_coordinates[2];
    }
    /// @brief Get the x coordinate (0)
#if __cplusplus >= 201702L
    template<typename = std::enable_if_t<(k_size >= 1) >>
#endif
    k_type x() const noexcept
    {
        return m_coordinates[0];
    }
    /// @brief Get the y coordinate (1)
#if __cplusplus >= 201702L
    template<typename = std::enable_if_t<(k_size >= 2) >>
#endif
    k_type y() const noexcept
    {
        return m_coordinates[1];
    }
    /// @brief Get the z coordinate (2)
#if __cplusplus >= 201702L
    template<typename = std::enable_if_t<(k_size >= 3) >>
#endif
    k_type z() const noexcept
    {
        return m_coordinates[2];
    }

  private:
    k_type m_coordinates[k_size];
};

} // namespace epstl
