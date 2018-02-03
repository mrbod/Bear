/*  Copyright (C) 2012-2017 by László Nagy
    This file is part of Bear.

    Bear is a tool to generate compilation database for clang tooling.

    Bear is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bear is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <dlfcn.h>

#if defined HAVE_NSGETENVIRON
# include <crt_externs.h>
#endif

#include <cstddef>
#include <algorithm>

#include "string_functions.h"

namespace {

    /**
     * Find the environment value in the given environments array.
     *
     * @param begin pointer to environment key=value pair array
     * @param end pointer to the end of the environment array
     * @param name_begin the environment key
     * @param name_end the environment key end pointer
     * @return the value
     */
    inline
    const char *get_env_value(const char **const begin,
                              const char **const end,
                              const char *const name_begin,
                              const char *const name_end) {
        for (const char **it = begin; it != end; ++it) {
            // Find the equal sign in the pointed string
            const char *const it_end = get_array_end(*it);
            const char *sep_it = *it;
            for (; sep_it != it_end; ++sep_it) {
                if (*sep_it == '=')
                    break;
            }
            // Check the equal sign is found.
            if (sep_it == it_end)
                continue;
            // Check that pointed string key is equal to the name.
            if (! std::equal(*it, sep_it, name_begin, name_end))  // TODO: kill memcmp
                continue;
            // Return the pointed string end.
            if (sep_it + 1 != nullptr)
                return sep_it + 1;
        }
        return nullptr;
    }

    /**
     * Get reference to the process environment array.
     *
     * @return pointer to the environment array.
     */
    inline
    const char **capture_env_array() {
#ifdef HAVE_NSGETENVIRON
        return = *_NSGetEnviron();
#else
        void *result = dlsym(RTLD_NEXT, "environ");
        return reinterpret_cast<const char **>(result);
#endif
    }

    /**
     * Copy the environment value into the given buffer.
     *
     * @param envp the environment array
     * @param name the environment key
     * @param dst the destination string buffer
     * @return true if the value was copied to the given buffer
     */
    inline
    bool capture_env_value(const char **const envp,
                           const char *const name,
                           char *const dst, const size_t dst_size) {
        const char **const env_begin = envp;
        const char **const env_end = get_array_end(envp);
        const char *const name_end = get_array_end(name);
        // Get the environment value.
        auto value = get_env_value(env_begin, env_end, name, name_end);
        if (value == nullptr)
            return false;
        // Check we have enough space to store it.
        const char *const value_end = get_array_end(value) + 1;
        if (dst_size <= (value_end - value))
            return false;
        // Copy it.
        copy(value, value_end, dst, dst + dst_size);

        return true;
    }

}
