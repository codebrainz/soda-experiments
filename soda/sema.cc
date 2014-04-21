#include <soda/sodainc.h> // pch
#include <soda/parentpointers.h>
#include <soda/typeannotator.h>
#include <soda/typereferences.h>
#include <soda/sema.h>

namespace Soda
{

struct SemaImpl
{
	TU& root;
	ParentPointers pp_pass;
	TypeAnnotator annot_pass;
	TypeReferences ref_pass;

	SemaImpl(TU& root)
		: root(root),
		  annot_pass(root),
		  ref_pass(root)
	{
	}

	void check()
	{
		root.accept(pp_pass);
		root.accept(annot_pass);
		root.accept(ref_pass);
	}
};

Sema::Sema(TU& root)
	: impl(new SemaImpl(root))
{
}

Sema::~Sema()
{
	delete impl;
}

void Sema::check()
{
	impl->check();
}

} // namespace Soda
