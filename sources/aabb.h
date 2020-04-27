#pragma once
#include <glm/glm.hpp>
#include <limits>

namespace glm
{
	template <class T>
	class aabb
	{
	public:
		// Default constructor creates aabb with largest possible negative volume.
		// Union with any point will make zero size aabb
		aabb() : minp(std::numeric_limits<T>().max()), maxp(-std::numeric_limits<T>().max()) {}

		// Creates aabb of zero size, where max == p and min == p
		explicit aabb(T p) : minp(p), maxp(p) {}

		// Initializes with given maxp and minp
		aabb(T minp, T maxp) : minp(minp), maxp(maxp) {}

		void set(const T& minp_, const T& maxp_) { minp = minp_; maxp = maxp_; }

		// Sets the abb to largest possible negative volume.
		void reset() { *this = aabb(); }

		T size() const { return maxp - minp; }

		T center() const { return (maxp + minp) / T(2); }

		// Returns true if bounds positive volume
		bool is_positive() const { return glm::all(glm::lessThan(minp, maxp)); }

		// Returns true if bounds negative volume
		bool is_negative() const { return glm::any(glm::greaterThan(minp, maxp)); }

		T minp, maxp;
	};

	// Union of a point with aabb
	template <class T>
	inline aabb<T> operator | (const aabb<T>& x, const T& point)
	{
		return aabb<T>(glm::min(x.minp, point), glm::max(x.maxp, point));
	}

	// Union of a point with aabb
	template <class T>
	inline aabb<T> operator | (const T& point, const aabb<T>& x)
	{
		return aabb<T>(glm::min(x.minp, point), glm::max(x.maxp, point));
	}

	// Union of a point with aabb
	template <class T>
	inline aabb<T> operator |= (const aabb<T>& x, const T& point)
	{
		x = x | point;
		return x;
	}

	// Union of a two aabb's
	template <class T>
	inline aabb<T> operator | (const aabb<T>& a, const aabb<T>& b)
	{
		return (a | b.minp) | b.maxp;
	}

	// Union of a two aabb's
	template <class T>
	inline aabb<T> operator |= (aabb<T>& a, const aabb<T>& b)
	{
		a = a | b;
		return a;
	}

	// Intersection of a two aabb's
	template <class T>
	inline aabb<T> operator & (const aabb<T>& a, const aabb<T>& b)
	{
		return aabb<T>(glm::max(a.minp, b.minp), glm::min(a.maxp, b.maxp));
	}

	// Intersection of a two aabb's
	template <class T>
	inline aabb<T> operator &= (aabb<T>& a, const aabb<T>& b)
	{
		a = a & b;
		return a;
	}

	// If intersection of the two aabb is not empty returns true, otherwise false
	template <class T>
	inline bool is_overlapping(aabb<T> a, aabb<T> b)
	{
		return !(a & b).is_negative();
	}

	// If the point is inside or on the border of the aabb returns true, otherwise false
	template <class T>
	inline bool is_inside(aabb<T> x, T point)
	{
		return glm::all(glm::lessThanEqual(x.minp, point)) && glm::all(glm::greaterThanEqual(x.maxp, point));
	}

	typedef aabb<vec2> aabb2;
	typedef aabb<vec3> aabb3;
	typedef aabb<ivec2> iaabb2;
	typedef aabb<ivec3> iaabb3;
}
