#ifndef SODA_PARSER_H
#define SODA_PARSER_H

#include <soda/ast.h>
#include <istream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

namespace Soda
{

class SyntaxError : public std::runtime_error
{
public:
	SyntaxError(const char *what,
	            const std::string& filename,
	            int pos_start,  int pos_end,
	            int line_start, int line_end,
	            int col_start,  int col_end,
	            const std::string& message);
	const std::string& filename() const noexcept { return fn; }
	int position() const noexcept { return pos_start; }
	int line() const noexcept { return line_start; }
	int column() const noexcept { return col_start; }
	const std::string& message() const noexcept { return msg; }
private:
	std::string fn;
	int pos_start, pos_end;
	int line_start, line_end;
	int col_start, col_end;
	std::string msg;
	friend void format_exception(std::ostream&, SyntaxError&);
};

void format_exception(std::ostream& stream, SyntaxError& err);

// Parse UTF-8 stream
void parse(TU& tu, std::istream& stream);

// Parse UTF-8 string
void parse(TU& tu, const std::string& str);

// Open tu.fn and parse resulting UTF-8 stream
void parse(TU& tu);

} // namespace Soda

#endif // SODA_PARSER_H
