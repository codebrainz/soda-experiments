#ifndef SODA_SOURCELOCATION_H
#define SODA_SOURCELOCATION_H

#include <ostream>

namespace Soda
{

struct SourceRange
{
	size_t start, end;
	SourceRange(size_t start=0, size_t end=0) : start(start), end(end) {}
};

struct SourceLocation
{
	SourceRange position, line, column;

	SourceLocation()
		: position(0,0), line(0,0), column(0,0) {}

	SourceLocation(SourceRange position,
	               SourceRange line,
	               SourceRange column)
		: position(position), line(line), column(column) {}

	void dump(std::ostream& stream)
	{
		stream << " line=\""   << line.start   << "," << line.end   << "\""
		       << " column=\"" << column.start << "," << column.end << "\"";
	}
};

} // namespace Soda

#endif // SODA_SOURCELOCATION_H
