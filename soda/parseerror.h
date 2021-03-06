#ifndef SODA_PARSEERROR_H
#define SODA_PARSEERROR_H

#include <soda/sourcelocation.h>
#include <stdexcept>
#include <string>
#include <ostream>

namespace Soda
{

class ParseError : public std::runtime_error
{
public:
	ParseError(const char *what,
	            const std::string& filename,
	            SourceLocation location,
	            const std::string& message);
	const std::string& filename() const noexcept { return fn; }
	int position() const noexcept { return location.offset.start; }
	int line() const noexcept { return location.line.start; }
	int column() const noexcept { return location.column.start; }
	const std::string& message() const noexcept { return msg; }
private:
	std::string fn;
	SourceLocation location;
	std::string msg;
	friend void format_exception(std::ostream&, ParseError&);
};


void format_exception(std::ostream& stream, ParseError& err);

} // namespace Soda

#endif // SODA_PARSEERROR_H
