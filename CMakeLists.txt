# It is just minimal.
# C++20 is required.
cmake_minimum_required (VERSION 3.8)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project (pug)

# CUI utility
add_executable	(pug	pug.hpp pug.cpp)

# Unit test with googletest.
# googletest:
#	Ex)  $ apt install libgtest-dev
find_package (GTest)
if (GTest_FOUND)
	enable_testing()

	add_executable			(pug-ut.exe		pug.hpp ut.cpp)
	target_link_libraries	(pug-ut.exe		${GTEST_BOTH_LIBRARIES})
	add_test				(unittest		pug-ut.exe)
endif()
