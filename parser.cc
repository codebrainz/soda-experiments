#include "sodainc.h" // pch
#include "parser.h"
#include "lexer.h"
#include <deque>
#include <cassert>
#include <stack>

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
std::stack<SourceLocation> loc_stack;

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

void push_loc() noexcept
{
	//std::cerr << "Pushing location" << std::endl;
	try { loc_stack.emplace(lex.token); }
	catch (...) {}
}

void pop_loc(Node *node=nullptr) noexcept
{
	//std::cerr << "Popping location" << std::endl;
	if (node)
	{
		try
		{
			node->location = loc_stack.top();
			loc_stack.pop();
		}
		catch (...) {}
	}
	else
	{
		try { loc_stack.pop(); }
		catch (...) {}
	}
}

//////////////////////////////////////////////////////////////////////////////

// tu ::= { stmt } .
void p_tu(TU& tu)
{
	p_stmt_list(tu.stmts, true);
}

// import_stmt ::= IMPORT IDENT [';'] .
Stmt *p_import_stmt()
{
	Stmt *stmt = nullptr;
	if (last == Token::IMPORT)
	{
		next();
		AstNode<Ident> ident(p_ident_expr());
		stmt = Import::create(ident);
	}
	return stmt;
}

// alias ::= ALIAS IDENT '=' IDENT [';'] .
Stmt *p_alias()
{
	Stmt *stmt = nullptr;
	if (accept(Token::ALIAS))
	{
		AstNode<Ident> type(p_ident_expr());
		expect('=');
		AstNode<Ident> alias(p_ident_expr());
		stmt = Alias::create(type, alias);
	}
	return stmt;
}

// var_decl ::= VAR IDENT { '=' expr } [';'] .
Stmt *p_var_decl()
{
	Stmt *stmt = nullptr;
	if (accept(Token::VAR))
	{
		AstNode<Ident> ident(p_ident_expr());
		if (accept('='))
		{
			AstNode<Expr> expr(p_expr());
			if (!expr)
			{
				std::stringstream ss;
				ss << "expected expression as right hand side of initial " <<
				      "assignment but got `" << lex.token.text << "' " <<
				      "(" << last << ")";
				SYNTAX_ERROR(ss.str());
			}
			else
				stmt = VarDecl::create(ident, expr);
		}
		else
			stmt = VarDecl::create(ident);
	}
	return stmt;
}

// func_def ::= FUN IDENT '(' arg_list ')' '{' stmt_list '}' [';'] .
Stmt *p_func_def()
{
	Stmt *stmt = nullptr;
	if (accept(Token::FUN))
	{
		AstNode<Ident> ident(p_ident_expr());
		expect('(');
		StmtList args; p_arg_list(args);
		expect(')');
		expect('{');
		StmtList stmts; p_stmt_list(stmts);
		expect('}');
		stmt = FuncDef::create(ident, args, stmts);
	}
	return stmt;
}

// arg_list ::= argument { , argument } .
void p_arg_list(StmtList& lst)
{
	Stmt *arg;
	while ((arg = p_argument()))
	{
		try
		{
			bool has_comma = accept(',');
			lst.emplace_back(arg);
			if (!has_comma)
				break;
		}
		catch (...)
		{
			delete arg;
			throw;
		}
	}
}

// argument ::= IDENT [ '=' expr ] .
Stmt *p_argument()
{
	Stmt *stmt = nullptr;
	if (last == Token::IDENT)
	{
		AstNode<Ident> name(p_ident_expr());
		if (accept('='))
		{
			AstNode<Expr> expr(p_expr());
			if (!expr)
			{
				std::stringstream ss;
				ss << "expected expression as right hand side of " <<
				      "default argument specification, got `" <<
				      lex.token.text << "' (" << last << ")";
				SYNTAX_ERROR(ss.str());
			}
			stmt = Argument::create(name, expr);
		}
		else
			stmt = Argument::create(name);
	}
	return stmt;
}

// class_def ::= CLASS IDENT '{' stmt_list '}' [';'] .
Stmt *p_class_def()
{
	Stmt *stmt = nullptr;
	if (accept(Token::CLASS))
	{
		AstNode<Ident> name(p_ident_expr());
		expect('{');
		StmtList stmts; p_stmt_list(stmts, true);
		expect('}');
		stmt = ClassDef::create(name, stmts);
	}
	return stmt;
}

