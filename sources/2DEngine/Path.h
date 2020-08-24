#pragma once
#include "utils/aabb.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>


namespace Render
{
	class Path
	{
	public:
		enum ArcType : uint8_t
		{
			Arc_TL,
			Arc_TR,
			Arc_BR,
			Arc_BL,
		};

		void PathClear() { m_path.clear(); }

		void PathLineTo(const glm::vec2& pos) { m_path.push_back(pos); }

		void PathArcTo(const glm::vec2& center, float radius, float a_min, float a_max, int num_segments);

		void Path90Arc(const glm::vec2& center, float radius, ArcType type);

		void PathRect(const glm::vec2& a, const glm::vec2& b, const glm::vec4& radius);

		const glm::vec2* Ptr() const { return &m_path[0]; }
		int Count() const {return m_path.size(); }

	private:
		std::vector<glm::vec2> m_path;
	};
}
