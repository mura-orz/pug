/// @file
///	@brief		Unit test of pug++ with gtest
///	@author		Mura
///	@copyright	(C) 2023-, Mura.

#include "pug.hpp"
#include <gtest/gtest.h>
#include <string>
#include <filesystem>
#include <system_error>

using namespace std::string_literals;

///	@name	Exceptions
///	@{

TEST(ex_syntax_error, Default) {
	xxx::pug::ex::syntax_error const	e;
	EXPECT_EQ("syntax_error"s, e.what());
}
TEST(ex_syntax_error, UderDefinedString) {
	xxx::pug::ex::syntax_error const	e{"any"s};
	EXPECT_EQ("any"s, e.what());
}
TEST(ex_syntax_error, UderDefinedLiteral) {
	xxx::pug::ex::syntax_error const	e{"any"};
	EXPECT_EQ("any"s, e.what());
}

TEST(ex_io_error, Default) {
	int const						code{static_cast<int>(std::errc::io_error)};
	std::error_code const			ec{code, std::generic_category() };
	xxx::pug::ex::io_error  const	e{ec};
	EXPECT_TRUE(std::string{e.what()}.find("io_error") != std::string::npos);
	auto const&						r	= e.code();
	EXPECT_EQ(code, r.value());
	EXPECT_EQ(std::generic_category(), r.category());
}
TEST(ex_io_error, WithPath) {
	int const						code{static_cast<int>(std::errc::io_error)};
	std::filesystem::path const		path{"/foo"};
	std::error_code const			ec{code, std::generic_category() };
	xxx::pug::ex::io_error  const	e{path, ec};
	EXPECT_TRUE(std::string{e.what()}.find(path.string()) != std::string::npos);
	auto const&						r	= e.code();
	EXPECT_EQ(code, r.value());
	EXPECT_EQ(std::generic_category(), r.category());
}

///	@}
