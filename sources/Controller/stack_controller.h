#pragma once
#include ""
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>


template <typename T>
class MController
{
	typedef std::map<float, T> KeyMap;
public:
	MController(){};
	~MController(){};

	void AddKey(float time, const T& val)
	{
		m_keyMap[time] = normalize(val);
	};

	template <int N, typename Type>
	const glm::vec3 normalize(const glm::vec<N, Type>& v){ return v; };

	const glm::quat normalize(const glm::quat& q){ return glm::normalize(q); };

	T GetValue(float time)
	{
		typename KeyMap::iterator itu = m_keyMap.upper_bound(time);
		typename KeyMap::iterator itl = itu;
		--itl;
		if (itl == m_keyMap.end())
		{
			return itu->second;
		}
		if (itu == m_keyMap.end())
		{
			return itl->second;
		}
		float lowerTime = itl->first;
		T& lowerVal = itl->second;
		float upperTime = itu->first;
		T& upperVal = itu->second;
		if (lowerTime == upperTime)
		{
			return lowerVal;
		}
		return interpolate(lowerTime, lowerVal, upperTime, upperVal, time);
	};

	template <int N, typename Type>
	const glm::vec<N, Type> interpolate(float lowerTime, glm::vec<N, Type> lowerVal, float upperTime, glm::vec<N, Type> upperVal, float time)
	{
		float mix = (time - lowerTime) / (upperTime - lowerTime);
		return lowerVal * (1.0f - mix) + upperVal * mix;
	};
	const glm::quat interpolate(float lowerTime, glm::quat lowerVal, float upperTime, glm::quat upperVal, float time)
	{
		float mix = (time - lowerTime) / (upperTime - lowerTime);
		glm::quat r = glm::slerp(lowerVal, upperVal, mix);
		return normalize(r);
	};
	void Clear()
	{
		m_keyMap.clear();
	};
	int Size()
	{
		return m_keyMap.size();
	};
	void Reserve(int n)
	{
		//m_keyMap.reserve(n);
	};
private:
	KeyMap	m_keyMap;
};
