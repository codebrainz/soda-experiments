#ifndef SODA_NODE_H
#define SODA_NODE_H

#include <memory>
#include <ostream>
#include <vector>
#include <string>

#define SODA_NODE_TYPES(T) \
	typedef std::shared_ptr<T> T##Ptr; \
	typedef std::vector<T##Ptr> T##List

namespace Soda
{

struct Node
{
	SourceLocation location;
	Node() {}
	virtual ~Node() {}
	virtual void dump(std::ostream& stream) = 0;
};

struct ExprImpl : public Node {};
typedef std::unique_ptr<ExprImpl> Expr;

struct StmtImpl : public Node {};
typedef std::unique_ptr<StmtImpl> Stmt;
typedef std::vector<Stmt> StmtList;

} // namespace Soda

#endif // SODA_NODE_H
