#pragma once
#include "Lexer.h"
#include "Node.h"
#include "ErrorHandler.h"
#include "BytecodeGenerator.h"

#include <string>
#include <vector>
#include <map>

namespace ExpessionEvaluator
{
	namespace detail
	{
		class Parser : public ErrorHandler
		{
		public:
			Parser();

			void AddSource(const char* source);

			void GenerateProgram(detail::BytecodeGenerator& bcg);

			void GenerateProgram(detail::JITGenerator& bcg);
		private:
			std::string GetLastTokenUnderscore();

			void NextToken();

			Lexer::Token GetLastToken();

			bool Accept(const Lexer::Token& token);

			bool Expect(const Lexer::Token& token);

			AST::INode* Expr();

			AST::INode* Block();

			AST::INode* Term();

			AST::INode* Factor();

			Lexer::Token m_lastToken;

			std::unique_ptr<Lexer> m_lexer;

			AST::INode* m_root;
		};

		inline Parser::Parser(): m_root(nullptr)
		{
		}

		inline void Parser::AddSource(const char* source)
		{
			m_lexer = std::make_unique<Lexer>(source);

			NextToken();

			AST::INode* root = Block();

			if (!root)
			{
				std::string result = GetLastTokenUnderscore();
				SetError("Error, expected block, but \"%s\" was found at position %d.\n%s",
				         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
				         result.c_str());
			}

			m_root = root;
		}

		inline void Parser::GenerateProgram(BytecodeGenerator& bcg)
		{
			m_root->Accept(&bcg);
			bcg.GetProgram().size = bcg.GetProgram().pointer;
		}

		inline void Parser::GenerateProgram(JITGenerator& bcg)
		{
			m_root->Accept(&bcg);
			bcg.Finalize();
			bcg.GetProgram().size = bcg.GetProgram().pointer;
		}

		inline std::string Parser::GetLastTokenUnderscore()
		{
			int start;
			int end;
			GetLastToken().GetPosition(start, end);
			return m_lexer->GetSymbolSequence().PrintAndHighLight(start, end);
		}

		inline void Parser::NextToken()
		{
			m_lastToken = m_lexer->GetNextToken();
		}

		inline Lexer::Token Parser::GetLastToken()
		{
			return m_lastToken;
		}

		inline bool Parser::Accept(const Lexer::Token& token)
		{
			if (GetLastToken() == token)
			{
				NextToken();
				return true;
			}
			return false;
		}

		inline bool Parser::Expect(const Lexer::Token& token)
		{
			if (Accept(token))
			{
				return true;
			}

			char cExpected = token;
			if (static_cast<char>(GetLastToken()) == '\0')
			{
				std::string result = m_lexer->GetSymbolSequence().PrintAndHighLight(GetLastToken().GetPosition());
				SetError("Error: Unexpected end of input\n%s", result.c_str());
			}
			else
			{
				std::string result = GetLastTokenUnderscore();
				std::string cFound = GetLastToken().GetSymbolSequence().ToString();
				SetError("Error: Unexpected token. Expected '%c', but '%s' was found at position: %d.\n%s", cExpected,
				         cFound.c_str(), GetLastToken().GetPosition(), result.c_str());
			}
			return false;
		}

		inline AST::INode* Parser::Factor()
		{
			Lexer::Token token = GetLastToken();

			if (Accept(Lexer::FLOAT) || Accept(Lexer::INT))
			{
				double value = static_cast<double>(atof(token.GetSymbolSequence().ToString().c_str()));
				return new AST::FloatNode(token, value); // Factor -> float
			}
			else if (Accept(Lexer::IDENTIFIER))
			{
				if (Accept(Lexer::LPAR))
				{
					std::vector<AST::INode*> args;
					auto expr = Expr();
					if (expr)
					{
						args.push_back(expr);
						while (Accept(Lexer::COMA))
						{
							auto expr = Expr();
							if (expr)
							{
								args.push_back(expr);
							}
							else
							{
								std::string result = GetLastTokenUnderscore();
								SetError("Error, expected expression, but \"%s\" was found at position %d.\n%s",
								         GetLastToken().GetSymbolSequence().ToString().c_str(),
								         GetLastToken().GetPosition(), result.c_str());
								return nullptr;
							}
						}
					}

					if (Expect(Lexer::RPAR))
					{
						std::string value = token.GetSymbolSequence().ToString();
						return new AST::FunctionCallNode(token, value, args);
					}
					else
					{
						std::string result = GetLastTokenUnderscore();
						SetError("Error, expected ')', but \"%s\" was found at position %d.\n%s",
						         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
						         result.c_str());
						return nullptr;
					}
				}
				else
				{
					std::string value = token.GetSymbolSequence().ToString();
					return new AST::RVariableNode(token, value); // Factor -> id
				}
			}
			else if (Accept(Lexer::MINUS))
			{
				AST::INode* factor = Factor();
				if (factor)
				{
					return new AST::UnaryMinusNode(token, std::unique_ptr<AST::INode>(factor)); // Factor -> -Factor
				}
				else
				{
					std::string result = GetLastTokenUnderscore();
					SetError(
							"Error, wrong unary minus operator, expacted factor, but \"%s\" was found at position %d.\n%s",
							GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
							result.c_str());
					return nullptr;
				}
			}
			else if (Accept(Lexer::PLUS))
			{
				auto factor = Factor();
				if (factor)
				{
					return new AST::UnaryPlusNode(token, std::unique_ptr<AST::INode>(factor)); // Factor -> +Factor
				}
				else
				{
					std::string result = GetLastTokenUnderscore();
					SetError(
							"Error, wrong unary plus operator, expected factor, but \"%s\" was found at position %d.\n%s",
							GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
							result.c_str());
					return nullptr;
				}
			}
			else if (Accept(Lexer::LPAR))
			{
				auto expr = Expr();
				if (expr)
				{
					if (Expect(Lexer::RPAR))
					{
						return expr; // Factor -> (Block)
					}
					else
					{
						std::string result = GetLastTokenUnderscore();
						SetError("Error, expected ')', but \"%s\" was found at position %d.\n%s",
						         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
						         result.c_str());
						return nullptr;
					}
				}
				else
				{
					std::string result = GetLastTokenUnderscore();
					SetError("Error, expected expression, but \"%s\" was found at position %d.\n%s",
					         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
					         result.c_str());
					return nullptr;
				}
			}

			return nullptr;
		}

