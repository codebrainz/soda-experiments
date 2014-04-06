#include "token.h"
#include <utf8/utf8.h>
#include <sstream>

std::ostream& operator<<(std::ostream& stream, const std::u32string& str)
{
	std::ostreambuf_iterator<char> start(stream.rdbuf());
	utf8::utf32to8(std::begin(str), std::end(str), start);
	return stream;
}

std::ostream& operator<<(std::ostream& stream, Soda::Token::Kind kind)
{
	stream << std::to_string(kind);
	return stream;
}

namespace std {
#define TS_CASE(T) case Soda::Token::T: return #T
	std::string to_string(Soda::Token::Kind kind)
	{
		switch (kind)
		{
			TS_CASE(END);
			TS_CASE(ERROR);
			TS_CASE(ZERO);
			TS_CASE(LT);
			TS_CASE(GT);
			TS_CASE(EQ);
			TS_CASE(PLUS);
			TS_CASE(MINUS);
			TS_CASE(IDENT);
			TS_CASE(HEX_ICONST);
			TS_CASE(BIN_ICONST);
			TS_CASE(OCT_ICONST);
			TS_CASE(DEC_ICONST);
			TS_CASE(CHAR_ICONST);
			TS_CASE(FCONST);
			TS_CASE(STR_LIT);
			TS_CASE(COMMENT);
			TS_CASE(RSHIFT);
			TS_CASE(RSHIFT_ASSIGN);
			TS_CASE(LSHIFT);
			TS_CASE(LSHIFT_ASSIGN);
			TS_CASE(ADD_ASSIGN);
			TS_CASE(INC_OP);
			TS_CASE(SUB_ASSIGN);
			TS_CASE(DEC_OP);
		}
		std::stringstream ss;
		ss << "Unknown Token (" << (int) kind << ")";
		return ss.str();
	}
#undef TS_CASE
}
