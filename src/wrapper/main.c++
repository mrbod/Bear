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

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <type_traits>
#include <variant>

template<class T> struct always_false : std::false_type {};

struct Error {
    explicit Error(const char * const message)
            : message_(message) {
    }

    ~Error() noexcept = default;

    const char* what() const noexcept {
        return message_;
    }

private:
    const char * const message_;
};

template <class T>
using Result = std::variant<Error, T>;


struct State {
    char * library;
    char * target;
    char ** command;

    State()
            : library(nullptr)
            , target(nullptr)
            , command(nullptr) {
    }
};

Result<State> parse(int argc, char *argv[]) {
    State result;

    int opt;
    while ((opt = getopt(argc, argv, "l:t:")) != -1) {
        switch (opt) {
            case 'l':
                result.library = optarg;
                break;
            case 't':
                result.target = optarg;
                break;
            default: /* '?' */
                return Result<State>(Error(
                        "Usage: wrapper [-t target_url] [-l path_to_libear] command"
                ));
        }
    }

    if (optind >= argc) {
        return Result<State>(Error(
                "Expected argument after options"
        ));
    } else {
        result.command = argv + optind;
    }

    return Result<State>(result);
}

int main(int argc, char *argv[], char *envp[]) {
    const Result<State>& state = parse(argc, argv);

    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, State>) {
            printf("library=%s; target=%s\n", arg.library, arg.target);
            printf("command argument:");
            for (char ** it = arg.command; *it != nullptr; ++it) {
                printf(" %s", *it);
            }
            printf("\n");
        } else if constexpr (std::is_same_v<T, Error>) {
            fprintf(stderr, "%s\n", arg.what());
            exit(EXIT_FAILURE);
        } else {
            static_assert(always_false<T>::value, "non-exhaustive visitor!");
        }
    }, state);

    exit(EXIT_SUCCESS);
}
