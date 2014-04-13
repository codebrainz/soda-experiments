#ifndef SODA_AST_H
#define SODA_AST_H

#include <soda/astvisitor.h>
#include <soda/token.h>
#include <soda/sourcelocation.h>
#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>
#include <cassert>

#define SODA_NODE_VISITABLE                        \
	public:                                        \
		virtual bool accept(AstVisitor& visitor) { \
			return visitor.visit(*this);           \
		}

namespace Soda
{

struct Node : public AstVisitable
{
	SourceLocation location;
	Node() {}
	virtual ~Node() {}
	virtual void dump(std::ostream& stream) {}
};

struct ExprImpl : public Node {};
typedef std::unique_ptr<ExprImpl> Expr;
struct IdentImpl;
typedef std::unique_ptr<IdentImpl> Ident;
struct StmtImpl : public Node {};
typedef std::unique_ptr<StmtImpl> Stmt;
typedef std::vector<Stmt> StmtList;

struct Alias : public StmtImpl
{
	Ident type, alias;
	Alias(Ident&& type, Ident&& alias)
		: type(std::move(type)), alias(std::move(alias)) {}
	SODA_NODE_VISITABLE
};

struct Argument : public StmtImpl
{
	Ident name;
	Expr value;
	Argument(Ident&& name, Expr&& value)
		: name(std::move(name)), value(std::move(value)) {}
	SODA_NODE_VISITABLE
};

struct BinOp : public ExprImpl
{
	char32_t op;
	Expr lhs, rhs;
	BinOp(char32_t op, Expr&& lhs, Expr&& rhs)
		: op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
	SODA_NODE_VISITABLE
};

struct BreakStmt : public StmtImpl
{
	virtual bool accept(AstVisitor& visitor)
	{
		return visitor.visit(*this);
	}
};

struct CaseStmt : public StmtImpl
{
	Expr expr;
	Stmt stmt;
	CaseStmt(Expr&& expr, Stmt&& stmt)
		: expr(std::move(expr)), stmt(std::move(stmt)) {}
	virtual bool accept(AstVisitor& visitor)
	{
		return visitor.visit(*this);
	}
};

struct ClassDef : public StmtImpl
{
	Ident name;
	Stmt block;
	ClassDef(Ident&& name, Stmt&& block)
		: name(std::move(name)), block(std::move(block)) {}
	SODA_NODE_VISITABLE
};

struct CompoundStmt : public StmtImpl
{
	StmtList stmts;
	CompoundStmt(StmtList&& stmts) : stmts(std::move(stmts)) {}
	SODA_NODE_VISITABLE
};

struct Float : public ExprImpl
{
	long double value;
	Float(std::u32string valstr);
	SODA_NODE_VISITABLE
};

struct FuncDef : public StmtImpl
{
	Ident name;
	StmtList args;
	Stmt block;
	FuncDef(Ident&& name, StmtList&& args, Stmt&& block)
		: name(std::move(name)), args(std::move(args)), block(std::move(block)) {}
	SODA_NODE_VISITABLE
};

struct IdentImpl : public ExprImpl
{
	std::u32string name;
	IdentImpl(std::u32string name=U"") : name(name) {}
	SODA_NODE_VISITABLE
};

struct IfStmt : public StmtImpl
{
	Expr if_expr;
	Stmt if_stmt, else_stmt;
	IfStmt(Expr&& if_expr, Stmt&& if_stmt, Stmt&& else_stmt)
		: if_expr(std::move(if_expr)),
		  if_stmt(std::move(if_stmt)),
		  else_stmt(std::move(else_stmt)) {}
	SODA_NODE_VISITABLE
};

struct Import : public StmtImpl
{
	Ident ident;
	Import(Ident&& ident) : ident(std::move(ident)) {}
	SODA_NODE_VISITABLE
};

struct Integer : public ExprImpl
{
	unsigned long long value;
	Integer(std::u32string valstr, int base=10);
	SODA_NODE_VISITABLE
};

struct ReturnStmt : public StmtImpl
{
	Expr expr;
	ReturnStmt(Expr&& expr) : expr(std::move(expr)) {}
	SODA_NODE_VISITABLE
};

struct StrLit : public ExprImpl
{
	std::u32string text;
	StrLit(std::u32string text) : text(text) {}
	SODA_NODE_VISITABLE
};

struct SwitchStmt : public StmtImpl
{
	Expr expr;
	StmtList stmts;
	SwitchStmt(Expr&& expr, StmtList&& stmts)
		: expr(std::move(expr)), stmts(std::move(stmts)) {}
	virtual bool accept(AstVisitor& visitor)
	{
		return visitor.visit(*this);
	}
};

struct TU : public StmtImpl
{
	StmtList stmts;
	std::string fn;
	TU(std::string fn) : fn(fn) {}
	SODA_NODE_VISITABLE
};

struct VarDecl : public StmtImpl
{
	Ident name;
	Expr assign_expr;
	VarDecl(Ident&& name, Expr&& expr)
		: name(std::move(name)),
		  assign_expr(std::move(assign_expr)) {}
	SODA_NODE_VISITABLE
};

} // namespace Soda

#endif // SODA_AST_H
