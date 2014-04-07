#include "lexer.h"
#include "utils.h"
#include <iostream>

namespace Soda
{

Lexer::Lexer(std::istream& stream)
	: token(), input(stream)
{
	input.next(); // prime input.last
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

// verify the current input.last is what we expect and then save it into
// the token text, update the token end location and finally advance the input.
void Lexer::skip(Input::char_type ch)
{
#ifndef NDEBUG
	if (ch != input.last)
	{
		std::cerr << "Wrong current character while skipping to " <<
			"next character. It was supposed to be '" << ch <<
			"' (" << (char)ch << ") but it was '" << input.last <<
			"' (" << (char)input.last << ")." << std::endl;
		std::terminate();
	}
#endif
	token.text += input.last;
	input.next();
	token_end();
}

Token::Kind Lexer::next()
{
	token.clear();

// Whitespace
	while (Soda::is_whitespace(input.last))
		input.next();

// Identifiers: [a-zA-Z_][a-zA-Z_0-9]*
	if (is_alpha(input.last) || input.last == '_')
	{
		token_start();
		skip(input.last);
		while (is_alnum(input.last) || input.last == '_')
			skip(input.last);
		token.kind = Token::IDENT;
	}
// Numbers
	else if (is_digit(input.last) ||
	         (input.last == '.' && is_digit(input.peek())))
	{
		bool found_dot = (input.last == '.');
		token_start();
		skip(input.last);
// 0[xX][a-fA-F0-9]*
		if (input.last == 'x' || input.last == 'X')
		{
			skip(input.last);
			token.text.clear(); token_start(); // skip over the 0x prefix
			while (is_hex(input.last))
				skip(input.last);
			token.kind = Token::HEX_ICONST;
		}
// 0[bB][0-1]*
		else if (input.last == 'b' || input.last == 'B')
		{
			skip(input.last);
			token.text.clear(); token_start(); // skip over the 0b prefix
			while (is_binary(input.last))
				skip(input.last);
			token.kind = Token::BIN_ICONST;
		}
// 0[oO][0-7]*
		else if (input.last == 'o' || input.last == 'O')
		{
			skip(input.last);
			token.text.clear(); token_start(); // skip over the 0o prefix
			while (is_octal(input.last))
				skip(input.last);
			token.kind = Token::OCT_ICONST;
		}
// ([0-9]+|([0-9]*\.[0-9]+|[0-9]+\.[0-9]*))
		else
		{
			while (is_digit(input.last) || input.last == '.')
			{
				if (input.last == '.')
				{
					if (!found_dot)
						found_dot = true;
					// else ERROR
				}
				skip(input.last);
			}
			if (found_dot)
				token.kind = Token::FCONST;
			else
				token.kind = Token::DEC_ICONST;
		}
	}
// Char Literal: "'"[^']+"'"
	else if (input.last == '\'')
	{
		skip('\'');
		token.text.clear(); token_start(); // skip the '
		while (input.last != '\'' && input.last != Input::END)
		{
			if (input.last == '\\' && input.peek() == '\'')
			{
				skip('\\');
				skip('\'');
			}
			else
				skip(input.last);
		}
		if (input.last != Input::END)
		{
			input.next(); // skip the trailing '
			token.kind = Token::CHAR_ICONST;
		}
		else
			token.kind = Token::ERROR;
	}
// String Literal: "\""[^"]*"\""
	else if (input.last == '"')
	{
		skip('"');
		token.text.clear(); token_start(); // skip the "
		while (input.last != '"' && input.last != Input::END)
		{
			if (input.last == '\\' && input.peek() == '"')
			{
				skip('\\');
				skip('\"');
			}
			else
				skip(input.last);
		}
		if (input.last != Input::END)
		{
			input.next(); // skip the trailing "
			token.kind = Token::STR_LIT;
		}
		else
			token.kind = Token::ERROR;
	}
// (">>="|">>"|">="|">")
	else if (input.last == '>')
	{
		token_start();
		skip('>');
		if (input.last == '>')
		{
			skip('>');
// ">>="
			if (input.last == '=')
			{
				skip('=');
				token.kind = Token::RSHIFT_ASSIGN;
			}
// ">>"
			else
				token.kind = Token::RSHIFT;
		}
// ">="
		else if (input.last == '=')
		{
			skip('=');
			token.kind = Token::GE_OP;
		}
// ">"
		else
			token.kind = Token::GT;
	}
// ("<<="|"<<"|"<="|"<")
	else if (input.last == '<')
	{
		token_start();
		skip('<');
		if (input.last == '<')
		{
			skip('<');
// "<<="
			if (input.last == '=')
			{
				skip('=');
				token.kind = Token::LSHIFT_ASSIGN;
			}
// "<<"
			else // <<
				token.kind = Token::LSHIFT;
		}
// "<="
		else if (input.last == '=')
		{
			skip('=');
			token.kind = Token::LE_OP;
		}
// "<"
		else // <
			token.kind = Token::LT;
	}
// ("+="|"++"|"+")
	else if (input.last == '+')
	{
		token_start();
		skip('+');
// "+="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::ADD_ASSIGN;
		}
// "++"
		else if (input.last == '+')
		{
			skip('+');
			token.kind = Token::INC_OP;
		}
// "+"
		else
			token.kind = Token::PLUS;
	}
// ("-="|"--"|"->"|"-")
	else if (input.last == '-')
	{
		token_start();
		skip('-');
// "-="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::SUB_ASSIGN;
		}
// "--"
		else if (input.last == '-')
		{
			skip('-');
			token.kind = Token::DEC_OP;
		}
// "->"
		else if (input.last == '>')
		{
			skip('>');
			token.kind = Token::PTR_OP;
		}
// "-"
		else
			token.kind = Token::MINUS;
	}
// ("*="|"*")
	else if (input.last == '*')
	{
		token_start();
		skip('*');
// "*="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::MUL_ASSIGN;
		}
// "*"
		else
			token.kind = Token::MULTIPLY;
	}
