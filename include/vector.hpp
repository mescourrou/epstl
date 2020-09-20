#pragma once

#include "container.hpp"

#include <cmath>
#include <initializer_list>

#ifdef USE_CUSTOM_STL
namespace epstl
{

/**
 * @brief Memory continuous dynamic array.
 *
 * @tparam T Type contained.
 * @tparam ALLOCATION_BATCH Size of allocations batches
 */
template <typename T, epstl::size_t ALLOCATION_BATCH = 5>
class vector : public linear_container<T>
{
    /**
     * @brief Vector iterator template class.
     *
     * @tparam IT_TYPE Type of return data (const T, T).
     * @tparam ASCENDING Is the iterator in the ascending way.
     */
    template<typename IT_TYPE, bool ASCENDING>
    class iteratorT
    {
      public:
        iteratorT(IT_TYPE* data) : m_data(data) {}

        iteratorT& operator++()
        {
            if (ASCENDING)
                m_data ++;
            else
                m_data --;
            return *this;
        }

        IT_TYPE* operator->() const
        {
            return m_data;
        }
        IT_TYPE* operator*() const
        {
            return m_data;
        }

        bool operator!=(const iteratorT<IT_TYPE, ASCENDING>& cmp) const
        {
            return m_data != cmp.m_data;
        }

      private:
        IT_TYPE* m_data;
    };

  public:

    /// Standard iterator.
    typedef iteratorT<T, true> iterator;
    /// Constant iterator.
    typedef iteratorT<const T, true> const_iterator;
    /// Reverse iterator.
    typedef iteratorT<T, false> reverse_iterator;
    /// Constant reverse iterator.
    typedef iteratorT<const T, false> const_reverse_iterator;

    /**
     * @brief Default constructor.
     */
    vector() = default;

    vector(std::initializer_list<T> list);
    ~vector() override;

    epstl::size_t push_back(T e) override;
    epstl::size_t pop_back() override;

    /**
     * @brief Get the number of items in the vector.
     */
    epstl::size_t size() const noexcept override
    {
        return m_size;
    }

    const T* at(int index) const noexcept override;
    T* at(int index) noexcept override;

    /**
     * @brief Get the value at the given index.
     *
     * @warning No control about the index is done.
     *
     * @param index Index where to take the value.
     * @return Constant value.
     */
    const T& operator[](int index) const
    {
        m_data[index];
    }

    /**
     * @brief Get the value at the given index.
     *
     * @warning No control about the index is done.
     *
     * @param index Index where to take the value.
     * @return Mutable value.
     */
    T& operator[](int index)
    {
        m_data[index];
    }

    /**
     * @brief Return the number of allocated slots.
     * @return Return the size of the allocated space.
     */
    epstl::size_t allocated() const noexcept
    {
        return m_allocated;
    }

    /**
     * @brief Get the begin iterator.
     */
    iterator begin() const
    {
        return iterator(m_data);
    }
    /**
     * @brief Get the end iterator.
     */
    iterator end() const
    {
        return iterator(m_data + m_size);
    }

    /**
     * @brief Get the begin constant iterator.
     */
    const_iterator cbegin() const
    {
        return const_iterator(m_data);
    }
    /**
     * @brief Get the end constant iterator.
     * @return
     */
    const_iterator cend() const
    {
        return const_iterator(m_data + m_size);
    }

    /**
     * @brief Get the begin reverse iterator.
     */
    reverse_iterator rbegin() const
    {
        return reverse_iterator(m_data + m_size - 1);
    }
    /**
     * @brief Get the end reverse iterator.
     * @return
     */
    reverse_iterator rend() const
    {
        return reverse_iterator(m_data ? m_data - 1 : nullptr);
    }

    void sort(bool ascending = true, bool (*less_operator)(const T&,
              const T&) = &less) override;
    void swap(epstl::size_t a, epstl::size_t b) override;

  private:
    /// Allocated slots.
    epstl::size_t m_allocated = 0;
    /// Used slots.
    epstl::size_t m_size = 0;
    /// C array.
    T* m_data = nullptr;

