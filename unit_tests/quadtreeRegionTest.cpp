#include "quadtreeRegionTest.hpp"
#include <quadtree_region.hpp>

namespace epstl
{


using ::testing::Values;

TEST_P(quadtreeRegionTest, SetAPixel)
{
    EXPECT_EQ(tree->size(), points.size());
    for (int row = y_min ; row < y_max; row++)
    {
        SCOPED_TRACE(std::string("row : ") + std::to_string(row));
        for (int col = x_min ; col < x_max; col++)
        {
            SCOPED_TRACE(std::string("col : ") + std::to_string(col));
            if (std::find(points.begin(), points.end(), std::pair<int, int> {col, row}) !=
                    points.end())
            {
                EXPECT_TRUE(tree->at(col, row));
            }
            else
                EXPECT_FALSE(tree->at(col, row));
        }
    }

}

TEST_P(quadtreeRegionTest, UnsetPixel)
{
    size_t expected_size = points.size();
    for (auto& pt : points)
    {
        EXPECT_EQ(tree->size(), expected_size);
        tree->unset(pt.first, pt.second);
        expected_size--;

    }
    EXPECT_EQ(tree->depth(), 0);
    EXPECT_EQ(tree->size(), expected_size);
}

#if __cplusplus >= 201702L
static const std::vector<std::pair<int, int>> dimensionValues {{7, 7}, {8, 8}, {10, 4}, {12, 5}, {7, 11}};

INSTANTIATE_TEST_SUITE_P(QuadtreeRegionDimensions,
                         quadtreeRegionTest,
                         ::testing::ValuesIn(dimensionValues),
                         [](const testing::TestParamInfo<std::pair<int, int>>& info)
{
    std::stringstream ss;
    ss << info.param.first << "x" << info.param.second;
    return ss.str();
});
#endif

} // namespace epstl
