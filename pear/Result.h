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

#include <variant>

template<typename T>
class Result {
private:
    using Error = const char *;
    using State = std::variant<T, Error>;
    State state;

public:
    Result() = delete;

private:
    explicit Result(const T &other) noexcept
            : state(State(other)) {
    }

    explicit Result(Error const error) noexcept
            : state(State(error)) {
    }

public:
    Result(const Result &other) noexcept = delete;

    Result(Result &&other) noexcept {
        state = other.state;
    }

    Result &operator=(const Result &other) = delete;

    Result &operator=(Result &&other) noexcept {
        if (this != &other) {
            state = other.state;
        }
        return *this;
    }

    ~Result() noexcept = default;

public:
    static Result success(const T &value) noexcept {
        return Result(value);
    }

    static Result failure(Error const value) noexcept {
        return Result(value);
    }

public:
    template<typename U>
    Result<U> map(U (*f)(const T &)) const noexcept {
        if (auto ptr = std::get_if<T>(&state)) {
            return Result<U>::success(f(*ptr));
        } else if (auto error = std::get_if<Error>(&state)) {
            return Result<U>::failure(*error);
        }
    }

    template<typename U>
    Result<U> bind(Result<U> (*f)(const T &)) const noexcept {
        if (auto ptr = std::get_if<T>(&state)) {
            return f(*ptr);
        } else if (auto error = std::get_if<Error>(&state)) {
            return Result<U>::failure(*error);
        }
    }

    T get_or_else(const T &value) const noexcept {
        if (auto ptr = std::get_if<T>(&state)) {
            return *ptr;
        } else {
            return value;
        }
    }

    void handle_with(void (*f)(const char *)) const noexcept {
        if (auto error = std::get_if<Error>(&state)) {
            f(*error);
        };
    }
};
