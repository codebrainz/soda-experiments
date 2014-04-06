#include "lexer.h"
#include <sstream>
#include <cassert>
#include <iostream>

using namespace Soda;

#define CHECK(inp_text, tok, str_check)      \
	do {                                     \
		std::string inp_text_(inp_text);     \
		inp_text_+=" ";                      \
		ss << inp_text_;                     \
		k = lex.next();                      \
		assert(k == Token::tok);             \
		assert(lex.token.text == str_check); \
	} while (0)

int main()
{
	std::stringstream ss;
	Token::Kind k;

	ss << " "; // prime input stream;

	Lexer lex(ss);

	CHECK("int ", IDENT, U"int");
	CHECK("double ", IDENT, U"double");
	CHECK("0x00ff ", HEX_ICONST, U"00ff");
	CHECK("0b1001 ", BIN_ICONST, U"1001");
	CHECK("0755 ", OCT_ICONST, U"0755");
	CHECK("123.456 ", FCONST, U"123.456");
	CHECK(".456", FCONST, U".456");
	CHECK("123.", FCONST, U"123.");
	CHECK("'a' ", CHAR_ICONST, U"a");
	CHECK("'\\''", CHAR_ICONST, U"\\'");
	CHECK("// some comment\n ", COMMENT, U" some comment");
	CHECK("/* a multi\n comment */", COMMENT, U" a multi\n comment ");

	return 0;
}
