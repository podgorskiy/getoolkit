#pragma once
#include "utils/aabb.h"
#include <inttypes.h>


namespace Render
{
	struct View
	{
		explicit View(glm::aabb2 view_box, int dpi=96): dpi(dpi),
			view_box(view_box.minp / GetPixelPerDotScalingFactor(), view_box.maxp / GetPixelPerDotScalingFactor())
		{}
		explicit View(glm::vec2 view_box, int dpi=96): dpi(dpi), view_box(glm::vec2(0, 0), view_box / GetPixelPerDotScalingFactor()){}

		float GetAspect() const
		{
			return view_box.size().x / float(view_box.size().y);
		}

		float GetPixelPerDotScalingFactor() const
		{
			return dpi / 72.0f;
		}

		uint16_t dpi = 72;
		glm::aabb2 view_box;
	};
}
