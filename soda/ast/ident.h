#ifndef SODA_AST_IDENT_H
#define SODA_AST_IDENT_H

namespace Soda
{

struct IdentImpl : public ExprImpl
{
	std::u32string name;
	IdentImpl(std::u32string name=U"") : name(name) {}
	virtual void dump(std::ostream& stream);
};

typedef std::unique_ptr<IdentImpl> Ident;


}

#endif // SODA_AST_IDENT_H
