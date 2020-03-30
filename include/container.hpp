#pragma once

#include <type_traits>
#include <_types.hpp>

/**
 * @namespace epstl
 * @brief Contains the custom stl
 */
namespace epstl
{


/**
 * @brief Main container class
 */
class container
{
  public:
    /**
     * @brief Default constructor
     */
    container() = default;
    /**
     * @brief Default destructor
     */
    virtual ~container() = default;

    /**
     * @brief Size of the container
     */
    virtual size_t size() const noexcept = 0;
};

/**
 * @brief Default less operator
 */
template <typename T1, typename T2>
bool less(const T1& t1, const T2& t2)
{
    return t1 < t2;
}

/**
 * @brief Sort order for linear_container
 */
enum sort_order
{
    ASCENDING, ///< A -> Z : Ascending sort
    DESCENDING ///< Z -> A : Descending sort
};

/**
 * @brief Linear container : container which has an single number index
 */
template <typename T>
class linear_container : public container
{
  public:
    /**
     * @brief Default constructor
     */
    linear_container() = default;
    /**
     * @brief Default destructor
     */
    virtual ~linear_container() = default;

    /**
     * @brief Push at the end of the container the value
     * @return Return the new size of the container
     */
    virtual size_t push_back(T) = 0;

    /**
     * @brief Remove the last item of the container
     * @return Return the new size of the container
     */
    virtual size_t pop_back() = 0;

    /**
     * @brief Sort the container
     * @param ascending Sort the container ascendingly
     * @param less_operator Less (<) operator
     */
    virtual void sort(bool ascending = true,
                      bool (*less_operator)(const T&, const T&) = &less) = 0;
    /**
     * @brief Get the value at the given index
     * @param index Index to look for
     * @return Return a const pointer on the value
     */
    virtual const T* at(int index) const noexcept = 0;
    /**
     * @brief Get the value at the given index
     * @param index Index to look for
     * @return Return a mutable pointer on the value
     */
    virtual T* at(int index) noexcept = 0;
    /**
     * @brief Swap the items at the given indexes
     * @param a Index of the first item
     * @param b Index of the second item
     */
    virtual void swap(epstl::size_t a, epstl::size_t b) = 0;
};

} // namespace epstl
