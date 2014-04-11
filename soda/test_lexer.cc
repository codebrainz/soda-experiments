#include <soda/sodainc.h> // pch
#include <soda/lexer.h>
#include <sstream>
#include <cassert>

using namespace Soda;

#define CHECK(inp_text, tok, str_check)                           \
	do {                                                          \
		std::string inp_text_(inp_text);                          \
		inp_text_+=" ";                                           \
		ss << inp_text_;                                          \
		k = lex.next();                                           \
		/*std::cout << "Kind: " << k << std::endl;*/              \
		/*std::cout << "Text: " << lex.token.text << std::endl;*/ \
		assert(k == Token::tok);                                  \
		assert(lex.token.text == str_check);                      \
	} while (0)

int main()
{
	std::stringstream ss;
	Token::Kind k;
	(void)k; // prevent warning in ndebug mode

	ss << " "; // prime input stream;

	Lexer lex(ss);

	CHECK("int", IDENT, U"int");
	CHECK("double", IDENT, U"double");

	CHECK("0x00ff", HEX_ICONST, U"00ff");
	CHECK("0b1001", BIN_ICONST, U"1001");
	CHECK("0o755", OCT_ICONST, U"755");
	CHECK("0755", DEC_ICONST, U"0755");
	CHECK("123.456", FCONST, U"123.456");
	CHECK(".456", FCONST, U".456");
	CHECK("123.", FCONST, U"123.");
	CHECK("'a'", CHAR_ICONST, U"a");
	CHECK("'\\''", CHAR_ICONST, U"\\'");
	CHECK("\"FOO\"", STR_LIT, U"FOO");
	CHECK("\"\\\"\"", STR_LIT, U"\\\"");

	//CHECK("// some comment\n", COMMENT, U"// some comment");
	//CHECK("/* a multi\n comment */", COMMENT, U" a multi\n comment ");

	CHECK(">>=", RSHIFT_ASSIGN, U">>=");
	CHECK("<<=", LSHIFT_ASSIGN, U"<<=");

	CHECK("+=", ADD_ASSIGN, U"+=");
	CHECK("-=", SUB_ASSIGN, U"-=");
	CHECK("*=", MUL_ASSIGN, U"*=");
	CHECK("/=", DIV_ASSIGN, U"/=");
	CHECK("%=", MOD_ASSIGN, U"%=");
	CHECK("&=", AND_ASSIGN, U"&=");
	CHECK("^=", XOR_ASSIGN, U"^=");
	CHECK("|=", OR_ASSIGN, U"|=");
	CHECK(">>", RSHIFT, U">>");
	CHECK("<<", LSHIFT, U"<<");
	CHECK("&&", LOG_AND, U"&&");
	CHECK("||", LOG_OR, U"||");
	CHECK("++", INC_OP, U"++");
	CHECK("--", DEC_OP, U"--");
	CHECK("->", PTR_OP, U"->");
	CHECK(">=", GE_OP, U">=");
	CHECK("<=", LE_OP, U"<=");
	CHECK("!=", NE_OP, U"!=");
	CHECK("==", EQ_OP, U"==");

	CHECK(">", GT, U">");
	CHECK("<", LT, U"<");
	CHECK("+", PLUS, U"+");
	CHECK("-", MINUS, U"-");
	CHECK("*", MULTIPLY, U"*");
	CHECK("/", DIVIDE, U"/");
	CHECK("%", MODULO, U"%");
	CHECK("&", BOOL_AND, U"&");
	CHECK("^", BOOL_XOR, U"^");
	CHECK("|", BOOL_OR, U"|");
	CHECK("!", NOT, U"!");
	CHECK("=", EQ, U"=");
	CHECK(".", DOT, U".");

	k = lex.next();
	assert(k == Token::END);

	return 0;
}
