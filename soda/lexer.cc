#include <soda/sodainc.h> // pch
#include <soda/lexer.h>
#include <soda/utils.h>
#include <iostream>

namespace Soda
{

struct Lexer::LexImpl
{

Token& token;
Input input;

LexImpl(Token& tok, std::istream& stream)
	: token(tok), input(stream)
{
	input.next(); // prime input.last
}

// Update the current token's start position and clear its text/kind
void begin_token()
{
	token.position.start = input.position;
	token.line.start = input.line;
	token.column.start = input.column;
	clear();
}

// Update the current token's end position (Note: it's called by advance()
// so doesn't often need to be called explicitly.
void end_token()
{
	token.position.end = input.position;
	token.line.end = input.line;
	token.column.end = input.column;
}

// Clear the current token's text and kind (Note: it's called by begin
// token so doesn't often need to be called explicitly).
void clear()
{
	token.clear();
}

// verify the current input.last is what we expect and then save it into
// the token text, update the token end location and finally advance the input.
void advance(Input::char_type ch = std::numeric_limits<Input::char_type>::max())
{
#ifndef NDEBUG
	if ((ch < std::numeric_limits<Input::char_type>::max())
	    && ch != input.last)
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
	end_token();
}

// The big-fat tokenizing routine
Token::Kind next()
{
	clear();

// Whitespace
	while (Soda::is_whitespace(input.last))
		input.next();

// Identifiers: [a-zA-Z_][a-zA-Z_0-9]*
	if (is_alpha(input.last) || input.last == '_')
	{
		begin_token();
		advance(input.last);
		while (is_alnum(input.last) || input.last == '_')
			advance(input.last);
		if (token.text == U"if")
			token.kind = Token::IF;
		else if (token.text == U"elif")
			token.kind = Token::ELIF;
		else if (token.text == U"else")
			token.kind = Token::ELSE;
		else if (token.text == U"fun")
			token.kind = Token::FUN;
		else if (token.text == U"var")
			token.kind = Token::VAR;
		else if (token.text == U"const")
			token.kind = Token::CONST;
		else if (token.text == U"struct")
			token.kind = Token::STRUCT;
		else if (token.text == U"enum")
			token.kind = Token::ENUM;
		else if (token.text == U"union")
			token.kind = Token::UNION;
		else if (token.text == U"alias")
			token.kind = Token::ALIAS;
		else if (token.text == U"return")
			token.kind = Token::RETURN;
		else if (token.text == U"import")
			token.kind = Token::IMPORT;
		else if (token.text == U"from")
			token.kind = Token::FROM;
		else if (token.text == U"class")
			token.kind = Token::CLASS;
		else
			token.kind = Token::IDENT;
	}
// Numbers
	else if (is_digit(input.last) ||
	         (input.last == '.' && is_digit(input.peek())))
	{
		bool found_dot = (input.last == '.');
		begin_token();
		advance(input.last);
// 0[xX][a-fA-F0-9]*
		if (input.last == 'x' || input.last == 'X')
		{
			advance(input.last);
			begin_token(); // skip over the 0x prefix
			while (is_hex(input.last))
				advance(input.last);
			token.kind = Token::HEX_ICONST;
		}
// 0[bB][0-1]*
		else if (input.last == 'b' || input.last == 'B')
		{
			advance(input.last);
			begin_token(); // skip over the 0b prefix
			while (is_binary(input.last))
				advance(input.last);
			token.kind = Token::BIN_ICONST;
		}
// 0[oO][0-7]*
		else if (input.last == 'o' || input.last == 'O')
		{
			advance(input.last);
			begin_token(); // skip over the 0o prefix
			while (is_octal(input.last))
				advance(input.last);
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
				advance(input.last);
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
		advance('\'');
		begin_token(); // skip the '
		while (input.last != '\'' && input.last != Input::END)
		{
			if (input.last == '\\' && input.peek() == '\'')
			{
				advance('\\');
				advance('\'');
			}
			else
				advance(input.last);
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
		advance('"');
		begin_token(); // skip the "
		while (input.last != '"' && input.last != Input::END)
		{
			if (input.last == '\\' && input.peek() == '"')
			{
				advance('\\');
				advance('\"');
			}
			else
				advance(input.last);
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
		begin_token();
		advance('>');
		if (input.last == '>')
		{
			advance('>');
// ">>="
			if (input.last == '=')
			{
				advance('=');
				token.kind = Token::RSHIFT_ASSIGN;
			}
// ">>"
			else
				token.kind = Token::RSHIFT;
		}
// ">="
		else if (input.last == '=')
		{
			advance('=');
			token.kind = Token::GE_OP;
		}
// ">"
		else
			token.kind = Token::GT;
	}
// ("<<="|"<<"|"<="|"<")
	else if (input.last == '<')
	{
		begin_token();
		advance('<');
		if (input.last == '<')
		{
			advance('<');
// "<<="
			if (input.last == '=')
			{
				advance('=');
				token.kind = Token::LSHIFT_ASSIGN;
			}
// "<<"
			else // <<
				token.kind = Token::LSHIFT;
		}
// "<="
		else if (input.last == '=')
		{
			advance('=');
			token.kind = Token::LE_OP;
		}
// "<"
		else // <
			token.kind = Token::LT;
	}
// ("+="|"++"|"+")
	else if (input.last == '+')
	{
		begin_token();
		advance('+');
// "+="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::ADD_ASSIGN;
		}
// "++"
		else if (input.last == '+')
		{
			advance('+');
			token.kind = Token::INC_OP;
		}
// "+"
		else
			token.kind = Token::PLUS;
	}
// ("-="|"--"|"->"|"-")
	else if (input.last == '-')
	{
		begin_token();
		advance('-');
// "-="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::SUB_ASSIGN;
		}
// "--"
		else if (input.last == '-')
		{
			advance('-');
			token.kind = Token::DEC_OP;
		}
// "->"
		else if (input.last == '>')
		{
			advance('>');
			token.kind = Token::PTR_OP;
		}
// "-"
		else
			token.kind = Token::MINUS;
	}
// ("*="|"*")
	else if (input.last == '*')
	{
		begin_token();
		advance('*');
// "*="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::MUL_ASSIGN;
		}
// "*"
		else
			token.kind = Token::MULTIPLY;
	}
// ("/="|"/"|"//"[^\n]*|"/*".*?"*/")
	else if (input.last == '/')
	{
		begin_token();
		advance('/');
// "/="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::DIV_ASSIGN;
		}
// "//"[^\n]*
		else if (input.last == '/') // // single comment
		{
			advance('/');
			while (!is_newline(input.last) && input.last != Input::END)
				advance();
			if (input.last != Input::END)
			{
				input.next(); // skip the newline
				token.kind = Token::COMMENT;
			}
			else
				token.kind = Token::END;
#if 1
			token.kind = next();
#endif
		}
// "/*".*?"*/"
		else if (input.last == '*') // /* multi-comment
		{
			advance('*');
			while (input.last != '*' && input.peek() != '/' && input.last != Input::END)
				advance();
			if (input.last != Input::END)
			{
				input.next(); input.next(); // skip the trailing */
				token.kind = Token::COMMENT;
			}
			else
				token.kind = Token::ERROR;
#if 1
			token.kind = next();
#endif
		}
// "/"
		else // /
			token.kind = Token::DIVIDE;
	}
// ("%="|"%")
	else if (input.last == '%')
	{
		begin_token();
		advance('%');
// "%="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::MOD_ASSIGN;
		}
// "%"
		else
			token.kind = Token::MODULO;
	}
// ("&="|"&&"|"&")
	else if (input.last == '&')
	{
		begin_token();
		advance('&');
// "&="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::AND_ASSIGN;
		}
// "&&"
		else if (input.last == '&')
		{
			advance('&');
			token.kind = Token::LOG_AND;
		}
// "&"
		else
			token.kind = Token::BOOL_AND;
	}
