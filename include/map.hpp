#pragma once

#include "container.hpp"
#include "math.hpp"
#include "pair.hpp"

#include <stdexcept>
#include <utility>

#if __cplusplus >= 201702L
#define CONSTEXPR_IF constexpr
#else
#define CONSTEXPR_IF
#endif
#ifdef USE_CUSTOM_STL

namespace epstl
{
/**
 * @brief Key based map
 */
template <typename key_t, typename item_t>
class map : public container
{
  private:
    /**
     * @brief Map tree node
     */
    struct node_t
    {
        node_t* left_node = nullptr; ///< Left subtree
        node_t* right_node = nullptr; ///< Right subtree
        node_t* parent = nullptr; ///< Parent (nullptr for the root)

        epstl::pair<key_t, item_t> content; ///< Key-value storage
    };

    /**
     * @brief Types of iterators, for template specialization
     */
    enum iterator_types
    {
        KEY_ORDER,  ///< A -> Z order
        KEY_REVERSE_ORDER ///< Z -> A order
    };

    /**
     * @brief Template of the iterator
     *
     * @tparam ret_t Type of return (epstl::pair<key_t, item_t> constant or not)
     * @tparam it_node_t Type of node (constant or not)
     * @tparam it_type Type of iterator (cf iterator_types enum)
     */
    template<typename ret_t, typename it_node_t, iterator_types it_type = KEY_ORDER>
    class iterator_t
    {
      public:
        /**
         * @brief Constructor
         * @param start_node First node to start with
         */
        iterator_t(it_node_t* start_node) : m_current_node(start_node) {}

        /**
         * @brief Pre-increment operator
         * @return Return the incremented iterator
         */
        iterator_t& operator++()
        {
            if CONSTEXPR_IF (it_type == KEY_ORDER)
            {
                // Climb the tree
                if (!m_current_node->right_node && m_current_node->parent
                        && m_current_node == m_current_node->parent->left_node)
                    m_current_node = m_current_node->parent;
                else if (m_current_node->right_node)
                    m_current_node = min_node(m_current_node->right_node);
                else
                    m_current_node = nullptr;

            }
            else if CONSTEXPR_IF (it_type == KEY_REVERSE_ORDER)
            {
                // Climb the tree
                if (!m_current_node->left_node && m_current_node->parent
                        && m_current_node == m_current_node->parent->right_node)
                    m_current_node = m_current_node->parent;
                else if (m_current_node->left_node)
                    m_current_node = max_node(m_current_node->left_node);
                else
                    m_current_node = nullptr;
            }
            return *this;
        }

        /**
         * @brief Decrement operator
         * @todo Map decrement operator to implement
         * @return Return the decremented iterator
         */
        iterator_t& operator--()
        {

        }

        /**
         * @brief Star access operator
         * @return Reference on the current element
         */
        ret_t& operator*()
        {
            return m_current_node->content;
        }

        /**
         * @brief Pointer access operator
         * @return Return a pointer on the current element
         */
        ret_t* operator->()
        {
            return &m_current_node->content;
        }

        /**
         * @brief Star access operator
         * @return Reference on the current element
         */
        ret_t& operator*() const
        {
            return m_current_node->content;
        }

        /**
         * @brief Pointer access operator
         * @return Return a pointer on the current element
         */
        ret_t* operator->() const
        {
            return &m_current_node->content;
        }

        /**
         * @brief Comparison operator
         * @param it Iterator to compare with
         * @return Return true if the two are differents
         */
        bool operator!=(const iterator_t& it) const
        {
            return m_current_node != it.m_current_node;
        }

        /**
         * @brief Bool operator to test if a map node is accessible
         */
        operator bool()
        {
            return m_current_node;
        }
      private:
        it_node_t* m_current_node; ///< Current used node

    };
  public:
    /// Standard iterator
    using iterator = iterator_t<epstl::pair<key_t, item_t>, node_t, KEY_ORDER>;
    /// Standard constant iterator
    using const_iterator =
        iterator_t<const epstl::pair<key_t, item_t>, const node_t, KEY_ORDER>;
    /// Reverse iterator
    using reverse_iterator =
        iterator_t<epstl::pair<key_t, item_t>, node_t, KEY_REVERSE_ORDER>;
    /// Constant reverse iterator
    using const_reverse_iterator =
        iterator_t<const epstl::pair<key_t, item_t>, const node_t, KEY_REVERSE_ORDER>;