// ("/="|"/"|"//"[^\n]*|"/*".*?"*/")
	else if (input.last == '/')
	{
		token_start();
		skip('/');
// "/="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::DIV_ASSIGN;
		}
// "//"[^\n]*
		else if (input.last == '/') // // single comment
		{
			skip('/');
			token.text.clear();
			while (!is_newline(input.last) && input.last != Input::END)
			{
				token.text += input.last;
				token_end();
				input.next();
			}
			if (input.last != Input::END)
			{
				input.next(); // skip the newline
				token.kind = Token::COMMENT;
			}
			else
				token.kind = Token::END;
		}
// "/*".*?"*/"
		else if (input.last == '*') // /* multi-comment
		{
			skip('*');
			token.text.clear();
			while (input.last != '*' && input.peek() != '/' && input.last != Input::END)
			{
				token.text += input.last;
				token_end();
				input.next();
			}
			if (input.last != Input::END)
			{
				input.next(); input.next(); // skip the trailing */
				token.kind = Token::COMMENT;
			}
			else
				token.kind = Token::ERROR;
		}
// "/"
		else // /
			token.kind = Token::DIVIDE;
	}
// ("%="|"%")
	else if (input.last == '%')
	{
		token_start();
		skip('%');
// "%="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::MOD_ASSIGN;
		}
// "%"
		else
			token.kind = Token::MODULO;
	}
// ("&="|"&&"|"&")
	else if (input.last == '&')
	{
		token_start();
		skip('&');
// "&="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::AND_ASSIGN;
		}
// "&&"
		else if (input.last == '&')
		{
			skip('&');
			token.kind = Token::LOG_AND;
		}
// "&"
		else
			token.kind = Token::BOOL_AND;
	}
// ("^="|"^")
	else if (input.last == '^')
	{
		token_start();
		skip('^');
// "^="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::XOR_ASSIGN;
		}
// "^"
		else
			token.kind = Token::BOOL_XOR;
	}
// ("|="|"||"|"|")
	else if (input.last == '|')
	{
		token_start();
		skip('|');
// "|="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::OR_ASSIGN;
		}
// "||"
		else if (input.last == '|')
		{
			skip('|');
			token.kind = Token::LOG_OR;
		}
// "|"
		else
			token.kind = Token::BOOL_OR;
	}
// ("!="|"!")
	else if (input.last == '!')
	{
		token_start();
		skip('!');
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::NE_OP;
		}
		else
			token.kind = Token::NOT;
	}
// ("=="|"=")
	else if (input.last == '=')
	{
		token_start();
		skip('=');
// "=="
		if (input.last == '=')
		{
			skip('=');
			token.kind = Token::EQ_OP;
		}
// "="
		else
			token.kind = Token::EQ;
	}

// Single characters

#define MATCH_SINGLE(ch, tok_kind)    \
	else if (input.last == ch) {      \
		token_start(); skip(ch);      \
		token.kind = Token::tok_kind; \
	}

	MATCH_SINGLE('.', DOT)
	MATCH_SINGLE('~', TILDE)
	MATCH_SINGLE('?', QUESTION)
	MATCH_SINGLE(':', COLON)
	MATCH_SINGLE(';', SEMICOLON)
	MATCH_SINGLE(',', COMMA)
	MATCH_SINGLE('(', LPAREN)
	MATCH_SINGLE(')', RPAREN)
	MATCH_SINGLE('[', LBRACKET)
	MATCH_SINGLE(']', RBRACKET)
	MATCH_SINGLE('{', LBRACE)
	MATCH_SINGLE('}', RBRACE)

#undef MATCH_SINGLE

// EOF or unmatched input
	if (token.kind == Token::ZERO)
	{
		if (input.last == Input::END) // EOF
			return Token::END;
		else // unmatched input
			return Token::ERROR;
	}

	return token.kind;
}

} // namespace Soda
