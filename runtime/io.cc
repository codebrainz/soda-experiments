#include "io.h"
#include <iostream>
#include <string>

struct SodaString_
{
	std::u32string str;
};

void soda_io_print(SodaString *str)
{
	std::cout << str->str << std::endl;
}
