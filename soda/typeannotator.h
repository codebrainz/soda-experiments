//
// An AST pass that adds symbol tables to scope nodes.
//
// Specifically, this pass:
//   - Checks for multiple definitions of types or variables
//   - Keeps track of the current namespace name in a stack
//   - Renames nodes with names to use their fully qualified names
//   - Keep a global symbol table with fully qualified names mapped
//     to their related statement (pointers).
//

#ifndef SODA_TYPEANNOTATOR_H
#define SODA_TYPEANNOTATOR_H

#include <soda/ast.h>
#include <soda/parseerror.h>
#include <string>
#include <unordered_map>
#include <stack>
#include <vector>
#include <sstream>

namespace Soda
{

struct TypeAnnotator : public AstVisitor
{
	TU& root;

	TypeAnnotator(TU& root) : root(root) {}

	std::stack<SymbolTable> scope_stack;
	std::vector<std::u32string> name_stack;
	SymbolTable symtab;

	std::u32string prefix()
	{
		std::u32string pfx;
		for (auto it = name_stack.begin(); it != name_stack.end(); ++it)
			{ pfx += *it; pfx += U"."; }
		if (pfx.size() > 0 && pfx[pfx.size()-1] == '.')
			pfx = pfx.substr(0, pfx.size() - 1);
		return pfx;
	}

	std::u32string fq_name(const std::u32string& name)
	{
		std::u32string f(prefix());
		if (f.size() > 0)
			f += U'.';
		f += name;
		return f;
	}

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
			throw ParseError("parse error", root.fn, stmt.location, ss.str());
		}
	}

	void begin_scope(std::u32string name=std::u32string())
	{
		if (!name.empty())
		{
			std::cerr << "+" << name << std::endl;
			name_stack.push_back(std::move(name));
		}
		scope_stack.push(SymbolTable());
	}

	void end_scope(SymbolTable& symbols)
	{
		if (!name_stack.empty())
		{
			std::cerr << "-" << name_stack.back() << std::endl;
			name_stack.pop_back();
		}
		symbols.swap(scope_stack.top());
		scope_stack.pop();
	}

//////////////////////////////////////////////////////////////////////////////

	bool visit(Alias& node)
	{
		define(node.type->name, node);
		node.type->name = fq_name(node.type->name);
		return true;
	}

	bool visit(Argument& node)
	{
		define(node.name->name, node);
		node.name->name = fq_name(node.name->name);
		return true;
	}

	bool visit(ClassDef& node)
	{
		define(node.name->name, node);
		node.name->name = fq_name(node.name->name);
		begin_scope(node.name->name);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
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
		node.name->name = fq_name(node.name->name);
		begin_scope(node.name->name);
		for (auto &args: node.args)
			args->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(FuncDef& node)
	{
		std::u32string tmp(fq_name(node.name->name));
		define(node.name->name, node);
		begin_scope(node.name->name);
		node.name->name = tmp;
		for (auto &arg : node.args)
			arg->accept(*this);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
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
		if (node.name)
		{
			begin_scope(node.name->name);
			//node.name->name = fq_name(node.name->name);
			node.name->name = prefix();
		}
		else
			begin_scope();
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope(node.symbols);
		return true;
	}

	bool visit(SwitchStmt& node)
	{
		begin_scope();
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
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
		node.name->name = fq_name(node.name->name);
		return true;
	}

//////////////////////////////////////////////////////////////////////////////
};

} // namespace Soda

#endif // SODA_TYPEANNOTATOR_H
