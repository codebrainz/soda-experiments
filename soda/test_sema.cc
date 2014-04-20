#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/parseerror.h>
#include <soda/parentpointers.h>
#include <soda/typeannotator.h>
#include <soda/typereferences.h>
#include <soda/debugvisitor.h>

#include <cassert>

using namespace Soda;

int main()
{
	TU root("test.soda");
	std::ifstream stream("test.soda");

	try
	{
		parse(root, stream);
	}
	catch (SyntaxError& err)
	{
		format_exception(std::cerr, err);
		return 1;
	}

	try
	{
		ParentPointers pp_pass;
		root.accept(pp_pass);

		TypeAnnotator annot_pass(root);
		root.accept(annot_pass);

		TypeReferences ref_pass(root);
		root.accept(ref_pass);

		DebugVisitor printer(std::cout);
		root.accept(printer);

	}
	catch (ParseError& err)
	{
		format_exception(std::cerr, err);
		return 2;
	}

	return 0;
}
