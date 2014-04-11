#ifndef SODA_AST_H
#define SODA_AST_H

#include <soda/sourcelocation.h>
#include <soda/token.h>
#include <soda/ast/node.h>

#include <soda/ast/number.h>
#include <soda/ast/ident.h>
#include <soda/ast/strlit.h>
#include <soda/ast/binop.h>

#include <soda/ast/compoundstmt.h>
#include <soda/ast/ifstmt.h>
#include <soda/ast/vardecl.h>
#include <soda/ast/classdef.h>
#include <soda/ast/argument.h>
#include <soda/ast/funcdef.h>
#include <soda/ast/returnstmt.h>
#include <soda/ast/alias.h>
#include <soda/ast/import.h>
#include <soda/ast/tu.h>

#include <string>
#include <memory>
#include <vector>
#include <ostream>
#include <iostream>
#include <cassert>

#endif // SODA_AST_H
