#!/usr/bin/make -f

ifeq ($(V),1)
	V_CXX   = $(CXX) -c
	V_CXXLD = $(CXX)
	V_DEPS  = $(CXX) -MM
	V_PCH   = $(CXX) -x c++-header -g
else
	V_CXX   = @echo "  [CXX]   $@" && $(CXX) -c
	V_CXXLD = @echo "  [CXXLD] $@" && $(CXX)
	V_DEPS  = @echo "  [DEPS]  $@" && $(CXX) -MM
	V_PCH   = @echo "  [PCH]   $@" && $(CXX) -x c++-header -g
endif

SODA_CXXFLAGS = $(CXXFLAGS) -std=c++11 -Wall -Werror -I. -I..
SODA_LIBS = $(LDFLAGS)

ifdef NDEBUG
	SODA_CXXFLAGS += -O3 -s -DNDEBUG=1
else
	SODA_CXXFLAGS += -g -O0
endif

LIB_SOURCES = \
	ast.cc \
	input.cc \
	lexer.cc \
	parseerror.cc \
	parser.cc \
	sema.cc \
	syntaxerror.cc \
	token.cc \
	utils.cc

LIB_OBJECTS = $(LIB_SOURCES:.cc=.o)
LIB_HEADERS = $(LIB_SOURCES:.cc=.h) \
              debugvisitor.h \
              parentpointers.h \
              sourcelocation.h \
              typeannotator.h \
              typereferences.h
SODAC_SOURCES = main.cc
SODAC_OBJECTS = $(SODAC_SOURCES:.cc=.o)

####
# ENTRY POINT
####
all: sodac

####
# OBJECT FILES
####
%.o: %.cc sodainc.gch
	$(V_CXX) -fPIC $(strip $(SODA_CXXFLAGS)) -o $@ $<

####
# PRECOMPILED HEADER
####
sodainc.gch: sodainc.h
	$(V_PCH) $(strip $(SODA_CXXFLAGS)) -o $@ $<

####
# SHARED LIBRARY
####
libsoda.so: $(LIB_OBJECTS)
	$(V_CXXLD) -shared $(strip $(SODA_CXXFLAGS)) -o $@ $(LIB_OBJECTS) $(strip $(SODA_LIBS))

####
# COMPILER DRIVER
####
sodac: $(SODAC_OBJECTS) libsoda.so
	$(V_CXXLD) $(strip $(SODA_CXXFLAGS)) -o $@ $(SODAC_OBJECTS) $(strip $(SODA_LIBS)) -L. -lsoda

####
# TESTS
####
TESTS = test_input test_lexer test_parser test_sema

test_input: test_input.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

test_lexer: test_lexer.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

test_parser: test_parser.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

test_sema: test_sema.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

check: $(TESTS)
	@export LD_LIBRARY_PATH=.
	./test_input
	./test_lexer
	./test_parser
	./test_sema

####
# MISC
####
makefile.cflags: flags_rebuild
	@echo '$(SODA_CXXFLAGS)' | cmp -s - $@ || echo '$(SODA_CXXFLAGS)' > $@

makefile.ldflags: flags_rebuild
	@echo '$(SODA_LIBS)' | cmp -s - $@ || echo '$(SODA_LIBS)' > $@

libsoda.so sodac: makefile.ldflags makefile
$(LIB_SOURCES) $(SODAC_SOURCES) sodainc.gch: makefile.cflags makefile

help:
	@echo "=====================================" && \
	echo  " SODA Compiler and Runtime Make File " && \
	echo  "=====================================\n" && \
	echo "Without arguments you get a silent build of compiler and library:" && \
	echo "    \`make'\n" && \
	echo "Build library only:" && \
	echo "    \`make libsoda.so'\n" && \
	echo "Build specific source file:" && \
	echo "    \`make basename_without_extension.o'\n" && \
	echo "Run tests:" && \
	echo "    \`make check'\n" && \
	echo "Cleanup built files:" && \
	echo "    \`make clean'\n" && \
	echo "For verbose output showing full commands:" && \
	echo "    \`make V=1'\n" && \
	echo "Written and maintained by Matthew Brush <mbrush@codebrainz.ca>"

makefile.deps:
	$(V_DEPS) -MM  $(strip $(SODA_CXXFLAGS)) $(LIB_SOURCES) $(SODAC_SOURCES) $(addsuffix .cc,$(TESTS)) > $@

-include makefile.deps

clean:
	$(RM) *.o libsoda.so sodac $(TESTS)
	$(RM) makefile.deps makefile.cflags makefile.ldflags sodainc.gch

.PHONY: all clean check flags_rebuild
