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
#include <unordered_map>
#include <cassert>

#define SODA_NODE_VISITABLE                        \
	public:                                        \
		virtual bool accept(AstVisitor& visitor) { \
			return visitor.visit(*this);           \
		}

namespace Soda
{

enum class AccessModifier
{
	DEFAULT,
	PRIVATE,
	PROTECTED,
	PUBLIC,
	INTERNAL,
};

enum class TypeQualifier
{
	NONE=0,
	CONST=(1<<0),
};

enum class StorageClassSpecifier
{
	NONE,
	STATIC,
};

struct Node : public AstVisitable
{
	Node *parent;
	SourceLocation location;
	Node() {}
	Node(SourceLocation& location) : location(location) {}
	Node(const SourcePosition& start_pos, const SourcePosition& end_pos,
	     Node *parent=nullptr)
		: parent(parent), location(start_pos, end_pos) {}
	virtual ~Node() {}
	size_t line() const { return location.line.start; }
	size_t column() const { return location.column.start; }
};

struct Stmt;

struct Expr : public Node
{
	template< typename... Args >
	Expr(Args... args) : Node(args...) {}
};
typedef std::unique_ptr<Expr> ExprPtr;
typedef std::vector<ExprPtr> ExprList;

struct Ident;
typedef std::unique_ptr<Ident> IdentPtr;
typedef std::vector<IdentPtr> IdentList;

struct TypeIdent;
typedef std::unique_ptr<TypeIdent> TypeIdentPtr;

struct Stmt : public Node
{
	template< typename... Args >
	Stmt(Args... args) : Node(args...) {}
};
typedef std::unique_ptr<Stmt> StmtPtr;
typedef std::vector<StmtPtr> StmtList;

typedef std::unordered_map<std::u32string, Stmt*> SymbolTable;

struct Block : public Stmt
{
	StmtPtr block;
	template< typename... Args >
	Block(StmtPtr&& block, Args... args)
		: Stmt(args...), block(std::move(block)) {}
};

struct Alias : public Stmt
{
	IdentPtr type;
	TypeIdentPtr alias;
	template< typename... Args >
	Alias(IdentPtr&& type, TypeIdentPtr&& alias, Args... args)
		: Stmt(args...), type(std::move(type)), alias(std::move(alias)) {}
	SODA_NODE_VISITABLE
};

struct Argument : public Stmt
{
	TypeIdentPtr type;
	IdentPtr name;
	ExprPtr value;
	template< typename... Args >
	Argument(TypeIdentPtr&& type, IdentPtr&& name, ExprPtr&& value, Args... args)
		: Stmt(args...),
		  type(std::move(type)),
		  name(std::move(name)),
		  value(std::move(value)) {}
	SODA_NODE_VISITABLE
};

struct BinOp : public Expr
{
	char32_t op;
	ExprPtr lhs, rhs;
	template< typename... Args >
	BinOp(char32_t op, ExprPtr&& lhs, ExprPtr&& rhs, Args... args)
		: Expr(args...), op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
	SODA_NODE_VISITABLE
};

struct BreakStmt : public Stmt
{
	template< typename... Args >
	BreakStmt(Args... args) : Stmt(args...) {}
	SODA_NODE_VISITABLE
};

struct CallExpr : public Expr
{
	IdentPtr ident;
	ExprList args;
	template< typename... Args >
	CallExpr(IdentPtr&& ident, ExprList&& args, Args... args_)
		: Expr(args_...), ident(std::move(ident)), args(std::move(args)) {}
	SODA_NODE_VISITABLE
};

struct CaseStmt : public Stmt
{
	ExprPtr expr; // expression between ( and )
	StmtPtr stmt; // statement body of the case block

	template< typename... Args >
	CaseStmt(ExprPtr&& expr, StmtPtr&& stmt, Args... args)
		: Stmt(args...),
		  expr(std::move(expr)),
		  stmt(std::move(stmt)) {}

