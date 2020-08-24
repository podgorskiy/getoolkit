#include "Block.h"
#include <functional>
#include <initializer_list>
#include <type_traits>


namespace UI
{
	const static constexpr uint8_t id_map[16] = {
			0xff, 0x00, 0x01, 0xff, // 0000 .. 0011, NA,    Left,   Right,  NA
			0x02, 0xff, 0xff, 0xff, // 0100 .. 0111, Width, NA,     NA,     NA
			0xff, 0x03, 0x04, 0xff, // 1000 .. 1011, NA,    CLeft,  CRight, NA
			0xff, 0xff, 0xff, 0xff  // 1100 .. 1111, NA,    NA,     NA,     NA
	};

    BlockPtr make_block(std::initializer_list<Constraint> constraints)
    {
    	return std::make_shared<Block>(std::initializer_list<Constraint>(constraints));
    }

    BlockPtr make_block(std::initializer_list<Constraint> constraints, Render::color c)
    {
    	auto block = std::make_shared<Block>(std::initializer_list<Constraint>(constraints));
    	block->EmplaceEmitter<SFillEmitter>(c);
    	return block;
    }

    BlockPtr make_block(std::initializer_list<Constraint> constraints,
    		Render::TexturePtr tex,
    		ImSize::Enum size,
    		ImPos::Enum pos,
    		ImTransform::Enum t)
    {
    	auto block = std::make_shared<Block>(std::initializer_list<Constraint>(constraints));
    	block->EmplaceEmitter<SImageEmitter>(std::move(tex), size, pos, t);
    	return block;
    }

	void ComputeValues(const glm::aabb2& root_box, const glm::aabb2& parent_box, const Constraint* cnst, int count, float ppd, float (&cst_values)[2][5], uint8_t (&mask)[2])
	{
		auto p_size = parent_box.size();
		auto r_size = root_box.size();
		for (int i = 0; i < count; ++i)
		{
			int id = uint8_t(cnst[i].type & Constraint::CnstV) >> Constraint::CnstVp;
			float p = p_size[id];
			switch(cnst[i].unit)
			{
				case Constraint::Percentage: p *= cnst[i].value / 100.0f; break;
				case Constraint::ValueHeight: p = r_size.y; p *= cnst[i].value / 100.0f; break;
				case Constraint::ValueWidth: p = r_size.x; p *= cnst[i].value / 100.0f; break;
				case Constraint::RValueHeight: p = p_size.y; p *= cnst[i].value / 100.0f; break;
				case Constraint::RValueWidth: p = p_size.x; p *= cnst[i].value / 100.0f; break;
				case Constraint::ValueMin: p = glm::min(r_size.x, r_size.y); p *= cnst[i].value / 100.0f; break;
				case Constraint::ValueMax: p = glm::max(r_size.x, r_size.y); p *= cnst[i].value / 100.0f; break;
				case Constraint::RValueMin: p = glm::min(p_size.x, p_size.y); p *= cnst[i].value / 100.0f; break;
				case Constraint::RValueMax: p = glm::max(p_size.x, p_size.y); p *= cnst[i].value / 100.0f; break;
				case Constraint::Pixel: p = cnst[i].value / ppd; break;
				case Constraint::Point: p = cnst[i].value; break;
				case Constraint::Centimeters: p = cnst[i].value * (72.0f / 2.54f); break;
				case Constraint::Millimeters: p = cnst[i].value * (72.0f / 25.4f); break;
				case Constraint::Inches: p = cnst[i].value * 72.0f; break;
				default:
					continue;
			}
			mask[id] |= uint8_t(cnst[i].type & uint8_t(Constraint::CnstV - 1));
			int sid = id_map[cnst[i].type & uint8_t(Constraint::CnstV - 1)];
			assert(sid != 0xff);
			cst_values[id][sid] = p;
		}
	}

