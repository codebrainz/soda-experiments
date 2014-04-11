#ifndef SODA_AST_CLASSDEF_H
#define SODA_AST_CLASSDEF_H

namespace Soda
{

struct ClassDef : public StmtImpl
{
	Ident name;
	StmtList stmts;

	ClassDef(Ident&& name, StmtList&& stmts)
		: name(std::move(name)),
		  stmts(std::move(stmts)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<ClassDef";
		location.dump(stream);
		if (name)
			stream << " name=\"" << name->name << "\"";
		if (stmts.size() > 0)
		{
			stream << ">";
			for (auto &stmt : stmts)
				stmt->dump(stream);
			stream << "</ClassDef>";
		}
		else
			stream << "/>";
	}
};

} // namespace Soda

#endif // SODA_AST_CLASSDEF_H
