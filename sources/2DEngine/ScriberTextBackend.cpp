#include "ScriberTextBackend.h"
// #include "views.h"

using namespace Render;


TextBackend::TextBackend()
{
	int size = 1024;

//	m_textureHandle = bgfx::createTexture2D(
//			(uint16_t) size, (uint16_t) size, false, 1, bgfx::TextureFormat::RG8,
//			BGFX_SAMPLER_POINT | BGFX_SAMPLER_UVW_CLAMP, nullptr
//	);
//
//	m_program = Render::MakeProgram("vs_text.bin", "fs_text.bin");
//
//	m_vertexSpec
//			.begin()
//			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Int16, true)
//			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true)
//			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
//			.end();

	u_texture = m_program->GetUniform("u_texture");
	u_stroke = m_program->GetUniform("u_stroke");
}


void TextBackend::UpdateTexture(Scriber::Image glyph, Scriber::u16vec2 pos)
{
//	const bgfx::Memory* memory = bgfx::copy(glyph.ptr<const uint8_t*>(0), (int) glyph.GetRowSizeAligned() * glyph.GetSize().y);
//	Scriber::ivec2 p(pos);
//	bgfx::updateTexture2D(m_textureHandle, 0, 0, p.x, p.y, glyph.GetSize().x, glyph.GetSize().y, memory, glyph.GetRowSizeAligned());
}


void TextBackend::Render(Scriber::Vertex* vertexBuffer, uint16_t* indexBuffer, uint16_t vertex_count,
                         uint16_t primitiveCount)
{
//	bgfx::TransientVertexBuffer tvb;
//	bgfx::TransientIndexBuffer tib;
//
//	bgfx::allocTransientVertexBuffer(&tvb, vertex_count, m_vertexSpec);
//	bgfx::allocTransientIndexBuffer(&tib, primitiveCount * 3);
//
//	memcpy(tvb.data, vertexBuffer, vertex_count * sizeof(Scriber::Vertex));
//	memcpy(tib.data, indexBuffer, primitiveCount * 3 * sizeof(uint16_t));
//
//	bgfx::setVertexBuffer(0, &tvb, 0, vertex_count);
//	bgfx::setIndexBuffer(&tib, 0, primitiveCount * 3);
//
//	bgfx::setTexture(0, u_texture.m_handle, m_textureHandle);
//
//	u_stroke.ApplyValue(glm::vec4(0.0, 0.0, 0.0, 1.0));
//
//	uint64_t state = 0
//	                 | BGFX_STATE_WRITE_RGB
//	                 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA);
//	bgfx::setState(state);
//
//	bgfx::submit(ViewIds::GUI, m_program->GetHandle(), 0);
}
