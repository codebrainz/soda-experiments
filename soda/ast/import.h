#ifndef SODA_AST_IMPORT_H
#define SODA_AST_IMPORT_H

namespace Soda
{


struct Import : public StmtImpl
{
	Ident ident;

	Import(Ident&& ident) : ident(std::move(ident)) {}

	virtual void dump(std::ostream& stream)
	{
		stream << "<Import name=\"" << ident->name << "\"";
		location.dump(stream);
		stream << "/>";
	}
};


} // namespace Soda

#endif // SODA_AST_IMPORT_H
