# CMakeList.txt : CMake project for pug, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("pug")

# Add source to this project's executable.
add_executable ("pug" "pug.cpp" "pug.hpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET "pug" PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
