#pragma once
#include <map>
#include <glm/glm.hpp>


template <typename T>
class MController
{
public:
	virtual ~MController(){};

	MController(): m_start(0), m_end(0), m_duration(0), m_start_time(-1e9) {};

	MController(float duration): m_duration(duration), m_start_time(-1e9) {};

	T GetValue(float time) const
	{
		auto v = (time - m_start_time) / m_duration;
		v = v > 1.0 ? 1.0 : v;
		return glm::mix(m_start, m_end, v);
	}

	void SetEnd(T v)
	{
		m_end = v;
	}

	void SetStartTime(float time)
	{
		m_start = GetValue(time);
		m_start_time = time;
	}

private:
	T m_start;
	T m_end;
	float m_duration;
	float m_start_time;
};
