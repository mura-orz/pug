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
	EXPECT_EQ("&lt;",	xxx::pug::impl::def::escapes.at('<'));
	EXPECT_EQ("&gt;",	xxx::pug::impl::def::escapes.at('>'));
	EXPECT_EQ("&amp;",	xxx::pug::impl::def::escapes.at('&'));
	EXPECT_EQ("&quot;",	xxx::pug::impl::def::escapes.at('"'));
	EXPECT_EQ("&#39;",	xxx::pug::impl::def::escapes.at('\''));
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

#if 0
	static std::regex const	binary_op_re{ R"(^([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)$)"};
	static std::regex const	string_re{ R"(^(['"])([^'"])(['"])$)" };	// TODO: escape sequence is unsupported.
	static std::regex const	integer_re{ R"(^(-?[0-9]+)$)" };

	static std::regex const	doctype_re{ R"(^[dD][oO][cC][tT][yY][pP][eE] ([A-Za-z0-9_]+)$)" };
	static std::regex const	tag_re{ R"(^([#.]?[A-Za-z_-][A-Za-z0-9_-]*))" };
	static std::regex const	attr_re{ R"(^([A-Za-z_-][A-Za-z0-9_-]*)(=['"][^'"]*['"])?[ ,]*)" };
	static std::regex const	id_re{ R"(^#([A-Za-z_-][A-Za-z0-9_-]*))" };
	static std::regex const	class_re{ R"(^\.([A-Za-z_-][A-Za-z0-9_-]*))" };

	static std::regex const	nest_re{ R"(^([\t]*)(.*)$)" };	///	@brief	This implementation supports only tabs as indent.
	static std::regex const	comment_re{ R"(^//-[ \t]?(.*)$)" };
	static std::regex const	empty_re{ R"(^[ \t]*$)" };
	static std::regex const	case_re{ R"(^case[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)$)" };
	static std::regex const	when_re{ R"(^when[ \t]+(["'])([A-Za-z_-][A-Za-z0-9_-]*)(["'])$)" };
	static std::regex const	break_re{ R"(^-[ \t]+break$)" };
	static std::regex const	if_re{ R"(^if[ \t]+(.*)$)" };
	static std::regex const	elif_re{ R"(^else[ \t]+if[ \t]+(.*)$)" };
	static std::regex const	else_re{ R"(^else[ \t]*$)" };
	static std::regex const	each_re{ R"(^each[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*in[ \t]*\[([^\]]*)\]$)" };
	static std::regex const	for_re{ R"(^-[ \t]+for[ \t]*\([ \t]*var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+);[ \t]*([ \tA-Za-z0-9_+*/%=<>!-]*);[ \t]*([ \tA-Za-z0-9_+*/%=<>!-]*)\)$)" };
	static std::regex const	var_re{ R"(^-[ \t]+var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)" };
	static std::regex const	const_re{ R"(^-[ \t]+const[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)" };
	static std::regex const	include_re{ R"(^include[ \t]+([^ ]+)$)" };
	static std::regex const	block_re{ R"(^block[ \t]+([^ ]+)$)" };
	static std::regex const	extends_re{ R"(^extends[ \t]+([^ ]+)$)" };
#endif

///	@}
