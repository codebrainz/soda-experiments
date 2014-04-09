#include "parser.h"
#include "lexer.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
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
		ss << "unexpected token `" << kind << "', expecting `" << last << "'";
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

// stmt ::= alias | var_decl | func_def .
Stmt *p_stmt()
{
	Stmt *stmt = nullptr;
	if ((stmt = p_alias()))
		return stmt;
	else if ((stmt = p_var_decl()))
		return stmt;
	else if ((stmt = p_func_def()))
		return stmt;
	else if ((stmt = p_return_stmt()))
		return stmt;
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

// func_def ::= FUN IDENT '(' arg_list ')' '{' stmt_list '}' [';'] .
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
		StmtList stmts = p_stmt_list();
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

// return_stmt ::= RETURN [ expr ] [';'] .
Stmt *p_return_stmt()
{
	Stmt *stmt = nullptr;
	if (last == Token::RETURN)
	{
		next();
		Expr *expr = p_expr();
		stmt = new ReturnStmt(expr);
		accept(';');
	}
	return stmt;
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

// number_expr: *_ICONST | FCONST
Expr *p_number_expr()
{
	Expr *exp;
	switch (last)
	{
		case Token::DEC_ICONST:
			exp = new Integer(text(), 10);
			break;
		case Token::HEX_ICONST:
			exp = new Integer(text(), 16);
			break;
		case Token::OCT_ICONST:
			exp = new Integer(text(), 8);
			break;
		case Token::BIN_ICONST:
			exp = new Integer(text(), 2);
			break;
		case Token::FCONST:
			exp = new Float(text());
			break;
		default: // error
			exp = nullptr;
			break;
	}
	next();
	return exp;
}

// paren_expr: '(' expr ')'
Expr *p_paren_expr()
{
	next();
	Expr *exp = p_expr();
	if (!exp)
		return nullptr;
	expect(')');
	return exp;
}

// ident_expr: IDENT
Expr *p_ident_expr()
{
	Expr *exp = new Ident(text());
	next();
	return exp;
}

// strlit_expr ::= STR_LIT { STR_LIT }
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

// primary_expr: ident_expr
//             | number_expr
//             | paren_expr
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
			std::cerr << "Unexpected token '" << last << "'" << std::endl;
			return nullptr;
	}
}

// expr: primary_expr bin_op_rhs
Expr *p_expr()
{
	Expr *lhs = p_primary_expr();
	if (!lhs)
		return nullptr;
	return p_bin_op_rhs(0, lhs);
}

// bin_op_rhs: (OPS primary_expr)*
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

#if 0

Stmt* p_var_decl()
{
	Stmt *stmt = nullptr;
	if (last == Token::IDENT) // skip over type
	{
		Ident *type = new Ident(text());
		save();
		if (accept(Token::IDENT)) // skip over the identifier
		{
			Ident *name = new Ident(text());
			if (accept(Token::EQUAL)) // check if have init assignment
			{
				// have var decl with init assignment
				Expr *exp = p_expr();
				if (!exp) // todo: throw exception?
				{
					delete type; delete name;
					restore();
					return nullptr;
				}
				expect(';');
				stmt = new VarDecl(type, name exp);
			}
			else if (accept(Token::LPAREN))
			{
				// have function decl and/or def
				while (true)
				{
					Ident *arg_type = new Ident(text());
					if (accept(Token::IDENT)) // argument type
					{

						expect(Token::IDENT);
					}
					else
					{
						delete arg_type;
						break;
					}
				}
				accept(';'); // allow ; after
			}
			else
			{
				// have a var decl without init assignment
				expect(';'); // require ; after
				stmt = new VarDecl(type, name);
			}
		}
		else
		{
			delete type;
			restore();
		}
	}
	return stmt;
}

Stmt* p_func_decl()
{
	Stmt *stmt = nullptr;
	if (last == Token::IDENT)
	{
		Ident *type = new Ident(text());
		save();
		if (accept(Token::IDENT))
		{
			Ident *name = new Ident(text());
			if (expect
		}
	}
}

Stmt* p_stmt()
{
	Stmt *stmt = nullptr;
#if 0
	if ((stmt = p_var_decl()))
		return stmt;
	if ((stmt = p_import()))
		return stmt;
	if ((stmt = p_compound_stmt()))
		return stmt;
	// ...
#endif
	return stmt;
}

#endif // #if 0

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
	stream << "error: " << err.fn << ":";
#if 0
	if (err.pos_start != err.pos_end)
		stream << err.pos_start << "-" << err.pos_end << ":";
	else
		stream << err.pos_start << ":";
#endif
	if (err.line_start != err.line_end)
		stream << err.line_start << "-" << err.line_end << ":";
	else
		stream << err.line_start << ":";
	if (err.col_start != err.col_end)
		stream << err.col_start << "-" << err.col_end << ": ";
	else
		stream << err.col_start;
	if (!err.msg.empty())
		stream << ": " << err.msg << "\n";
	else
		stream << "\n";
}

} // namespace Soda
