#include "Path.h"

using namespace Render;


void Path::PathArcTo(const glm::vec2& center, float radius, float a_min, float a_max, int num_segments)
{
	if (radius == 0.0f)
	{
		m_path.push_back(center);
		return;
	}

	m_path.reserve(m_path.size() + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = a_min + ((float) i / (float) num_segments) * (a_max - a_min);
		m_path.push_back(center + glm::vec2(glm::cos(a), glm::cos(a)) * radius);
	}
}

void Path::Path90Arc(const glm::vec2& center, float radius, ArcType type)
{
	if (radius == 0.0f)
	{
		m_path.push_back(center);
		return;
	}

	int num_segments = int(0.5f + 5.2f / 4.0f * (float)powf(radius - 0.8f, 0.53f));
	num_segments = std::min(num_segments, 180);


	m_path.reserve(m_path.size() + (num_segments + 1));
	for (int i = 0; i <= num_segments; i++)
	{
		const float a = ((float) i / (float) num_segments) * glm::pi<float>() / 2.0f;
		switch(type)
		{
			case Arc_TL:
				m_path.push_back(center + glm::vec2(-glm::cos(a),-glm::sin(a)) * radius);
				break;
			case Arc_TR:
				m_path.push_back(center + glm::vec2( glm::sin(a),-glm::cos(a)) * radius);
				break;
			case Arc_BR:
				m_path.push_back(center + glm::vec2( glm::cos(a), glm::sin(a)) * radius);
				break;
			case Arc_BL:
				m_path.push_back(center + glm::vec2(-glm::sin(a), glm::cos(a)) * radius);
				break;
		}
	}
}

void Path::PathRect(const glm::vec2& a, const glm::vec2& c, const glm::vec4& radius)
{
    glm::vec2 b(c.x, a.y), d(a.x, c.y);
	Path90Arc(a + glm::vec2( radius.x, radius.x), radius.x, Arc_TL);
	Path90Arc(b + glm::vec2(-radius.y, radius.y), radius.y, Arc_TR);
	Path90Arc(c + glm::vec2(-radius.z,-radius.z), radius.z, Arc_BR);
	Path90Arc(d + glm::vec2( radius.w,-radius.w), radius.w, Arc_BL);
}
