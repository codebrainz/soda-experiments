#include <soda/syntaxerror.h>

namespace Soda
{

SyntaxError::SyntaxError(const char *what,
                         const std::string& filename,
                         SourceLocation location,
                         const std::string& message)
	: std::runtime_error(what),
	  fn(filename),
	  location(location),
	  msg(message)
{
}

enum Color
{
	CLR_BLACK,
	CLR_RED,
	CLR_GREEN,
	CLR_YELLOW,
	CLR_BLUE,
	CLR_MAGENTA,
	CLR_CYAN,
	CLR_WHITE,
};

void format_exception(std::ostream& stream, SyntaxError& err)
{
	stream << "\x1B[31merror\x1B[0m:\x1B[33m" << err.fn << "\x1B[0m:";
	if (err.location.line.start != err.location.line.end)
		stream << "\x1B[35m" << err.location.line.start + 1 << "-" << err.location.line.end + 1 << "\x1B[0m:";
	else
		stream << "\x1B[35m" << err.location.line.start + 1 << "\x1B[0m:";
	if (err.location.column.start != err.location.column.start)
		stream << "\x1B[36m" << err.location.column.start << "-" << err.location.column.end;
	else
		stream << "\x1B[36m" << err.location.column.start;
	if (!err.msg.empty())
		stream << "\x1B[0m: " << err.msg << "\n";
	else
		stream << "\x1B[0m\n";
}

} // namespace Soda
