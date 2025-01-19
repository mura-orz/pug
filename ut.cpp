/// @file
///	@brief		Unit test of pug++ with gtest
///	@author		Mura
///	@copyright	(C) 2023-, Mura.

#include <system_error>
#include "pug.hpp"
#include <filesystem>
#include <gtest/gtest.h>
#include <string>

using namespace std::string_literals;

///	@name	Exceptions
///	@{

TEST(ex_syntax_error, Default) {
	xxx::pug::ex::syntax_error const e;
	EXPECT_EQ("syntax_error"s, e.what());
}
TEST(ex_syntax_error, UserDefinedString) {
	xxx::pug::ex::syntax_error const e{"any"s};
	EXPECT_EQ("any"s, e.what());
}
TEST(ex_syntax_error, UserDefinedLiteral) {
	xxx::pug::ex::syntax_error const e{"any"};
	EXPECT_EQ("any"s, e.what());
}

TEST(ex_io_error, Default) {
	int const					 code{static_cast<int>(std::errc::io_error)};
	std::error_code const		 ec{code, std::generic_category()};
	xxx::pug::ex::io_error const e{ec};
	EXPECT_TRUE(std::string{e.what()}.find("io_error") != std::string::npos);
	auto const& r = e.code();
	EXPECT_EQ(code, r.value());
	EXPECT_EQ(std::generic_category(), r.category());
}
TEST(ex_io_error, WithPath) {
	int const					 code{static_cast<int>(std::errc::io_error)};
	std::filesystem::path const	 path{"/foo"};
	std::error_code const		 ec{code, std::generic_category()};
	xxx::pug::ex::io_error const e{path, ec};
	EXPECT_TRUE(std::string{e.what()}.find(path.string()) != std::string::npos);
	auto const& r = e.code();
	EXPECT_EQ(code, r.value());
	EXPECT_EQ(std::generic_category(), r.category());
}

