//
// An AST pass that adds symbol tables to scope nodes.
//

#ifndef SODA_TYPEANNOTATOR_H
#define SODA_TYPEANNOTATOR_H

#include <soda/ast.h>
#include <soda/parseerror.h>
#include <string>
#include <unordered_map>
#include <stack>
#include <sstream>

namespace Soda
{

struct TypeAnnotator : public AstVisitor
{
	TU& root;

	TypeAnnotator(TU& root) : root(root) {}

	std::stack<SymbolTable> scope_stack;

	void define(const std::u32string& name, Stmt& stmt)
	{
		SymbolTable& symbols = scope_stack.top();
		auto found = symbols.find(name);
		if (found == symbols.end())
			symbols[name] = &stmt;
		else
		{
			std::stringstream ss;
			ss << "multiple definitions of symbol `" << name
			   << "' previous declaration was on line "
			   << found->second->location.line.start + 1
			   << " at column "
			   << found->second->location.column.start;
			throw ParseError("parse error", root.fn,
				SourceLocation(stmt.location), ss.str());
		}
	}

	void begin_scope()
	{
		scope_stack.push(SymbolTable());
	}

	void end_scope(SymbolTable& symbols)
	{
		//symbols.swap(scope_stack.top());
		symbols = scope_stack.top();
		scope_stack.pop();
	}

//////////////////////////////////////////////////////////////////////////////

	// TODO: alias

	bool visit(ClassDef& node)
	{
		define(node.name->name, node);
		begin_scope();
		node.block->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(CompoundStmt& node)
	{
		begin_scope();
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(Delegate& node)
	{
		define(node.name->name, node);
		begin_scope();
		for (auto &args: node.args)
			args->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(FuncDef& node)
	{
		define(node.name->name, node);
		begin_scope();
		for (auto &arg : node.args)
			arg->accept(*this);
		node.block->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(IfStmt& node)
	{
		node.if_stmt->accept(*this);
		if (node.else_stmt)
			node.else_stmt->accept(*this);
		return true;
	}

	bool visit(Namespace& node)
	{
		begin_scope();
		node.block->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(TU& node)
	{
		begin_scope();
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(VarDecl& node)
	{
		define(node.name->name, node);
		return true;
	}

//////////////////////////////////////////////////////////////////////////////
};

} // namespace Soda

#endif // SODA_TYPEANNOTATOR_H
