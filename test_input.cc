#include "input.h"
#include <sstream>
#include <cassert>
#include <iostream>

using namespace Soda;

struct Test
{
	const char *str;
	Input::char_type last;
	Input::size_type pos;
	Input::size_type line;
	Input::size_type col;
};

static const Test tests[] = {
	{ "a",  'a',  3, 1, 1 },
	{ "\n", '\n', 4, 2, 0 },
	{ "\f", '\f', 5, 3, 0 },
	{ " ",  ' ',  6, 3, 1 },
	{ "\t", '\t', 7, 3, 2 },
	{ "\r", '\r', 8, 4, 0 },
	// todo: test some unicode stuff
};

static const size_t n_tests = sizeof(tests) / sizeof(tests[0]);

int main()
{
	std::stringstream ss;
	Input inp(ss);

	// Initial state
	assert(inp.last == 0);
	//assert(inp.peek() == Input::END);
	assert(inp.position == 0);
	assert(inp.line == 0);
	assert(inp.column == 0);

	ss << "\r\n"; // ensure taken together
	inp.next();
	assert(inp.last == '\n');  // ensure skips the \r
	assert(inp.position == 2); // position accounts for both chars
	assert(inp.line == 1);     // but only one line is incremented
	assert(inp.column == 0);   // and the column is still reset

	for (size_t i = 0; i < n_tests; i++)
	{
		ss << tests[i].str;
		inp.next();
		assert(inp.last == tests[i].last);
		assert(inp.position == tests[i].pos);
		assert(inp.line == tests[i].line);
		assert(inp.column == tests[i].col);
	}

	// EOF
	inp.next();
	assert(inp.last == Input::END);
	assert(inp.peek() == Input::END);
	assert(inp.position == tests[n_tests - 1].pos);
	assert(inp.line == tests[n_tests - 1].line);
	assert(inp.column == tests[n_tests - 1].col);

	return 0;

}
