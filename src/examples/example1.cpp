#include <soci.h>
#include <soci-postgresql.h>
#include <boost-geometry.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <exception>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
using namespace soci;
using namespace std;

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

        bool b(true);
        short i2(-1);
        int i4(-1);
        long long i8(-1);
        
        //typedef std::vector<std::uint8_t> binary_t;
        {
            soci::binary_string varlenbin21;
            sql << "select varlenbin21 from soci_data_types limit 1", into(varlenbin21);
            clog << "varlenbin21=" << varlenbin21.data_.size() << endl;
            boost::geometry::model::d2::point_xy<double> p0;
            if (!boost::geometry::read_wkb(varlenbin21.data_.begin(), varlenbin21.data_.end(), p0))
                throw std::runtime_error("read_wkb failed");
            clog << boost::geometry::wkt(p0) << std::endl;
        }

        {
            boost::geometry::model::d2::point_xy<double> p0;
            //boost::geometry::model::d2::point_xy<int> p1;
            sql << "select varlenbin21 from soci_data_types limit 1", into(p1);
            clog << boost::geometry::wkt(p0) << std::endl;
        }

        


        //clog << "b=" << b << endl;
        //clog << "i2=" << i2 << endl;
        //clog << "i4=" << i4 << endl;
        //clog << "i8=" << i8 << endl;

        //b = false;
        //sql << "update soci_data_types set bool = :b", use(b, "b");
        //

        //row r;
        //sql << "select bool from soci_data_types limit 1", into(r);
        //column_properties const& props = r.get_properties(0);

        //clog << "b=" << b << endl;
    }
    catch (exception const &e)
    {
        cerr << "Error: " << e.what() << '\n';
    }
}