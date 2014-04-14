#ifndef SODA_LEXER_H
#define SODA_LEXER_H

#include <soda/input.h>
#include <soda/token.h>
#include <istream>
#include <vector>

namespace Soda
{

class Lexer
{
public:
	Token token;
	Lexer(std::istream& stream);
	~Lexer();
	Token::Kind next();
private:
	struct LexImpl;
	LexImpl *impl;
	Lexer(const Lexer&);
	Lexer& operator=(const Lexer&);
};

typedef std::vector<Token> TokenList;
TokenList tokenize(std::istream& stream);

} // namespace Soda

#endif // SODA_LEXER_H
