#include "vectorTest.hpp"
#include <vector.hpp>

namespace epstl
{

struct testStruct
{
    double x, y;
};
#ifdef USE_CUSTOM_STL
TEST_F(vectorTest, Creation)
{
    vector<int> v{1, 2, 3};

    EXPECT_EQ(*v.at(0), 1);
    EXPECT_EQ(*v.at(1), 2);
    EXPECT_EQ(*v.at(2), 3);
}

TEST_F(vectorTest, pushBack)
{
    vector<testStruct, 5> v{{0, 0}, {1, 1}, {2, 2}, {3, 3}};

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v.allocated(), 5);

    EXPECT_EQ(v.push_back({4, 4}), 5);
    EXPECT_EQ(v.allocated(), 5);

    v.push_back({5, 5});

    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v.allocated(), 10);

}

TEST_F(vectorTest, popBack)
{
    vector<testStruct, 5> v{{0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}};

    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v.allocated(), 10);

    EXPECT_EQ(v.pop_back(), 5);
    EXPECT_EQ(v.allocated(), 5);

    EXPECT_EQ(v.pop_back(), 4);
    EXPECT_EQ(v.allocated(), 5);
}

TEST_F(vectorTest, iterator)
{
    vector<testStruct, 5> v{{0, 0}, {1, 1}, {2, 2}};

    double count = 0;
    for (auto i : v)
    {
        EXPECT_EQ(i->x, count);
        EXPECT_EQ(i->y, count);
        EXPECT_LE(count, 2);
        count++;
    }
    EXPECT_EQ(count, 3.0);
}

TEST_F(vectorTest, constIterator)
{
    vector<testStruct, 5> v{{0, 0}, {1, 1}, {2, 2}};

    double count = 0;
    for (auto itr = v.cbegin(); itr != v.cend(); ++itr)
    {
        EXPECT_EQ(itr->x, count);
        EXPECT_EQ(itr->y, count);
        EXPECT_LE(count, 2);
        count++;
    }
    EXPECT_EQ(count, 3.0);
}

TEST_F(vectorTest, reverseIterator)
{
    vector<testStruct, 5> v{{0, 0}, {1, 1}, {2, 2}};

    double count = 2;
    for (auto itr = v.rbegin(); itr != v.rend(); ++itr)
    {
        EXPECT_EQ(itr->x, count);
        EXPECT_EQ(itr->y, count);
        EXPECT_GE(count, 0);
        count--;
    }
    EXPECT_EQ(count, -1.0);
}

TEST_F(vectorTest, sort)
{
    vector<int> v{30, 40, 12, 50, 29, 59, 40, 20};
    vector<int> expected{12, 20, 29, 30, 40, 40, 50, 59};

    v.sort();
    for (unsigned int i = 0; i < v.size(); i++)
    {
        EXPECT_EQ(*v.at(i), *expected.at(i));
    }
}

TEST_F(vectorTest, sortDescending)
{
    vector<int> v{30, 40, 12, 50, 29, 59, 40, 20};
    vector<int> expected{59, 50, 40, 40, 30, 29, 20, 12};

    v.sort(false);
    for (unsigned int i = 0; i < v.size(); i++)
    {
        EXPECT_EQ(*v.at(i), *expected.at(i));
    }
}
#endif

} // namespace epstl
