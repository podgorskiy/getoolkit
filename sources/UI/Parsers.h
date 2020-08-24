#pragma once
#include "Block.h"
#include "Serialization/parser.h"
#include "2DEngine/color.h"


namespace UI
{
	inline bool AcceptUnit(serialization::Parser& parser, Constraint::Unit& unit)
	{
		if (parser.AcceptStr("px")) unit = Constraint::Pixel;
		else if (parser.AcceptStr("%")) unit = Constraint::Percentage;
		else if (parser.AcceptStr("pt")) unit = Constraint::Point;
		else if (parser.AcceptStr("cm")) unit = Constraint::Centimeters;
		else if (parser.AcceptStr("mm")) unit = Constraint::Millimeters;
		else if (parser.AcceptStr("in")) unit = Constraint::Inches;
		else if (parser.AcceptStr("vh%")) unit = Constraint::RValueHeight;
		else if (parser.AcceptStr("vw%")) unit = Constraint::RValueWidth;
		else if (parser.AcceptStr("sh%")) unit = Constraint::SValueHeight;
		else if (parser.AcceptStr("sw%")) unit = Constraint::SValueWidth;
		else if (parser.AcceptStr("vh")) unit = Constraint::ValueHeight;
		else if (parser.AcceptStr("vw")) unit = Constraint::ValueWidth;
		else if (parser.AcceptStr("vmin%")) unit = Constraint::RValueMin;
		else if (parser.AcceptStr("vmax%")) unit = Constraint::RValueMax;
		else if (parser.AcceptStr("vmin")) unit = Constraint::ValueMin;
		else if (parser.AcceptStr("vmax")) unit = Constraint::ValueMax;
		else return false;
		return true;
	}

	inline bool ParseUnitValue(const char* x, Constraint::Unit& unit, float& value)
	{
		serialization::Parser parser(x);
		if (parser.AcceptFloat(value))
		{
			parser.AcceptWhiteSpace();
			if (AcceptUnit(parser, unit))
			{
				parser.AcceptWhiteSpace();
				return parser.EOS();
			}
		}
		return false;
	}

	inline bool ParseTime(const char* x, float& value)
	{
		serialization::Parser parser(x);
		parser.AcceptWhiteSpace();
		if (parser.AcceptFloat(value))
		{
			parser.AcceptWhiteSpace();
			if (parser.AcceptStr("s"))
			{
			}
			else if (parser.AcceptStr("ms"))
			{
				value /= 1000.0f;
			}
			parser.AcceptWhiteSpace();
			return parser.EOS();
		}
		return false;
	}

	inline bool ParseUnitValueList(const char* x, Constraint::Unit* unit, float* value, int max_components, int* components)
	{
		serialization::Parser parser(x);

		int k = 0;
		while (k < max_components)
		{
			parser.AcceptWhiteSpace();
			if (parser.AcceptFloat(value[k]))
			{
				parser.AcceptWhiteSpace();
				if (AcceptUnit(parser, unit[k]))
				{
					++k;
					continue;
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
		*components = k;
		parser.AcceptWhiteSpace();
		return parser.EOS();
	}

	inline bool ParseSharpColor(const char* x, Render::color& c)
	{
		serialization::Parser parser(x);
		c[3] = 255;
		uint8_t tmp = 0;
		if (parser.AcceptChar('#') | true)
		{
			if (parser.AcceptHexByte(c[0]))
			{
				if (parser.AcceptHexByte(c[1]))
				{
					if (parser.AcceptHexByte(c[2]))
					{
						if (parser.AcceptHexByte(tmp))
						{
							c[3] = tmp;
						}
						parser.AcceptWhiteSpace();
						return parser.EOS();
					}
				}
			}
		}
		return false;
	}

	inline bool ParseRGBColor(const char* x, Render::color& c)
	{
		serialization::Parser parser(x);
		c[3] = 255;
		int tmp = 0;
		if (parser.AcceptStr("rgb("))
		{
			parser.AcceptWhiteSpace();
			if (parser.AcceptInt(tmp) && parser.AcceptChar(','))
			{
				c[0] = tmp;
				parser.AcceptWhiteSpace();
				if (parser.AcceptInt(tmp) && parser.AcceptChar(','))
				{
					c[1] = tmp;
					parser.AcceptWhiteSpace();
					if (parser.AcceptInt(tmp))
					{
						c[2] = tmp;
						parser.AcceptWhiteSpace();
						if (parser.AcceptStr(")"))
						{
							parser.AcceptWhiteSpace();
							return parser.EOS();
						}
					}
				}
			}
		}
		return false;
	}

	inline bool ParseRGBAColor(const char* x, Render::color& c)
	{
		serialization::Parser parser(x);
		c[3] = 255;
		int tmp = 0;
		if (parser.AcceptStr("rgba("))
		{
			parser.AcceptWhiteSpace();
			if (parser.AcceptInt(tmp) && parser.AcceptChar(','))
			{
				c[0] = tmp;
				parser.AcceptWhiteSpace();
				if (parser.AcceptInt(tmp) && parser.AcceptChar(','))
				{
					c[1] = tmp;
					parser.AcceptWhiteSpace();
					if (parser.AcceptInt(tmp) && parser.AcceptChar(','))
					{
						c[2] = tmp;
						parser.AcceptWhiteSpace();
						if (parser.AcceptInt(tmp))
						{
							c[3] = tmp;
							if (parser.AcceptStr(")"))
							{
								parser.AcceptWhiteSpace();
								return parser.EOS();
							}
						}
					}
				}
			}
		}
		return false;
	}

	inline bool ParseColor(const char* x, Render::color& c)
	{
		if (ParseSharpColor(x, c)) return true;
		else if (ParseRGBColor(x, c)) return true;
		else return ParseRGBAColor(x, c);
	}
}
