#include "Encoder.h"
#include "Commands.h"
#include <MemRefFile.h>

using namespace Render;


Encoder::Encoder()
{
	m_sciscors.set_any();
	m_command_queue = fsal::File(new fsal::MemRefFile());
}

void Encoder::PushScissors(glm::aabb2 box)
{
	m_command_queue.Write(C_SetScissors);

	if (!scissors_stack.empty())
	{
		auto rect = scissors_stack.back();
		box &= rect;
	}
	m_sciscors = box;
	scissors_stack.push_back(box);
	m_command_queue.Write(box);
}

void Encoder::PopScissors()
{
	scissors_stack.pop_back();
	if (!scissors_stack.empty())
	{
		auto box = scissors_stack.back();
		m_command_queue.Write(C_SetScissors);
		m_command_queue.Write(box);
		m_sciscors = box;
	}
	else
	{
		m_command_queue.Write(C_ResetScissors);
		m_sciscors.set_any();
	}
}

void Encoder::Rect(glm::aabb2 rect, color col, glm::vec4 radius)
{
	m_command_queue.Write(C_RectCol);
	m_command_queue.Write(rect);
	m_command_queue.Write(radius);
	m_command_queue.Write(col);
}

void Encoder::Rect(glm::aabb2 rect, TexturePtr texture, glm::aabb2 uv, glm::vec4 radius)
{
	if (glm::is_overlapping(m_sciscors, rect))
	{
		m_command_queue.Write(C_RectTex);
		m_command_queue.Write(rect);
		m_command_queue.Write(radius);
		m_command_queue.Write(uv);
		m_command_queue.Write(texture.get());
	}
}

void Encoder::Rect(glm::aabb2 rect, const glm::mat3& transform, TexturePtr texture, glm::aabb2 uv)
{
	if (glm::is_overlapping(m_sciscors, rect))
	{
		m_command_queue.Write(C_RectTex);
		m_command_queue.Write(rect);
		m_command_queue.Write(transform);
		m_command_queue.Write(uv);
		m_command_queue.Write(texture.get());
	}
}

void Encoder::Text(glm::aabb2 rect, const char* text, size_t len)
{
	m_command_queue.Write(C_Text);
	m_command_queue.Write(rect);
	if (len == 0)
	{
		len = strlen(text);
	}
	m_command_queue.Write(len + 1);
	m_command_queue.Write((const uint8_t*)text, len);
	m_command_queue.Write(char(0));
}
