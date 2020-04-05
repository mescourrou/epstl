#include <gtest/gtest.h>
#include "vectorTest.hpp"
#include "mapTest.hpp"
#include "workTest.hpp"
#include "workerThreadTest.hpp"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
