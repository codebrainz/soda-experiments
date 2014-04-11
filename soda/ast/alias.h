#ifndef SODA_AST_ALIAS_H
#define SODA_AST_ALIAS_H

namespace Soda
{

struct Alias : public StmtImpl
{
	Ident type, alias;

	Alias(Ident&& type, Ident&& alias)
		: type(std::move(type)),
		  alias(std::move(alias)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<Alias";
		location.dump(stream);
		if (type || alias)
		{
			stream << ">";
			if (type)
				type->dump(stream);
			if (alias)
				alias->dump(stream);
			stream << "</Alias>";
		}
		else
			stream << "/>";
	}
};

} // namespace Soda

#endif // SODA_AST_ALIAS_H
