#pragma once
#include "Shader.h"
#include "VertexBuffer.h"
#include "VertexSpec.h"
#include <glm/glm.hpp>
#include <vector>


namespace Render
{
	class DebugRenderer
	{
		struct Vertex
		{
			glm::vec3 p;
			glm::vec<4, uint8_t> c;
		};
	public:
		static constexpr glm::ivec3 Red()
		{ return glm::ivec3(0xaa, 0x00, 0x00); }

		static constexpr glm::ivec3 Green()
		{ return glm::ivec3(0x00, 0xaa, 0x00); }

		static constexpr glm::ivec3 Yellow()
		{ return glm::ivec3(0xaa, 0x55, 0x00); }

		static constexpr glm::ivec3 Blue()
		{ return glm::ivec3(0x00, 0x00, 0xaa); }

		static constexpr glm::ivec3 Magenta()
		{ return glm::ivec3(0xaa, 0x00, 0xaa); }

		static constexpr glm::ivec3 Cyan()
		{ return glm::ivec3(0x00, 0xaa, 0xaa); }

		static constexpr glm::ivec3 White()
		{ return glm::ivec3(0xaa, 0xaa, 0xaa); }

		DebugRenderer();

		~DebugRenderer();

		void Init();

		void PushVertex(const glm::vec3& p0, const glm::ivec3& color);

		void PushVertex(const glm::vec3& p0, const glm::ivec4& color);

		void EmitLines();

		void EmitLineStrip();

		void EmitPoints();

		void EmitTriangles();

		void Draw(const glm::mat4& viewProjection);

	private:
		std::vector<Vertex> m_vertexArray;
		std::vector<int> m_pointIndexArray;
		std::vector<int> m_lineIndexArray;
		std::vector<int> m_trianglesIndexArray;
		int m_vertexIt;

		unsigned int m_uniform_transform;
		ProgramPtr m_program;
		VertexSpec m_vertexSpec;
	};


	inline void DrawCoordinateSystemOrigin(DebugRenderer& debug, const glm::mat4& transform, float size = 20.0f)
	{
		debug.PushVertex(glm::vec3(0.0f, 0.0f, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(size, 0.0f, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(0.0f, 0.0f, 0.0f), DebugRenderer::Green());
		debug.PushVertex(glm::vec3(0.0f, size, 0.0f), DebugRenderer::Green());
		debug.PushVertex(glm::vec3(0.0f, 0.0f, 0.0f), DebugRenderer::Blue());
		debug.PushVertex(glm::vec3(0.0f, 0.0f, size), DebugRenderer::Blue());
		debug.EmitLines();
		debug.Draw(transform);
	}

	inline void DrawRect(DebugRenderer& debug, glm::vec2 min, glm::vec2 max, const glm::mat4& transform)
	{
		debug.PushVertex(glm::vec3(min, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(max.x, min.y, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(max.x, min.y, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(max, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(max, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(min.x, max.y, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(min.x, max.y, 0.0f), DebugRenderer::Red());
		debug.PushVertex(glm::vec3(min, 0.0f), DebugRenderer::Red());
		debug.EmitLines();
		debug.Draw(transform);
	}

	inline void DrawRect(DebugRenderer& debug, glm::vec2 min, glm::vec2 max, const glm::mat3& transform)
	{
		glm::mat4 tr(1.0f);
		tr[0] = glm::vec4(transform[0], 0);
		tr[1] = glm::vec4(transform[1], 0);
		tr[3] = glm::vec4(glm::vec2(transform[2]), 0.0f, 1.0f);

		DrawRect(debug, min, max, tr);
	}

	inline void DrawCross(DebugRenderer& debug, const glm::ivec3& color, const glm::mat4& transform, float size = 20.0f)
	{
		debug.PushVertex(glm::vec3(-size, 0.0f, 0.0f), color);
		debug.PushVertex(glm::vec3(size, 0.0f, 0.0f), color);
		debug.PushVertex(glm::vec3(0.0f, -size, 0.0f), color);
		debug.PushVertex(glm::vec3(0.0f, size, 0.0f), color);
		debug.PushVertex(glm::vec3(0.0f, 0.0f, -size), color);
		debug.PushVertex(glm::vec3(0.0f, 0.0f, size), color);
		debug.EmitLines();
		debug.Draw(transform);
	}
}