    /**
    * @brief Default constructor
    */
    map() = default;
    /**
     * @brief Create a map with the given less operator
     * @param less_operator Less (<) operator to use
     */
    map(bool (*less_operator)(const key_t& k1, const key_t& k2)) :
        m_less_operator(less_operator) {}

    ~map() override;

    size_t size() const noexcept override;

    item_t& operator[](const key_t& key);
    bool insert(key_t key, item_t item);

    /**
     * @brief Get the height of the map tree
     * @return
     */
    epstl::size_t height() const noexcept
    {
        return height(m_root);
    }

    const item_t* at(const key_t& key) const noexcept;
    item_t* at(const key_t& key) noexcept;
    size_t erase(const key_t& key);

    /**
     * @brief Get the standard begin iterator
     */
    iterator begin()
    {
        return iterator(min_node(m_root));
    }

    /**
     * @brief Get the standard end iterator
     */
    iterator end()
    {
        return iterator(nullptr);
    }

    /**
     * @brief Get the standard begin constant iterator
     */
    const_iterator begin() const
    {
        return const_iterator(min_node(m_root));
    }

    /**
     * @brief Get the standard end constant iterator
     */
    const_iterator end() const
    {
        return const_iterator(nullptr);
    }

    /**
     * @brief Get the reverse begin iterator
     */
    reverse_iterator rbegin()
    {
        return reverse_iterator(max_node(m_root));
    }

    /**
     * @brief Get the reverse end iterator
     */
    reverse_iterator rend()
    {
        return reverse_iterator(nullptr);
    }

    /**
     * @brief Get the reverse begin constant iterator
     */
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(max_node(m_root));
    }

    /**
     * @brief Get the reverse begin constant iterator
     */
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(nullptr);
    }

  private:
    bool free_recursive(node_t* node);
    bool insert_recursive(node_t* current_node, key_t& key, item_t& item) noexcept;
    bool erase_recursive(node_t* current_node, const key_t& key);
    epstl::size_t height(node_t* root) const noexcept;
    void balance_node(node_t* node) noexcept;
    auto search(const key_t& key) const noexcept -> map<key_t, item_t>::node_t*;

    void left_rotate(node_t* node) noexcept;
    void right_rotate(node_t* node) noexcept;

    static node_t* min_node(node_t* node);
    static const node_t* min_node(const node_t* node);
    static node_t* max_node(node_t* node);
    static const node_t* max_node(const node_t* node);

    /// Less (<) operator to use
    bool (*m_less_operator)(const key_t& k1, const key_t& k2) = &less<key_t, key_t>;
    epstl::size_t m_size = 0; ///< Size of the map

    node_t* m_root = nullptr; ///< Root node of the tree
};

/**
 * @brief Destructor which free the contained values
 * @todo Destructor to implement
 */
template<typename key_t, typename item_t>
map<key_t, item_t>::~map()
{

}

/**
 * @brief Get the size of the map : number of items inside
 */
template <typename key_t, typename item_t>
size_t map<key_t, item_t>::size() const noexcept
{
    return m_size;
}

/**
 * @brief Get the value at the given key
 * @todo To implement
 */
template<typename key_t, typename item_t>
item_t& map<key_t, item_t>::operator[](const key_t& key)
{

}

/**
 * @brief Insert the item at the given key
 * @param key Key of the item
 * @param item Item to insert
 * @return Return true if the insertion was successful
 */
template<typename key_t, typename item_t>
bool map<key_t, item_t>::insert(key_t key, item_t item)
{
    if (!m_root)
    {
        node_t* new_node = new node_t;
        new_node->content.first = std::move(key);
        new_node->content.second = std::move(item);
        m_root = new_node;
        m_size++;
        return true;
    }

    if (!insert_recursive(m_root, key, item))
        return false;
    // Equilibrate the tree
    short diff = height(m_root->left_node) - height(m_root->right_node);
    if (diff < -1)
        left_rotate(m_root);
    else if (diff > 1)
        right_rotate(m_root);
    return true;
}

/**
 * @brief Get a const pointer on the item at the given key
 * @param key Key to look for
 * @return Const pointer on the value or nullptr if the key was not found
 */
template<typename key_t, typename item_t>
const item_t* map<key_t, item_t>::at(const key_t& key) const noexcept
{
    node_t* node = search(key);
    if (node)
        return &node->payload;

    return nullptr;
}

