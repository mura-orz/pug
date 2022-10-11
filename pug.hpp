﻿///	@file
///	@brief		pug++  - Pug-THML translator
///	@author		Mura
///	@copyright	(c) 2022-, Mura.

#ifndef xxx_PUG_HPP_
#define xxx_PUG_HPP_

#include <algorithm>
#include <regex>
#include <ranges>
#include <exception>
#include <functional>
#include <numeric>
#include <vector>
#include <tuple>
#include <stack>
#include <set>
#include <utility>
#include <string>
#include <string_view>
#include <locale>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace xxx::pug {
namespace ex {

///	@brief	Syntax error exception.
class syntax_error : public std::runtime_error {
public:
	///	@brief	Constructor.
	syntax_error() noexcept : std::runtime_error("syntax_error") {}
	///	@brief	Constructor.
	///	@param[in]	message		Message to display.
	explicit	syntax_error(std::string const& message) noexcept : std::runtime_error(message) {}
	///	@brief	Constructor.
	///	@param[in]	message		Message to display.
	explicit	syntax_error(char const* const message) noexcept : std::runtime_error(message) {}
};

///	@brief	I/O error exception.
class io_error : public std::ios_base::failure {
public:
	///	@brief	Constructor.
	explicit	io_error(std::error_code const& code) : std::ios_base::failure("io_error", code) {}
	///	@brief	Constructor.
	///	@param[in]	path	Path of the file.
	explicit	io_error(std::filesystem::path const& path, std::error_code const& code) : std::ios_base::failure(path.string(), code) {}
};

}	// namespace ex
namespace impl {
namespace def {
	static std::set<std::string_view> const	void_tags{ "br", "hr", "img", "meta", "input", "link", "area", "base", "col", "embed", "param", "source", "track", "wbr" };
	static std::unordered_map<char, std::string> const	escapes{ { '<', "&lt;" }, { '>', "&gt;" }, { '&', "&amp;" }, { '"', "&quot;" }, { '\'',"&#39;" } };

