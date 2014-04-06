#include "lexer.h"
#include <sstream>
#include <cassert>
#include <iostream>

using namespace Soda;

int main()
{
	std::stringstream ss;
	Token::Kind k;

	ss << "int double 0x00ff 0b1001 0755 123.456 .456 123. 0123. 'a' '\\'' "
	      "// some comment\n /* a multi\n comment */";

	Lexer lex(ss);

	k = lex.next();
	assert(k == Token::IDENT);
	assert(lex.token.text == U"int");

	k = lex.next();
	assert(k == Token::IDENT);
	assert(lex.token.text == U"double");

	k = lex.next();
	assert(k == Token::HEX_ICONST);
	assert(lex.token.text == U"00ff");

	k = lex.next();
	assert(k == Token::BIN_ICONST);
	assert(lex.token.text == U"1001");

	k = lex.next();
	assert(k == Token::OCT_ICONST);
	assert(lex.token.text == U"0755");

	k = lex.next();
	assert(k == Token::FCONST);
	assert(lex.token.text == U"123.456");

	k = lex.next();
	assert(k == Token::FCONST);
	assert(lex.token.text == U".456");

	k = lex.next();
	assert(k == Token::FCONST);
	assert(lex.token.text == U"123.");

	k = lex.next();
	assert(k == Token::FCONST);
	assert(lex.token.text == U"0123.");

	k = lex.next();
	assert(k == Token::CHAR_ICONST);
	assert(lex.token.text == U"a");

	k = lex.next();
	assert(k == Token::CHAR_ICONST);
	std::cout << "'" << lex.token.text << "'" << std::endl;
	assert(lex.token.text == U"\\'");

	k = lex.next();
	assert(k == Token::COMMENT);
	assert(lex.token.text == U" some comment");

	k = lex.next();
	assert(k == Token::COMMENT);
	assert(lex.token.text == U" a multi\n comment ");

	return 0;
}
