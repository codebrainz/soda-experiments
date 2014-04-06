SODA_CXXFLAGS = $(CXXFLAGS) -std=c++11 -Wall -Werror -I.
SODA_LIBS = $(LDFLAGS)

ifdef NDEBUG
	SODA_CXXFLAGS += -O3 -s -DNDEBUG=1
else
	SODA_CXXFLAGS += -g -O0
endif

SODA_SOURCES = input.cc lexer.cc main.cc parser.cc token.cc
SODA_OBJECTS = $(SODA_SOURCES:.cc=.o)
SODA_HEADERS = $(wildcard *.h)

all: sodac test_input test_lexer

sodac: $(SODA_OBJECTS)
	$(CXX) $(SODA_CXXFLAGS) -o $@ $^ $(SODA_LIBS)

%.o: %.cc
	$(CXX) -c -fPIC $(SODA_CXXFLAGS) -o $@ $<

test_input: input.cc test_input.cc
	$(CXX) -o $@ $(SODA_CXXFLAGS) $^ $(SODA_LIBS)

test_lexer: input.cc token.cc lexer.cc test_lexer.cc
	$(CXX) -o $@ $(SODA_CXXFLAGS) $^ $(SODA_LIBS)

makefile.deps:
	$(CXX) -MM  $(SODA_CXXFLAGS) $(wildcard *.cc) > $@

-include makefile.deps

clean:
	$(RM) *.o sodac test_input test_lexer makefile.deps