    void quick_sort(epstl::size_t begin, epstl::size_t end, bool ascending,
                    bool (*less_operator)(const T&, const T&) = &less);
    epstl::size_t partition(epstl::size_t begin, epstl::size_t end, bool ascending,
                            bool (*less_operator)(const T&, const T&) = &less);
};

/**
 * @brief Get the value at the given index.
 * @param index Index to get. If the index is negative, the size-index value is taken.
 * @return Constant pointer on the value. Null if out of bound.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
const T* vector<T, ALLOCATION_BATCH>::at(int index) const noexcept
{
    if (std::abs(index) >= m_size)
        return nullptr;
    if (index < 0)
        index = m_size - index;

    return &m_data[index];

}

/**
 * @brief Get the value at the given index.
 * @param index Index to get. If the index is negative, the size-index value is taken.
 * @return Mutable pointer on the value. Null if out of bound.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
T* vector<T, ALLOCATION_BATCH>::at(int index) noexcept
{
    if (std::abs(index) >= m_size)
        return nullptr;
    if (index < 0)
        index = m_size - index;

    return &m_data[index];
}

/**
 * @brief Constructor from initializer list.
 * @param list List of values to initialize the vector with.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
vector<T, ALLOCATION_BATCH>::vector(std::initializer_list<T> list)
{
    if (list.size() == 0)
        return;
    m_size = list.size();
    m_allocated = (std::ceil((float)m_size / (float)ALLOCATION_BATCH)) *
                  ALLOCATION_BATCH;
    m_data = new T[m_allocated];

    epstl::size_t i = 0;
    for (const auto& item : list)
    {
        m_data[i] = T(item);
        i++;
    }
}

/**
 * @brief Destructor.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
vector<T, ALLOCATION_BATCH>::~vector()
{
    delete[] m_data;
}

/**
 * @brief Add the item at the end of the vector.
 * @param e Element to add.
 * @return Return the new size of the vector.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
epstl::size_t vector<T, ALLOCATION_BATCH>::push_back(T e)
{
    if (m_size + 1 > m_allocated)
    {
        m_allocated = (std::ceil((float)(m_size + 1) / (float)ALLOCATION_BATCH)) *
                      ALLOCATION_BATCH;
        T* new_vector = new T[m_allocated];
        for (epstl::size_t i = 0; i < m_size; i++)
        {
            new_vector[i] = std::move(m_data[i]);
        }
        delete[] m_data;
        m_data = new_vector;
    }
    m_data[m_size] = std::move(e);

    return ++m_size;
}

/**
 * @brief Remove the last item of the vector.
 * @return Return the new size of the vector.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
epstl::size_t vector<T, ALLOCATION_BATCH>::pop_back()
{
    m_size--;
    if (m_size <= m_allocated - ALLOCATION_BATCH)
    {
        m_allocated = (std::ceil((float)m_size / (float)ALLOCATION_BATCH)) *
                      ALLOCATION_BATCH;
        T* new_vector = new T[m_allocated];
        for (epstl::size_t i = 0; i < m_size; i++)
        {
            new_vector[i] = std::move(m_data[i]);
        }
        delete[] m_data;
        m_data = new_vector;
    }
    return m_size;
}

/**
 * @brief Sort the vector using quick sort.
 * @param ascending Sort the vector ascendingly.
 * @param less_operator Less (<) operator.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
void vector<T, ALLOCATION_BATCH>::sort(bool ascending,
                                       bool (*less_operator)(const T&, const T&))
{
    quick_sort(0, m_size - 1, ascending, less_operator);
}

/**
 * @brief Swap the values at the two given indexes.
 * @param a Index of the first value.
 * @param b Index of the second velue.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
void vector<T, ALLOCATION_BATCH>::swap(size_t a, size_t b)
{
    T tmp(std::move(m_data[a]));
    m_data[a] = std::move(m_data[b]);
    m_data[b] = std::move(tmp);
}

/**
 * @brief Apply quick sort algorithm on the vector.
 * @param begin Begin index for the quick sort.
 * @param end End index for the quick sort.
 * @param ascending Sort the vector ascendingly.
 * @param less_operator Less (<) opeartor.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
void vector<T, ALLOCATION_BATCH>::quick_sort(size_t begin, size_t end,
        bool ascending, bool (*less_operator)(const T&, const T&))
{
    if (begin < end)
    {
        epstl::size_t pivot_index = partition(begin, end, ascending, less_operator);
        if (pivot_index >= 1)
            quick_sort(begin, pivot_index - 1, ascending, less_operator);
        quick_sort(pivot_index + 1, end, ascending, less_operator);
    }
}

/**
 * @brief Create a partition between the given index, using the last index as pivot.
 * @param begin Begin index of the partition.
 * @param end End index of the partition.
 * @param ascending Put the element higher than the pivot in the upper part of the partition.
 * @param less_operator Less (<) operator.
 * @return Return the index of the pivot.
 */
template<typename T, epstl::size_t ALLOCATION_BATCH>
size_t vector<T, ALLOCATION_BATCH>::partition(size_t begin, size_t end,
        bool ascending, bool (*less_operator)(const T&, const T&))
{
    // We choose the pivot as the last element of the partition
    T& pivot = m_data[end];

    epstl::ssize_t smallerElement = begin - 1;
    for (size_t i = begin; i < end; i++)
    {
        if (less_operator(m_data[i], pivot) == ascending)
        {
            smallerElement++;
            swap(smallerElement, i);
        }
    }
    swap(smallerElement + 1, end);
    return smallerElement + 1;
}


}

#else
#include <vector>
namespace epstl
{

template <class T>
using vector = std::vector<T>;

} // namespace epstl
#endif
