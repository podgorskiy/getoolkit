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
	const char* vertex_shader_src = R"(#version 300 es
		in vec2 a_position;
		in vec2 a_uv;
		in vec4 a_color;

		uniform mat4 u_transform;

		out vec4 v_color;

		void main()
		{
			v_color = a_color;
			gl_Position = u_transform * vec4(a_position, 0.0, 1.0);
		}
	)";

	const char* fragment_shader_src = R"(#version 300 es
		precision mediump float;
		in vec4 v_color;
		out vec4 color;

		void main()
		{
			color = v_color;
		}
	)";

	m_program = Render::MakeProgram(vertex_shader_src, fragment_shader_src);

	m_vertexSpec = Render::VertexSpecMaker()
			.PushType<glm::vec2>("a_position")
			.PushType<glm::vec2>("a_uv")
			.PushType<glm::vec<4, uint8_t> >("a_color", true);

	m_vertexSpec.CollectHandles(m_program);

	m_uniform_transform = m_program->GetUniformLocation("u_transform");

	glGenBuffers(1, &m_indexBufferHandle);
	glGenBuffers(1, &m_vertexBufferHandle);

	u_transform = m_program->GetUniform("u_transform");
	u_texture = m_program->GetUniform("u_texture");

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
	//m_text_driver.SetBackend(m_text_backend);

	//auto id = m_text_driver.NewTypeface("NotoSans");
	//m_text_driver.AndFontToTypeface(id, "fonts/NotoSans-Regular.ttf", Scriber::FontStyle::Regular);
}

void Renderer2D::SetUp(View view_box)
{
	m_prj = glm::ortho(view_box.view_box.minp.x, view_box.view_box.maxp.x, view_box.view_box.maxp.y, view_box.view_box.minp.y);
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
				int num_vertex = m_mesher.vcount();
				int num_index = m_mesher.icount();

				GLint id;
				glGetIntegerv(GL_CURRENT_PROGRAM, &id);

				glDisable(GL_DEPTH_TEST);
				glDisable(GL_SCISSOR_TEST);
				glEnable(GL_BLEND);
				glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
				glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

				m_program->Use();
				glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &m_prj[0][0]);

				glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferHandle);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferHandle);

				m_vertexSpec.Enable();

				glBufferData(GL_ARRAY_BUFFER, num_vertex * sizeof(Vertex), m_mesher.vptr(), GL_DYNAMIC_DRAW);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_index * sizeof(int), m_mesher.iptr(), GL_DYNAMIC_DRAW);

				m_mesher.PrimReset();

				if (num_vertex > 2)
				{
					glDrawElements(GL_TRIANGLES, (GLsizei)num_index, GL_UNSIGNED_SHORT, 0); //m_indexArray.data());
				}

				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

				m_vertexSpec.Disable();

				//auto r = std::static_pointer_cast<TextBackend>(m_text_backend);
				//r->m_transform = m_prj;
				//m_text_driver.Render();

				glUseProgram(id);
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
