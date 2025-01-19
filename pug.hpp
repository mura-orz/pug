///	@file
///	@brief		pug++  - Pug-HTML translator
///	@author		Mura
///	@copyright	(c) 2022-, Mura.

#ifndef xxx_PUG_HPP_
#define xxx_PUG_HPP_

#include <string_view>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <numeric>
#include <ranges>
#include <regex>
#include <set>
#include <stack>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

namespace xxx::pug {
namespace ex {

///	@brief	Syntax error exception.
class syntax_error : public std::runtime_error {
public:
	///	@brief	Constructor.
	syntax_error() noexcept :
		std::runtime_error("syntax_error") {}
	///	@brief	Constructor.
	///	@param[in]	message		Message to display.
	explicit syntax_error(std::string const& message) noexcept :
		std::runtime_error(message) {}
	///	@brief	Constructor.
	///	@param[in]	message		Message to display.
	explicit syntax_error(char const* const message) noexcept :
		std::runtime_error(message) {}
};

///	@brief	I/O error exception.
class io_error : public std::ios_base::failure {
public:
	///	@brief	Constructor.
	///	@param[in]	code	Error code.
	explicit io_error(std::error_code const& code) :
		std::ios_base::failure("io_error", code) {}
	///	@brief	Constructor.
	///	@param[in]	path	Path of the file.
	///	@param[in]	code	Error code.
	explicit io_error(std::filesystem::path const& path, std::error_code const& code) :
		std::ios_base::failure(path.string(), code) {}
};

}	 // namespace ex
namespace impl {
namespace def {
static std::set<std::string_view> const void_tags{"br", "hr", "img", "meta", "input", "link", "area", "base", "col", "embed", "param", "source", "track", "wbr"};
static std::set<std::string_view> const compare_ops{"==", "===", "!=", "!==", "<", "<=", ">", ">="};
static std::set<std::string_view> const assign_ops{
	"=",
	"+=",
	"-=",
	"*=",
	"/=",
	"%=",
};
static std::unordered_map<char, std::string> const escapes{{'<', "&lt;"}, {'>', "&gt;"}, {'&', "&amp;"}, {'"', "&quot;"}, {'\'', "&#39;"}};

static std::string_view const raw_html_sv{"."};
static std::string_view const folding_sv{"| "};
static std::string_view const comment_sv{"//-"};
static std::string_view const raw_comment_sv{"//"};
static std::string_view const var_sv{"#{"};
static std::string_view const default_sv{"default"};

static std::regex const binary_op_re{R"(^([^ \t]+)[ \t]+([^ \t]+)[ \t]+([^ \t]+)$)"};
static std::regex const string_re{R"(^(['"])([^'"]*)(['"])$)"};	   // TODO: escape sequence is unsupported.
static std::regex const integer_re{R"(^(-?[0-9]+)$)"};

static std::regex const doctype_re{R"(^[dD][oO][cC][tT][yY][pP][eE] ([A-Za-z0-9_]+)$)"};
static std::regex const tag_re{R"(^([#.]?[A-Za-z_-][A-Za-z0-9_-]*))"};
static std::regex const attr_re{R"(^([A-Za-z_-][A-Za-z0-9_-]*)(=['"][^'"]*['"])?[ ,]*)"};
static std::regex const id_re{R"(^#([A-Za-z_-][A-Za-z0-9_-]*))"};
static std::regex const class_re{R"(^\.([A-Za-z_-][A-Za-z0-9_-]*))"};

static std::regex const nest_re{R"(^([\t]*)(.*)$)"};	///	@brief	This implementation supports only tabs as indent.
static std::regex const comment_re{R"(^//-[ \t]?(.*)$)"};
static std::regex const empty_re{R"(^[ \t]*$)"};
static std::regex const case_re{R"(^case[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)$)"};
static std::regex const when_re{R"(^when[ \t]+(["'])([A-Za-z_-][A-Za-z0-9_-]*)(["'])$)"};
static std::regex const break_re{R"(^-[ \t]+break$)"};
static std::regex const if_re{R"(^if[ \t]+(.*)$)"};
static std::regex const elif_re{R"(^else[ \t]+if[ \t]+(.*)$)"};
static std::regex const else_re{R"(^else[ \t]*$)"};
static std::regex const each_re{R"(^each[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*in[ \t]*\[([^\]]*)\]$)"};
static std::regex const for_re{R"(^-[ \t]+for[ \t]*\([ \t]*var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+);[ \t]*([ \tA-Za-z0-9_+*/%=<>!-]*);[ \t]*([ \tA-Za-z0-9_+*/%=<>!-]*)\)$)"};
static std::regex const var_re{R"(^-[ \t]+var[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)"};
static std::regex const const_re{R"(^-[ \t]+const[ \t]+([A-Za-z_-][A-Za-z0-9_-]*)[ \t]*=[ \t]*([^;]+)$)"};
static std::regex const include_re{R"(^include[ \t]+([^ ]+)$)"};
static std::regex const block_re{R"(^block[ \t]+([^ ]+)$)"};
static std::regex const extends_re{R"(^extends[ \t]+([^ ]+)$)"};
// TODO: mixin
}	 // namespace def

