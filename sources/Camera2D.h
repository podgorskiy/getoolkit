#pragma once
#include <glm/glm.hpp>

class Camera2D
{
public:
	Camera2D();

	void SetFOV(float f);
	float GetFOV() const;

	void SetPos(glm::vec2 pos);

	glm::vec2 GetPos();

	void Move(float x, float y);

	void Scroll(float x);

	glm::mat4 GetTransform();

	void UpdateViewProjection(int w, int h);

	void TogglePanning(bool down);

	glm::mat3 GetCanvasToWorld() const;
	glm::mat3 GetWorldToCanvas() const;

	bool m_panningActive;
	glm::ivec2 m_mouseNow = glm::ivec2(0);
private:
	

	glm::ivec2 m_mouseLast = glm::ivec2(0);
	glm::vec2 m_pos = glm::vec2(0);
	glm::vec2 m_delta = glm::vec2(0);

	glm::mat4 m_view;
	glm::mat4 m_proj;

	int width = 0;
	int height = 0;

	float m_fov = 1.0f;

	int32_t m_z = 0;

	bool m_blockMouse = false;
};
