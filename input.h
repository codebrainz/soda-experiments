#ifndef SODA_INPUT_H
#define SODA_INPUT_H

#include <cstddef>
#include <istream>
#include <limits>

namespace Soda
{

class Input
{
public:
	typedef char32_t char_type;
	typedef size_t size_type;

	static const char_type END = std::numeric_limits<char_type>::max();

	Input(std::istream& stream);
	char_type next();
	char_type peek();
	bool eof() const;
	void skip_whitespace();

	size_type position, line, column;
	char_type last;

	std::istream& input_stream() { return stream; }

private:
	std::istream& stream;
	std::istreambuf_iterator<char> end, iter;
	bool has_peeked;
	char_type peeked;
};

} // namespace Soda

#endif // SODA_INPUT_H
