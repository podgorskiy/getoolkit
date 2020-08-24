#pragma once
#include "color.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Render
{
	struct Vertex
	{
		Vertex() = default;

		Vertex(glm::vec2 pos, glm::vec2 uv, color col): pos(pos), uv(uv), col(col)
		{}

		glm::vec2 pos;
		glm::vec2 uv;
		color col;
	};
}
