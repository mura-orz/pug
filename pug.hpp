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

class syntax_error : public std::runtime_error {
public:
	syntax_error() noexcept : std::runtime_error("syntax_error") {}
	syntax_error(std::string const& message) noexcept : std::runtime_error(message) {}
	syntax_error(char const* const message) noexcept : std::runtime_error(message) {}
};

class io_error : public std::nested_exception {
public:
	auto const&		path() const noexcept { return path_; }

	io_error() : std::nested_exception(), path_{ "io_error" } {}
	io_error(std::filesystem::path const& path) : std::nested_exception(), path_{ path.string() } {}
private:
	std::string		path_;
};

}	// namespace ex
namespace impl {

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
	} catch (std::ios_base::failure const&) {
		throw ex::io_error(path);
	}
}

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

using	nest_t		= std::size_t;
using	line_t		= std::pair<nest_t, std::string_view>;
using	svmatch		= std::match_results<std::string_view::const_iterator>;

inline std::string_view		to_str(std::string_view s, svmatch const& m, std::size_t n) noexcept {
	return n < m.size() ? s.substr(m.position(n), m.length(n)) : std::string_view{};
}

std::regex const	nest_re{ "^([\t]*)(.*)$" };	// This implementation supports only tabs as indent.

inline line_t	get_line_nest(std::string_view const line) {
	if (svmatch m; std::regex_match(line.cbegin(), line.cend(), m, nest_re)) {
		return { m.length(1), to_str(line, m, 2) };
	} else {
		return { 0u, line };
	}
}

class line_node_t {
public:
	nest_t										nest() const noexcept { return line_.first; }
	std::string									tabs() const { return std::string( nest(), '\t'); }
	auto const&									line() const noexcept {	return line_.second; }
	auto&										push_nest(line_t const& line, std::shared_ptr<line_node_t> parent)	{	children_.push_back(std::make_shared<line_node_t>(line, parent)); return children_.back(); }
	auto&										push_nest(line_t&& line, std::shared_ptr<line_node_t> parent)		{	children_.emplace_back(std::make_shared<line_node_t>(line, parent)); return children_.back(); }
	auto const&									children() const noexcept { return children_; }
	auto const									parent() const noexcept { return parent_.lock(); }
	auto										parent() noexcept { return parent_.lock(); }
	bool										holding() const noexcept { return holding_; }
	void										set_holding(bool on) noexcept { holding_ = on; }
	void										clear_children() noexcept { children_.clear(); }
	std::shared_ptr<line_node_t const>			previous() const {
		auto const parent = parent_.lock();
		return !parent || parent->children().empty() ? nullptr : parent->children().back();
	}
	explicit	line_node_t(line_t const& line, std::shared_ptr<line_node_t> parent) noexcept : children_{}, parent_{ parent }, line_{ line }, holding_{} {}
	line_node_t() noexcept : children_{}, parent_{}, line_{}, holding_{} {}
private:
	std::vector<std::shared_ptr<line_node_t>>	children_;
	std::weak_ptr<line_node_t>					parent_;
	line_t										line_;
	bool										holding_;
};
inline std::shared_ptr<line_node_t>	pop_nest(std::shared_ptr<line_node_t> node, nest_t nest) {
	return ! node || node->nest() <= nest ? node : pop_nest(node->parent(), nest);
};

