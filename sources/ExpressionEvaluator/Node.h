#pragma once
#include "SymbolSequence.h"
#include "Lexer.h"

#include <memory>


namespace ExpessionEvaluator
{
	class IIGenerator;

	namespace AST
	{
		class INode
		{
		public:
            INode(Lexer::Token token): m_token(token){};
            ~INode(){};
			virtual void Accept(IIGenerator *dispatcher) = 0;
			Lexer::Token m_token;
		};

		class FloatNode : public INode
		{
		public:
			double m_value;
			FloatNode(Lexer::Token token, double value) : INode(token), m_value(value) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class RVariableNode : public INode
		{
		public:
			std::string m_name;
            RVariableNode(Lexer::Token token, const std::string& name) : INode(token),  m_name(name) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class FunctionCallNode : public INode
        {
        public:
            std::vector<std::unique_ptr<INode> > args;
            std::string m_name;
            FunctionCallNode(Lexer::Token token, const std::string& name, std::vector<INode*> arguments) : INode(token),  m_name(name)
            {
                for (auto& node : arguments)
                {
                    args.push_back(std::unique_ptr<INode>(node));
                }
            };
            virtual void Accept(IIGenerator *dispatcher);
        };

		class AddNode : public INode
		{
		public:
			std::unique_ptr<INode> LHS, RHS;
			AddNode(Lexer::Token token, std::unique_ptr<INode> LHS, std::unique_ptr<INode> RHS) : INode(token),  LHS(std::move(LHS)), RHS(std::move(RHS)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class SubNode : public INode
		{
		public:
			std::unique_ptr<INode> LHS, RHS;
			SubNode(Lexer::Token token, std::unique_ptr<INode> LHS, std::unique_ptr<INode> RHS) : INode(token),  LHS(std::move(LHS)), RHS(std::move(RHS)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class MulNode : public INode
		{
		public:
			std::unique_ptr<INode> LHS, RHS;
			MulNode(Lexer::Token token, std::unique_ptr<INode> LHS, std::unique_ptr<INode> RHS) : INode(token),  LHS(std::move(LHS)), RHS(std::move(RHS)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class DivNode : public INode
		{
		public:
			std::unique_ptr<INode> LHS, RHS;
			DivNode(Lexer::Token token, std::unique_ptr<INode> LHS, std::unique_ptr<INode> RHS) : INode(token),  LHS(std::move(LHS)), RHS(std::move(RHS)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class UnaryMinusNode : public INode
		{
		public:
			std::unique_ptr<INode> child;
            explicit UnaryMinusNode(Lexer::Token token, std::unique_ptr<INode> node) : INode(token),  child(std::move(node)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class UnaryPlusNode : public INode
		{
		public:
			std::unique_ptr<INode> child;
            UnaryPlusNode(Lexer::Token token, std::unique_ptr<INode> node) : INode(token),  child(std::move(node)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};

		class CombineNode : public INode
		{
		public:
			std::unique_ptr<INode> childA, childB;
			CombineNode(std::unique_ptr<INode> childA, std::unique_ptr<INode> childB) : INode(Lexer::Token()),  childA(std::move(childA)), childB(std::move(childB)) {};
			virtual void Accept(IIGenerator *dispatcher);
		};
	}

	class IIGenerator
	{
	public:
		virtual void Dispatch(AST::FloatNode *node) = 0;
		virtual void Dispatch(AST::RVariableNode *node) = 0;
        virtual void Dispatch(AST::FunctionCallNode *node) = 0;
		virtual void Dispatch(AST::AddNode *node) = 0;
		virtual void Dispatch(AST::SubNode *node) = 0;
		virtual void Dispatch(AST::MulNode *node) = 0;
		virtual void Dispatch(AST::DivNode *node) = 0;
		virtual void Dispatch(AST::UnaryMinusNode *node) = 0;
		virtual void Dispatch(AST::UnaryPlusNode *node) = 0;
		virtual void Dispatch(AST::CombineNode *node) = 0;
	};

	inline void AST::AddNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::SubNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::MulNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::DivNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::FloatNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::CombineNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::RVariableNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::FunctionCallNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::UnaryPlusNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
	inline void AST::UnaryMinusNode::Accept(IIGenerator *dispatcher) { dispatcher->Dispatch(this); }
}
