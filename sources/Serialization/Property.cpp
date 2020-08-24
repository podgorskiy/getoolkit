#include "Property.h"
#include "parser.h"
#include "utils/type_names.h"
#include "utils/assertion.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtx/quaternion.hpp>

#include <float.h>
#include <iterator>
#include <sstream>


namespace serialization
{
	int GetComponentCount(const char* str)
	{
		int i = 0;
		Parser parser(str);
		while(parser.AcceptWhiteSpace());
		while(parser.AcceptNonWhiteSpace())
		{
			while(parser.AcceptNonWhiteSpace());
			while(parser.AcceptWhiteSpace());
			++i;
		}
		return i;
	}

	template<typename T>
	struct PropertyReader
	{
		static bool Read(const char* str, T& p)
		{
			Parser parser(str);
			bool result = parser.Read<T>(p);
			ASSERT(result, "Failed to read %s. Expected type: %s", str, misc::GetTypeName<T>())
			while (parser.AcceptWhiteSpace())
			;
			ASSERT(parser.EOS(), "Not all data read of: %s", str);
			return parser.EOS() && result;
		}
	};

	template<>
	struct PropertyReader<std::string>
	{
		static bool Read(const char* str, std::string& p)
		{
			p = str;
			return true;
		}
	};

	template<int Dim, typename T>
	struct PropertyReader< glm::vec<Dim, T> >
	{
		static bool Read(const char* str, glm::vec<Dim, T>& p)
		{
			Parser parser(str);
			bool result = true;
			for (int i = 0; i < Dim; ++ i)
			{
				if (!parser.Read<T>(p[i]))
				{
					ASSERT(false, "Failed to read %s. Expected type: %s", str, misc::GetTypeName<glm::vec<Dim, T> >());
					result = false;
					break;
				}
			}
			while (parser.AcceptWhiteSpace())
			;
			ASSERT(parser.EOS(), "Not all data read of: %s", str);
			return parser.EOS() && result;
		}
	};

	template<int Dim1, int Dim2, typename T>
	struct PropertyReader< glm::mat<Dim1, Dim2, T> >
	{
		static bool Read(const char* str, glm::mat<Dim1, Dim2, T>& value)
		{
			Parser p(str);
			bool result = true;
			for (int i = 0; i < Dim1; ++i)
			{
				for (int j = 0; j < Dim2; ++j)
				{
					if (!p.Read<T>(value[j][i]))
					{
						ASSERT(false, "Failed to read %s. Expected type: %s", str, misc::GetTypeName<glm::mat<Dim1, Dim2, T> >());
						result = false;
						break;
					}
				}
			}
			while (p.AcceptWhiteSpace())
			;
			ASSERT(p.EOS(), "Not all data read of: %s", str);
			return p.EOS() && result;
		}
	};

	template<typename T>
	bool ReadProperty(const char* str, T& value)
	{
		return PropertyReader<T>::Read(str, value);
	}

	template<typename T>
	struct PropertyWriter
	{
		static std::string Write(const T& p)
		{
			return fmt::format("{}", p);
		}
	};

	template<>
	struct PropertyWriter<float>
	{
		static std::string Write(const float& p)
		{
			char buff[32];
			sprintf(buff, "%f", p);
			char* x = buff + strlen(buff) - 1;
			while(*x == '0') *x-- = '\0';
			if (*x == '.') *x = '\0';
			return buff;
		}
	};

	template<>
	struct PropertyWriter<bool>
	{
		static std::string Write(const bool& p)
		{
			return p ? "true" : "false";
		}
	};

	template<>
	struct PropertyWriter<int>
	{
		static std::string Write(const int& p)
		{
			char buff[16];
			sprintf(buff, "%d", p);
			return buff;
		}
	};

	template<int Dim, typename T>
	struct PropertyWriter< glm::vec<Dim, T> >
	{
		static std::string Write(const glm::vec<Dim, T>& p)
		{
			std::string result;
			for (int i = 0; i < Dim; ++ i)
			{
				result += PropertyWriter<T>::Write(p[i]);
				if (i != Dim)
				{
					result.push_back(' ');
				}
			}
			return result;
		}
	};

	template<int Dim1, int Dim2, typename T>
	struct PropertyWriter< glm::mat<Dim1, Dim2, T> >
	{
		static std::string Write(const glm::mat<Dim1, Dim2, T>& p)
		{
			std::string result;
			for (int i = 0; i < Dim1; ++i)
			{
				for (int j = 0; j < Dim2; ++j)
				{
					result += PropertyWriter<T>::Write(p[j][i]);
					if (i != Dim1 && j != Dim2)
					{
						result.push_back(' ');
					}
				}
			}
			return result;
		}
	};

	template<typename T>
	std::string WriteProperty(const T& p)
	{
		return PropertyWriter<T>::Write(p);
	}

#define RegisterType(X) template bool ReadProperty<X>(const char* str, X& value);\
	template std::string WriteProperty<X>(const X& value);


	RegisterType(bool)
	RegisterType(int)
	RegisterType(float)
	RegisterType(std::string)
	RegisterType(glm::vec2)
	RegisterType(glm::vec3)
	RegisterType(glm::vec4)
	RegisterType(glm::ivec2)
	RegisterType(glm::ivec3)
	RegisterType(glm::ivec4)
	RegisterType(glm::bvec2)
	RegisterType(glm::bvec3)
	RegisterType(glm::bvec4)
	RegisterType(glm::mat2)
	RegisterType(glm::mat3)
	RegisterType(glm::mat4)
	RegisterType(glm::mat2x3)
	RegisterType(glm::mat3x2)
	RegisterType(glm::mat3x4)
	RegisterType(glm::mat4x3)
}
