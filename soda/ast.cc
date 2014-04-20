#include <soda/sodainc.h> // pch
#include <soda/ast.h>
#include <soda/utils.h>
#include <soda/parser.h>
#include <utf8/utf8.h>
#include <algorithm>

namespace Soda
{

Integer::Integer(std::u32string valstr, int base, const SourcePosition& spos,
                 const SourcePosition& end)
	: Expr(spos, end), value(0)
{
	size_t pos=0;
	std::string u8val = utf8_encode(valstr);
	//std::cout << "UTF8 Value: " << u8val << std::endl;
	value = std::stoull(u8val, &pos, base);
	if (pos != u8val.size())
		throw std::invalid_argument("std::stoull");
}

Float::Float(std::u32string valstr, const SourcePosition& spos,
             const SourcePosition& end)
	: Expr(spos, end), value(0.0)
{
	size_t pos=0;
	std::string u8val = utf8_encode(valstr);
	value = std::stold(u8val, &pos);
	if (pos != u8val.size())
		throw std::invalid_argument("std::stold");
}

} // namespace Soda