	static std::string_view const	raw_html_sv{ "." };
	static std::string_view const	folding_sv{ "| " };
	static std::string_view const	comment_sv{ "//-" };
	static std::string_view const	raw_comment_sv{ "//" };
	static std::string_view const	var_sv{ "#{" };
	static std::string_view const	default_sv{ "default" };

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
	static std::regex const	elif_re{ R"(^else[v \t]+if[ \t]+(.*)$)" };
	static std::regex const	else_re{ R"(^else[ \t]+(.*)$)" };
	static std::regex const	each_re{ R"(^each[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*in[ \t]*\[([^\]]*)\]$)" };
	static std::regex const	for_re{ R"(^-[ \t]+for[ \t]*\(var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+);([ \tA-Za-z0-9_+*/%=<>!-]*);([ \tA-Za-z0-9_+*/%=<>!-]*)\)$)" };
	static std::regex const	var_re{ R"(^-[ \t]+var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)" };
	static std::regex const	const_re{ R"(^-[ \t]+const[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)" };
	static std::regex const	include_re{ R"(^include[ \t]+([^ ]+)$)" };
	static std::regex const	block_re{ R"(^block[ \t]+([^ ]+)$)" };
	static std::regex const	extends_re{ R"(^extends[ \t]+([^ ]+)$)" };
	// TODO: mixin
}	// namespace def

///	@brief	Reads the file as string.
///	@param[in]	path	Path of the file to read.
///	@return		Context of the file.
///	@throws		xxx::pug::ex::io_error		It throws the exception if an I/O error occurred.
inline std::string		load_file(std::filesystem::path const& path) {
	try {
		std::ifstream	ifs;
		ifs.exceptions(std::ios::badbit | std::ios::failbit);
		ifs.open(path, std::ios::in | std::ios::binary);

		auto const	size	= std::filesystem::file_size(path);
		if (size == 0u)		return std::string{};

		std::vector<char>	v;
		v.reserve(size + 1u);
		std::ranges::copy(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>(), std::back_inserter(v));
		v.push_back('\0');
		return &v[0];
	} catch (std::ios_base::failure const& e) {
		throw ex::io_error(path.string(), e.code());
	}
}

///	@brief	Splits string with new lines.
///	@param[in]	str		String to split.
///	@return		List of lines.
///	@warning	Keep original string available because it returns view of the string.
inline std::vector<std::string_view>	split_lines(std::string_view const str) {
	std::vector<std::string_view>	v;
	std::string_view		s	= str;
	for (auto pos = s.find('\n'); pos != std::string_view::npos; pos = s.find('\n')) {
		auto const	line	= s.substr(0, pos);
		s	= s.substr(pos + 1);
		if (line.empty())					continue;
		auto const	crlf	= line.back() == '\r';
		if (crlf && line.size() == 1u)		continue;
		v.push_back(crlf ? line.substr(0, line.size() - 1) : line);
	}
	if ( ! s.empty()) {
		v.push_back(s);
	}
	return v;
}

///	@brief	Nested level.
using	nest_t		= std::size_t;
///	@brief	Primitive line.
///		The first element is nested level.
///		The second element is view of string.
///	@warning	Keep original string available because it returns view of the string.
using	line_t		= std::pair<nest_t, std::string_view>;
///	@brief	Regular expression result using view of string.
///	@warning	Keep original string available because it returns view of the string.
using	svmatch		= std::match_results<std::string_view::const_iterator>;

///	@brief	Gets a captured string matched to regular expression.
///		It returns partial view of the @p s string,
///		which differs from 'm.str(n)' that generates a new string.
///	@warning	Keep original string available because it returns view of the string.
/// @param[in]	s		Target string of the regular expression.
/// @param[in]	m		Matching result of the regular expression.
/// @param[in]	n		Index of the captured result. Exceptionaly, zero means while of matching.
/// @return		A captured string or while of matching.
///	@warning	Keep original string available because it returns view of the string.
inline std::string_view		to_str(std::string_view s, svmatch const& m, std::size_t n) noexcept {
	return n < m.size() ? s.substr(m.position(n), m.length(n)) : std::string_view{};
}

///	@brief	Gets a nested line from a raw line string.
///	@param[in]	line	A raw line string.
///	@return		Nested line.
///	@warning	Keep original string available because it returns view of the string.
inline line_t	get_line_nest(std::string_view const line) {
	if (svmatch m; std::regex_match(line.cbegin(), line.cend(), m, def::nest_re)) {
		return { m.length(1), to_str(line, m, 2) };
	} else {
		return { 0u, line };
	}
}

///	@brief	Node of nested lines.
///	@warning	Keep original string available because it returns view of the string.
class line_node_t {
public:
	///	@brief	Gets the nested level of the node.
	///	@return		Nested level.
	nest_t										nest() const noexcept { return line_.first; }
	///	@brief	Gets the tabs to indent.
	///	@return		Tabs to indent.
	std::string									tabs() const { return std::string( nest(), '\t'); }
	///	@brief	Gets the line of the node.
	///	@return		Line of the node.
	auto const&									line() const noexcept {	return line_.second; }
	///	@brief	Push the @p line as a child of the @p parent.
	/// @param[in]	line	Line to push.
	/// @param[in]	parent	Parent of the @p line.
	///	@return		The pushed line.
	auto&										push_nest(line_t const& line, std::shared_ptr<line_node_t> parent)	{	children_.push_back(std::make_shared<line_node_t>(line, parent)); return children_.back(); }
	///	@copydoc	line_node_t::push_nest(line_t const&,std::shared_ptr<line_node_t>)
	auto&										push_nest(line_t&& line, std::shared_ptr<line_node_t> parent)		{	children_.emplace_back(std::make_shared<line_node_t>(line, parent)); return children_.back(); }
	///	@brief	Gets the children of the node.
	///	@return		the children of the node.
	std::vector<std::shared_ptr<line_node_t const>>		children() const noexcept { return std::vector<std::shared_ptr<line_node_t const>>(children_.cbegin(), children_.cend()); }
	///	@brief	Gets the parent of the node.
	///	@return		the parent of the node.
	///				It ruturns null if the node is the root of nodes.
	auto const									parent() const noexcept { return parent_.lock(); }
	///	@copydoc	line_node_t::push_nest()
	auto										parent() noexcept { return parent_.lock(); }
	///	@brief	Gets whether the node is folding or not.
	///	@return		It returns true if the node is folding; otherwise, it returns false.
	bool										folding() const noexcept { return folding_; }
	///	@brief	Sets whether the node is folding or not.
	///	@param[in]	on		Whether the node is folding or not.
	/// @arg	true		Node is folding.
	/// @arg	false		Node is not folding.
	void										set_folding(bool on) noexcept { folding_ = on; }
	///	@brief	Clears all the children.
	void										clear_children() noexcept { children_.clear(); }
	///	@brief	Gets the previous 'sister' line.
	///		The 'sister' is a child of the same parent.
	///	@return		The previous 'sister' line.
	std::shared_ptr<line_node_t const>			previous() const {
		auto const parent = parent_.lock();
		return ! parent || parent->children().empty() ? nullptr : parent->children().back();
	}
	///	@brief	Constructor.
	///	@param[in]	line	Line
	///	@param[in]	parent	Larent of this node.
	explicit	line_node_t(line_t const& line, std::shared_ptr<line_node_t> parent) noexcept : children_{}, parent_{ parent }, line_{ line }, folding_{} {}
	///	@brief	Constructor.
	line_node_t() noexcept : children_{}, parent_{}, line_{}, folding_{} {}
private:
	std::vector<std::shared_ptr<line_node_t>>	children_;	///< @brief	Children of the node.
	std::weak_ptr<line_node_t>					parent_;	///< @brief	Parent of the node.
	line_t										line_;		///< @brief	Line of the node.
	bool										folding_;	///< @brief	Whether folding or not.
};

///	@brief	Pops nested nodes to the @p nest or less level.
///		It returns an ancestor has the nested level less than or equal to the @p nest.
///	@param[in]	node	The current node.
///	@param[in]	nest	Nested level to pop.
///	@return		The poped node.
inline std::shared_ptr<line_node_t>	pop_nest(std::shared_ptr<line_node_t> node, nest_t nest) {
	return ! node || node->nest() <= nest ? node : pop_nest(node->parent(), nest);
}

///	@brief	Dumps hierarchy of nodes to the output stream.
///	@param[in,out]	os	Output stream.
///	@param[in]	node	Node to dump.
///	@param[in]	nest	Current nested level.
inline void dump_lines(std::ostream& os, std::shared_ptr<line_node_t const> node, size_t nest=0u) {
	if (!node)	return;

	os << std::string(nest, '\t') << node->line() << ":" << node->nest();
	if (auto const& ch = node->children(); ! ch.empty()) {
		os << "{" << std::endl;
		std::for_each(ch.cbegin(), ch.cend(), [&os, nest](auto const& a) { dump_lines(os, a, nest + 1); });
		os << std::string(nest, '\t') << "}" << std::endl;
	} else {
		os	<< "{}" << std::endl;
	}
}

///	@brief	Parses file contet as pug.
///	@param[in]	pug		File context formed as pug.
///	@param[in]	nest	Base of nested level. It is added to nested levels of parsed nodes.
///	@return		The root of parsed nodes.
///	@warning	Keep original string available because it returns view of the string.
inline std::shared_ptr<line_node_t>		parse_file(std::string_view pug, nest_t nest=0u) {
	auto		root		= std::make_shared<line_node_t>(line_t{ nest, std::string_view{} }, nullptr);
	auto const	raw_lines	= split_lines(pug);
	auto const	lines		= raw_lines | std::views::transform(&get_line_nest) | std::views::transform([nest](auto const& a) { return line_t{ a.first + nest, a.second }; });

	// Parses to tree of nested lines.
	(void)std::accumulate(lines.begin(), lines.end(), root, [nest](auto&& previous, auto const& a) {
		auto	parent	= previous->parent() ? previous->parent() : previous;
		if (a.second.starts_with(def::folding_sv)) {
			if (parent == previous) {
				throw ex::syntax_error();		// Folding line never be the top.
			}
			parent->set_folding(true);			// Parent is folding.
		}
		if (a.second.starts_with(def::comment_sv)) {
			line_t const	line{ previous->nest(), a.second };
			previous	= parent->push_nest(line, parent);			// Comment is always in the current level.
		} else if (a.second.starts_with(def::raw_comment_sv)) {
			// There is nothing to do.								// Drops pug comment.
		} else if (std::regex_match(a.second.cbegin(), a.second.cend(), def::empty_re)) {
			// There is nothing to do.		
			// Drops empty line.
		} else if (previous->nest() == a.first) {
			previous	= parent->push_nest(a, parent);				// This line is a sister of the previous line.
		} else if (parent->nest() < a.first) {
			if (a.first <= previous->nest()) {
				previous = parent->push_nest(a, parent);			// This line is a grandchild of the previous line.
			} else {
				previous = previous->push_nest(a, previous);		// This line is a child of the previous line.
			}
		} else {
			previous	= pop_nest(previous, a.first);
			if (previous->nest() < a.first) {
				previous	= previous->push_nest(a, previous);		// This line is a cousin of the previous line.
			} else {
				parent		= previous->parent() ? previous->parent() : previous;
				previous	= parent->push_nest(a, parent);			// This line is an aunt of the previous line.
			}
		}
		return previous;
	});
	return root;
}

///	@brief	Gets the nodes is whether folding or not.
///	@param[in]	line		A line as base point.
///	@param[in]	parent_only	Range of nodes to check.
/// @arg		true		This function returns true if parent is folding regardless of the @p line folding.
/// @arg		false		This function returns true if parent or the @p line is folding.
///	@return		Whether folding or not. See above.
inline bool is_folding(std::shared_ptr<line_node_t const> line, bool parent_only=false) {
	if (auto const parent = line->parent(); parent && parent->folding()) {
		return true;
	}
	return ! parent_only && line->folding();
}

///	@brief	Parsing context,
class context_t {
	///	@brief	Map of blocks.
	using blocks_t		= std::unordered_map<std::string_view, std::shared_ptr<line_node_t const>>;
	///	@brief	Map of blocks.
	using variables_t	= std::unordered_map<std::string_view, std::string>;
public:
	// ------------------------------
	// Block for expands

