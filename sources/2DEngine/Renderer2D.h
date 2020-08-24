#pragma once
#include "Encoder.h"
#include "Mesher.h"
#include "View.h"
#include "Vertices.h"
#include "Render/Shader.h"
#include "utils/aabb.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <Scriber.h>
#include <IRenderAPI.h>


namespace Render
{
	class Renderer2D
	{
	public:

		Renderer2D();
	    ~Renderer2D();

	    Encoder* GetEncoder() { return &m_encoder; };

	    void SetUp(View view);
		void Init();
		void Draw();

		bool m_gamma_correction;

	private:
		Scriber::Driver m_text_driver;

		Encoder m_encoder;
		Mesher m_mesher;

		bool scissoring_enabled = false;

		glm::aabb2 current_sciscors;

		Render::ProgramPtr m_programCol;
		Render::ProgramPtr m_programTex;
		Render::Uniform u_texture;
//		bgfx::VertexLayout m_vertexSpec;
		Scriber::IRenderAPIPtr m_text_backend;
	};
}
