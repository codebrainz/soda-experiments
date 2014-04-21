//
// An AST pass that fills in back-pointers in nodes to their parent nodes.
//

#ifndef SODA_PARENTPOINTERS_H
#define SODA_PARENTPOINTERS_H

#include <soda/ast.h>
#include <stack>

namespace Soda
{

class ParentPointers : public AstVisitor
{
	std::stack<Node*> parents;

	void push_parent(Node *node) { parents.push(node); }

	Node *pop_parent()
	{
		Node *node = nullptr;
		if (parents.size() > 0)
		{
			node = parents.top();
			parents.pop();
		}
		return node;
	}

	Node *top_parent()
	{
		Node *node = nullptr;
		if (parents.size() > 0)
			node = parents.top();
		return node;
	}

//////////////////////////////////////////////////////////////////////////////

	bool visit(Alias& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.type->accept(*this);
		node.alias->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(Argument& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.type->accept(*this);
		node.name->accept(*this);
		node.value->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(BinOp& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.lhs->accept(*this);
		node.rhs->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(BreakStmt& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(CallExpr& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.ident->accept(*this);
		for (auto &arg : node.args)
			arg->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(CaseStmt& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		if (node.expr) // eg. nullptr for `default` case
			node.expr->accept(*this);
		node.stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(ClassDef& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.name->accept(*this);
		for (auto &expr : node.bases)
			expr->accept(*this);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(CompoundStmt& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(Delegate& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.type->accept(*this);
		node.name->accept(*this);
		for (auto &arg : node.args)
			arg->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(EmptyStmt& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(ExprStmt& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.expr->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(Float& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(FuncDef& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.type->accept(*this);
		node.name->accept(*this);
		for (auto &arg : node.args)
			arg->accept(*this);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(Ident& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(IfStmt& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.if_expr->accept(*this);
		if (node.if_stmt)
			node.if_stmt->accept(*this);
		if (node.else_stmt)
			node.else_stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(Import& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.ident->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(Integer& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(Namespace& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		if (node.name)
			node.name->accept(*this);
		node.block->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(ReturnStmt& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		if (node.expr)
			node.expr->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(StrLit& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(SwitchStmt& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.expr->accept(*this);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(TypeIdent& node)
	{
		node.parent = top_parent();
		return true;
	}

	bool visit(TU& node)
	{
		node.parent = top_parent(); // ie. nullptr
		push_parent(&node);
		for (auto &stmt : node.stmts)
			stmt->accept(*this);
		pop_parent();
		return true;
	}

	bool visit(VarDecl& node)
	{
		node.parent = top_parent();
		push_parent(&node);
		node.type->accept(*this);
		node.name->accept(*this);
		if (node.assign_expr)
			node.assign_expr->accept(*this);
		pop_parent();
		return true;
	}

//////////////////////////////////////////////////////////////////////////////

};

} // namespace Soda

#endif // SODA_PARENTPOINTERS_H
