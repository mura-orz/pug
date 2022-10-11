///	@file
///	@brief		pug++  - Pug-HTML translator
///	@author		Mura
///	@copyright	(c) 2022-, Mura.

#include "pug.hpp"

namespace {

///	@brief	Gets a usage string of this program.
///	@return		A usage string of this program.
inline std::string	get_usage() {
	return
		"===[ pug2html ]===  (c) 2022-, Mura." "\n" "\n"
		"[USAGE] $ pug  (options)  {pug file}"  "\n"
		"[options] " "\n"	
		"  -h     : shows this usage only" "\n";
}

///	@brief	Gets a usage string of this program.
///	@tparam		C	Type of the @p container.
///	@tparam		I	Type of the @p item.
///	@param[in]	container	Container to check.
///	@param[in]	item		Item to check.
///	@return		It returns whether the @p container contains the @p item or not.
///	@todo		requires
template<typename C, typename I>
inline bool	contains(C const& container, I const& item) noexcept {
	return std::ranges::find(container, item) != std::ranges::cend(container);
}

///	@brief	Gets the arguments excluding options that starts with the '-' indicator.
///		This function aims to handle the arguments of this program.
///		- An argument that starts with '-' is an 'option', which is a directive to the program.
///		- An argument that does not start with '-' is an 'argument', which is a target of the program.
///		A file name that starts with '-' cannot be specified.
///		Single '-' character that means piped input is not supported. Such argument is dealt as an option.
///	@param[in]	arguments	All the arguments.
///	@return		The arguments.
inline std::vector<std::string_view>		get_arguments(std::vector<std::string_view> const& arguments) {
	auto args	= arguments | std::views::filter([](auto const& a) { return ! a.starts_with('-'); }) | std::views::common;
	return std::vector<std::string_view>(args.begin(), args.end());
}

///	@brief	Gets an output HTML path from the original pug.
///	@param[in]	path		Path of the original pug file.
///	@return		Path of the output HTML
inline std::string							get_ouput_filename(std::string_view path) {
	return std::filesystem::path{ path }.replace_extension(".html").string();
}

///	@brief	Outputs the @p context into the file of the @p path.
///		If an exception occurred, state of the file was unspecified.
///	@param[in]	path		Path of output file.
///	@param[in]	context		Context to output.
///	@throws		xxx::pug::ex::io_error		It throws the exception if an I/O error occurred.
inline void									output(std::filesystem::path const& path, std::string_view content) {
	try {
		std::ofstream	ofs;
		ofs.exceptions(std::ios::badbit | std::ios::failbit);
		ofs.open(path, std::ios::out | std::ios::binary);

		std::ranges::copy(content, std::ostreambuf_iterator<char>(ofs));
	} catch (std::ios_base::failure const& e) {
		throw xxx::pug::ex::io_error(path.string(), e.code());
	}
}

}	// namespace

///	@name	err
///	@brief	Error messages to display.
namespace err {
static char const	Unexpected[]		= "Unexpected exception occurred.";
static char const	No_pugfile[]		= "No pug file is specified.";
static char const	Several_pugfiles[]	= "Several pug files are specified.";
static char const	Syntax_error[]		= "Syntax error found.";
static char const	IO_failed[]			= "I/O error occurred.";
}	// namespace err

///	@brief	Main entry of this program.
///	@param[in]	ac	Argument count.
///	@param[in]	av	Argument values.
///	@return		It returns zero if the program finished translation; otherwise,
///				it returns a negative value if the program failed; otherwise,
///				it returns a positive value if the program shows usage only.
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
			std::string const	pug		= xxx::pug::pug_file(paths.front());
			output(get_ouput_filename(paths.front()), pug);
			return 0;
		}
	} catch (xxx::pug::ex::syntax_error const& e) {
		std::cerr	<< err::Syntax_error << " : " << e.what() << std::endl;
	} catch (xxx::pug::ex::io_error const& e) {
		std::cerr	<< err::IO_failed << " : " << e.what() << " [" << e.code() << "]" << std::endl;
	} catch (std::exception const& e) {
		std::cerr	<< err::Unexpected << " : " << e.what() << std::endl;
	} catch (...) {
		std::cerr	<< err::Unexpected << std::endl;
	}
	return -1;
}
