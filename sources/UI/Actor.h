#pragma once
#include "UI/color.h"
#include "utils/aabb.h"
#include "UI/View.h"
#include "UI/Enums.h"

#include <glm/matrix.hpp>

#include <memory>
#include <inttypes.h>


namespace UI
{
	class Actor;
	class Block;

	typedef std::shared_ptr<Actor> ActorPtr;
	typedef std::shared_ptr<Block> BlockPtr;

	class Actor
	{
	public:
		Actor() = default;

		virtual ~Actor(){}
	private:
		BlockPtr m_block;
	};
}
