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
Stmt p_import_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::IMPORT))
		return Stmt(new Import(p_ident_expr(), spos, end()));
	return Stmt(nullptr);
}

// alias ::= ALIAS IDENT '=' IDENT .
Stmt p_alias()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::ALIAS))
	{
		Ident type(p_ident_expr());
		EXPECT('=');
		return Stmt(new Alias(std::move(type), p_ident_expr(), spos, end()));
	}
	return Stmt(nullptr);
}

// var_decl ::= VAR [ ':' IDENT ] IDENT { '=' expr } .
Stmt p_var_decl()
{
	SourcePosition spos = start();
	Ident type(nullptr);
	if (ACCEPT(Token::VAR))
	{
		if (ACCEPT(':'))
		{
			Ident t(p_ident_expr());
			if (t)
				type = std::move(t);
			else
			{
				std::stringstream ss;
				ss << "expected a type name identifier after `:', got `"
				   << text() << " (" << current() << ")";
				SYNTAX_ERROR(ss.str());
			}
		}
		Ident name(p_ident_expr());
		if (ACCEPT('='))
		{
			Expr expr(p_expr());
			if (!expr)
			{
				std::stringstream ss;
				ss << "expected expression as right hand side of initial " <<
				      "assignment but got `" << text() << "' " <<
				      "(" << current() << ")";
				SYNTAX_ERROR(ss.str());
			}
			else
			{
				return Stmt(new VarDecl(std::move(type), std::move(name),
					std::move(expr), spos, end()));
			}
		}
		else
		{
			return Stmt(new VarDecl(std::move(type), std::move(name),
				Expr(nullptr), spos, end()));
		}
	}
	return Stmt(nullptr);
}

// func_def ::= FUN IDENT '(' arg_list ')' '{' stmt_list '}' .
Stmt p_func_def()
{
	SourcePosition spos = start();
	Ident type(nullptr);
	if (ACCEPT(Token::FUN))
	{
		if (ACCEPT(':'))
		{
			Ident t(p_ident_expr());
			if (t)
				type = std::move(t);
			else
			{
				std::stringstream ss;
				ss << "expected a type name identifier after `:', got `"
				   << text() << " (" << current() << ")";
				SYNTAX_ERROR(ss.str());
			}
		}
		Ident ident(p_ident_expr());
		EXPECT('(');
		StmtList args;
		p_arg_list(args);
		EXPECT(')');
		return Stmt(new FuncDef(std::move(type), std::move(ident), std::move(args),
			p_compound_stmt(), spos, end()));
	}
	return Stmt(nullptr);
}

// arg_list ::= argument { , argument } .
void p_arg_list(StmtList& lst)
{
	do
	{
		Stmt stmt(p_argument());
		if (!stmt)
			break;
		lst.push_back(std::move(stmt));
	}
	while (ACCEPT(','));
}

// argument ::= IDENT [ '=' expr ] .
Stmt p_argument()
{
	SourcePosition spos = start();
	Ident type(nullptr);
	if (current() == Token::IDENT)
	{
		Ident name(p_ident_expr());
		if (current() == Token::IDENT)
		{
			type = std::move(name);
			name = std::move(p_ident_expr());
		}
		if (ACCEPT('='))
		{
			Expr expr(p_expr());
			if (!expr)
			{
				std::stringstream ss;
				ss << "expected expression as right hand side of " <<
				      "default argument specification, got `" <<
				      text() << "' (" << current() << ")";
				SYNTAX_ERROR(ss.str());
			}
			return Stmt(new Argument(std::move(type), std::move(name), std::move(expr), spos, end()));
		}
		else
			return Stmt(new Argument(std::move(type), std::move(name), Expr(nullptr), spos, end()));
	}
	return Stmt(nullptr);
}

// class_def ::= CLASS IDENT '{' stmt_list '}' .
Stmt p_class_def()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::CLASS))
		return Stmt(new ClassDef(p_ident_expr(), p_compound_stmt(true), spos, end()));
	return Stmt(nullptr);
}

// case ::= CASE expr ':' .
Stmt p_case()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::CASE))
	{
		Expr expr(p_expr());
		EXPECT(':');
		return Stmt(new CaseStmt(std::move(expr), p_stmt(), spos, end()));
	}
	return Stmt(nullptr);
}

// default ::= DEFAULT ':' .
Stmt p_default()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::DEFAULT))
	{
		EXPECT(':');
		return Stmt(new CaseStmt(Expr(nullptr), p_stmt(), spos, end()));
	}
	return Stmt(nullptr);
}

// case_list ::= { (case | default) } .
void p_case_list(StmtList& case_list)
{
	while (true)
	{
		Stmt case_stmt(p_case());
		if (case_stmt)
			case_list.push_back(std::move(case_stmt));
		else
		{
			Stmt def_stmt(p_default());
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
Stmt p_switch_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::SWITCH))
	{
		EXPECT('(');
		Expr expr(p_expr());
		EXPECT(')');
		EXPECT('{');
		StmtList cases;
		p_case_list(cases);
		EXPECT('}');
		return Stmt(new SwitchStmt(std::move(expr), std::move(cases), spos, end()));
	}
	return Stmt(nullptr);
}

