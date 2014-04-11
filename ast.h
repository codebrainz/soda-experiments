#ifndef SODA_AST_H
#define SODA_AST_H

#include "token.h"
#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>
#include <cassert>

namespace Soda
{

struct Node;

struct SourceRange
{
	size_t start, end;
	SourceRange(size_t start=0, size_t end=0) : start(start), end(end) {}
	size_t length() { return (end - start); }
};

struct SourceLocation
{
	SourceRange position, line, column;
	SourceLocation() : position(0,0), line(0,0), column(0,0) {}
	SourceLocation(Token& token)
		: position(token.position.start, token.position.end),
		  line(token.line.start, token.line.end),
		  column(token.column.start, token.column.end) {}
	SourceLocation(SourceRange position, SourceRange line, SourceRange column)
		: position(position), line(line), column(column) {}
	void dump(std::ostream& stream)
	{
		stream /*<< " position=\"" << position.start << "," << position.end << "\""*/
		       << " line=\""     << line.start     << "," << line.end << "\""
		       << " column=\""   << column.start   << "," << column.end << "\"";
	}
};

struct Node
{
	SourceLocation location;
	Node() {}
	virtual ~Node() {}
	virtual void dump(std::ostream& stream) = 0;
};

template< typename T >
class AstNode
{
public:
	AstNode(T* node_ptr) : node(node_ptr) {}
	AstNode() { node = new T; }
	~AstNode() { delete node; }
	T* get() const { return node; }
	void clear() { node = nullptr; }
	T* steal() { T* n = node; node = nullptr; return n; }
	T* operator->() { return node; }
	T& operator*() { return *node; }
	bool operator!() { return (node == nullptr); }
	operator bool() { return (node != nullptr); }
	AstNode& operator=(T* rhs)
	{
		delete node;
		node = rhs;
	}
private:
	T *node;
};

struct Expr : public Node {};
struct Stmt : public Node {};

struct Integer : public Expr
{
	unsigned long long value;
	Integer(std::u32string valstr, int base=10);
	virtual void dump(std::ostream& stream)
	{
		stream << "<Integer value=\"" << value << "\"";
		location.dump(stream);
		stream << "/>";
	}
};

struct Float : public Expr
{
	long double value;
	Float(std::u32string valstr);
	virtual void dump(std::ostream& stream)
	{
		stream << "<Float value=\"" << value << "\"";
		location.dump(stream);
		stream << "/>";
	}
};

struct Ident : public Expr
{
	std::u32string name;
	Ident(std::u32string name=U"") : name(name) {}
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
		stream << "<Block";
		location.dump(stream);
		stream << ">";
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
		stream << "<TU href=\"" << fn << "\"";
		location.dump(stream);
		stream << ">";
		for (auto &stmt : stmts)
			stmt->dump(stream);
		stream << "</TU>";
	}
};

struct VarDecl : public Stmt
{
	IdentPtr type, name;
	ExprPtr assign;

	static VarDecl* create(AstNode<Ident>& name, AstNode<Expr>& assign)
	{
		VarDecl* self = new VarDecl(name.get(), assign.get());
		name.clear(); assign.clear();
		return self;
	}

	static VarDecl* create(AstNode<Ident>& name)
	{
		VarDecl* self = new VarDecl(name.get());
		name.clear();
		return self;
	}

	virtual void dump(std::ostream& stream)
	{
		stream << "<VarDecl";
		location.dump(stream);
		stream << ">";
		name->dump(stream);
		if (assign)
		{
			stream << "<AssignExpr>";
			assign->dump(stream);
			stream << "</AssignExpr>";
		}
		stream << "</VarDecl>";
	}

private:
	VarDecl(Ident *name, Expr *assign=nullptr)
		: name(name), assign(assign) {}
};

struct StructDecl : public Stmt
{
	IdentPtr name;
	StmtList members;
	StructDecl(Ident *name, const StmtList& members)
		: name(name), members(members) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<StructDecl";
		location.dump(stream);
		stream << ">";
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

	static Argument* create(AstNode<Ident>& name, AstNode<Expr>& value)
	{
		Argument* self = new Argument(name.get(), value.get());
		name.clear(); value.clear();
		return self;
	}

	static Argument* create(AstNode<Ident>& name)
	{
		Argument* self = new Argument(name.get());
		name.clear();
		return self;
	}

	virtual void dump(std::ostream& stream)
	{
		stream << "<Argument";
		location.dump(stream);
		if (name || value)
		{
			if (name)
				stream << " name=\"" << name->name << "\"";
			if (value)
			{
				stream << ">";
				value->dump(stream);
				stream << "</Argument>";
			}
			else
				stream << "/>";
		}
	}

private:
	Argument(Ident *name, Expr *value=nullptr) : name(name), value(value) {}
};

