#ifndef SODA_TYPEREFERENCES_H
#define SODA_TYPEREFERENCES_H

#include <soda/ast.h>
#include <vector>

namespace Soda
{

class TypeReferences : public AstVisitor
{
private:
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

	Stmt* find_decl_in_scope(SymbolTable& symtab, TypeIdent& type)
	{
		auto found = symtab.find(type.name);
		if (found == symtab.end())
			return nullptr;
		return found->second;
	}

	Stmt* find_decl(TypeIdent& type)
	{
		if (scope_stack.empty())
			return nullptr;
		for (auto it = scope_stack.rbegin(); it != scope_stack.rend(); ++it)
		{
			SymbolTable *symtab = *it;
			if (symtab)
			{
				Stmt *stmt = find_decl_in_scope(*symtab, type);
				if (stmt)
					return stmt;
			}
		}
		return nullptr;
	}

public:
	TypeReferences(TU& root) : root(root), scope_stack() {}

	bool visit(ClassDef& node)
	{
		begin_scope(node.symbols);
		node.block->accept(*this);
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
		node.block->accept(*this);
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
		Stmt *decl = find_decl(*node.type);
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
};

} // namespace Soda

#endif // SODA_TYPEREFERENCES_H