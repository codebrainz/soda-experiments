
#if 0

#include <istream>
#include <iostream>
#include <cctype>
#include <string>



struct Token {

	enum Kind {
		END=-2,
		ERROR=-1,
		ZERO=0,

		IDENT=256,
		VAR,
		NUMBER,
		COMMENT,

		DEC_ICONST=500,
		HEX_ICONST,
		BIN_ICONST,
		OCT_ICONST,
		DEC_FCONST,

		LINE_COMMENT=600,
		MULTI_COMMENT=601,
	};

	struct Range {
		unsigned int start, end;
		Range(unsigned int start=0, unsigned int end=0)
			: start(start), end(end) {}
	};

	Kind kind;
	Range line, column;
	std::string text;

	Token(Kind kind=ZERO, Range line=Range(0,0), Range column=Range(0,0))
		: kind(kind), line(line), column(column), text("") {}

	operator bool() const {
		return (kind > ZERO);
	}

	bool operator!() const {
		return (kind <= ZERO);
	}

	Token& operator=(Kind kind) {
		this->kind = kind;
		return *this;
	}

	operator Kind() const {
		return kind;
	}
};


class Parser {

public:
	Parser(std::istream& stream, std::string fn="<stream>")
		: last_ch(' '), stream(stream), fn(fn) { }

	Token::Kind get_token()
	{
		token.text = "";

		skip_ws();

		Token::Kind tok;
		if (!!(tok = t_identifier()))
			return tok;

		if (!!(tok = t_integer()))
			return tok;

		if (!!(tok = t_comment()))
			return tok;

		if (!last_ch)
			return Token::Kind::END;

		if (last_ch == Token::Kind::ERROR)
			return Token::Kind::ERROR;

		int this_ch = last_ch;
		last_ch = stream.get();

		return (Token::Kind) this_ch;
	}

private:
	int last_ch;
	std::istream& stream;
	std::string fn;
	Token token;
	unsigned int line, column;
	bool done;

	int next_ch()
	{
		last_ch = stream.get();
		if (last_ch == '\n') // todo: handle carriage returns and unicode LEs
		{
			line++;
			column = 0;
		}
		else
		{
			column++;
		}
		done = stream.eof();
		return last_ch;
	}

	int peek()
	{
		return stream.peek();
	}

	void start_token()
	{
		token.line.start = line;
		token.column.start = column;
	}

	void end_token()
	{
		token.line.end = line;
		token.column.end = column;
	}

	void skip_ws()
	{
		while (isspace(last_ch))
			last_ch = stream.get();
	}

	void skip_to_eol()
	{
		while (last_ch != '\n')
			last_ch = stream.get();
	}

	bool is_hex(int ch)
	{
		return ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z')
				|| (ch >= 'a' && ch <= 'z'));
	}

	bool is_octal(int ch)
	{
		return (ch >= '0' && ch <= '7');
	}

	Token::Kind t_identifier() // [a-zA-Z_][a-zA-Z_0-9]*
	{
		if (isalpha(last_ch) || last_ch == '_')
		{
			token.text = last_ch;

			start_token();
			while (isalnum((last_ch = stream.get())) || last_ch == '_')
				token.text += last_ch;
			end_token();

			if (token.text == "var")
				token.kind = Token::Kind::VAR;
			// ... other keywords ...
			else
				token.kind = Token::Kind::IDENT;
		}
		return token.kind;
	}

	Token::Kind t_float_or_integer()
	{
		bool is_float = false;
		token.text += last_ch;
		start_token();
		while (isdigit(next_ch()) || last_ch == '.')
		{
			if (last_ch == '.')
				is_float = true;
			token.text += last_ch;
		}
		end_token();
		if (is_float)
			token.kind = Token::DEC_FCONST;
		else
			token.kind = Token::DEC_ICONST;
		return token.kind;
	}

	Token::Kind t_integer()
	{
		if (isdigit(last_ch))
		{
			if (last_ch == '0')
			{
				next_ch();
				if (last_ch == 'x' || last_ch == 'X') // 0[xX][a-fA-F0-9]+
				{
					next_ch();
					start_token();
					while (is_hex(last_ch))
					{
						token.text += last_ch;
						last_ch = stream.get();
					}
					end_token();
					token.kind = Token::Kind::HEX_ICONST;
				}
				else if (last_ch == 'b' || last_ch == 'B') // 0[bB][0-1]+
				{
					next_ch();
					start_token();
					while (last_ch == '0' || last_ch == '1')
					{
						token.text += last_ch;
						last_ch = stream.get();
					}
					end_token();
					token.kind = Token::Kind::BIN_ICONST;
				}
				else if (last_ch == 'o' || last_ch == 'O') // 0[oO][0-7]+
				{
					next_ch();
					start_token();
					while (is_octal(last_ch))
					{
						token.text += last_ch;
						last_ch = stream.get();
					}
					end_token();
					token.kind = Token::Kind::OCT_ICONST;
				}
				else // 0[0-9]*
				{
					start_token();
					while (is_octal(last_ch))
					{
						token.text += last_ch;
						last_ch = stream.get();
					}
					end_token();
					token.kind = Token::Kind::OCT_ICONST;
				}
			}
			else // [1-9][0-9]* | ([1-9]+\.[0-9]*)
			{
				t_float_or_integer();
			}
		}
		else if (last_ch == '.' && isdigit(peek())) // \.[0-9]+)
		{
			t_float_or_integer();
		}
		else
		{
			token.kind = Token::Kind::ZERO;
		}
		return token.kind;
	}

	Token::Kind t_comment()
	{
		if (last_ch == '/' && peek() == '*')
		{
			next_ch(); // skip the *
			start_token();
			while (next_ch() != '*' && peek() != '/')
			{
				token.text += last_ch;
				end_token(); // just keep updating end position to avoid grabbing */
				if (stream.eof())
				{
					token.text = "End of file was reached inside a multi-line comment";
					token.kind = Token::Kind::ERROR;
					return token.kind;
				}
			}
			next_ch(); // skip the trailing /
			token.kind = Token::Kind::MULTI_COMMENT;
		}
		else if (last_ch == '/' && stream.peek() == '/')
		{
			next_ch(); // skip the /
			start_token();
			while (next_ch() != '\n')
			{
				token.text += last_ch;
				end_token(); // just keep updatin end position to avoid grabbing newline
				if (stream.eof())
				{
					token.text = "End of file was reached inside a line comment";
					token.kind = Token::Kind::ERROR;
					return token.kind;
				}
			}
			next_ch(); // skip the trailing newline
			token.kind = Token::Kind::LINE_COMMENT;
		}
		else
		{
			token.kind = Token::Kind::ZERO;
		}
		return token.kind;
	}
};

int main()
{
	Token::Kind token;
	Parser parser(std::cin, "<stdin>");
	while ((token = parser.get_token()) != Token::Kind::END)
	{
		std::cout << "Token: " << (int) token << std::endl;
		if (token == Token::Kind::ERROR)
		{
			std::cerr << "EOF\n" << std::endl;
			break;
		}
	}
	return 0;
}
#endif