	glm::aabb2 SolveConstraints(const glm::aabb2& root_box, const glm::aabb2& parent_box, const Constraint* cnst, int count, const float (&cst_values)[2][5], const uint8_t (&mask)[2])
	{
		glm::aabb2 box = parent_box;
		for (uint8_t id = 0; id < 2; ++id)
		{
			float pl = parent_box.minp[id];
			float pr = parent_box.maxp[id];
			float l = pl;
			float r = pr;
			float w = 0.0f;
			switch (mask[id])
			{
				case Constraint::Right:
					l = r = pr - cst_values[id][id_map[Constraint::Right]];
					break;
				case Constraint::CenterRight:
					l = r = pr - cst_values[id][id_map[Constraint::CenterRight]];
					break;
				case Constraint::Left:
					l = r = pl + cst_values[id][id_map[Constraint::Left]];
					break;
				case Constraint::CenterLeft:
					l = r = pl + cst_values[id][id_map[Constraint::CenterLeft]];
					break;
				case Constraint::Left | Constraint::Right:
					l = pl + cst_values[id][id_map[Constraint::Left]];
					r = pr - cst_values[id][id_map[Constraint::Right]];
					break;
				case Constraint::Left | Constraint::Width:
					l = pl + cst_values[id][id_map[Constraint::Left]];
					r = l + cst_values[id][id_map[Constraint::Width]];
					break;
				case Constraint::Width | Constraint::Right:
					r = pr - cst_values[id][id_map[Constraint::Right]];
					l = r - cst_values[id][id_map[Constraint::Width]];
					break;
				case Constraint::CenterLeft | Constraint::Width:
					l = r = pl + cst_values[id][id_map[Constraint::CenterLeft]];
					w = cst_values[id][id_map[Constraint::Width]];
					r += w / 2.0f;
					l -= w / 2.0f;
					break;
				case Constraint::Width:
					l = r = (pr + pl) / 2.0f;
					w = cst_values[id][id_map[Constraint::Width]];
					r += w / 2.0f;
					l -= w / 2.0f;
					break;
				case Constraint::Width | Constraint::CenterRight:
					l = r = pr - cst_values[id][id_map[Constraint::CenterRight]];
					w = cst_values[id][id_map[Constraint::Width]];
					r += w / 2.0f;
					l -= w / 2.0f;
					break;
				case Constraint::Left | Constraint::Right | Constraint::Width:
					l = pl + cst_values[id][id_map[Constraint::Left]];
					r = pr - cst_values[id][id_map[Constraint::Right]];
					w = (cst_values[id][id_map[Constraint::Width]] + (r - l)) / 2.0f;
					l = r = (l + r) / 2.0;
					r += w / 2.0f;
					l -= w / 2.0f;
					break;
				default:
					spdlog::error("Unsupported combination of constraints: {}", mask[id]);
			}
			box.minp[id] = l;
			box.maxp[id] = r;
		}
		auto s_size = box.size();
		for (int i = 0; i < count; ++i)
		{
			int id = uint8_t(cnst[i].type & Constraint::CnstV) >> Constraint::CnstVp;
			float p;
			switch(cnst[i].unit)
			{
				case Constraint::SValueHeight: p = s_size.y; p *= cnst[i].value / 100.0f;
				switch (mask[id])
				{
					case Constraint::Right:
						box.minp[id] = box.maxp[id] - p;
						break;
					case Constraint::Left:
						box.maxp[id] = box.minp[id] + p;
						break;
					case Constraint::CenterLeft:
					case Constraint::CenterRight:
						box.minp[id] -= p / 2;
						box.maxp[id] += p / 2;
						break;
				}
				break;
				case Constraint::SValueWidth: p = s_size.x; p *= cnst[i].value / 100.0f;
				switch (mask[id])
				{
					case Constraint::Right:
						box.minp[id] = box.maxp[id] - p;
						break;
					case Constraint::Left:
						box.maxp[id] = box.minp[id] + p;
						break;
					case Constraint::CenterLeft:
					case Constraint::CenterRight:
						box.minp[id] -= p / 2;
						box.maxp[id] += p / 2;
				}
				break;
				default:
					continue;
			}
		}
		return box;
	}

	glm::vec4 ResolveRadius(const glm::aabb2& this_box, const glm::vec4& values, const Constraint::Unit* units,  float ppd)
	{
    	glm::vec4 output = glm::vec4(0);
		for (int i = 0; i < 4; ++i)
		{
			float p = 0;
			float v = values[i];
			switch(units[i])
			{
				case Constraint::Percentage: p = glm::mean(this_box.size()) * v / 100.0f; break;
				case Constraint::Pixel: p = v / ppd; break;
				case Constraint::Point: p = v; break;
				case Constraint::Centimeters: p = v * (72.0f / 2.54f); break;
				case Constraint::Millimeters: p = v * (72.0f / 25.4f); break;
				case Constraint::Inches: p = v * 72.0f; break;
				default:
					continue;
			}
			output[i] = p;
		}
		return output;
	}

