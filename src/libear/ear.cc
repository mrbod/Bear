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

#include "config.h"

#include <dlfcn.h>

#if defined HAVE_POSIX_SPAWN || defined HAVE_POSIX_SPAWNP

# include <spawn.h>

#endif

#if defined HAVE_NSGETENVIRON
# include <crt_externs.h>
#endif

#include <cstddef>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <string_view>


extern "C" {

void on_load() __attribute__((constructor));
void on_unload() __attribute__((destructor));

#ifdef HAVE_EXECVE
int execve(const char *path, char *const argv[], char *const envp[]);
#endif

#ifdef HAVE_EXECV
int execv(const char *path, char *const argv[]);
#endif

#ifdef HAVE_EXECVPE
int execvpe(const char *file, char *const argv[], char *const envp[]);
#endif

#ifdef HAVE_EXECVP
int execvp(const char *file, char *const argv[]);
#endif

#ifdef HAVE_EXECVP2
int execvP(const char *file, const char *search_path, char *const argv[]);
#endif

#ifdef HAVE_EXECT
int exect(const char *path, char *const argv[], char *const envp[]);
#endif

#ifdef HAVE_EXECL
int execl(const char *path, const char *arg, ...);
#endif

#ifdef HAVE_EXECLP
int execlp(const char *file, const char *arg, ...);
#endif

#ifdef HAVE_EXECLE
// int execle(const char *path, const char *arg, ..., char * const envp[]);
int execle(const char *path, const char *arg, ...);
#endif

#ifdef HAVE_POSIX_SPAWN
int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]);
#endif

#ifdef HAVE_POSIX_SPAWNP
int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]);
#endif

}


#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT "libear: (" __FILE__ ":" TOSTRING(__LINE__) ") "

#define PERROR(msg) do { perror(AT msg); } while (0)

#define ERROR_AND_EXIT(msg) do { PERROR(msg); exit(EXIT_FAILURE); } while (0)

#define DLSYM(TYPE_, VAR_, SYMBOL_)                                 \
    union {                                                         \
        void *from;                                                 \
        TYPE_ to;                                                   \
    } cast;                                                         \
    if (0 == (cast.from = dlsym(RTLD_NEXT, SYMBOL_))) {             \
        PERROR("dlsym");                                            \
        exit(EXIT_FAILURE);                                         \
    }                                                               \
    TYPE_ const VAR_ = cast.to;


namespace {
    std::atomic<bool> loaded = false;
    bool initialized = false;

    constexpr size_t page_size = 4096;
    char target[page_size];
    char library[page_size];
    char wrapper[page_size];

    constexpr char const target_env_key_literal[] = "BEAR_TARGET";
    constexpr char const library_env_key_literal[] = "BEAR_LIBRARY";
    constexpr char const wrapper_env_key_literal[] = "BEAR_WRAPPER";
    constexpr size_t target_env_key_size = sizeof(target_env_key_literal);
    constexpr size_t library_env_key_size = sizeof(library_env_key_literal);
    constexpr size_t wrapper_env_key_size = sizeof(wrapper_env_key_literal);
    constexpr std::string_view target_env_key(target_env_key_literal, target_env_key_size);
    constexpr std::string_view library_env_key(library_env_key_literal, library_env_key_size);
    constexpr std::string_view wrapper_env_key(wrapper_env_key_literal, wrapper_env_key_size);

    /**
     * Return a pointer to the last element of a nullptr terminated array.
     *
     * @param begin the input array to count,
     * @return the pointer which points the nullptr.
     */
    template<typename T>
    T *get_array_end(T *const begin) {
        auto it = begin;
        while (*it != nullptr)
            ++it;
        return it;
    }

    /**
     * Return the size of a nullptr terminated array.
     *
     * @param begin the input array to count,
     * @return the size of the array.
     */
    template<typename T>
    size_t get_array_length(T *const begin) {
        return get_array_end(begin) - begin;
    }

    /**
     * Find the environment value in the given environments array.
     *
     * @param begin the environment key=value pair array
     * @param name the environment key
     * @return the value
     */
    std::string_view get_env_value(char **const begin, std::string_view const &name) {
        auto const end = get_array_end(begin);
        auto const key_ptr = std::find_if(begin, end, [](auto env_it) {
            auto const env = std::string_view(env_it);
            // todo: implement it
            return false;
        });
        if (key_ptr != end) {
            auto const key = std::string_view(*key_ptr);
            auto const eq = std::find(key.begin(), key.end(), '=');
            return (eq != key.end())
                   ? std::string_view(eq, key.end() - eq)
                   : std::string_view();
        }
        return std::string_view();
    }

    char **capture_env_array() {
#ifdef HAVE_NSGETENVIRON
        return = *_NSGetEnviron();
#else
        void *result = dlsym(RTLD_NEXT, "environ");
        return static_cast<char **>(result);
#endif
    }

