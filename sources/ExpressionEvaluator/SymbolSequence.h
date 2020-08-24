#pragma once
#include <vector>
#include <string>
#include <string.h>

namespace ExpessionEvaluator
{
	class Symbol
	{
	public:
		Symbol();
		Symbol(const Symbol& other);
		Symbol(char c, int position = -1);
		bool operator == (const Symbol& other) const;
		bool operator == (const char& other) const;
		operator char() const;
		int GetPosition() const;
		bool IsWhiteSpace() const;
		bool IsAlphabetic() const;
		bool IsDigit() const;
	private:
		char value;
		int position;
	};

	class SymbolSequence
	{
	public:
		class const_iterator
		{
		public:
			const_iterator() = default;

			const_iterator(const SymbolSequence& sequence);
			void SetToEnd(const SymbolSequence& sequence);

			const Symbol& operator *() const;
			const const_iterator operator++(int);
			const const_iterator operator++();
			const Symbol* operator->() const;
			bool operator == (const const_iterator& other) const;
			bool operator != (const const_iterator& other) const;
		private:
			std::vector<Symbol>::const_iterator m_sequenceIterator;
		};

		SymbolSequence() = default;
		SymbolSequence(const const_iterator& begin, const const_iterator& end);

		std::string ToString();

		void Construct(const char* str);

		const_iterator const_begin() const;
		const_iterator const_end() const;

		std::string PrintAndHighLight(int position) const;
		std::string PrintAndHighLight(int start, int end) const;

	private:
		std::vector<Symbol> m_sequence;
	};

	inline Symbol::Symbol() :value('\0'), position(-1)
	{}

	inline Symbol::Symbol(const Symbol& other) : value(other.value), position(other.position)
	{}

	inline Symbol::Symbol(char c, int position /*= -1*/) : value(c), position(position)
	{}

	inline bool Symbol::operator == (const Symbol& other) const
	{
		return value == other.value;
	}

	inline bool Symbol::operator == (const char& other) const
	{
		return value == other;
	}

	inline Symbol::operator char() const
	{
		return value;
	}

	inline int Symbol::GetPosition() const
	{
		return position;
	}

	inline bool Symbol::IsWhiteSpace() const
	{
		return value == ' ' || value == '\t' || value == '\n' || value == '\f' || value == '\r';
	}

	inline bool Symbol::IsAlphabetic() const
	{
		return ((value <= 'z' && value >= 'a') || (value <= 'Z' && value >= 'A') || value == '_');
	}

	inline bool Symbol::IsDigit() const
	{
		return value >= '0' && value <= '9';
	}

	inline SymbolSequence::const_iterator::const_iterator(const SymbolSequence& sequence) : m_sequenceIterator(sequence.m_sequence.begin())
	{
	}

	inline void SymbolSequence::const_iterator::SetToEnd(const SymbolSequence& sequence)
	{
		m_sequenceIterator = sequence.m_sequence.end();
	}

	inline const Symbol& SymbolSequence::const_iterator::operator *() const
	{
		return *m_sequenceIterator;
	}

	inline const SymbolSequence::const_iterator SymbolSequence::const_iterator::operator++(int)
	{
		SymbolSequence::const_iterator oldValue(*this);
		++(*this);
		return oldValue;
	}

	inline const SymbolSequence::const_iterator SymbolSequence::const_iterator::operator++()
	{
		++m_sequenceIterator;
		return *this;
	}

	inline const Symbol* SymbolSequence::const_iterator::operator->() const
	{
		return &(*m_sequenceIterator);
	}

	inline bool SymbolSequence::const_iterator::operator == (const SymbolSequence::const_iterator& other) const
	{
		return m_sequenceIterator == other.m_sequenceIterator;
	}

	inline bool SymbolSequence::const_iterator::operator != (const SymbolSequence::const_iterator& other) const
	{
		return m_sequenceIterator != other.m_sequenceIterator;
	}

	/* SymbolSequence */
	inline SymbolSequence::SymbolSequence(const SymbolSequence::const_iterator& begin,
	                               const SymbolSequence::const_iterator& end)
	{
		for (SymbolSequence::const_iterator it = begin; it != end; ++it)
		{
			m_sequence.push_back(*it);
		}
		m_sequence.push_back(Symbol('\0', m_sequence.size()));
	}

	inline std::string SymbolSequence::ToString()
	{
		std::string tmp;
		for (std::vector<Symbol>::iterator it = m_sequence.begin(); it != m_sequence.end(); ++it)
		{
			if (*it != '\0')
			{
				tmp.push_back(*it);
			}
		}
		return tmp;
	}

	inline void SymbolSequence::Construct(const char* str)
	{
		int length(strlen(str));
		m_sequence.resize(length);
		for (int i = 0; i < length; ++i)
		{
			m_sequence[i] = Symbol(str[i], i);
		}
		m_sequence.push_back(Symbol('\0', m_sequence.size()));
	}

	inline SymbolSequence::const_iterator SymbolSequence::const_begin() const
	{
		return SymbolSequence::const_iterator(*this);
	}

	inline SymbolSequence::const_iterator SymbolSequence::const_end() const
	{
		SymbolSequence::const_iterator it;
		it.SetToEnd(*this);
		return it;
	}

	inline std::string SymbolSequence::PrintAndHighLight(int position) const
	{
		return PrintAndHighLight(position, position + 1);
	}

	inline std::string SymbolSequence::PrintAndHighLight(int startPos, int endPos) const
	{
		std::string result;
		SymbolSequence::const_iterator linestart = const_begin();
		SymbolSequence::const_iterator startIt = const_end();
		for (SymbolSequence::const_iterator it = const_begin(); it != const_end(); ++it)
		{
			if (*it == '\n')
			{
				linestart = it;
				++linestart;
			}
			else
			{
				if (it->GetPosition() == startPos)
				{
					startIt = it;
					break;
				}
			}
		}
		for (SymbolSequence::const_iterator it = linestart; it != const_end(); ++it)
		{
			if (*it == '\n' || *it == '\0')
			{
				break;
			}
			result += static_cast<char>(*it);
		}
		result += "\n";
		for (SymbolSequence::const_iterator it = linestart; it != const_end() && it != startIt; ++it)
		{
			result += " ";
		}
		for (SymbolSequence::const_iterator it = startIt; it != const_end() && it->GetPosition() != endPos; ++it)
		{
			result += "^";
		}
		result += "\n";
		return result;
	}
}
