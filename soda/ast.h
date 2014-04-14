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
	Node(SourceLocation& location) : location(location) {}
	Node(const SourcePosition& start_pos, const SourcePosition& end_pos)
		: location(start_pos, end_pos) {}
	virtual ~Node() {}
	size_t line() const { return location.line.start; }
	size_t column() const { return location.column.start; }
};

struct ExprImpl : public Node
{
	template< typename... Args >
	ExprImpl(Args... args) : Node(args...) {}
};
typedef std::unique_ptr<ExprImpl> Expr;

struct IdentImpl;
typedef std::unique_ptr<IdentImpl> Ident;

struct StmtImpl : public Node
{
	template< typename... Args >
	StmtImpl(Args... args) : Node(args...) {}
};
typedef std::unique_ptr<StmtImpl> Stmt;

typedef std::vector<Stmt> StmtList;

struct Alias : public StmtImpl
{
	Ident type, alias;
	template< typename... Args >
	Alias(Ident&& type, Ident&& alias, Args... args)
		: StmtImpl(args...), type(std::move(type)), alias(std::move(alias)) {}
	SODA_NODE_VISITABLE
};

struct Argument : public StmtImpl
{
	Ident type, name;
	Expr value;
	template< typename... Args >
	Argument(Ident&& type, Ident&& name, Expr&& value, Args... args)
		: StmtImpl(args...),
		  type(std::move(type)),
		  name(std::move(name)),
		  value(std::move(value)) {}
	SODA_NODE_VISITABLE
};

struct BinOp : public ExprImpl
{
	char32_t op;
	Expr lhs, rhs;
	template< typename... Args >
	BinOp(char32_t op, Expr&& lhs, Expr&& rhs, Args... args)
		: ExprImpl(args...), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
	SODA_NODE_VISITABLE
};

struct BreakStmt : public StmtImpl
{
	template< typename... Args >
	BreakStmt(Args... args) : StmtImpl(args...) {}
	SODA_NODE_VISITABLE
};

struct CaseStmt : public StmtImpl
{
	Expr expr;
	Stmt stmt;
	template< typename... Args >
	CaseStmt(Expr&& expr, Stmt&& stmt, Args... args)
		: StmtImpl(args...), expr(std::move(expr)), stmt(std::move(stmt)) {}
	SODA_NODE_VISITABLE
};

struct ClassDef : public StmtImpl
{
	Ident name;
	Stmt block;
	template< typename... Args >
	ClassDef(Ident&& name, Stmt&& block, Args... args)
		: StmtImpl(args...), name(std::move(name)), block(std::move(block)) {}
	SODA_NODE_VISITABLE
};

struct CompoundStmt : public StmtImpl
{
	StmtList stmts;
	template< typename... Args >
	CompoundStmt(StmtList&& stmts, Args... args)
		: StmtImpl(args...), stmts(std::move(stmts)) {}
	SODA_NODE_VISITABLE
};

struct Float : public ExprImpl
{
	long double value;
	Float(std::u32string valstr, const SourcePosition& spos,
		const SourcePosition& end);
	SODA_NODE_VISITABLE
};

struct FuncDef : public StmtImpl
{
	Ident type, name;
	StmtList args;
	Stmt block;
	template< typename... Args >
	FuncDef(Ident&& type, Ident&& name, StmtList&& args, Stmt&& block, Args... args_)
		: StmtImpl(args_...),
		  type(std::move(type)),
		  name(std::move(name)),
		  args(std::move(args)),
		  block(std::move(block)) {}
	SODA_NODE_VISITABLE
};

struct IdentImpl : public ExprImpl
{
	std::u32string name;
	template< typename... Args >
	IdentImpl(std::u32string name, Args... args)
		: ExprImpl(args...), name(name) {}
	SODA_NODE_VISITABLE
};

struct IfStmt : public StmtImpl
{
	Expr if_expr;
	Stmt if_stmt, else_stmt;
	template< typename... Args >
	IfStmt(Expr&& if_expr, Stmt&& if_stmt, Stmt&& else_stmt, Args... args)
		: StmtImpl(args...),
		  if_expr(std::move(if_expr)),
		  if_stmt(std::move(if_stmt)),
		  else_stmt(std::move(else_stmt)) {}
	SODA_NODE_VISITABLE
};

struct Import : public StmtImpl
{
	Ident ident;
	template< typename... Args >
	Import(Ident&& ident, Args... args)
		: StmtImpl(args...), ident(std::move(ident)) {}
	SODA_NODE_VISITABLE
};

struct Integer : public ExprImpl
{
	unsigned long long value;
	Integer(std::u32string valstr, int base, const SourcePosition& spos,
		const SourcePosition& end);
	SODA_NODE_VISITABLE
};

struct ReturnStmt : public StmtImpl
{
	Expr expr;
	template< typename... Args >
	ReturnStmt(Expr&& expr, Args... args)
		: StmtImpl(args...), expr(std::move(expr)) {}
	SODA_NODE_VISITABLE
};

struct StrLit : public ExprImpl
{
	std::u32string text;
	template< typename... Args >
	StrLit(std::u32string text, Args... args)
		: ExprImpl(args...), text(text) {}
	SODA_NODE_VISITABLE
};

struct SwitchStmt : public StmtImpl
{
	Expr expr;
	StmtList stmts;
	template< typename... Args >
	SwitchStmt(Expr&& expr, StmtList&& stmts, Args... args)
		: StmtImpl(args...), expr(std::move(expr)), stmts(std::move(stmts)) {}
	SODA_NODE_VISITABLE
};

struct TU : public StmtImpl
{
	StmtList stmts;
	std::string fn;
	template< typename... Args >
	TU(std::string fn, Args... args) : StmtImpl(args...), fn(fn) {}
	SODA_NODE_VISITABLE
};

struct VarDecl : public StmtImpl
{
	Ident type, name;
	Expr assign_expr;
#if 0
	template< typename... Args >
	VarDecl(Ident&& name, Expr&& expr, Args... args)
		: StmtImpl(args...),
		  type(nullptr),
		  name(std::move(name)),
		  assign_expr(std::move(expr)) {}
#endif
	template< typename... Args >
	VarDecl(Ident&& type, Ident&& name, Expr&& expr, Args... args)
		: StmtImpl(args...),
		  type(std::move(type)),
		  name(std::move(name)),
		  assign_expr(std::move(expr)) {}
	SODA_NODE_VISITABLE
};

} // namespace Soda

#endif // SODA_AST_H
