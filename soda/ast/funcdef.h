#ifndef SODA_FUNCDEF_H
#define SODA_FUNCDEF_H

namespace Soda
{

struct FuncDef : public StmtImpl
{
	Ident name;
	StmtList args, stmts;

	FuncDef(Ident&& name, StmtList&& args, StmtList&& stmts)
		: name(std::move(name)),
		  args(std::move(args)),
		  stmts(std::move(stmts)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<FuncDef";
		location.dump(stream);
		stream << ">";
		name->dump(stream);
		if (args.size() > 0)
		{
			stream << "<Args>";
			for (auto &stmt : args)
				stmt->dump(stream);
			stream << "</Args>";
		}
		if (stmts.size() > 0)
		{
			stream << "<Stmts>";
			for (auto &stmt : stmts)
				stmt->dump(stream);
			stream << "</Stmts>";
		}
		stream << "</FuncDef>";
	}
};

} // namespace Soda

#endif // SODA_FUNCDEF_H
