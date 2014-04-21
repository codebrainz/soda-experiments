#ifndef SODA_SEMA_H
#define SODA_SEMA_H

#include <soda/ast.h>

namespace Soda
{

struct SemaImpl;

class Sema
{
public:
	Sema(TU& root);
	~Sema();
	void check();
private:
	SemaImpl *impl;
};

} // namespace Soda

#endif // SODA_SEMA_H
