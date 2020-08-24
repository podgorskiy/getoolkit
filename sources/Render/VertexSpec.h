#pragma once
#include "Shader.h"
#include "Types.h"
#include <GL/gl3w.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>


namespace Render
{
	class VertexSpec
	{
		friend class VertexSpecMaker;
	public:
		struct Attribute
		{
			std::string name;
			int32_t components = 0;
			uint32_t type = 0;
			uint8_t normalized = 0;
			int32_t stride = 0;
			int32_t offset = 0;
			uint32_t handle = 0;
		};

		VertexSpec() = default;

		explicit VertexSpec(const std::vector<Attribute>& attributes)
		{
			int i = 0;
			for (const auto& a: attributes)
			{
				m_attributes[i++] = a;
			}
		}

		void CollectHandles(const ProgramPtr& program)
		{
			for (int i = 0; m_attributes[i].components != 0; ++i)
			{
				Attribute& attr = m_attributes[i];
				attr.handle = program->GetAttribLocation(attr.name.c_str());
			}
		}

		void Enable(const void* ptr = nullptr)
		{
			for (int i = 0; m_attributes[i].components != 0; ++i)
			{
				Attribute& attr = m_attributes[i];
				if (attr.handle == uint32_t(-1))
					continue;
				glEnableVertexAttribArray(attr.handle);
				auto offset_ = static_cast<size_t>(attr.offset);
				glVertexAttribPointer(attr.handle, attr.components, attr.type, attr.normalized, attr.stride,
				                      (uint8_t*)ptr + offset_);
			}
		}

		void Disable()
		{
			for (int i = 0; m_attributes[i].components != 0; ++i)
			{
				Attribute& attr = m_attributes[i];
				if (attr.handle == uint32_t(-1))
					continue;
				glDisableVertexAttribArray(attr.handle);
			}
		}

		Attribute m_attributes[16];
	};

	class VertexSpecMaker
	{
		VertexSpec spec;
		int num = 0;
		int offset = 0;
	public:
		template<typename T>
		class TypeDesc
		{
		public:
			TypeDesc(): type(VarType::GetGLMapping<T>()), num(1), size(sizeof(T)){}
			const unsigned int type;
			const unsigned int num;
			const unsigned int size;
		};

		template<int C, typename T>
		class TypeDesc<glm::vec<C, T> >
		{
		public:
			TypeDesc(): type(VarType::GetGLMapping<T>()), num(C), size(sizeof(T) * C){}
			const unsigned int type;
			const unsigned int num;
			const unsigned int size;
		};

		template<typename T>
		VertexSpecMaker& PushType(const std::string& name, bool normalized=false)
		{
			TypeDesc<T> tdesc;
			spec.m_attributes[num].type = tdesc.type;
			spec.m_attributes[num].name = name;
			spec.m_attributes[num].offset = offset;
			offset += tdesc.size;
			spec.m_attributes[num].components = tdesc.num;
			spec.m_attributes[num].normalized = normalized;
			num += 1;
			return *this;
		}

		VertexSpecMaker()
		{}

		operator VertexSpec()
		{
			for (int i = 0; i < num; ++i)
			{
				spec.m_attributes[i].stride = offset;
			}
			return spec;
		}
	};
}