inline void dump_lines(std::ostream& os, std::shared_ptr<line_node_t> node, size_t nest=0u) {
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

inline std::shared_ptr<line_node_t>		parse_file(std::string_view source, nest_t nest=0u) {
	auto		root	= std::make_shared<line_node_t>(line_t{ nest, std::string_view{} }, nullptr);
	auto const	lines	= split_lines(source) | std::views::transform(&get_line_nest) | std::views::transform([nest](auto const& a) { return line_t{ a.first + nest, a.second }; });

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

inline bool is_holding(std::shared_ptr<line_node_t> line, bool parent_only=false) {
	if (auto const parent = line->parent(); parent && parent->holding()) {
		return true;
	}
	return ! parent_only && line->holding();
}

std::ostream&	parse_lines(std::ostream& os, std::shared_ptr<line_node_t> line, std::filesystem::path const& path);

inline std::tuple<std::string_view,std::string_view, std::shared_ptr<line_node_t>>	parse_element(std::string_view s, std::ostream& os, std::shared_ptr<line_node_t> line, std::filesystem::path const& path) {
	if ( ! line)	throw std::invalid_argument(__func__);
	std::set<std::string_view> const	void_tags{ "br", "hr", "img", "meta", "input", "link", "area", "base", "col", "embed", "param", "source", "track", "wbr" };
	std::string_view const	raw_html{ "." };
	std::regex const	include_re{ R"(^include[ \t]+([^ ]+)$)" };
	std::regex const	block_re{ R"(^block[ \t]+([^ ]+)$)" };
	std::regex const	extends_re{ R"(^extends[ \t]+([^ ]+)$)" };
	std::regex const	doctype_re{ R"(^[dD][oO][cC][tT][yY][pP][eE] ([A-Za-z0-9_]+)$)" };
	std::regex const	for_re{ R"(^-[ \t]+for[ \t]*\(var[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*([^;]+);[ \tA-Za-z0-9_+*/%=<>!-]*;\)$)" };
	std::regex const	var_re{ R"(^-[ \t]+var[ \t]+([A-Za-z_][A-Za-z0-9_]*)[ \t]*=[ \t]*([^;]+)$)" };
	std::regex const	tag_re{ R"(^([#.]?[A-Za-z0-9_-]+))"};
	std::regex const	attr_re{ R"(^([A-Za-z0-9_-]+)(=['"][^'"]*['"])?[ ,]*)" };
	std::regex const	id_re{ R"(^#([A-Za-z0-9_-]+))" };
	std::regex const	class_re{ R"(^\.([A-Za-z0-9_-]+))" };

	if (s.empty() && line->parent()) {
		os	<< '\n';
	} else if (s.starts_with("| ")) {
		os	<< s.substr(2);
		return { std::string_view{}, std::string_view{}, nullptr };
	} else if (auto const& ch = line->children(); s == raw_html) {
		std::for_each(ch.cbegin(), ch.cend(), [&os](auto const& a) { os << a->tabs() << a->line() << '\n'; });
		line->clear_children();	// Outputs raw HTML here and removes them from the tree.
	} else if (svmatch m; std::regex_match(s.cbegin(), s.cend(), m, include_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const	pug		= std::filesystem::path{ path }.replace_filename(to_str(s, m, 1));
		auto const	source	= load_file(pug);	// This string will be invalidated at the end of this function.
		auto const	sub		= parse_file(source, line->nest());
		(void) parse_lines(os, sub, path);		// Thus, output of the included pug must be finised here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, extends_re)) {
		// Opens an including pug file from relative path of the current pug.
		auto const	pug		= std::filesystem::path{ path }.replace_filename(to_str(s, m, 1));
		auto const	source	= load_file(pug);	// This string will be invalidated at the end of this function.
		auto const	sub		= parse_file(source, line->nest());
		(void) parse_lines(os, sub, path);		// Thus, output of the included pug must be finised here.
	} else if (std::regex_match(s.cbegin(), s.cend(), m, block_re)) {
		return { std::string_view{}, to_str(s, m, 1), line };	// TODO:
	} else if (std::regex_match(s.cbegin(), s.cend(), m, doctype_re)) {
		os << "<!DOCTYPE " << to_str(s, m, 1) << ">" << '\n';
	} else if (std::regex_match(s.cbegin(), s.cend(), m, for_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: for " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_match(s.cbegin(), s.cend(), m, var_re)) {
		// TODO:
		std::ranges::for_each(line->children(), [&os](auto const& a) {
			os	<< "<!-- TODO: var " << a->line() << " -->" << '\n';
		});
	} else if (std::regex_search(s.cbegin(), s.cend(), m, tag_re)) {
		// TODO: This implementation supports only the following order: tag#id.class.class(attr,attr)

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
			return { s, void_tag ? std::string_view{} : tag, nullptr };
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
			return { s.substr(2), void_tag ? std::string_view{} : tag, nullptr };
		} else {
			bool const	escaped = s.starts_with('=');
			os	<< (s.starts_with(' ') ? s.substr(1) : s);
			if ( ! is_holding(line)) {
				os	<< '\n';
			}
			return { std::string_view{}, void_tag ? std::string_view{} : tag, nullptr };
		}
	} else {
		throw ex::syntax_error();
	}
	return { std::string_view{}, std::string_view{}, nullptr };
}

using blocks_t	= std::unordered_map<std::string_view, std::shared_ptr<line_node_t>>;

inline std::ostream& parse_lines(std::ostream& os, std::shared_ptr<line_node_t> line, std::filesystem::path const& path) {
	if ( ! line)		return os;
	std::regex const	comment_re{ R"(^//-[ \t]?(.*)$)" };
	std::regex const	empty_re{ R"(^[ \t]*$)" };
	auto const			str		= line->line();

	blocks_t	blocks;

	if (svmatch m; std::regex_match(str.cbegin(), str.cend(), m, comment_re)) {
		os	<< line->tabs() << "<!-- " << to_str(str, m, 1) << " -->" << '\n';
	} else {
		std::stack<std::string_view>	tags;
		for (auto result = std::make_tuple(str, std::string_view{}, std::shared_ptr<line_node_t>{}); !std::get<0>(result).empty();) {
			result	= parse_element(std::get<0>(result), os, line, path);
			if (auto const& block = std::get<2>(result); block) {
				blocks.insert({ std::get<1>(result), block });
			} else if (auto const& tag = std::get<1>(result); ! tag.empty()) {
				tags.push(tag);
			}
		}
		if ( ! line->children().empty()) {
			std::ranges::for_each(line->children(), [&os, &path](auto const& a) { parse_lines(os, a, path); });
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
	return os;
}

}	// namespace impl

inline std::string		pug(std::filesystem::path const& path) {
	using namespace std::string_view_literals;

	auto const	source		= impl::load_file(path);
	auto const	root		= impl::parse_file(source);

	// TODO:	impl::dump_lines(std::clog, root);

	std::ostringstream	oss;
	impl::parse_lines(std::clog, root, path);	// TODO:
	impl::parse_lines(oss, root, path);

	return oss.str();
}

}	// namespace xxx::pug

#endif	// xxx_PUG_HPP_
