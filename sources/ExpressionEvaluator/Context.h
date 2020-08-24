#pragma once
#include <cmath>
#include <map>
#include <type_traits>
#include <list>
#include <time.h>
#include "Mangle.h"
#include "InterpretedProgram.h"
#include "JProgram.h"

namespace ExpessionEvaluator
{
	enum Type
	{
		Interpreted,
		JIT
	};

	template<Type type>
	using Program = typename std::conditional<type==Interpreted, InterpretedProgram, JitProgram>::type;

	template<Type type>
	inline Program<type> Compile(const char* source);


	template<>
	inline Program<Interpreted> Compile<Interpreted>(const char* source)
	{
		detail::Parser parser;

		parser.AddSource(source);

		if (parser.HasErrorOccurred())
		{
			printf("%s", parser.GetErrorMessage().c_str());

			return InterpretedProgram();
		}

		detail::BytecodeGenerator bcg;

		parser.GenerateProgram(bcg);

		if (bcg.HasErrorOccurred())
		{
			printf("%s", bcg.GetErrorMessage().c_str());

			return InterpretedProgram();
		}
		return bcg.GetProgram();
	}

	template<>
	inline Program<JIT> Compile<JIT>(const char* source)
	{
		detail::Parser parser;

		parser.AddSource(source);

		if (parser.HasErrorOccurred())
		{
			printf("%s", parser.GetErrorMessage().c_str());

			return JitProgram();
		}

		detail::JITGenerator bcg;

		parser.GenerateProgram(bcg);

		if (bcg.HasErrorOccurred())
		{
			printf("%s", bcg.GetErrorMessage().c_str());

			return JitProgram();
		}
		return bcg.GetProgram();
	}

	namespace detail
	{
		namespace builtin
		{
			inline double round(double a)
			{
				return (int) (a + 0.5);
			}

			inline double max(double a, double b)
			{
				return (a > b) ? a : b;
			}

			inline double min(double a, double b)
			{
				return (a < b) ? a : b;
			}

			inline double atan2_(double a, double b)
			{
				return std::atan2(a, b);
			}

			inline double step(double edge, double x)
			{
				return x < edge ? 0 : 1.0;
			}

			inline double mix(double a, double b, double x)
			{
				return a * (1.0 - x) + b * x;
			}

			inline double select(double a, double b, double x)
			{
				return x == 0 ? a : b;
			}

			inline double pow_(double x, double y)
			{
				return pow(x, y);
			}
			inline double clock_()
			{
				return clock();
			}
			inline double clamp(double x, double min, double max)
			{
				return x > max ? max : (x < min ? min : x);
			}
		}

		template <typename T> struct remove_class {};
		template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> { typedef R type(A...); };
		template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> { typedef R type(A...); };

		template<typename capture, typename return_type, typename... arg_type>
		inline void LWrapper(Stack* stack, const void* f)
		{
			auto& f_ = ((capture*) f)->f;
			return_type result = f_(stack->Pop<arg_type>()...);
			stack->Push(result);
		}

		template<typename return_type, typename... arg_type>
		inline void Wrapper(Stack* stack, const void* f)
		{
			typedef return_type(* Func)(arg_type...);
			return_type result = ((Func) f)(stack->Pop<arg_type>()...);
			stack->Push(result);
		}

		template<typename... T>
		void dummy(T...)
		{}

		template<typename T>
		struct assert_double
		{
			static_assert(std::is_same<T, double>::value, "Type must be double");
		};

		class cpp_function
		{
		public:
			cpp_function()
			{}

			template<typename return_type, typename... arg_type>
			cpp_function(return_type (* f)(arg_type...))
			{
				typedef return_type(* Func)(arg_type...);
				initialize((Func) nullptr);
				initialize_vanila(f);
			}

			template <typename Func>
			cpp_function(Func&& f)
			{
				typedef typename remove_class<decltype(&std::remove_reference<Func>::type::operator())>::type* func_ptr;
				initialize((func_ptr) nullptr);
				initialize_lambda(f, (func_ptr) nullptr);
			}

