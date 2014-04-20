#ifndef SODA_UTILS_H
#define SODA_UTILS_H

#include <uchar.h>
#include <string>

namespace Soda
{

bool is_newline(char32_t ch);
bool is_whitespace(char32_t ch);
bool is_upper(char32_t ch);
bool is_lower(char32_t ch);
bool is_alpha(char32_t ch);
bool is_digit(char32_t ch);
bool is_alnum(char32_t ch);
bool is_hex(char32_t ch);
bool is_binary(char32_t ch);
bool is_octal(char32_t ch);

std::string utf8_encode(const std::u32string& u32str);

} // namespace Soda

#endif // SODA_UTILS_H
