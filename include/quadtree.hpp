#pragma once

#include <initializer_list>
#include <ostream>
#include <functional>
#include <iostream>

#include "container.hpp"
#include "exception.hpp"
#include "math.hpp"
#include "pair.hpp"

namespace epstl
{

/**
 * @enum behaviour_t
 * @brief Flags for quadtree behaviour
 */
enum behaviour_t
{
    quadtree_no_replace = 1,        ///< Do not replace data when using insert
    quadtree_multithread = 1 << 1   ///< Use multithread operation when possible
};

/**
 * @brief Point quadtree
 *
 * Create a quadtree structure with one point maximum by quadrant.
 *
 * The item contained in the quadtree has to have a default value. By
 * default, the default value is the default constructor of the type.
 * If this value is set by the user, it will be invisible.
 *
 * Example :
 * @code
 * // Create a quadtree with integers as keys and containing chars
 * // The size of the tree is 20 by 20, centered on zero.
 * epstl::quadtree<int, char> tree(20, 20);
 *
 * // Insert 'a' into the tree
 * epstl::size_t tree_size = tree.insert(5, 5, 'a');
 * // Get the value at 5,5
 * int value = tree.at(5, 5); // returns 'a'
 *
 * tree_size = tree.insert(3, 3, 'b');
 *
 * epstl::pair<int> keys;
 * tree.find('c', keys); // Return false
 * tree.find('a', keys); // Return true. keys contains now 5,5
 *
 * // Remove items
 * tree.remove_all(100); // remove 5,5
 * tree.remove(3, 3); // remove 110
 * @endcode
 */
template<typename key_t, typename item_t>
class quadtree : public container
{
  protected:
    /**
     * @brief Carthesian position
     */
    struct position_t
    {
        key_t x = 0;
        key_t y = 0;
        key_t z = 0;
    };

    /**
     * @brief Rectangular bound with center position
     */
    struct rect_bound_t
    {
        key_t left = 0;
        key_t right = 0;
        key_t top = 0;
        key_t bottom = 0;

        position_t center;

        /**
         * @brief Tells if the coordinates are inside the bounds
         * @param x X coordinate of the point to test
         * @param y Y coordinate of the point to test
         * @return True if the point is inside
         */
        bool isInside(key_t x, key_t y) const
        {
            return x >= left && x < right && y >= bottom && y < top;
        }
    };

    /**
     * @brief Recursive quadrant structure for point quadtree
     */
    struct quadrant_t
    {
        item_t data;
        position_t data_position;
        rect_bound_t bound;
        quadrant_t* ne = nullptr;
        quadrant_t* nw = nullptr;
        quadrant_t* sw = nullptr;
        quadrant_t* se = nullptr;
        quadrant_t* parent = nullptr;
    };

  public:
    /**
     * @brief Construct a quadtree with the given center and width/height
     * @param center_x X coordinate of the center
     * @param center_y Y coordinate of the center
     * @param width Width of the root
     * @param height Height of the root
     * @todo Size problem
     */
#if __cplusplus >= 201702L
    explicit quadtree(key_t center_x, key_t center_y, key_t width, key_t height) :
        m_width(width), m_height(height), m_center{center_x, center_y}, m_default_value{} {}
#else
    explicit quadtree(key_t center_x, key_t center_y, key_t width, key_t height) :
        m_width(width), m_height(height)
    {
        m_center.x = center_x;
        m_center.y = center_y;
    }
#endif

    /**
     * @brief Construct a quadtree centered on 0,0, with the given width/height
     * @param width Width of the root
     * @param height Height of the root
     */
    explicit quadtree(key_t width, key_t height) :
        quadtree<key_t, item_t>(0, 0, width, height) {}

    /**
     * @brief Construct a quadtree with the given center and width/height and the default value
     * @param center_x X coordinate of the center
     * @param center_y Y coordinate of the center
     * @param width Width of the root
     * @param height Height of the root
     * @param default_value Default value to use
     */
    explicit quadtree(key_t center_x, key_t center_y, key_t width, key_t height,
                      const item_t& default_value) :
        quadtree(center_x, center_y, width, height)
    {
        m_default_value = default_value;
    }

