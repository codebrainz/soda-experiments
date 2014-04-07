#ifndef SODA_AST_H
#define SODA_AST_H

#include "token.h"
#include <string>
#include <memory>
#include <vector>
#include <ostream>

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
	virtual void dump(std::ostream& stream)
	{
		stream << "<Ident name=\"" << "u32encoded" << "\"/>";
	}
};

struct String : public Expr
{
	std::u32string value;
	String(std::u32string value) : value(value) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<String>" << "u32encoded" << "\"/>";
	}
};

typedef std::shared_ptr<Expr> ExprPtr;
typedef std::shared_ptr<Stmt> StmtPtr;

typedef std::vector<ExprPtr> ExprList;
typedef std::vector<StmtPtr> StmtList;

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
	Ident type, name;
	VarDecl(std::u32string type, std::u32string name)
		: type(type), name(name) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<VarDecl>";
		type.dump(stream);
		name.dump(stream);
		stream << "</VarDecl>";
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

} // namespace Soda

#endif // SODA_AST_H
