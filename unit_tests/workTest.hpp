#pragma once

#include <gtest/gtest.h>
#include <work.hpp>

namespace epstl
{

class WorkTest : public testing::Test
{
  public:

    static inline bool callEffectuated1 = false;
    static inline bool callEffectuated2 = false;
    static inline bool callEffectuated3 = false;
};

}

