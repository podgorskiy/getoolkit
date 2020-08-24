#include "Renderer2D.h"
#include "ScriberTextBackend.h"
#include "Commands.h"
#include "Render/Shader.h"
#include <spdlog/spdlog.h>
#include <fsal.h>
#include <FileInterface.h>
//#include <bgfx/bgfx.h>


using namespace Render;



Renderer2D::Renderer2D(): m_gamma_correction(false)
{
	scissoring_enabled = false;
}

Renderer2D::~Renderer2D()
{

}

void Renderer2D::Init()
{
//	m_programCol = Render::MakeProgram("vs_gui.bin", "fs_gui.bin");
//	m_programTex = Render::MakeProgram("vs_gui.bin", "fs_gui_tex.bin");
//
//	u_texture = m_programTex->GetUniform("u_texture");
//
//	m_vertexSpec
//		.begin()
//		.add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
//		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
//		.add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
//		.end();
//
//	Scriber::Driver::SetCustomIOFunctions(
//			[this](const char* filename, const char* mode)
//			{
//				fsal::Mode m = fsal::kRead;
//				if (strcmp(mode, "rb") == 0)
//				{
//					m = fsal::kRead;
//				}
//				if (strcmp(mode, "wb") == 0)
//				{
//					m = fsal::kWrite;
//				}
//				auto f = fsal::FileSystem().Open(filename, m);
//				if (!f)
//					return (Scriber::UserFile*)nullptr;
//				return (Scriber::UserFile*)new std::shared_ptr<fsal::FileInterface>(f.GetInterface());
//			},
//			+[](Scriber::UserFile* userfile)
//			{
//				delete (std::shared_ptr<fsal::FileInterface>*)userfile;
//				return 0;
//			},
//			+[](void* ptr, size_t size, size_t count, Scriber::UserFile* userfile)
//			{
//				size_t read = 0;
//				(*(std::shared_ptr<fsal::FileInterface>*)userfile)->ReadData((uint8_t*)ptr, size * count, &read);
//				return read;
//			},
//			+[](Scriber::UserFile* userfile, long int offset, int origin)
//			{
//				auto file = (*(std::shared_ptr<fsal::FileInterface>*)userfile);
//
//				switch (origin)
//				{
//				case 0:
//					return (int)!file->SetPosition(offset);
//				case 1:
//					return (int)!file->SetPosition(file->GetPosition() + offset);
//				case 2:
//					return (int)!file->SetPosition(file->GetSize() + offset);
//				}
//				return 1;
//			},
//			+[](Scriber::UserFile* userfile)
//			{
//				auto file = (*(std::shared_ptr<fsal::FileInterface>*)userfile);
//				return file->GetPosition();
//			});

	//m_text_backend = std::make_shared<TextBackend>();
	m_text_driver.SetBackend(m_text_backend);

	auto id = m_text_driver.NewTypeface("NotoSans");
	m_text_driver.AndFontToTypeface(id, "fonts/NotoSans-Regular.ttf", Scriber::FontStyle::Regular);
}

void Renderer2D::SetUp(View view_box)
{
	auto prj = glm::ortho(view_box.view_box.minp.x, view_box.view_box.maxp.x, view_box.view_box.maxp.y, view_box.view_box.minp.y);
	//bgfx::setViewTransform(ViewIds::GUI, nullptr, &prj[0]);
}


