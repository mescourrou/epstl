#pragma once

#include <cstdint>

namespace epstl
{

#ifdef USE_CUSTOM_STL
typedef uint32_t size_t; ///< Standard size
#else
typedef std::size_t size_t; ///< Standard size
#endif
typedef uint64_t ssize_t; ///< Extended size

} // namespace epstl
