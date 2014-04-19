#ifndef SODA_SYMTAB_H
#define SODA_SYMTAB_H

#include <soda/ast.h>
#include <string>
#include <stack>
#include <unordered_map>

namespace Soda
{

struct Stmt;

class SymbolTable
{
public:
	typedef std::unordered_map<std::u32string, Stmt*> SymbolMap;
	SymbolMap name_map;
	bool is_defined(const std::u32string& name);
	void define(std::u32string name, Stmt& stmt);
	Stmt* lookup(const std::u32string& name);
	SymbolTable(TU& tu) : tu(tu) {}
private:
	TU& tu;
};

typedef std::stack<SymbolTable> ScopeStack;

} // namespace Soda

#endif // SODA_SYMTAB_H
