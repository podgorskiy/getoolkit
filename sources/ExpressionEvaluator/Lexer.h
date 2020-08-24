#pragma once
#include "SymbolSequence.h"
#include "ErrorHandler.h"
#include <string>

namespace ExpessionEvaluator
{
	class Lexer : public ErrorHandler
	{
	public:
		enum TokenTypes: char
		{
			ASSIGN = '=',
			PLUS = '+',
			MINUS = '-',
			MUL = '*',
			DIV = '/',
			LPAR = '(',
			RPAR = ')',
			MOD = '%',
			POW = '^',
			DOT = '.',
			COMA = ',',
			SEMICOLON = ';',
			END_OF_STRING = '\0',
			IDENTIFIER = 'd',
			FLOAT = 'f',
			INT = 'i',
		};

		class Token
		{
		public:
			Token();

			Token(const Token& other);

			Token(Token t, const SymbolSequence::const_iterator& pos);

			Token(Token t, const SymbolSequence::const_iterator& posStart, const SymbolSequence::const_iterator& posEnd);

			Token(TokenTypes c);

			SymbolSequence GetSymbolSequence() const;

			int GetPosition() const;

			void GetPosition(int& start, int& end) const;

			bool operator == (const Token& other);

			operator char() const;

		private:
			char m_char;
			SymbolSequence::const_iterator m_iteratorStart;
			SymbolSequence::const_iterator m_iteratorEnd;
		};

		Lexer(const char* expression);

		Token GetNextToken();

		const SymbolSequence& GetSymbolSequence() const;

	private:
		bool AcceptChar(char c);

		void ExpectChar(char c);

		void NextCharacter();

		SymbolSequence m_sequence;
		SymbolSequence::const_iterator m_iterator;
	};

	inline Lexer::Token::Token() :m_char('\0')
	{
	}

	inline Lexer::Token::Token(Token t, const SymbolSequence::const_iterator& pos) :m_char(t.m_char), m_iteratorStart(pos), m_iteratorEnd(pos)
	{
		++m_iteratorEnd;
	}

	inline Lexer::Token::Token(Token t, const SymbolSequence::const_iterator& posStart, const SymbolSequence::const_iterator& posEnd) :m_char(t.m_char), m_iteratorStart(posStart), m_iteratorEnd(posEnd)
	{
	}

	inline Lexer::Token::Token(const Token& other)
	{
		m_char = other.m_char;
		m_iteratorStart = other.m_iteratorStart;
		m_iteratorEnd = other.m_iteratorEnd;
	}

	inline Lexer::Token::Token(TokenTypes c) :m_char(c)
	{
	}

	inline bool Lexer::Token::operator == (const Token& other)
	{
		return other.m_char == m_char;
	}

	inline Lexer::Token::operator char() const
	{
		return m_char;
	}

	inline SymbolSequence Lexer::Token::GetSymbolSequence() const
	{
		return SymbolSequence(m_iteratorStart, m_iteratorEnd);
	}

	inline int Lexer::Token::GetPosition() const
	{
		return m_iteratorStart->GetPosition();
	}

	inline void Lexer::Token::GetPosition(int& start, int& end) const
	{
		start = m_iteratorStart->GetPosition();
		end = m_iteratorEnd->GetPosition();
	}

	inline Lexer::Lexer(const char* expression)
	{
		m_sequence.Construct(expression);
		m_iterator = m_sequence.const_begin();
	}

	inline const SymbolSequence& Lexer::GetSymbolSequence() const
	{
		return m_sequence;
	}

	inline Lexer::Token Lexer::GetNextToken()
	{
		while (m_iterator->IsWhiteSpace())
		{
			NextCharacter();
		}

		SymbolSequence::const_iterator iteratorStart = m_iterator;

		if (AcceptChar(LPAR))
		{
			return Token(LPAR, iteratorStart);
		}
		else if (AcceptChar(RPAR))
		{
			return Token(RPAR, iteratorStart);
		}
		else if (AcceptChar(ASSIGN))
		{
			return Token(ASSIGN, iteratorStart);
		}
		else if (AcceptChar(MUL))
		{
			return Token(MUL, iteratorStart);
		}
		else if (AcceptChar(DIV))
		{
			return Token(DIV, iteratorStart);
		}
		else if (AcceptChar(PLUS))
		{
			return Token(PLUS, iteratorStart);
		}
		else if (AcceptChar(MINUS))
		{
			return Token(MINUS, iteratorStart);
		}
		else if (AcceptChar(MOD))
		{
			return Token(MOD, iteratorStart);
		}
		else if (AcceptChar(POW))
		{
			return Token(POW, iteratorStart);
		}
		else if (AcceptChar(DOT))
		{
			return Token(DOT, iteratorStart);
		}
		else if (AcceptChar(COMA))
		{
			return Token(COMA, iteratorStart);
		}
		else if (AcceptChar(SEMICOLON))
		{
			return Token(SEMICOLON, iteratorStart);
		}
		else if (AcceptChar('\0'))
		{
			return Token(END_OF_STRING, iteratorStart);
		}

		if (m_iterator->IsAlphabetic())
		{
			NextCharacter();

			while (m_iterator->IsAlphabetic() || m_iterator->IsDigit())
			{
				NextCharacter();
			}

			return Token(IDENTIFIER, iteratorStart, m_iterator);
		}
		else if (m_iterator->IsDigit())
		{
			NextCharacter();

			while (m_iterator->IsDigit())
			{
				NextCharacter();
			}

			if (*m_iterator == '.')
			{
				NextCharacter();

				while (m_iterator->IsDigit())
				{
					NextCharacter();
				}
				return Token(FLOAT, iteratorStart, m_iterator);
			}

			return Token(INT, iteratorStart, m_iterator);
		}

		SetError("Error, unknown symbol \"%c\" at position %d.", *m_iterator, m_iterator->GetPosition());

		return Token(END_OF_STRING, m_iterator);
	}

	inline bool Lexer::AcceptChar(char c)
	{
		if (*m_iterator == c)
		{
			NextCharacter();
			return true;
		}
		else
		{
			return false;
		}
	}

	inline void Lexer::ExpectChar(char c)
	{
		bool accepted = AcceptChar(c);
		if (!accepted)
		{
			if ('\0' == *m_iterator)
			{
				SetError("Error, unexpected end of string.");
			}
			else
			{
				SetError("Error, unexpected symbol at position %d. Expected \"%c\", but \"%c\" was found", m_iterator->GetPosition(), c, (char)*m_iterator);
			}
		}
	}

	inline void Lexer::NextCharacter()
	{
		++m_iterator;
	}
}
