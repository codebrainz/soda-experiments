#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/debugvisitor.h>
#include <fstream>

using namespace Soda;

void print_help()
{
	std::cout << "Usage: sodac [options] source_files...\n" <<
	             "\n" <<
	             "Options\n" <<
	             "-------\n" <<
	             "  source_files   One or more sodac files to compile\n" <<
	             "  -h, --help     Show this help message and exit\n" <<
	             "  -V, --version  Show version information\n\n";
}

int main(int argc, char *argv[])
{
	for (int i=1; i < argc; i++)
	{
		std::string arg(argv[i]);
		if (arg == "-h" || arg == "--help")
		{
			print_help();
			return 0;
		}
		else if (arg == "-V" || arg == "--version")
		{
			std::cout << "0.01" << std::endl;
			return 0;
		}
		else
		{
			try
			{
				TU tu(argv[i]);
				if (tu.fn == "-")
				{
					tu.fn = "<stdin>";
					parse(tu, std::cin);
				}
				else
				{
					std::ifstream f(tu.fn);
					if (!f.is_open())
					{
						std::cerr << "error: failed to open input file '"
								  << tu.fn << "'" << std::endl;
						return 0;
					}
					parse(tu);
				}
				DebugVisitor visitor(std::cout);
				tu.accept(visitor);
				return 0;
			}
			catch (SyntaxError& err)
			{
				format_exception(std::cerr, err);
				return 1;
			}
		}
	}
	return 1;
}
