#ifndef SODA_DEBUG_VISITOR_H
#define SODA_DEBUG_VISITOR_H

#include <soda/ast.h>
#include <ostream>

namespace Soda
{

class DebugVisitor : public AstVisitor
{
public:
	DebugVisitor(std::ostream& stream, int indent_width=2)
		: s(stream),
		  indent_level(0),
		  indent_width(indent_width),
		  indent_str(indent_width, ' ')
	{
	}

private:
	std::ostream& s;
	int indent_level, indent_width;
	std::string indent_str;

	std::string indent()
	{
		std::string total_indent;
		for (int i = 0; i < indent_level; i++)
			total_indent += indent_str;
		return total_indent;
	}

	std::string pos(Node& node)
	{
		std::stringstream ss;
		// FIXME: don't munge the numbers here, fix the Input class to
		// store them properly
		ss << "(" << (node.line() + 1) << ","
		           << node.column() - 1 << ")";
		return ss.str();
	}

	bool visit(Alias& node)
	{
		s << indent() << "(alias " << pos(node) << "\n";
		indent_level++;
		node.type->accept(*this);
		s << "\n";
		node.alias->accept(*this);
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(Argument& node)
	{
		s << indent() << "(argument " << pos(node) << "\n";
		indent_level++;
		node.type->accept(*this);
		node.name->accept(*this);
		if (node.value)
		{
			s << "\n";
			node.value->accept(*this);
		}
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(BinOp& node)
	{
		s << indent() << "(binexpr " << pos(node) << " '";
		if (node.op >= 32 && node.op <= 126)
			s << (char) node.op;
		else
			s << (Token::Kind) node.op;
		s << "'\n";
		indent_level++;
		node.lhs->accept(*this);
		s << "\n";
		node.rhs->accept(*this);
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(BreakStmt& node)
	{
		s << indent() << "(break " << pos(node) << ")";
		return true;
	}

	bool visit(CaseStmt& node)
	{
		if (node.expr)
		{
			s << indent() << "(case " << pos(node) << "\n";
			indent_level++;
			node.expr->accept(*this);
			s << "\n";
		}
		else
		{
			s << indent() << "(default " << pos(node) << "\n";
			indent_level++;
		}
		node.stmt->accept(*this);
		indent_level--;
		s << ")";
		return true;
	}

	bool visit(ClassDef& node)
	{
		s << indent() << "(classdef " << pos(node) << "\n";
		indent_level++;
		node.name->accept(*this);
		s << "\n";
		if (!node.bases.empty())
		{
			s << indent() << "(bases\n";
			indent_level++;
			for (size_t i=0; i < node.bases.size(); i++)
			{
				node.bases[i]->accept(*this);
				if (i+1 != node.bases.size())
					s << "\n";
			}
			indent_level--;
			s << ")\n";
		}
		node.block->accept(*this);
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(CompoundStmt& node)
	{
		s << indent() << "(compoundstmt " << pos(node) << "";
		indent_level++;
		if (node.stmts.empty())
			s << " ";
		else
		{
			s << "\n";
			for (size_t i=0; i < node.stmts.size(); i++)
			{
				node.stmts[i]->accept(*this);
				if (i + 1 != node.stmts.size())
					s << "\n";
			}
			s << ")";
		}
		indent_level--;
		return true;
	}

	bool visit(Float& node)
	{
		s << indent() << "(float " << pos(node) << " '" << node.value << "')";
		return true;
	}

	bool visit(FuncDef& node)
	{
		s << indent() << "(funcdef ";
		switch (node.access)
		{
			case AccessModifier::DEFAULT:
				break;
			case AccessModifier::PRIVATE:
				s << "private ";
				break;
			case AccessModifier::PROTECTED:
				s << "protected ";
				break;
			case AccessModifier::PUBLIC:
				s << "public ";
				break;
			case AccessModifier::INTERNAL:
				s << "internal ";
				break;
		}
		if (node.storage == StorageClassSpecifier::STATIC)
			s << "static ";
		s << pos(node) << "\n";
		indent_level++;
		if (node.type)
			node.type->accept(*this);
		node.name->accept(*this);
		if (!node.args.empty())
		{
			s << "\n";
			for (size_t i=0; i < node.args.size(); i++)
			{
				node.args[i]->accept(*this);
				if (i + 1 != node.args.size())
					s << "\n";
			}
		}
		s << "\n";
		node.block->accept(*this);
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(Ident& node)
	{
		s << indent() << "(ident " << pos(node) << " '" << node.name << "')";
		return true;
	}

	bool visit(IfStmt& node)
	{
		s << indent() << "(ifstmt " << pos(node) << "\n";
		indent_level++;
		node.if_expr->accept(*this);
		s << "\n";
		node.if_stmt->accept(*this);
		if (node.else_stmt)
		{
			s << "\n";
			node.else_stmt->accept(*this);
		}
		indent_level--;
		return true;
	}

	bool visit(Import& node)
	{
		s << indent() << "(import " << pos(node) << "\n";
		indent_level++;
		node.ident->accept(*this);
		indent_level--;
		s << ")";
		return true;
	}

	bool visit(Integer& node)
	{
		s << indent() << "(integer " << pos(node) << " '" << node.value << "')";
		return true;
	}

	bool visit(Namespace& node)
	{
		s << indent() << "(namespace " << pos(node) << "\n";
		indent_level++;
		if (node.name)
		{
			node.name->accept(*this);
			s << "\n";
		}
		node.stmt->accept(*this);
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(ReturnStmt& node)
	{
		s << indent() << "(return " << pos(node) << "";
		if (node.expr)
		{
			s << "\n";
			indent_level++;
			node.expr->accept(*this);
			indent_level--;
		}
		s << ")";
		return true;
	}

	bool visit(StrLit& node)
	{
		s << indent() << "(strlit " << pos(node) << " '" << node.text << "')";
		return true;
	}

	bool visit(SwitchStmt& node)
	{
		s << indent() << "(switchstmt " << pos(node) << "\n";
		indent_level++;
		node.expr->accept(*this);
		s << "\n";
		if (!node.stmts.empty())
		{
			for (size_t i=0; i < node.stmts.size(); i++)
			{
				node.stmts[i]->accept(*this);
				if (i + 1 != node.stmts.size())
					s << "\n";
			}
		}
		indent_level--;
		s << ")";
		return true;
	}

	bool visit(TypeIdent& node)
	{
		s << indent() << "(type ";
		if (node.is_const)
			s << "const ";
		s << pos(node) << " '" << node.name << "')\n";
		return true;
	}

	bool visit(TU& node)
	{
		s << indent() << "(tu '" << node.fn << "'\n";
		indent_level++;
		for (size_t i=0; i < node.stmts.size(); i++)
		{
			node.stmts[i]->accept(*this);
			if (i + 1 != node.stmts.size())
				s << "\n";
		}
		s << ")\n";
		indent_level--;
		return true;
	}

	bool visit(VarDecl& node)
	{
		s << indent() << "(vardecl ";
		switch (node.access)
		{
			case AccessModifier::DEFAULT:
				break;
			case AccessModifier::PRIVATE:
				s << "private ";
				break;
			case AccessModifier::PROTECTED:
				s << "protected ";
				break;
			case AccessModifier::PUBLIC:
				s << "public ";
				break;
			case AccessModifier::INTERNAL:
				s << "internal ";
				break;
		}
		if (node.storage == StorageClassSpecifier::STATIC)
			s << "static ";
		s << pos(node) << "\n";
		indent_level++;
		if (node.type)
			node.type->accept(*this);
		node.name->accept(*this);
		if (node.assign_expr)
		{
			s << "\n";
			node.assign_expr->accept(*this);
		}
		s << ")";
		indent_level--;
		return true;
	}
};

} // namespace Soda

#endif // SODA_DEBUG_VISITOR_H
