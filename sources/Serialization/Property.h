#pragma once
#include <string>

namespace serialization
{
	int GetComponentCount(const char* str);

	template<typename T>
	bool ReadProperty(const std::string& str, T& p);

	template<typename T>
	bool ReadProperty(const char* str, T& p);

	template<typename T>
	inline bool ReadProperty(const std::string& str, T& p)
	{
		return ReadProperty<T>(str.c_str(), p);
	}

	template<typename T>
	std::string WriteProperty(const T& p);
}
