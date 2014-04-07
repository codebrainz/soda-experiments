#include "parser.h"
#include <iostream>
#include <cassert>

using namespace Soda;

int main()
{
	TU tu("<stream>");
	parse(tu, "1+2*3+4/5*(6+7)*8");
	assert(tu.stmts.size() > 0);
	tu.dump(std::cout);
	return 0;
}