	SODA_NODE_VISITABLE
};

struct CCodeParam : public Stmt
{
	std::u32string name, value;
	template< typename... Args >
	CCodeParam(std::u32string&& name, std::u32string&& value, Args... args)
		: Stmt(args...),
		  name(std::move(name)),
		  value(std::move(value)) {}

	SODA_NODE_VISITABLE
};

typedef std::unique_ptr<CCodeParam> CCodeParamPtr;
typedef std::vector<CCodeParamPtr> CCodeParamList;

struct CCode : public Stmt
{
	CCodeParamList params;
	template< typename... Args >
	CCode(CCodeParamList&& params, Args... args)
		: Stmt(args...), params(std::move(params)) {}

	SODA_NODE_VISITABLE
};

typedef std::unique_ptr<CCode> CCodePtr;

struct ClassDef : public Stmt
{
	IdentPtr name;       // name of the class
	ExprList bases;      // list of base class names
	StmtList stmts;      // list of statements between { and }
	SymbolTable symbols; // names bound between { and }

	template< typename... Args >
	ClassDef(IdentPtr&& name, ExprList&& bases, StmtList&& stmts, Args... args)
		: Stmt(args...),
		  name(std::move(name)),
		  bases(std::move(bases)),
		  stmts(std::move(stmts)) {}

	SODA_NODE_VISITABLE
};

struct CompoundStmt : public Stmt
{
	StmtList stmts;      // statements between { and }
	SymbolTable symbols; // names bound between { and }
	template< typename... Args >
	CompoundStmt(StmtList&& stmts, Args... args)
		: Stmt(args...), stmts(std::move(stmts)) {}
	SODA_NODE_VISITABLE
};

struct Delegate : public Stmt
{
	TypeIdentPtr type;   // return type
	IdentPtr name;       // delegate name
	StmtList args;       // arguments list
	SymbolTable symbols; // names bound in delegate scope (ie. args only)

	template< typename... Args >
	Delegate(TypeIdentPtr&& type, IdentPtr&& name, StmtList&& args, Args... args_)
		: Stmt(args_...),
		  type(std::move(type)),
		  name(std::move(name)),
		  args(std::move(args)) {}

	SODA_NODE_VISITABLE
};

struct EmptyStmt : public Stmt
{
	template< typename... Args >
	EmptyStmt(Args... args) : Stmt(args...) {}
	SODA_NODE_VISITABLE
};

struct ExprStmt : public Stmt
{
	ExprPtr expr;
	template< typename... Args >
	ExprStmt(ExprPtr&& expr, Args... args)
		: Stmt(args...), expr(std::move(expr)) {}
	SODA_NODE_VISITABLE
};

struct Float : public Expr
{
	long double value;
	Float(std::u32string valstr, const SourcePosition& spos,
		const SourcePosition& end);
	SODA_NODE_VISITABLE
};

struct FuncDecl : public Stmt
{
	TypeIdentPtr type;
	IdentPtr name;
	StmtList args;
	CCodePtr ccode;

	template< typename... Args >
	FuncDecl(TypeIdentPtr&& type, IdentPtr&& name, StmtList&& args, Args... args_)
		: Stmt(args_...),
		  type(std::move(type)),
		  name(std::move(name)),
		  args(std::move(args)) {}

	SODA_NODE_VISITABLE
};

struct FuncDef : public Stmt
{
	AccessModifier access;         // public, private, etc.
	StorageClassSpecifier storage; // static, etc.
	TypeIdentPtr type;             // return type
	IdentPtr name;                 // function name
	StmtList args, stmts;          // arguments list
	SymbolTable symbols;           // names bound in this function's scope

	template< typename... Args >
	FuncDef(AccessModifier access,
	        StorageClassSpecifier storage,
	        TypeIdentPtr&& type,
	        IdentPtr&& name,
	        StmtList&& args,
	        StmtList&& stmts,
	        Args... args_)
		: Stmt(args_...),
		  access(access),
		  storage(storage),
		  type(std::move(type)),
		  name(std::move(name)),
		  args(std::move(args)),
		  stmts(std::move(stmts)) {}

