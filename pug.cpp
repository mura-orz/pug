///	@file
///	@brief		pug++  - Pug-THML translator
///	@author		Mura
///	@copyright	(c) 2022-, Mura.

#include "pug.hpp"

namespace {

inline std::string	get_usage() {
	return
		"===[ pug2html ]===  (c) 2022-, Mura." "\n" "\n"
		"[USAGE] $ pug  (options)  {pug file}"  "\n"
		"[options] " "\n"	
		"  -h     : shows this usage only" "\n";
}

template<typename C, typename I>
inline bool	contains(C const& container, I const& item) noexcept {
	return std::ranges::find(container, item) != std::ranges::cend(container);
}

inline std::vector<std::string_view>		get_arguments(std::vector<std::string_view> const& arguments) {
	auto args	= arguments | std::views::filter([](auto const& a) { return !a.starts_with('-'); }) | std::views::common;
	return std::vector<std::string_view>(args.begin(), args.end());
}

inline std::string							get_ouput_filename(std::string_view path) {
	return std::filesystem::path{ path }.replace_extension(".html").string();
}

inline void									output_pug(std::filesystem::path const& path, std::string_view pug) {
	try {
		std::ofstream	ofs;
		ofs.exceptions(std::ios::badbit | std::ios::failbit);
		ofs.open(path, std::ios::out | std::ios::binary);

		std::ranges::copy(pug, std::ostreambuf_iterator<char>(ofs));
	} catch (std::ios_base::failure const&) {
		throw xxx::pug::ex::io_error(path);
	}
}

}	// namespace

namespace err {
static char const	Unexpected[]		= "Unexpected exception occurred.";
static char const	No_pugfile[]		= "No pug file is specified.";
static char const	Several_pugfiles[]	= "Several pug files are specified.";
static char const	Syntax_error[]		= "Syntax error found.";
static char const	IO_failed[]			= "I/O error occurred.";
}	// namespace err

int		main(int ac, char* av[]) {
	try {
		std::ios::sync_with_stdio(false);
		std::locale::global(std::locale(""));

		std::vector<std::string_view> const		args(av + 1, av + ac);
		if (contains(args, "-h")) {
			std::clog	<< get_usage();
			return 1;
		} else if (auto const paths = get_arguments(args); paths.empty()) {
			std::clog	<< get_usage() << '\n' << err::No_pugfile << '\n';
		} else if (1u < paths.size()) {
			std::clog	<< get_usage() << '\n' << err::Several_pugfiles << '\n';
		} else {
			std::string const	pug		= xxx::pug::pug(paths.front());
			output_pug(get_ouput_filename(paths.front()), pug);
			return 0;
		}
	} catch (xxx::pug::ex::syntax_error const& e) {	// TODO: use progonal syntax error instead.
		std::cerr	<< err::Syntax_error << " : " << e.what() << std::endl;
	} catch (xxx::pug::ex::io_error const& e) {
		try {
			e.rethrow_nested();
		} catch (std::ios_base::failure const& ee) {
			std::cerr	<< err::IO_failed << " : " << e.path() << " [" << ee.code() << "]" << std::endl;
		}
	} catch (std::exception const& e) {
		std::cerr	<< err::Unexpected << " : " << e.what() << std::endl;
	} catch (...) {
		std::cerr	<< err::Unexpected << std::endl;
	}
	return -1;
}
