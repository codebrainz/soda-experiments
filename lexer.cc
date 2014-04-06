#include "lexer.h"
#include <iostream>

namespace Soda
{

Lexer::Lexer(std::istream& stream)
	: token(), input(stream)
{
	input.next(); // prime input.last
}

Token::Kind Lexer::next()
{
	if (input.eof())
		return Token::END;

	input.skip_whitespace();

	if (t_identifier())
		return token.kind;
	else if (t_number())
		return token.kind;
	else if (t_comment())
		return token.kind;
	// ...
	else if (t_symbols())
		return token.kind;

	std::cerr << "WTF!?" << std::endl;
	return Token::ERROR;
}

void Lexer::token_start()
{
	token.position.start = input.position;
	token.line.start = input.line;
	token.column.start = input.column;
}

void Lexer::token_end()
{
	token.position.end = input.position;
	token.line.end = input.line;
	token.column.end = input.column;
}

static inline bool is_upper(Input::char_type ch)
{
	return (ch >= 'A' && ch <= 'Z');
}

static inline bool is_lower(Input::char_type ch)
{
	return (ch >= 'a' && ch <= 'z');
}

static inline bool is_alpha(Input::char_type ch)
{
	return (is_lower(ch) || is_upper(ch));
}

static inline bool is_digit(Input::char_type ch)
{
	return (ch >= '0' && ch <= '9');
}

static inline bool is_alnum(Input::char_type ch)
{
	return (is_alpha(ch) || is_digit(ch));
}

Token::Kind Lexer::t_identifier()
{
	token.clear();
	if (is_alpha(input.last) || input.last == '_')
	{
		token.text = input.last;
		token_start();
		while (is_alnum(input.next()))
			token.text += input.last;
		token_end();
		token.kind = Token::IDENT;
	}
	return token.kind;
}

static inline bool is_hex(Input::char_type ch)
{
	return (ch >= '0' && ch <= '9') ||
	       (ch >= 'A' && ch <= 'F') ||
	       (ch >= 'a' && ch <= 'f');
}

static inline bool is_binary(Input::char_type ch)
{
	return (ch == '0' || ch == '1');
}

static inline bool is_octal(Input::char_type ch)
{
	return (ch >= '0' && ch <= '7');
}

Token::Kind Lexer::t_prefixed_number()
{
	token.clear();
	if (input.last == '0')
	{
		switch (input.peek())
		{
			case 'x':
			case 'X':
				token_start();
				input.next(); input.next(); // skip 0[xX]
				token.text = input.last;
				while (is_hex(input.next()))
					token.text += input.last;
				token_end();
				token.kind = Token::HEX_ICONST;
				break;
			case 'b':
			case 'B':
				token_start();
				input.next(); input.next(); // skip 0[bB]
				token.text = input.last;
				while (is_binary(input.next()))
					token.text += input.last;
				token_end();
				token.kind = Token::BIN_ICONST;
				break;
			case 'o':
			case 'O':
				token_start();
				input.next(); input.next(); // skip 0[oO]
				token.text = input.last;
				while (is_octal(input.next()))
					token.text += input.last;
				token_end();
				token.kind = Token::OCT_ICONST;
				break;
			default:
				break;
		}
	}
	return token.kind;
}

Token::Kind Lexer::t_float_or_integer()
{
	token.clear();
	if (is_digit(input.last)
	    || (input.last == '.' && is_digit(input.peek()))) // can start with `.[0-9]+`
	{
		bool is_octal = false, is_float = false;

		if (input.last == '0')
			is_octal = true;

		token_start();
		while (is_digit(input.last) || (!is_float && input.last == '.'))
		{
			if (input.last == '.')
				is_float = true;
			token.text += input.last;
			input.next();
		}
		token_end();

		if (is_octal && !is_float)
			token.kind = Token::OCT_ICONST;
		else if (is_float)
			token.kind = Token::FCONST;
		else
			token.kind = Token::DEC_ICONST;
	}
	return token.kind;
}

Token::Kind Lexer::t_char_lit()
{
	token.clear();
	if (input.last == '\'')
	{
		token_start();
		input.next(); // skip the current '
		while (input.last != '\'')
		{
			token.text += input.last;
			std::cout << input.last <<std::endl;
			if (input.last == '\\' && input.peek() == '\'')
			{
				token.text += input.next(); // store the next '
				std::cout << "skipped. " << input.last << std::endl;
			}
			input.next();
		}
		token_end();
		token.kind = Token::CHAR_ICONST;
	}
	return token.kind;
}

Token::Kind Lexer::t_number()
{
	token.clear();
	if (t_prefixed_number())
		return token.kind;
	else if (t_float_or_integer())
		return token.kind;
	else if (t_char_lit())
		return token.kind;
	return token.kind;
}

// FIXME: copied from input.cc:is_newline()
static inline bool is_newline(Input::char_type ch)
{ // http://www.unicode.org/standard/reports/tr13/tr13-5.html
	switch (ch)
	{
		case 0x000A: // Line Feed
		case 0x000B: // Vertical Tab
		case 0x000C: // Form Feed
		case 0x000D: // Carriage Return
		case 0x0085: // Next Line
		case 0x2028: // Line Separator
		case 0x2029: // Paragraph Separator
			return true;
		default:
			return false; // Not a new line character
	}
}

Token::Kind Lexer::t_single_comment()
{
	token.clear();
	if (input.last == '/' && input.peek() == '/')
	{
		token_start();
		input.next(); input.next(); // skip over //
		while (!is_newline(input.last) && input.last != Input::END)
		{
			token.text += input.last;
			input.next();
		}
		token_end();
		token.kind = Token::COMMENT;
	}
	return token.kind;
}

Token::Kind Lexer::t_multi_comment()
{
	token.clear();
	if (input.last == '/' && input.peek() == '*')
	{
		token_start();
		input.next(); // skip the /*
		while (input.last != '*' && input.peek() != '/' && input.last != Input::END)
		{
			token.text += input.last;
			input.next();
		}

		if (input.last != Input::END)
		{
			input.next(); // skip over the / we peeked
			token_end();
			token.kind = Token::COMMENT;
		}
		else
		{
			// TODO: error
			token.kind = Token::END;
		}
	}
	return token.kind;
}

Token::Kind Lexer::t_comment()
{
	token.clear();
	if (t_single_comment())
		return token.kind;
	else if (t_multi_comment())
		return token.kind;
	return token.kind;
}

Token::Kind Lexer::t_string_double()
{
	return Token::ERROR;
}

Token::Kind Lexer::t_string_triple_double()
{
	return Token::ERROR;
}

Token::Kind Lexer::t_string_triple_single()
{
	return Token::ERROR;
}

Token::Kind Lexer::t_string_lit()
{
	token.clear();
	if (t_string_double())
		return token.kind;
	else if (t_string_triple_double())
		return token.kind;
	else if (t_string_triple_single())
		return token.kind;
	return token.kind;
}

Token::Kind Lexer::t_symbols()
{
	token.clear();
	if (input.last == '>')
	{
		token_start();
		if (input.peek() == '>')
		{
			input.next(); // skip the 2nd >
			if (input.peek() == '=') // >>=
			{
				input.next(); // skip the =
				token_end();
				token.text = U">>=";
				token.kind = Token::RSHIFT_ASSIGN;
			}
			else // >>
			{
				token_end();
				token.text = U">>";
				token.kind = Token::RSHIFT;
			}
		}
		else // >
		{
			token_end();
			token.text = U">";
			token.kind = Token::GT;
		}
	}
	else if (input.last == '<') // <|<<|<<=
	{
		token_start();
		if (input.peek() == '<') // <<|<<=
		{
			input.next();
			if (input.peek() == '=') // <<=
			{
				token_end();
				token.text = U"<<=";
				token.kind = Token::LSHIFT_ASSIGN;
			}
			else // <<
			{
				token_end();
				token.text = U"<<";
				token.kind = Token::LSHIFT;
			}
		}
		else // <
		{
			token_end();
			token.text = U"<";
			token.kind = Token::LT;
		}
	}
	else if (input.last == '+') // +|+=|++
	{
		token_start();
		if (input.peek() == '=') // +=
		{
			input.next(); // skip the =
			token_end();
			token.text = U"+=";
			token.kind = Token::ADD_ASSIGN;
		}
		else if (input.peek() == '+') // ++
		{
			input.next(); // skip the 2nd +
			token_end();
			token.text = U"++";
			token.kind = Token::INC_OP;
		}
		else // +
		{
			token_end();
			token.text = U"+";
			token.kind = Token::PLUS;
		}
	}
	else if (input.last == '-') // -|-=|--
	{
		token_start();
		if (input.peek() == '=') // -=
		{
			input.next(); // skip the =
			token_end();
			token.text = U"-=";
			token.kind = Token::SUB_ASSIGN;
		}
		else if (input.peek() == '-') // --
		{
			input.next(); // skip the -
			token_end();
			token.text = U"--";
			token.kind = Token::DEC_OP;
		}
		else // -
		{
			token_end();
			token.text = U"-";
			token.kind = Token::MINUS;
		}
	}
	return token.kind;
}

} // namespace Soda
