//
// Copyright (C) 2011 Mateusz Loskot
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_BOOST_GEOMETRY_H_INCLUDED
#define SOCI_BOOST_GEOMETRY_H_INCLUDED

#include "binary.h"
#include "type-conversion-traits.h"
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>

namespace soci
{

template<>
struct type_conversion<boost::geometry::model::d2::point_xy<double> >
{

    typedef binary_string base_type;

    static void from_base(base_type const& in, indicator ind, boost::geometry::model::d2::point_xy<double>& out)
    {
        if (ind == i_null)
        {
            throw soci_error("Null value not allowed for this type");
        }

        if (!boost::geometry::read_wkb(in.data_.begin(), in.data_.end(), out))
            throw std::runtime_error("Failed to read geometry from binary stream");
    }

    static void to_base(boost::geometry::model::d2::point_xy<double> const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_ok;
    }
};

} // namespace soci

#endif // SOCI_BOOST_GEOMETRY_H_INCLUDED
