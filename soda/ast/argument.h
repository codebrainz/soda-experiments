#ifndef SODA_AST_ARGUMENT_H
#define SODA_AST_ARGUMENT_H

namespace Soda
{

struct Argument : public StmtImpl
{
	Ident name;
	Expr value;

	Argument(Ident&& name, Expr&& value)
		: name(std::move(name)),
		  value(std::move(value)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<Argument";
		location.dump(stream);
		if (name || value)
		{
			if (name)
				stream << " name=\"" << name->name << "\"";
			if (value)
			{
				stream << ">";
				value->dump(stream);
				stream << "</Argument>";
			}
			else
				stream << "/>";
		}
	}
};

} // namespace Soda

#endif // SODA_AST_ARGUMENT_H