	///	@brief	Gets the block.
	///	@param[in]	tag		Name of the block.
	///	@return		The block.
	auto const&		block(std::string_view tag) const { return blocks_.at(tag); }
	///	@brief	Has the block or not.
	///	@param[in]	tag		Name of the block.
	///	@return		It returns true if the block exists; otherwise, it returns false.
	bool			has_block(std::string_view tag) const noexcept { return blocks_.contains(tag); }
	///	@brief	Sets the block.
	///	@param[in]	tag		Name of the block. Empty is invalid.
	///	@param[in]	block	Line of the block.
	void			set_block(std::string_view tag, std::shared_ptr<line_node_t const> block) {
		if (tag.empty())	throw std::invalid_argument(__func__);
		blocks_[tag]	= block;
	}

	// ------------------------------
	// Variables.

	///	@brief	Gets all the variables.
	///	@return		All the variables in current context.
	auto const&		variables() const noexcept{ return variables_; }
	///	@brief	Has the variable or not.
	///	@param[in]	tag		Name of the variable.
	///	@return		It returns true if the variable exists; otherwise, it returns false.
	bool			has_variable(std::string_view tag) const noexcept { return variables_.contains(tag); }
	///	@brief	Sets the variable.
	///	@param[in]	tag		Name of the variable. Empty is invalid.
	///	@param[in]	block	Line of the variable.
	void			set_variable(std::string_view tag, std::string_view variable) {
		if (tag.empty())	throw std::invalid_argument(__func__);
		variables_[tag]	= variable;
	}
	///	@brief	Constructor.
	context_t() noexcept : blocks_{}, variables_{} {}
private:
	blocks_t		blocks_;		///< @brief	Blocks.
	variables_t		variables_;		///< @brief	Variables.
};

///	@brief	Replaces all the variables (#{xxx}) in the @p str.
///	@param[in]	context	Context including variables.
///	@param[in]	str		Input string.
///	@return	Replaced string.
inline std::string	replace_variables(context_t const& context, std::string_view str) {
	if (str.find(def::var_sv) == std::string_view::npos)	return std::string{ str };

	// It is not effective.because this implementation will scan while of the string by variables-count times.
	auto	s	= std::string{str};
	std::ranges::for_each(context.variables(), [&s](auto const& a) {
		s	= std::regex_replace(s, std::regex{ R"(#\{)" + std::string{a.first} + R"(\})" }, a.second);
	});
	return s;
}

std::tuple<std::string,context_t>	parse_line(context_t const&, std::shared_ptr<line_node_t const>, std::filesystem::path const&);

///	@brief	Parses a element from the @p line.
///		This implementation supports only the following order:
///			tag#id.class.class(attr,attr)
///		This implementation supports only single line element:
///		Only element can be nested by ': '.
///	@param[in]	s		Pug source.
///	@param[in]	line	Line of the pug.
///	@param[in]	context	Parsing context.
///	@return		It returns the followings:
///		-#	Remaingin string of the line.
///		-#	Output stream.
///		-#	Tag name to close later.
///	@warning	Keep original string available because it returns view of the string.
inline std::tuple<std::string_view, std::string, std::string_view>
		parse_element(std::string_view s, std::shared_ptr<line_node_t const> line) {
	if ( ! line)	throw std::invalid_argument(__func__);

	if (s.empty() && line->parent()) {
		return { std::string_view{}, "\n", std::string_view{} };
	} else if (s == def::raw_html_sv) {
		auto const&	children	= line->children();
		return { std::string_view{}, std::accumulate(std::ranges::cbegin(children), std::ranges::cend(children), std::ostringstream{}, [](auto&&os, auto const& a) {
				os	<< a->tabs() << a->line() << '\n';
				return std::move(os);
			}).str(), std::string_view{} };
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, def::doctype_re)) {
		// This implementation allows it is nested by the ': ' sequense.
		std::ostringstream	os;
		os	<< "<!DOCTYPE " << to_str(s, m, 1) << ">" << '\n';
		return { std::string_view{}, os.str(), std::string_view{} };
	} else if (std::regex_search(s.cbegin(), s.cend(), m, def::tag_re)) {
		// Tag
		auto tag				= to_str(s, m, 1);
		auto const	void_tag	= def::void_tags.contains(tag);
		std::ostringstream	os;
		if ( ! is_folding(line, true)) {
			os	<< line->tabs();
		}
		os	<< "<";
		if (tag.starts_with('.') || tag.starts_with('#')) {
			using namespace std::string_view_literals;
			tag	= "div"sv;
			os	<< tag;		// The 'div' tag can be omitted.
		} else {
			os	<< tag;
			s	= s.substr(tag.length());
		}
		bool	escape = false;
		if (s.empty() || s.starts_with(": ")) {
			s	= s.empty() ? std::string_view{} : s.substr(2);
			os	<< (void_tag ? " />" : ">");
			os	<< (is_folding(line) ? "" : "\n");
			return {s, os.str(), void_tag ? std::string_view{} : tag};
		} else if (s.starts_with("!=")) {
			s	= s.substr(2);
		} else if (s.starts_with('=')) {
			bool	escape = true;
			s	= s.substr(1);
		}

		// ID
		if (std::regex_search(s.cbegin(), s.cend(), m, def::id_re)) {
			os	<< R"( id=")" << to_str(s, m, 1) << R"(")";
			s	= s.substr(m.length());
		}
		// Class
		if (s.starts_with('.')) {
			os	<< R"( class=")";
			bool	first	= true;
			for (; std::regex_search(s.cbegin(), s.cend(), m, def::class_re); s = s.substr(m.length())) {
				if ( ! first) {
					os	<< ' ';
				}
				os	<< to_str(s, m, 1);
				first	= false;
			}
			os	<< R"(")";
		}
		// Attributes
		if (s.starts_with('(')) {
			s	= s.substr(1);
			for ( ; std::regex_search(s.cbegin(), s.cend(), m, def::attr_re); s = s.substr(m.length())) {
				os	<< " " << to_str(s, m, 1);
				if (auto const parameter = to_str(s, m, 2); 1u < parameter.size()) {
					if (parameter.at(1) != parameter.back())	throw ex::syntax_error();
					os	<< R"(=")" << parameter.substr(2, parameter.size()-3) << R"(")";
				}
			}
			if ( ! s.starts_with(')')) {
				throw ex::syntax_error();
			}
			os	<< " ";
			s	= s.substr(1);
		}
		os	<< (void_tag ? " />" : ">");

		if (s.starts_with(": ")) {
			return {s.substr(2), os.str(), void_tag ? std::string_view{} : tag};
		} else {
			auto const	c	= s.starts_with(' ') ? s.substr(1) : s;
			if (escape) {
				auto const 	escaped	= c | std::views::transform([](auto const& a) {
					return def::escapes.contains(a) ? def::escapes.at(a) : std::string(1u, a); });
				std::ranges::for_each(escaped, [&os](auto const& a) { os << a; });
			} else {
				os	<< c;
			}
			if ( ! is_folding(line)) {
				os	<< '\n';
			}
			return {std::string_view{}, os.str(), void_tag ? std::string_view{} : tag};
		}
	} else {
		throw ex::syntax_error();
	}
}

