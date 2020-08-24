#pragma once
#include "Render/Shader.h"
#include <IRenderAPI.h>
// #include <bgfx/bgfx.h>


namespace Render
{
	class TextBackend : public Scriber::IRenderAPI
	{
	public:
		TextBackend();

		void SaveTextureToFile() override
		{};

		void UpdateTexture(Scriber::Image glyph, Scriber::u16vec2 pos) override;

		void ClearTexture() override
		{
		}

		void Render(Scriber::Vertex* vertexBuffer, uint16_t* indexBuffer, uint16_t vertex_count, uint16_t primitiveCount) override;

		//bgfx::TextureHandle m_textureHandle;
		Render::ProgramPtr m_program;
		//bgfx::VertexLayout m_vertexSpec;
		Render::Uniform u_texture;
		Render::Uniform u_stroke;
	};
}
