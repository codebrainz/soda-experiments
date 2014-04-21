#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/lexer.h>
#include <deque>
#include <cassert>
#include <stack>
#include <sstream>
#include <fstream>

namespace Soda
{

#define SYNTAX_ERROR(msg) \
	throw Soda::SyntaxError("syntax error", tu.fn, \
		SourceLocation(tokens[index].location), msg)

#define CHECK_SEMI(exp)                                             \
	do { if (!ACCEPT(';')) {                                        \
		std::stringstream ss;                                       \
		ss << "expected semicolon at end of `" exp "' statement, got `" \
		   << text() << "' (" << current() << ")";                  \
		SYNTAX_ERROR(ss.str());                                     \
	} } while (0)

struct Parser
{

TU& tu;
SourcePosition last_end;
TokenList tokens;
size_t index;

Parser(std::istream& stream, TU& tu)
	: tu(tu), tokens(std::move(tokenize(stream))), index(0)
{
	tokens.push_back(Token());
	tokens.back().kind = Token::END;
}

// retrieve the current token kind
Token::Kind current()
{
	assert(index < tokens.size());
	return tokens[index].kind;
}

// advance in the tokens and return the next (ie. new current) token kind
Token::Kind next()
{
	if (index < (tokens.size() - 1))
	{
		last_end = tokens[index].location.end();
		return tokens[++index].kind;
	}
	return Token::END;
}

// look ahead of the current token by n tokens and return its kind
Token::Kind lookahead(size_t n=1)
{
	size_t new_index = index + n;
	if (new_index < tokens.size())
		return tokens[new_index].kind;
	return Token::END;
}

// gets the text of the current token
const std::u32string& text() const
{
	return tokens[index].text;
}

// gets the start position of the current token
SourcePosition start() const
{
	return tokens[index].location.start();
}

// gets the end position of the previous  token
SourcePosition end() const
{
	return last_end;
}

// if the 'current()' token is what is expected, advance and return true
bool accept(Token::Kind kind, const char *file, unsigned int line)
{
	if (current() == kind)
	{
		next();
		return true;
	}
	return false;
}

// if the 'current()' token is what is expected, advance, otherwise throw exception
void expect(Token::Kind kind, const char *file, unsigned int line)
{
	if (!accept(kind, file, line))
	{
		std::stringstream ss;
		ss << "unexpected token `" << current() << "', expecting `" << kind << "'";
#ifndef NDEBUG
		ss << ".\n\x1B[34m\x1B[47mcallsite\x1B[0m: " << file
		   << ":\x1B[47m" << line << "\x1B[0m";
#endif
		SYNTAX_ERROR(ss.str());
	}
}

#define ACCEPT(kind) accept((Token::Kind)(kind), __FILE__, __LINE__)
#define EXPECT(kind) expect((Token::Kind)(kind), __FILE__, __LINE__)

int get_prec()
{
	// FIXME: check these
	switch (current())
	{
		case Token::LOG_AND:
		case Token::LOG_OR:
			return 10;
		case Token::LE_OP:
		case Token::GE_OP:
		case Token::NE_OP:
		case Token::EQ_OP:
			return 20;
		case Token::LSHIFT:
		case Token::RSHIFT:
			return 30;
		case '&':
		case '|':
		case '^':
			return 40;
		case '<':
		case '>':
			return 50;
		case '+':
		case '-':
			return 60;
		case '*':
		case '/':
		case '%':
			return 70;
		case Token::DEC_OP:
		case Token::INC_OP:
			return 80;
		default:
			return -1;
	}
}

void parse()
{
	p_tu(tu);
}

//////////////////////////////////////////////////////////////////////////////

//> func_decl ::= type_ident ident_expr '(' arg_list ')' ';' .
StmtPtr p_func_decl()
{
	SourcePosition spos = start();
	size_t save_index = index;
	TypeIdentPtr type(p_type_ident());
	if (type)
	{
		IdentPtr name(p_ident_expr());
		if (name)
		{
			if (ACCEPT('('))
			{
				StmtList args; p_arg_list(args);
				EXPECT(')');
				CHECK_SEMI("external function declaration");
				return StmtPtr(new FuncDecl(std::move(type), std::move(name),
					std::move(args), spos, end()));
			}
		}
	}
	index = save_index;
	return StmtPtr(nullptr);
}

//> ccode ::= '[' CCODE [ccode_params] ']' .
StmtPtr p_ccode()
{
	SourcePosition spos = start();
	size_t saved_index = index;
	if (ACCEPT('['))
	{
		EXPECT(Token::CCODE);
		CCodeParamList params;
		p_ccode_params(params);
		EXPECT(']');
		CCodePtr ccptr(new CCode(std::move(params), spos, end()));
		StmtPtr fdecl(p_func_decl());
		if (fdecl)
		{
			FuncDecl *fdeclp = dynamic_cast<FuncDecl*>(fdecl.get());
			assert(fdeclp);
			fdeclp->ccode = std::move(ccptr);
			return std::move(fdecl);
		}
		else
		{
			std::stringstream ss;
			ss << "expected external function declaration after `[CCode]' "
			   << "specification, got `" << text() << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
	}
	index = saved_index;
	return StmtPtr(nullptr);
}

//> ccode_params ::= '(' ccode_param { ',' ccode_param } ')' .
void p_ccode_params(CCodeParamList& lst)
{
	if (ACCEPT('('))
	{
		do
		{
			CCodeParamPtr cp(p_ccode_param());
			if (!cp)
				break;
			else
				lst.push_back(std::move(cp));
		}
		while (ACCEPT(','));
		EXPECT(')');
	}
}

//> ccode_param ::= IDENT '=' STR_LIT .
CCodeParamPtr p_ccode_param()
{
	SourcePosition spos = start();
	std::u32string name(text());
	if (ACCEPT(Token::IDENT))
	{
		EXPECT('=');
		std::u32string value(text());
		EXPECT(Token::STR_LIT);
		return CCodeParamPtr(new CCodeParam(std::move(name), std::move(value), spos, end()));
	}
	return CCodeParamPtr(nullptr);
}

//> tu ::= { stmt_list } .
void p_tu(TU& tu)
{
	p_stmt_list(tu.stmts, true);
}

//> namespace ::= NAMESPACE [ IDENT ] '{' stmt_list '}' .
StmtPtr p_namespace_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::NAMESPACE))
	{
		IdentPtr name(nullptr);
		if (current() == Token::IDENT)
			name = std::move(p_ident_expr());
		EXPECT('{');
		StmtList stmts;
		p_stmt_list(stmts, true);
		EXPECT('}');
		return StmtPtr(new Namespace(std::move(name), std::move(stmts), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> import_stmt ::= IMPORT fq_ident_expr ';' .
StmtPtr p_import_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::IMPORT))
	{
		IdentPtr name(p_fq_ident_expr());
		if (!name)
		{
			std::stringstream ss;
			ss << "expected identifier after `import', got `" << text()
			   << " (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		CHECK_SEMI("import");
		return StmtPtr(new Import(std::move(name), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> alias ::= ALIAS IDENT '=' IDENT ';' .
StmtPtr p_alias()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::ALIAS))
	{
		IdentPtr type(p_ident_expr());
		if (!type)
		{
			std::stringstream ss;
			ss << "expected identifier after `alias', got `" << text()
			   << " (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		EXPECT('=');
		TypeIdentPtr alias(p_type_ident());
		if (!alias)
		{
			std::stringstream ss;
			ss << "expected identifier after `=', got `" << text()
			   << " (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		CHECK_SEMI("alias");
		return StmtPtr(new Alias(std::move(type),
		                         std::move(alias),
		                         spos, end()));
	}
	return StmtPtr(nullptr);
}

//> delegate ::= DELEGATE type_ident IDENT '(' arg_list ')' ';'
StmtPtr p_delegate_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::DELEGATE))
	{
		TypeIdentPtr type(p_type_ident());
		if (!type)
		{
			std::stringstream ss;
			ss << "expected type name after `delegate', got `" << text()
			   << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		IdentPtr name(p_ident_expr());
		if (!name)
		{
			std::stringstream ss;
			ss << "expected identifier after `delegate' type name, got `"
			   << text() << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		EXPECT('(');
		StmtList args;
		p_arg_list(args);
		EXPECT(')');
		CHECK_SEMI("delegate");
		return StmtPtr(new Delegate(std::move(type),
		                            std::move(name),
		                            std::move(args),
		                            spos, end()));
	}
	return StmtPtr(nullptr);
}

//> type_ident ::= [const] fq_ident_expr .
TypeIdentPtr p_type_ident()
{
	SourcePosition spos = start();
	size_t start_index = index;
	bool is_const = false;
	if (ACCEPT(Token::CONST))
		is_const = true;
	IdentPtr type(p_fq_ident_expr());
	if (type)
		return TypeIdentPtr(new TypeIdent(type->name, is_const, spos, end()));
	index = start_index;
	return TypeIdentPtr(nullptr);
}

//> specifiers ::= ((private | protected | public | internal) | static)
//>                    { ((private | protected | public | internal) | static) } .
void p_specifiers(AccessModifier& access, StorageClassSpecifier& storage)
{
	access    = AccessModifier::DEFAULT;
	storage   = StorageClassSpecifier::NONE;
	while (true)
	{
		switch (current())
		{
			case Token::PRIVATE:   access    = AccessModifier::PRIVATE;       break;
			case Token::PROTECTED: access    = AccessModifier::PROTECTED;     break;
			case Token::PUBLIC:    access    = AccessModifier::PUBLIC;        break;
			case Token::INTERNAL:  access    = AccessModifier::INTERNAL;      break;
			case Token::STATIC:    storage   = StorageClassSpecifier::STATIC; break;
			default:
				return;
		}
		next();
	}
}

//> var_decl ::= specifiers type_ident ident_expr [ '=' expr ] ';' .
StmtPtr p_var_decl(bool as_arg=false)
{
	SourcePosition spos = start();
	size_t save_index = index;
	AccessModifier access;
	StorageClassSpecifier storage;
	p_specifiers(access, storage);
	TypeIdentPtr type(p_type_ident());
	if (type)
	{
		IdentPtr name(p_ident_expr());
		if (name)
		{
			if (ACCEPT('='))
			{
				ExprPtr expr(p_expr());
				if (!expr)
				{
					std::stringstream ss;
					ss << "expected expression as right hand side of initial " <<
						  "assignment but got `" << text() << "' " <<
						  "(" << current() << ")";
					SYNTAX_ERROR(ss.str());
				}
				if (!as_arg)
					CHECK_SEMI("initial assignment variable declaration");
				return StmtPtr(new VarDecl(access, storage, std::move(type),
					std::move(name), std::move(expr), spos, end()));
			}
			else
			{
				if (!as_arg)
					CHECK_SEMI("variable declaration");
				return StmtPtr(new VarDecl(access, storage, std::move(type),
					std::move(name), ExprPtr(nullptr), spos, end()));
			}
		}
	}
	index = save_index;
	return StmtPtr(nullptr);
}

//> func_def ::= specifiers type_ident ident_expr '(' arg_list ')' stmt .
StmtPtr p_func_def()
{
	SourcePosition spos = start();
	size_t save_index = index;
	AccessModifier access;
	StorageClassSpecifier storage;
	p_specifiers(access, storage);
	TypeIdentPtr type(p_type_ident());
	if (type)
	{
		IdentPtr name(p_ident_expr());
		if (name)
		{
			if (ACCEPT('('))
			{
				StmtList args; p_arg_list(args);
				EXPECT(')');
				EXPECT('{');
				StmtList stmts;
				p_stmt_list(stmts);
				EXPECT('}');
				return StmtPtr(new FuncDef(access, storage, std::move(type),
					std::move(name), std::move(args), std::move(stmts), spos, end()));
			}
		}
	}
	index = save_index;
	return StmtPtr(nullptr);
}

//> arg_list ::= var_decl { ',' var_decl } .
void p_arg_list(StmtList& lst)
{
	do
	{
		StmtPtr stmt(p_var_decl(true));
		if (!stmt)
			break;
		lst.push_back(std::move(stmt));
	}
	while (ACCEPT(','));
}

//> bases_list ::= fq_ident_expr { ',' fq_ident_expr } .
void p_bases_list(ExprList& lst)
{
	do
	{
		ExprPtr base(p_fq_ident_expr());
		if (!base)
			break;
		lst.emplace_back(std::move(base));
	}
	while (ACCEPT(','));
}

//> class_def ::= CLASS IDENT [':' bases_list ] compound_stmt ';' .
StmtPtr p_class_def()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::CLASS))
	{
		IdentPtr name(p_ident_expr());
		ExprList bases;
		if (ACCEPT(':'))
		{
			p_bases_list(bases);
			if (bases.empty())
			{
				std::stringstream ss;
				ss << "expected one or more base class identifiers after `:', "
				   << "got `" << text() << "' (" << current() << ")";
				SYNTAX_ERROR(ss.str());
			}
		}
		EXPECT('{');
		StmtList stmts;
		p_stmt_list(stmts, true);
		EXPECT('}');
		return StmtPtr(new ClassDef(std::move(name), std::move(bases),
			std::move(stmts), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> case ::= CASE expr stmt .
StmtPtr p_case()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::CASE))
	{
		ExprPtr expr(p_expr());
		if (!expr)
		{
			std::stringstream ss;
			ss << "expecting expression in `case', got `" << text()
			   << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		StmtPtr stmt(p_stmt());
		if (!stmt)
		{
			std::stringstream ss;
			ss << "expecting stmt in `case' block, got `" << text()
			   << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		return StmtPtr(new CaseStmt(std::move(expr), std::move(stmt), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> default ::= DEFAULT stmt .
StmtPtr p_default()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::DEFAULT))
	{
		StmtPtr stmt(p_stmt());
		if (!stmt)
		{
			std::stringstream ss;
			ss << "expecting statement in default block, got `" << text()
			   << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		return StmtPtr(new CaseStmt(ExprPtr(nullptr), std::move(stmt), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> case_list ::= { (case | default) } .
void p_case_list(StmtList& case_list)
{
	while (true)
	{
		StmtPtr case_stmt(p_case());
		if (case_stmt)
			case_list.push_back(std::move(case_stmt));
		else
		{
			StmtPtr def_stmt(p_default());
			if (def_stmt)
				case_list.push_back(std::move(def_stmt));
			else
				break;
		}
		//while (ACCEPT(';'))
		//	;
	}
}

//> switch_stmt ::= SWITCH '(' expr ')' '{' case_list '}' .
StmtPtr p_switch_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::SWITCH))
	{
		EXPECT('(');
		ExprPtr expr(p_expr());
		EXPECT(')');
		EXPECT('{');
		StmtList cases;
		p_case_list(cases);
		EXPECT('}');
		return StmtPtr(new SwitchStmt(std::move(expr), std::move(cases), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> if_stmt ::= IF '(' expr ')' stmt [ ELSE stmt ] .
StmtPtr p_if_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::IF))
	{
		EXPECT('(');
		ExprPtr expr(p_expr());
		if (!expr)
		{
			std::stringstream ss;
			ss << "expected condition expression in `if', got " <<
			      "`" << text() << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		EXPECT(')');
		StmtPtr if_stmt(p_stmt());
		if (ACCEPT(Token::ELSE))
		{
			StmtPtr else_stmt(p_stmt());
			return StmtPtr(new IfStmt(std::move(expr),
			                          std::move(if_stmt),
			                          std::move(else_stmt), spos, end()));
		}
		else
		{
			return StmtPtr(new IfStmt(std::move(expr),
			                          std::move(if_stmt),
			                          StmtPtr(nullptr), spos, end()));
		}
	}
	return StmtPtr(nullptr);
}

//> return_stmt ::= RETURN [ expr ] ';' .
StmtPtr p_return_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::RETURN))
	{
		ExprPtr expr(p_expr());
		CHECK_SEMI("return");
		return StmtPtr(new ReturnStmt(std::move(expr), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> break_stmt ::= BREAK ';' .
StmtPtr p_break_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::BREAK))
	{
		CHECK_SEMI("break");
		return StmtPtr(new BreakStmt(spos, end()));
	}
	return StmtPtr(nullptr);
}

//> compound_stmt ::= '{' p_stmt_list '}' .
StmtPtr p_compound_stmt(bool top_level=false)
{
	SourcePosition spos = start();
	if (ACCEPT('{'))
	{
		StmtList stmts;
		p_stmt_list(stmts, top_level);
		EXPECT('}');
		return StmtPtr(new CompoundStmt(std::move(stmts), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> expr_stmt ::= call_expr ';' .
StmtPtr p_expr_stmt()
{
	SourcePosition spos = start();
	ExprPtr expr(p_expr());
	if (expr)
	{
		CHECK_SEMI("expression statement");
		return StmtPtr(new ExprStmt(std::move(expr), spos, end()));
	}
	return StmtPtr(nullptr);
}

//> empty_stmt ::= ';'
StmtPtr p_empty_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(';'))
		return StmtPtr(new EmptyStmt(spos, end()));
	return StmtPtr(nullptr);
}

//> stmt ::= alias
//>       |  break_stmt
//>       |  class_def
//>       |  compound_stmt
//>       |  if_stmt
//>       |  import_stmt
//>       |  return_stmt
//>       |  switch_stmt
//>       |  expr_stmt
//>       |  func_def
//>       |  var_decl
//>       |  empty_stmt
//>       .
StmtPtr p_stmt(bool top_level=false)
{
#define TRY_STMT(name) \
	do { StmtPtr stmt(p_##name()); if (stmt) { return stmt; } } while (0)

	TRY_STMT(ccode);

	TRY_STMT(alias);
	TRY_STMT(class_def);
	TRY_STMT(import_stmt);
	TRY_STMT(namespace_stmt);
	TRY_STMT(delegate_stmt);

	if (!top_level)
	{
		TRY_STMT(break_stmt);
		TRY_STMT(compound_stmt);
		TRY_STMT(if_stmt);
		TRY_STMT(return_stmt);
		TRY_STMT(switch_stmt);
	}

	TRY_STMT(func_def);
	TRY_STMT(var_decl);

	if (!top_level)
		TRY_STMT(expr_stmt);

	// remove empty statements
	if (p_empty_stmt())
		return p_stmt(top_level);

	return StmtPtr(nullptr);

#undef TRY_STMT
}

//> stmt_list ::= stmt { stmt } .
void p_stmt_list(StmtList& lst, bool top_level=false)
{
	while (true)
	{
		StmtPtr stmt(p_stmt(top_level));
		if (!stmt)
			break;
		lst.push_back(std::move(stmt));
	}
}

//> number_expr ::= ??INTEGER_CONSTANTS?? | FCONST .
ExprPtr p_number_expr()
{
	int base;
	SourcePosition spos = start();

	switch (current())
	{
		case Token::DEC_ICONST: base = 10; break;
		case Token::HEX_ICONST: base = 16; break;
		case Token::OCT_ICONST: base =  8; break;
		case Token::BIN_ICONST: base =  2; break;
		case Token::FCONST:     base =  0; break;
		default:                base = -1; break;
	}

	if (base < 0)
		return ExprPtr(nullptr);

	try
	{
		if (base == 0)
		{
			ExprPtr expr(new Float(text(), spos, end()));
			next();
			return expr;
		}
		else
		{
			ExprPtr expr(new Integer(text(), base, spos, end()));
			next();
			return expr;
		}
	}
	catch (std::invalid_argument&)
	{
		std::stringstream ss;
		ss << "failed to parse ";
		if (base == 0)
			ss << "floating point number `" << text() << "'";
		else
			ss << "base " << base << " integer `" << text() << "'";
		if (text().empty())
			ss << ". prefixed numbers must have at least 1 digit";
		SYNTAX_ERROR(ss.str());
	}

	return ExprPtr(nullptr);
}

//> paren_expr ::= '(' expr ')' .
ExprPtr p_paren_expr()
{
	if (ACCEPT('('))
	{
		ExprPtr expr(p_expr());
		if (expr)
		{
			EXPECT(')');
			return expr;
		}
	}
	return ExprPtr(nullptr);
}

//> ident_expr ::= IDENT .
IdentPtr p_ident_expr()
{
	if (current() == Token::IDENT)
	{
		std::u32string name(text());
		EXPECT(Token::IDENT);
		return IdentPtr(new Ident(name, start(), end()));
	}
	return IdentPtr(nullptr);
}

//> fq_ident_expr ::= IDENT { '.' IDENT } .
IdentPtr p_fq_ident_expr()
{
	if (current() == Token::IDENT)
	{
		SourcePosition spos = start();
		std::u32string name;
		while (current() == Token::IDENT)
		{
			name += text();
			EXPECT(Token::IDENT);
			if (ACCEPT(Token::DOT))
				name += U".";
			else
				break;
		}
		return IdentPtr(new Ident(name, spos, end()));
	}
	return IdentPtr(nullptr);
}

//> strlit_expr ::= STR_LIT { STR_LIT } .
ExprPtr p_strlit_expr()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::STR_LIT))
	{
		std::u32string txt;
		do
		{
			txt += text().substr(1, text().size() - 2);
			next();
		} while (current() == Token::STR_LIT);
		return ExprPtr(new StrLit(txt, spos, end()));
	}
	return ExprPtr(nullptr);
}

ExprPtr p_call_expr()
{
	SourcePosition spos = start();
	size_t start_index = index;
	IdentPtr ident(p_fq_ident_expr());
	if (ident)
	{
		if (ACCEPT('('))
		{
			ExprList args;
			do
			{
				ExprPtr expr(p_expr());
				if (!expr)
					break;
				args.push_back(std::move(expr));
			}
			while (ACCEPT(','));
			EXPECT(')');
			return ExprPtr(new CallExpr(std::move(ident),
			                            std::move(args),
			                            spos, end()));
		}
	}
	index = start_index;
	return ExprPtr(nullptr);
}

//> primary_expr ::= ident_expr
//>               | number_expr
//>               | strlit_expr
//>               | paren_expr
//>               .
ExprPtr p_primary_expr()
{
#define TRY_EXPR(name) \
	do { ExprPtr expr(p_##name()); if (expr) { return expr; } } while(0)

	TRY_EXPR(call_expr);
	TRY_EXPR(ident_expr);
	TRY_EXPR(number_expr);
	TRY_EXPR(strlit_expr);
	TRY_EXPR(paren_expr);

#if 0
	std::stringstream ss;
	if (text().empty())
		ss << "syntax error";
	else
		ss << "unexpected token `" << text() << "' (" << current() << ")";
	ss << ", expecting primary expression";
	SYNTAX_ERROR(ss.str());
#endif

	return ExprPtr(nullptr);

#undef TRY_EXPR
}

//> expr ::= primary_expr [ bin_op_rhs ] .
ExprPtr p_expr()
{
	SourcePosition spos = start();
	ExprPtr lhs(p_primary_expr());
	if (!lhs)
		return ExprPtr(nullptr);
	return p_bin_op_rhs(0, lhs.release(), spos);
}

//> bin_op_rhs ::= { ??OPERATORS?? primary_expr } .
ExprPtr p_bin_op_rhs(int expr_prec, Expr* lhs, SourcePosition spos)
{
	while (true)
	{
		int tok_prec = get_prec();
		if (tok_prec < expr_prec)
			return ExprPtr(lhs);
		char32_t op = current();
		next();
		ExprPtr rhs(p_primary_expr());
		if (!rhs)
			return nullptr;
		int next_prec = get_prec();
		if (tok_prec < next_prec)
		{
			rhs.reset(p_bin_op_rhs(tok_prec + 1, rhs.release(), spos).release());
			if (!rhs)
				return nullptr;
		}
		lhs = new BinOp(op, ExprPtr(lhs), std::move(rhs), spos, end());
		spos = start();
	}
}

//////////////////////////////////////////////////////////////////////////////

}; // struct Parser

void parse(TU& tu, std::istream& stream)
{
	Parser p(stream, tu);
	p.parse();
}

void parse(TU& tu, const std::string& str)
{
	std::stringstream ss(str);
	parse(tu, ss);
}

void parse(TU& tu)
{
	std::ifstream stream(tu.fn);
	parse(tu, stream);
}

} // namespace Soda
