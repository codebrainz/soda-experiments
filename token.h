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
};

} // namespace Soda

std::ostream& operator<<(std::ostream& stream, const std::u32string& str);
std::ostream& operator<<(std::ostream& stream, Soda::Token::Kind kind);

namespace std {
	std::string to_string(Soda::Token::Kind kind);
}

#endif // SODA_TOKEN_H
