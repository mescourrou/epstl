#include <gtest/gtest.h>

#include <geometry_tools.hpp>

namespace epstl
{

TEST(geometryToolsTest, PointInitializerList)
{
    Point<3> pt{1, 2, 3};
    EXPECT_EQ(pt[0], 1);
    EXPECT_EQ(pt[1], 2);
    EXPECT_EQ(pt[2], 3);
    EXPECT_EQ(pt.size(), 3);
}

}
