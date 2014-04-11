#ifndef SODA_AST_COMPOUND_STMT_H
#define SODA_AST_COMPOUND_STMT_H

namespace Soda
{

struct CompoundStmt : public StmtImpl
{
	StmtList stmts;
	CompoundStmt(StmtList&& stmts) : stmts(std::move(stmts)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<CompoundStmt>";
		for (auto &stmt : stmts)
			stmt->dump(stream);
		stream << "</CompoundStmt>";
	}
};

} // namespace Soda

#endif // SODA_AST_COMPOUND_STMT_H
