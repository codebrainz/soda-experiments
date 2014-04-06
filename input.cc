#include "input.h"
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

static inline bool is_newline(Input::char_type ch)
{ // http://www.unicode.org/standard/reports/tr13/tr13-5.html
	switch (ch)
	{
		case 0x000A: // Line Feed
		case 0x000B: // Vertical Tab
		case 0x000C: // Form Feed
		case 0x000D: // Carriage Return
		case 0x0085: // Next Line
		case 0x2028: // Line Separator
		case 0x2029: // Paragraph Separator
			return true;
		default:
			return false; // Not a new line character
	}
}

static inline bool is_whitespace(Input::char_type ch)
{ // http://en.wikipedia.org/wiki/Whitespace_character#Unicode
	switch (ch)
	{
		case 0x0009: // Horizontal Tab
		case 0x000A: // Line Feed
		case 0x000B: // Vertical Tab
		case 0x000C: // Form Feed
		case 0x000D: // Carriage Return
		case 0x0020: // Space
		case 0x0085: // Next Line
		case 0x00A0: // No-Break Space
		case 0x1680: // Ogham Space Mark
		case 0x2000: // EN Quad
		case 0x2001: // EM Quad
		case 0x2002: // EN Space
		case 0x2003: // EM Space
		case 0x2004: // Three-Per-EM Space
		case 0x2005: // Four-Per-EM Space
		case 0x2006: // Size-Per-EM Space
		case 0x2007: // Figure Space
		case 0x2008: // Punctuation Space
		case 0x2009: // Thin Space
		case 0x200A: // Hair Space
		case 0x2028: // Line Separator
		case 0x2029: // Paragraph Separator
		case 0x202F: // Narrow No-Break Space
		case 0x205F: // Medium Mathematical Space
		case 0x3000: // Ideographic Space
			return true;
		default:
			return false;
	}
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
		has_peeked = false;
		return (last = peeked);
	}

	if (stream.peek() == EOF)
		return (last = Input::END);

	last = static_cast<Input::char_type>(utf8::next(iter, end));
	 // Skip BOM (TODO: zero-width non-breaking space and word joiner?)
	if (position == 0 && last == 0xFEFF)
		last = static_cast<Input::char_type>(utf8::next(iter, end));
	position++;

	if (is_newline(last))
	{
		line++;
		column = 0;
		if (last == '\r' && stream.peek() == '\n')
		{
			last = static_cast<Input::char_type>(utf8::next(iter, end));
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
	if (eof() || stream.peek() == EOF)
		peeked = Input::END;
	else
		peeked = static_cast<Input::char_type>(utf8::next(iter, end));

	return peeked;
}

} // namespace Soda