    explicit quadtree(const quadtree& copy);
    explicit quadtree(quadtree&& move);
    ~quadtree() override;

    quadtree& operator=(const quadtree& copy);
    quadtree& operator=(quadtree&& move);

    /**
     * @brief Get the size
     *
     * Override of container::size
     * @return Return the number of points
     */
    size_t size() const noexcept override
    {
        return m_size;
    }

    /**
     * @brief Get the depth of the tree.
     *
     * The depth is the number of subdivision. If there is only the root, the
     * depth is 0. If the quadrant is only divided by 4 once, the depth is 1.
     */
    virtual size_t depth() const noexcept final
    {
        return m_depth;
    }

    /**
     * @brief Get the current default value of the tree
     */
    virtual const item_t& default_value() const noexcept final
    {
        return m_default_value;
    }

    virtual size_t insert(key_t x, key_t y, const item_t& item);

    virtual const item_t& at(key_t x, key_t y) const;
    virtual item_t& at(key_t x, key_t y);

    /**
     * @brief Set the behaviour flags
     *
     * See @ref epstl::behaviour_t enum to create the flag.
     *
     * @code
     * set_behaviour_flag(epstl::quadtree_no_replace | epstl::quadtree_multithread)
     * @endcode
     * @param flag Combinaison of behaviour_t
     */
    virtual void set_behaviour_flag(uint8_t flag) final
    {
        m_behaviour_flag = flag;
    }

    /**
     * @brief Print the quadtree in the given stream
     * @param stream Stream to print inside
     */
    virtual void print(std::ostream& stream) const
    {
        stream << "Root:\n";
        print_quadrant(stream, m_root, 0);
    }

    virtual bool find(const item_t& item, epstl::pair<key_t>& keys,
                      std::function<bool(const item_t&, const item_t&)> criterion
                      = [](const item_t& i1, const item_t& i2)
    {
        return i1 == i2;
    }) const;

    virtual bool find(const item_t& item,
                      std::function<bool(const item_t&, const item_t&)> criterion
                      = [](const item_t& i1, const item_t& i2)
    {
        return i1 == i2;
    }) const;

    virtual void remove(key_t x, key_t y);
    virtual void remove_all(const item_t& item,
                            std::function<bool(const item_t&, const item_t&)> criterion
                            = [](const item_t& i1, const item_t& i2)
    {
        return i1 == i2;
    });

  protected:
    virtual quadrant_t* clone_quadrant(const quadrant_t* quadrant) const;
    virtual void free_quadrant(quadrant_t* quadrant);
    virtual bool insert_quadrant(quadrant_t* quadrant, key_t x, key_t y,
                                 const item_t& item);
    virtual quadrant_t** select_quadrant(quadrant_t* quadrant, key_t x,
                                         key_t y) const;
    virtual void create_quadrants(quadrant_t* parent);
    virtual const item_t& get_value(quadrant_t* quadrant, key_t x, key_t y) const;
    virtual item_t& get_value(quadrant_t* quadrant, key_t x, key_t y);
    virtual void print_quadrant(std::ostream& stream, quadrant_t* quadrant,
                                uint32_t shifts) const;
    virtual void shift_stream(std::ostream& stream, uint32_t shifts,
                              const char* separator) const final;

    virtual bool find_quadrant(quadrant_t* quadrant, const item_t& item,
                               epstl::pair<key_t>& keys,
                               std::function<bool(const item_t&, const item_t&)> criterion) const;

    virtual bool remove_quadrant(quadrant_t* quadrant, key_t x, key_t y);
    virtual bool remove_all_quadrant(quadrant_t* quadrant, const item_t& item,
                                     std::function<bool (const item_t&, const item_t&)> criterion);
    virtual size_t compute_depth(quadrant_t* quadrant) const;



