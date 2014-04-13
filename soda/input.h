#ifndef SODA_INPUT_H
#define SODA_INPUT_H

#include <soda/sourcelocation.h>
#include <cstddef>
#include <istream>
#include <limits>
#include <uchar.h>

namespace Soda
{

class Input
{
public:
	static const char32_t END = std::numeric_limits<char32_t>::max();

	Input(std::istream& stream);
	char32_t next();
	char32_t peek();
	bool eof() const;
	void skip_whitespace();

	SourcePosition position;
	char32_t last;

	std::istream& input_stream() { return stream; }

private:
	std::istream& stream;
	std::istreambuf_iterator<char> end, iter;
	bool has_peeked;
	char32_t peeked;
};

} // namespace Soda

#endif // SODA_INPUT_H
