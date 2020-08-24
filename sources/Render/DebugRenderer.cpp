#include "DebugRenderer.h"
#include "Shader.h"
#include <tuple>


using namespace Render;


DebugRenderer::DebugRenderer()
{
}

DebugRenderer::~DebugRenderer()
{

}

void DebugRenderer::Init()
{
	const char* vertex_shader_src = R"(
		attribute vec3 a_position;
		attribute vec4 a_color;

		uniform mat4 u_transform;

		varying vec4 v_color;

		void main()
		{
			v_color = a_color;
			gl_Position = u_transform * vec4(a_position, 1.0);;
		}
	)";

	const char* fragment_shader_src = R"(
		varying vec4 v_color;

		void main()
		{
			gl_FragColor = v_color;
		}
	)";

	m_program = Render::MakeProgram(vertex_shader_src, fragment_shader_src);

	m_vertexSpec = Render::VertexSpecMaker()
			.PushType<glm::vec3>("a_position")
			.PushType<glm::vec<4, uint8_t> >("a_color");

	m_vertexSpec.CollectHandles(m_program);

	m_uniform_transform = m_program->GetUniformLocation("u_transform");
}

void DebugRenderer::PushVertex(const glm::vec3& p, const glm::ivec3& color)
{
	PushVertex(p, glm::ivec4(color, 255));
}

void DebugRenderer::PushVertex(const glm::vec3& p, const glm::ivec4& color)
{
	Vertex v;
	v.p = p;
	v.c = color;
	m_vertexArray.push_back(v);
}

void DebugRenderer::EmitLineStrip()
{
	for (int i = m_vertexIt + 1, s = m_vertexArray.size(); i < s; ++i)
	{
		m_lineIndexArray.push_back(m_vertexIt);
		m_lineIndexArray.push_back(i);
		m_vertexIt = i;
	}
	m_vertexIt = m_vertexArray.size();
}

void DebugRenderer::EmitLines()
{
	for (int i = m_vertexIt, s = m_vertexArray.size(); i < s; ++i)
	{
		m_lineIndexArray.push_back(i);
	}
	m_vertexIt = m_vertexArray.size();
}

void DebugRenderer::EmitPoints()
{
	for (int i = m_vertexIt, s = m_vertexArray.size(); i < s; ++i)
	{
		m_pointIndexArray.push_back(i);
	}
	m_vertexIt = m_vertexArray.size();
}

void DebugRenderer::EmitTriangles()
{
	for (int i = m_vertexIt, s = m_vertexArray.size(); i < s; ++i)
	{
		m_trianglesIndexArray.push_back(i);
	}
	m_vertexIt = m_vertexArray.size();
}

void DebugRenderer::Draw(const glm::mat4& transform)
{
	GLint id;
	glGetIntegerv(GL_CURRENT_PROGRAM, &id);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_program->Use();
	glUniformMatrix4fv(m_uniform_transform, 1, GL_FALSE, &transform[0][0]);

	m_vertexSpec.Enable(&m_vertexArray[0]);

	if (m_lineIndexArray.size() > 1)
	{
		glDrawElements(GL_LINES, (GLsizei)m_lineIndexArray.size(), GL_UNSIGNED_INT, &m_lineIndexArray[0]);
	}
	if (m_pointIndexArray.size() > 0)
	{
		glDrawElements(GL_POINTS, (GLsizei)m_pointIndexArray.size(), GL_UNSIGNED_INT, &m_pointIndexArray[0]);
	}
	if (m_trianglesIndexArray.size() > 2)
	{
		glDrawElements(GL_TRIANGLES, (GLsizei)m_trianglesIndexArray.size(), GL_UNSIGNED_INT, &m_trianglesIndexArray[0]);
	}

	m_vertexSpec.Disable();
	glUseProgram(id);

	m_vertexIt = 0;
	m_lineIndexArray.resize(0);
	m_pointIndexArray.resize(0);
	m_trianglesIndexArray.resize(0);
	m_vertexArray.resize(0);
}
