#pragma once
#include <memory>
#include <vector>
#include <inttypes.h>
#include <assert.h>
#include <map>
#include <string>
#include "Stack.h"


namespace ExpessionEvaluator
{
	enum Opcodes: uint8_t
	{
		PushConstToStack = 1,
		LoadVar,
		InvokeInstruction,
		AddInstruction,
		SubInstruction,
		MulInstruction,
		DivInstruction,
		UnaryMinusInstruction,
	};

	struct InterpretedProgram
	{
		InterpretedProgram(): pointer(0), idc(0), size(0), data(nullptr)
		{
			data = (uint8_t*)malloc(512);
		}

		template<typename T>
		T Read() const
		{
			T val = *reinterpret_cast<const T*>(data + pointer);
			pointer += sizeof(T);
			return val;
		}

		template<typename T>
		void Write(T val)
		{
			*reinterpret_cast<T*>(data + pointer) = val;
			pointer += sizeof(T);
		}

		uint16_t Symbol(const std::string& name)
		{
			auto it = symbols.find(name);
			if (it == symbols.end())
			{
				symbols[name] = idc;
				relocations.push_back(std::make_pair(idc, pointer));
				return idc++;
			}
			else
			{
				relocations.push_back(std::make_pair(it->second, pointer));
				return it->second;
			}
		}

		double Eval() const
		{
			pointer = 0;
			while(pointer < size)
			{
				ExecStep();
			}
			return stack->Pop<double>();
		}

		void ExecStep() const
		{
			auto opcode = Read<uint8_t>();
			switch (opcode)
			{
				case PushConstToStack:
				{
					auto value = Read<double>();
					stack->Push<double>(value);
					break;
				}
				case LoadVar:
				{
					auto* var = Read<const double*>();
					stack->Push<double>(*var);
					break;
				}
				case InvokeInstruction:
				{
					auto f = Read<const void*>();
					auto w = Read<void (*const)(Stack*, const void*)>();
					w(stack, f);
					break;
				}
				case AddInstruction:
				{
					auto rhs = stack->Pop<double>();
					auto lhs = stack->Pop<double>();
					auto result = lhs + rhs;
					stack->Push<double>(result);
					break;
				}
				case SubInstruction:
				{
					auto rhs = stack->Pop<double>();
					auto lhs = stack->Pop<double>();
					auto result = lhs - rhs;
					stack->Push<double>(result);
					break;
				}
				case MulInstruction:
				{
					auto rhs = stack->Pop<double>();
					auto lhs = stack->Pop<double>();
					auto result = lhs * rhs;
					stack->Push<double>(result);
					break;
				}
				case DivInstruction:
				{
					auto rhs = stack->Pop<double>();
					auto lhs = stack->Pop<double>();
					auto result = lhs / rhs;
					stack->Push<double>(result);
					break;
				}
				case UnaryMinusInstruction:
				{
					auto val = stack->Pop<double>();
					auto result = -val;
					stack->Push<double>(result);
					break;
				}
				default:
				{
					break;
				}
			}
		}

		mutable int pointer;
		uint16_t idc;

		std::map<std::string, uint16_t> symbols;
		std::vector<std::pair<uint16_t, uint16_t> > relocations;
		int size;
		uint8_t* data;
		Stack* stack;
	};
}
