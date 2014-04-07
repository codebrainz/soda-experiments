#ifndef SODA_PARSER_H
#define SODA_PARSER_H

#include "ast.h"
#include <istream>
#include <string>
#include <stdexcept>

namespace Soda
{

class ParseError : public std::runtime_error
{
public:
	ParseError(const char *what_="parse error") : std::runtime_error(what_) {}
};

// Parse UTF-8 stream
void parse(TU& tu, std::istream& stream);

// Parse UTF-8 string
void parse(TU& tu, const std::string& str);

// Open tu.fn and parse resulting UTF-8 stream
void parse(TU& tu);

} // namespace Soda

#endif // SODA_PARSER_H
