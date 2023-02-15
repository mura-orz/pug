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
	add_executable		(pug-ut.exe		pug.hpp ut.cpp)
	add_custom_command	(TARGET pug-ut.exe		POST_BUILD	COMMAND ./pug-ut.exe)
endif()