/**
 * @brief Get a mutable pointer on the item at the given key
 * @param key Key to look for
 * @return Mutable pointer on the value or nullptr if the key was not found
 */
template<typename key_t, typename item_t>
item_t* map<key_t, item_t>::at(const key_t& key) noexcept
{
    node_t* node = search(key);
    if (node)
        return &node->content.second;

    return nullptr;
}

/**
 * @brief Erase the given key
 * @param key Key to erase
 * @return Return the new size of the map
 */
template<typename key_t, typename item_t>
size_t map<key_t, item_t>::erase(const key_t& key)
{
    if (erase_recursive(m_root, key))
        balance_node(m_root);
    return m_size;
}

/**
 * @brief Free the tree behind the given root
 * @param node Root of the tree to free
 * @return Return true if the node was free
 */
template<typename key_t, typename item_t>
bool map<key_t, item_t>::free_recursive(map::node_t* node)
{
    if (!node)
        return false;
    free_recursive(node->left_node);
    free_recursive(node->right_node);
    if (node->parent)
    {
        if (node->parent->left_node == node)
            node->parent->left_node = nullptr;
        else if (node->parent->right_node == node)
            node->parent->right_node = nullptr;
    }
    delete node;
    return true;
}

/**
 * @brief Compute the height of the tree with the given root
 * @param root Root of the tree where to compute the height
 */
template<typename key_t, typename item_t>
size_t map<key_t, item_t>::height(node_t* root) const noexcept
{
    if (!root)
        return 0;
    return epstl::max(height(root->left_node), height(root->right_node)) + 1;
}

/**
 * @brief Balance given node
 *
 * Does a left or a right rotation if a branch is heavier than the other.
 * This method is not recursive, it does not balance the whole tree.
 *
 * @param node Node to balance
 */
template<typename key_t, typename item_t>
void map<key_t, item_t>::balance_node(map::node_t* node) noexcept
{
    if (!node)
        return;
    short diff = height(node->left_node) - height(node->right_node);
    if (diff < -1)
        left_rotate(node);
    else if (diff > 1)
        right_rotate(node);
}

/**
 * @brief Recusive version of insert
 * @param current_node Root of the tree to insert into
 * @param key Key of the item
 * @param item Item to insert
 * @return Return true if the item was inserted
 */
template<typename key_t, typename item_t>
bool map<key_t, item_t>::insert_recursive(node_t* current_node, key_t& key,
        item_t& item) noexcept
{
    if (current_node->content.first == key)
        return false;
    if (m_less_operator(key, current_node->content.first))
    {
        if (current_node->left_node)
            current_node = current_node->left_node;
        else
        {
            node_t* new_node = new node_t;
            new_node->content.first = std::move(key);
            new_node->content.second = std::move(item);
            new_node->parent = current_node;
            current_node->left_node = new_node;
            m_size++;
            return true;
        }
    }
    else
    {
        if (current_node->right_node)
            current_node = current_node->right_node;
        else
        {
            node_t* new_node = new node_t;
            new_node->content.first = std::move(key);
            new_node->content.second = std::move(item);
            new_node->parent = current_node;
            current_node->right_node = new_node;
            m_size++;
            return true;
        }
    }
    if (!insert_recursive(current_node, key, item))
        return false;

    balance_node(current_node);
    return true;

}

/**
 * @brief Erase the given key in the tree with the given root
 *
 * @param current_node Root of the tree where to find the key and erase it
 * @return Return true if the key was found in the tree and was erased
 */
