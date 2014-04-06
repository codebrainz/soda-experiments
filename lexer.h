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

private:
	Input input;
	void token_start();
	void token_end();
	bool is_whitespace(Input::char_type ch);

	Token::Kind t_identifier();

	Token::Kind t_number();
	Token::Kind t_prefixed_number();
	Token::Kind t_float_or_integer();
	Token::Kind t_char_lit();

	Token::Kind t_single_comment();
	Token::Kind t_multi_comment();
	Token::Kind t_comment();

	Token::Kind t_string_double();
	Token::Kind t_string_triple_double();
	Token::Kind t_string_triple_single();
	Token::Kind t_string_lit();

	Token::Kind t_symbols();

};

} // namespace Soda

#endif // SODA_LEXER_H
