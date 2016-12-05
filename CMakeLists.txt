cmake_minimum_required (VERSION 3.1)
project (blessed)

add_subdirectory(vendor/libtwittercpp)
add_subdirectory(vendor/verbly)
add_subdirectory(vendor/yaml-cpp EXCLUDE_FROM_ALL)

include_directories(vendor/libtwittercpp/src vendor/verbly/lib vendor/yaml-cpp/include)
add_executable(blessed blessed.cpp)
set_property(TARGET blessed PROPERTY CXX_STANDARD 11)
set_property(TARGET blessed PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(blessed twitter++ yaml-cpp verbly)
