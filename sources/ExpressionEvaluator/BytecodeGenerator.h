#pragma once
#include "ErrorHandler.h"
#include "Node.h"
#include "Mangle.h"
#include "InterpretedProgram.h"
#include "JProgram.h"


namespace ExpessionEvaluator
{
	namespace detail
	{
		class BytecodeGenerator : public IIGenerator, public ErrorHandler
		{
		public:
			void Dispatch(AST::FloatNode* node) override;

			void Dispatch(AST::RVariableNode* node) override;

			void Dispatch(AST::FunctionCallNode* node) override;

			void Dispatch(AST::AddNode* node) override;

			void Dispatch(AST::SubNode* node) override;

			void Dispatch(AST::MulNode* node) override;

			void Dispatch(AST::DivNode* node) override;

			void Dispatch(AST::UnaryMinusNode* node) override;

			void Dispatch(AST::UnaryPlusNode* node) override;

			void Dispatch(AST::CombineNode* node) override;

			InterpretedProgram& GetProgram()
			{
				return m_program;
			}

		private:
			template<uint8_t I, typename N>
			void DispatchArithmetic(N* node);

			InterpretedProgram m_program;
		};

		inline void BytecodeGenerator::Dispatch(AST::FloatNode* node)
		{
			m_program.Write(PushConstToStack);
			m_program.Write(node->m_value);
		}

		inline void BytecodeGenerator::Dispatch(AST::RVariableNode* node)
		{
			m_program.Write(LoadVar);
			uint16_t id = m_program.Symbol(node->m_name);
			m_program.Write(id);
		}

		inline void BytecodeGenerator::Dispatch(AST::FunctionCallNode* node)
		{
			for (auto& expr : node->args)
			{
				expr->Accept(this);
			}

			m_program.Write(InvokeInstruction);
			uint16_t id = m_program.Symbol(detail::Mangle(node->m_name, node->args.size()));
			m_program.Write(id);
		}

		template<uint8_t I, typename N>
		inline void BytecodeGenerator::DispatchArithmetic(N* node)
		{
			node->LHS->Accept(this);
			node->RHS->Accept(this);
			m_program.Write(I);
		}

		inline void BytecodeGenerator::Dispatch(AST::AddNode* node)
		{
			DispatchArithmetic<AddInstruction, AST::AddNode>(node);
		}

		inline void BytecodeGenerator::Dispatch(AST::SubNode* node)
		{
			DispatchArithmetic<SubInstruction, AST::SubNode>(node);
		}

		inline void BytecodeGenerator::Dispatch(AST::MulNode* node)
		{
			DispatchArithmetic<MulInstruction, AST::MulNode>(node);
		}

		inline void BytecodeGenerator::Dispatch(AST::DivNode* node)
		{
			DispatchArithmetic<DivInstruction, AST::DivNode>(node);
		}

		inline void BytecodeGenerator::Dispatch(AST::UnaryMinusNode* node)
		{
			node->child->Accept(this);
			m_program.Write(UnaryMinusInstruction);
		}

		inline void BytecodeGenerator::Dispatch(AST::UnaryPlusNode* node)
		{
			node->child->Accept(this);
		}

		inline void BytecodeGenerator::Dispatch(AST::CombineNode* node)
		{
			node->childA->Accept(this);
			node->childB->Accept(this);
		}

		class JITGenerator : public IIGenerator, public ErrorHandler
		{
		public:
			void Dispatch(AST::FloatNode* node) override;

			void Dispatch(AST::RVariableNode* node) override;

			void Dispatch(AST::FunctionCallNode* node) override;

			void Dispatch(AST::AddNode* node) override;

			void Dispatch(AST::SubNode* node) override;

			void Dispatch(AST::MulNode* node) override;

			void Dispatch(AST::DivNode* node) override;

			void Dispatch(AST::UnaryMinusNode* node) override;

			void Dispatch(AST::UnaryPlusNode* node) override;

			void Dispatch(AST::CombineNode* node) override;

			JitProgram& GetProgram()
			{
				return m_program;
			}

			void Finalize()
			{
				m_program.pop_xmm(0);
				m_program.ret();
				m_program.Translate();
			}

		private:
			JitProgram m_program;
		};

		inline void JITGenerator::Dispatch(AST::FloatNode* node)
		{
			m_program.movabs_rax();
			m_program.write_v64b(node->m_value);
			m_program.push_rax();
		}

		inline void JITGenerator::Dispatch(AST::RVariableNode* node)
		{
			m_program.movabs_rax();
			uint64_t id = m_program.Symbol(node->m_name);
			m_program.write_m64b(id);
			m_program.push_mrax();
		}

		inline void JITGenerator::Dispatch(AST::FunctionCallNode* node)
		{
			for (auto& expr : node->args)
			{
				expr->Accept(this);
			}

			for (int i = (int)node->args.size()-1; i >= 0; --i)
			{
				m_program.pop_xmm(i);
			}

			m_program.movabs_rax();
			uint64_t id = m_program.Symbol(detail::Mangle(node->m_name, node->args.size()));
			m_program.write_m64b(id);
			m_program.call_rax();
			m_program.push_xmm(0);
		}

		inline void JITGenerator::Dispatch(AST::AddNode* node)
		{
			node->LHS->Accept(this);
			node->RHS->Accept(this);
			m_program.pop_xmm(1);
			m_program.pop_xmm(0);
			m_program.add_xmm1_xmm0();
			m_program.push_xmm(0);
		}

		inline void JITGenerator::Dispatch(AST::SubNode* node)
		{
			node->LHS->Accept(this);
			node->RHS->Accept(this);
			m_program.pop_xmm(1);
			m_program.pop_xmm(0);
			m_program.sub_xmm1_xmm0();
			m_program.push_xmm(0);
		}

		inline void JITGenerator::Dispatch(AST::MulNode* node)
		{
			node->LHS->Accept(this);
			node->RHS->Accept(this);
			m_program.pop_xmm(1);
			m_program.pop_xmm(0);
			m_program.mul_xmm1_xmm0();
			m_program.push_xmm(0);
		}

		inline void JITGenerator::Dispatch(AST::DivNode* node)
		{
			node->LHS->Accept(this);
			node->RHS->Accept(this);
			m_program.pop_xmm(1);
			m_program.pop_xmm(0);
			m_program.div_xmm1_xmm0();
			m_program.push_xmm(0);
		}

		inline void JITGenerator::Dispatch(AST::UnaryMinusNode* node)
		{
			node->child->Accept(this);
			m_program.pop_xmm(0);
			m_program.neg_xmm0();
			m_program.push_xmm(0);
		}

		inline void JITGenerator::Dispatch(AST::UnaryPlusNode* node)
		{
			node->child->Accept(this);
		}

		inline void JITGenerator::Dispatch(AST::CombineNode* node)
		{
			node->childA->Accept(this);
			node->childB->Accept(this);
		}
	}
}
