#pragma once
#include <map>
#include <glm/glm.hpp>


template <typename T>
class TController
{
public:
	virtual ~TController(){};

	T GetValue(float time) const
	{
		auto v = (m_start_time - time) / m_duration;
		v = v > 1.0 ? 1.0 : v;
		return glm::mix(m_start, m_end, v);
	}

	void SetTo(T x)
	{
		m_start = x;
		m_end = x;
	}

	void TransitionTo(T x, float time)
	{
		m_start = GetValue(time);
		m_end = x;
		m_start_time = time;
	}

	void SetDuration(float d)
	{
		m_duration = d;
	}

private:
	T	m_start;
	T	m_end;
	float m_duration;
	float m_start_time;
};
