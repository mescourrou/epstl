#pragma once

#include "quadtree.hpp"
#include "vector.hpp"

namespace epstl
{

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
template<typename key_t = int>
class quadtree_region : public quadtree<key_t, bool>
{
    bool isIndide(const vector<key_t>& polygon, key_t x, key_t y) const;
  public:
    /**
     * @brief Construct a quadtree with the given center and width/height
     * @param center_x X coordinate of the center
     * @param center_y Y coordinate of the center
     * @param width Width of the root
     * @param height Height of the root
     */
    explicit quadtree_region(key_t center_x, key_t center_y, key_t width,
                             key_t height) :
        quadtree<key_t, bool>(center_x, center_y, width, height, false) {}

    /**
     * @brief Construct a quadtree centered on 0,0, with the given width/height
     * @param width Width of the root
     * @param height Height of the root
     */
    explicit quadtree_region(key_t width, key_t height) :
        quadtree<key_t, bool>(0, 0, width, height) {}

    /**
     * @brief Construct a quadtree with the given center and width/height and the default value
     * @param center_x X coordinate of the center
     * @param center_y Y coordinate of the center
     * @param width Width of the root
     * @param height Height of the root
     * @param default_value Default value to use
     */
    explicit quadtree_region(key_t center_x, key_t center_y, key_t width,
                             key_t height,
                             bool default_value) :
        quadtree<key_t, bool>(center_x, center_y, width, height, default_value) {}

    explicit quadtree_region(const quadtree_region& copy)  :
        quadtree<key_t, bool> (copy) {}

    explicit quadtree_region(quadtree_region&& move) :
        quadtree<key_t, bool> (move) {}
    ~quadtree_region() override = default;

    quadtree_region& operator=(const quadtree_region& copy)
    {
        return quadtree<key_t, bool>::operator=(copy);
    };
    quadtree_region& operator=(quadtree_region&& move)
    {
        return quadtree<key_t, bool>::operator=(move);
    }

    size_t insert(key_t x, key_t y, const bool& item) override;
    size_t insert_region(const vector<key_t>& polygon_points, const bool& item);

    void set(key_t x, key_t y)
    {
        insert(x, y, true);
    }
    void unset(key_t x, key_t y)
    {
        insert(x, y, false);
    }

    void set_region(const vector<key_t>& polygon_points)
    {
        insert_region(polygon_points, true);
    }
    void unset_region(const vector<key_t>& polygon_points)
    {
        insert_region(polygon_points, false);
    }

    void print(std::ostream& stream) const override;

