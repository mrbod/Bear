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
#include <unistd.h>
#if defined HAVE_POSIX_SPAWN || defined HAVE_POSIX_SPAWNP
# include <spawn.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <atomic>

#include "environment.h"

// TODO: these macros are using libc methods (perror, exit)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT "libear: (" __FILE__ ":" TOSTRING(__LINE__) ") "

#define PERROR(msg) do { perror(AT msg); } while (false)

#define ERROR_AND_EXIT(msg) do { PERROR(msg); exit(EXIT_FAILURE); } while (false)


namespace {
    constexpr size_t page_size = 4096;
    constexpr char target_env_key[] = "BEAR_TARGET";
    constexpr char library_env_key[] = "BEAR_LIBRARY";
    constexpr char wrapper_env_key[] = "BEAR_WRAPPER";
    constexpr char target_flag[] = "-t";
    constexpr char library_flag[] = "-l";

    std::atomic<bool> loaded = false;
    bool initialized = false;

    char target[page_size];
    char library[page_size];
    char wrapper[page_size];
}

/**
 * Library entry point.
 *
 * The first method to call after the library is loaded into memory.
 */
extern "C" void on_load() __attribute__((constructor));
extern "C" void on_load() {
    // Test whether on_load was called already.
    if (loaded.exchange(true))
        return;
    // Try to get the environment variables.
    auto const env_ptr = capture_env_array();
    if (env_ptr == nullptr)
        return;
    // Capture the relevant environment variables.
    initialized =
            capture_env_value(env_ptr, target_env_key, target, page_size) &&
            capture_env_value(env_ptr, library_env_key, library, page_size) &&
            capture_env_value(env_ptr, wrapper_env_key, wrapper, page_size);
}

/**
 * Library exit point.
 *
 * The last method which needs to be called when the library is unloaded.
 */
extern "C" void on_unload() __attribute__((destructor));
extern "C" void on_unload() {
    initialized = false;
}

/* These are the methods we are try to hijack.
 */
namespace {

    int execve_wrapper(char *const src[],
                     const char ** const envp,
                     int (*fp)(const char *, const char **, const char **)) {
        const size_t src_length = get_array_length(src);
        const size_t dst_length = src_length + 6;
        const char *dst[dst_length] = {
                [0]=wrapper,
                [1]=target_flag,
                [2]=target,
                [3]=library_flag,
                [4]=library,
        };
        copy(src, src + src_length, &dst[5], dst + src_length);

        return fp(wrapper, dst, envp);
    }

    template <typename F>
    F typed_dlsym(const char *name) {
        void *symbol = dlsym(RTLD_NEXT, name);
        if (symbol == nullptr) {
            ERROR_AND_EXIT("dlsym");
        }
        return reinterpret_cast<F>(symbol);
    }
}


#ifdef HAVE_EXECVE

extern "C"
int execve(const char *, char *const argv[], char *const envp[]) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    using execve_t = int (*)(const char*, const char **, const char **);
    auto fp = typed_dlsym<execve_t>("execvpe");

    auto const_envp = const_cast<const char **>(envp);

    return execve_wrapper(argv, const_envp, fp);
}

#endif

#ifdef HAVE_EXECV

extern "C"
int execv(const char *, char *const argv[]) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    using execve_t = int (*)(const char*, const char **, const char **);
    auto fp = typed_dlsym<execve_t>("execve");

    auto const_envp = capture_env_array();

    return execve_wrapper(argv, const_envp, fp);
}

#endif

#ifdef HAVE_EXECVPE

extern "C"
int execvpe(const char *, char *const argv[], char *const envp[]) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    using execve_t = int (*)(const char*, const char **, const char **);
    auto fp = typed_dlsym<execve_t>("execvpe");

    auto const_envp = const_cast<const char **>(envp);

    return execve_wrapper(argv, const_envp, fp);
}

#endif

#ifdef HAVE_EXECVP

extern "C"
int execvp(const char *file, char *const argv[]) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    using execve_t = int (*)(const char*, const char **, const char **);
    auto fp = typed_dlsym<execve_t>("execvp");

    auto const_envp = capture_env_array();

    return execve_wrapper(argv, const_envp, fp);
}

#endif

#ifdef HAVE_EXECVP2

extern "C"
int execvP(const char *file, const char *search_path, char *const argv[]) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    using execve_t = int (*)(const char*, const char **, const char **);
    auto fp = typed_dlsym<execve_t>("execve");

    auto const_envp = capture_env_array();

    return execve_wrapper(argv, const_envp, fp);
}

#endif

#ifdef HAVE_EXECT

extern "C"
int exect(const char *path, char *const argv[], char *const envp[]) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    using execve_t = int (*)(const char*, const char **, const char **);
    auto fp = typed_dlsym<execve_t>("execve");

    auto const_envp = const_cast<const char **>(envp);

    return execve_wrapper(argv, const_envp, fp);
}

#endif

#ifdef HAVE_EXECL

extern "C"
int execl(const char *path, const char *arg, ...) {
    if (not initialized)
        ERROR_AND_EXIT("not initialized");

    va_list count;
    va_start(count, arg);
    va_list copy;
    va_copy(copy, count);
    // Count the arguments
    size_t arg_count = 0;
    for (char const *it = arg; it; it = va_arg(count, char const *)) {
        ++arg_count;
    }
    va_end(count);

    const size_t args_size = arg_count + 6;
    const char *args[args_size] = {
            [0]=wrapper,
            [1]=target_flag,
            [2]=target,
            [3]=library_flag,
            [4]=library
    };
    // Copy the arguments
    const char **dst_it = &args[5];
    for (char const *src_it = arg; src_it; src_it = va_arg(copy, char const *)) {
        *dst_it++ = src_it;
    }
    va_end(copy);

    using execv_t = int (*)(const char *, const char *argv[]);
    auto fp = typed_dlsym<execv_t>("execv");

    return fp(args[0], args);
}

#endif

#ifdef HAVE_EXECLP

extern "C"
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
extern "C"
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

extern "C"
int posix_spawn(pid_t *pid, const char *path,
                const posix_spawn_file_actions_t *file_actions,
                const posix_spawnattr_t *attrp,
                char *const argv[], char *const envp[]) {
    return -1;
}

#endif

#ifdef HAVE_POSIX_SPAWNP

extern "C"
int posix_spawnp(pid_t *pid, const char *file,
                 const posix_spawn_file_actions_t *file_actions,
                 const posix_spawnattr_t *attrp,
                 char *const argv[], char *const envp[]) {
    return -1;
}

#endif
