#ifndef SODA_AST_NUMBER_H
#define SODA_AST_NUMBER_H

namespace Soda
{

struct Number : public ExprImpl
{
};

SODA_NODE_TYPES(Number);

struct Integer : public Number
{
	unsigned long long value;

	Integer(std::u32string valstr, int base=10);

	virtual void dump(std::ostream& stream)
	{
		stream << "<Integer value=\"" << value << "\"";
		location.dump(stream);
		stream << "/>";
	}
};

SODA_NODE_TYPES(Integer);

struct Float : public Number
{
	long double value;

	Float(std::u32string valstr);

	virtual void dump(std::ostream& stream)
	{
		stream << "<Float value=\"" << value << "\"";
		location.dump(stream);
		stream << "/>";
	}
};

SODA_NODE_TYPES(Float);

} // namespace Soda

#endif // SODA_AST_NUMBER_H
