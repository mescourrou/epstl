#include "mathTest.hpp"
#include <math.hpp>

namespace epstl
{

TEST_F(mathTest, Max)
{
    EXPECT_EQ(max(1, 2, 3, 4, 5), 5);
}

} // namespace epstl
