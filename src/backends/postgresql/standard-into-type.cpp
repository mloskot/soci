//
// Copyright (C) 2011 Mateusz Loskot
// Copyright (C) 2011 Artur Bać
// Copyright (C) 2004-2008 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_POSTGRESQL_SOURCE
#include "soci-postgresql.h"
#include "common.h"
#include "rowid.h"
#include "binary.h"
#include "blob.h"
#include "endian_value.hpp"
#include <libpq/libpq-fs.h> // libpq
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <sstream>

unsigned long ByteSwap2 (unsigned long nLongNumber)
{
   return (((nLongNumber&0x000000FF)<<24)+((nLongNumber&0x0000FF00)<<8)+
   ((nLongNumber&0x00FF0000)>>8)+((nLongNumber&0xFF000000)>>24));
}

#ifdef SOCI_POSTGRESQL_NOPARAMS
#define SOCI_POSTGRESQL_NOBINDBYNAME
#endif // SOCI_POSTGRESQL_NOPARAMS

using namespace soci;
using namespace soci::details;
using namespace soci::details::postgresql;

void postgresql_standard_into_type_backend::define_by_pos(
    int& position, void* data, exchange_type type)
{
    data_ = data;
    type_ = type;
    position_ = position++;
}

void postgresql_standard_into_type_backend::pre_fetch()
{
    // nothing to do here
}

