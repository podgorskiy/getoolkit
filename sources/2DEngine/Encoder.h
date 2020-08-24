#pragma once
#include "Render/Texture.h"
#include "color.h"
#include "utils/aabb.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fsal.h>
//#include <Scriber.h>


namespace Render
{
	class Encoder
	{
	public:
		Encoder();

		void PushScissors(glm::aabb2 box);

		void PopScissors();

		void Rect(glm::aabb2 rect, color c, glm::vec4 radius = glm::vec4(0));

		void Rect(glm::aabb2 rect, TexturePtr texture, glm::aabb2 uv = glm::aabb2(glm::vec2(1.0), glm::vec2(0.0)), glm::vec4 radius = glm::vec4(0));

		void Rect(glm::aabb2 rect, const glm::mat3& transform, TexturePtr texture, glm::aabb2 uv = glm::aabb2(glm::vec2(1.0), glm::vec2(0.0)));

		void Text(glm::aabb2 rect, const char* text, size_t len = 0);

		fsal::File GetCommandQueue() { return m_command_queue; }
	private:
		std::vector<glm::aabb2> scissors_stack;
		glm::aabb2 m_sciscors;
		fsal::File m_command_queue;
	};
}