    bool capture_env_value(char **envp, std::string_view const &name, char *dst) {
        // Get the environment value.
        auto value = get_env_value(envp, name);
        // Check we have enough space to store it.
        if (value.empty() || (value.size() >= page_size))
            return false;
        // Copy it.
        auto end_ptr = std::copy(value.begin(), value.end(), dst);
        // Make a zero terminated string.
        *end_ptr = 0;

        return true;
    }
}

/* Initialization method to Captures the relevant environment variables.
 */

void on_load() {
    // Test whether on_load was called already.
    if (loaded.exchange(true))
        return;

    auto const env_ptr = capture_env_array();
    initialized =
            capture_env_value(env_ptr, target_env_key, target) &&
            capture_env_value(env_ptr, library_env_key, library) &&
            capture_env_value(env_ptr, wrapper_env_key, wrapper);
}

void on_unload() {
    initialized = false;
}

/* These are the methods we are try to hijack.
 */
namespace {
//    char const **string_array_from_varargs(char const *const arg, va_list *args) {
//        return nullptr;
//    }

    int exec_wrapper(char *const src[], int (*f)(const char *, char **const)) {
        constexpr char const *target_flag = "-t";
        constexpr char const *library_flag = "-l";

        auto const src_length = get_array_length(src);
        char *prefix[] = {
                wrapper,
                (char *) target_flag,
                target,
                (char *) library_flag,
                library
        };
        char *dst[src_length + sizeof(prefix) + 1];
        auto ptr = std::copy_n(prefix, sizeof(prefix), dst);
        auto end_ptr = std::copy_n(src, src_length, ptr);
        *end_ptr = nullptr;

        return f(wrapper, dst);
    }
}

namespace {

#ifdef HAVE_EXECVE

    int call_execve(const char *path, char *const argv[], char *const envp[]);

#endif
#ifdef HAVE_EXECVP

    int call_execvp(const char *file, char *const argv[]);

#endif
#ifdef HAVE_EXECVPE

    int call_execvpe(const char *file, char *const argv[], char *const envp[]);

#endif
#ifdef HAVE_EXECVP2

    int call_execvP(const char *file, const char *search_path, char *const argv[]);

#endif
#ifdef HAVE_EXECT

    int call_exect(const char *path, char *const argv[], char *const envp[]);

#endif
#ifdef HAVE_POSIX_SPAWN

    int call_posix_spawn(pid_t *pid, const char *path,
                         const posix_spawn_file_actions_t *file_actions,
                         const posix_spawnattr_t *attrp,
                         char *const argv[],
                         char *const envp[]);

#endif
#ifdef HAVE_POSIX_SPAWNP

    int call_posix_spawnp(pid_t *pid, const char *file,
                          const posix_spawn_file_actions_t *file_actions,
                          const posix_spawnattr_t *attrp,
                          char *const argv[],
                          char *const envp[]);

#endif

}


#ifdef HAVE_EXECVE

int execve(const char *path, char *const argv[], char *const envp[]) {
    if (initialized) {
        return exec_wrapper(argv, [](const char *path2, char **const argv2) {
            return -2;
        });
    }
    return -1;
}

#endif

#ifdef HAVE_EXECV

int execv(const char *path, char *const argv[]) {
    return -1;
}

#endif

#ifdef HAVE_EXECVPE

int execvpe(const char *file, char *const argv[], char *const envp[]) {
    return -1;
}

#endif

#ifdef HAVE_EXECVP

int execvp(const char *file, char *const argv[]) {
    return -1;
}

#endif

#ifdef HAVE_EXECVP2
int execvP(const char *file, const char *search_path, char *const argv[]) {
    return -1;
}
#endif

#ifdef HAVE_EXECT
int exect(const char *path, char *const argv[], char *const envp[]) {
    return -1;
}
#endif

#ifdef HAVE_EXECL

int execl(const char *path, const char *arg, ...) {
//    va_list args;
//    va_start(args, arg);
//    char const **argv = string_array_from_varargs(arg, &args);
//    va_end(args);

    return -1;
}

#endif

#ifdef HAVE_EXECLP

int execlp(const char *file, const char *arg, ...) {
//    va_list args;
//    va_start(args, arg);
//    char const **argv = string_array_from_varargs(arg, &args);
//    va_end(args);

    return -1;
}

#endif

#ifdef HAVE_EXECLE

// int execle(const char *path, const char *arg, ..., char * const envp[]);
int execle(const char *path, const char *arg, ...) {
//    va_list args;
//    va_start(args, arg);
//    char const **argv = string_array_from_varargs(arg, &args);
//    char const **envp = va_arg(args, char const **);
//    va_end(args);

    return -1;
}

#endif

#ifdef HAVE_POSIX_SPAWN

int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]) {
    return -1;
}

#endif

#ifdef HAVE_POSIX_SPAWNP

int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]) {
    return -1;
}

#endif