struct StrLit : public Expr
{
	std::u32string text;
	StrLit(std::u32string text) : text(text) {}
	virtual void dump(std::ostream& stream)
	{
		if (!text.empty())
		{
			stream << "<StrLit";
			location.dump(stream);
			stream << ">" << text << "</StrLit>";
		}
		else
		{
			stream << "<StrLit";
			location.dump(stream);
			stream << "/>";
		}
	}
};

struct ReturnStmt : public Stmt
{
	ExprPtr expr;

	static ReturnStmt* create(AstNode<Expr>& expr)
	{
		ReturnStmt* self = new ReturnStmt(expr.get());
		expr.clear();
		return self;
	}

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

private:
	ReturnStmt(Expr *expr) : expr(expr) {}
};

struct FuncDef : public Stmt
{
	IdentPtr type, name;
	StmtList args, stmts;

	static FuncDef* create(AstNode<Ident>& name, StmtList& args, StmtList& stmts)
	{
		FuncDef* self = new FuncDef(name.get());
		name.clear();
		self->args.swap(args);
		self->stmts.swap(stmts);
		return self;
	}

	virtual void dump(std::ostream& stream)
	{
		stream << "<FuncDef";
		location.dump(stream);
		stream << ">";
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

private:
	FuncDef(Ident *name) : name(name) {}
};

struct ExprStmt : public Stmt
{
	ExprPtr exp;
	ExprStmt(Expr *exp) : exp(exp) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<ExprStmt";
		location.dump(stream);
		stream << ">";
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
		stream << "<BinOp op=\"" << (char)op << "\"";
		location.dump(stream);
		stream << ">";
		lhs->dump(stream);
		rhs->dump(stream);
		stream << "</BinOp>";
	}
};

struct Alias : public Stmt
{
	IdentPtr type, alias;

	static Alias* create(AstNode<Ident>& type, AstNode<Ident>& alias)
	{
		Alias* self = new Alias(type.get(), alias.get());
		type.clear(); alias.clear();
		return self;
	}

	virtual void dump(std::ostream& stream)
	{
		stream << "<Alias";
		location.dump(stream);
		if (type || alias)
		{
			stream << ">";
			if (type)
				type->dump(stream);
			if (alias)
				alias->dump(stream);
			stream << "</Alias>";
		}
		else
			stream << "/>";
	}

private:
	Alias(Ident *type, Ident *alias) : type(type), alias(alias) {}
};

struct Import : public Stmt
{
	IdentPtr ident;

	static Import* create(AstNode<Ident>& ident)
	{
		Import *self = new Import(ident.get());
		ident.clear();
		return self;
	}

	virtual void dump(std::ostream& stream)
	{
		stream << "<Import name=\"" << ident->name << "\"";
		location.dump(stream);
		stream << "/>";
	}

private:
	Import(Ident *ident) : ident(ident) {}
};

struct ClassDef : public Stmt
{
	IdentPtr name;
	StmtList stmts;

	static ClassDef* create(AstNode<Ident>& name, StmtList& stmts)
	{
		ClassDef* self = new ClassDef(name.get());
		name.clear();
		self->stmts.swap(stmts);
		return self;
	}

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

private:
	ClassDef(Ident *name) : name(name) {}
};

struct IfStmt : public Stmt
{
	ExprPtr if_expr;
	StmtList if_stmts, elseif_stmts, else_stmts;

	static IfStmt* create(AstNode<Expr>& if_expr, StmtList& if_stmts,
	                      StmtList& elseif_stmts, StmtList& else_stmts)
	{
		IfStmt* self = new IfStmt(if_expr.get());
		if_expr.clear();
		self->if_stmts.swap(if_stmts);
		self->elseif_stmts.swap(elseif_stmts);
		self->else_stmts.swap(else_stmts);
		return self;
	}

	static IfStmt* create(AstNode<Expr>& expr, StmtList& stmts)
	{
		IfStmt* self = new IfStmt(expr.get());
		expr.clear();
		self->if_stmts.swap(stmts);
		return self;
	}

	virtual void dump(std::ostream& stream)
	{
		stream << "<IfStmt";
		location.dump(stream);
		stream << ">";
		if_expr->dump(stream);
		if (if_stmts.size() > 0)
		{
			stream << "<Stmts>";
			for (auto &stmt : if_stmts)
				stmt->dump(stream);
			stream << "</Stmts>";
		}
		if (elseif_stmts.size() > 0)
		{
			stream << "<!-- Elifs -->";
			for (auto &stmt : elseif_stmts)
				stmt->dump(stream);
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

private:
	IfStmt(Expr *if_expr) : if_expr(if_expr) {}
};

} // namespace Soda

#endif // SODA_AST_H