			template<typename return_type, typename... arg_type>
			void initialize(return_type (*)(arg_type...))
			{
				assert_double<return_type>();
				dummy(assert_double<arg_type>()...);
				arg_count = sizeof...(arg_type);
			}

			template<typename return_type, typename... arg_type>
			void initialize_vanila(return_type (*f)(arg_type...))
			{
				func = (void*) f;
				wrapper = &Wrapper<return_type, arg_type...>;
			}

			template<typename Func, typename return_type, typename... arg_type>
			void initialize_lambda(Func&& ff, return_type (*)(arg_type...))
			{
				struct capture
				{
					typename std::remove_reference<Func>::type f;
				};
				func = new capture {std::forward<Func>(ff)};
				wrapper = &LWrapper<capture, return_type, arg_type...>;
			}

			void (* wrapper)(Stack*, const void*);

			void* func;
			int arg_count = 0;
		};

		class BuiltInFunction
		{
		public:
			BuiltInFunction(void (* wrapper)(Stack*, const void*), void* func): wrapper(wrapper), func(func)
			{}

			BuiltInFunction(): wrapper(nullptr), func(nullptr)
			{}

			operator bool()
			{
				return func != nullptr && wrapper != nullptr;
			}

			void (* wrapper)(Stack*, const void*);

			void* func;
		};

		template<typename T>
		class SymbolTable
		{
			std::map<std::string, T> m_symbols;
		public:
			T Get(const std::string& name) const
			{
				auto it = m_symbols.find(name);
				if (it != m_symbols.end())
				{
					return it->second;
				}
				return T();
			}

			void Add(const std::string& name, T var)
			{
				auto it = m_symbols.find(name);
				if (it == m_symbols.end())
				{
					m_symbols[name] = var;
				}
				else
				{
					//error
				}
			}
		};

		typedef SymbolTable<BuiltInFunction> FunctionTable;
		typedef SymbolTable<double*> VarTable;

		template<Type t>
		void _func(FunctionTable&, const char *, Program<t>&) {};

		template<>
		void _func<Interpreted>(FunctionTable& table, const char *name, Program<Interpreted>& p)
		{
			detail::cpp_function function([&p]()
			                              {
				                              return p.Eval();
			                              });
			table.Add(detail::Mangle(name, function.arg_count), {function.wrapper, function.func });
		}

		template<>
		void _func<JIT>(FunctionTable& table, const char *name, Program<JIT>& p)
		{
			detail::cpp_function function(p.Eval);
			table.Add(detail::Mangle(name, function.arg_count), {function.wrapper, function.func });
		}
	}

	template<Type type>
	class Context
	{
	public:
		Context(bool jit=true): use_jit(jit)
		{
			func("round", detail::builtin::round);
			func("max", detail::builtin::max);
			func("min", detail::builtin::min);
			func("step", detail::builtin::step);
			func("mix", detail::builtin::mix);
			func("lerp", detail::builtin::mix);
			func("select", detail::builtin::select);
			func("clock", detail::builtin::clock_);
			func("clamp", detail::builtin::clamp);

#define MATH_OP(name) func( #name, +[](double x){ return name(x); })
			MATH_OP(log);
			MATH_OP(exp);
			MATH_OP(log10);
			MATH_OP(exp10);
			MATH_OP(sin);
			MATH_OP(cos);
			MATH_OP(asin);
			MATH_OP(acos);
			MATH_OP(tan);
			MATH_OP(tanh);
			MATH_OP(atan);
			MATH_OP(atanh);
			MATH_OP(fabs);
			MATH_OP(sqrt);
#undef MATH_OP
			func("atan2", detail::builtin::atan2_);
			func("pow", detail::builtin::pow_);
		};

		void var(const std::string& name, double* val)
		{
			return m_symbolTable.Add(name, val);
		}

