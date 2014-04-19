#include <soda/symtab.h>
#include <soda/ast.h>
#include <soda/syntaxerror.h>
#include <sstream>

namespace Soda
{

bool SymbolTable::is_defined(const std::u32string& name)
{
	return (name_map.find(name) != name_map.end());
}

void SymbolTable::define(std::u32string name, Stmt& stmt)
{
	auto found = name_map.find(name);
	if (found == name_map.end())
		name_map.emplace(std::move(name), &stmt);
	else
	{
		std::stringstream ss;
		ss << "multiple defintions of `" << name << "', previous definition "
		   << "is on line " << stmt.location.line.start << " column "
		   << stmt.location.column.start;
		throw SyntaxError("syntax error", tu.fn,
			SourceLocation(found->second->location), ss.str());
	}
}

Stmt* SymbolTable::lookup(const std::u32string& name)
{
	auto found = name_map.find(name);
	if (found != name_map.end())
		return found->second;
	return nullptr;
}

} // namespace Soda