// if_stmt ::= IF '(' expr ')'
//             { (ELIF | ELSE IF) '(' expr ')' '{' local_stmt_list '}' }
//             [ ELSE '{' local_stmt_list '}' ] .
Stmt *p_if_stmt()
{
	Stmt *stmt = nullptr;
	if (accept(Token::IF))
	{
		expect('(');
		AstNode<Expr> if_expr(p_expr());
		if (!if_expr)
		{
			std::stringstream ss;
			ss << "expected condition expression in `if', got " <<
			      "`" << lex.token.text << "' (" << last << ")";
			SYNTAX_ERROR(ss.str());
		}
		expect(')');

		expect('{');
		StmtList if_stmts; p_stmt_list(if_stmts);
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
				AstNode<Expr> elseif_expr(p_expr());
				if (!elseif_expr)
				{
					std::stringstream ss;
					ss << "expected conditional expression in ";
					if (is_elif)
						ss << "`elif'";
					else
						ss << "`else if'";
					ss << ", got `" << lex.token.text << "' (" << last << ")";
					SYNTAX_ERROR(ss.str());
				}
				expect(')');
				expect('{');
				StmtList elseif_body_stmts;
				p_stmt_list(elseif_body_stmts);
				expect('}');
				AstNode<IfStmt> elseif_stmt(IfStmt::create(elseif_expr, elseif_body_stmts));
				elseif_stmts.emplace_back(elseif_stmt.get());
				elseif_stmt.clear();
				continue;
			}
			else
			{
				expect('{');
				p_stmt_list(else_stmts);
				expect('}');
				break;
			}
		}

		stmt = IfStmt::create(if_expr, if_stmts, elseif_stmts, else_stmts);
	}
	return stmt;
}

// return_stmt ::= RETURN [ expr ] [';'] .
Stmt *p_return_stmt()
{
	Stmt *stmt = nullptr;
	if (accept(Token::RETURN))
	{
		AstNode<Expr> expr(p_expr());
		stmt = ReturnStmt::create(expr);
	}
	return stmt;
}

// stmt ::= alias
//       | import
//       | var_decl
//       | func_def
//       | class_def
//       | return_stmt
//       | if_stmt
//       .
Stmt *p_stmt(bool top_level=false)
{
	Stmt *stmt = nullptr;
	push_loc();
	if ((stmt = p_alias()))
	{
		pop_loc(stmt);
		return stmt;
	}
	else if ((stmt = p_import_stmt()))
	{
		pop_loc(stmt);
		return stmt;
	}
	else if ((stmt = p_var_decl()))
	{
		pop_loc(stmt);
		return stmt;
	}
	else if ((stmt = p_func_def()))
	{
		pop_loc(stmt);
		return stmt;
	}
	else if ((stmt = p_class_def()))
	{
		pop_loc(stmt);
		return stmt;
	}
	else if (!top_level)
	{
		if ((stmt = p_return_stmt()))
		{
			pop_loc(stmt);
			return stmt;
		}
		else if ((stmt = p_if_stmt()))
		{
			pop_loc(stmt);
			return stmt;
		}
	}
	pop_loc(stmt);
	return stmt;
}

// stmt_list ::= stmt { stmt } .
void p_stmt_list(StmtList& lst, bool top_level=false)
{
	Stmt *stmt;
	while ((stmt = p_stmt(top_level)))
	{
		try
		{
			lst.emplace_back(stmt);
			while (accept(';'))
				; // skip stray semi-colons
		}
		catch (...)
		{
			delete stmt;
			throw;
		}
	}
	while (accept(';'))
		; // skip stray semi-colons
}

// number_expr ::= ??INTEGER_CONSTANTS?? | FCONST .
Expr *p_number_expr()
{
	Expr *exp = nullptr;
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
		delete exp;
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
	catch (SyntaxError&)
	{
		delete exp;
		throw;
	}

	return exp;
}

// paren_expr ::= '(' expr ')' .
Expr *p_paren_expr()
{
	next();
	AstNode<Expr> expr(p_expr());
	if (!expr)
		return nullptr;
	expect(')');
	return expr.steal();
}

// ident_expr ::= IDENT .
Ident *p_ident_expr()
{
	Ident *expr;
	push_loc();
	AstNode<Ident> ident(new Ident(text()));
	expect(Token::IDENT);
	expr = ident.steal();
	pop_loc(expr);
	return expr;
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
		}
	}
}

// expr ::= primary_expr bin_op_rhs .
Expr *p_expr()
{
	push_loc();
	push_loc();
	AstNode<Expr> lhs(p_primary_expr());
	if (!lhs)
	{
		pop_loc();
		return nullptr;
	}
	pop_loc(lhs.get());
	AstNode<Expr> bin_op(p_bin_op_rhs(0, lhs.get()));
	lhs.clear();
	pop_loc(bin_op.get());
	return bin_op.steal();
}

// bin_op_rhs ::= { ??OPERATORS?? primary_expr } .
Expr *p_bin_op_rhs(int expr_prec, Expr *lhs)
{
	// TODO: make exception-safe
	while (true)
	{
		int tok_prec = get_prec();
		if (tok_prec < expr_prec)
			return lhs;
		char32_t op = last;
		next();
		push_loc();
		Expr *rhs = p_primary_expr();
		if (!rhs)
		{
			pop_loc();
			return nullptr;
		}
		pop_loc(rhs);
		int next_prec = get_prec();
		if (tok_prec < next_prec)
		{
			push_loc();
			rhs = p_bin_op_rhs(tok_prec + 1, rhs);
			if (!rhs)
				return nullptr;
			pop_loc(rhs);
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