	inline void Traverse(const BlockPtr& block, const BlockPtr& parent, const std::function<void(Block* block, Block* parent)>& lambda)
	{
		lambda(block.get(), parent.get());
		for (auto& child: block->m_childs)
		{
			Traverse(child, block, lambda);
		}
	}

	inline void Traverse(const BlockPtr& block, const BlockPtr& parent, const std::function<void(Block* block, Block* parent)>& lambda_pre, const std::function<void(Block* block, Block* parent)>& lambda_post)
	{
		lambda_pre(block.get(), parent.get());
		for (auto& child: block->m_childs)
		{
			Traverse(child, block, lambda_pre, lambda_post);
		}
		lambda_post(block.get(), parent.get());
	}

	void Render(Render::Renderer2D* renderer, const BlockPtr& root, Render::View view, float time, int flags)
	{
    	renderer->SetUp(view);
    	Render::Encoder* encoder = renderer->GetEncoder();
		Traverse(root, nullptr, [encoder, time, flags](UI::Block* block, UI::Block* parent)
		{
			if (block->IsClipping())
				encoder->PushScissors(block->GetBox());
			block->Emit(encoder, time, flags);
		}, [encoder](UI::Block* block, UI::Block* parent)
		{
			if (block->IsClipping())
				encoder->PopScissors();
		});
		renderer->Draw();
	}

	void InterpolateTransitionValues(const Constraint* tcnst, MController<float>* ctrl, int count, float (&alt_cst_values)[2][5],  float (&dst_cst_values)[2][5], float time)
	{
		for (int i = 0; i < count; ++i)
		{
			int id = uint8_t(tcnst[i].type & Constraint::CnstV) >> Constraint::CnstVp;
			int sid = id_map[tcnst[i].type & uint8_t(Constraint::CnstV - 1)];

			auto v_end = alt_cst_values[id][sid];

			ctrl[i].SetEnd(v_end);
			auto v = ctrl[i].GetValue(time);

			dst_cst_values[id][sid] = v;
		}
	}

	void DoLayout(const BlockPtr& block, const Render::View& view, float time)
	{
		Traverse(block, nullptr, [view, time](Block* block, Block* parent)
		{
			glm::aabb2 parent_box = parent == nullptr ? view.view_box : parent->GetBox();
			const auto& cnst = block->GetConstraints();

			uint8_t mask[2] = {0};
			float cst_values[2][5];
			ComputeValues(view.view_box, parent_box, cnst.data(), cnst.size(), view.GetPixelPerDotScalingFactor(), cst_values, mask);
			glm::aabb2 current_box;

			if (block->GetTransitionMask() != 0)
			{
				auto& ctrl = block->GetControllers();
				const auto& tcnst = block->GetTransitionConstraints();
				auto& ttcnst = block->GetTransitionConstraintsTarget();

				float alt_cst_values[2][5];
				ComputeValues(view.view_box, parent_box, tcnst.data(), tcnst.size(), view.GetPixelPerDotScalingFactor(), alt_cst_values, mask);

				InterpolateTransitionValues(&tcnst[0], &ctrl[0], ctrl.size(), alt_cst_values, cst_values, time);

				if (!ttcnst.empty())
				{
					for (auto& c: ttcnst)
					{
						block->UpdateProp(c.type, c.unit, c.value, time);
					}
					ttcnst.clear();
				}
			}

			current_box = SolveConstraints(view.view_box, parent_box, cnst.data(), cnst.size(), cst_values, mask);

			block->SetBox(current_box);
			block->SetRadius(ResolveRadius(current_box, block->GetRadiusVal(), block->GetRadiusUnits(), view.GetPixelPerDotScalingFactor()));
		});
	}

	void Action(const BlockPtr& block, const Render::View& view, glm::vec2 mouse_pos, bool trigger, bool mouse_left_click)
	{
		Traverse(block, nullptr, [view, mouse_pos, trigger, mouse_left_click](Block* block, Block* parent)
		{
			glm::aabb2 box = block->GetBox();
			if (glm::is_inside(box, mouse_pos))
			{
				// block->SetBox(glm::aabb2(box.minp - 10.0f , box.maxp + 10.0f));
			}
		});
	}
}