		template <typename Function>
		void func(const char *name, Function &&f)
		{
			detail::cpp_function function(f);
			m_functionTable.Add(detail::Mangle(name, function.arg_count), {function.wrapper, function.func });
		}

		void func(const char *name, const char* func)
		{
			Program<type> p = ExpessionEvaluator::Compile<type>(func);
			m_programs[name] = p;
			auto& p_ = m_programs[name];
			detail::_func<type>(m_functionTable, name, p_);
		}

		Program<type>& get_func(const char* name)
		{
			auto it = m_programs.find(name);
			if (it != m_programs.end())
			{
				return  it->second;
			}
			static Program<type> none;
			return none;
		}

		void Link()
		{
			for (auto& p: m_programs)
			{
				Link(p.second);
			}
		}

		void Link(InterpretedProgram& program)
		{
			std::vector<void*> pointers;
			std::vector<void*> wrapper;
			pointers.resize(program.symbols.size(), nullptr);
			wrapper.resize(program.symbols.size(), nullptr);
			int func_count = 0;
			for (auto& symbol : program.symbols)
			{
				void* p = (void*)m_symbolTable.Get(symbol.first);
				if (p == nullptr)
				{
					detail::BuiltInFunction b = m_functionTable.Get(symbol.first);
					if (!b)
					{
						printf("Error: Undefined symbol: '%s'\n", detail::Demangle(symbol.first).c_str());
						program.size = 0;
						return;
					}
					pointers[symbol.second] = b.func;
					wrapper[symbol.second] = (void*)b.wrapper;
					++func_count;
				}
				else
				{
					pointers[symbol.second] = p;
				}
			}

			int size_rl = program.size + program.relocations.size() * (sizeof(void*) - sizeof(uint16_t)) + func_count * sizeof(void*);
			auto* data_rl = (uint8_t*)malloc(size_rl);

			int dst = 0;
			int src = 0;
			int r = 0;

			while (src < program.size)
			{
				if (r < (int)program.relocations.size() && src == program.relocations[r].second)
				{
					*(void**)(data_rl + dst) = pointers[program.relocations[r].first];
					dst += sizeof(void*);
					if (wrapper[program.relocations[r].first] != nullptr)
					{
						*(void**) (data_rl + dst) = wrapper[program.relocations[r].first];
						dst += sizeof(void*);
					}
					src += 2;
					++r;
				}
				else
				{
					data_rl[dst++] = program.data[src++];
				}
			}
			free(program.data);
			program.data = data_rl;
			program.size = dst;
			program.stack = &m_stack;
			assert(program.size == size_rl);
		}

		void Link(JitProgram& program)
		{
			//hexDump(nullptr, mem, pointer);
			std::vector<void*> pointers;
			pointers.resize(program.symbols.size(), nullptr);
			for (auto& symbol : program.symbols)
			{
				void* p = (void*) m_symbolTable.Get(symbol.first);
				if (p == nullptr)
				{
					detail::BuiltInFunction b = m_functionTable.Get(symbol.first);
					if (!b)
					{
						printf("Error: Undefined symbol: '%s'\n", detail::Demangle(symbol.first).c_str());
						program.size = 0;
						return;
					}
					pointers[symbol.second] = b.func;
				}
				else
				{
					pointers[symbol.second] = p;
				}
			}

			int src = 0;
			int r = 0;

			while (src < program.size && r < (int)program.relocations_sym.size())
			{
				if (r < (int)program.relocations_sym.size() && src == program.relocations_pointer[r])
				{
					*(void**) (program.mem + src) = pointers[program.relocations_sym[r]];
					src += sizeof(void*);
					++r;
				}
				else
				{
					++src;
				}
			}

			hexDump(nullptr, program.mem, program.pointer);
		}

		Stack m_stack;
		detail::VarTable m_symbolTable;
		detail::FunctionTable m_functionTable;
		std::map<std::string, Program<type> > m_programs;
		bool use_jit;
	};

	typedef Context<Interpreted> INTContext;
	typedef Context<JIT> JITContext;
}
