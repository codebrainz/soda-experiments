#include "input.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <vector>
#include <uchar.h>
#include <utf8/utf8.h>

namespace Soda
{

Input::Input(std::istream& stream)
	:  position(0), line(0), column(0), last(0), stream(stream), iter(stream.rdbuf()),
	   has_peeked(false), peeked(0)
{
}

void Input::skip_whitespace()
{
	while (is_whitespace(last))
		last = next();
}

bool Input::eof() const
{
	return stream.eof();
}

Input::char_type Input::next()
{
	if (has_peeked)
	{
		//std::cout << "Have a peeked char ready (" << peeked << ")" << std::endl;
		has_peeked = false;
		return (last = peeked);
	}

	if (eof() || stream.peek() == EOF)
		return (last = Input::END);

	last = utf8::next(iter, end);
	// skip BOM, zero-width non-breaking space, or WORD JOINER
	while (last == 0xFEFF || last == 0x2060)
		last = utf8::next(iter, end);
	position++;

	if (is_newline(last))
	{
		line++;
		column = 0;
		if (last == '\r' && stream.peek() == '\n')
		{
			last = utf8::next(iter, end);
			position++;
		}
	}
	else
		column++;

	return last;
}

Input::char_type Input::peek()
{
	if (has_peeked)
	{
		has_peeked = false;
		return peeked;
	}

	has_peeked = true;
	if (eof())
	{
		std::cout << "Peeked during eof()" << std::endl;
		peeked = Input::END;
	}
	else if (stream.peek() == EOF)
	{
		std::cout << "Peeked when next is eof()" << std::endl;
		peeked = Input::END;
	}
	else
		peeked = static_cast<Input::char_type>(utf8::next(iter, end));

	return peeked;
}

} // namespace Soda
