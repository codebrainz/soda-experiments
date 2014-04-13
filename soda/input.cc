#include <soda/sodainc.h> // pch
#include <soda/input.h>
#include <soda/utils.h>
#include <utf8/utf8.h>

namespace Soda
{

Input::Input(std::istream& stream)
	:  position(0,0,0),
	   last(0),
	   stream(stream),
	   iter(stream.rdbuf()),
	   has_peeked(false),
	   peeked(0)
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

char32_t Input::next()
{
	if (has_peeked)
	{
		has_peeked = false;
		last = peeked;
	}
	else
	{
		if (eof() || stream.peek() == EOF)
			return (last = Input::END);
		last = utf8::next(iter, end);
		// skip BOM, zero-width non-breaking space, or WORD JOINER
		while (last == U'\uFEFF' || last == U'\u2060')
		{
			if (eof() || stream.peek() == EOF)
				return (last = Input::END);
			last = utf8::next(iter, end);
		}
	}

	position.offset++;

	if (is_newline(last))
	{
		position.line++;
		position.column = 0;
		if (last == '\r' && stream.peek() == '\n')
		{
			last = utf8::next(iter, end);
			position.offset++;
		}
	}
	else
		position.column++;

	return last;
}

char32_t Input::peek()
{
	if (has_peeked)
	{
		has_peeked = false;
		return peeked;
	}

	has_peeked = true;
	if (eof() || stream.peek() == EOF)
		peeked = Input::END;
	else
		peeked = utf8::next(iter, end);

	return peeked;
}

} // namespace Soda
