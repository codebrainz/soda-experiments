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

	std::string pos_attrs(Node& node)
	{
		std::stringstream ss;
		ss << " line=\"" << node.location.line.start << "\""
		   << " column=\"" << node.location.column.start << "\"";
		return ss.str();
	}

	bool visit(Alias& node)
	{
		s << indent() << "(alias\n";
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
		s << indent() << "(argument\n";
		indent_level++;
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
		s << indent() << "(binexpr '" << node.op << "'\n";
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
		s << indent() << "(break)";
		return true;
	}

	bool visit(CaseStmt& node)
	{
		if (node.expr)
		{
			s << indent() << "(case\n";
			indent_level++;
			node.expr->accept(*this);
			s << "\n";
		}
		else
		{
			s << indent() << "(default\n";
			indent_level++;
		}
		node.stmt->accept(*this);
		indent_level--;
		s << ")";
		return true;
	}

	bool visit(ClassDef& node)
	{
		s << indent() << "(classdef\n";
		indent_level++;
		node.name->accept(*this);
		s << "\n";
		node.block->accept(*this);
		s << ")";
		indent_level--;
		return true;
	}

	bool visit(CompoundStmt& node)
	{
		s << indent() << "(compoundstmt";
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

	bool visit(FuncDef& node)
	{
		s << indent() << "(funcdef\n";
		indent_level++;
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

	bool visit(IdentImpl& node)
	{
		s << indent() << "(ident '" << node.name << "')";
		return true;
	}

	bool visit(IfStmt& node)
	{
		s << indent() << "(ifstmt\n";
		indent_level++;
		node.if_expr->accept(*this);
		s << "\n";
		node.if_stmt->accept(*this);
		s << "\n";
		node.else_stmt->accept(*this);
		indent_level--;
		return true;
	}

	bool visit(Import& node)
	{
		s << indent() << "(import\n";
		indent_level++;
		node.ident->accept(*this);
		indent_level--;
		s << ")";
		return true;
	}

	bool visit(Integer& node)
	{
		s << indent() << "(integer '" << node.value << "')";
		return true;
	}

	bool visit(Float& node)
	{
		s << indent() << "(float '" << node.value << "')";
		return true;
	}

	bool visit(ReturnStmt& node)
	{
		s << indent() << "(return";
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
		s << indent() << "(strlit '" << node.text << "')";
		return true;
	}

	bool visit(SwitchStmt& node)
	{
		s << indent() << "(switchstmt\n";
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
		s << indent() << "(vardecl\n";
		indent_level++;
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
