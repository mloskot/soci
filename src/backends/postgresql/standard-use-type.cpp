//
// Copyright (C) 2004-2008 Maciej Sobczak, Stephen Hutton
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#define SOCI_POSTGRESQL_SOURCE
#include "soci-postgresql.h"
#include "blob.h"
#include "rowid.h"
#include <soci-platform.h>
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

void postgresql_standard_use_type_backend::bind_by_pos(
    int & position, void * data, exchange_type type, bool /* readOnly */)
{
    // readOnly is ignored, because PostgreSQL does not support
    // any data to be written back to used (bound) objects.

    data_ = data;
    type_ = type;
    position_ = position++;
}

void postgresql_standard_use_type_backend::bind_by_name(
    std::string const & name, void * data, exchange_type type, bool /* readOnly */)
{
    // readOnly is ignored, because PostgreSQL does not support
    // any data to be written back to used (bound) objects.

    data_ = data;
    type_ = type;
    name_ = name;
}

void postgresql_standard_use_type_backend::pre_use(indicator const* ind)
{
    buf_.data = nullptr;
    buf_.size = 0;
    buf_.binary = false;

    if (nullptr != ind && *ind == i_null)
    {
        // leave the working buffer as empty
    }
    else
    {
        // allocate and fill the buffer with text-formatted client data
        switch (type_)
        {
            // TODO: merge with soci-type-bool branch
            //case x_bool:
            //    {
            //        buf_.size = 2;
            //        buf_.data = new char[buf_.size];
            //        buf_.data[0] = (*static_cast<bool *>(data_) ) ? '1' : '0'; 
            //        buf_.data[1] = '\0'; 
            //    }
            //    break;
        case x_char:
            {
                buf_.size = 2;
                buf_.data = new char[buf_.size];
                buf_.data[0] = *static_cast<char *>(data_);
                buf_.data[1] = '\0';
            }
            break;
        case x_stdstring:
            {
                std::string* s = static_cast<std::string*>(data_);
                buf_.size = s->size() + 1;
                buf_.data = new char[buf_.size];
                std::strcpy(buf_.data, s->c_str());
            }
            break;
        case x_short:
            {
                buf_.size = std::numeric_limits<short>::digits10 + 3;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%d", static_cast<int>(*static_cast<short *>(data_)));
            }
            break;
        case x_integer:
            {
                buf_.size = std::numeric_limits<int>::digits10 + 3;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%d", *static_cast<int*>(data_));
            }
            break;
        case x_unsigned_long:
            {
                buf_.size = std::numeric_limits<unsigned long>::digits10 + 2;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%lu", *static_cast<unsigned long *>(data_));
            }
            break;
        case x_long_long:
            {
                buf_.size = std::numeric_limits<long long>::digits10 + 3;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%lld", *static_cast<long long *>(data_));
            }
            break;
        case x_unsigned_long_long:
            {
                buf_.size = std::numeric_limits<unsigned long long>::digits10 + 2;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%llu", *static_cast<unsigned long long *>(data_));
            }
            break;
        case x_double:
            {
                // no need to overengineer it (KISS)...
                buf_.size = 100;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%.20g", *static_cast<double *>(data_));
            }
            break;
        case x_stdtm:
            {
                buf_.size = 20;
                buf_.data = new char[buf_.size];
                std::tm* t = static_cast<std::tm *>(data_);
                snprintf(buf_.data, buf_.size, "%d-%02d-%02d %02d:%02d:%02d",
                    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                    t->tm_hour, t->tm_min, t->tm_sec);
            }
            break;
        case x_rowid:
            {
                // RowID is internally identical to unsigned long
                rowid* rid = static_cast<rowid*>(data_);
                postgresql_rowid_backend* rbe =
                    static_cast<postgresql_rowid_backend*>(rid->get_backend());

                buf_.size = std::numeric_limits<unsigned long>::digits10 + 2;
                buf_.data = new char[buf_.size];

                snprintf(buf_.data, buf_.size, "%lu", rbe->value_);
            }
            break;
        case x_blob:
            {
                blob* b = static_cast<blob*>(data_);
                postgresql_blob_backend* bbe = static_cast<postgresql_blob_backend*>(b->get_backend());

                buf_.size = std::numeric_limits<unsigned long>::digits10 + 2;
                buf_.data = new char[buf_.size];
                snprintf(buf_.data, buf_.size, "%lu", bbe->oid_);
            }
            break;
        default:
            throw soci_error("Use element used with non-supported type.");
        }
    }

    if (position_ > 0)
    {
        // binding by position
        statement_.useByPosBuffers_[position_] = &buf_;
    }
    else
    {
        // binding by name
        statement_.useByNameBuffers_[name_] = &buf_;
    }
}

void postgresql_standard_use_type_backend::post_use(bool /*gotData*/, indicator* /*ind*/)
{
    // PostgreSQL does not support any data moving back the same channel,
    // so there is nothing to do here.
    // In particular, there is nothing to protect, because both const and non-const
    // objects will never be modified.

    // clean up the working buffer, it might be allocated anew in
    // the next run of preUse
    clean_up();
}

void postgresql_standard_use_type_backend::clean_up()
{
    if (nullptr != buf_.data)
    {
        delete [] buf_.data;
        buf_.data = nullptr;
        buf_.size = 0;
        buf_.binary = false;
    }
}
