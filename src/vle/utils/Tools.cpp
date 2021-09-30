/*
 * This file is part of VLE, a framework for multi-modeling, simulation
 * and analysis of complex dynamical systems.
 * https://www.vle-project.org
 *
 * Copyright (c) 2003-2018 Gauthier Quesnel <gauthier.quesnel@inra.fr>
 * Copyright (c) 2003-2018 ULCO http://www.univ-littoral.fr
 * Copyright (c) 2007-2018 INRA http://www.inra.fr
 *
 * See the AUTHORS or Authors.txt file for copyright owners and
 * contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <vle/utils/Exception.hpp>
#include <vle/utils/Tools.hpp>
#include <vle/utils/Types.hpp>

#include "utils/i18n.hpp"

#include <iomanip>
#include <sstream>

#include <cstdarg>

#ifdef _WIN32
#include <io.h>
#endif

namespace vle {
namespace utils {

std::string
format(const char* fmt, ...) noexcept
{
    try {
        int n;
        int size = 256;
        std::string ret(size, '\0');

        for (;;) {
            va_list ap;
            va_start(ap, fmt);
            n = vsnprintf(&ret[0], size, fmt, ap);
            va_end(ap);

            if (n < 0)
                return {};

            if (n < size) {
                ret.erase(n, std::string::npos);
                return ret;
            }

            size = n + 1;
            ret.resize(size);
        }
    } catch (const std::bad_alloc& e) {
        return {};
    }
}

std::string
vformat(const char* fmt, va_list ap) noexcept
{
    try {
        int n;
        int size = 256;
        std::string ret(size, '\0');

        for (;;) {
            n = vsnprintf(&ret[0], size, fmt, ap);

            if (n < 0)
                return {};

            if (n < size) {
                ret.erase(n, std::string::npos);
                return ret;
            }

            size = n + 1;
            ret.resize(size);
        }
    } catch (const std::bad_alloc& e) {
        return {};
    }
}

template<typename T>
bool
is(const std::string& str)
{
    std::stringstream s(str);
    T t;
    s >> t;

    return not s.fail();
}

template<>
bool VLE_API
is<bool>(const std::string& str)
{
    if (str == "true")
        return true;

    if (str == "false")
        return false;

    std::stringstream s(str);
    bool b;
    s >> b;

    return not s.fail();
}

template VLE_API bool
is<int8_t>(const std::string& str);
template VLE_API bool
is<int16_t>(const std::string& str);
template VLE_API bool
is<int32_t>(const std::string& str);
template VLE_API bool
is<uint8_t>(const std::string& str);
template VLE_API bool
is<uint16_t>(const std::string& str);
template VLE_API bool
is<uint32_t>(const std::string& str);
template VLE_API bool
is<double>(const std::string& str);
template VLE_API bool
is<float>(const std::string& str);

template<typename T>
std::string
to(const T t)
{
    std::ostringstream o;
    o << t;
    return o.str();
}

template<>
std::string VLE_API
to<bool>(const bool t)
{
    if (t) {
        return "true";
    } else {
        return "false";
    }
}

template VLE_API std::string
to<int8_t>(const int8_t t);
template VLE_API std::string
to<int16_t>(const int16_t t);
template VLE_API std::string
to<int32_t>(const int32_t t);
template VLE_API std::string
to<uint8_t>(const uint8_t t);
template VLE_API std::string
to<uint16_t>(const uint16_t t);
template VLE_API std::string
to<uint32_t>(const uint32_t t);
template VLE_API std::string
to<double>(const double t);
template VLE_API std::string
to<float>(const float t);

template<typename T>
T
to(const std::string& str)
{
    std::stringstream s(str);
    T ret;
    if (s >> ret)
        return ret;

    throw utils::ArgError(_("Can not convert `%s'"), str.c_str());
}

template VLE_API bool
to<bool>(const std::string& str);
template VLE_API int8_t
to<int8_t>(const std::string& str);
template VLE_API int16_t
to<int16_t>(const std::string& str);
template VLE_API int32_t
to<int32_t>(const std::string& str);
template VLE_API uint8_t
to<uint8_t>(const std::string& str);
template VLE_API uint16_t
to<uint16_t>(const std::string& str);
template VLE_API uint32_t
to<uint32_t>(const std::string& str);
template VLE_API double
to<double>(const std::string& str);
template VLE_API float
to<float>(const std::string& str);

bool
isLocaleAvailable(const std::string& locale)
{
    try {
        std::locale tmp(locale.c_str());
        return true;
    } catch (const std::runtime_error& /*e*/) {
        return false;
    }
}

std::string
toScientificString(const double& v, bool locale)
{
    std::ostringstream o;
    if (locale) {
        std::locale selected("");
        o.imbue(selected);
    }

    o << std::setprecision(std::numeric_limits<double>::digits10) << v;

    return o.str();
}

void
tokenize(const std::string& str,
         std::vector<std::string>& tokens,
         const std::string& delim,
         bool trimEmpty)
{
    std::string::size_type pos, lastPos = 0, length = str.length();

    while (lastPos < length + 1) {
        pos = str.find_first_of(delim, lastPos);
        if (pos == std::string::npos) {
            pos = length;
        }
        if (pos != lastPos || !trimEmpty)
            tokens.emplace_back(str.data() + lastPos, pos - lastPos);
        lastPos = pos + 1;
    }
}
}
} // namespace vle utils