// ("^="|"^")
	else if (input.last == '^')
	{
		begin_token();
		advance('^');
// "^="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::XOR_ASSIGN;
		}
// "^"
		else
			token.kind = Token::BOOL_XOR;
	}
// ("|="|"||"|"|")
	else if (input.last == '|')
	{
		begin_token();
		advance('|');
// "|="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::OR_ASSIGN;
		}
// "||"
		else if (input.last == '|')
		{
			advance('|');
			token.kind = Token::LOG_OR;
		}
// "|"
		else
			token.kind = Token::BOOL_OR;
	}
// ("!="|"!")
	else if (input.last == '!')
	{
		begin_token();
		advance('!');
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::NE_OP;
		}
		else
			token.kind = Token::NOT;
	}
// ("=="|"=")
	else if (input.last == '=')
	{
		begin_token();
		advance('=');
// "=="
		if (input.last == '=')
		{
			advance('=');
			token.kind = Token::EQ_OP;
		}
// "="
		else
			token.kind = Token::EQ;
	}

// Single characters

#define MATCH_SINGLE(ch, tok_kind)    \
	else if (input.last == ch) {      \
		begin_token(); advance(ch);   \
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

}; // LexerImpl

Lexer::Lexer(std::istream& stream)
	: token(), impl(new LexImpl(token, stream))
{
}

Lexer::~Lexer()
{
	delete impl;
}

Token::Kind Lexer::next()
{
	return impl->next();
}

} // namespace Soda