	SODA_NODE_VISITABLE
};

struct Ident : public Expr
{
	std::u32string name;
	Stmt *decl;
	template< typename... Args >
	Ident(std::u32string name, Args... args)
		: Expr(args...), name(name), decl(nullptr) {}
	SODA_NODE_VISITABLE
};

struct IfStmt : public Stmt
{
	ExprPtr if_expr;
	StmtPtr if_stmt, else_stmt;
	template< typename... Args >
	IfStmt(ExprPtr&& if_expr, StmtPtr&& if_stmt, StmtPtr&& else_stmt, Args... args)
		: Stmt(args...),
		  if_expr(std::move(if_expr)),
		  if_stmt(std::move(if_stmt)),
		  else_stmt(std::move(else_stmt)) {}
	SODA_NODE_VISITABLE
};

struct Import : public Stmt
{
	IdentPtr ident;
	template< typename... Args >
	Import(IdentPtr&& ident, Args... args)
		: Stmt(args...), ident(std::move(ident)) {}
	SODA_NODE_VISITABLE
};

struct Integer : public Expr
{
	unsigned long long value;
	Integer(std::u32string valstr, int base, const SourcePosition& spos,
		const SourcePosition& end);
	SODA_NODE_VISITABLE
};

struct Namespace : public Stmt
{
	IdentPtr name;
	StmtList stmts;
	SymbolTable symbols;
	template< typename... Args >
	Namespace(IdentPtr&& name, StmtList&& stmts, Args... args)
		: Stmt(args...),
		  name(std::move(name)),
		  stmts(std::move(stmts)) {}
	SODA_NODE_VISITABLE
};

struct ReturnStmt : public Stmt
{
	ExprPtr expr;
	template< typename... Args >
	ReturnStmt(ExprPtr&& expr, Args... args)
		: Stmt(args...), expr(std::move(expr)) {}
	SODA_NODE_VISITABLE
};

struct StrLit : public Expr
{
	std::u32string text;
	template< typename... Args >
	StrLit(std::u32string text, Args... args)
		: Expr(args...), text(text) {}
	SODA_NODE_VISITABLE
};

struct SwitchStmt : public Stmt
{
	ExprPtr expr;
	StmtList stmts;
	SymbolTable symbols;
	template< typename... Args >
	SwitchStmt(ExprPtr&& expr, StmtList&& stmts, Args... args)
		: Stmt(args...), expr(std::move(expr)), stmts(std::move(stmts)) {}
	SODA_NODE_VISITABLE
};

struct TypeIdent : public Stmt
{
	std::u32string name;
	bool is_const;
	Stmt* decl;
	template< typename... Args >
	TypeIdent(std::u32string name, bool is_const, Args... args)
		: Stmt(args...), name(name), is_const(is_const), decl(nullptr) {}
	SODA_NODE_VISITABLE
};

struct TU : public Stmt
{
	StmtList stmts;
	SymbolTable symbols;
	std::string fn;
	template< typename... Args >
	TU(std::string fn, Args... args) : Stmt(args...), fn(fn) {}
	SODA_NODE_VISITABLE
};

struct VarDecl : public Stmt
{
	AccessModifier access;
	StorageClassSpecifier storage;
	TypeIdentPtr type;
	IdentPtr name;
	ExprPtr assign_expr;
	template< typename... Args >
	VarDecl(AccessModifier access,
	        StorageClassSpecifier storage,
	        TypeIdentPtr&& type,
	        IdentPtr&& name,
	        ExprPtr&& expr,
	        Args... args)
		: Stmt(args...),
		  access(access),
		  storage(storage),
		  type(std::move(type)),
		  name(std::move(name)),
		  assign_expr(std::move(expr)) {}
	SODA_NODE_VISITABLE
};

} // namespace Soda

#endif // SODA_AST_H
