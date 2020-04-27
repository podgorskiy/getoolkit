#include "Camera2D.h"

#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

Camera2D::Camera2D()
{
	m_mouseNow = glm::ivec3(0);
	m_mouseLast = glm::ivec3(0);
	m_fov = 1.0f;
	m_panningActive = false;
	m_blockMouse = false;
	m_z = 0;
}

glm::mat3 Camera2D::GetCanvasToWorld() const
{
	return 
		  glm::mat3(1.0, 0, 0, 0, -1.0, 0, 0, 0, 1.0) 
		* glm::mat3(1.0 / m_fov, 0, 0, 0, 1.0 / m_fov, 0, 0, 0, 1.0) 
		* glm::mat3(1.0, 0, 0, 0, 1.0, 0, m_pos.x, -m_pos.y, 1.0);
}

glm::mat3 Camera2D::GetWorldToCanvas() const
{
	return glm::inverse(GetCanvasToWorld());
}

void Camera2D::Move(float x, float y)
{
	m_mouseNow = glm::ivec2(x, -y);
	if (m_panningActive)
	{
		m_delta += m_mouseNow - m_mouseLast;
	}
	m_mouseLast = m_mouseNow;

	glm::vec2 canvasPos = GetWorldToCanvas() * glm::vec3(m_mouseNow, 1.0f);
}

void Camera2D::SetFOV(float f)
{
	m_z = 0;
	if (f < 1.0f)
	{
		m_z = -int(round(logf(f) / logf(0.9f)));
	}
	else if (f == 1.0f)
	{
		m_z = 0;
	}
	else
	{
		m_z = int(round(logf(f) / logf(1.1f)));
	}
	Scroll(0);
}

float Camera2D::GetFOV() const
{
	return m_fov;
}

void Camera2D::SetPos(glm::vec2 pos)
{
	m_pos = pos;
}

glm::vec2 Camera2D::GetPos()
{
	return m_pos;
}

void Camera2D::Scroll(float x)
{
	glm::vec2 canvasPosOld = GetWorldToCanvas() * glm::vec3(m_mouseNow, 1.0f);

	m_z += int(round(x));
	m_z = glm::clamp(m_z, -70, 70);
	float fov_old = m_fov;
	m_fov = 1.0f;
	if (m_z < 0)
	{
		for (int i = m_z; i < 0; ++i)
		{
			m_fov *= 0.9f;
		}
	}
	else if (m_z > 0)
	{
		for (int i = 0; i < m_z; ++i)
		{
			m_fov *= 1.1f;
		}
	}

	glm::vec2 canvasPosNew = GetWorldToCanvas() * glm::vec3(m_mouseNow, 1.0f);
	glm::vec2 deltaPos = canvasPosOld - canvasPosNew;
	deltaPos.y *= -1.0;
	m_pos -= deltaPos;
}

void Camera2D::UpdateViewProjection(int w, int h)
{
	height = h;
	width = w;
	if (!m_blockMouse)
	{
		m_pos += glm::vec2(m_delta) * m_fov;
		m_delta = glm::vec2(0.0f);
	}
	if (m_z == 0)
	{
		m_pos = glm::ivec2(m_pos);
	}

	glm::mat3 c2w = GetCanvasToWorld();
	glm::mat3 w2c = GetWorldToCanvas();
	glm::vec2 _pos = c2w * glm::vec3(0, 0, 1.0);
	//m_pos = glm::round(m_pos);
	//m_pos = w2c * glm::vec3(m_pos, 1.0);
	//m_pos.x = -m_pos.x;
	m_view = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / height / m_fov)) * glm::translate(glm::mat4(1.0f), glm::vec3(m_pos, 0.0f));
	float aspect = width * 1.0f / height;
	const auto m = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0, 1.0, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(2.0, 2.0, 0.0));
	m_proj = m * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / aspect, 1.0f, 1.0f));
}


glm::mat4 Camera2D::GetTransform()
{
	return m_proj * m_view;
}

void Camera2D::TogglePanning(bool enable)
{
	m_panningActive = enable;
}
