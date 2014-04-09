#!/usr/bin/env make

ifeq ($(V),1)
	V_CXX   = $(CXX) -c
	V_CXXLD = $(CXX)
	V_DEPS  = $(CXX) -MM
	V_PCH   = $(CXX) -x c++-header -g
else
	V_CXX   = @echo "  [`tput setaf 6`CXX`tput sgr0`]   `tput bold`$@`tput sgr0`" && $(CXX) -c
	V_CXXLD = @echo "  [`tput setaf 3`CXXLD`tput sgr0`] `tput bold`$@`tput sgr0`" && $(CXX)
	V_DEPS  = @echo "  [`tput setaf 4`DEPS`tput sgr0`]  `tput bold`$@`tput sgr0`" && $(CXX) -MM
	V_PCH   = @echo "  [`tput setaf 5`PCH`tput sgr0`]   `tput bold`$@`tput sgr0`" && $(CXX) -x c++-header -g
endif

SODA_CXXFLAGS = $(CXXFLAGS) -std=c++0x -Wall -Werror -I.
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
	parser.cc \
	token.cc \
	utils.cc

LIB_OBJECTS = $(LIB_SOURCES:.cc=.o)
LIB_HEADERS = $(LIB_SOURCES:.cc=.h)
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
TESTS = test_input test_lexer test_parser

test_input: test_input.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

test_lexer: test_lexer.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

test_parser: test_parser.o | libsoda.so
	$(V_CXXLD) -o $@ $(strip $(SODA_CXXFLAGS)) $^ $(strip $(SODA_LIBS)) -L. -lsoda

check: $(TESTS)
	@export LD_LIBRARY_PATH=.
	./test_input
	./test_lexer
	./test_parser | tidy -xml -asxml -w 80 -i -q

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
	@echo "`tput setaf 3`=====================================`tput sgr0`" && \
	echo  "`tput setaf 1` SODA Compiler and Runtime Make File `tput sgr0`" && \
	echo  "`tput setaf 3`=====================================`tput sgr0`\n" && \
	echo "Without arguments you get a silent build of `tput smul`compiler and library`tput rmul`:" && \
	echo "    \``tput bold`make`tput sgr0`'\n" && \
	echo "Build `tput smul`library only`tput rmul`:" && \
	echo "    \``tput bold`make libsoda.so`tput sgr0`'\n" && \
	echo "Build `tput smul`specific source file`tput rmul`:" && \
	echo "    \``tput bold`make basename_without_extension.o`tput sgr0`'\n" && \
	echo "Run `tput smul`tests`tput rmul`:" && \
	echo "    \``tput bold`make check`tput sgr0`'\n" && \
	echo "Cleanup `tput smul`built files`tput rmul`:" && \
	echo "    \``tput bold`make clean`tput sgr0`'\n" && \
	echo "For `tput smul`verbose output`tput rmul` showing full commands:" && \
	echo "    \``tput bold`make V=1`tput sgr0`'\n" && \
	echo "Written and maintained by Matthew Brush <`tput setaf 6`mbrush@codebrainz.ca`tput sgr0`>"

makefile.deps:
	$(V_DEPS) -MM  $(strip $(SODA_CXXFLAGS)) $(LIB_SOURCES) $(SODAC_SOURCES) > $@

-include makefile.deps

clean:
	$(RM) *.o libsoda.so sodac $(TESTS)
	$(RM) makefile.deps makefile.cflags makefile.ldflags sodainc.gch

.PHONY: all clean check flags_rebuild
