//
// An AST pass that finds declarations for type identifiers and base classes.
//

#ifndef SODA_TYPEREFERENCES_H
#define SODA_TYPEREFERENCES_H

#include <soda/ast.h>
#include <vector>

namespace Soda
{

struct TypeReferences : public AstVisitor
{
	typedef std::vector<SymbolTable*> ScopeStack;
	TU& root;
	ScopeStack scope_stack;

	void begin_scope(SymbolTable& symtab)
	{
		scope_stack.push_back(&symtab);
	}

	void end_scope()
	{
		scope_stack.pop_back();
	}

	Stmt* find_decl_in_scope(SymbolTable& symtab, const std::u32string& name)
	{
		auto found = symtab.find(name);
		if (found == symtab.end())
			return nullptr;
		return found->second;
	}

	Stmt* find_decl(const std::u32string& name)
	{
		if (scope_stack.empty())
			return nullptr;
		for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
		{
			SymbolTable *symtab = *it;
			if (symtab)
			{
				Stmt *stmt = find_decl_in_scope(*symtab, name);
				if (stmt)
					return stmt;
			}
		}
		return nullptr;
	}

	TypeReferences(TU& root) : root(root), scope_stack() {}

//////////////////////////////////////////////////////////////////////////////

	bool visit(Alias& node)
	{
		Stmt *decl = find_decl(node.alias->name);
		if (!decl)
		{
			std::stringstream ss;
			ss << "unknown type name `" << node.alias->name << "'";
			throw ParseError("parse error", root.fn, node.alias->location, ss.str());
		}
		else
			node.alias->decl = decl;
		return true;
	}

	bool visit(Argument& node)
	{
		Stmt *decl = find_decl(node.type->name);
		if (!decl)
		{
			std::stringstream ss;
			ss << "unknown type name `" << node.type->name << "'";
			throw ParseError("parse error", root.fn, node.type->location, ss.str());
		}
		else
			node.type->decl = decl;
		return true;
	}

	bool visit(ClassDef& node)
	{
		for (auto &base_expr : node.bases)
		{
			Ident* ident = dynamic_cast<Ident*>(base_expr.get());
			if (ident)
			{
				Stmt *decl = find_decl(ident->name);
				if (!decl)
				{
					std::stringstream ss;
					ss << "unknown type name `" << ident->name << "'";
					throw ParseError("parse error", root.fn, base_expr->location, ss.str());
				}
				else
					ident->decl = decl;
			}
		}
		begin_scope(node.symbols);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope();
		return true;
	}

	bool visit(CompoundStmt& node)
	{
		begin_scope(node.symbols);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope();
		return true;
	}

	bool visit(FuncDef& node)
	{
		begin_scope(node.symbols);
		for (auto &arg : node.args)
			arg->accept(*this);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope();
		return true;
	}

	bool visit(Namespace& node)
	{
		begin_scope(node.symbols);
		node.block->accept(*this);
		end_scope();
		return true;
	}

	bool visit(TU& node)
	{
		begin_scope(node.symbols);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		end_scope();
		return true;
	}

	bool visit(VarDecl& node)
	{
		Stmt *decl = find_decl(node.type->name);
		if (!decl)
		{
			std::stringstream ss;
			ss << "unknown type name `" << node.type->name << "'";
			throw ParseError("parse error", root.fn, node.type->location, ss.str());
		}
		else
			node.type->decl = decl;
		return true;
	}

//////////////////////////////////////////////////////////////////////////////
};

} // namespace Soda

#endif // SODA_TYPEREFERENCES_H
