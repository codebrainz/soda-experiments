#include "sodainc.h" // pch
#include "parser.h"
#include <cassert>

using namespace Soda;

int main()
{
	TU tu("test.soda");
	std::ifstream stream("test.soda");
	try
	{
		parse(tu, stream);
		assert(tu.stmts.size() > 0);
		tu.dump(std::cout);
	}
	catch (SyntaxError& err)
	{
		format_exception(std::cerr, err);
		return 1;
	}
	return 0;
}
