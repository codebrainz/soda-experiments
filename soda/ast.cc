#include <soda/sodainc.h> // pch
#include <soda/ast.h>
#include <soda/parser.h>
#include <utf8/utf8.h>
#include <algorithm>

namespace Soda
{

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

Integer::Integer(std::u32string valstr, int base)
	: value(0)
{
	size_t pos=0;
	std::string u8val = utf8_encode(valstr);
	//std::cout << "UTF8 Value: " << u8val << std::endl;
	value = std::stoull(u8val, &pos, base);
	if (pos != u8val.size())
		throw std::invalid_argument("std::stoull");
}

Float::Float(std::u32string valstr)
	: value(0.0)
{
	size_t pos=0;
	std::string u8val = utf8_encode(valstr);
	value = std::stold(u8val, &pos);
	if (pos != u8val.size())
		throw std::invalid_argument("std::stold");
}

} // namespace Soda
