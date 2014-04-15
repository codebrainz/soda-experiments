#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/lexer.h>
#include <deque>
#include <cassert>
#include <stack>

namespace Soda
{

#define SYNTAX_ERROR(msg) \
	throw Soda::SyntaxError("syntax error", tu.fn, \
		SourceLocation(tokens[index].location), msg)

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

// tu ::= { stmt } .
void p_tu(TU& tu)
{
	p_stmt_list(tu.stmts, true);
}

// import_stmt ::= IMPORT IDENT .
StmtPtr p_import_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::IMPORT))
		return StmtPtr(new Import(p_ident_expr(), spos, end()));
	return StmtPtr(nullptr);
}

// alias ::= ALIAS IDENT '=' IDENT .
StmtPtr p_alias()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::ALIAS))
	{
		IdentPtr type(p_ident_expr());
		EXPECT('=');
		return StmtPtr(new Alias(std::move(type), p_ident_expr(), spos, end()));
	}
	return StmtPtr(nullptr);
}

// type_ident ::= [const] ident_expr .
TypeIdentPtr p_type_ident()
{
	SourcePosition spos = start();
	size_t start_index = index;
	bool is_const = false;
	if (ACCEPT(Token::CONST))
		is_const = true;
	IdentPtr type(p_ident_expr());
	if (type)
		return TypeIdentPtr(new TypeIdent(type->name, is_const, spos, end()));
	index = start_index;
	return TypeIdentPtr(nullptr);
}

// specifiers ::= ((private | protected | public | internal) | static)
//                    { ((private | protected | public | internal) | static) } .
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

// var_decl ::= specifiers type_ident ident_expr [ '=' expr ] .
StmtPtr p_var_decl()
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
				return StmtPtr(new VarDecl(access, storage, std::move(type),
					std::move(name), std::move(expr), spos, end()));
			}
			else
			{
				return StmtPtr(new VarDecl(access, storage, std::move(type),
					std::move(name), ExprPtr(nullptr), spos, end()));
			}
		}
	}
	index = save_index;
	return StmtPtr(nullptr);
}

// func_def ::= specifiers type_ident ident_expr '(' arg_list ')' compound_stmt .
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
				StmtPtr stmt(p_compound_stmt());
				if (!stmt)
				{
					std::stringstream ss;
					ss << "expected a compound statement after function declaration, got `"
					   << text() << " (" << current() << ")";
					SYNTAX_ERROR(ss.str());
				}
				return StmtPtr(new FuncDef(access, storage, std::move(type),
					std::move(name), std::move(args), std::move(stmt), spos, end()));
			}
		}
	}
	index = save_index;
	return StmtPtr(nullptr);
}

// arg_list ::= var_decl { ',' var_decl } .
void p_arg_list(StmtList& lst)
{
	do
	{
		StmtPtr stmt(p_var_decl());
		if (!stmt)
			break;
		lst.push_back(std::move(stmt));
	}
	while (ACCEPT(','));
}

// ident_list ::= ident_expr { ',' ident_expr } .
void p_ident_list(ExprList& lst)
{
	do
	{
		ExprPtr base(p_ident_expr());
		if (!base)
			break;
		lst.emplace_back(std::move(base));
	}
	while (ACCEPT(','));
}

// class_def ::= CLASS IDENT [':' ident_list ] '{' stmt_list '}' .
StmtPtr p_class_def()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::CLASS))
	{
		IdentPtr name(p_ident_expr());
		ExprList bases;
		if (ACCEPT(':'))
		{
			p_ident_list(bases);
			if (bases.empty())
			{
				std::stringstream ss;
				ss << "expected one or more base class identifiers after `:', "
				   << "got `" << text() << "' (" << current() << ")";
				SYNTAX_ERROR(ss.str());
			}
		}
		StmtPtr stmt(p_compound_stmt(true));
		if (!stmt)
		{
			std::stringstream ss;
			ss << "expected compound statement after class declaration, "
			   << "got `" << text() << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		return StmtPtr(new ClassDef(std::move(name), std::move(bases),
			std::move(stmt), spos, end()));
	}
	return StmtPtr(nullptr);
}

// case ::= CASE expr ':' .
StmtPtr p_case()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::CASE))
	{
		ExprPtr expr(p_expr());
		EXPECT(':');
		return StmtPtr(new CaseStmt(std::move(expr), p_stmt(), spos, end()));
	}
	return StmtPtr(nullptr);
}

// default ::= DEFAULT ':' .
StmtPtr p_default()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::DEFAULT))
	{
		EXPECT(':');
		return StmtPtr(new CaseStmt(ExprPtr(nullptr), p_stmt(), spos, end()));
	}
	return StmtPtr(nullptr);
}

// case_list ::= { (case | default) } .
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
		while (ACCEPT(';'))
			;
	}
}

// switch_stmt ::= SWITCH '(' expr ')' '{' case_list '}' .
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

// if_stmt ::= IF '(' expr ')' stmt [ ELSE stmt ] .
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

