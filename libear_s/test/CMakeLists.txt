find_package(GTest REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../include)

add_executable(ear_test ../test/ear_test.cc)
target_link_libraries(ear_test GTest::GTest GTest::Main)

enable_testing()
add_test(NAME EarUnitTests COMMAND ear_test)
