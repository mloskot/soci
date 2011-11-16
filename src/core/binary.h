//
// Copyright (C) 2011 Mateusz Loskot <mateusz@loskot.net>
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_BINARY_H_INCLUDED
#define SOCI_BINARY_H_INCLUDED
#include "soci-config.h"
#include <cstdint>
#include <vector>

namespace soci
{

struct SOCI_DECL binary_string
{
    std::vector<std::uint8_t> data_;
};

} // namespace soci

#endif // SOCI_BINARY_H_INCLUDED