template<typename key_t, typename item_t>
bool map<key_t, item_t>::erase_recursive(map::node_t* current_node,
        const key_t& key)
{
    if (!current_node)
        return false;

    if (current_node->content.first == key)
    {
        if (current_node->left_node || current_node->right_node)
        {
            if (!current_node->left_node) // Only the right node
            {
                // Set parent -> child link
                if (current_node->parent->left_node == current_node)
                    current_node->parent->left_node = current_node->right_node;
                else if (current_node->parent->right_node == current_node)
                    current_node->parent->right_node = current_node->right_node;

                // Set child -> parent link
                if (current_node->right_node)
                    current_node->right_node->parent = current_node->parent;
            }
            else if (!current_node->right_node) // Only the left node
            {
                // Set parent -> child link
                if (current_node->parent->left_node == current_node)
                    current_node->parent->left_node = current_node->left_node;
                else if (current_node->parent->right_node == current_node)
                    current_node->parent->right_node = current_node->left_node;

                // Set child -> parent link
                if (current_node->left_node)
                    current_node->left_node->parent = current_node->parent;
            }
            else // Left and right node
            {
                node_t* min = min_node(current_node->right_node);
                if (min->parent)
                    min->parent->left_node = nullptr;
                min->parent = current_node->parent;
                min->left_node = current_node->left_node;

                if (min->right_node)
                    max_node(min->right_node)->right_node = current_node->right_node;
                else
                    min->right_node = current_node->right_node;

                if (current_node->parent->left_node == current_node)
                    current_node->parent->left_node = min;
                else if (current_node->parent->right_node == current_node)
                    current_node->parent->right_node = min;
            }
        }
        else if (current_node->parent)
        {
            if (current_node->parent->left_node == current_node)
                current_node->parent->left_node = nullptr;
            else if (current_node->parent->right_node == current_node)
                current_node->parent->right_node = nullptr;
        }
        delete current_node;
        current_node = nullptr;
        m_size--;
        return true;
    }

    if (erase_recursive(current_node->left_node, key) ||
            erase_recursive(current_node->right_node, key))
    {
        balance_node(current_node);
        return true;
    }
    return false;
}

/**
 * @brief Search for the key and return the node found
 * @param key Key to look for
 * @return Pointer on the note. Null if not found
 */
template<typename key_t, typename item_t>
auto map<key_t, item_t>::search(const key_t& key) const noexcept ->
map<key_t, item_t>::node_t*
{
    node_t* current_node = m_root;
    while (current_node)
    {
        if (current_node->content.first == key)
            return current_node;
        if (m_less_operator(key, current_node->content.first))
            current_node = current_node->left_node;
        else
            current_node = current_node->right_node;
    }
    return nullptr;
}

/**
 * @brief Do a left rotation on the given node
 * @param node Node to rotate
 */
template<typename key_t, typename item_t>
void map<key_t, item_t>::left_rotate(map::node_t* node) noexcept
{
    if (!node || !node->right_node)
        return;
    node_t* pivot = node->right_node;

    node->right_node = pivot->left_node;
    if (node->right_node)
        node->right_node->parent = node;

    pivot->left_node = node;

    node_t* node_parent = node->parent;
    if (node->parent)
    {
        if (node->parent->left_node == node)
            node->parent->left_node = pivot;
        else
            node->parent->right_node = pivot;
    }
    else
        m_root = pivot;
    node->parent = pivot;
    pivot->parent = node_parent;
}

/**
 * @brief Do a right rotation on the given node
 * @param node Node to rotate
 */
template<typename key_t, typename item_t>
void map<key_t, item_t>::right_rotate(map::node_t* node) noexcept
{
    if (!node || !node->left_node)
        return;
    node_t* pivot = node->left_node;

    node->left_node = pivot->right_node;
    if (node->left_node)
        node->left_node->parent = node;

    pivot->right_node = node;

    node_t* node_parent = node->parent;
    if (node->parent)
    {
        if (node->parent->left_node == node)
            node->parent->left_node = pivot;
        else
            node->parent->right_node = pivot;
    }
    else
        m_root = pivot;
    node->parent = pivot;
    pivot->parent = node_parent;
}

/**
 * @brief Get the node with the biggest key of the given tree
 */
template<typename key_t, typename item_t>
auto map<key_t, item_t>::max_node(map::node_t* node) -> map::node_t*
{
    if (node->right_node)
        return max_node(node->right_node);
    return node;
}

/**
 * @brief Get the node with the biggest key of the given tree
 */
template<typename key_t, typename item_t>
auto map<key_t, item_t>::max_node(const map::node_t* node) -> const map::node_t*
{
    if (node->right_node)
        return max_node(node->right_node);
    return node;
}

/**
 * @brief Get the node with the smallest key of the given tree
 */
template<typename key_t, typename item_t>
auto map<key_t, item_t>::min_node(map::node_t* node) -> map::node_t*
{
    if (node->left_node)
        return min_node(node->left_node);
    return node;
}

/**
 * @brief Get the node with the smallest key of the given tree
 */
template<typename key_t, typename item_t>
auto map<key_t, item_t>::min_node(const map::node_t* node) -> const map::node_t*
{
    if (node->left_node)
        return min_node(node->left_node);
    return node;
}

} // namespace epstl

#else
#include <map>
#endif
