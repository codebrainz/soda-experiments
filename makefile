SODA_CXXFLAGS = $(CXXFLAGS) -std=c++11 -Wall -Werror -I.
SODA_LIBS = $(LDFLAGS)

ifdef NDEBUG
	SODA_CXXFLAGS += -O3 -s -DNDEBUG=1
else
	SODA_CXXFLAGS += -g -O0
endif

SODA_SOURCES = main.cc input.cc lexer.cc token.cc ast.cc parser.cc utils.cc
SODA_OBJECTS = $(SODA_SOURCES:.cc=.o)
SODA_HEADERS = $(wildcard *.h)

TEST = @echo "  [TEST] $<" && ./$<

all: sodac

sodac: $(SODA_OBJECTS)
	$(CXX) $(SODA_CXXFLAGS) -o $@ $^ $(SODA_LIBS)

%.o: %.cc
	$(CXX) -c -fPIC $(SODA_CXXFLAGS) -o $@ $<

test_input: input.cc utils.cc test_input.cc
	$(CXX) -o $@ $(SODA_CXXFLAGS) $^ $(SODA_LIBS)

test_lexer: input.cc token.cc lexer.cc utils.cc test_lexer.cc
	$(CXX) -o $@ $(SODA_CXXFLAGS) $^ $(SODA_LIBS)

test_parser: input.cc token.cc lexer.cc utils.cc ast.cc parser.cc test_parser.cc
	$(CXX) -o $@ $(SODA_CXXFLAGS) $^ $(SODA_LIBS)

makefile.deps:
	$(CXX) -MM  $(SODA_CXXFLAGS) $(wildcard *.cc) > $@

-include makefile.deps

clean:
	$(RM) *.o sodac test_input test_lexer test_parser makefile.deps

check-input: test_input
	$(TEST)

check-lexer: test_lexer
	$(TEST)

check-parser: test_parser
	$(TEST)

check: check-input check-lexer check-parser

.PHONY: all clean check-input check-lexer check-parser check
