#include <soda/sodainc.h> // pch
#include <soda/parser.h>
#include <soda/lexer.h>
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
	try
	{
		loc_stack.emplace(
			SourceRange(lex.token.position.start, lex.token.position.end),
			SourceRange(lex.token.line.start, lex.token.line.end),
			SourceRange(lex.token.column.start, lex.token.column.end)
		);
	}
	catch (...) {}
}

void pop_loc(Node *node=nullptr) noexcept
{
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
Stmt p_import_stmt()
{
	if (accept(Token::IMPORT))
		return Stmt(new Import(Ident(p_ident_expr())));
	return Stmt(nullptr);
}

// alias ::= ALIAS IDENT '=' IDENT [';'] .
Stmt p_alias()
{
	if (accept(Token::ALIAS))
	{
		Ident type(p_ident_expr());
		expect('=');
		Ident alias(p_ident_expr());
		return Stmt(new Alias(std::move(type), std::move(alias)));
	}
	return Stmt(nullptr);
}

// var_decl ::= VAR IDENT { '=' expr } [';'] .
Stmt p_var_decl()
{
	if (accept(Token::VAR))
	{
		Ident ident(p_ident_expr());
		if (accept('='))
		{
			Expr expr(p_expr());
			if (!expr)
			{
				std::stringstream ss;
				ss << "expected expression as right hand side of initial " <<
				      "assignment but got `" << lex.token.text << "' " <<
				      "(" << last << ")";
				SYNTAX_ERROR(ss.str());
			}
			else
				return Stmt(new VarDecl(std::move(ident), std::move(expr)));
		}
		else
			return Stmt(new VarDecl(std::move(ident), Expr(nullptr)));
	}
	return Stmt(nullptr);
}

// func_def ::= FUN IDENT '(' arg_list ')' '{' stmt_list '}' [';'] .
Stmt p_func_def()
{
	if (accept(Token::FUN))
	{
		Ident ident(p_ident_expr());
		expect('(');
		StmtList args;
		p_arg_list2(args);
		expect(')');
		expect('{');
		StmtList stmts;
		p_stmt_list(stmts);
		expect('}');
		return Stmt(
			new FuncDef(std::move(ident), std::move(args), std::move(stmts))
		);
	}
	return Stmt(nullptr);
}

// arg_list ::= argument { , argument } .
void p_arg_list2(StmtList& lst)
{
	do
	{
		Stmt stmt(p_argument());
		if (!stmt)
			break;
		lst.push_back(std::move(stmt));
	}
	while (accept(','));
}

// argument ::= IDENT [ '=' expr ] .
Stmt p_argument()
{
	if (last == Token::IDENT)
	{
		Ident name(p_ident_expr());
		if (accept('='))
		{
			Expr expr(p_expr());
			if (!expr)
			{
				std::stringstream ss;
				ss << "expected expression as right hand side of " <<
				      "default argument specification, got `" <<
				      lex.token.text << "' (" << last << ")";
				SYNTAX_ERROR(ss.str());
			}
			return Stmt(new Argument(std::move(name), std::move(expr)));
		}
		else
			return Stmt(new Argument(std::move(name), Expr(nullptr)));
	}
	return Stmt(nullptr);
}

// class_def ::= CLASS IDENT '{' stmt_list '}' [';'] .
Stmt p_class_def()
{
	if (accept(Token::CLASS))
	{
		Ident name(p_ident_expr());
		expect('{');
		StmtList stmts;
		p_stmt_list(stmts, true);
		expect('}');
		return Stmt(new ClassDef(std::move(name), std::move(stmts)));
	}
	return Stmt(nullptr);
}

Stmt p_if_stmt()
{
	if (accept(Token::IF))
	{
		expect('(');
		Expr expr(p_expr());
		if (!expr)
		{
			std::stringstream ss;
			ss << "expected condition expression in `if', got " <<
			      "`" << lex.token.text << "' (" << last << ")";
			SYNTAX_ERROR(ss.str());
		}
		expect(')');
		Stmt if_stmt(p_stmt());
		if (accept(Token::ELSE))
		{
			Stmt else_stmt(p_stmt());
			return Stmt(
				new IfStmt(std::move(expr), std::move(if_stmt), std::move(else_stmt)));
		}
		else
		{
			return Stmt(
				new IfStmt(std::move(expr), std::move(if_stmt), Stmt(nullptr)));
		}
	}
	return Stmt(nullptr);
}

// return_stmt ::= RETURN [ expr ] [';'] .
Stmt p_return_stmt()
{
	if (accept(Token::RETURN))
		return Stmt(new ReturnStmt(std::move(p_expr())));
	return Stmt(nullptr);
}

Stmt p_block_stmt()
{
	if (accept('{'))
	{
		StmtList stmts;
		p_stmt_list(stmts);
		expect('}');
		return Stmt(new CompoundStmt(std::move(stmts)));
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

	//push_loc();

	TRY_STMT(alias);
	TRY_STMT(import_stmt);
	TRY_STMT(var_decl);
	TRY_STMT(func_def);
	TRY_STMT(class_def);

	if (!top_level)
	{
		TRY_STMT(return_stmt);
		TRY_STMT(if_stmt);
		TRY_STMT(block_stmt);
	}

	//pop_loc(stmt);
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
		while (accept(';'))
			;
	}
	while (accept(';'))
		;
}

// number_expr ::= ??INTEGER_CONSTANTS?? | FCONST .
Expr p_number_expr()
{
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
		return Expr(nullptr);

	try
	{
		if (base == 0)
			return Expr(new Float(text()));
		else
			return Expr(new Integer(text(), base));
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

	return Expr(nullptr);
}

// paren_expr ::= '(' expr ')' .
Expr p_paren_expr()
{
	next();
	Expr expr(p_expr());
	if (!expr)
		return Expr(nullptr);
	expect(')');
	return expr;
}

// ident_expr ::= IDENT .
Ident p_ident_expr()
{
	Ident ident(new IdentImpl(text()));
	expect(Token::IDENT);
	return ident;
}

// strlit_expr ::= STR_LIT { STR_LIT } .
Expr p_strlit_expr()
{
	if (last == Token::STR_LIT)
	{
		std::u32string text;
		do
		{
			text += lex.token.text.substr(1, lex.token.text.size() - 2);
			next();
		} while (last == Token::STR_LIT);
		return Expr(new StrLit(text));
	}
	return Expr(nullptr);
}

// primary_expr ::= ident_expr | number_expr | paren_expr .
Expr p_primary_expr()
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
Expr p_expr()
{
	Expr lhs(p_primary_expr());
	if (!lhs)
		return Expr(nullptr);
	return p_bin_op_rhs(0, lhs.release());
}

// bin_op_rhs ::= { ??OPERATORS?? primary_expr } .
Expr p_bin_op_rhs(int expr_prec, ExprImpl* lhs)
{
	// TODO: make exception-safe
	while (true)
	{
		int tok_prec = get_prec();
		if (tok_prec < expr_prec)
			return Expr(lhs);
		char32_t op = last;
		next();
		Expr rhs(p_primary_expr());
		if (!rhs)
			return nullptr;
		int next_prec = get_prec();
		if (tok_prec < next_prec)
		{
			rhs.reset(p_bin_op_rhs(tok_prec + 1, rhs.release()).release());
			if (!rhs)
				return nullptr;
		}
		lhs = new BinOp(op, Expr(lhs), std::move(rhs));
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