// if_stmt ::= IF '(' expr ')' stmt [ ELSE stmt ] .
Stmt p_if_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::IF))
	{
		EXPECT('(');
		Expr expr(p_expr());
		if (!expr)
		{
			std::stringstream ss;
			ss << "expected condition expression in `if', got " <<
			      "`" << text() << "' (" << current() << ")";
			SYNTAX_ERROR(ss.str());
		}
		EXPECT(')');
		Stmt if_stmt(p_stmt());
		if (ACCEPT(Token::ELSE))
		{
			Stmt else_stmt(p_stmt());
			return Stmt(new IfStmt(std::move(expr),
			                       std::move(if_stmt),
			                       std::move(else_stmt), spos, end()));
		}
		else
		{
			return Stmt(new IfStmt(std::move(expr),
			                       std::move(if_stmt),
			                       Stmt(nullptr), spos, end()));
		}
	}
	return Stmt(nullptr);
}

// return_stmt ::= RETURN [ expr ] .
Stmt p_return_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::RETURN))
		return Stmt(new ReturnStmt(std::move(p_expr()), spos, end()));
	return Stmt(nullptr);
}

// break_stmt ::= BREAK .
Stmt p_break_stmt()
{
	SourcePosition spos = start();
	if (ACCEPT(Token::BREAK))
		return Stmt(new BreakStmt(spos, end()));
	return Stmt(nullptr);
}

// compound_stmt ::= '{' p_stmt_list '}'
Stmt p_compound_stmt(bool top_level=false)
{
	SourcePosition spos = start();
	if (ACCEPT('{'))
	{
		StmtList stmts;
		p_stmt_list(stmts, top_level);
		EXPECT('}');
		return Stmt(new CompoundStmt(std::move(stmts), spos, end()));
	}
	return Stmt(nullptr);
}

// stmt ::= alias
//       | import
//       | var_decl
//       | func_def
//       | class_def
//       | return_stmt
//       | if_stmt
//       .
Stmt p_stmt(bool top_level=false)
{
#define TRY_STMT(name)         \
	do {                       \
		Stmt stmt(p_##name()); \
		if (stmt)              \
			return stmt;       \
	} while (0)

	TRY_STMT(alias);
	TRY_STMT(import_stmt);
	TRY_STMT(var_decl);
	TRY_STMT(func_def);
	TRY_STMT(class_def);

	if (!top_level)
	{
		TRY_STMT(return_stmt);
		TRY_STMT(if_stmt);
		TRY_STMT(switch_stmt);
		TRY_STMT(compound_stmt);
		TRY_STMT(break_stmt);
	}

	return Stmt(nullptr);

#undef TRY_STMT
}

// stmt_list ::= stmt { stmt } .
void p_stmt_list(StmtList& lst, bool top_level=false)
{
	while (true)
	{
		Stmt stmt(p_stmt(top_level));
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
Expr p_number_expr()
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
		return Expr(nullptr);

	try
	{
		if (base == 0)
		{
			Expr expr(new Float(text(), spos, end()));
			next();
			return expr;
		}
		else
		{
			Expr expr(new Integer(text(), base, spos, end()));
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

	return Expr(nullptr);
}

// paren_expr ::= '(' expr ')' .
Expr p_paren_expr()
{
	if (ACCEPT('('))
	{
		Expr expr(p_expr());
		if (expr)
		{
			EXPECT(')');
			return expr;
		}
	}
	return Expr(nullptr);
}

// ident_expr ::= IDENT { '.' IDENT } .
Ident p_ident_expr()
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
		return Ident(new IdentImpl(name, spos, end()));
	}
	return Ident(nullptr);
}

// strlit_expr ::= STR_LIT { STR_LIT } .
Expr p_strlit_expr()
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
		return Expr(new StrLit(txt, spos, end()));
	}
	return Expr(nullptr);
}

// primary_expr ::= ident_expr
//               | number_expr
//               | strlit_expr
//               | paren_expr
//               .
Expr p_primary_expr()
{
#define TRY_EXPR(name) \
	do { Expr expr(p_##name()); if (expr) { return expr; } } while(0)

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

	return Expr(nullptr);

#undef TRY_EXPR
}

// expr ::= primary_expr [ bin_op_rhs ] .
Expr p_expr()
{
	SourcePosition spos = start();
	Expr lhs(p_primary_expr());
	if (!lhs)
		return Expr(nullptr);
	return p_bin_op_rhs(0, lhs.release(), spos);
}

// bin_op_rhs ::= { ??OPERATORS?? primary_expr } .
Expr p_bin_op_rhs(int expr_prec, ExprImpl* lhs, SourcePosition spos)
{
	while (true)
	{
		int tok_prec = get_prec();
		if (tok_prec < expr_prec)
			return Expr(lhs);
		char32_t op = current();
		next();
		Expr rhs(p_primary_expr());
		if (!rhs)
			return nullptr;
		int next_prec = get_prec();
		if (tok_prec < next_prec)
		{
			rhs.reset(p_bin_op_rhs(tok_prec + 1, rhs.release(), spos).release());
			if (!rhs)
				return nullptr;
		}
		lhs = new BinOp(op, Expr(lhs), std::move(rhs), spos, end());
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
