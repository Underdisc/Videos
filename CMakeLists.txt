cmake_minimum_required(VERSION 3.18)
project($ENV{target})
add_subdirectory($ENV{varkorRepo} varkor)
add_executable($ENV{target})
add_subdirectory(src)
target_link_libraries($ENV{target} varkorLibrary)
set_property(TARGET $ENV{target} PROPERTY RUNTIME_OUTPUT_DIRECTORY
  "${CMAKE_SOURCE_DIR}/working")

