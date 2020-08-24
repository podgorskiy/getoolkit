#pragma once
#include <string>

namespace ExpessionEvaluator
{
	namespace detail
	{
		inline std::string Mangle(const std::string& func_name, int arg_count)
		{
			char buff[256];
			sprintf(buff, "_Z%d%s%s", (int) func_name.size(), func_name.c_str(), std::string(arg_count, 'd').c_str());
			return buff;
		}

		inline std::string Demangle(const std::string& symbol)
		{
			const char* it = symbol.c_str();

			auto accept = [&it](char c)
			{
				if (*it == c)
				{
					++it;
					return true;
				}
				return false;
			};
			auto accept_int = [&it](int& d)
			{
				char* pEnd;
				long int number = strtol(it, &pEnd, 10);
				if (it != pEnd)
				{
					it = pEnd;
					d = number;
					return true;
				}
				return false;
			};
			if (accept('_'))
			{
				if (accept('Z'))
				{
					int size;
					if (accept_int(size))
					{
						if ((int) strlen(it) >= size)
						{
							std::string name(it, it + size);
							it += size;
							int arg_count = 0;
							std::string args;
							if (accept('d'))
							{
								args += "double";
								++arg_count;
								while (accept('d'))
								{
									args += ", double";
									++arg_count;
								}
							}

							if (*it == '\0')
							{
								return std::string("double ") + name + '(' + args + ')';
							}
						}
					}
				}
			}
			return symbol;
		}
	}
}