///	@brief	Parses children of the @p line.
///	@param[in]	context		Parsing context. It is not contant reference but copied.
///	@param[in]	children	Lines of children.
///	@param[in]	path		Path of the pug.
/// @return		It returns the following:
///		-#	Generated HTLM string
///		-#	Context.
inline	std::tuple<std::string, context_t>	parse_children(context_t context, std::vector<std::shared_ptr<line_node_t const>> const& children, std::filesystem::path const& path) {
	return { std::accumulate(std::ranges::cbegin(children), std::ranges::cend(children), std::ostringstream{}, [&context, &path](auto&& os, auto const& a) {
			auto const[s, c]	= parse_line(context, a, path);
			context = c;
			os	<< s;
			return std::move(os);
		}).str(), context };
}

///	@brief	Parses a line of pug.
///		The following limitations will cause syntax error.
///		-	This implementation supports only tabs as indent.
///		-	This implementation supports only the following order of emenet:
///				-	tag#id.class.class(attr,attr)
///		-	This implementation supports only single line element:
///		-	This implementation does not support inline 'style'.
///		-	This implementation supports the 'extends' as forward reference only.
///				The implementation does not support default of the 'extends'.
///		-	This implementation supports boolean attribute as only true.
///			The following expressions are not supported:
///				-	attr=true
///				-	attr=false
///		-	This implementation supports only either the '#id' style or the '(id='..')' style in an element.
///		-	This implementation supports only either the '.class' style or the '(class='..')' style in an element.
///	@param[in]	context	Parsing context.
///	@param[in]	line	Line of the pug.
///	@param[in]	path	Path of the pug.
/// @return		It returns the following:
///		-#	Generated HTLM string
///		-#	Context.
///	@todo	The 'mixin' directive.
inline	std::tuple<std::string,context_t>	parse_line(context_t const& context, std::shared_ptr<line_node_t const> line, std::filesystem::path const& path) {
	if ( ! line)		return { std::string{}, context };

	if (auto const& s = line->line(); s.starts_with(def::folding_sv)) {
		return { replace_variables(context, s.substr(2)), context};
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, def::comment_re)) {
		auto const	out = line->tabs() + "<!-- " + replace_variables(context, to_str(s, m, 1)) + " -->" + '\n';
		return { out, context };
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, def::include_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const	pug		= std::filesystem::path{ path }.replace_filename(to_str(s, m, 1));
		auto const	source	= load_file(pug);		// This string will be invalidated at the end of this function.
		auto const	sub		= parse_file(source, line->nest());
		return parse_line(context, sub, path);	// Thus, output of the included pug must be finised here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::extends_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const	pug		= std::filesystem::path{ path }.replace_filename(to_str(s, m, 1));
		auto const	source	= load_file(pug);		// This string will be invalidated at the end of this function.
		auto const	sub		= parse_file(source, line->nest());
		return parse_line(context, sub, path);	// Thus, output of the included pug must be finised here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::block_re)) {
		if (auto const& tag = to_str(s, m, 1); context.has_block(tag)) {
			// TODO: increases indent.
			return parse_children(context, context.block(tag)->children(), path);
		} else {
			context_t	ctx			= context;
			ctx.set_block(tag, line);
			return { std::string{}, ctx };
		}
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::if_re)) {
		// Is statement
		{
			auto const&	expression	= to_str(s, m, 1);
			auto const	condition	= true;	// TODO:
			if (condition) {
				// Ignores following elses.
				return parse_children(context, line->children(), path);
			}
		}

		// Collects else-if and elses.
		std::vector<std::pair<std::string_view, std::shared_ptr<line_node_t const>>>	elifs;
		std::shared_ptr<line_node_t const>												else_;
		{
			auto const	parent	= line->parent();
			if ( ! parent)		throw ex::syntax_error();
			auto const&		children	= line->children();
			for (auto itr = std::ranges::find(children, line), end = std::ranges::cend(children); itr != end; ++itr) {
				auto const&		line	= (*itr)->line();
				if (svmatch m; std::regex_match(line.cbegin(), line.cend(), m, def::elif_re)) {
					if (else_)	throw ex::syntax_error();		// The 'else' appears at only the end of the sequence.
					elifs.push_back({to_str(line, m, 1), *itr});
				} else if (std::regex_match(line.cbegin(), line.cend(), m, def::else_re)) {
					if (else_)	throw ex::syntax_error();		// The 'else' appears only once.
					else_	= *itr;
				} else {
					break;
				}
			}
		}
		for (auto const& elif : elifs) {
			auto const&	expression	= elif.first;
			auto const	condition	= true;	// TODO:
			if (condition) {
				return parse_children(context, line->children(), path);
			}
		}
		if (else_) {
			return parse_children(context, else_->children(), path);
		} else {
			return { std::string{}, context };
		}
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::elif_re)) {
		// There is nothing to do because it is handled at if directive. 
		return { std::string{}, context };
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::else_re)) {
		// There is nothing to do because it is handled at if directive. 
		return { std::string{}, context };
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::case_re)) {
		auto const	var			= to_str(s, m, 0);
		// TODO:
		using	cases_t = std::vector<std::pair<std::string_view, std::shared_ptr<line_node_t const>>>;;
		auto const	contains = [](cases_t const& cases, std::string_view tag) {
			return std::ranges::find_if(cases, [tag](auto const& a) { return a.first == tag; }) != std::ranges::cend(cases);
		};
		auto const&	children	= line->children();
		auto const	cases		= std::accumulate(std::ranges::cbegin(children), std::ranges::cend(children), cases_t{}, [contains](auto&& out, auto const& a) {
				if (auto const& ss = a->line(); ss == def::default_sv) {
					if (contains(out, std::string_view{}))		throw ex::syntax_error();
					out.push_back({std::string_view{}, a});
				} else if (svmatch mm; std::regex_match(ss.cbegin(), ss.cend(), mm, def::when_re)) {
					if (to_str(ss, mm, 0) != to_str(ss, mm, 2)) {
						throw ex::syntax_error();
					}
					auto const	label	= to_str(a->line(), mm, 1);
					if (contains(out, label))		throw ex::syntax_error();
					out.push_back({ label, a });
				} else {
					throw ex::syntax_error();
				}
				return std::move(out);
			});
		auto const	parse_cases = [](context_t context, cases_t const& cases, std::string_view label, std::filesystem::path const& path) -> std::tuple<std::string, context_t> {
			for (auto itr = std::ranges::find_if(cases, [label](auto const& a) { return a.first == label; }); itr != std::ranges::cend(cases); ++itr) {
				auto const& children	= itr->second->children();
				if (children.empty())	continue;
				auto const&	line		= children.front()->line();
				if (svmatch mm; std::regex_match(line.cbegin(), line.cend(), mm, def::break_re)) {
					break;
				}
				return parse_children(context, children, path);
			}
			return { std::string{}, context };
		};
		if (contains(cases, var)) {
			return parse_cases(context, cases, var, path);
		} else if (contains(cases, std::string_view{})) {
			return parse_cases(context, cases, std::string_view{}, path);
		} else {
			return { std::string{}, context };
		}
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::for_re)) {
		auto const	var			= to_str(s, m, 0);
		auto const	initial		= to_str(s, m, 1);
		auto const	condition	= to_str(s, m, 2);
		auto const	advance		= to_str(s, m, 3);
		// TODO:
		return parse_children(context, line->children(), path);
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::each_re)) {
		// TODO:
		auto const& name	= to_str(s, m, 1);
		std::istringstream	iss(std::string{ to_str(s, m, 2) });
		std::vector<std::string>	items;
		for (std::string item; std::getline(iss, item, ','); ) {
			auto const	begin	= item.find_first_not_of(" \t");
			if (begin == std::string::npos)	throw ex::syntax_error();
			auto const	end		= item.find_last_not_of(" \t,");
			items.emplace_back(item.substr(begin, end - begin));
		}

		if (items.empty()) {
			return { std::string{}, context };
		}
		context_t	ctx		= context;
		auto const	outs	= items | std::views::transform([&ctx, name, &line, &path](auto const& a) {
			ctx.set_variable(name, a);
			return parse_children(ctx, line->children(), path);
		});
		return { std::accumulate(std::ranges::cbegin(outs), std::ranges::cend(outs), std::ostringstream{}, [](auto&& os, auto const& a) {
				os	<< std::get<0>(a);
				return std::move(os);
			}).str(), std::get<1>(outs.back()) };
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::var_re) || std::regex_match(s.cbegin(), s.cend(), m, def::const_re)) {
		auto const& name	= to_str(s, m, 1);
		auto const& value	= to_str(s, m, 2);
		context_t	ctx		= context;
		ctx.set_variable(name, (value.starts_with('"') || value.starts_with("'")) ? value.substr(1, value.size() - 2) : value);
		return { std::string{}, ctx };
	} else {
		std::ostringstream				oss;
		std::stack<std::string_view>	tags;
		for (auto result = std::make_tuple(s, std::string{}, std::string_view{}); ! std::get<0>(result).empty(); ) {
			result	= parse_element(std::get<0>(result), line);
			if (auto const& tag = std::get<2>(result); ! tag.empty()) {
				tags.push(tag);
			}
			oss	<< replace_variables(context, std::get<1>(result));
		}

		auto const[ss,ctx]	= parse_children(context, line->children(), path);
		oss << ss;

		for ( ; !tags.empty(); tags.pop()) {
			if ( ! is_folding(line)) {
				oss	<< line->tabs();
			}
			oss	<< "</" << tags.top() << ">";
			if ( ! is_folding(line)) {
				oss	<< '\n';
			}
		}
		if (line->folding()) {
			oss	<< '\n';
		}
		return { oss.str(), ctx };
	}
}

}	// namespace impl

///	@brief	Translates a pug string to HTML string.
///	@param[in]	pug		Source string formatted in pug.
///	@param[in]	path	Path of working directory.
///	@return		String of generated HTML.
inline std::string		pug_string(std::string_view pug, std::filesystem::path const& path = "./") {
	auto const	root = impl::parse_file(pug);

	// TODO:	impl::dump_lines(std::clog, root);
	auto const [out, ctx] = impl::parse_line(impl::context_t{}, root, path);	// TODO:
	return out;
}

///	@brief	Translates a pug file to HTML string.
///	@param[in]	path	Path of the pug file.
///	@return		String of generated HTML.
inline std::string		pug_file(std::filesystem::path const& path) {
	auto const	source		= impl::load_file(path);
	return pug_string(source, path);
}

}	// namespace xxx::pug

#endif	// xxx_PUG_HPP_
