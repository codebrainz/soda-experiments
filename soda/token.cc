#include <soda/sodainc.h> // pch
#include <soda/token.h>
#include <ostream>
#include <sstream>
#include <utf8/utf8.h>

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
			TS_CASE(MULTIPLY);
			TS_CASE(DIVIDE);
			TS_CASE(MODULO);
			TS_CASE(BOOL_AND);
			TS_CASE(BOOL_XOR);
			TS_CASE(BOOL_OR);
			TS_CASE(NOT);
			TS_CASE(DOT);
			TS_CASE(TILDE);
			TS_CASE(QUESTION);
			TS_CASE(COLON);
			TS_CASE(SEMICOLON);
			TS_CASE(COMMA);
			TS_CASE(LPAREN);
			TS_CASE(RPAREN);
			TS_CASE(LBRACKET);
			TS_CASE(RBRACKET);
			TS_CASE(LBRACE);
			TS_CASE(RBRACE);
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
			TS_CASE(MUL_ASSIGN);
			TS_CASE(DIV_ASSIGN);
			TS_CASE(MOD_ASSIGN);
			TS_CASE(AND_ASSIGN);
			TS_CASE(XOR_ASSIGN);
			TS_CASE(OR_ASSIGN);
			TS_CASE(LOG_AND);
			TS_CASE(LOG_OR);
			TS_CASE(PTR_OP);
			TS_CASE(LE_OP);
			TS_CASE(GE_OP);
			TS_CASE(NE_OP);
			TS_CASE(EQ_OP);
			TS_CASE(CONST);
			TS_CASE(STATIC);
			TS_CASE(PUBLIC);
			TS_CASE(PRIVATE);
			TS_CASE(PROTECTED);
			TS_CASE(INTERNAL);
			TS_CASE(STRUCT);
			TS_CASE(ENUM);
			TS_CASE(UNION);
			TS_CASE(ALIAS);
			TS_CASE(VAR);
			TS_CASE(FUN);
			TS_CASE(RETURN);
			TS_CASE(IMPORT);
			TS_CASE(FROM);
			TS_CASE(CLASS);
			TS_CASE(IF);
			TS_CASE(ELIF);
			TS_CASE(ELSE);
			TS_CASE(SWITCH);
			TS_CASE(CASE);
			TS_CASE(DEFAULT);
			TS_CASE(BREAK);
			TS_CASE(VOID);
			TS_CASE(NAMESPACE);
			TS_CASE(DELEGATE);
			TS_CASE(CCODE);
		}
		std::stringstream ss;
		ss << "Unknown Token (" << (int) kind << ")";
		return ss.str();
	}
#undef TS_CASE
}
