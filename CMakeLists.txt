cmake_minimum_required (VERSION 3.1)
project (blessed)

add_subdirectory(vendor/libtwittercpp)
add_subdirectory(vendor/verbly)

find_package(PkgConfig)
pkg_check_modules(yaml-cpp yaml-cpp REQUIRED)

include_directories(
  vendor/libtwittercpp/src
  vendor/verbly/lib
  ${yaml-cpp_INCLUDE_DIRS})

add_executable(blessed blessed.cpp)
set_property(TARGET blessed PROPERTY CXX_STANDARD 11)
set_property(TARGET blessed PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries(blessed twitter++ ${yaml-cpp_LIBRARIES} verbly)