TEST(def_void_ops, Contains) {
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("br"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("hr"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("img"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("meta"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("input"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("link"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("area"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("base"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("col"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("embed"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("param"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("source"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("track"));
	EXPECT_TRUE(xxx::pug::impl::def::void_tags.contains("wbr"));
}
TEST(def_void_ops, Count) {
	EXPECT_EQ(14, xxx::pug::impl::def::void_tags.size());
}
TEST(def_compare_ops, Contains) {
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains("=="));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains("==="));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains("!="));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains("!=="));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains("<"));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains("<="));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains(">"));
	EXPECT_TRUE(xxx::pug::impl::def::compare_ops.contains(">="));
}
TEST(def_compare_ops, Count) {
	EXPECT_EQ(8, xxx::pug::impl::def::compare_ops.size());
}
TEST(def_assign_ops, Contains) {
	EXPECT_TRUE(xxx::pug::impl::def::assign_ops.contains("="));
	EXPECT_TRUE(xxx::pug::impl::def::assign_ops.contains("+="));
	EXPECT_TRUE(xxx::pug::impl::def::assign_ops.contains("-="));
	EXPECT_TRUE(xxx::pug::impl::def::assign_ops.contains("*="));
	EXPECT_TRUE(xxx::pug::impl::def::assign_ops.contains("/="));
	EXPECT_TRUE(xxx::pug::impl::def::assign_ops.contains("%="));
}
TEST(def_assign_ops, Count) {
	EXPECT_EQ(6, xxx::pug::impl::def::assign_ops.size());
}
TEST(def_escapes, Contains) {
	EXPECT_TRUE(xxx::pug::impl::def::escapes.contains('<'));
	EXPECT_TRUE(xxx::pug::impl::def::escapes.contains('>'));
	EXPECT_TRUE(xxx::pug::impl::def::escapes.contains('&'));
	EXPECT_TRUE(xxx::pug::impl::def::escapes.contains('"'));
	EXPECT_TRUE(xxx::pug::impl::def::escapes.contains('\''));
}
TEST(def_escapes, Entity) {
	EXPECT_EQ("&lt;", xxx::pug::impl::def::escapes.at('<'));
	EXPECT_EQ("&gt;", xxx::pug::impl::def::escapes.at('>'));
	EXPECT_EQ("&amp;", xxx::pug::impl::def::escapes.at('&'));
	EXPECT_EQ("&quot;", xxx::pug::impl::def::escapes.at('"'));
	EXPECT_EQ("&#39;", xxx::pug::impl::def::escapes.at('\''));
}
TEST(def_escapes, Count) {
	EXPECT_EQ(5, xxx::pug::impl::def::escapes.size());
}
TEST(def_raw_html_sv, String) {
	EXPECT_EQ(".", xxx::pug::impl::def::raw_html_sv);
}
TEST(def_folding_sv, String) {
	EXPECT_EQ("| ", xxx::pug::impl::def::folding_sv);
}
TEST(def_comment_sv, String) {
	EXPECT_EQ("//-", xxx::pug::impl::def::comment_sv);
}
TEST(def_raw_comment_sv, String) {
	EXPECT_EQ("//", xxx::pug::impl::def::raw_comment_sv);
}
TEST(def_var_sv, String) {
	EXPECT_EQ("#{", xxx::pug::impl::def::var_sv);
}
TEST(def_default_sv, String) {
	EXPECT_EQ("default", xxx::pug::impl::def::default_sv);
}

TEST(def_binary_op_re, Regex) {
	// Separators between operator and operands are required
	// because it deals as just tokens.
	{
		std::string const s{R"(ab + cd)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("ab"s, m.str(1));
		EXPECT_EQ("+"s, m.str(2));
		EXPECT_EQ("cd"s, m.str(3));
	}
	{
		std::string const s{R"(1 + 3)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("1"s, m.str(1));
		EXPECT_EQ("+"s, m.str(2));
		EXPECT_EQ("3"s, m.str(3));
	}
	{
		std::string const s{R"(+ 123 %)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("+"s, m.str(1));
		EXPECT_EQ("123"s, m.str(2));
		EXPECT_EQ("%"s, m.str(3));
	}
	{
		std::string const s{R"(ab	+	cd)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("ab"s, m.str(1));
		EXPECT_EQ("+"s, m.str(2));
		EXPECT_EQ("cd"s, m.str(3));
	}
	{
		std::string const s{R"(ab 	+ cd)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("ab"s, m.str(1));
		EXPECT_EQ("+"s, m.str(2));
		EXPECT_EQ("cd"s, m.str(3));
	}
	{
		std::string const s{R"( 1 + 3)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
	}
	{
		std::string const s{R"(1 + 3 )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
	}
	{
		std::string const s{R"(1 3)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
	}
	{
		std::string const s{R"(13)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
	}
	{
		std::string const s{R"(1 + - 3)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::binary_op_re));
	}
}
TEST(def_string_re, Regex) {
	{
		std::string const s{R"("")"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\""s, m.str(1));
		EXPECT_EQ(""s, m.str(2));
		EXPECT_EQ("\""s, m.str(3));
	}
	{
		std::string const s{R"("a")"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\""s, m.str(1));
		EXPECT_EQ("a"s, m.str(2));
		EXPECT_EQ("\""s, m.str(3));
	}
	{	 //  Such case is passed, too.
		std::string const s{R"("a')"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\""s, m.str(1));
		EXPECT_EQ("a"s, m.str(2));
		EXPECT_EQ("'"s, m.str(3));
	}
	{	 // Such case is passed, too.
		std::string const s{R"('a")"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("'"s, m.str(1));
		EXPECT_EQ("a"s, m.str(2));
		EXPECT_EQ("\""s, m.str(3));
	}
	{
		std::string const s{R"('a')"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("'"s, m.str(1));
		EXPECT_EQ("a"s, m.str(2));
		EXPECT_EQ("'"s, m.str(3));
	}
	{
		std::string const s{R"('"')"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
	}
	{
		std::string const s{R"( 'a')"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
	}
	{
		std::string const s{R"('a' )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::string_re));
	}
}
TEST(def_integer_re, Regex) {
	{
		std::string const s{R"(0)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("0"s, m.str(1));
	}
	{
		std::string const s{R"(0123456789)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("0123456789"s, m.str(1));
	}
	{
		std::string const s{R"(-0123456789)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("-0123456789"s, m.str(1));
	}
	{
		std::string const s{R"( -1)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
	}
	{
		std::string const s{R"(-1 )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
	}
	{
		std::string const s{R"(a)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
	}
	{
		std::string const s{R"(-1a)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::integer_re));
	}
}

TEST(def_doctype_re, Regex) {
	// A part of type is dealt as case-sensitive although 'doctype' keyword is case-insensitive.
	{
		std::string const s{R"(doctype abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(DOCTYPE abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(DocType abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(doctype abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(doctype 1)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("1"s, m.str(1));
	}
	{
		std::string const s{R"(doctype __)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("__"s, m.str(1));
	}
	{
		std::string const s{R"(decltype abc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
	}
	{
		std::string const s{R"(doctypeabc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
	}
	{
		std::string const s{R"( doctype abc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
	}
	{
		std::string const s{R"(doctype abc )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::doctype_re));
	}
}

TEST(def_tag_re, Regex) {
	{
		std::string const s{R"(abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(#abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("#abc"s, m.str(1));
	}
	{
		std::string const s{R"(.abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ(".abc"s, m.str(1));
	}
	{
		std::string const s{R"(a)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("a"s, m.str(1));
	}
	{
		std::string const s{R"(Abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc"s, m.str(1));
	}
	{
		std::string const s{R"(abc-xyz)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc-xyz"s, m.str(1));
	}
	{
		std::string const s{R"(abc_xyz)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc_xyz"s, m.str(1));
	}
	{
		std::string const s{R"(a1)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("a1"s, m.str(1));
	}
	{
		std::string const s{R"(abc )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
	}
	{
		std::string const s{R"( abc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
	}
	{
		std::string const s{R"(1a)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
	}
	{
		std::string const s{R"(1)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::tag_re));
	}
}

TEST(def_attr_re, Regex) {
	{
		std::string const s{R"(abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
		EXPECT_EQ(""s, m.str(2));
	}
	{
		std::string const s{R"(abc,)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
		EXPECT_EQ(""s, m.str(2));
	}
	{
		std::string const s{R"(abc="xyz")"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
		EXPECT_EQ(R"(="xyz")"s, m.str(2));
	}
	{
		std::string const s{R"(abc="xyz",)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
		EXPECT_EQ(R"(="xyz")"s, m.str(2));
	}
	{
		std::string const s{R"(Abc="")"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc"s, m.str(1));
		EXPECT_EQ(R"(="")"s, m.str(2));
	}
	{
		std::string const s{R"(_="_",)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("_"s, m.str(1));
		EXPECT_EQ(R"(="_")"s, m.str(2));
	}
	{
		std::string const s{R"(-="-",)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("-"s, m.str(1));
		EXPECT_EQ(R"(="-")"s, m.str(2));
	}
	{
		std::string const s{R"(Abc1="Xyz9")"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc1"s, m.str(1));
		EXPECT_EQ(R"(="Xyz9")"s, m.str(2));
	}
	{
		std::string const s{R"(Abc1=Xyz9)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
	}
	{
		std::string const s{R"(Abc1-"Xyz9")"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
	}
	{
		std::string const s{R"(Abc1#Xyz9)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::attr_re));
	}
}

TEST(def_id_re, Regex) {
	{
		std::string const s{R"(#abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(#_)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("_"s, m.str(1));
	}
	{
		std::string const s{R"(#Abc9_-)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc9_-"s, m.str(1));
	}
	{
		std::string const s{R"( #a)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
	}
	{
		std::string const s{R"(#a )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
	}
	{
		std::string const s{R"(#9a)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
	}
	{
		std::string const s{R"(Abc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
	}
	{
		std::string const s{R"(Abc#Xyz)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::id_re));
	}
}

TEST(def_class_re, Regex) {
	{
		std::string const s{R"(.abc)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("abc"s, m.str(1));
	}
	{
		std::string const s{R"(.Aa_9-)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Aa_9-"s, m.str(1));
	}
	{
		std::string const s{R"(.-)"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("-"s, m.str(1));
	}
	{
		std::string const s{R"(abc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
	}
	{
		std::string const s{R"( .abc)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
	}
	{
		std::string const s{R"(.abc )"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
	}
	{
		std::string const s{R"(.9)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
	}
	{
		std::string const s{R"(abc.xyz)"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::class_re));
	}
}

TEST(def_nest_re, Regex) {
	// This implementation supports only tabs as indent.
	{
		std::string const s{"abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ(""s, m.str(1));
		EXPECT_EQ("abc"s, m.str(2));
	}
	{
		std::string const s{"\t\tabc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\t\t"s, m.str(1));
		EXPECT_EQ("abc"s, m.str(2));
	}
	{
		std::string const s{"\t\t123"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\t\t"s, m.str(1));
		EXPECT_EQ("123"s, m.str(2));
	}
	{
		std::string const s{"\t\t$$"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\t\t"s, m.str(1));
		EXPECT_EQ("$$"s, m.str(2));
	}
	{
		std::string const s{"\t\tabc\t\t"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\t\t"s, m.str(1));
		EXPECT_EQ("abc\t\t"s, m.str(2));
	}
	{
		std::string const s{"  abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ(""s, m.str(1));
		EXPECT_EQ("  abc"s, m.str(2));
	}
	{
		std::string const s{"\t abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\t"s, m.str(1));
		EXPECT_EQ(" abc"s, m.str(2));
	}
	{
		std::string const s{" \tabc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::nest_re));
		EXPECT_EQ(3, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ(""s, m.str(1));
		EXPECT_EQ(" \tabc"s, m.str(2));
	}
}

TEST(def_comment_re, Regex) {
	{
		std::string const s{"//-Abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::comment_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc"s, m.str(1));
	}
	{
		std::string const s{"//- Abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::comment_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc"s, m.str(1));
	}
	{
		std::string const s{"//-\tAbc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::comment_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc"s, m.str(1));
	}
	{
		std::string const s{"//-9"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::comment_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("9"s, m.str(1));
	}
	{
		std::string const s{"//Abc"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::comment_re));
	}
	{
		std::string const s{"/-Abc"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::comment_re));
	}
}

TEST(def_empty_re, Regex) {
	{
		std::string const s{""};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::empty_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"  "};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::empty_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"\t\t"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::empty_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{" \t"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::empty_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"a"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::empty_re));
	}
	{
		std::string const s{" a"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::empty_re));
	}
}

TEST(def_case_re, Regex) {
	{
		std::string const s{"case Abc123"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc123"s, m.str(1));
	}
	{
		std::string const s{"case\tAbc123"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc123"s, m.str(1));
	}
	{
		std::string const s{"Case Abc123"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
	}
	{
		std::string const s{"case"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
	}
	{
		std::string const s{"Abc"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
	}
	{
		std::string const s{"case 1"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
	}
	{
		std::string const s{"case Abc "};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
	}
	{
		std::string const s{" case Abc"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::case_re));
	}
}

TEST(def_when_re, Regex) {
	{
		std::string const s{"when \"Abc123\""};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\""s, m.str(1));
		EXPECT_EQ("Abc123"s, m.str(2));
		EXPECT_EQ("\""s, m.str(3));
	}
	{
		std::string const s{"when\t\"Abc123\""};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\""s, m.str(1));
		EXPECT_EQ("Abc123"s, m.str(2));
		EXPECT_EQ("\""s, m.str(3));
	}
	{
		std::string const s{"when\t'Abc123'"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("'"s, m.str(1));
		EXPECT_EQ("Abc123"s, m.str(2));
		EXPECT_EQ("'"s, m.str(3));
	}
	{
		std::string const s{"when\t\"Abc123'"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
		EXPECT_EQ(4, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("\""s, m.str(1));
		EXPECT_EQ("Abc123"s, m.str(2));
		EXPECT_EQ("'"s, m.str(3));
	}
	{
		std::string const s{"when Abc123"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
	}
	{
		std::string const s{"when"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
	}
	{
		std::string const s{"Abc"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
	}
	{
		std::string const s{"when \"1\""};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
	}
	{
		std::string const s{"when \"Abc\" "};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
	}
	{
		std::string const s{" when \"Abc\""};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::when_re));
	}
}

TEST(def_break_re, Regex) {
	{
		std::string const s{"- break"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::break_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"-\tbreak"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::break_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{" - break"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::break_re));
	}
	{
		std::string const s{"- break "};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::break_re));
	}
	{
		std::string const s{"- BREAK"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::break_re));
	}
}

TEST(def_if_re, Regex) {
	{
		std::string const s{"if Abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc", m.str(1));
	}
	{
		std::string const s{"if\tAbc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc", m.str(1));
	}
	{
		std::string const s{"if "};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("", m.str(1));
	}
	{
		std::string const s{"IF "};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
	}
	{
		std::string const s{"if"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
	}
	{
		std::string const s{"if"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
	}
	{
		std::string const s{" if"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::if_re));
	}
}

TEST(def_elif_re, Regex) {
	{
		std::string const s{"else if Abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc", m.str(1));
	}
	{
		std::string const s{"else if\tAbc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc", m.str(1));
	}
	{
		std::string const s{"else if "};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("", m.str(1));
	}
	{
		std::string const s{"else\tif Abc"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
		EXPECT_EQ(2, m.size());
		EXPECT_EQ(s, m.str());
		EXPECT_EQ("Abc", m.str(1));
	}
	{
		std::string const s{"elseif Abc"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
	}
	{
		std::string const s{"else IF "};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
	}
	{
		std::string const s{"else if"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
	}
	{
		std::string const s{"else if"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
	}
	{
		std::string const s{" else if"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::elif_re));
	}
}

TEST(def_else_re, Regex) {
	{
		std::string const s{"else"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"else\t"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"else "};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{"else \t"};
		std::smatch		  m;
		EXPECT_TRUE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
		EXPECT_EQ(1, m.size());
		EXPECT_EQ(s, m.str());
	}
	{
		std::string const s{" else"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
	}
	{
		std::string const s{"\telse "};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
	}
	{
		std::string const s{"ELSE"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
	}
	{
		std::string const s{"elif"};
		std::smatch		  m;
		EXPECT_FALSE(std::regex_match(s.cbegin(), s.cend(), m, xxx::pug::impl::def::else_re));
	}
}

#if 0
	static std::regex const	each_re{ R"(^each[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*in[ \t]*\[([^\]]*)\]$)" };
	static std::regex const	for_re{ R"(^-[ \t]+for[ \t]*\([ \t]*var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+);[ \t]*([ \tA-Za-z0-9_+*/%=<>!-]*);[ \t]*([ \tA-Za-z0-9_+*/%=<>!-]*)\)$)" };
	static std::regex const	var_re{ R"(^-[ \t]+var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)" };
	static std::regex const	const_re{ R"(^-[ \t]+const[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)" };
	static std::regex const	include_re{ R"(^include[ \t]+([^ ]+)$)" };
	static std::regex const	block_re{ R"(^block[ \t]+([^ ]+)$)" };
	static std::regex const	extends_re{ R"(^extends[ \t]+([^ ]+)$)" };
#endif

///	@}
