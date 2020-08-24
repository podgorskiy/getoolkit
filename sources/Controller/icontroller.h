#pragma once
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


template <typename T>
class IController
{
public:
	virtual ~IController(){};

	T GetValue(float time) const;

private:
	KeyMap	m_keyMap;
};