    quadrant_t* m_root = nullptr;   ///< Root quadrant of the quadtree
    size_t m_size = 0;              ///< Number of points in the tree
    size_t m_depth = 0;             ///< Depth of the tree
    item_t m_default_value;         ///< Default value of the items
    item_t m_exposed_default_value; ///< Default value for mutable reference
    key_t m_height;                 ///< Height of the root quadrant
    key_t m_width;                  ///< Width of the root quadrant
    position_t m_center;            ///< Center of the quadrant

    uint8_t m_behaviour_flag = 0;   ///< Behaviour flags
};

/**
 * @brief Copy constructor
 */
template<typename key_t, typename item_t>
quadtree<key_t, item_t>::quadtree(const quadtree<key_t, item_t>& copy) :
    m_width(copy.m_width), m_height(copy.m_height), m_center(copy.m_center),
    m_depth(copy.m_depth), m_size(copy.m_size),
    m_default_value(copy.m_default_value)
{
    m_root = clone_quadrant(copy.m_root);
}

/**
 * @brief Move constructor
 */
template<typename key_t, typename item_t>
quadtree<key_t, item_t>::quadtree(quadtree<key_t, item_t>&& move) :
    m_width(move.m_width), m_height(move.m_height), m_center(move.m_center),
    m_depth(move.m_depth), m_size(move.m_size),
    m_default_value(move.m_default_value)
{
    m_root = move.m_root;
    move.m_root = nullptr;
    move.m_size = 0;
    move.m_depth = 0;
}

/**
 * @brief Destructor
 *
 * Free all dynamically allocated quadrants
 */
template<typename key_t, typename item_t>
quadtree<key_t, item_t>::~quadtree()
{
    free_quadrant(m_root);
}

/**
 * @brief Assignation operator
 */
template<typename key_t, typename item_t>
quadtree<key_t, item_t>&
quadtree<key_t, item_t>::operator=(const quadtree& copy)
{
    m_root = clone_quadrant(copy.m_root);

    m_size = copy.m_size;
    m_depth = copy.m_depth;
    m_width = copy.m_width;
    m_height = copy.m_height;
    m_center = copy.m_center;
    m_default_value = copy.m_default_value;

    return *this;
}

/**
 * @brief Assignation operator with move
 */
template<typename key_t, typename item_t>
quadtree<key_t, item_t>&
quadtree<key_t, item_t>::operator=(quadtree&& move)
{
    m_root = move.m_root;

    m_size = move.m_size;
    move.m_size = 0;

    m_depth = move.m_depth;
    move.m_depth = 0;

    m_width = move.m_width;
    m_height = move.m_height;
    m_center = move.m_center;
    m_default_value = move.m_default_value;

    return *this;
}

/**
 * @brief Insert the item at the given coordinates
 *
 * Update the depth.
 *
 * @param x X coordinate of the item
 * @param y Y coordinate of the item
 * @param item Item to copy in the tree
 * @return Size of the new tree (number of items)
 */
template<typename key_t, typename item_t>
size_t quadtree<key_t, item_t>::insert(key_t x, key_t y, const item_t& item)
{
    if (!m_root)
    {
        m_root = new quadrant_t;
        m_root->bound.left = m_center.x - m_width / 2.;
        m_root->bound.right = m_root->bound.left + m_width;
        m_root->bound.bottom = m_center.y - m_height / 2.;
        m_root->bound.top = m_root->bound.bottom + m_height;
        m_root->bound.center = m_center;
        m_root->data = item;
        m_root->data_position.x = x;
        m_root->data_position.y = y;
        m_size++;
    }
    else
    {
        insert_quadrant(m_root, x, y, item);
        this->m_depth = compute_depth(m_root);
    }

    return m_size;
}

/**
 * @brief Get a constant reference on the item at the given coordinates
 *
 * If there is no point at the given coordinates, the default value is returned.
 *
 * @param x X coordinate to get
 * @param y Y coordinate to get
 * @return Constant reference on the value
 */
template<typename key_t, typename item_t>
const item_t& quadtree<key_t, item_t>::at(key_t x, key_t y) const
{
    return get_value(m_root, x, y);
}

/**
 * @brief Get a mutable reference on the item at the given coordinates
 *
 * If there is no point at the given coordinates, the default value is returned.
 * The mutable reference on the default value is not the real value.
 *
 * @param x X coordinate to get
 * @param y Y coordinate to get
 * @return Mutable reference on the value
 */
template<typename key_t, typename item_t>
item_t& quadtree<key_t, item_t>::at(key_t x, key_t y)
{
    m_exposed_default_value = m_default_value;
    return get_value(m_root, x, y);
}

/**
 * @brief Find the item given and return the coordinates with the keys argument
 *
 * @param item Item to look for
 * @param[out] keys Output containing the coordinates of the item, if it was found.
 * @param criterion Comparaison criterion to apply
 * @return Return true if the item was found
 */
template<typename key_t, typename item_t>
bool quadtree<key_t, item_t>::find(const item_t& item, epstl::pair<key_t>& keys,
                                   std::function<bool (const item_t&, const item_t&)> criterion) const
{
    return find_quadrant(m_root, item, keys, criterion);
}

/**
 * @brief Tells if the item is contained in the tree
 *
 * @param item Item to look for
 * @param criterion Comparaison criterion to apply
 * @return Return true if the item was found
 */
template<typename key_t, typename item_t>
bool quadtree<key_t, item_t>::find(const item_t& item,
                                   std::function<bool (const item_t&, const item_t&)> criterion) const
{
    epstl::pair<key_t> keys;
    return find(item, keys, criterion);
}

/**
 * @brief Remove the item at the given coordinates
 * @param x X coordinate to remove
 * @param y Y coordinate to remove
 */
template<typename key_t, typename item_t>
void quadtree<key_t, item_t>::remove(key_t x, key_t y)
{
    remove_quadrant(m_root, x, y);
    m_depth = compute_depth(m_root);
}

/**
 * @brief Remove all item matching the given one
 *
 * @param item Item to remove
 * @param criterion Comparaison criterion to apply
 */
template<typename key_t, typename item_t>
void quadtree<key_t, item_t>::remove_all(const item_t& item,
        std::function<bool (const item_t&, const item_t&)> criterion)
{
    remove_all_quadrant(m_root, item, criterion);
    m_depth = compute_depth(m_root);
}

/**
 * @brief Clone the quadrant and its children
 * @param quadrant Quadrant to clone
 * @return Return the pointer on the new quadrant
 */
template<typename key_t, typename item_t>
typename quadtree<key_t, item_t>::quadrant_t*
quadtree<key_t, item_t>::clone_quadrant(const quadrant_t* quadrant) const
{
    if (quadrant)
    {
        quadrant_t* clone = new quadrant_t;
        clone->parent = nullptr;
        if (clone->ne = clone_quadrant(quadrant->ne))
            clone->ne->parent = clone;
        if (clone->nw = clone_quadrant(quadrant->nw))
            clone->nw->parent = clone;
        if (clone->sw = clone_quadrant(quadrant->sw))
            clone->sw->parent = clone;
        if (clone->se = clone_quadrant(quadrant->se))
            clone->se->parent = clone;
        clone->bound = quadrant->bound;
        clone->data = quadrant->data;
        return clone;
    }
    else
    {
        return nullptr;
    }
}

/**
 * @brief Free the memory of the given quadrant
 * @param quadrant Quadrant to free
 */
template<typename key_t, typename item_t>
void quadtree<key_t, item_t>::free_quadrant(quadrant_t* quadrant)
{
    if (quadrant)
    {
        free_quadrant(quadrant->ne);
        free_quadrant(quadrant->nw);
        free_quadrant(quadrant->sw);
        free_quadrant(quadrant->se);

        delete quadrant;
    }
}

/**
 * @brief Insert the item on the quadrant at the given coordinates
 *
 * @param quadrant Quadrant to use (for recursive use)
 * @param x X coordinate of the item to insert
 * @param y Y coordinate of the item to insert
 * @param item Item to copy into the tree
 * @return true if the quadrant has been changed
 */
template<typename key_t, typename item_t>
bool quadtree<key_t, item_t>::insert_quadrant(quadrant_t* quadrant, key_t x,
        key_t y, const item_t& item)
{
    if (!quadrant)
        throw epstl::implementation_exception("insertion in a null quadrant");
    if (!quadrant->bound.isInside(x, y))
        return false;
    if (quadrant->ne) // If there is a quadrant division
    {
        return insert_quadrant(quadrant->ne, x, y, item) ||
               insert_quadrant(quadrant->nw, x, y, item) ||
               insert_quadrant(quadrant->sw, x, y, item) ||
               insert_quadrant(quadrant->se, x, y, item);
    }

    if (quadrant->data == m_default_value)
    {
        quadrant->data = item;
        quadrant->data_position.x = x;
        quadrant->data_position.y = y;
        m_size++;
        return true;
    }

    if (quadrant->data_position.x != x || quadrant->data_position.y != y)
    {
        // Division
        create_quadrants(quadrant);

        insert_quadrant(quadrant->ne, quadrant->data_position.x,
                        quadrant->data_position.y, quadrant->data);
        insert_quadrant(quadrant->nw, quadrant->data_position.x,
                        quadrant->data_position.y, quadrant->data);
        insert_quadrant(quadrant->sw, quadrant->data_position.x,
                        quadrant->data_position.y, quadrant->data);
        insert_quadrant(quadrant->se, quadrant->data_position.x,
                        quadrant->data_position.y, quadrant->data);
        m_size--;

        return insert_quadrant(quadrant->ne, x, y, item) ||
               insert_quadrant(quadrant->nw, x, y, item) ||
               insert_quadrant(quadrant->sw, x, y, item) ||
               insert_quadrant(quadrant->se, x, y, item);

    }
    else
    {
        if (!(m_behaviour_flag & quadtree_no_replace))
        {
            quadrant->data = item;
            return true;
        }
    }

    return false;

}

/**
 * @brief Select the quadrant wich contains the given coordinates
 * @param quadrant Quadrant to consider
 * @param x X coordinate to look for
 * @param y Y coordinate to look for
 * @return Pointer on the pointer of the selected quadrant
 */
template<typename key_t, typename item_t>
typename quadtree<key_t, item_t>::quadrant_t**
quadtree<key_t, item_t>::select_quadrant(quadrant_t* quadrant, key_t x,
        key_t y) const
{
    if (!quadrant->bound.isInside(x, y))
        return nullptr;
    if (x >= quadrant->bound.center.x && y >= quadrant->bound.center.y)
        return &quadrant->ne;
    if (x < quadrant->bound.center.x && y >= quadrant->bound.center.y)
        return &quadrant->nw;
    if (x >= quadrant->bound.center.x && y < quadrant->bound.center.y)
        return &quadrant->se;
    if (x < quadrant->bound.center.x && y < quadrant->bound.center.y)
        return &quadrant->sw;
    return nullptr;

}

/**
 * @brief Create the quadrant children of the parent
 *
 * @param parent Parent where to create children
 * @todo What happens if we can't devide by 2 ? Ex: parent quadrant is already 1x1 and key_t is int.
 * @warning Dynamic allocation with new operator
 */
template<typename key_t, typename item_t>
void quadtree<key_t, item_t>::create_quadrants(quadrant_t* parent)
{
    quadrant_t* ne = new quadrant_t;
    ne->parent = parent;
    ne->data = m_default_value;
    ne->bound.left = parent->bound.center.x;
    ne->bound.right = parent->bound.right;
    ne->bound.top = parent->bound.top;
    ne->bound.bottom = parent->bound.center.y;
    ne->bound.center.x = (ne->bound.left + ne->bound.right) / 2.;
    ne->bound.center.y = (ne->bound.top + ne->bound.bottom) / 2.;
    parent->ne = ne;

    quadrant_t* nw = new quadrant_t;
    nw->parent = parent;
    nw->data = m_default_value;
    nw->bound.left = parent->bound.left;
    nw->bound.right = parent->bound.center.x;
    nw->bound.top = parent->bound.top;
    nw->bound.bottom = parent->bound.center.y;
    nw->bound.center.x = (nw->bound.left + nw->bound.right) / 2.;
    nw->bound.center.y = (nw->bound.top + nw->bound.bottom) / 2.;
    parent->nw = nw;

    quadrant_t* sw = new quadrant_t;
    sw->parent = parent;
    sw->data = m_default_value;
    sw->bound.left = parent->bound.left;
    sw->bound.right = parent->bound.center.x;
    sw->bound.top = parent->bound.center.y;
    sw->bound.bottom = parent->bound.bottom;
    sw->bound.center.x = (sw->bound.left + sw->bound.right) / 2.;
    sw->bound.center.y = (sw->bound.top + sw->bound.bottom) / 2.;
    parent->sw = sw;

    quadrant_t* se = new quadrant_t;
    se->parent = parent;
    se->data = m_default_value;
    se->bound.left = parent->bound.center.x;
    se->bound.right = parent->bound.right;
    se->bound.top = parent->bound.center.y;
    se->bound.bottom = parent->bound.bottom;
    se->bound.center.x = (se->bound.left + se->bound.right) / 2.;
    se->bound.center.y = (se->bound.top + se->bound.bottom) / 2.;
    parent->se = se;

}

/**
 * @brief Get the value at the given position
 *
 * Return the default value if there is nothing at the given coordinates
 * @param quadrant Quadrant to look into
 * @param x X coordinate to look for
 * @param y Y coordinate to look for
 * @return Constant reference on the value
 */
template<typename key_t, typename item_t>
const item_t& quadtree<key_t, item_t>::get_value(quadrant_t* quadrant, key_t x,
        key_t y) const
{
    if (!quadrant->bound.isInside(x, y))
        return m_default_value;
    if (quadrant->ne)
    {
        quadrant_t** selected_quadrant = select_quadrant(quadrant, x, y);
        if (!selected_quadrant)
            return m_default_value;
        return get_value(*selected_quadrant, x, y);
    }
    else
    {
        if (quadrant->data_position.x == x && quadrant->data_position.y == y)
            return quadrant->data;
        else
            return m_default_value;
    }
}

/**
 * @brief Get the value at the given position
 *
 * Return the default value if there is nothing at the given coordinates
 *
 * @warning Need to set m_exposed_default_value before calling the method
 *
 * @param quadrant Quadrant to look into
 * @param x X coordinate to look for
 * @param y Y coordinate to look for
 * @return Mutable reference on the value
 */
template<typename key_t, typename item_t>
item_t&
quadtree<key_t, item_t>::get_value(quadrant_t* quadrant, key_t x, key_t y)
{
    if (!quadrant->bound.isInside(x, y))
        return m_exposed_default_value;
    if (quadrant->ne)
    {
        quadrant_t** selected_quadrant = select_quadrant(quadrant, x, y);
        if (!selected_quadrant)
            return m_exposed_default_value;
        return get_value(*selected_quadrant, x, y);
    }
    else
    {
        if (quadrant->data_position.x == x && quadrant->data_position.y == y)
            return quadrant->data;
        else
            return m_exposed_default_value;
    }
}

/**
 * @brief Recursive method to print the given quadran in the stream
 *
 * @param stream Stream to print into
 * @param quadrant Quadrant to print
 * @param shifts Shifts to apply
 */
template<typename key_t, typename item_t>
void quadtree<key_t, item_t>::print_quadrant(std::ostream& stream,
        quadrant_t* quadrant, uint32_t shifts) const
{
    if (quadrant)
    {
        shift_stream(stream, shifts, "| ");
        stream << "[ " << quadrant->bound.left << ", " << quadrant->bound.right <<
               " ], [ " << quadrant->bound.bottom << ", " << quadrant->bound.top << " ]\n";
        if (quadrant->ne)
        {
            shift_stream(stream, shifts, "| ");
            stream << "NE : \n";
            print_quadrant(stream, quadrant->ne, shifts + 1);
            shift_stream(stream, shifts, "| ");
            stream << "NW : \n";
            print_quadrant(stream, quadrant->nw, shifts + 1);
            shift_stream(stream, shifts, "| ");
            stream << "SW : \n";
            print_quadrant(stream, quadrant->sw, shifts + 1);
            shift_stream(stream, shifts, "| ");
            stream << "SE : \n";
            print_quadrant(stream, quadrant->se, shifts + 1);
            shift_stream(stream, shifts, "| ");
            stream << "-\n";
        }
        else
        {
            shift_stream(stream, shifts, "| ");
            stream << "Data : " << quadrant->data << "\n";
            shift_stream(stream, shifts, "| ");
            stream << "Data position : " << quadrant->data_position.x << ", " <<
                   quadrant->data_position.y << "\n";
        }

    }
}

/**
 * @brief Print into the stream the separator as many as there are shifts
 *
 * @param stream Stream to print into
 * @param shifts Number of separator to print
 * @param separator Separator to print. Use '\t' for tabulation for example
 */
template<typename key_t, typename item_t>
void quadtree<key_t, item_t>::shift_stream(std::ostream& stream,
        uint32_t shifts, const char* separator) const
{
    for (uint32_t i = 0; i < shifts; i++)
    {
        stream << separator;
    }
}

/**
 * @brief Recursive method for find
 * @param quadrant Quadrant to look into
 * @param item Item to look for
 * @param[out] keys Output for the coordinates of the item if it was found
 * @param criterion Comparaison criterion to apply
 */
template<typename key_t, typename item_t>
bool quadtree<key_t, item_t>::find_quadrant(quadrant_t* quadrant,
        const item_t& item, epstl::pair<key_t>& keys,
        std::function<bool (const item_t&, const item_t&)> criterion) const
{
    if (!quadrant)
        return false;
    if (quadrant->ne)
    {
        if (m_behaviour_flag & epstl::quadtree_multithread)
        {
            // TODO
            return find_quadrant(quadrant->ne, item, keys, criterion) ||
                   find_quadrant(quadrant->nw, item, keys, criterion) ||
                   find_quadrant(quadrant->sw, item, keys, criterion) ||
                   find_quadrant(quadrant->se, item, keys, criterion);
        }
        else
        {
            return find_quadrant(quadrant->ne, item, keys, criterion) ||
                   find_quadrant(quadrant->nw, item, keys, criterion) ||
                   find_quadrant(quadrant->sw, item, keys, criterion) ||
                   find_quadrant(quadrant->se, item, keys, criterion);
        }
    }
    else if (criterion(quadrant->data, item))
    {
        keys.first = quadrant->data_position.x;
        keys.second = quadrant->data_position.y;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Recursive method for remove method
 *
 * @param quadrant Quadrant to look into
 * @param x X coordinate to look for
 * @param y Y coordinate to look for
 * @return Return true if the qudrant is left empty
 */
template<typename key_t, typename item_t>
bool quadtree<key_t, item_t>::remove_quadrant(quadrant_t* quadrant, key_t x,
        key_t y)
{
    if (!quadrant)
        return true;
    if (quadrant->ne)
    {
        bool ne_empty = remove_quadrant(quadrant->ne, x, y);
        bool nw_empty = remove_quadrant(quadrant->nw, x, y);
        bool sw_empty = remove_quadrant(quadrant->sw, x, y);
        bool se_empty = remove_quadrant(quadrant->se, x, y);

        if (ne_empty || nw_empty || sw_empty || se_empty)
        {
            // If all quadrants are empty, the parent is empty too
            if (ne_empty && nw_empty && sw_empty && se_empty)
            {
                quadrant->ne = nullptr;
                quadrant->nw = nullptr;
                quadrant->sw = nullptr;
                quadrant->se = nullptr;
                quadrant->data = m_default_value;
                quadrant->data_position = {};
                return true;
            }
            // Else, if only 1 quadrant is not empty, we bring up the data and
            // delete the 4 quadrants
            else if (ne_empty + nw_empty + sw_empty + se_empty == 3)
            {
                quadrant_t* not_empty_one = nullptr;
                if (!ne_empty)
                    not_empty_one = quadrant->ne;
                else if (!nw_empty)
                    not_empty_one = quadrant->nw;
                else if (!sw_empty)
                    not_empty_one = quadrant->sw;
                else
                    not_empty_one = quadrant->se;

                quadrant->data = not_empty_one->data;
                quadrant->data_position = not_empty_one->data_position;

                quadrant->ne = nullptr;
                quadrant->nw = nullptr;
                quadrant->sw = nullptr;
                quadrant->se = nullptr;
                return false;
            }
        }
        return false;
    }
    else if (quadrant->data_position.x == x && quadrant->data_position.y == y)
    {
        quadrant->data = m_default_value;
        quadrant->data_position = {};
        m_size--;
        return true;
    }
    else if (quadrant->data == m_default_value)
    {
        return true;
    }
    return false;
}

/**
 * @brief Recursive method for remove_all method
 * @param quadrant Quadrant to look into
 * @param item Item to look for
 * @param criterion Comparaison criterion to apply
 * @return Return true if the quadrant was left empty
 */
template<typename key_t, typename item_t>
bool quadtree<key_t, item_t>::remove_all_quadrant(quadrant_t* quadrant,
        const item_t& item, std::function<bool (const item_t&, const item_t&)>
        criterion)
{
    if (!quadrant)
        return true;
    if (quadrant->ne)
    {
        bool ne_empty = remove_all_quadrant(quadrant->ne, item, criterion);
        bool nw_empty = remove_all_quadrant(quadrant->nw, item, criterion);
        bool sw_empty = remove_all_quadrant(quadrant->sw, item, criterion);
        bool se_empty = remove_all_quadrant(quadrant->se, item, criterion);

        if (ne_empty || nw_empty || sw_empty || se_empty)
        {
            // If all quadrants are empty, the parent is empty too
            if (ne_empty && nw_empty && sw_empty && se_empty)
            {
                quadrant->ne = nullptr;
                quadrant->nw = nullptr;
                quadrant->sw = nullptr;
                quadrant->se = nullptr;
                quadrant->data = m_default_value;
                quadrant->data_position = {};
                return true;
            }
            // Else, if only 1 quadrant is not empty, we bring up the data and
            // delete the 4 quadrants
            else if (ne_empty + nw_empty + sw_empty + se_empty == 3)
            {
                quadrant_t* not_empty_one = nullptr;
                if (!ne_empty)
                    not_empty_one = quadrant->ne;
                else if (!nw_empty)
                    not_empty_one = quadrant->nw;
                else if (!sw_empty)
                    not_empty_one = quadrant->sw;
                else
                    not_empty_one = quadrant->se;

                quadrant->data = not_empty_one->data;
                quadrant->data_position = not_empty_one->data_position;

                quadrant->ne = nullptr;
                quadrant->nw = nullptr;
                quadrant->sw = nullptr;
                quadrant->se = nullptr;
                return false;
            }
        }
        return false;
    }
    else if (criterion(quadrant->data, item))
    {
        quadrant->data = m_default_value;
        quadrant->data_position = {};
        m_size--;
        return true;
    }
    else if (quadrant->data == m_default_value)
    {
        return true;
    }
    return false;
}

/**
 * @brief Recursive method to compute the depth of the quadrant
 * @param quadrant Quadrant where to compute the depth
 * @return Return the depth of the given quarant
 */
template<typename key_t, typename item_t>
size_t quadtree<key_t, item_t>::compute_depth(quadrant_t* quadrant) const
{
    if (!quadrant)
        return 0;
    if (quadrant->ne)
        return epstl::max(compute_depth(quadrant->ne),
                          compute_depth(quadrant->nw),
                          compute_depth(quadrant->sw),
                          compute_depth(quadrant->se)) + 1;

    return 0;
}


} // namespace epstl
