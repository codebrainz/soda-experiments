#ifndef SODA_STRLIT_H
#define SODA_STRLIT_H

namespace Soda
{

struct StrLit : public ExprImpl
{
	std::u32string text;
	StrLit(std::u32string text) : text(text) {}
	virtual void dump(std::ostream& stream)
	{
		if (!text.empty())
		{
			stream << "<StrLit";
			location.dump(stream);
			stream << ">" << text << "</StrLit>";
		}
		else
		{
			stream << "<StrLit";
			location.dump(stream);
			stream << "/>";
		}
	}
};

} // namespace Soda

#endif // SODA_STRLIT_H
