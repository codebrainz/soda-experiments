#ifndef SODA_AST_TU_H
#define SODA_AST_TU_H

namespace Soda
{

struct TU : public StmtImpl
{
	StmtList stmts;
	std::string fn;
	TU(std::string fn) : fn(fn) {}
	virtual void dump(std::ostream& stream)
	{
		stream << "<TU href=\"" << fn << "\"";
		location.dump(stream);
		stream << ">";
		for (auto &stmt : stmts)
			stmt->dump(stream);
		stream << "</TU>";
	}
};



} // namespace Soda

#endif // SODA_AST_TU_H
