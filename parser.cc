#include "parser.h"
#include "lexer.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace Soda
{

struct Parser
{

Lexer& lex;
TU& tu;
Token tok;
Token::Kind last;

Parser(Lexer& lex, TU& tu)
	: lex(lex), tu(tu), last(lex.next())
{
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

bool accept(char32_t ch)
{
	return accept((Token::Kind)ch);
}

bool accept(Token::Kind kind)
{
	if (last == kind)
	{
		tok = lex.token;
		last = lex.next();
		return true;
	}
	return false;
}

void expect(Token::Kind kind)
{
	if (!accept(kind))
	{
		std::stringstream ss;
		ss << "Failed to accept token '" << kind << "'!";
		throw std::runtime_error(ss.str());
	}
}

void expect(char32_t ch)
{
	expect((Token::Kind)ch);
}

Token::Kind next()
{
	tok = lex.token;
	last = lex.next();
	return last;
}

void parse()
{
	Expr *exp;
	while ((exp = p_expr()))
	{
		tu.stmts.emplace_back(new ExprStmt(exp));
		exp = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////////

// number_expr: *_ICONST | FCONST
Expr *p_number_expr()
{
	Expr *exp;
	switch (last)
	{
		case Token::DEC_ICONST:
			exp = new Integer(lex.token.text, 10);
			break;
		case Token::HEX_ICONST:
			exp = new Integer(lex.token.text, 16);
			break;
		case Token::OCT_ICONST:
			exp = new Integer(lex.token.text, 8);
			break;
		case Token::BIN_ICONST:
			exp = new Integer(lex.token.text, 2);
			break;
		case Token::FCONST:
			exp = new Float(lex.token.text);
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
	Expr *exp = new Ident(lex.token.text);
	next();
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
		case '(':
			return p_paren_expr();
		default: // error
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

} // namespace Soda
