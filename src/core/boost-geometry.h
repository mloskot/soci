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

namespace detail 
{

template <typename Iterator, typename Geometry>
void make_geometry(Iterator begin, Iterator end, Geometry& out)
{
    if (!boost::geometry::read_wkb(begin, end, out))
        throw soci_error("Failed to make geometry from well-known-binary stream");
}

} // namespace detail 

template<typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem>
struct type_conversion<boost::geometry::model::point<CoordinateType, DimensionCount, CoordinateSystem>>
{

    typedef boost::geometry::model::point<CoordinateType, DimensionCount, CoordinateSystem> target_type;
    typedef binary_string base_type;

    static void from_base(base_type const& in, indicator ind, target_type& out)
    {
        if (ind == i_null)
            throw soci_error("Null value not allowed for this type");

        detail::make_geometry(in.data_.begin(), in.data_.end(), out);
    }

    static void to_base(target_type const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_null; // i_ok
    }
};

template<typename CoordinateType, typename CoordinateSystem>
struct type_conversion<boost::geometry::model::d2::point_xy<CoordinateType, CoordinateSystem>>
{
    typedef boost::geometry::model::d2::point_xy<CoordinateType, CoordinateSystem> target_type;
    typedef binary_string base_type;

    static void from_base(base_type const& in, indicator ind, target_type& out)
    {
        if (ind == i_null)
            throw soci_error("Null value not allowed for this type");

        detail::make_geometry(in.data_.begin(), in.data_.end(), out);
    }

    static void to_base(target_type const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_null; // i_ok
    }
};

} // namespace soci

#endif // SOCI_BOOST_GEOMETRY_H_INCLUDED
