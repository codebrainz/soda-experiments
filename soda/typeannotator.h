#ifndef SODA_TYPEANNOTATOR_H
#define SODA_TYPEANNOTATOR_H

#include <soda/ast.h>

namespace Soda
{

class TypeAnnotator : public AstVisitor
{
public:
	TypeAnnotator(TU& tu) : tu(tu) {}

private:
	ScopeStack scopes;
	TU& tu;

	void enter_scope()
	{
		scopes.push(SymbolTable(tu));
	}

	void leave_scope()
	{
		scopes.pop();
	}

	bool visit(TU& node)
	{
		enter_scope();
		for (auto &stmt : node.stmts)
			stmt.accept(*this);
		leave_scope();
		return true;
	}

	bool visit(VarDecl& node)
	{
		scopes.top().define(node.name->name, node);
		return true;
	}

	bool visit(FuncDef& node)
	{
		scopes.top().define(node.name->name, node);
		enter_scope();
		node.block->accept(*this);
		leave_scope();
		return true;
	}

	bool visit(ClassDef& node)
	{
		scopes.top().define(node.name->name, node);
		enter_scope();
		node.block->accept(*this);
		leave_scope();
		return true;
	}

	bool visit(Namespace& node)
	{
		scopes.top().define(node.name->name, node);
		enter_scope();
		node.stmt->accept(*this);
		leave_scope();
		return true;
	}
};

} // namespace Soda

#endif // SODA_TYPEANNOTATOR_H
