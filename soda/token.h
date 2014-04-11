#ifndef SODA_TOKEN_H
#define SODA_TOKEN_H

#include <limits>
#include <string>

namespace Soda
{

struct Token
{
	typedef char32_t char_type;
	typedef size_t size_type;

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
	};

	struct Range {
		size_type start, end;
		Range(size_type start=0, size_type end=0)
			: start(start), end(end) {}
	};

	Kind kind;
	Range position, line, column;
	std::u32string text;

	Token() : kind(ZERO), position(0,0), line(0,0), column(0,0), text() {}

	void clear()
	{
		text.clear();
		kind = ZERO;
	}

	Token(const Token& tok)
		: kind(tok.kind), position(tok.position), line(tok.line),
		column(tok.column), text(tok.text) {}

	Token& operator=(const Token& rhs)
	{
		if (&rhs != this)
		{
			kind = rhs.kind;
			position = rhs.position;
			line = rhs.line;
			column = rhs.column;
			text = rhs.text;
		}
		return *this;
	}

	void swap(Token& rhs)
	{
		Kind k = rhs.kind; rhs.kind = kind; kind = k;
		Range r = rhs.position; rhs.position = position; position = r;
		r = rhs.line; rhs.line = line; line = r;
		r = rhs.column; rhs.column = column; column = r;
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
