#include "sodainc.h" // pch
#include "parser.h"
#include "lexer.h"
#include <deque>
#include <cassert>

namespace Soda
{

#define SYNTAX_ERROR(msg)                                      \
		throw Soda::SyntaxError(                               \
			"syntax error", tu.fn,                             \
			lex.token.position.start, lex.token.position.end,  \
			lex.token.line.start, lex.token.line.end,          \
			lex.token.column.start, lex.token.column.end, msg)

struct Parser
{

Lexer& lex;
TU& tu;
Token::Kind last;
std::deque<Token> saved_tokens;
bool saving;

Parser(Lexer& lex, TU& tu)
	: lex(lex), tu(tu), last(lex.next()), saving(false)
{
}

const std::u32string& text() const
{
	return lex.token.text;
}

// begin recording tokens so we can backtrack later (try not to use this)
// clears any existing saved tokens
void save()
{
	saved_tokens.clear();
	saving = true;
}

// next token should come from oldest saved token after calling this
void restore()
{
	saving = false;
}

// clear the saved tokens and stop recording future ones
void reset()
{
	saved_tokens.clear();
	saving = false;
}

// update 'last' token and return the kind of it (ie. advance in token stream)
Token::Kind next()
{
	if (saving)
		saved_tokens.push_back(lex.token);
	else if (!saved_tokens.empty())
	{
		lex.token = saved_tokens.front();
		saved_tokens.pop_front();
		return (last = lex.token.kind);
	}
	return (last = lex.next());
}

bool accept(char32_t ch)
{
	return accept((Token::Kind)ch);
}

// if the 'last' token is what is expected, advance and return true
bool accept(Token::Kind kind)
{
	if (last == kind)
	{
		next();
		return true;
	}
	return false;
}

void expect(char32_t ch)
{
	expect((Token::Kind)ch);
}

// if the 'last' token is what is expected, advance, otherwise throw exception
void expect(Token::Kind kind)
{
	if (!accept(kind))
	{
		std::stringstream ss;
		ss << "unexpected token `" << last << "', expecting `" << kind << "'";
		SYNTAX_ERROR(ss.str());
	}
}

int get_prec()
{
	// TODO: add rest
	switch (last)
	{
		case '<':
		case '>':
			return 10;
		case '+':
		case '-':
			return 20;
		case '*':
		case '/':
		case '%':
			return 40;
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

	while (true)
	{
		Stmt *stmt = p_stmt();
		if (!stmt)
		{
			if (last != Token::END)
			{
				std::stringstream ss;
				ss << "unexpected token `" << last << "' encountered";
				SYNTAX_ERROR(ss.str());
				next();
			}
			break;
		}
		tu.stmts.emplace_back(stmt);
	}
}

// stmt ::= alias
//        | import_stmt
//        | var_decl
//        | func_def
//        | class_def
//        .
Stmt *p_stmt()
{
	Stmt *stmt = nullptr;
	if ((stmt = p_alias()))
		return stmt;
	else if ((stmt = p_import_stmt()))
		return stmt;
	else if ((stmt = p_var_decl()))
		return stmt;
	else if ((stmt = p_func_def()))
		return stmt;
	else if ((stmt = p_class_def()))
		return stmt;
	return stmt;
}

// import_stmt ::= IMPORT IDENT [';'] .
Stmt *p_import_stmt()
{
	Stmt *stmt = nullptr;
	if (last == Token::IMPORT)
	{
		next();
		Ident *ident = new Ident(text());
		expect(Token::IDENT);
		stmt = new Import(ident);
		accept(';');
	}
	return stmt;
}

// alias ::= ALIAS IDENT '=' IDENT [';'] .
Stmt *p_alias()
{
	Stmt *stmt = nullptr;
	if (accept(Token::ALIAS))
	{
		Ident *type = new Ident(text());
		expect(Token::IDENT);
		expect('=');
		Ident *alias = new Ident(text());
		expect(Token::IDENT);
		accept(';');
		stmt = new Alias(type, alias);
	}
	return stmt;
}

// var_decl ::= VAR IDENT { '=' expr } [';'] .
Stmt *p_var_decl()
{
	Stmt *stmt = nullptr;
	if (accept(Token::VAR))
	{
		Ident *name = new Ident(text());
		expect(Token::IDENT);
		if (accept('='))
		{
			Expr *exp = p_expr();
			assert(exp);
			stmt = new VarDecl(new Ident(U"__placeholder__"), name, exp);
		}
		else
			stmt = new VarDecl(new Ident(U"__placeholder__"), name);
		accept(';');
	}
	return stmt;
}

// func_def ::= FUN IDENT '(' arg_list ')' '{' local_stmt_list '}' [';'] .
Stmt *p_func_def()
{
	Stmt *stmt = nullptr;
	if (accept(Token::FUN))
	{
		Ident *name = new Ident(text());
		expect(Token::IDENT);
		expect('(');
		StmtList args = p_arg_list();
		expect(')');
		expect('{');
		StmtList stmts = p_local_stmt_list();
		expect('}');
		accept(';');
		stmt = new FuncDef(new Ident(U"__placeholder__"), name, args, stmts);
	}
	return stmt;
}

// arg_list ::= IDENT [ '=' expr ] { ',' IDENT [ '=' expr ] } .
StmtList p_arg_list()
{
	StmtList lst;
	while (last == Token::IDENT)
	{
		Ident *name = new Ident(text());
		next();
		Expr *expr = nullptr;
		if (accept('='))
		{
			expr = p_expr();
			assert(expr);
		}
		lst.emplace_back(new Argument(name, expr));
		if (!accept(','))
			break;
	}
	return lst;
}

// class_def ::= CLASS IDENT '{' stmt_list '}' [';'] .
Stmt *p_class_def()
{
	Stmt *stmt = nullptr;
	if (last == Token::CLASS)
	{
		next();
		Ident *name = new Ident(text());
		expect(Token::IDENT);
		expect('{');
		StmtList stmts = p_stmt_list();
		expect('}');
		accept(';');
		stmt = new ClassDef(name, stmts);
	}
	return stmt;
}

// if_stmt ::= IF '(' expr ')'
//             { ELSE IF '(' expr ')' '{' local_stmt_list '}' }
//             [ ELSE '{' local_stmt_list '}' ] .
Stmt *p_if_stmt()
{
	Stmt *stmt = nullptr;
	if (last == Token::IF)
	{
		next();

		expect('(');
		Expr *if_expr = p_expr();
		expect(')');

		expect('{');
		StmtList if_stmts = p_local_stmt_list();
		expect('}');

		StmtList elseif_stmts; // IfStmt sequence
		StmtList else_stmts;

		while (last == Token::ELSE || last == Token::ELIF)
		{
			bool is_elif = (last == Token::ELIF);
			next();
			if (is_elif || accept(Token::IF))
			{
				expect('(');
				Expr *elseif_expr = p_expr();
				expect(')');
				expect('{');
				StmtList elseif_body_stmts = p_local_stmt_list();
				expect('}');
				elseif_stmts.emplace_back(new IfStmt(elseif_expr, elseif_body_stmts));
				continue;
			}
			else
			{
				expect('{');
				else_stmts = p_local_stmt_list();
				expect('}');
				break;
			}
		}

		stmt = new IfStmt(if_expr, if_stmts, elseif_stmts, else_stmts);
	}
	return stmt;
}

// return_stmt ::= RETURN [ expr ] [';'] .
Stmt *p_return_stmt()
{
	Stmt *stmt = nullptr;
	if (last == Token::RETURN)
	{
		next();
		Expr *expr = p_expr();
		stmt = new ReturnStmt(expr);
	}
	return stmt;
}

// local_stmt ::= alias
//             | import_stmt
//             | var_decl
//             | func_def
//             | return_stmt
//             | class_def
//             | if_stmt
//             .
Stmt *p_local_stmt()
{
	Stmt *stmt = nullptr;
	if ((stmt = p_alias()))
		return stmt;
	else if ((stmt = p_import_stmt()))
		return stmt;
	else if ((stmt = p_var_decl()))
		return stmt;
	else if ((stmt = p_func_def()))
		return stmt;
	else if ((stmt = p_return_stmt()))
		return stmt;
	else if ((stmt = p_class_def()))
		return stmt;
	else if ((stmt = p_if_stmt()))
		return stmt;
	return stmt;
}

// local_stmt_list ::= local_stmt { local_stmt } .
StmtList p_local_stmt_list()
{
	StmtList lst;
	Stmt *stmt;
	while ((stmt = p_local_stmt()))
	{
		lst.emplace_back(stmt);
		accept(';');
	}
	accept(';');
	return lst;
}

// stmt_list ::= stmt { stmt } .
StmtList p_stmt_list()
{
	StmtList lst;
	Stmt *stmt;
	while ((stmt = p_stmt()))
	{
		lst.emplace_back(stmt);
		accept(';');
	}
	accept(';');
	return lst;
}

// number_expr ::= ??INTEGER_CONSTANTS?? | FCONST .
Expr *p_number_expr()
{
	Expr *exp;
	int base;

	switch (last)
	{
		case Token::DEC_ICONST: base = 10; break;
		case Token::HEX_ICONST: base = 16; break;
		case Token::OCT_ICONST: base = 8;  break;
		case Token::BIN_ICONST: base = 2;  break;
		case Token::FCONST:     base = 0;  break;
		default:                base = -1; break;
	}

	if (base < 0)
		return nullptr;

	try
	{
		if (base == 0)
			exp = new Float(text());
		else
			exp = new Integer(text(), base);
		next();
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

	return exp;
}

// paren_expr ::= '(' expr ')' .
Expr *p_paren_expr()
{
	next();
	Expr *exp = p_expr();
	if (!exp)
		return nullptr;
	expect(')');
	return exp;
}

// ident_expr ::= IDENT .
Expr *p_ident_expr()
{
	Expr *exp = new Ident(text());
	next();
	return exp;
}

// strlit_expr ::= STR_LIT { STR_LIT } .
Expr *p_strlit_expr()
{
	Expr *exp = nullptr;
	if (last == Token::STR_LIT)
	{
		std::u32string text;
		do
		{
			text += lex.token.text.substr(1, lex.token.text.size() - 2);
			next();
		} while (last == Token::STR_LIT);
		exp = new StrLit(text);
	}
	return exp;
}

// primary_expr ::= ident_expr | number_expr | paren_expr .
Expr *p_primary_expr()
{
	switch (last)
	{
		case Token::IDENT:
			return p_ident_expr();
		case Token::DEC_ICONST:
		case Token::HEX_ICONST:
		case Token::OCT_ICONST:
		case Token::BIN_ICONST:
		case Token::FCONST:
			return p_number_expr();
		case Token::STR_LIT:
			return p_strlit_expr();
		case '(':
			return p_paren_expr();
		default: // error
		{
			std::stringstream ss;
			if (text().empty())
				ss << "syntax error";
			else
				ss << "unexpected token `" << text() << "' (" << last << ")";
			ss << ", expecting primary expression";
			SYNTAX_ERROR(ss.str());
			return nullptr;
		}
	}
}

// expr ::= primary_expr bin_op_rhs .
Expr *p_expr()
{
	Expr *lhs = p_primary_expr();
	if (!lhs)
		return nullptr;
	return p_bin_op_rhs(0, lhs);
}

// bin_op_rhs ::= { ??OPERATORS?? primary_expr } .
Expr *p_bin_op_rhs(int expr_prec, Expr *lhs)
{
	while (true)
	{
		int tok_prec = get_prec();
		if (tok_prec < expr_prec)
			return lhs;
		char32_t op = last;
		next();
		Expr *rhs = p_primary_expr();
		if (!rhs)
			return nullptr;
		int next_prec = get_prec();
		if (tok_prec < next_prec)
		{
			rhs = p_bin_op_rhs(tok_prec + 1, rhs);
			if (!rhs)
				return nullptr;
		}
		lhs = new BinOp(op, lhs, rhs);
	}
}

//////////////////////////////////////////////////////////////////////////////

}; // struct Parser

void parse(TU& tu, std::istream& stream)
{
	Lexer lex(stream);
	Parser p(lex, tu);
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
                         int pos_start,  int pos_end,
                         int line_start, int line_end,
                         int col_start,  int col_end,
                         const std::string& message)
	: std::runtime_error(what),
	  fn(filename),
	  pos_start(pos_start), pos_end(pos_end),
	  line_start(line_start), line_end(line_end),
	  col_start(col_start), col_end(col_end),
	  msg(message)
{
}

void format_exception(std::ostream& stream, SyntaxError& err)
{
	stream << "\x1B[31merror\x1B[0m:\x1B[33m" << err.fn << "\x1B[0m:";
	if (err.line_start != err.line_end)
		stream << "\x1B[35m" << err.line_start + 1 << "-" << err.line_end + 1 << "\x1B[0m:";
	else
		stream << "\x1B[35m" << err.line_start + 1 << "\x1B[0m:";
	if (err.col_start != err.col_end)
		stream << "\x1B[36m" << err.col_start << "-" << err.col_end;
	else
		stream << "\x1B[36m" << err.col_start;
	if (!err.msg.empty())
		stream << "\x1B[0m: " << err.msg << "\n";
	else
		stream << "\x1B[0m\n";
}

} // namespace Soda
