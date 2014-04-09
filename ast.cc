#include "sodainc.h" // pch
#include "ast.h"
#include "parser.h"
#include <algorithm>
#include <utf8/utf8.h>

namespace Soda
{

Node::Node()
	: position(0, 0), line(0,0), column(0,0) {}

void Node::tag_start(Token& tok)
{
	position.start = tok.position.start;
	line.start = tok.line.start;
	column.start = tok.column.start;
}

void Node::tag_start(size_t pos, size_t line_, size_t col)
{
	position.start = pos;
	line.start = line_;
	column.start = col;
}

void Node::tag_end(Token& tok)
{
	position.end = tok.position.end;
	line.end = tok.line.end;
	column.end = tok.column.end;
}

void Node::tag_end(size_t pos, size_t line_, size_t col)
{
	position.end = pos;
	line.end = line_;
	column.end = col;
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

void Ident::dump(std::ostream& stream)
{
	stream << "<Ident name=\"" << utf8_encode(name) << "\"/>";
}


void String::dump(std::ostream& stream)
{
	stream << "<String>" << utf8_encode(value) << "\"/>";
}

} // namespace Soda
