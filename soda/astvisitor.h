#ifndef SODA_AST_VISITOR_H
#define SODA_AST_VISITOR_H

namespace Soda
{

class Alias;
class Argument;
class BinOp;
class BreakStmt;
class CallExpr;
class CaseStmt;
class ClassDef;
class CompoundStmt;
class Delegate;
class EmptyStmt;
class ExprStmt;
class Float;
class FuncDef;
class Ident;
class IfStmt;
class Import;
class Integer;
class Namespace;
class ReturnStmt;
class StrLit;
class SwitchStmt;
class TU;
class TypeIdent;
class VarDecl;

class AstVisitor
{
public:
	virtual bool visit(Alias&) { return true; }
	virtual bool visit(Argument&) { return true; }
	virtual bool visit(BinOp&) { return true; }
	virtual bool visit(BreakStmt&) { return true; }
	virtual bool visit(CallExpr&) { return true; }
	virtual bool visit(CaseStmt&) { return true; }
	virtual bool visit(ClassDef&) { return true; }
	virtual bool visit(CompoundStmt&) { return true; }
	virtual bool visit(Delegate&) { return true; }
	virtual bool visit(EmptyStmt&) { return true; }
	virtual bool visit(ExprStmt&) { return true; }
	virtual bool visit(Float&) { return true; }
	virtual bool visit(FuncDef&) { return true; }
	virtual bool visit(Ident&) { return true; }
	virtual bool visit(IfStmt&) { return true; }
	virtual bool visit(Import&) { return true; }
	virtual bool visit(Integer&) { return true; }
	virtual bool visit(Namespace&) { return true; }
	virtual bool visit(ReturnStmt&) { return true; }
	virtual bool visit(StrLit&) { return true; }
	virtual bool visit(SwitchStmt&) { return true; }
	virtual bool visit(TU&) { return true; }
	virtual bool visit(TypeIdent&) { return true; }
	virtual bool visit(VarDecl&) { return true; }
};

class AstVisitable
{
public:
	virtual bool accept(AstVisitor& vistor) = 0;
};

} // namespace Soda

#endif // SODA_AST_VISITOR_H
