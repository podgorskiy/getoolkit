#pragma once
#include "Encoder.h"
#include "Mesher.h"
#include "View.h"
#include "Vertices.h"
#include "Render/Shader.h"
#include "Render/VertexSpec.h"
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
		Render::Uniform u_transform;
		Render::VertexSpec m_vertexSpec;
		Scriber::IRenderAPIPtr m_text_backend;

		glm::mat4 m_prj;
		unsigned int m_uniform_transform;
		Render::ProgramPtr m_program;
		uint32_t m_vertexBufferHandle;
		uint32_t m_indexBufferHandle;
	};
}
