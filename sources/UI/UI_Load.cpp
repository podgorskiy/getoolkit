#include "UI_Load.h"
#include "Parsers.h"
#include "utils/assertion.h"
#include "ExpressionEvaluator/ExpressionEvaluator.h"
#include "Render/TextureReaders/TextureLoader.h"
#include <yaml-cpp/yaml.h>
#include <iostream>


namespace UI
{
	void ReadConstraint(YAML::Node node, const char* name, Constraint::Type type, const BlockPtr& blk, ExpessionEvaluator::INTContext& ctx)
	{
		auto read_cst = +[](const BlockPtr& blk, std::string str, Constraint::Type type, ExpessionEvaluator::INTContext& ctx)
		{
			Constraint::Unit unit;
			float value;
			// spdlog::info("{}: {}", name, x);
			if (ParseUnitValue(str.c_str(), unit, value))
			{
				return Constraint(type, unit, value);
			}
			else
			{
				auto pos = str.find(':');
				if (pos != std::string::npos)
				{
					str[pos] = '\0';
					ctx.func("anonymus", str.c_str());
					ctx.Link();
					value = ctx.get_func("anonymus").Eval();
					serialization::Parser parser(str.c_str() + pos + 1);
					parser.AcceptWhiteSpace();
					if (AcceptUnit(parser, unit))
					{
						parser.AcceptWhiteSpace();
						if (parser.EOS())
						{
							return Constraint(type, unit, value);
						}
					}
				}
			}
		};

		auto cnstr = node[name];
		if (cnstr.IsDefined())
		{
			if (cnstr.IsMap())
			{
				auto duration = cnstr["duration"];
				auto d_s = duration.as<std::string>("0");
				float duration_v = 0.0f;
				ParseTime(d_s.c_str(), duration_v);
				auto vcnstr = cnstr["value"];
				auto tcnstr = cnstr["target"];
				std::string x = vcnstr.as<std::string>("0");
				auto ctsr = read_cst(blk, x, type, ctx);
				blk->PushConstraint(ctsr);
				if (tcnstr.IsDefined())
				{
					std::string t = tcnstr.as<std::string>("0");
					blk->PushTargetTransitionConstraints(read_cst(blk, t, type, ctx));
				}

				blk->SetTransitionProperty(type, duration_v);
			}
			else
			{
				std::string x = cnstr.as<std::string>();
				auto ctsr = read_cst(blk, x, type, ctx);
				blk->PushConstraint(ctsr);
			}
		}
	}

	bool BuildList(const BlockPtr& parent, const YAML::Node& sequence, ExpessionEvaluator::INTContext& ctx, const std::map<std::string, Render::color>& color_map);

	void LoadEmmitters(YAML::Node node, const BlockPtr& block, const std::map<std::string, Render::color>& color_map)
	{
		auto bg_color = node["bg_color"];
		auto bg_img = node["bg_img"];
		auto text_node = node["text"];
		if (bg_color.IsDefined())
		{
			Render::color c;
			auto str = bg_color.as<std::string>();
			str.erase(std::remove (str.begin(), str.end(), ' '), str.end());

			if (ParseColor(str.c_str(), c))
			{
			}
			else
			{
				auto it = color_map.find(str);
				if (it != color_map.end())
					c = it->second;
			}
			spdlog::info("Color: {}", c);
    	    block->EmplaceEmitter<SFillEmitter>(c);
		}
		else if (bg_img.IsDefined())
		{
			auto path = bg_img.as<std::string>();
			auto texture = Render::LoadTexture(path);
			spdlog::info("Texture: {}", path);

			ImSize::Enum size = ImSize::Auto;
			auto bg_size = node["bg_size"];
			if (bg_size.IsDefined())
			{
				auto s = bg_size.as<std::string>();
				if (s == "auto")
				{
					size = ImSize::Auto;
				}
				else if (s == "contain")
				{
					size = ImSize::Contain;
				}
				else if (s == "cover")
				{
					size = ImSize::Cover;
				}
				else if (s == "fill")
				{
					size = ImSize::Fill;
				}
				else
				{
					ASSERT(false, "Wrong bg_size %s", s.c_str());
				}
			}
			ImPos::Enum pos = ImPos::LeftTop;
			auto bg_pos = node["bg_pos"];
			if (bg_pos.IsDefined())
			{
				auto s = bg_pos.as<std::string>();
				if (s == "left_top")
				{
					pos = ImPos::LeftTop;
				}
				else if (s == "left_center")
				{
					pos = ImPos::leftCenter;
				}
				else if (s == "left_bottom")
				{
					pos = ImPos::leftBottom;
				}
				else if (s == "right_top")
				{
					pos = ImPos::RightTop;
				}
				else if (s == "right_center")
				{
					pos = ImPos::RightCenter;
				}
				else if (s == "right_bottom")
				{
					pos = ImPos::RightBottom;
				}
				else if (s == "center_top")
				{
					pos = ImPos::CenterTop;
				}
				else if (s == "center_center")
				{
					pos = ImPos::CenterCenter;
				}
				else if (s == "center_bottom")
				{
					pos = ImPos::CenterBottom;
				}
				else
				{
					ASSERT(false, "Wrong bg_pos %s", s.c_str());
				}
			}
			ImTransform::Enum t = ImTransform::None;
			auto bg_transform = node["bg_transform"];
			if (bg_transform.IsDefined())
			{
				auto s = bg_transform.as<std::string>();
				if (s == "none")
				{
					t = ImTransform::None;
				}
				else if (s == "flip_x")
				{
					t = ImTransform::FlipX;
				}
				else if (s == "flip_y")
				{
					t = ImTransform::FlipY;
				}
				else
				{
					ASSERT(false, "Wrong bg_transform %s", s.c_str());
				}
			}
    	    block->EmplaceEmitter<SImageEmitter>(std::move(texture), size, pos, t);
		}
		else if (text_node.IsDefined())
		{
			auto text = text_node.as<std::string>();
			spdlog::info("Text: {}", text);
    	    block->EmplaceEmitter<STextEmitter>(text);
		}
	}

