#ifndef SODA_LEXER_H
#define SODA_LEXER_H

#include "input.h"
#include "token.h"
#include <string>
#include <ostream>

namespace Soda
{

class Lexer
{
public:
	Token token;
	Lexer(std::istream& stream);
	Token::Kind next();

	std::istream& input_stream() { return input.input_stream(); }

private:
	Input input;
	void token_start();
	void token_end();
	bool is_whitespace(Input::char_type ch);
	void skip(Input::char_type ch);
};

} // namespace Soda

#endif // SODA_LEXER_H
