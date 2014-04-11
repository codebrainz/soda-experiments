#ifndef SODA_AST_BINOP_H
#define SODA_AST_BINOP_H

namespace Soda
{

struct BinOp : public ExprImpl
{
	char32_t op;
	Expr lhs, rhs;

	BinOp(char32_t op, Expr&& lhs, Expr&& rhs)
		: op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<BinOp op=\"" << (char)op << "\"";
		location.dump(stream);
		stream << ">";
		lhs->dump(stream);
		rhs->dump(stream);
		stream << "</BinOp>";
	}
};

} // namespace Soda

#endif // SODA_AST_BINOP_H
