#ifndef SODA_PARSER_H
#define SODA_PARSER_H

#include <soda/ast.h>
#include <soda/syntaxerror.h>
#include <istream>
#include <string>

namespace Soda
{

// Parse UTF-8 stream
void parse(TU& tu, std::istream& stream);

// Parse UTF-8 string
void parse(TU& tu, const std::string& str);

// Open tu.fn and parse resulting UTF-8 stream
void parse(TU& tu);

} // namespace Soda

#endif // SODA_PARSER_H
