#pragma once
#include <inttypes.h>


namespace UI
{
	struct Constraint
	{
		enum Type : uint8_t
		{
			CnstL = 1u << 0u, // Left
			CnstR = 1u << 1u, // Right
			CnstS = 1u << 2u, // Width
			CnstC = 1u << 3u, // Size
			CnstVp = 4u,
			CnstV = 1u << CnstVp, // Vertical flag

			Invalid = 0x0,
			Left = CnstL,
			Right = CnstR,
			Width = CnstS,
			CenterLeft = CnstC | CnstL,
			CenterRight = CnstC | CnstR,
			Top = CnstV | CnstL,
			Bottom = CnstV | CnstR,
			Height = CnstV | CnstS,
			CenterTop = CnstV | CnstC | CnstL,
			CenterBottom = CnstV | CnstC | CnstR,
		};

		static uint8_t to_mask(Type t)
		{
			if (t & CnstV) return (uint8_t(t) << 4);
			return t;
		}

		enum Unit : uint8_t
		{
			Percentage,
			ValueHeight,
			ValueWidth,
			RValueHeight,
			RValueWidth,
			SValueHeight,
			SValueWidth,
			ValueMin,
			ValueMax,
			RValueMin,
			RValueMax,
			Pixel,
			Point,
			Centimeters,
			Millimeters,
			Inches,
		};

		Constraint(Type type, Unit unit, float value): type(type), unit(unit), value(value)
		{};

		Constraint(): type(Invalid), unit(Pixel), value(0.0f)
		{};

		Type type;
		Unit unit;
		float value;
	};

	struct ImSize
	{
		enum Enum : uint8_t
		{
			Auto,
			Contain,
			Cover,
			Fill,
		};
	};

	struct ImPos
	{
		enum Enum : uint8_t
		{
			LeftTop,
			leftCenter,
			leftBottom,
			RightTop,
			RightCenter,
			RightBottom,
			CenterTop,
			CenterCenter,
			CenterBottom,
		};
	};

	struct ImTransform
	{
		enum Enum : uint8_t
		{
			None = 0,
			FlipX = 1 << 0,
			FlipY = 1 << 1
		};
	};


	namespace lit
	{
#define _LITERAL(Type, T, Unit, U) \
    inline Constraint operator "" _##T##U(long double x) { return Constraint(Constraint::Type, Constraint::Unit, (float)x); }\
    inline Constraint operator "" _##T##U(unsigned long long int x) { return Constraint(Constraint::Type, Constraint::Unit, (float)x); }

#define LITERAL(Type, T)\
    _LITERAL(Type, T, Percentage, pe) _LITERAL(Type, T, Pixel, px) _LITERAL(Type, T, Point, pt) \
    _LITERAL(Type, T, Centimeters, cm) _LITERAL(Type, T, Millimeters, mm) _LITERAL(Type, T, Inches, in) \
    _LITERAL(Type, T, ValueHeight, vh) _LITERAL(Type, T, ValueWidth, vw) \
    _LITERAL(Type, T, ValueMin, vmin) _LITERAL(Type, T, ValueMin, vmax) \
    _LITERAL(Type, T, Point, )

		LITERAL(Left, l)

		LITERAL(Right, r)

		LITERAL(Width, w)

		LITERAL(CenterLeft, cl)

		LITERAL(CenterRight, cr)

		LITERAL(Top, t)

		LITERAL(Bottom, b)

		LITERAL(Height, h)

		LITERAL(CenterTop, ct)

		LITERAL(CenterBottom, cb)

		typedef UI::ImSize S;
		typedef UI::ImPos P;
		typedef UI::ImTransform T;
	}
}