void postgresql_standard_into_type_backend::post_fetch(
    bool gotData, bool calledFromFetch, indicator* ind)
{
    if (calledFromFetch && !gotData)
    {
        // this is a normal end-of-rowset condition,
        // no need to do anything (fetch() will return false)
        return;
    }

    if (gotData)
    {
        // postgresql_ positions start at 0
        int const pos = position_ - 1;

        // first, deal with indicators
        if (PQgetisnull(statement_.result_, statement_.currentRow_, pos) != 0)
        {
            if (nullptr == ind)
            {
                throw soci_error("Null value fetched and no indicator defined.");
            }

            *ind = i_null;

            // no need to convert data if it is null
            return;
        }
        else
        {
            if (nullptr != ind)
                *ind = i_ok;
        }

        // raw data, in text format or binary format
        details::buffer_descriptor buf =
        {
            PQgetvalue(statement_.result_, statement_.currentRow_, pos),
            static_cast<std::size_t>(PQgetlength(statement_.result_, statement_.currentRow_, pos)),
            PQfformat(statement_.result_, pos) == 1
        };

        switch (type_)
        {
        case x_char:
            {
                char* dest = static_cast<char *>(data_);
                *dest = *buf.data;
            }
            break;
        case x_stdstring:
            {
                std::string* dest = static_cast<std::string *>(data_);
                dest->assign(buf.data, buf.size);
            }
            break;
        case x_binary_string:
            {
                binary_string* dest = static_cast<binary_string*>(data_);
                if (buf.binary)
                {
                    std::uint8_t const* buf_begin = reinterpret_cast<std::uint8_t const*>(buf.data);
                    dest->data_.assign(buf_begin, buf_begin + buf.size);
                }
                else
                {
                    std::size_t retbuflen = 0;
                    unsigned char* bytea = PQunescapeBytea(reinterpret_cast<unsigned char*>(buf.data), &retbuflen);
                    dest->data_.assign(bytea, bytea + retbuflen);
                    PQfreemem(bytea);
                }
            }
            break;
        case x_short:
            {
                short* dest = static_cast<short*>(data_);
                if (buf.binary)
                    *dest = (buf.data[0]<<8) | buf.data[1];
                else
                    *dest = string_to_integer<short>(buf.data);
            }
            break;
        case x_integer:
            {
                int* dest = static_cast<int*>(data_);
                if (buf.binary)
                {
                    switch (buf.size)
                    {
                    case 1:
                        *dest = buf.data[0];
                        break;
                    case 2:
                        *dest = ((int)buf.data[0]<<8) | (int)buf.data[1];
                        break;
                    case 4:
                        //*dest = ((int)buf.data[0]<<24) | ((int)buf.data[1]<<16) |((int)buf.data[2]<<8) | (int)buf.data[3];
                        details::endian_value<int> v;
                        v.load<details::big_endian_tag>(buf.data);
                        *dest = v;
                        break;
                    }
                }
                else
                {
                    *dest = string_to_integer<int>(buf.data);
                }
            }
            break;
        case x_unsigned_long:
            {
                unsigned long* dest = static_cast<unsigned long*>(data_);
                if (buf.binary)
                {
                    details::endian_value<unsigned long> v;
                    v.load<details::big_endian_tag>(buf.data);
                    unsigned long xxx = v;

                    unsigned long xxx2 = ByteSwap2(xxx);
                    *dest = v; // ((unsigned)buf.data[0]<<24) | ((unsigned)buf.data[1]<<16) | ((unsigned)buf.data[2]<<8) | (unsigned)buf.data[3];
                }
                else
                    *dest = string_to_unsigned_integer<unsigned long>(buf.data);
            }
            break;
        case x_long_long:
            {
                long long* dest = static_cast<long long*>(data_);
                if (buf.binary)
                {
                    union
                    {
                        long long value;
                        uint8_t bytes[8]; 
                    } tmp;
                    tmp.bytes[7] = buf.data[0];
                    tmp.bytes[6] = buf.data[1];
                    tmp.bytes[5] = buf.data[2];
                    tmp.bytes[4] = buf.data[3];
                    tmp.bytes[3] = buf.data[4];
                    tmp.bytes[2] = buf.data[5];
                    tmp.bytes[1] = buf.data[6];
                    tmp.bytes[0] = buf.data[7];
                    *dest = tmp.value;		
                }
                else
                {
                    *dest = string_to_integer<long long>(buf.data);
                }
            }
            break;
        case x_unsigned_long_long:
            {
                unsigned long long* dest = static_cast<unsigned long long*>(data_);
                if (buf.binary)
                {
                    union
                    {
                        unsigned long long value;
                        uint8_t bytes[8]; 
                    } tmp;
                    tmp.bytes[7] = buf.data[0];
                    tmp.bytes[6] = buf.data[1];
                    tmp.bytes[5] = buf.data[2];
                    tmp.bytes[4] = buf.data[3];
                    tmp.bytes[3] = buf.data[4];
                    tmp.bytes[2] = buf.data[5];
                    tmp.bytes[1] = buf.data[6];
                    tmp.bytes[0] = buf.data[7];
                    *dest = tmp.value;		
                }
                else
                {
                    *dest = string_to_unsigned_integer<unsigned long long>(buf.data);
                }
            }
            break;
        case x_double:
            {
                double* dest = static_cast<double*>(data_);
                if (buf.binary)
                {
                    switch (buf.size)
                    {
                    case 4:
                        {
                            union
                            {
                                float value;
                                uint8_t bytes[4]; 
                            } tmp;
                            tmp.bytes[3] = buf.data[0];
                            tmp.bytes[2] = buf.data[1];
                            tmp.bytes[1] = buf.data[2];
                            tmp.bytes[0] = buf.data[3];
                            *dest = tmp.value;
                            break;
                        }
                    case 8:
                        {
                            union
                            {
                                double value;
                                uint8_t bytes[8]; 
                            } tmp;
                            tmp.bytes[7] = buf.data[0];
                            tmp.bytes[6] = buf.data[1];
                            tmp.bytes[5] = buf.data[2];
                            tmp.bytes[4] = buf.data[3];
                            tmp.bytes[3] = buf.data[4];
                            tmp.bytes[2] = buf.data[5];
                            tmp.bytes[1] = buf.data[6];
                            tmp.bytes[0] = buf.data[7];
                            *dest = tmp.value;
                            break;
                        }
                    }
                }
                else
                {
                    *dest = string_to_double(buf.data);
                }
            }
            break;
        case x_stdtm:
            {
                // attempt to parse the string and convert to std::tm
                std::tm* dest = static_cast<std::tm*>(data_);
                parse_std_tm(*dest, buf.data);
            }
            break;
        case x_rowid:
            {
                // RowID is internally identical to unsigned long

                rowid* rid = static_cast<rowid*>(data_);
                postgresql_rowid_backend* rbe = 
                    static_cast<postgresql_rowid_backend*>(rid->get_backend());

                if (buf.binary)
                    rbe->value_ = ((unsigned)buf.data[0]<<24) | ((unsigned)buf.data[1]<<16) |((unsigned)buf.data[2]<<8) | (unsigned)buf.data[3];
                else
                    rbe->value_ = string_to_unsigned_integer<unsigned long>(buf.data);
            }
            break;
        case x_blob:
            {
                unsigned long oid = string_to_unsigned_integer<unsigned long>(buf.data);
                int const fd = lo_open(statement_.session_.conn_, oid, INV_READ | INV_WRITE);
                if (fd == -1)
                {
                    throw soci_error("Cannot open the blob object.");
                }

                blob* b = static_cast<blob*>(data_);
                postgresql_blob_backend* bbe =
                    static_cast<postgresql_blob_backend*>(b->get_backend());

                if (bbe->fd_ != -1)
                {
                    lo_close(statement_.session_.conn_, bbe->fd_);
                }

                bbe->fd_ = fd;
                bbe->oid_ = oid;
            }
            break;

        default:
            throw soci_error("Into element used with non-supported type.");
        }
    }
}

void postgresql_standard_into_type_backend::clean_up()
{
    // nothing to do here
}
