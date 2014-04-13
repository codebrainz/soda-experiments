#include <soda/sodainc.h> // pch
#include <soda/input.h>
#include <sstream>
#include <cassert>

using namespace Soda;

struct Test
{
	const char *str;
	size_t len;
	char32_t last;
	size_t pos;
	size_t line;
	size_t col;
};

static const Test tests[] = {
	{ u8"a",       1, U'a',       1,  0,  1 }, // 1. whatever letter
	{ u8" ",       1, U' ',       2,  0,  2 }, // 2. normal space
	{ u8"\t",      1, U'\t',      3,  0,  3 }, // 3. tabs
	{ u8"\0",      1, U'\0',      4,  0,  4 }, // 4. \0 is treated like any other codepoint
	{ u8"\n",      1, U'\n',      5,  1,  0 }, // 5. Line feed
	{ u8"\v",      1, U'\v',      6,  2,  0 }, // 6. Vertical tab
	{ u8"\f",      1, U'\f',      7,  3,  0 }, // 7. Form feed
	{ u8"\r",      1, U'\r',      8,  4,  0 }, // 8. Carriage return
	{ u8"\u0085",  2, U'\u0085',  9,  5,  0 }, // 9. Next line
	{ u8"\u2028",  3, U'\u2028',  10, 6,  0 }, // 10. Line separator
	{ u8"\u2029",  3, U'\u2029',  11, 7,  0 }, // 11. Paragraph separator
	{ u8"\r\n",    2, U'\n',      13, 8,  0 }, // 12. Carriage return and line feed (CRLF)
};

static const size_t n_tests = sizeof(tests) / sizeof(tests[0]);

int main()
{
	std::stringstream ss;

	// Load up the input stream before passing to Input constructor
	//ss.write(u8"\uFEFF", 3);
	//ss.write(u8"\u2060", 3);
	for (size_t i = 0; i < n_tests; i++)
		ss.write(tests[i].str, tests[i].len);

	Input inp(ss);

	// Initial state
	assert(inp.last == 0);
	assert(inp.peek() != Input::END);
	assert(inp.position == SourcePosition(0,0,0));

	// Tests
	for (size_t i = 0; i < n_tests; i++)
	{
		inp.next();
		assert(inp.last == tests[i].last);
		assert(inp.position ==
			SourcePosition(tests[i].pos, tests[i].line, tests[i].col));
	}

	// EOF
	inp.next();
	assert(inp.last == Input::END);
	assert(inp.peek() == Input::END);
	assert(inp.position ==
		SourcePosition(tests[n_tests - 1].pos, tests[n_tests - 1].line,
			tests[n_tests - 1].col));

	return 0;

}
