#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/parseerror.h>
#include <soda/sema.h>
#include <soda/debugvisitor.h>
#include <cassert>
#include <fstream>

using namespace Soda;

int main()
{
	TU root("test.soda");
	std::ifstream stream("test.soda");

	try
	{
		parse(root, stream);
	}
	catch (SyntaxError& err)
	{
		format_exception(std::cerr, err);
		return 1;
	}

	try
	{
		Sema sema(root);
		sema.check();
		DebugVisitor printer(std::cout);
		root.accept(printer);
		//std::cin.get(); // pause

	}
	catch (ParseError& err)
	{
		format_exception(std::cerr, err);
		return 2;
	}

	return 0;
}
