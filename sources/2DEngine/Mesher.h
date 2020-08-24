#pragma once
#include "utils/aabb.h"
#include "color.h"
#include "Vertices.h"
#include "Path.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


namespace Render
{
	class Mesher
	{
	public:
		void PrimReset();

		void PrimReserve(int idx_count, int vtx_count);

		void PrimRect(const glm::vec2& a, const glm::vec2& c, const glm::vec2& uv_a, const glm::vec2& uv_c, color col);
		void PrimRect(const glm::vec2& a, const glm::vec2& c, const glm::mat2x3& t, const glm::vec2& uv_a, const glm::vec2& uv_c, color col);

		void PrimRectRounded(const glm::vec2& a, const glm::vec2& c, const glm::vec4& radius, color col);

		void PrimConvexPolyFilled(const glm::vec2* points, int count, color col);

		int vcount() const { return m_vertexArray.size(); }
		int icount() const { return m_indexArray.size(); }

		const uint16_t* iptr() const { return m_indexArray.data(); }
		const Vertex* vptr() const { return m_vertexArray.data(); }
	private:
		std::vector<uint16_t> m_indexArray;
		std::vector<Vertex> m_vertexArray;
		Path m_path;

		uint16_t m_current_index;
		uint16_t* m_index_write_ptr;
		Vertex* m_vertex_write_ptr;
	};
}
