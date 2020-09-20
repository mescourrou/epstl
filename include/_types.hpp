#pragma once

#include <cstdint>

namespace epstl
{

#ifdef USE_CUSTOM_STL
/// Standard size
typedef uint32_t size_t;
#else
/// Standard size
typedef std::size_t size_t;
#endif
/// Extended size
typedef uint64_t ssize_t;

} // namespace epstl
