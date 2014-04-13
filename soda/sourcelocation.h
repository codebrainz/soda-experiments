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
	SourceRange offset, line, column;

	SourceLocation(size_t offset_start=0, size_t offset_end=0,
	               size_t line_start=0,   size_t line_end=0,
	               size_t column_start=0, size_t column_end=0)
		: offset(offset_start, offset_end),
		  line(line_start, line_end),
		  column(column_start, column_end) {}

	SourcePosition pos_start()  const {
		return SourcePosition(offset.start, line.start, column.start);
	}
	SourcePosition pos_end() const {
		return SourcePosition(offset.end, line.end, column.end);
	}
};

struct SourcePosition
{
	size_t offset, line, column;
	SourcePosition(size_t offset=0, size_t line=0, size_t column=0)
		: offset(offset), line(line), column(column) {}
	bool operator==(const SourcePosition& rhs)
	{
		return (offset == rhs.offset && line == rhs.line && column == rhs.column);
	}
	bool operator!=(const SourcePosition& rhs)
	{
		return !(*this == rhs);
	}
};

} // namespace Soda

#endif // SODA_SOURCELOCATION_H
