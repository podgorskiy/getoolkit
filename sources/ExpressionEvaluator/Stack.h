#pragma once
#include <inttypes.h>

namespace ExpessionEvaluator
{
	class Stack
	{
	public:
		Stack(): m_sp(&m_stack[0])
		{};

		uint8_t m_stack[1024];
		uint8_t* m_sp;

		template<typename T>
		void Push(T var)
		{
			*reinterpret_cast<T*>(m_sp) = var;
			m_sp += sizeof(T);
		}

		template<typename T>
		T Pop()
		{
			m_sp -= sizeof(T);
			return *reinterpret_cast<T*>(m_sp);
		}
	};
}
