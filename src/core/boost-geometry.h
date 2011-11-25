//
// Copyright (C) 2011 Mateusz Loskot <mateusz@loskot.net>
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_BOOST_GEOMETRY_H_INCLUDED
#define SOCI_BOOST_GEOMETRY_H_INCLUDED

#include "binary.h"
#include "type-conversion-traits.h"
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>

namespace soci
{

namespace detail 
{

template <typename Iterator, typename Geometry>
void make_geometry(Geometry& out, Iterator begin, Iterator end, indicator ind)
{
    if (ind == i_null)
        throw soci_error("Null value not allowed for geometry type");

    if (!boost::geometry::read_wkb(begin, end, out))
        throw soci_error("Failed to make geometry from well-known-binary stream");
}

} // namespace detail 

template<typename CoordinateType, std::size_t DimensionCount, typename CoordinateSystem>
struct type_conversion
    <
        boost::geometry::model::point
        <
            CoordinateType, DimensionCount, CoordinateSystem
        >
    >
{

    typedef binary_string base_type;
    typedef boost::geometry::model::point
        <
            CoordinateType, DimensionCount, CoordinateSystem
        > target_type;
    
    static void from_base(base_type const& in, indicator ind, target_type& out)
    {
        detail::make_geometry(out, in.data_.begin(), in.data_.end(), ind);
    }

    static void to_base(target_type const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_null; // i_ok
    }
};

template<typename CoordinateType, typename CoordinateSystem>
struct type_conversion
    <
        boost::geometry::model::d2::point_xy<CoordinateType, CoordinateSystem>
    >
{
    typedef binary_string base_type;
    typedef boost::geometry::model::d2::point_xy<CoordinateType, CoordinateSystem> target_type;
    
    static void from_base(base_type const& in, indicator ind, target_type& out)
    {
        detail::make_geometry(out, in.data_.begin(), in.data_.end(), ind);
    }

    static void to_base(target_type const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_null; // i_ok
    }
};

template
<
    typename Point,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct type_conversion<boost::geometry::model::linestring<Point, Container, Allocator>>
{
    typedef binary_string base_type;
    typedef boost::geometry::model::linestring<Point, Container, Allocator> target_type;

    static void from_base(base_type const& in, indicator ind, target_type& out)
    {
        detail::make_geometry(out, in.data_.begin(), in.data_.end(), ind);
    }

    static void to_base(target_type const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_null; // i_ok
    }
};

template
<
    typename Point,
    bool ClockWise,
    bool Closed,
    template<typename, typename> class PointList,
    template<typename, typename> class RingList,
    template<typename> class PointAlloc,
    template<typename> class RingAlloc
>
struct type_conversion
    <
        boost::geometry::model::polygon
        <
            Point, ClockWise, Closed, PointList, RingList, PointAlloc, RingAlloc
        >
    >
{
    typedef binary_string base_type;
    typedef boost::geometry::model::polygon
        <
            Point, ClockWise, Closed, PointList, RingList, PointAlloc, RingAlloc
        > target_type;
    

    static void from_base(base_type const& in, indicator ind, target_type& out)
    {
        detail::make_geometry(out, in.data_.begin(), in.data_.end(), ind);
    }

    static void to_base(target_type const& in, base_type& out, indicator& ind)
    {
        out.data_.clear(); //out = ... //write wkb
        ind = i_null; // i_ok
    }
};

} // namespace soci

#endif // SOCI_BOOST_GEOMETRY_H_INCLUDED
