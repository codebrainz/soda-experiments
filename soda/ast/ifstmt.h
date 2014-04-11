#ifndef SODA_AST_IFSTMT_H
#define SODA_AST_IFSTMT_H

namespace Soda
{

struct IfStmt : public StmtImpl
{
	Expr if_expr;
	Stmt if_stmt, else_stmt;

	IfStmt(Expr&& if_expr, Stmt&& if_stmt, Stmt&& else_stmt)
		: if_expr(std::move(if_expr)),
		  if_stmt(std::move(if_stmt)),
		  else_stmt(std::move(else_stmt)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<IfStmt";
		location.dump(stream);
		stream << ">";
		if (if_expr)
			if_expr->dump(stream);
		if (if_stmt)
			if_stmt->dump(stream);
		if (else_stmt)
			else_stmt->dump(stream);
		stream << "</IfStmt>";
	}
};

} // namespace Soda

#endif // SODA_AST_IFSTMT_H
