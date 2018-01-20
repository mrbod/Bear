set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(wrapper main.c)

include(GNUInstallDirs)
install(TARGETS wrapper
        RUNTIME DESTINATION CMAKE_INSTALL_LIBEXECDIR)