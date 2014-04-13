#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/debugvisitor.h>
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
		DebugVisitor printer(std::cout);
		tu.accept(printer);
	}
	catch (SyntaxError& err)
	{
		format_exception(std::cerr, err);
		return 1;
	}
	return 0;
}
