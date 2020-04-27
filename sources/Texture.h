#pragma once
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Types.h"
#include "IReader.h"


namespace Render
{
	class Texture;

	typedef std::shared_ptr<Texture> TexturePtr;

	class Texture
	{
		Texture(const Texture&) = delete; // non construction-copyable
		Texture& operator=( const Texture& ) = delete; // non copyable
	public:
		enum TextureType
		{
			Texture_1D,
			Texture_2D,
			Texture_3D,
			Texture_Cube,
			Invalid,
		};

		Texture();

		static TexturePtr LoadTexture(TextureReader reader);

		void Bind(int slot);

		void UnBind();

		struct TextureHeader
		{
			glm::ivec3 size;
			int MIPMapCount;
			uint32_t gltextype;
			TextureType type;
			bool cubemap;
			bool compressed;
		};

		~Texture();

	private:
		TextureHeader header;
		unsigned int m_textureHandle;
	};
}
