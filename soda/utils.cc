#include <soda/sodainc.h>
#include <soda/utils.h>
#include <vector>
#include <utf8/utf8.h>

// TODO: these functions should probably be inline

namespace Soda
{

bool is_newline(char32_t ch)
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

bool is_whitespace(char32_t ch)
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

bool is_upper(char32_t ch)
{
	return (ch >= 'A' && ch <= 'Z');
}

bool is_lower(char32_t ch)
{
	return (ch >= 'a' && ch <= 'z');
}

bool is_alpha(char32_t ch)
{
	return (is_lower(ch) || is_upper(ch));
}

bool is_digit(char32_t ch)
{
	return (ch >= '0' && ch <= '9');
}

bool is_alnum(char32_t ch)
{
	return (is_alpha(ch) || is_digit(ch));
}

bool is_hex(char32_t ch)
{
	return (ch >= '0' && ch <= '9') ||
	       (ch >= 'A' && ch <= 'F') ||
	       (ch >= 'a' && ch <= 'f');
}

bool is_binary(char32_t ch)
{
	return (ch == '0' || ch == '1');
}

bool is_octal(char32_t ch)
{
	return (ch >= '0' && ch <= '7');
}

// FIXME: make this good
std::string utf8_encode(const std::u32string& u32str)
{
	const char32_t *buf = u32str.data();
	size_t len = u32str.size();
	std::vector<unsigned char> utf8result;
	utf8::utf32to8(buf, buf + len, std::back_inserter(utf8result));
	std::string str("");
	str.reserve(utf8result.size());
	for (auto &ch : utf8result)
		str += ch;
	return str;
}

} // namespace Soda
