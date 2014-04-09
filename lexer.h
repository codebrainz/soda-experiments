#ifndef SODA_LEXER_H
#define SODA_LEXER_H

#include "input.h"
#include "token.h"
#include <ostream>

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

} // namespace Soda

#endif // SODA_LEXER_H
