#pragma once
#include "utils/stack_vector.h"
#include "2DEngine/color.h"
#include "utils/glm_ext.h"
#include "2DEngine/View.h"
#include "UI/Enums.h"
#include "UI/Emitters.h"
#include "Render/Texture.h"
#include "Controller/mcontroller.h"

#include <glm/matrix.hpp>

#include <memory>
#include <initializer_list>
#include <inttypes.h>
#include <2DEngine/Renderer2D.h>


namespace UI
{
	class Block;
	typedef std::shared_ptr<Block> BlockPtr;

	class Block
	{
		friend void Traverse(const BlockPtr& block, const BlockPtr& parent, const std::function<void(Block* block, Block* parent)>& lambda);
		friend void Traverse(const BlockPtr& block, const BlockPtr& parent, const std::function<void(Block* block, Block* parent)>& lambda_pre, const std::function<void(Block* block, Block* parent)>& lambda_post);
	public:
		typedef stack::vector<MController<float>, 1> PropControllers;
		typedef stack::vector<Constraint, 1> TransitionConstraints;

		Block() = default;
		~Block()
		{
			if (has_emitter) GetEmitter()->~IEmitter();
		}

		explicit Block(std::initializer_list<Constraint> cnst): m_constraints(cnst) {}

		void AddChild(const BlockPtr& child) { m_childs.push_back(child); }
		glm::vec2 GetPositionUL() const { return m_box.minp; }
		glm::vec2 GetPositionC() const { return m_box.center(); }
		glm::vec2 GetSize() const { return m_box.size(); }
		glm::mat3 GetTransform() const
		{
			glm::vec2 d = GetSize();
			glm::vec2 p = GetPositionC();
			return {
					glm::vec3(m_rotation * d.x, 0.0f),
					glm::vec3(glm::vec2(-m_rotation.y, m_rotation.x) * d.y, 0.0f),
					glm::vec3(p, 1.0)
			};
		}
		glm::aabb2 GetBox() const { return m_box; }
		void SetBox(const glm::aabb2& box) { m_box = box; }
		glm::vec4 GetRadius() const { return m_radius; }
		glm::vec4 GetRadiusVal() const { return m_radius_val; }
		const Constraint::Unit* GetRadiusUnits() const { return &m_radius_unit[0]; }
		void SetRadius(const glm::vec4& r) { m_radius = r; }
		void SetRadiusVal(const glm::vec4& r) { m_radius_val = r; }
		void SetRadiusUnit(const Constraint::Unit* u) { memcpy(m_radius_unit, u, 4 * sizeof(Constraint::Unit)); }
		void PushConstraint(const Constraint& cnst) { m_constraints.push_back(cnst); };
		const stack::vector<Constraint, 4>& GetConstraints() const { return m_constraints; };
		PropControllers& GetControllers() { return m_controllers; };
		const TransitionConstraints& GetTransitionConstraints() const { return m_transition_constraints; };
		TransitionConstraints& GetTransitionConstraintsTarget() { return m_transition_target_constraints; };
		void Emit(Render::Encoder* r, float time = 0.0f, int flags = 0) {	if (has_emitter) (*GetEmitter())(r, this, time, flags); }
		void PushTargetTransitionConstraints(const Constraint& cnst) { m_transition_target_constraints.push_back(cnst); };

		template <typename R, typename... Ts>
		void EmplaceEmitter(Ts&&... args) {
		    new (userdata) R(std::forward<Ts>(args)...);
		    has_emitter = true;
		}
		void EnableClipping(bool flag) { clip_overflow = flag; }
		bool IsClipping() const { return clip_overflow; }

		uint8_t GetTransitionMask() { return m_transition_mask; }

		void UpdateProp(Constraint::Type type, Constraint::Unit new_unit, float new_value, float time)
		{
			for (int i = 0, l = m_constraints.size(); i < l; ++i )
			{
				if (m_constraints[i].type == type)
				{
					if (Constraint::to_mask(type) & m_transition_mask)
					{
						for (int j = 0, jl = m_transition_constraints.size(); j < jl; ++j)
						{
							if (m_transition_constraints[j].type == type)
							{
								m_transition_constraints[j].unit = new_unit;
								m_transition_constraints[j].value = new_value;
								m_controllers[j].SetStartTime(time);
								return;
							}
						}
						m_constraints[i].unit = new_unit;
						m_constraints[i].value = new_value;
					}
					else
					{
						m_constraints[i].unit = new_unit;
						m_constraints[i].value = new_value;
						return;
					}
				}
			}
		}

		void SetTransitionProperty(Constraint::Type type, float duration)
		{
			for (int i = 0, l = m_constraints.size(); i < l; ++i)
			{
				if (m_constraints[i].type == type)
				{
					m_transition_mask |= Constraint::to_mask(type);
					m_controllers.push_back(MController<float>(duration));
					m_transition_constraints.push_back(m_constraints[i]);
					return;
				}
			}
			spdlog::error("SetTransitionProperty {}, but it does not exist", int(type));
		}

	private:
		IEmitter* GetEmitter() { return (IEmitter*)userdata; }

		glm::aabb2 m_box;
		glm::vec4 m_radius;
		glm::vec4 m_radius_val = glm::vec4(0);
		Constraint::Unit m_radius_unit[4] = { Constraint::Point };
		stack::vector<Constraint, 4> m_constraints;
		TransitionConstraints m_transition_constraints;
		TransitionConstraints m_transition_target_constraints;
		PropControllers m_controllers;
		uint8_t m_transition_mask;

		stack::vector<BlockPtr, 4> m_childs;
		glm::vec2 m_rotation = glm::vec2(1.0f, 0.0f);
		uint8_t userdata[EmitterSizeCheck::DataSize] = {0};
		bool has_emitter = false;
		bool clip_overflow = false;
	};

    BlockPtr make_block(std::initializer_list<Constraint> constraints);

    BlockPtr make_block(std::initializer_list<Constraint> constraints, Render::color c);

    BlockPtr make_block(std::initializer_list<Constraint> constraints,
    		Render::TexturePtr tex,
    		ImSize::Enum size = ImSize::Auto,
    		ImPos::Enum pos = ImPos::LeftTop,
    		ImTransform::Enum t = ImTransform::None);

	void Render(Render::Renderer2D* renderer, const BlockPtr& root, Render::View view, float time = 0.0f, int flags = 0);

	void DoLayout(const BlockPtr& block, const Render::View& view, float time);

	void Action(const BlockPtr& root, const Render::View& view, glm::vec2 mouse_pos, bool trigger, bool mouse_left_click);
}