///	@brief	Reads the file as string.
///	@param[in]	path	Path of the file to read.
///	@return		Context of the file.
///	@throws		xxx::pug::ex::io_error		It throws the exception if an I/O error occurred.
inline std::string load_file(std::filesystem::path const& path) {
	try {
		std::ifstream ifs;
		ifs.exceptions(std::ios::badbit | std::ios::failbit);
		ifs.open(path, std::ios::in | std::ios::binary);

		auto const size = std::filesystem::file_size(path);
		if (size == 0u) return std::string{};

		std::vector<char> v;
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
inline std::vector<std::string_view> split_lines(std::string_view const str) {
	std::vector<std::string_view> v;
	std::string_view			  s = str;
	for (auto pos = s.find('\n'); pos != std::string_view::npos; pos = s.find('\n')) {
		auto const line = s.substr(0, pos);
		s				= s.substr(pos + 1);
		if (line.empty()) continue;
		auto const crlf = line.back() == '\r';
		if (crlf && line.size() == 1u) continue;
		v.push_back(crlf ? line.substr(0, line.size() - 1) : line);
	}
	if (! s.empty()) {
		v.push_back(s);
	}
	return v;
}

///	@brief	Nested level.
using nest_t = std::size_t;
///	@brief	Primitive line.
///		The first element is nested level.
///		The second element is view of string.
///	@warning	Keep original string available because it returns view of the string.
using line_t = std::pair<nest_t, std::string_view>;
///	@brief	Regular expression result using view of string.
///	@warning	Keep original string available because it returns view of the string.
using svmatch = std::match_results<std::string_view::const_iterator>;

///	@brief	Gets a captured string matched to regular expression.
///		It returns partial view of the @p s string,
///		which differs from 'm.str(n)' that generates a new string.
///	@warning	Keep original string available because it returns view of the string.
/// @param[in]	s		Target string of the regular expression.
/// @param[in]	m		Matching result of the regular expression.
/// @param[in]	n		Index of the captured result. Exceptionally, zero means while of matching.
/// @return		A captured string or while of matching.
///	@warning	Keep original string available because it returns view of the string.
inline std::string_view to_str(std::string_view s, svmatch const& m, std::size_t n) noexcept {
	return n < m.size() ? s.substr(m.position(n), m.length(n)) : std::string_view{};
}

///	@brief	Gets a nested line from a raw line string.
///	@param[in]	line	A raw line string.
///	@return		Nested line.
///	@warning	Keep original string available because it returns view of the string.
inline line_t get_line_nest(std::string_view const line) {
	if (svmatch m; std::regex_match(line.cbegin(), line.cend(), m, def::nest_re)) {
		return {m.length(1), to_str(line, m, 2)};
	} else {
		return {0u, line};
	}
}

///	@brief	Node of nested lines.
///	@warning	Keep original string available because it returns view of the string.
class line_node_t {
public:
	///	@brief	Gets the nested level of the node.
	///	@return		Nested level.
	nest_t nest() const noexcept { return line_.first; }
	///	@brief	Gets the tabs to indent.
	///	@return		Tabs to indent.
	std::string tabs() const { return std::string(nest(), '\t'); }
	///	@brief	Gets the line of the node.
	///	@return		Line of the node.
	auto const& line() const noexcept { return line_.second; }
	///	@brief	Push the @p line as a child of the @p parent.
	/// @param[in]	line	Line to push.
	/// @param[in]	parent	Parent of the @p line.
	///	@return		The pushed line.
	auto& push_nest(line_t const& line, std::shared_ptr<line_node_t> parent) {
		children_.push_back(std::make_shared<line_node_t>(line, parent));
		return children_.back();
	}
	///	@copydoc	line_node_t::push_nest(line_t const&,std::shared_ptr<line_node_t>)
	auto& push_nest(line_t&& line, std::shared_ptr<line_node_t> parent) {
		children_.emplace_back(std::make_shared<line_node_t>(line, parent));
		return children_.back();
	}
	///	@brief	Gets the children of the node.
	///	@return		the children of the node.
	std::vector<std::shared_ptr<line_node_t const>> children() const noexcept { return std::vector<std::shared_ptr<line_node_t const>>(children_.cbegin(), children_.cend()); }
	///	@brief	Gets the parent of the node.
	///	@return		the parent of the node.
	///				It returns null if the node is the root of nodes.
	auto parent() const noexcept { return parent_.lock(); }
	///	@brief	Gets whether the node is folding or not.
	///	@return		It returns true if the node is folding; otherwise, it returns false.
	bool folding() const noexcept { return folding_; }
	///	@brief	Sets whether the node is folding or not.
	///	@param[in]	on		Whether the node is folding or not.
	/// @arg	true		Node is folding.
	/// @arg	false		Node is not folding.
	void set_folding(bool on) noexcept { folding_ = on; }
	///	@brief	Clears all the children.
	void clear_children() noexcept { children_.clear(); }
	///	@brief	Gets the previous 'sister' line.
	///		The 'sister' is a child of the same parent.
	///	@return		The previous 'sister' line.
	std::shared_ptr<line_node_t const> previous() const {
		auto const parent = parent_.lock();
		return ! parent || parent->children().empty() ? nullptr : parent->children().back();
	}
	///	@brief	Constructor.
	///	@param[in]	line	Line
	///	@param[in]	parent	Parent of this node.
	explicit line_node_t(line_t const& line, std::shared_ptr<line_node_t> parent) noexcept :
		children_{}, parent_{parent}, line_{line}, folding_{} {}
	///	@brief	Constructor.
	line_node_t() noexcept :
		children_{}, parent_{}, line_{}, folding_{} {}

private:
	std::vector<std::shared_ptr<line_node_t>> children_;	///< @brief	Children of the node.
	std::weak_ptr<line_node_t>				  parent_;		///< @brief	Parent of the node.
	line_t									  line_;		///< @brief	Line of the node.
	bool									  folding_;		///< @brief	Whether folding or not.
};

///	@brief	Pops nested nodes to the @p nest or less level.
///		It returns an ancestor has the nested level less than or equal to the @p nest.
///	@param[in]	node	The current node.
///	@param[in]	nest	Nested level to pop.
///	@return		The popped node.
inline std::shared_ptr<line_node_t> pop_nest(std::shared_ptr<line_node_t> node, nest_t nest) {
	return ! node || node->nest() <= nest ? node : pop_nest(node->parent(), nest);
}

///	@brief	Dumps hierarchy of nodes to the output stream.
///	@param[in,out]	os	Output stream.
///	@param[in]	node	Node to dump.
///	@param[in]	nest	Current nested level.
inline void dump_lines(std::ostream& os, std::shared_ptr<line_node_t const> node, size_t nest = 0u) {
	if (! node) return;
	std::size_t const limit = 16;
	auto const		  s		= node->line();
	std::string const line	= limit < s.size() ? std::string{s.substr(0, limit)} + " ... " + std::string{s.substr(s.size() - limit)} : std::string{s};
	os << std::string(nest, '\t') << line << ":" << node->nest();
	if (auto const& ch = node->children(); ! ch.empty()) {
		os << "{" << std::endl;
		std::for_each(ch.cbegin(), ch.cend(), [&os, nest](auto const& a) { dump_lines(os, a, nest + 1); });
		os << std::string(nest, '\t') << "}" << std::endl;
	} else {
		os << "{}" << std::endl;
	}
}

///	@brief	Parses file context as pug.
///	@param[in]	pug		File context formed as pug.
///	@param[in]	nest	Base of nested level. It is added to nested levels of parsed nodes.
///	@return		The root of parsed nodes.
///	@warning	Keep original string available because it returns view of the string.
inline std::shared_ptr<line_node_t> parse_file(std::string_view pug, nest_t nest = 0u) {
	auto	   root		 = std::make_shared<line_node_t>(line_t{nest, std::string_view{}}, nullptr);
	auto const raw_lines = split_lines(pug);
	auto const lines	 = raw_lines | std::views::transform(&get_line_nest) | std::views::transform([nest](auto const& a) { return line_t{a.first + nest, a.second}; });

	// Parses to tree of nested lines.
	(void)std::accumulate(lines.begin(), lines.end(), root, [](auto&& previous, auto const& a) {
		auto parent = previous->parent() ? previous->parent() : previous;
		if (a.second.starts_with(def::folding_sv)) {
			if (parent == previous) {
				throw ex::syntax_error(__func__ + std::to_string(__LINE__));	// Folding line never be the top.
			}
			parent->set_folding(true);	  // Parent is folding.
		}
		if (a.second.starts_with(def::comment_sv)) {
			line_t const line{previous->nest(), a.second};
			previous = parent->push_nest(line, parent);	   // Comment is always in the current level.
		} else if (a.second.starts_with(def::raw_comment_sv)) {
			// There is nothing to do.								// Drops pug comment.
		} else if (std::regex_match(a.second.cbegin(), a.second.cend(), def::empty_re)) {
			// There is nothing to do.
			// Drops empty line.
		} else if (previous->nest() == a.first) {
			previous = parent->push_nest(a, parent);	// This line is a sister of the previous line.
		} else if (parent->nest() < a.first) {
			if (a.first <= previous->nest()) {
				previous = parent->push_nest(a, parent);	// This line is a grandchild of the previous line.
			} else {
				previous = previous->push_nest(a, previous);	// This line is a child of the previous line.
			}
		} else {
			previous = pop_nest(previous, a.first);
			if (previous->nest() < a.first) {
				previous = previous->push_nest(a, previous);	// This line is a cousin of the previous line.
			} else {
				parent	 = previous->parent() ? previous->parent() : previous;
				previous = parent->push_nest(a, parent);	// This line is an aunt of the previous line.
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
inline bool is_folding(std::shared_ptr<line_node_t const> line, bool parent_only = false) {
	if (auto const parent = line->parent(); parent && parent->folding()) {
		return true;
	}
	return ! parent_only && line->folding();
}

///	@brief	Parsing context,
class context_t {
	///	@brief	Map of blocks.
	using blocks_t = std::unordered_map<std::string_view, std::shared_ptr<line_node_t const>>;
	///	@brief	Map of blocks.
	using variables_t = std::unordered_map<std::string_view, std::string>;

public:
	// ------------------------------
	// Block for expands

	///	@brief	Gets the block.
	///	@param[in]	tag		Name of the block.
	///	@return		The block.
	auto const& block(std::string_view tag) const { return blocks_.at(tag); }
	///	@brief	Has the block or not.
	///	@param[in]	tag		Name of the block.
	///	@return		It returns true if the block exists; otherwise, it returns false.
	bool has_block(std::string_view tag) const noexcept { return blocks_.contains(tag); }
	///	@brief	Sets the block.
	///	@param[in]	tag		Name of the block. Empty is invalid.
	///	@param[in]	block	Line of the block.
	void set_block(std::string_view tag, std::shared_ptr<line_node_t const> block) {
		if (tag.empty()) throw std::invalid_argument(__func__);
		blocks_[tag] = block;
	}

	// ------------------------------
	// Variables.

	///	@brief	Gets all the variables.
	///	@return		All the variables in current context.
	auto const& variables() const noexcept { return variables_; }
	///	@brief	Gets the variable.
	///	@param[in]	tag		Name of the variable.
	///	@return		The variable.
	auto const& variable(std::string_view tag) const noexcept { return variables_.at(tag); }
	///	@brief	Has the variable or not.
	///	@param[in]	tag		Name of the variable.
	///	@return		It returns true if the variable exists; otherwise, it returns false.
	bool has_variable(std::string_view tag) const noexcept { return variables_.contains(tag); }
	///	@brief	Sets the variable.
	///	@param[in]	tag		Name of the variable. Empty is invalid.
	///	@param[in]	block	Line of the variable.
	void set_variable(std::string_view tag, std::string_view variable) {
		if (tag.empty()) throw std::invalid_argument(__func__);
		variables_[tag] = variable;
	}
	///	@brief	Constructor.
	context_t() noexcept :
		blocks_{}, variables_{} {}

private:
	blocks_t	blocks_;	   ///< @brief	Blocks.
	variables_t variables_;	   ///< @brief	Variables.
};

///	@brief	Replaces all the variables (#{xxx}) in the @p str.
///	@param[in]	context	Context including variables.
///	@param[in]	str		Input string.
///	@return	Replaced string.
inline std::string replace_variables(context_t const& context, std::string_view str) {
	if (str.find(def::var_sv) == std::string_view::npos) return std::string{str};

	// It is not effective.because this implementation will scan while of the string by variables-count times.
	auto s = std::string{str};
	std::ranges::for_each(context.variables(), [&s](auto const& a) {
		s = std::regex_replace(s, std::regex{R"(#\{)" + std::string{a.first} + R"(\})"}, a.second);
	});
	return s;
}

std::tuple<std::string, context_t> parse_line(context_t const&, std::shared_ptr<line_node_t const>, std::filesystem::path const&);

///	@brief	Parses a element from the @p line.
///		This implementation supports only the following order:
///			tag#id.class.class(attr,attr)
///		This implementation supports only single line element:
///		Only element can be nested by ': '.
///	@param[in]	s		Pug source.
///	@param[in]	line	Line of the pug.
///	@param[in]	context	Parsing context.
///	@return		It returns the followings:
///		-#	Remaining string of the line.
///		-#	Output stream.
///		-#	Tag name to close later.
///	@warning	Keep original string available because it returns view of the string.
inline std::tuple<std::string_view, std::string, std::string_view>
parse_element(std::string_view s, std::shared_ptr<line_node_t const> line) {
	if (! line) throw std::invalid_argument(__func__);

	if (s.empty() && line->parent()) {
		return {std::string_view{}, "\n", std::string_view{}};
	} else if (s == def::raw_html_sv) {
		auto const& children = line->children();
		return {std::string_view{}, std::accumulate(std::ranges::cbegin(children), std::ranges::cend(children), std::ostringstream{}, [](auto&& os, auto const& a) {
										os << a->tabs() << a->line() << '\n';
										return std::move(os);
									}).str(),
				std::string_view{}};
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, def::doctype_re)) {
		// This implementation allows it is nested by the ': ' sequence.
		std::ostringstream os;
		os << "<!DOCTYPE " << to_str(s, m, 1) << ">" << '\n';
		return {std::string_view{}, os.str(), std::string_view{}};
	} else if (std::regex_search(s.cbegin(), s.cend(), m, def::tag_re)) {
		// Tag
		auto			   tag		= to_str(s, m, 1);
		auto const		   void_tag = def::void_tags.contains(tag);
		std::ostringstream os;
		if (! is_folding(line, true)) {
			os << line->tabs();
		}
		os << "<";
		if (tag.starts_with('.') || tag.starts_with('#')) {
			using namespace std::string_view_literals;
			tag = "div"sv;
			os << tag;	  // The 'div' tag can be omitted.
		} else {
			os << tag;
			s = s.substr(tag.length());
		}
		bool escape = false;
		if (s.empty() || s.starts_with(": ")) {
			s = s.empty() ? std::string_view{} : s.substr(2);
			os << (void_tag ? " />" : ">");
			os << (is_folding(line) ? "" : "\n");
			return {s, os.str(), void_tag ? std::string_view{} : tag};
		} else if (s.starts_with("!=")) {
			s = s.substr(2);
		} else if (s.starts_with('=')) {
			escape = true;
			s	   = s.substr(1);
		}

		// ID
		if (std::regex_search(s.cbegin(), s.cend(), m, def::id_re)) {
			os << R"( id=")" << to_str(s, m, 1) << R"(")";
			s = s.substr(m.length());
		}
		// Class
		if (s.starts_with('.')) {
			os << R"( class=")";
			bool first = true;
			for (; std::regex_search(s.cbegin(), s.cend(), m, def::class_re); s = s.substr(m.length())) {
				if (! first) {
					os << ' ';
				}
				os << to_str(s, m, 1);
				first = false;
			}
			os << R"(")";
		}
		// Attributes
		if (s.starts_with('(')) {
			s = s.substr(1);
			for (; std::regex_search(s.cbegin(), s.cend(), m, def::attr_re); s = s.substr(m.length())) {
				os << " " << to_str(s, m, 1);
				if (auto const parameter = to_str(s, m, 2); 1u < parameter.size()) {
					if (parameter.at(1) != parameter.back()) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
					os << R"(=")" << parameter.substr(2, parameter.size() - 3) << R"(")";
				}
			}
			if (! s.starts_with(')')) {
				throw ex::syntax_error(__func__ + std::to_string(__LINE__));
			}
			os << " ";
			s = s.substr(1);
		}
		os << (void_tag ? " />" : ">");

		if (s.starts_with(": ")) {
			return {s.substr(2), os.str(), void_tag ? std::string_view{} : tag};
		} else {
			auto const c = s.starts_with(' ') ? s.substr(1) : s;
			if (escape) {
				auto const escaped = c | std::views::transform([](auto const& a) { return def::escapes.contains(a) ? def::escapes.at(a) : std::string(1u, a); });
				std::ranges::for_each(escaped, [&os](auto const& a) { os << a; });
			} else {
				os << c;
			}
			if (! is_folding(line)) {
				os << '\n';
			}
			return {std::string_view{}, os.str(), void_tag ? std::string_view{} : tag};
		}
	} else {
		throw ex::syntax_error(__func__ + std::to_string(__LINE__));
	}
}

///	@brief	Parses children of the @p line.
///	@param[in]	context		Parsing context. It is not constant reference but copied.
///	@param[in]	children	Lines of children.
///	@param[in]	path		Path of the pug.
/// @return		It returns the following:
///		-#	Generated HTML string
///		-#	Context.
inline std::tuple<std::string, context_t> parse_children(context_t context, std::vector<std::shared_ptr<line_node_t const>> const& children, std::filesystem::path const& path) {
	return {std::accumulate(std::ranges::cbegin(children), std::ranges::cend(children), std::ostringstream{}, [&context, &path](auto&& os, auto const& a) {
				auto const [s, c] = parse_line(context, a, path);
				context			  = c;
				os << s;
				return std::move(os);
			}).str(),
			context};
}

namespace eval {

///	@brief	Operand value.
using operand_t = std::variant<long long, bool, std::string_view>;

///	@brief	Operand value to string.
struct operand_to_str {
	std::string operator()(long long v) const { return std::to_string(v); }
	std::string operator()(bool v) const { return v ? "true" : "false"; }
	std::string operator()(std::string_view v) const { return std::string{v}; }
};

///	@brief	Gets an operand value.
///		- If the @p str is boolean, it returns true or false.
///		- If the @p str is integer, it returns its value of long long integer.
///		- If the @p str is string, it returns a view of its string.
///		- If the @p is variable, it returns the value as integer, boolean, or string view.
///	@param[in]	context	Context.
///	@param[in]	str		String.
///	@return		Operand value.
inline operand_t to_operand(context_t const& context, std::string_view str) {
	auto const variable = context.has_variable(str);
	auto const operand	= variable ? context.variable(str) : str;

	if (operand == "true")
		return true;
	else if (operand == "false")
		return false;
	else if (svmatch mm; std::regex_match(operand.cbegin(), operand.cend(), mm, def::integer_re)) {
		long long		   i{};
		std::istringstream iss{std::string{operand}};
		iss >> i;
		return i;
	} else if (std::regex_match(operand.cbegin(), operand.cend(), mm, def::string_re)) {
		if (to_str(operand, mm, 1) != to_str(operand, mm, 3)) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
		return to_str(operand, mm, 2);
	} else if (variable) {
		return operand;
	}
	throw ex::syntax_error(__func__ + std::to_string(__LINE__));
};

///	@brief	Assigns value to variable.
/// @param[in]	context		Context.
/// @param[in]	variable	Variable name.
/// @param[in]	op			Assign operator.
/// @param[in]	value		Value to set.
///	@return		It returns evaluation result of the @p condition.
inline context_t assign(context_t context, std::string_view variable, std::string_view op, operand_t const& value) {
	if (! def::assign_ops.contains(op)) {
		// TODO: Currently, it supports simple assign operators only.
		throw ex::syntax_error(__func__ + std::to_string(__LINE__));
	}
	if (! context.has_variable(variable) && op != "=") {
		throw ex::syntax_error(__func__ + std::to_string(__LINE__));
	} else if (op == "=") {
		context.set_variable(variable, std::visit(eval::operand_to_str{}, value));
	} else if (eval::operand_t v = to_operand(context, variable); std::holds_alternative<std::string_view>(v)) {
		if (auto const var = std::get<std::string_view>(v); op == "+=") {
			context.set_variable(variable, std::string{var} + std::visit(eval::operand_to_str{}, value));
		} else
			throw ex::syntax_error(__func__ + std::to_string(__LINE__));
	} else if (std::holds_alternative<long long>(v)) {
		if (auto const var = std::get<long long>(v); std::holds_alternative<std::string_view>(value) && op == "+=") {
			context.set_variable(variable, std::to_string(var) + std::string{std::get<std::string_view>(value)});
		} else if (std::holds_alternative<long long>(value)) {
			if (auto const val = std::get<long long>(value); op == "+=") {
				context.set_variable(variable, std::to_string(var + val));
			} else if (op == "-=") {
				context.set_variable(variable, std::to_string(var - val));
			} else if (op == "*=") {
				context.set_variable(variable, std::to_string(var * val));
			} else if (op == "/=") {
				if (val == 0) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				context.set_variable(variable, std::to_string(var / val));
			} else if (op == "%=") {
				if (val == 0) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				context.set_variable(variable, std::to_string(var % val));
			} else
				throw ex::syntax_error(__func__ + std::to_string(__LINE__));
		} else
			throw ex::syntax_error(__func__ + std::to_string(__LINE__));
	}
	return context;
}

///	@brief	Compares two operands.
/// @param[in]	lhs		Left-hand-side operand.
/// @param[in]	op		Binary comparison operator.
/// @param[in]	rhs		Right-hand-side operand.
///	@return		It returns evaluation result of the @p condition.
inline bool compare(operand_t const& lhs, std::string_view op, operand_t const& rhs) {
	if (! def::compare_ops.contains(op)) {
		// TODO: Currently, it supports simple binary comparison operators only.
		throw ex::syntax_error(__func__ + std::to_string(__LINE__));
	}
	if (std::holds_alternative<bool>(lhs)) {
		auto const lv = std::get<bool>(lhs);
		if (std::holds_alternative<bool>(rhs)) {
			auto const rv = std::get<bool>(rhs);
			if (op == "==" || op == "===")
				return lv == rv;
			else if (op == "!=" || op == "!==")
				return lv != rv;
		} else if (std::holds_alternative<long long>(rhs)) {
			auto const rv = std::get<long long>(rhs);
			if (op == "==" || op == "===")
				return lv == (rv != 0);
			else if (op == "!=" || op == "!==")
				return lv != (rv != 0);
		} else if (std::holds_alternative<std::string_view>(rhs)) {
			auto const rv = std::get<std::string_view>(rhs);
			if (op == "==" || op == "===")
				return lv != rv.empty();
			else if (op == "!=" || op == "!==")
				return lv == rv.empty();
		}
	} else if (std::holds_alternative<long long>(lhs)) {
		auto const lv = std::get<long long>(lhs);
		if (std::holds_alternative<long long>(rhs)) {
			auto const rv = std::get<long long>(rhs);
			if (op == "==" || op == "===")
				return lv == rv;
			else if (op == "!=" || op == "!==")
				return lv != rv;
			else if (op == "<")
				return lv < rv;
			else if (op == "<=")
				return lv <= rv;
			else if (op == ">")
				return lv > rv;
			else if (op == ">=")
				return lv >= rv;
		} else if (std::holds_alternative<bool>(rhs)) {
			auto const rv = std::get<bool>(rhs);
			if (op == "==" || op == "===")
				return (lv != 0) == rv;
			else if (op == "!=" || op == "!==")
				return (lv != 0) != rv;
		} else if (std::holds_alternative<std::string_view>(rhs)) {
			auto const rv = std::get<std::string_view>(rhs);
			if (op == "==" || op == "===")
				return std::to_string(lv) == rv;
			else if (op == "!=" || op == "!==")
				return std::to_string(lv) != rv;
		}
	} else if (std::holds_alternative<std::string_view>(lhs)) {
		auto const lv = std::get<std::string_view>(lhs);
		auto const rv = std::visit(operand_to_str{}, rhs);
		if (op == "==" || op == "===")
			return lv == rv;
		else if (op == "!=" || op == "!==")
			return lv != rv;
	}
	throw ex::syntax_error(__func__ + std::to_string(__LINE__));
}

}	 // namespace eval

///	@brief	Evaluates the @p condition.
///	@param[in]	context		Context.
///	@param[in]	expression	Condition.
///	@return		It returns the followings:
///		-#	Result of the evaluation.
///		-#	Context.
inline std::tuple<bool, context_t> evaluate(context_t const& context, std::string_view expression) {
	if (svmatch m; std::regex_match(expression.cbegin(), expression.cend(), m, def::binary_op_re)) {
		auto const&			  op  = to_str(expression, m, 2);
		eval::operand_t const rhs = eval::to_operand(context, to_str(expression, m, 3));
		if (def::compare_ops.contains(op)) {
			eval::operand_t const lhs = eval::to_operand(context, to_str(expression, m, 1));
			return {eval::compare(lhs, op, rhs), context};
		} else if (def::assign_ops.contains(op)) {
			return {true, eval::assign(context, to_str(expression, m, 1), op, rhs)};
		}
	}
	// TODO: Currently, it supports simple binary comparison operators only.
	throw ex::syntax_error(__func__ + std::to_string(__LINE__));
}

///	@brief	Parses a line of pug.
///	@param[in]	context	Parsing context.
///	@param[in]	line	Line of the pug.
///	@param[in]	path	Path of the pug.
/// @return		It returns the following:
///		-#	Generated HTML string
///		-#	Context.
inline std::tuple<std::string, context_t> parse_line(context_t const& context, std::shared_ptr<line_node_t const> line, std::filesystem::path const& path) {
	if (! line) return {std::string{}, context};

	if (auto const& s = line->line(); s.starts_with(def::folding_sv)) {
		return {replace_variables(context, s.substr(2)), context};
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, def::comment_re)) {
		auto const out = line->tabs() + "<!-- " + replace_variables(context, to_str(s, m, 1)) + " -->" + '\n';
		return {out, context};
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, def::include_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const pug	  = std::filesystem::path{path}.replace_filename(to_str(s, m, 1));
		auto const source = load_file(pug);	   // This string will be invalidated at the end of this function.
		auto const sub	  = parse_file(source, line->nest());
		return parse_line(context, sub, path);	  // Thus, output of the included pug must be finished here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::extends_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const pug	  = std::filesystem::path{path}.replace_filename(to_str(s, m, 1));
		auto const source = load_file(pug);	   // This string will be invalidated at the end of this function.
		auto const sub	  = parse_file(source, line->nest());
		return parse_line(context, sub, path);	  // Thus, output of the included pug must be finished here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::block_re)) {
		if (auto const& tag = to_str(s, m, 1); context.has_block(tag)) {
			// TODO: increases indent.
			return parse_children(context, context.block(tag)->children(), path);
		} else {
			context_t ctx = context;
			ctx.set_block(tag, line);
			return {std::string{}, ctx};
		}
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::if_re)) {
		// If statement
		if (auto const& condition = to_str(s, m, 1); std::get<0>(evaluate(context, condition))) {
			// Ignores following elses.
			return parse_children(context, line->children(), path);
		}

		// Collects else-if and elses.
		std::vector<std::pair<std::string_view, std::shared_ptr<line_node_t const>>> elifs;
		std::shared_ptr<line_node_t const>											 else_;
		{
			auto const parent = line->parent();
			if (! parent) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
			auto const& children = parent->children();
			for (auto itr = ++std::ranges::find(children, line), end = std::ranges::cend(children); itr != end; ++itr) {
				auto const& line = (*itr)->line();
				if (svmatch m; std::regex_match(line.cbegin(), line.cend(), m, def::elif_re)) {
					if (else_) throw ex::syntax_error(__func__ + std::to_string(__LINE__));	   // The 'else' appears at only the end of the sequence.
					elifs.push_back({to_str(line, m, 1), *itr});
				} else if (std::regex_match(line.cbegin(), line.cend(), m, def::else_re)) {
					if (else_) throw ex::syntax_error(__func__ + std::to_string(__LINE__));	   // The 'else' appears only once.
					else_ = *itr;
				} else {
					break;
				}
			}
		}
		for (auto const& elif: elifs) {
			if (std::get<0>(evaluate(context, elif.first))) {
				return parse_children(context, elif.second->children(), path);
			}
		}
		if (else_) {
			return parse_children(context, else_->children(), path);
		} else {
			return {std::string{}, context};
		}
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::elif_re)) {
		// There is nothing to do because it is handled at if directive.
		return {std::string{}, context};
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::else_re)) {
		// There is nothing to do because it is handled at if directive.
		return {std::string{}, context};
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::case_re)) {
		auto const ss  = to_str(s, m, 1);
		auto const var = context.has_variable(ss) ? context.variable(ss) : ss;
		// TODO:
		using cases_t		= std::vector<std::pair<std::string_view, std::shared_ptr<line_node_t const>>>;
		auto const contains = [](cases_t const& cases, std::string_view tag) {
			return std::ranges::find_if(cases, [tag](auto const& a) { return a.first == tag; }) != std::ranges::cend(cases);
		};
		auto const& children	= line->children();
		auto const	cases		= std::accumulate(std::ranges::cbegin(children), std::ranges::cend(children), cases_t{}, [contains](auto&& out, auto const& a) {
			   if (auto const& ss = a->line(); ss == def::default_sv) {
				   if (contains(out, std::string_view{})) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				   out.push_back({std::string_view{}, a});
			   } else if (svmatch mm; std::regex_match(ss.cbegin(), ss.cend(), mm, def::when_re)) {
				   if (to_str(ss, mm, 1) != to_str(ss, mm, 3)) {
					   throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				   }
				   auto const label = to_str(a->line(), mm, 2);
				   if (contains(out, label)) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				   out.push_back({label, a});
			   } else {
				   throw ex::syntax_error(__func__ + std::to_string(__LINE__));
			   }
			   return std::move(out);
		   });
		auto const	parse_cases = [](context_t context, cases_t const& cases, std::string_view label, std::filesystem::path const& path) -> std::tuple<std::string, context_t> {
			 for (auto itr = std::ranges::find_if(cases, [label](auto const& a) { return a.first == label; }); itr != std::ranges::cend(cases); ++itr) {
				 auto const& children = itr->second->children();
				 if (children.empty()) continue;
				 auto const& line = children.front()->line();
				 if (svmatch mm; std::regex_match(line.cbegin(), line.cend(), mm, def::break_re)) {
					 break;
				 }
				 return parse_children(context, children, path);
			 }
			 return {std::string{}, context};
		};
		if (contains(cases, var)) {
			return parse_cases(context, cases, var, path);
		} else if (contains(cases, std::string_view{})) {
			return parse_cases(context, cases, std::string_view{}, path);
		} else {
			return {std::string{}, context};
		}
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::for_re)) {
		auto const var		 = to_str(s, m, 1);
		auto const initial	 = to_str(s, m, 2);
		auto const condition = to_str(s, m, 3);
		auto const advance	 = to_str(s, m, 4);

		std::ostringstream oss;
		{
			context_t		ctx = context;
			eval::operand_t v	= eval::to_operand(ctx, initial);	 // TODO: It supports a single literal only.
			ctx.set_variable(var, std::visit(eval::operand_to_str{}, v));
			while (std::get<0>(evaluate(ctx, condition))) {	   // TODO: It supports simple binary comparison only.
				auto r		 = parse_children(ctx, line->children(), path);
				auto [ss, c] = evaluate(std::get<1>(r), advance);	 // TODO:
				oss << std::get<0>(r);
				ctx = c;
			}
		}
		return {oss.str(), context};	// Drops ctx.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::each_re)) {
		// TODO:
		auto const&				 name = to_str(s, m, 1);
		std::istringstream		 iss(std::string{to_str(s, m, 2)});
		std::vector<std::string> items;
		for (std::string item; std::getline(iss, item, ',');) {
			auto const begin = item.find_first_not_of(" \t");
			if (begin == std::string::npos) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
			auto const end = item.find_last_not_of(" \t,");
			auto const i   = item.substr(begin, end - begin + 1);
			if (i.starts_with('"') || i.starts_with("'")) {
				if (i.size() < 2) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				if (i.front() != i.back()) throw ex::syntax_error(__func__ + std::to_string(__LINE__));
				items.emplace_back(i.substr(1, i.size() - 2));
			} else {
				items.emplace_back(i);
			}
		}

		if (items.empty()) {
			return {std::string{}, context};
		}
		context_t  ctx	= context;
		auto const outs = items | std::views::transform([&ctx, name, &line, &path](auto const& a) {
							  ctx.set_variable(name, a);
							  return parse_children(ctx, line->children(), path);
						  });
		return {std::accumulate(std::ranges::cbegin(outs), std::ranges::cend(outs), std::ostringstream{}, [](auto&& os, auto const& a) {
					os << std::get<0>(a);
					return std::move(os);
				}).str(),
				std::get<1>(outs.back())};
	} else if (std::regex_match(s.cbegin(), s.cend(), m, def::var_re) || std::regex_match(s.cbegin(), s.cend(), m, def::const_re)) {
		auto const& name  = to_str(s, m, 1);
		auto const& value = to_str(s, m, 2);
		context_t	ctx	  = context;
		ctx.set_variable(name, (value.starts_with('"') || value.starts_with("'")) ? value.substr(1, value.size() - 2) : value);
		return {std::string{}, ctx};
	} else {
		std::ostringstream			 oss;
		std::stack<std::string_view> tags;
		for (auto result = std::make_tuple(s, std::string{}, std::string_view{}); ! std::get<0>(result).empty();) {
			result = parse_element(std::get<0>(result), line);
			if (auto const& tag = std::get<2>(result); ! tag.empty()) {
				tags.push(tag);
			}
			oss << replace_variables(context, std::get<1>(result));
		}

		auto const [ss, ctx] = parse_children(context, line->children(), path);
		oss << ss;

		for (; ! tags.empty(); tags.pop()) {
			if (! is_folding(line)) {
				oss << line->tabs();
			}
			oss << "</" << tags.top() << ">";
			if (! is_folding(line)) {
				oss << '\n';
			}
		}
		if (line->folding()) {
			oss << '\n';
		}
		return {oss.str(), ctx};
	}
}

}	 // namespace impl

///	@brief	Translates a pug string to HTML string.
///	@param[in]	pug		Source string formatted in pug.
///	@param[in]	path	Path of working directory.
///	@return		String of generated HTML.
inline std::string pug_string(std::string_view pug, std::filesystem::path const& path = "./") {
	auto const root		  = impl::parse_file(pug);
	auto const [out, ctx] = impl::parse_line(impl::context_t{}, root, path);
	return out;
}

///	@brief	Translates a pug file to HTML string.
///	@param[in]	path	Path of the pug file.
///	@return		String of generated HTML.
inline std::string pug_file(std::filesystem::path const& path) {
	auto const source = impl::load_file(path);
	return pug_string(source, path);
}

}	 // namespace xxx::pug

#endif	  // xxx_PUG_HPP_
