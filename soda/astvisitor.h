#ifndef SODA_AST_VISITOR_H
#define SODA_AST_VISITOR_H

namespace Soda
{

class Alias;
class Argument;
class BinOp;
class BreakStmt;
class CaseStmt;
class ClassDef;
class CompoundStmt;
class FuncDef;
class Ident;
class FuncDef;
class IfStmt;
class Import;
class Integer;
class Float;
class ReturnStmt;
class StrLit;
class SwitchStmt;
class TypeIdent;
class TU;
class VarDecl;


class AstVisitor
{
public:
	virtual bool visit(Alias&) = 0;
	virtual bool visit(Argument&) = 0;
	virtual bool visit(BinOp&) = 0;
	virtual bool visit(BreakStmt&) = 0;
	virtual bool visit(CaseStmt&) = 0;
	virtual bool visit(ClassDef&) = 0;
	virtual bool visit(CompoundStmt&) = 0;
	virtual bool visit(FuncDef&) = 0;
	virtual bool visit(Ident&) = 0;
	virtual bool visit(IfStmt&) = 0;
	virtual bool visit(Import&) = 0;
	virtual bool visit(Integer&) = 0;
	virtual bool visit(Float&) = 0;
	virtual bool visit(ReturnStmt&) = 0;
	virtual bool visit(StrLit&) = 0;
	virtual bool visit(SwitchStmt&) = 0;
	virtual bool visit(TypeIdent&) = 0;
	virtual bool visit(TU&) = 0;
	virtual bool visit(VarDecl&) = 0;
};

class AstVisitable
{
public:
	virtual bool accept(AstVisitor& vistor) = 0;
};

} // namespace Soda

#endif // SODA_AST_VISITOR_H