	BlockPtr BuildBlock(YAML::Node node, ExpessionEvaluator::INTContext& ctx, const std::map<std::string, Render::color>& color_map)
	{
		auto block = UI::make_block({});
		auto name = node["name"];
		if (name.IsDefined())
		{
			spdlog::info("Node name: {}", name.as<std::string>());
		}
		ReadConstraint(node, "width", Constraint::Width, block, ctx);
		ReadConstraint(node, "height", Constraint::Height, block, ctx);

		ReadConstraint(node, "top", Constraint::Top, block, ctx);
		ReadConstraint(node, "bottom", Constraint::Bottom, block, ctx);
		ReadConstraint(node, "left", Constraint::Left, block, ctx);
		ReadConstraint(node, "right", Constraint::Right, block, ctx);

		ReadConstraint(node, "ctop", Constraint::CenterTop, block, ctx);
		ReadConstraint(node, "cbottom", Constraint::CenterBottom, block, ctx);
		ReadConstraint(node, "cleft", Constraint::CenterLeft, block, ctx);
		ReadConstraint(node, "cright", Constraint::CenterRight, block, ctx);

		auto cnstr = node["border-radius"];
		if (cnstr.IsDefined())
		{
			std::string x = cnstr.as<std::string>();
			// spdlog::info("{}: {}", name, x);
			glm::vec4 value;
			Constraint::Unit unit[4];
			int components;
			if (ParseUnitValueList(x.c_str(), &unit[0], &value[0], 4, &components))
			{
				switch(components)
				{
					case 1: value[3] = value[2] = value[1] = value[0]; unit[3] = unit[2] = unit[1] = unit[0]; break;
					case 2: value[2] = value[0]; value[3] = value[1]; unit[2] = unit[0]; unit[3] = unit[1]; break;
					case 3: value[3] = value[1]; unit[3] = unit[1]; break;
					default: break;
				}

				block->SetRadiusVal(value);
				block->SetRadiusUnit(unit);
			}
			else
			{
				spdlog::error("Error parsing radius {}: {}", name.as<std::string>("noname"), x);
			}
		}

		LoadEmmitters(node, block, color_map);

		auto blocks = node["blocks"];
		if (blocks.IsDefined())
		{
			BuildList(block, blocks, ctx, color_map);
		}
		auto clip_overflow = node["clip_overflow"];
		if (clip_overflow.IsDefined())
		{
			block->EnableClipping(clip_overflow.as<bool>());
		}

		return block;
	}

	bool BuildList(const BlockPtr& parent, const YAML::Node& sequence, ExpessionEvaluator::INTContext& ctx, const std::map<std::string, Render::color>& color_map)
	{
		ASSERT(sequence.IsSequence(), "'blocks' must be sequence")
		for (YAML::Node node: sequence)
		{
			parent->AddChild(BuildBlock(node, ctx, color_map));
		}
		return true;
	}

	BlockPtr Load(const fsal::File& f)
	{
		using namespace UI::lit;
		using Render::operator""_c;
		spdlog::info("Loading: {}", f.GetPath().string().c_str());
		YAML::Node root_node = YAML::Load(std::string(f));

		ExpessionEvaluator::INTContext ctx;
		//ExpessionEvaluator::JITContext ctx;

		auto vars = root_node["vars"];
		auto colors = root_node["colors"];

		std::vector<double> data;
		if (vars.IsDefined())
		{
			data.resize(vars.size());
			int i = 0;
			for (auto var: vars)
			{
				std::string key = var.first.as<std::string>();
				auto value = var.second.as<double>();
				data[i] = value;
				ctx.var(key, &data[i]);
				++i;
			}
		}
		std::map<std::string, Render::color> color_map;
		if (colors.IsDefined())
		{
			for (auto c: colors)
			{
				std::string key = c.first.as<std::string>();
				auto value = c.second.as<std::string>();
				Render::color col;
				if (ParseColor(value.c_str(), col))
					color_map[key] = col;
			}
		}

		auto root = UI::make_block({0_l, 100_wpe, 0_t, 100_hpe});

		LoadEmmitters(root_node, root, color_map);

		auto blocks = root_node["blocks"];

		ASSERT(blocks.IsSequence(), "'blocks' must be sequence, in file %s", f.GetPath().string().c_str())

		BuildList(root, blocks, ctx, color_map);

		return root;
	}
}
