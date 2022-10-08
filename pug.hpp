///	@file
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
	v.reserve(str.size());
	std::ranges::for_each(str | std::views::split(std::views::single('\n')) | std::views::common, [&v](auto const& line) {
			auto const	crlf	= ( ! line.empty() && line.back() == '\r');
			auto		end		= line.end();
			v.emplace_back(std::string_view{ line.begin(), crlf ? --end : end});
		});
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

///	@brief	Regular expression of nested line.
///		This implementation supports only tabs as indent.
std::regex const	nest_re{ "^([\t]*)(.*)$" };

///	@brief	Gets a nested line from a raw line string.
///	@param[in]	line	A raw line string.
///	@return		Nested line.
///	@warning	Keep original string available because it returns view of the string.
inline line_t	get_line_nest(std::string_view const line) {
	if (svmatch m; std::regex_match(line.cbegin(), line.cend(), m, nest_re)) {
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
	auto const&									children() const noexcept { return children_; }
	///	@brief	Gets the parent of the node.
	///	@return		the parent of the node.
	///				It ruturns null if the node is the root of nodes.
	auto const									parent() const noexcept { return parent_.lock(); }
	///	@copydoc	line_node_t::push_nest()
	auto										parent() noexcept { return parent_.lock(); }
	///	@brief	Gets whether the node is holding or not.
	///	@return		It returns true if the node is holding; otherwise, it returns false.
	bool										holding() const noexcept { return holding_; }
	///	@brief	Sets whether the node is holding or not.
	///	@param[in]	on		Whether the node is holding or not.
	/// @arg	true		Node is holding.
	/// @arg	false		Node is not holding.
	void										set_holding(bool on) noexcept { holding_ = on; }
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
	explicit	line_node_t(line_t const& line, std::shared_ptr<line_node_t> parent) noexcept : children_{}, parent_{ parent }, line_{ line }, holding_{} {}
	///	@brief	Constructor.
	line_node_t() noexcept : children_{}, parent_{}, line_{}, holding_{} {}
private:
	std::vector<std::shared_ptr<line_node_t>>	children_;	///< @brief	Children of the node.
	std::weak_ptr<line_node_t>					parent_;	///< @brief	Parent of the node.
	line_t										line_;		///< @brief	Line of the node.
	bool										holding_;	///< @brief	Whether holding or not.
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
	auto		root	= std::make_shared<line_node_t>(line_t{ nest, std::string_view{} }, nullptr);
	auto const	lines	= split_lines(pug) | std::views::transform(&get_line_nest) | std::views::transform([nest](auto const& a) { return line_t{ a.first + nest, a.second }; });

	// Parses to tree of nested lines.
	std::regex const	empty_re{ R"(^[ \t]*$)" };
	(void)std::accumulate(lines.begin(), lines.end(), root, [empty_re, nest](auto&& previous, auto const& a) {
		auto	parent = previous->parent() ? previous->parent() : previous;
		if (a.second.starts_with("//-")) {
			line_t const	line{ previous->nest(), a.second };
			previous	= parent->push_nest(line, parent);			// Comment is always in the current level.
		} else if (a.second.starts_with("//")) {
			// There is nothing to do.								// Drops pug comment.
		} else if (std::regex_match(a.second.cbegin(), a.second.cend(), empty_re)) {
			// There is nothing to do.		
			// Drops empty line.
		} else if (a.second.starts_with("| ")) {
			if (parent->holding()) {
				if (previous->nest() != a.first) {
					throw ex::syntax_error();
				}
				previous	= parent->push_nest(a, parent);			// Following holding lines are sisters.
			} else {
				if (a.first < previous->nest()) {
					throw ex::syntax_error();
				}
				previous->set_holding(true);
				previous	= previous->push_nest(a, previous);		// The first holding line is a child of previous line.
			}
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

///	@brief	Gets the nodes is whether holding or not.
///	@param[in]	line		A line as base point.
///	@param[in]	parent_only	Range of nodes to check.
/// @arg		true		This function returns true if parent is holding regardless of the @p line holding.
/// @arg		false		This function returns true if parent or the @p line is holding.
///	@return		Whether holding or not. See above.
inline bool is_holding(std::shared_ptr<line_node_t> line, bool parent_only=false) {
	if (auto const parent = line->parent(); parent && parent->holding()) {
		return true;
	}
	return ! parent_only && line->holding();
}

///	@brief	Map of blocks.
using blocks_t = std::unordered_map<std::string_view, std::shared_ptr<line_node_t>>;

///	@brief	Parsing context,
class context_t {
public:
	auto const& block(std::string_view tag) const { return blocks_.at(tag); }
	bool			has_block(std::string_view tag) const noexcept { return blocks_.contains(tag); }
	void			set_block(std::string_view tag, std::shared_ptr<line_node_t> block) {
		if (tag.empty())	throw std::invalid_argument(__func__);
		blocks_[tag] = block;
	}
	context_t() noexcept : blocks_{} {}
private:
	blocks_t	blocks_;	///< @brief	Blocks.
};

context_t	parse_line(std::ostream&, context_t const&, std::shared_ptr<line_node_t>, std::filesystem::path const&);

///	@brief	Parses a element from the @p line.
///		This implementation supports only the following order:
///			tag#id.class.class(attr,attr)
///		This implementation supports only single line element:
///	@param[in]	s		Pug.
///	@param[in,out]	os	Output stream.
///	@param[in]	context	Parsing context.
///	@param[in]	line	Line of the pug.
///	@param[in]	path	Path of the pug.
///	@return		It returns the followings:
///		-#	Remaingin string of the line.
///		-#	Tag name to close later, or block name.
///		-#	The root node of block.
///	@todo	The 'var' directive.
///	@todo	The 'for' directive.
///	@todo	The 'each' directive.
///	@todo	The 'switch' directive.
///	@todo	The 'if'-'else' directives.
///	@todo	The 'mixin' directive.
///	@todo	The '=' directive.
///	@warning	Keep original string available because it returns view of the string.
inline std::tuple<std::string_view,std::string_view, std::shared_ptr<line_node_t>, context_t>	parse_element(std::string_view s, std::ostream& os, context_t const& context, std::shared_ptr<line_node_t> line, std::filesystem::path const& path) {
	if ( ! line)	throw std::invalid_argument(__func__);
	std::set<std::string_view> const	void_tags{ "br", "hr", "img", "meta", "input", "link", "area", "base", "col", "embed", "param", "source", "track", "wbr" };
	std::string_view const	raw_html{ "." };
	std::regex const	include_re{ R"(^include[ \t]+([^ ]+)$)" };
	std::regex const	block_re{ R"(^block[ \t]+([^ ]+)$)" };
	std::regex const	extends_re{ R"(^extends[ \t]+([^ ]+)$)" };
	std::regex const	doctype_re{ R"(^[dD][oO][cC][tT][yY][pP][eE] ([A-Za-z0-9_]+)$)" };
	std::regex const	switch_re{ R"(^case[ \t]+([A-Za-z_][A-Za-z0-9_]*)$)" };
	std::regex const	if_re{ R"(^if[ \t]+(.*)$)" };
	std::regex const	elif_re{ R"(^else if[ \t]+(.*)$)" };
	std::regex const	else_re{ R"(^else[ \t]+(.*)$)" };
	std::regex const	each_re{ R"(^each[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*in[ \t]*\[([^\]]*)\]$)" };
	std::regex const	for_re{ R"(^-[ \t]+for[ \t]*\(var[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*([^;]+);[ \tA-Za-z0-9_+*/%=<>!-]*;\)$)" };
	std::regex const	var_re{ R"(^-[ \t]+var[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*([^;]+)$)" };
	std::regex const	tag_re{ R"(^([#.]?[A-Za-z0-9_-]+))"};
	std::regex const	attr_re{ R"(^([A-Za-z0-9_-]+)(=['"][^'"]*['"])?[ ,]*)" };
	std::regex const	id_re{ R"(^#([A-Za-z0-9_-]+))" };
	std::regex const	class_re{ R"(^\.([A-Za-z0-9_-]+))" };

	std::remove_const<std::remove_reference<decltype(context)>::type>::type	ctx = context;

	if (s.empty() && line->parent()) {
		os	<< '\n';
	} else if (s.starts_with("| ")) {
		os	<< s.substr(2);
		return { std::string_view{}, std::string_view{}, nullptr, ctx };
	} else if (auto const& ch = line->children(); s == raw_html) {
		std::for_each(ch.cbegin(), ch.cend(), [&os](auto const& a) { os << a->tabs() << a->line() << '\n'; });
		line->clear_children();	// Outputs raw HTML here and removes them from the tree.
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, include_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const	pug		= std::filesystem::path{ path }.replace_filename(to_str(s, m, 1));
		auto const	source	= load_file(pug);	// This string will be invalidated at the end of this function.
		auto const	sub		= parse_file(source, line->nest());
		ctx	= parse_line(os, ctx, sub, path);		// Thus, output of the included pug must be finised here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, extends_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const	pug		= std::filesystem::path{ path }.replace_filename(to_str(s, m, 1));
		auto const	source	= load_file(pug);	// This string will be invalidated at the end of this function.
		auto const	sub		= parse_file(source, line->nest());
		ctx	= parse_line(os, ctx, sub, path);		// Thus, output of the included pug must be finised here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, block_re)) {
		return { std::string_view{}, to_str(s, m, 1), line, ctx };	// TODO:
	} else if (std::regex_match(s.cbegin(), s.cend(), m, doctype_re)) {
		os << "<!DOCTYPE " << to_str(s, m, 1) << ">" << '\n';
	} else if (std::regex_match(s.cbegin(), s.cend(), m, if_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: if " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, elif_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: else-if " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, else_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: else " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, switch_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: switch-case " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, for_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: for " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, each_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: each " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, var_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: var " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_search(s.cbegin(), s.cend(), m, tag_re)) {
		// Tag
		auto tag		= to_str(s, m, 1);
		auto const	void_tag = void_tags.contains(tag);
		if ( ! is_holding(line, true)) {
			os << line->tabs();
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
		if (s.empty() || s.starts_with(": ")) {
			s	= s.empty() ? std::string_view{} : s.substr(2);
			os	<< (void_tag ? " />" : ">");
			os	<< (is_holding(line) ? "" : "\n");
			return { s, void_tag ? std::string_view{} : tag, nullptr, ctx };
		}

		// ID
		if (std::regex_search(s.cbegin(), s.cend(), m, id_re)) {
			os	<< R"( id=")" << to_str(s, m, 1) << R"(")";
			s	= s.substr(m.length());
		}
		// Class
		if (s.starts_with('.')) {
			os	<< R"( class=")";
			bool	first	= true;
			for (; std::regex_search(s.cbegin(), s.cend(), m, class_re); s = s.substr(m.length())) {
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
			for ( ; std::regex_search(s.cbegin(), s.cend(), m, attr_re); s = s.substr(m.length())) {
				os	<< " " << to_str(s, m, 1);
				if (auto const parameter = to_str(s, m, 2); 1u < parameter.size()) {
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
			return { s.substr(2), void_tag ? std::string_view{} : tag, nullptr, ctx };
		} else {
			bool const	escaped = s.starts_with('=');
			os	<< (s.starts_with(' ') ? s.substr(1) : s);
			if ( ! is_holding(line)) {
				os	<< '\n';
			}
			return { std::string_view{}, void_tag ? std::string_view{} : tag, nullptr, ctx };
		}
	} else {
		throw ex::syntax_error();
	}
	return { std::string_view{}, std::string_view{}, nullptr, ctx };
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
///	@param[in.out]	os	Output stream.
///	@param[in]	context	Parsing context.
///	@param[in]	line	Line of the pug.
///	@param[in]	path	Path of the pug.
inline	context_t	parse_line(std::ostream& os, context_t const& context, std::shared_ptr<line_node_t> line, std::filesystem::path const& path) {
	if ( ! line)		return context;
	std::regex const	comment_re{ R"(^//-[ \t]?(.*)$)" };
	std::regex const	empty_re{ R"(^[ \t]*$)" };
	auto const			str		= line->line();

	std::remove_const<std::remove_reference<decltype(context)>::type>::type	ctx		= context;

	if (svmatch m; std::regex_match(str.cbegin(), str.cend(), m, comment_re)) {
		os	<< line->tabs() << "<!-- " << to_str(str, m, 1) << " -->" << '\n';
	} else {
		std::stack<std::string_view>	tags;
		for (auto result = std::make_tuple(str, std::string_view{}, std::shared_ptr<line_node_t>{}, context_t{}); !std::get<0>(result).empty();) {
			result	= parse_element(std::get<0>(result), os, context, line, path);
			if (auto const& block = std::get<2>(result); block) {
				if (auto const& tag = std::get<1>(result); context.has_block(tag)) {
					// TODO: increases indent.
					std::ranges::for_each(context.block(tag)->children(), [&os, &ctx, &path](auto const& a) { ctx = parse_line(os, ctx, a, path); });
				} else {
					ctx.set_block(tag, block);
					return ctx;
				}
			} else if (auto const& tag = std::get<1>(result); ! tag.empty()) {
				tags.push(tag);
			}
		}
		if ( ! line->children().empty()) {
			std::ranges::for_each(line->children(), [&os, &ctx, &path](auto const& a) { ctx	= parse_line(os, ctx, a, path); });
		}
		for ( ; !tags.empty(); tags.pop()) {
			if ( ! is_holding(line)) {
				os	<< line->tabs();
			}
			os	<< "</" << tags.top() << ">";
			if ( ! is_holding(line)) {
				os	<< '\n';
			}
		}
		if (line->holding()) {
			os	<< '\n';
		}
	}
	return ctx;
}

}	// namespace impl

///	@brief	Translates a pug file to HTML string.
///	@param[in]	path	Path of the pug file.
///	@return		String of generated HTML.
inline std::string		pug(std::filesystem::path const& path) {
	auto const	source		= impl::load_file(path);
	auto const	root		= impl::parse_file(source);

	// TODO:	impl::dump_lines(std::clog, root);
	{
		(void)impl::parse_line(std::clog, impl::context_t{}, root, path);	// TODO:
	}
	std::ostringstream	oss;
	(void)impl::parse_line(oss, impl::context_t{}, root, path);
	return oss.str();
}

}	// namespace xxx::pug

#endif	// xxx_PUG_HPP_
