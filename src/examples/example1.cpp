// Example working with OGC geometries from PostgreSQL/PostGIS database
// into Boost.Geometry models fetched directly in binary format.
// For sample output, see at the bottom.
#include <cstdint>
#include <deque>
#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <soci.h>
#include <soci-postgresql.h>
#include <boost-geometry.h>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/utility.hpp>
using namespace soci;
using namespace std;
namespace bg = boost::geometry;

void create_table_data_types(soci::session& sql)
{
    sql << "DROP TABLE IF EXISTS soci_data_types";
    sql << "CREATE TABLE soci_data_types (id serial, s8 bigserial, bool boolean, i2 smallint, i4 int, i8 bigint, "
        "f4 real, f8 double precision, dec decimal, num numeric, "
        "c1 \"char\", c32 character(32), cv32 varchar(32), txt text, "
        "varlenbin21 bytea, ts timestamp, dt date, tm time,"
        "CONSTRAINT soci_data_types_pk PRIMARY KEY(id))";
}
void insert_sample_data_types(soci::session& sql)
{
    sql << "INSERT INTO soci_data_types(bool, i2, i4, i8, "
        "f4, f8, \"dec\", num, c1, c32, cv32, txt,"
        "varlenbin21, ts, dt, tm) "
        "VALUES (TRUE, -32768, 2147483647, 9223372036854775807,"
        "pi(), pi(), pi(), 1234, 'C', 'This is 32 long text', 'This is 32 long varchar text', 'This is text',"
        "decode('0101000000000000000000f03f0000000000000040', 'hex'), now(), now(), now())";
}

int main()
{
    try
    {
        session sql(soci::postgresql, "dbname=mloskot user=mloskot");
        bool create_table(false);
        if (create_table)
        {
            create_table_data_types(sql);
            insert_sample_data_types(sql);
        }

        // Geometry as raw BYTEA allows user to parse it
        {
            bg::model::point<float, 2,  bg::cs::geographic<bg::degree>> p0;
            bg::model::d2::point_xy<double> p1;
            bg::model::d2::point_xy<int> p2;
            sql << "SELECT varlenbin21 FROM soci_data_types LIMIT 1", into(p0);
            clog << bg::wkt(p0) << std::endl;
            sql << "SELECT varlenbin21 FROM soci_data_types LIMIT 1", into(p1);
            clog << bg::wkt(p1) << std::endl;
            sql << "SELECT varlenbin21 FROM soci_data_types LIMIT 1", into(p2);
            clog << bg::wkt(p2) << std::endl;
        }

        typedef bg::model::point<double, 2,  bg::cs::geographic<bg::degree>> point0_t;
        typedef bg::model::point<float, 2,  bg::cs::cartesian> point1_t;
        typedef bg::model::d2::point_xy<int> point2_t;

        // POINT
        {
            // TODO: currently, binary format is requested only in simple case, no use() support
            //int id(1);
            //sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=:id", into(p0), use(id);

            point0_t p0;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=1", into(p0);
            clog << bg::wkt(p0) << std::endl;

            point1_t p1;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=1", into(p1);
            clog << bg::wkt(p1) << std::endl;

            point2_t p2;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=1", into(p2);
            clog << bg::wkt(p2) << std::endl;
        }

        // LINESTRING
        {
            bg::model::linestring<point0_t> line0;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=2", into(line0);
            clog << bg::wkt(line0) << std::endl;

            bg::model::linestring<point1_t> line1;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=2", into(line1);
            clog << bg::wkt(line1) << std::endl;

            bg::model::linestring<point2_t> line2;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=2", into(line2);
            clog << bg::wkt(line2) << std::endl;

            // std::deque as non-default container
            bg::model::linestring<point0_t, std::deque> deque0;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=2", into(deque0);
            clog << bg::wkt(deque0) << std::endl;

            bg::model::linestring<point1_t, std::deque> deque1;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=2", into(deque1);
            clog << bg::wkt(deque1) << std::endl;

            bg::model::linestring<point2_t, std::deque> deque2;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=2", into(deque2);
            clog << bg::wkt(deque2) << std::endl;
        }

        // POLYGON
        {
            bg::model::polygon<point0_t> poly0;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=3", into(poly0);
            clog << bg::wkt(poly0) << std::endl;

            bg::model::polygon<point1_t> poly1;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=3", into(poly1);
            clog << bg::wkt(poly1) << std::endl;

            bg::model::polygon<point2_t> poly2;
            sql << "SELECT ST_AsBinary(geom) FROM soci_geoms WHERE id=3", into(poly2);
            clog << bg::wkt(poly2) << std::endl;
        }
    }
    catch (exception const &e)
    {
        cerr << "Error: " << e.what() << '\n';
    }
}

// Geometry loaded from PostGIS samples:
// http://postgis.org/documentation/manual-svn/using_postgis_dbmanagement.html#OpenGISWKBWKT
// using the following inserts:
//insert into soci_geoms (geom) values (st_GeometryFromText('POINT(0 0)', 4326));
//insert into soci_geoms (geom) values (st_GeometryFromText('LINESTRING(0 0,1 1,1 2)', 4326));
//insert into soci_geoms (geom) values (st_GeometryFromText('POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1, 2 1, 2 2, 1 2,1 1))', 4326));

// Sample output produced by this example:
//POINT(1 2)
//POINT(1 2)
//POINT(1 2)
//POINT(1 2)
//POINT(1 2)
//POINT(1 2)
//LINESTRING(0 0,1 1,1 2)
//LINESTRING(0 0,1 1,1 2)
//LINESTRING(0 0,1 1,1 2)
//LINESTRING(0 0,1 1,1 2)
//LINESTRING(0 0,1 1,1 2)
//LINESTRING(0 0,1 1,1 2)
//POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1))
//POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1))
//POLYGON((0 0,4 0,4 4,0 4,0 0),(1 1,2 1,2 2,1 2,1 1))
