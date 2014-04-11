#include <soda/sodainc.h> // pch
#include <soda/lexer.h>
#include <iostream>

using namespace Soda;

int main()
{
	Lexer lex(std::cin);
	while (lex.next() != Token::END)
	{
		if (lex.token.kind == Token::ERROR)
		{
			std::cerr << "Error in input, '" << lex.token.text << "' not recognized" << std::endl;
			return 1;
		}
		else
		{
			std::cout << "Token Text ("
					  << lex.token.kind << "): "
					  << lex.token.text << std::endl;
			lex.next();
		}
	}
	return 0;
}