void Renderer2D::Draw()
{
	auto command_queue =  m_encoder.GetCommandQueue();

	command_queue.Write(C_End);
	command_queue.Seek(0);
	m_mesher.PrimReset();

	//bgfx::TextureHandle tex;

	Command cmd;
	while (command_queue.Read(cmd) && cmd != C_End)
	{
		bool need_flush = false;
		switch(cmd)
		{
			case C_RectCol:
			{
				glm::aabb2 rect;
				color col;
				glm::vec4 radius;
				command_queue.Read(rect);
				command_queue.Read(radius);
				command_queue.Read(col);

				if (m_gamma_correction)
				{
					col = glm::pow(glm::vec4(col) / 255.0f, glm::vec4(2.2f)) * 255.0f;
				}

				if (radius == glm::vec4(0))
				{
					m_mesher.PrimRect(rect.minp, rect.maxp, glm::vec2(0.0, 0.0), glm::vec2(0.0, 0.0), col);
				}
				else
				{
					m_mesher.PrimRectRounded(rect.minp, rect.maxp, radius, col);
				}
			}
			need_flush = true;
			break;

			case C_RectTex:
			{
				glm::aabb2 rect;
				glm::aabb2 uv;
				glm::vec4  radius;
				command_queue.Read(rect);
				command_queue.Read(radius);
				command_queue.Read(uv);
				//command_queue.Read(tex);

				if (radius == glm::vec4(0))
				{
					m_mesher.PrimRect(rect.minp, rect.maxp, uv.minp, uv.maxp, color(0));
				}
				else
				{

				}
			}
			need_flush = true;
			break;

			case C_RectTexTr:
			{
				glm::aabb2 rect;
				glm::aabb2 uv;
				glm::mat2x3 transoform;
				command_queue.Read(rect);
				command_queue.Read(transoform);
				command_queue.Read(uv);
				//command_queue.Read(tex);

				m_mesher.PrimRect(rect.minp, rect.maxp, transoform, uv.minp, uv.maxp, color(0));
			}
			need_flush = true;
			break;

			case C_Text:
			{
				glm::aabb2 rect;
				size_t len;
				command_queue.Read(rect);
				command_queue.Read(len);
				auto ptr = (const char*)command_queue.GetDataPointer() + command_queue.Tell();
				command_queue.Seek(len, fsal::File::CurrentPosition);
				m_text_driver.DrawLabel(ptr, rect.minp.x, rect.minp.y, Scriber::Font(0, 32, Scriber::FontStyle::Regular, 0xFFFFFFFF, 1));
			}
			need_flush = true;
			break;
			case C_SetScissors:
			{
				command_queue.Read(current_sciscors);
				scissoring_enabled = true;
			}
			need_flush = true;
			break;
			case C_ResetScissors:
			{
				current_sciscors.reset();
				scissoring_enabled = false;
			}
			need_flush = true;
			break;
		}

		if (scissoring_enabled)
		{
//			bgfx::setScissor(
//					(int)current_sciscors.minp.x,
//					(int)current_sciscors.minp.y,
//					(int)current_sciscors.size().x,
//					(int)current_sciscors.size().y);
		}

		switch(cmd)
		{
			case C_RectCol:
			{
//				int num_vertex = m_mesher.vcount();
//				int num_index = m_mesher.icount();
//				bgfx::TransientVertexBuffer tvb;
//				bgfx::TransientIndexBuffer tib;
//
//				bgfx::allocTransientVertexBuffer(&tvb, num_vertex, m_vertexSpec);
//				bgfx::allocTransientIndexBuffer(&tib, num_index);
//
//				memcpy(tvb.data, m_mesher.vptr(), num_vertex * sizeof(Vertex));
//				memcpy(tib.data, m_mesher.iptr(), num_index * sizeof(uint16_t));
//
//				m_mesher.PrimReset();
//
//				bgfx::setVertexBuffer(0, &tvb, 0, num_vertex);
//				bgfx::setIndexBuffer(&tib, 0, num_index);
//
//				uint64_t state = 0
//				                 | BGFX_STATE_WRITE_RGB
//				                 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
//				bgfx::setState(state);
//
//				bgfx::submit(ViewIds::GUI, m_programCol->GetHandle(), 0);
			}
			break;
			case C_RectTex:
			{
//				int num_vertex = m_mesher.vcount();
//				int num_index = m_mesher.icount();
//				bgfx::TransientVertexBuffer tvb;
//				bgfx::TransientIndexBuffer tib;
//
//				bgfx::allocTransientVertexBuffer(&tvb, num_vertex, m_vertexSpec);
//				bgfx::allocTransientIndexBuffer(&tib, num_index);
//
//				memcpy(tvb.data, m_mesher.vptr(), num_vertex * sizeof(Vertex));
//				memcpy(tib.data, m_mesher.iptr(), num_index * sizeof(uint16_t));
//
//				m_mesher.PrimReset();
//
//				bgfx::setVertexBuffer(0, &tvb, 0, num_vertex);
//				bgfx::setIndexBuffer(&tib, 0, num_index);
//
//				bgfx::setTexture(0, u_texture.m_handle, tex);
//
//				uint64_t state = 0
//				                 | BGFX_STATE_WRITE_RGB
//				                 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
//				bgfx::setState(state);
//
//				bgfx::submit(ViewIds::GUI, m_programTex->GetHandle(), 0);
			}
			break;
			case C_Text:
			{
				m_text_driver.Render();
			}
			break;
		}

		if (need_flush)
		{
		}
	}
	command_queue.Seek(0);
}
