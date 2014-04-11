#ifndef SODA_AST_RETURNSTMT_H
#define SODA_AST_RETURNSTMT_H

namespace Soda
{

struct ReturnStmt : public StmtImpl
{
	Expr expr;

	ReturnStmt(Expr&& expr) : expr(std::move(expr)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<ReturnStmt";
		location.dump(stream);
		if (expr)
		{
			stream << ">";
			expr->dump(stream);
			stream << "</ReturnStmt>";
		}
		else
			stream << "/>";
	}
};

} // namespace Soda

#endif // SODA_AST_RETURNSTMT_H