		inline AST::INode* Parser::Term()
		{
			Lexer::Token token = GetLastToken();

			auto factor = Factor();
			if (factor)
			{
				while (true)
				{
					if (Accept(Lexer::MUL))
					{
				        auto factor2 = Factor();
						if (factor2)
						{
							factor = new AST::MulNode(token, std::unique_ptr<AST::INode>(factor),
													  std::unique_ptr<AST::INode>(factor2)); // Term -> Factor * Factor
						}
						else
						{
							std::string result = GetLastTokenUnderscore();
							SetError("Error, expected term, but \"%s\" was found at position %d.\n%s",
							         GetLastToken().GetSymbolSequence().ToString().c_str(),
							         GetLastToken().GetPosition(),
							         result.c_str());
							return nullptr;
						}
					}
					else if (Accept(Lexer::DIV))
					{
				        auto factor2 = Factor();
						if (factor2)
						{
							factor = new AST::DivNode(token, std::unique_ptr<AST::INode>(factor),
							                        std::unique_ptr<AST::INode>(factor2)); // Block -> Term / Block
						}
						else
						{
							std::string result = GetLastTokenUnderscore();
							SetError("Error, expected term, but \"%s\" was found at position %d.\n%s",
							         GetLastToken().GetSymbolSequence().ToString().c_str(),
							         GetLastToken().GetPosition(),
							         result.c_str());
							return nullptr;
						}
					}
					else
					{
						break;
					}
				}
				return factor; // Term -> factor
			}
			return nullptr;
		}

		inline AST::INode* Parser::Expr()
		{
			Lexer::Token token = GetLastToken();
			AST::INode* term = Term();
			if (term)
			{
				while (true)
				{
					if (Accept(Lexer::PLUS))
					{
						Lexer::Token token2 = GetLastToken();
						auto term2 = Term();
						if (term2)
						{
							term = new AST::AddNode(token, std::unique_ptr<AST::INode>(term),
							                        std::unique_ptr<AST::INode>(term2)); // Block -> Term + Block
							token = token2;
						}
						else
						{
							std::string result = GetLastTokenUnderscore();
							SetError("Error, expected expression, but \"%s\" was found at position %d.\n%s",
							         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
							         result.c_str());
							return nullptr;
						}
					}
					else if (Accept(Lexer::MINUS))
					{
						Lexer::Token token2 = GetLastToken();
						auto term2 = Term();
						if (term2)
						{
							term = new AST::SubNode(token, std::unique_ptr<AST::INode>(term),
							                        std::unique_ptr<AST::INode>(term2)); // Block -> Term - Block
							token = token2;
						}
						else
						{
							std::string result = GetLastTokenUnderscore();
							SetError("Error, expected expression, but \"%s\" was found at position %d.\n%s",
							         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
							         result.c_str());
							return nullptr;
						}
					}
					else
					{
						break;
					}
				}

				return term;
			}
			return nullptr;
		}

		inline AST::INode* Parser::Block()
		{
			auto expr = Expr();
			if (expr)
			{
				if (Accept(Lexer::END_OF_STRING))
				{
					return expr;
				}
				else
				{
					std::string result = GetLastTokenUnderscore();
					SetError("Error, expected end of string, but \"%s\" was found at position %d.\n%s",
					         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
					         result.c_str());
					return nullptr;
				}
			}
			else
			{
				std::string result = GetLastTokenUnderscore();
				SetError("Error, expected expression, but \"%s\" was found at position %d.\n%s",
				         GetLastToken().GetSymbolSequence().ToString().c_str(), GetLastToken().GetPosition(),
				         result.c_str());
				return nullptr;
			}
		}
	}
}
