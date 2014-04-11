#ifndef SODA_AST_VARDECL_H
#define SODA_AST_VARDECL_H

namespace Soda
{

struct VarDecl : public StmtImpl
{
	Ident name;
	Expr assign_expr;

	VarDecl(Ident&& name, Expr&& expr)
		: name(std::move(name)),
		  assign_expr(std::move(assign_expr)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<VarDecl";
		location.dump(stream);
		stream << ">";
		name->dump(stream);
		if (assign_expr)
		{
			stream << "<AssignExpr>";
			assign_expr->dump(stream);
			stream << "</AssignExpr>";
		}
		stream << "</VarDecl>";
	}
};

} // namespace Soda

#endif // SODA_AST_VARDECL_H
