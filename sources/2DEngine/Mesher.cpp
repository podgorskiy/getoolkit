#include "Mesher.h"

using namespace Render;


void Mesher::PrimReserve(int idx_count, int vtx_count)
{
	m_vertexArray.resize(m_vertexArray.size() + vtx_count);
	m_indexArray.resize(m_indexArray.size() + idx_count);
	m_index_write_ptr = &m_indexArray[m_indexArray.size() - idx_count];
	m_vertex_write_ptr = &m_vertexArray[m_vertexArray.size() - vtx_count];
}


void Mesher::PrimReset()
{
	m_vertexArray.resize(0);
	m_indexArray.resize(0);
	m_current_index = 0;
	m_index_write_ptr = &m_indexArray[0];
	m_vertex_write_ptr = &m_vertexArray[0];
}


void Mesher::PrimRect(const glm::vec2& a, const glm::vec2& c, const glm::vec2& uv_a, const glm::vec2& uv_c, color col)
{
	PrimReserve(6, 4);
    glm::vec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    auto idx = m_current_index;
    m_index_write_ptr[0] = idx; m_index_write_ptr[1] = idx+1; m_index_write_ptr[2] = idx+2;
    m_index_write_ptr[3] = idx; m_index_write_ptr[4] = idx+2; m_index_write_ptr[5] = idx+3;
    m_vertex_write_ptr[0].pos = a; m_vertex_write_ptr[0].uv = uv_a; m_vertex_write_ptr[0].col = col;
    m_vertex_write_ptr[1].pos = b; m_vertex_write_ptr[1].uv = uv_b; m_vertex_write_ptr[1].col = col;
    m_vertex_write_ptr[2].pos = c; m_vertex_write_ptr[2].uv = uv_c; m_vertex_write_ptr[2].col = col;
    m_vertex_write_ptr[3].pos = d; m_vertex_write_ptr[3].uv = uv_d; m_vertex_write_ptr[3].col = col;
    m_vertex_write_ptr += 4;
    m_current_index += 4;
    m_index_write_ptr += 6;
}


void Mesher::PrimRect(const glm::vec2& _a, const glm::vec2& _c, const glm::mat2x3& t, const glm::vec2& uv_a, const glm::vec2& uv_c, color col)
{
	PrimReserve(6, 4);
    glm::vec2 b(_c.x, _a.y), d(_a.x, _c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);

    glm::vec2 a = t * glm::vec3(_a, 1.0f);
    glm::vec2 c = t * glm::vec3(_c, 1.0f);
    b = t * glm::vec3(b, 1.0f);
    d = t * glm::vec3(d, 1.0f);

    auto idx = m_current_index;
    m_index_write_ptr[0] = idx; m_index_write_ptr[1] = idx+1; m_index_write_ptr[2] = idx+2;
    m_index_write_ptr[3] = idx; m_index_write_ptr[4] = idx+2; m_index_write_ptr[5] = idx+3;
    m_vertex_write_ptr[0].pos = a; m_vertex_write_ptr[0].uv = uv_a; m_vertex_write_ptr[0].col = col;
    m_vertex_write_ptr[1].pos = b; m_vertex_write_ptr[1].uv = uv_b; m_vertex_write_ptr[1].col = col;
    m_vertex_write_ptr[2].pos = c; m_vertex_write_ptr[2].uv = uv_c; m_vertex_write_ptr[2].col = col;
    m_vertex_write_ptr[3].pos = d; m_vertex_write_ptr[3].uv = uv_d; m_vertex_write_ptr[3].col = col;
    m_vertex_write_ptr += 4;
    m_current_index += 4;
    m_index_write_ptr += 6;
}

void Mesher::PrimRectRounded(const glm::vec2& a, const glm::vec2& c, const glm::vec4& radius, color col)
{
	m_path.PathClear();
	m_path.PathRect(a, c, radius);
	PrimConvexPolyFilled(m_path.Ptr(), m_path.Count(), col);
}

void Mesher::PrimConvexPolyFilled(const glm::vec2* points, int count, color col)
{
    if (count < 3)
        return;

	const float anti_aliasing = 1.0f;
	const color col_trans = color(glm::vec<3, uint8_t>(col), 0);
	const int idx_count = glm::max(0, (count - 2) * 3) + count * 6;
	const int vtx_count = count * 2;
	PrimReserve(idx_count, vtx_count);

	for (int i = 2; i < count; i++)
	{
		m_index_write_ptr[0] = m_current_index;
		m_index_write_ptr[1] = m_current_index + 2 * i - 2;
		m_index_write_ptr[2] = m_current_index + 2 * i - 0;
		m_index_write_ptr += 3;
	}

	glm::vec2 p1 = points[count - 2];
	glm::vec2 p2 = points[count - 1];
	glm::vec2 n1 = glm::normalize(p2 - p1);
	n1 = glm::vec2(n1.y, -n1.x);
	for (int i0 = count - 2, i1 = count - 1, i2 = 0; i2 < count; i0 = i1, i1 = i2++)
	{
		p1 = p2;
		p2 = points[i2];

		glm::vec2 n0 = n1;
		n1 = glm::normalize(p2 - p1);
		n1 = glm::vec2(n1.y, -n1.x);
		float dm_x = (n0.x + n1.x) * 0.5f;
		float dm_y = (n0.y + n1.y) * 0.5f;
		float k = 0.5f * anti_aliasing / std::max(dm_x * dm_x + dm_y * dm_y, 0.5f);
		dm_x *= k;
		dm_y *= k;

		m_vertex_write_ptr[0].pos.x = (p1.x - dm_x);
		m_vertex_write_ptr[0].pos.y = (p1.y - dm_y);
		m_vertex_write_ptr[0].uv = glm::vec2(0.0, 0.0);
		m_vertex_write_ptr[0].col = col;
		m_vertex_write_ptr[1].pos.x = (p1.x + dm_x);
		m_vertex_write_ptr[1].pos.y = (p1.y + dm_y);
		m_vertex_write_ptr[1].uv = glm::vec2(0.0, 0.0);
		m_vertex_write_ptr[1].col = col_trans;
		m_vertex_write_ptr += 2;

		m_index_write_ptr[0] = m_current_index + 2 * i1 + 0;
		m_index_write_ptr[1] = m_current_index + 2 * i0 + 0;
		m_index_write_ptr[2] = m_current_index + 2 * i0 + 1;
		m_index_write_ptr[3] = m_current_index + 2 * i0 + 1;
		m_index_write_ptr[4] = m_current_index + 2 * i1 + 1;
		m_index_write_ptr[5] = m_current_index + 2 * i1 + 0;
		m_index_write_ptr += 6;
	}
	m_current_index += vtx_count;
}