// return_stmt ::= RETURN [ expr ] .
StmtPtr p_return_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::RETURN))
		return StmtPtr(new ReturnStmt(std::move(p_expr()), spos, end()));
	return StmtPtr(nullptr);
}

// break_stmt ::= BREAK .
StmtPtr p_break_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::BREAK))
		return StmtPtr(new BreakStmt(spos, end()));
	return StmtPtr(nullptr);
}

// compound_stmt ::= '{' p_stmt_list '}'
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

// stmt ::= alias
//       | import
//       | func_def
//       | var_decl
//       | class_def
//       | return_stmt
//       | if_stmt
//       .
StmtPtr p_stmt(bool top_level=false)
{
#define TRY_STMT(name)         \
	do {                       \
		StmtPtr stmt(p_##name()); \
		if (stmt)              \
			return stmt;       \
	} while (0)

	TRY_STMT(alias);
	TRY_STMT(import_stmt);
	TRY_STMT(class_def);

	if (!top_level)
	{
		TRY_STMT(return_stmt);
		TRY_STMT(if_stmt);
		TRY_STMT(switch_stmt);
		TRY_STMT(compound_stmt);
		TRY_STMT(break_stmt);
	}

	TRY_STMT(func_def);
	TRY_STMT(var_decl);

	return StmtPtr(nullptr);

#undef TRY_STMT
}

// stmt_list ::= stmt { stmt } .
void p_stmt_list(StmtList& lst, bool top_level=false)
{
	while (true)
	{
		StmtPtr stmt(p_stmt(top_level));
		if (!stmt)
			break;
		lst.push_back(std::move(stmt));
		while (ACCEPT(';'))
			;
	}
	while (ACCEPT(';'))
		;
}

// number_expr ::= ??INTEGER_CONSTANTS?? | FCONST .
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

// paren_expr ::= '(' expr ')' .
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

// ident_expr ::= IDENT { '.' IDENT } .
IdentPtr p_ident_expr()
{
	if (current() == Token::IDENT)
	{
		SourcePosition spos = start();
		std::u32string name(text());
		while (ACCEPT(Token::IDENT))
		{
			if (ACCEPT('.'))
			{
				if (current() != Token::IDENT)
				{
					std::stringstream ss;
					ss << "expecting an identifier after `.', got `" << text()
					   << "' (" << current() << ")";
					SYNTAX_ERROR(ss.str());
				}
				name += U".";
				name += text();
				continue;
			}
			else
				break;
		}
		return IdentPtr(new Ident(name, spos, end()));
	}
	return IdentPtr(nullptr);
}

// strlit_expr ::= STR_LIT { STR_LIT } .
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

// primary_expr ::= ident_expr
//               | number_expr
//               | strlit_expr
//               | paren_expr
//               .
ExprPtr p_primary_expr()
{
#define TRY_EXPR(name) \
	do { ExprPtr expr(p_##name()); if (expr) { return expr; } } while(0)

	TRY_EXPR(ident_expr);
	TRY_EXPR(number_expr);
	TRY_EXPR(strlit_expr);
	TRY_EXPR(paren_expr);

{
	std::stringstream ss;
	if (text().empty())
		ss << "syntax error";
	else
		ss << "unexpected token `" << text() << "' (" << current() << ")";
	ss << ", expecting primary expression";
	SYNTAX_ERROR(ss.str());
}

	return ExprPtr(nullptr);

#undef TRY_EXPR
}

// expr ::= primary_expr [ bin_op_rhs ] .
ExprPtr p_expr()
{
	SourcePosition spos = start();
	ExprPtr lhs(p_primary_expr());
	if (!lhs)
		return ExprPtr(nullptr);
	return p_bin_op_rhs(0, lhs.release(), spos);
}

// bin_op_rhs ::= { ??OPERATORS?? primary_expr } .
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

SyntaxError::SyntaxError(const char *what,
                         const std::string& filename,
                         SourceLocation location,
                         const std::string& message)
	: std::runtime_error(what),
	  fn(filename),
	  location(location),
	  msg(message)
{
}


enum Color
{
	CLR_BLACK,
	CLR_RED,
	CLR_GREEN,
	CLR_YELLOW,
	CLR_BLUE,
	CLR_MAGENTA,
	CLR_CYAN,
	CLR_WHITE,
};

void format_exception(std::ostream& stream, SyntaxError& err)
{
	stream << "\x1B[31merror\x1B[0m:\x1B[33m" << err.fn << "\x1B[0m:";
	if (err.location.line.start != err.location.line.end)
		stream << "\x1B[35m" << err.location.line.start + 1 << "-" << err.location.line.end + 1 << "\x1B[0m:";
	else
		stream << "\x1B[35m" << err.location.line.start + 1 << "\x1B[0m:";
	if (err.location.column.start != err.location.column.start)
		stream << "\x1B[36m" << err.location.column.start << "-" << err.location.column.end;
	else
		stream << "\x1B[36m" << err.location.column.start;
	if (!err.msg.empty())
		stream << "\x1B[0m: " << err.msg << "\n";
	else
		stream << "\x1B[0m\n";
}

} // namespace Soda
