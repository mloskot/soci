//
// Copyright (C) 2004-2008 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_POSTGRESQL_SOURCE
#include "soci-postgresql.h"
#include "common.h"
#include <libpq/libpq-fs.h> // libpq
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <limits>
#include <sstream>

#ifdef SOCI_POSTGRESQL_NOPARAMS
#define SOCI_POSTGRESQL_NOBINDBYNAME
#endif // SOCI_POSTGRESQL_NOPARAMS

#ifdef _MSC_VER
#pragma warning(disable:4355 4996)
#define snprintf _snprintf
#endif

using namespace soci;
using namespace soci::details;
using namespace soci::details::postgresql;


void postgresql_vector_use_type_backend::bind_by_pos(int & position,
        void * data, exchange_type type)
{
    data_ = data;
    type_ = type;
    position_ = position++;
}

void postgresql_vector_use_type_backend::bind_by_name(
    std::string const & name, void * data, exchange_type type)
{
    data_ = data;
    type_ = type;
    name_ = name;
}

void postgresql_vector_use_type_backend::pre_use(indicator const * ind)
{
    std::size_t const vsize = size();
    for (size_t i = 0; i != vsize; ++i)
    {
        details::buffer_descriptor buf = { nullptr, 0, false };

        // the data in vector can be either i_ok or i_null
        if (nullptr == ind || ind[i] != i_null)
        {
            // allocate and fill the buffer with text-formatted client data
            switch (type_)
            {
            case x_char:
                {
                    std::vector<char>* pv = static_cast<std::vector<char>*>(data_);
                    std::vector<char>& v = *pv;

                    buf.size = 2;
                    buf.data = new char[buf.size];
                    buf.data [0] = v[i];
                    buf.data [1] = '\0';
                }
                break;
            case x_stdstring:
                {
                    std::vector<std::string>* pv = static_cast<std::vector<std::string>*>(data_);
                    std::vector<std::string>& v = *pv;

                    buf.size = v[i].size() + 1;
                    buf.data = new char[buf.size];
                    std::strcpy(buf.data, v[i].c_str());
                }
                break;
            case x_short:
                {
                    std::vector<short>* pv = static_cast<std::vector<short>*>(data_);
                    std::vector<short>& v = *pv;

                    buf.size = std::numeric_limits<short>::digits10 + 3;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%d", static_cast<int>(v[i]));
                }
                break;
            case x_integer:
                {
                    std::vector<int>* pv = static_cast<std::vector<int>*>(data_);
                    std::vector<int>& v = *pv;

                    buf.size = std::numeric_limits<int>::digits10 + 3;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%d", v[i]);
                }
                break;
            case x_unsigned_long:
                {
                    std::vector<unsigned long>* pv = static_cast<std::vector<unsigned long>*>(data_);
                    std::vector<unsigned long>& v = *pv;

                    buf.size = std::numeric_limits<unsigned long>::digits10 + 2;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%lu", v[i]);
                }
                break;
            case x_long_long:
                {
                    std::vector<long long>* pv = static_cast<std::vector<long long>*>(data_);
                    std::vector<long long>& v = *pv;

                    buf.size = std::numeric_limits<long long>::digits10 + 3;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%lld", v[i]);
                }
                break;
            case x_unsigned_long_long:
                {
                    std::vector<unsigned long long>* pv =
                        static_cast<std::vector<unsigned long long>*>(data_);
                    std::vector<unsigned long long>& v = *pv;

                    buf.size = std::numeric_limits<unsigned long long>::digits10 + 2;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%llu", v[i]);
                }
                break;
            case x_double:
                {
                    // no need to overengineer it (KISS)...
                    std::vector<double>* pv = static_cast<std::vector<double>*>(data_);
                    std::vector<double>& v = *pv;

                    buf.size = 100;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%.20g", v[i]);
                }
                break;
            case x_stdtm:
                {
                    std::vector<std::tm>* pv = static_cast<std::vector<std::tm>*>(data_);
                    std::vector<std::tm>& v = *pv;

                    buf.size = 20;
                    buf.data = new char[buf.size];
                    snprintf(buf.data, buf.size, "%d-%02d-%02d %02d:%02d:%02d",
                        v[i].tm_year + 1900, v[i].tm_mon + 1, v[i].tm_mday,
                        v[i].tm_hour, v[i].tm_min, v[i].tm_sec);
                }
                break;
            default:
                throw soci_error("Use vector element used with non-supported type");
            }
        }

        buffers_.push_back(buf);
    }

    if (position_ > 0)
    {
        // binding by position
        statement_.useByPosBuffers_[position_] = &buffers_[0];
    }
    else
    {
        // binding by name
        statement_.useByNameBuffers_[name_] = &buffers_[0];
    }
}

std::size_t postgresql_vector_use_type_backend::size()
{
    std::size_t sz = 0; // dummy initialization to please the compiler
    switch (type_)
    {
        // simple cases
    case x_char:
        sz = get_vector_size<char>(data_);
        break;
    case x_short:
        sz = get_vector_size<short>(data_);
        break;
    case x_integer:
        sz = get_vector_size<int>(data_);
        break;
    case x_unsigned_long:
        sz = get_vector_size<unsigned long>(data_);
        break;
    case x_long_long:
        sz = get_vector_size<long long>(data_);
        break;
    case x_unsigned_long_long:
        sz = get_vector_size<unsigned long long>(data_);
        break;
    case x_double:
        sz = get_vector_size<double>(data_);
        break;
    case x_stdstring:
        sz = get_vector_size<std::string>(data_);
        break;
    case x_stdtm:
        sz = get_vector_size<std::tm>(data_);
        break;
    default:
        throw soci_error("Use vector element used with non-supported type.");
    }

    return sz;
}

void postgresql_vector_use_type_backend::clean_up()
{
    std::size_t const bsize = buffers_.size();
    for (std::size_t i = 0; i != bsize; ++i)
    {
        details::buffer_descriptor& buf = buffers_[i];
        delete [] buf.data;
        buf.data = nullptr;
        buf.size = 0;

        // TODO: does it make any diff if x_string_binary can be null
        //buf.binary = false;
    }
}
