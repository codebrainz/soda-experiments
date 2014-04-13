#ifndef SODA_TOKEN_H
#define SODA_TOKEN_H

#include <soda/sourcelocation.h>
#include <limits>
#include <string>

namespace Soda
{

struct Token
{
	enum Kind {
		END=std::numeric_limits<int>::min(),
		ERROR=-1,
		ZERO=0,

		LT='<',
		GT='>',
		EQ='=',
		PLUS='+',
		MINUS='-',
		MULTIPLY='*',
		DIVIDE='/',
		MODULO='%',
		BOOL_AND='&',
		BOOL_XOR='^',
		BOOL_OR='|',
		NOT='!',
		DOT='.',
		TILDE='~',
		QUESTION='?',
		COLON=':',
		SEMICOLON=';',
		COMMA=',',
		LPAREN='(',
		RPAREN=')',
		LBRACKET='[',
		RBRACKET=']',
		LBRACE='{',
		RBRACE='}',

		IDENT=256,
		HEX_ICONST,
		BIN_ICONST,
		OCT_ICONST,
		DEC_ICONST,
		CHAR_ICONST,
		FCONST,

		STR_LIT,

		COMMENT=400,

		RSHIFT=500,
		RSHIFT_ASSIGN,
		LSHIFT,
		LSHIFT_ASSIGN,
		ADD_ASSIGN,
		INC_OP,
		SUB_ASSIGN,
		DEC_OP,
		MUL_ASSIGN,
		DIV_ASSIGN,
		MOD_ASSIGN,
		AND_ASSIGN,
		XOR_ASSIGN,
		OR_ASSIGN,
		LOG_AND,
		LOG_OR,
		PTR_OP,
		LE_OP,
		GE_OP,
		NE_OP,
		EQ_OP,

		CONST=700,
		STATIC,
		PUBLIC,
		PRIVATE,
		PROTECTED,
		INTERNAL,

		STRUCT=100,
		ENUM,
		UNION,
		ALIAS,
		VAR,
		FUN,
		RETURN,
		IMPORT,
		FROM,
		CLASS,
		IF,
		ELIF,
		ELSE,
		SWITCH,
		CASE,
		DEFAULT,
		BREAK,
	};

	Kind kind;
	SourceLocation location;
	std::u32string text;

	Token() : kind(ZERO), location(0,0,0,0,0,0), text() {}

	void clear()
	{
		text.clear();
		kind = ZERO;
	}

	Token(const Token& tok)
		: kind(tok.kind), location(tok.location), text(tok.text) {}

	Token& operator=(const Token& rhs)
	{
		if (&rhs != this)
		{
			kind = rhs.kind;
			location = rhs.location;
			text = rhs.text;
		}
		return *this;
	}

	void swap(Token& rhs)
	{
		Kind k = rhs.kind;
		rhs.kind = kind;
		kind = k;
		SourceLocation loc = rhs.location;
		rhs.location = location;
		location = loc;
		text.swap(rhs.text);
	}
};

} // namespace Soda

std::ostream& operator<<(std::ostream& stream, const std::u32string& str);
std::ostream& operator<<(std::ostream& stream, Soda::Token::Kind kind);

namespace std {
	std::string to_string(Soda::Token::Kind kind);
}

#endif // SODA_TOKEN_H
