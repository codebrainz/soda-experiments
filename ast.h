#ifndef SODA_AST_H
#define SODA_AST_H

#include "token.h"
#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>

namespace Soda
{

struct Node
{
	struct Range {
		size_t start, end;
		Range(size_t start=0, size_t end=0) : start(start), end(end) {}
	};

	Range position, line, column;

	Node();
	virtual ~Node() {}
	void tag_start(Token& tok);
	void tag_start(size_t pos, size_t line_, size_t col);
	void tag_end(Token& tok);
	void tag_end(size_t pos, size_t line_, size_t col);

	virtual void dump(std::ostream& stream) = 0;
};

struct Expr : public Node {};
struct Stmt : public Node {};

struct Integer : public Expr
{
	unsigned long long value;
	Integer(std::u32string valstr, int base=10);
	virtual void dump(std::ostream& stream)
	{
		stream << "<Integer value=\"" << value << "\"/>";
	}
};

struct Float : public Expr
{
	long double value;
	Float(std::u32string valstr);
	virtual void dump(std::ostream& stream)
	{
		stream << "<Float value=\"" << value << "\"/>";
	}
};

struct Ident : public Expr
{
	std::u32string name;
	Ident(std::u32string name) : name(name) {}
	virtual void dump(std::ostream& stream);
};

struct String : public Expr
{
	std::u32string value;
	String(std::u32string value) : value(value) {}
	virtual void dump(std::ostream& stream);
};

typedef std::shared_ptr<Expr> ExprPtr;
typedef std::shared_ptr<Stmt> StmtPtr;

typedef std::vector<ExprPtr> ExprList;
typedef std::vector<StmtPtr> StmtList;

typedef std::shared_ptr<Ident> IdentPtr;
typedef std::vector<IdentPtr> IdentList;

struct Block : public Expr
{
	StmtList stmts;
	virtual void dump(std::ostream& stream)
	{
		stream << "<Block>";
		for (auto &stmt : stmts)
			stmt->dump(stream);
		stream << "</Block>";
	}
};

struct TU : public Block
{
	std::string fn;
	TU(std::string fn) : fn(fn) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<TU href=\"" << fn << "\">";
		for (auto &stmt : stmts)
			stmt->dump(stream);
		stream << "</TU>";
	}
};

struct VarDecl : public Stmt
{
	IdentPtr type, name;
	ExprPtr assign;
	VarDecl(Ident *type, Ident *name, Expr *assign=nullptr)
		: type(type), name(name), assign(assign) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<VarDecl>";
		//type->dump(stream);
		name->dump(stream);
		if (assign)
		{
			stream << "<AssignExpr>";
			assign->dump(stream);
			stream << "</AssignExpr>";
		}
		stream << "</VarDecl>";
	}
};

struct StructDecl : public Stmt
{
	IdentPtr name;
	StmtList members;
	StructDecl(Ident *name, const StmtList& members)
		: name(name), members(members) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<StructDecl>";
		name->dump(stream);
		if (members.size() > 0)
		{
			stream << "<Members>";
			for (auto &stmt : members)
				stmt->dump(stream);
			stream << "</Members>";
		}
		stream << "</StructDecl>";
	}
};

struct Argument : public Stmt
{
	IdentPtr name;
	ExprPtr value;
	Argument(Ident *name, Expr *value) : name(name), value(value) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<Argument name=\"" << name->name << "\"";
		if (value)
		{
			stream << ">";
			value->dump(stream);
			stream << "</Argument>";
		}
		else
			stream << "/>";
	}
};

struct StrLit : public Expr
{
	std::u32string text;
	StrLit(std::u32string text) : text(text) {}
	virtual void dump(std::ostream& stream)
	{
		if (!text.empty())
			stream << "<StrLit>" << text << "</StrLit>";
		else
			stream << "<StrLit/>";
	}
};

struct ReturnStmt : public Stmt
{
	ExprPtr expr;
	ReturnStmt(Expr *expr) : expr(expr) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<ReturnStmt";
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

struct FuncDef : public Stmt
{
	IdentPtr type, name;
	StmtList args;
	StmtList stmts;
	FuncDef(Ident *type, Ident *name, const StmtList& args,
	        const StmtList& stmts)
		: type(type), name(name), args(args), stmts(stmts) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<FuncDef>";
		//type->dump(stream);
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

struct ExprStmt : public Stmt
{
	ExprPtr exp;
	ExprStmt(Expr *exp) : exp(exp) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<ExprStmt>";
		exp->dump(stream);
		stream << "</ExprStmt>";
	}
};

struct BinOp : public Expr
{
	char32_t op;
	ExprPtr lhs, rhs;
	BinOp(char32_t op, Expr *lhs, Expr *rhs)
		: op(op), lhs(lhs), rhs(rhs) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<BinOp op=\"" << (char)op << "\">";
		lhs->dump(stream);
		rhs->dump(stream);
		stream << "</BinOp>";
	}
};

struct Alias : public Stmt
{
	IdentPtr type;
	IdentPtr alias;
	Alias(Ident *type, Ident *alias) : type(type), alias(alias) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<Alias>";
		type->dump(stream);
		alias->dump(stream);
		stream << "</Alias>";
	}
};

struct Import : public Stmt
{
	IdentPtr ident;
	Import(Ident *ident) : ident(ident) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<Import name=\"" << ident->name << "\"/>";
	}
};

struct ClassDef : public Stmt
{
	IdentPtr name;
	StmtList stmts;
	ClassDef(Ident *name, const StmtList& stmts) : name(name), stmts(stmts) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<ClassDef name=\"" << name->name << "\"";
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

struct IfStmt : public Stmt
{
	ExprPtr if_expr;
	StmtList if_stmts;
	StmtList elifs; // sequence of IfStmts
	StmtList else_stmts;
	IfStmt(Expr *if_expr, const StmtList& if_stmts,
	       const StmtList& elifs=StmtList(),
	       const StmtList& else_stmts=StmtList())
		: if_expr(if_expr), if_stmts(if_stmts), elifs(elifs),
		  else_stmts(else_stmts) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<IfStmt>";
		if_expr->dump(stream);
		if (if_stmts.size() > 0)
		{
			stream << "<Stmts>";
			for (auto &stmt : if_stmts)
				stmt->dump(stream);
			stream << "</Stmts>";
		}
		if (elifs.size() > 0)
		{
			stream << "<!-- Elifs -->";
			for (auto &elif : elifs)
				elif->dump(stream);
		}
		if (else_stmts.size() > 0)
		{
			stream << "<Else>";
			for (auto &stmt : else_stmts)
				stmt->dump(stream);
			stream << "</Else>";
		}
		stream << "</IfStmt>";
	}
};

} // namespace Soda

#endif // SODA_AST_H
