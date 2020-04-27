#include "Texture.h"
#include <GL/gl3w.h>
#include <stdio.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <string.h>

using namespace Render;

Texture::Texture(): header({{0, 0, 0}, 0, 0, Invalid, false, false }), m_textureHandle(uint32_t(-1))
{
	glGenTextures(1, &m_textureHandle);
}

void Texture::Bind(int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(header.gltextype, m_textureHandle);
}

void Texture::UnBind()
{
	glBindTexture(header.gltextype, 0);
}

Texture::~Texture()
{
	if (m_textureHandle != uint32_t(-1))
	{
		glDeleteTextures(1, &m_textureHandle);
		m_textureHandle = -1;
	}
}

TexturePtr Texture::LoadTexture(TextureReader reader)
{
	TexturePtr texture = std::make_shared<Texture>();
	texture->header.size = reader.GetSize(0);

	auto decoded = Render::DecodePixelType((uint64_t)reader.GetFormat().pixel_format);
	// int channel_count = decoded.channel_names.size();
	int dimensionality = 3;
	texture->header.type = Texture::Texture_3D;
	if (texture->header.size.z == 1)
	{
		dimensionality = 2;
		texture->header.type = Texture::Texture_2D;
	}
	if (texture->header.size.y == 1)
	{
		dimensionality = 1;
		texture->header.type = Texture::Texture_1D;
	}
	texture->header.cubemap = reader.GetFaceCount() == 6;
	if (texture->header.cubemap)
	{
		assert(dimensionality == 2);
		texture->header.type = Texture::Texture_Cube;
	}

	assert(glm::all(glm::greaterThanEqual(texture->header.size, glm::ivec3(0))));
	assert(reader.GetFaceCount() == 1 || reader.GetFaceCount() == 6);

	texture->header.MIPMapCount = reader.GetMipmapCount();
	texture->header.cubemap = reader.GetFaceCount() == 6;
	texture->header.compressed = decoded.compressed;

	texture->header.gltextype = 0;
	switch(texture->header.type)
	{
		case Texture::Texture_1D:
			texture->header.gltextype = GL_TEXTURE_1D;
			break;
		case Texture::Texture_2D:
			texture->header.gltextype = GL_TEXTURE_2D;
			break;
		case Texture::Texture_3D:
			texture->header.gltextype = GL_TEXTURE_3D;
			break;
		case Texture::Texture_Cube:
			texture->header.gltextype = GL_TEXTURE_CUBE_MAP;
			break;
	}

	texture->Bind(0);

	auto glformat = Render::GetGLMappedTypes(reader.GetFormat());
	uint32_t internal_format = glformat[0];
	uint32_t import_format = glformat[1];
	uint32_t channel_type = glformat[2];

	for (int mipmap = 0; mipmap < reader.GetMipmapCount(); ++mipmap)
	{
		for (int face = 0; face < reader.GetFaceCount(); ++face)
		{
			auto block_size = reader.GetSize(mipmap);
			auto blob = reader.Read(mipmap, face);

			if (texture->header.compressed)
			{
				glCompressedTexImage2D(texture->header.gltextype + face, mipmap, internal_format, block_size.x, block_size.y, 0, blob.size, blob.data.get());
			}
			else
			{
				glTexImage2D(texture->header.gltextype + face, mipmap, internal_format, block_size.x, block_size.y, 0, import_format, channel_type, blob.data.get());
			}
		}
	}

	if (texture->header.MIPMapCount > 1)
	{
		glTexParameteri(texture->header.gltextype, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		glTexParameteri(texture->header.gltextype, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glTexParameteri(texture->header.gltextype, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	texture->UnBind();

	return texture;
}

#include <doctest.h>

TEST_CASE("[Render] PVRReader")
{
	spdlog::info("TexturePtr size: {}", sizeof(TexturePtr));
}

