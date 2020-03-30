#include "mathTest.hpp"
#include <math.hpp>

namespace epstl
{

TEST_F(mathTest, Max)
{
    EXPECT_EQ(max(1, 2, 3, 4, 5), 5);
}

TEST_F(mathTest, Modulo)
{
    EXPECT_TRUE(abs(modulo(1.2, 0.5) - 0.2) < 0.0001);
    EXPECT_TRUE(abs(modulo(-1.2f, 1.f) - (-0.2)) < 0.0001);

    EXPECT_TRUE(abs(modulo2(1.2, -1., 2.) - 1.2) < 0.0001);
    EXPECT_TRUE(abs(modulo2(2.2, -1., 2.) + 0.8) < 0.0001);
    EXPECT_TRUE(abs(modulo2(0.2, 1., 3.) - 2.2) < 0.0001);
}

} // namespace epstl