  protected:
    bool insert_quadrant(typename quadtree<key_t, bool>::quadrant_t* quadrant,
                         key_t x, key_t y,
                         const bool& item) override;
    const bool& get_value(typename quadtree<key_t, bool>::quadrant_t* quadrant,
                          key_t x, key_t y) const override;
    bool& get_value(typename quadtree<key_t, bool>::quadrant_t* quadrant, key_t x,
                    key_t y) override;
};

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
template<typename key_t>
size_t quadtree_region<key_t>::insert(key_t x, key_t y, const bool& item)
{
    if (!this->m_root)
    {
        this->m_root = new typename quadtree<key_t, bool>::quadrant_t;
        this->m_root->bound.left = this->m_center.x - this->m_width / 2.;
        this->m_root->bound.right = this->m_root->bound.left + this->m_width;
        this->m_root->bound.bottom = this->m_center.y - this->m_height / 2.;
        this->m_root->bound.top = this->m_root->bound.bottom + this->m_height;
        this->m_root->bound.center = this->m_center;
        this->m_root->data = this->m_default_value;
    }
    insert_quadrant(this->m_root, x, y, item);
    this->m_depth = this->compute_depth(this->m_root);

    return this->m_size;
}

template<typename key_t>
size_t quadtree_region<key_t>::insert_region(const vector<key_t>&
        polygon_points, const bool& item)
{
    if (!this->m_root)
    {
        this->m_root = new typename quadtree<key_t, bool>::quadrant_t;
        this->m_root->bound.left = this->m_center.x - this->m_width / 2.;
        this->m_root->bound.right = this->m_root->bound.left + this->m_width;
        this->m_root->bound.bottom = this->m_center.y - this->m_height / 2.;
        this->m_root->bound.top = this->m_root->bound.bottom + this->m_height;
        this->m_root->bound.center = this->m_center;
        this->m_root->data = this->m_default_value;
    }
    return this->size();

}

template<typename key_t>
void quadtree_region<key_t>::print(std::ostream& stream) const
{
    if (!this->m_root)
    {
        stream << "Empty quadtree\n";
        return;
    }
    stream << "Tree:\n";
    for (key_t row = this->m_root->bound.top; row > this->m_root->bound.bottom;
            row--)
    {
        for (key_t col = this->m_root->bound.left; col < this->m_root->bound.right;
                col++)
        {
            bool state = this->at(col, row);
            stream << (state ? '1' : '0') << " ";
        }
        stream << "\n";
    }
}

/**
 * @brief Insert the item on the quadrant at the given coordinates
 *
 * @param quadrant Quadrant to use (for recursive use)
 * @param x X coordinate of the item to insert
 * @param y Y coordinate of the item to insert
 * @param item Item to copy into the tree
 */
template<typename key_t>
bool quadtree_region<key_t>::insert_quadrant(typename
        quadtree<key_t, bool>::quadrant_t* quadrant, key_t x,
        key_t y, const bool& item)
{
    if (!quadrant)
        throw epstl::implementation_exception("insertion in a null quadrant");
    if (!quadrant->bound.isInside(x, y))
        return false;
    if (quadrant->ne) // If there is a quadrant division
    {
        uint8_t modified_quadrants =
            insert_quadrant(quadrant->ne, x, y, item)
            + insert_quadrant(quadrant->nw, x, y, item)
            + insert_quadrant(quadrant->sw, x, y, item)
            + insert_quadrant(quadrant->se, x, y, item);
        if (modified_quadrants > 0)
        {
            // If all quadrants are the same
            if (this->compute_depth(quadrant) == 1 &&
                    quadrant->ne->data == quadrant->nw->data &&
                    quadrant->ne->data == quadrant->sw->data &&
                    quadrant->ne->data == quadrant->se->data)
            {
                quadrant->data = quadrant->ne->data;
                this->free_quadrant(quadrant->ne);
                this->free_quadrant(quadrant->nw);
                this->free_quadrant(quadrant->sw);
                this->free_quadrant(quadrant->se);
                quadrant->ne = nullptr;
                quadrant->nw = nullptr;
                quadrant->sw = nullptr;
                quadrant->se = nullptr;
                return  true;
            }
        }
        return false;
    }

    if (quadrant->data != item)
    {
        if ((quadrant->bound.left != quadrant->bound.center.x &&
                quadrant->bound.right != quadrant->bound.center.x) ||
                (quadrant->bound.bottom != quadrant->bound.center.y &&
                 quadrant->bound.top != quadrant->bound.center.y))
        {
            // Division
            this->create_quadrants(quadrant);

            insert_quadrant(quadrant->ne, quadrant->ne->bound.center.x,
                            quadrant->ne->bound.center.y, quadrant->data);
            insert_quadrant(quadrant->nw, quadrant->nw->bound.center.x,
                            quadrant->nw->bound.center.y, quadrant->data);
            insert_quadrant(quadrant->sw, quadrant->sw->bound.center.x,
                            quadrant->sw->bound.center.y, quadrant->data);
            insert_quadrant(quadrant->se, quadrant->se->bound.center.x,
                            quadrant->se->bound.center.y, quadrant->data);

            insert_quadrant(quadrant->ne, x, y, item);
            insert_quadrant(quadrant->nw, x, y, item);
            insert_quadrant(quadrant->sw, x, y, item);
            insert_quadrant(quadrant->se, x, y, item);
            return false;
        }
        else
        {
            if (item)
                this->m_size++;
            else
                this->m_size--;
            quadrant->data = item;
            return true;
        }
    }

    return false;

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
template<typename key_t>
const bool& quadtree_region<key_t>::get_value(typename
        quadtree<key_t, bool>::quadrant_t* quadrant, key_t x,
        key_t y) const
{
    if (!quadrant->bound.isInside(x, y))
        return this->m_default_value;
    if (quadrant->ne)
    {
        auto** selected_quadrant = this->select_quadrant(quadrant, x, y);
        if (!selected_quadrant)
            return this->m_default_value;
        return get_value(*selected_quadrant, x, y);
    }
    else
    {
        return quadrant->data;
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
template<typename key_t>
bool&
quadtree_region<key_t>::get_value(typename quadtree<key_t, bool>::quadrant_t*
                                  quadrant, key_t x, key_t y)
{
    if (!quadrant->bound.isInside(x, y))
        return this->m_exposed_default_value;
    if (quadrant->ne)
    {
        auto** selected_quadrant = this->select_quadrant(quadrant, x, y);
        if (!selected_quadrant)
            return this->m_exposed_default_value;
        return get_value(*selected_quadrant, x, y);
    }
    else
    {
        return quadrant->data;
    }
}

} // namespace epstl
