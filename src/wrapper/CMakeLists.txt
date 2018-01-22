set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti")

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(wrapper main.cc Result.h)
set_target_properties(wrapper PROPERTIES LINKER_LANGUAGE C)

include(GNUInstallDirs)
install(TARGETS wrapper
        RUNTIME DESTINATION CMAKE_INSTALL_LIBEXECDIR)