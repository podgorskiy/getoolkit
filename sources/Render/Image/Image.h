#pragma once
#include "utils/align.h"
#include "utils/common.h"
#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Image
{
public:
	enum DataType: uint8_t
	{
		R8 = 0,
		R16,
		R32,
		RG16,
		RGB8,
		RGBA8,

		FLOAT_POINT = 0x10,
		RF = FLOAT_POINT,
		RGF,
		RGBF,
		RGBAF,

		COMPRESSED = 0x20
	};

	enum DFType
	{
		Chamfer3x3,
		DeadReckoning3x3,
		DeadReckoning5x5
	};

	enum
	{
		Alignment = 16
	};

	typedef uint8_t byte;
	typedef glm::vec<3, uint8_t> pixelRGB8;
	typedef glm::vec<4, uint8_t> pixelRGBA8;
	typedef glm::vec<2, float> pixelRGF;
	typedef glm::vec<3, float> pixelRGBF;
	typedef glm::vec<4, float> pixelRGBAF;
	typedef byte pixelR8;
	typedef uint16_t pixelR16;
	typedef uint32_t pixelR32;
	typedef glm::vec<2, int16_t> pixelRG16;
	typedef float pixelRF;

	// Generation
	static Image Empty(glm::ivec2 size, DataType d);

	template<typename T>
	static Image Empty(glm::ivec2 size);

	static Image Empty(const Image& sameAs);

	Image Copy() const;

	Image OpenView(const glm::ivec2 pos, const glm::ivec2 size);

	// IO
	static Image FromRawData(const void* data, DataType d, glm::ivec2 size);

	void* ToRawData();

	void Assign(const Image& x);
	
	// Basic operation
	Image Pad(int padding_x, int padding_y) const;

	Image Transpose() const;

	Image Resample(glm::ivec2 size, bool fast=false) const;

	// Attributes
	DataType GetType() const;

	size_t GetRowSizeAligned() const;

	size_t GetRowSize() const;

	bool IsValid() const;

	static size_t GetBPP(DataType d);

	size_t GetBPP() const;

	glm::ivec2 GetSize() const;

	int GetChannelCount() const;

	int rows() const;

	int cols() const;

	// Elements access
	template<typename T>
	T* GetRow(int j);

	template<typename T>
	const T* GetRow(int j) const;

	template<typename T>
	const T* ptr(int j) const;

	template<typename T>
	T* ptr(int j);

	// Carving
	Image CarveVertical(const std::vector<short>& seam) const;

	void CarveVerticalInplace(const std::vector<short>& seam);

	template<typename T>
	Image ResizeSimple(Image seamMapX, Image seamMapY, glm::ivec2 newSize) const;
	template<typename T>
	Image ResizeNaive(Image seamMapX, Image seamMapY, glm::ivec2 newSize) const;
	template<typename T>
	Image ResizeSemi(Image seamMapX, Image seamMapY, const std::vector<uint16_t>& col, const std::vector<uint16_t>& rows, glm::ivec2 newSize) const;
	template<typename T>
	Image ResizeCollisionFix(Image seamMapX, Image seamMapY, glm::ivec2 newSize) const;

	// Casting
	template<typename T>
	Image Cast() const;

	// Scalar ops
	Image Mul(float x) const;

	Image Mul(int x) const;

	Image Add(float x) const;

	Image Add(int x) const;

	Image Div(int x) const;

	Image Step(float x) const;

	Image Step(int x) const;

	//Elementwise ops
	Image Add(const Image& x) const;

	Image Sub(const Image& x) const;

	Image Mul(const Image& x) const;

	Image Div(const Image& x) const;

	// Misc
	Image ComputeDF(DFType type = DeadReckoning3x3) const;

	Image ConnectedComponents(int* count = nullptr) const;

	Image GaussBlur(float r) const;

	Image GaussBlurX(float r) const;

	template<typename T>
	Image ResizeX(Image seamMap, int reduction, bool fixCollisions = false) const;
	template<typename T>
	Image ResizeY(Image seamMap, int reduction, bool fixCollisions = false) const;
	template<typename T>
	Image ResizeX(const std::vector<uint16_t>& col, int reduction) const;
	template<typename T>
	Image ResizeY(const std::vector<uint16_t>& col, int reduction) const;

	template<typename T, typename I>
	Image _GaussBlurX(float x) const;

	template<typename V, typename D>
	Image _Mul(V x) const;

	template<typename V, typename D>
	Image _Div(V x) const;

	template<typename V, typename D>
	Image _Add(V x) const;

	template<typename V, typename D>
	Image _Sub(V x) const;

	template<typename T>
	Image _Add(Image x) const;

	template<typename T>
	Image _Sub(Image x) const;

	template<typename T>
	Image _Mul(Image x) const;

	template<typename T>
	Image _Div(Image x) const;

	template<typename V, typename D>
	Image _Step(V x) const;

	template<typename V, typename D>
	Image _StepV(V x) const;

	template<typename T1, typename T2>
	Image _Cast(Image& out) const;

	void SaveToTGA(const char* filename);

private:
	std::shared_ptr<byte> _ptr;
	glm::ivec2 size = glm::ivec2(0);
	size_t data_size = 0;
	size_t offset = 0;
	size_t row_size = 0;
	size_t row_stride = 0;
	DataType dataType = R8;
};

inline Image operator - (const Image& a)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul(-1.0f);
	}
	else
	{
		return a.Mul(-1);
	}
}

inline Image operator + (const Image& a)
{
	return a;
}

inline Image operator + (const Image& a, const Image& x)
{
	return a.Add(x);
}

inline Image operator - (const Image& a, const Image& x)
{
	return a.Sub(x);
}

inline Image operator * (const Image& a, const Image& x)
{
	return a.Mul(x);
}

inline Image operator / (const Image& a, const Image& x)
{
	return a.Div(x);
}

inline Image operator * (const Image& a, float x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul(x);
	}
	else
	{
		return a.Mul((int)x);
	}
}

inline Image operator * (const Image& a, double x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul((float)x);
	}
	else
	{
		return a.Mul((int)x);
	}
}

inline Image operator * (const Image& a, int x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul((float)x);
	}
	else
	{
		return a.Mul((int)x);
	}
}

template<typename T, int d>
inline Image operator * (const Image& a, glm::vec<d, T> x)
{
	return a._Mul<glm::vec<d, T>, glm::vec<d, T> >(x);
}

template<typename T, int d>
inline Image operator * (glm::vec<d, T> x, const Image& a)
{
	return a * x;
}

inline Image operator * (float x, const Image& a)
{
	return a * x;
}

inline Image operator * (double x, const Image& a)
{
	return a * x;
}

inline Image operator / (const Image& a, float x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul(1.0f / (float)x);
	}
	else
	{
		return a.Div((int)x);
	}
}

inline Image operator / (const Image& a, double x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul(1.0f / (float)x);
	}
	else
	{
		return a.Div((int)x);
	}
}

template<typename T, int d>
inline Image operator / (const Image& a, glm::vec<d, T> x)
{
	return a._Div<glm::vec<d, T>, glm::vec<d, T> >(x);
}

inline Image operator * (int x, const Image& a)
{
	return a * x;
}

inline Image operator / (const Image& a, int x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Mul(1.0f / (float)x);
	}
	else
	{
		return a.Div((int)x);
	}
}

inline Image operator + (const Image& a, int x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Add((float)x);
	}
	else
	{
		return a.Add((int)x);
	}
}

inline Image operator + (const Image& a, float x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Add((float)x);
	}
	else
	{
		return a.Add((int)x);
	}
}

inline Image operator + (const Image& a, double x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Add((float)x);
	}
	else
	{
		return a.Add((int)x);
	}
}

template<typename T, int d>
inline Image operator + (const Image& a, glm::vec<d, T> x)
{
	return a._Add<glm::vec<d, T>, glm::vec<d, T> >(x);
}

template<typename T, int d>
inline Image operator + (glm::vec<d, T> x, const Image& a)
{
	return a + x;
}

inline Image operator + (int x, const Image& a)
{
	return a + x;
}

inline Image operator + (float x, const Image& a)
{
	return a + x;
}

inline Image operator + (double x, const Image& a)
{
	return a + x;
}

inline Image operator - (const Image& a, int x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Add(-(float)x);
	}
	else
	{
		return a.Add(-(int)x);
	}
}

inline Image operator - (const Image& a, float x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Add(-(float)x);
	}
	else
	{
		return a.Add(-(int)x);
	}
}

inline Image operator - (const Image& a, double x)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return a.Add(-(float)x);
	}
	else
	{
		return a.Add(-(int)x);
	}
}

template<typename T, int d>
inline Image operator - (const Image& a, glm::vec<d, T> x)
{
	return a._Sub<glm::vec<d, T>, glm::vec<d, T> >(x);
}

template<typename T, int d>
inline Image operator - (glm::vec<d, T> x, const Image& a)
{
	return -a + x;
}

inline Image operator - (int x, const Image& a)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return (a * -1.0f).Add((float)x);
	}
	else
	{
		return (a * -1).Add((int)x);
	}
}

inline Image operator - (float x, const Image& a)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return (a * -1.0f).Add((float)x);
	}
	else
	{
		return (a * -1).Add((int)x);
	}
}

inline Image operator - (double x, const Image& a)
{
	if (a.GetType() & Image::FLOAT_POINT)
	{
		return (a * -1.0f).Add((float)x);
	}
	else
	{
		return (a * -1).Add((int)x);
	}
}

inline glm::ivec2 Image::GetSize() const
{
	return size;
}

inline Image::DataType Image::GetType() const
{
	return dataType;
}

inline size_t Image::GetRowSizeAligned() const
{
	return row_stride;
}

inline size_t Image::GetRowSize() const
{
	return size.x * GetBPP();
}

template<typename T>
inline T* Image::GetRow(int j)
{
	return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
}

template<typename T>
inline const T* Image::GetRow(int j) const
{
	return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
}

template<typename T>
inline T* Image::ptr(int j)
{
	return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
}

template<typename T>
inline const T* Image::ptr(int j) const
{
	return reinterpret_cast<T*>(_ptr.get() + offset + row_stride * j);
}

template<typename T1, typename T2>
inline T2 CastPixel(T1 x)
{
	return static_cast<T2>(x);
}

template<>
inline float CastPixel(Image::pixelRG16 x)
{
	return static_cast<float>(x.x);
}

template<>
inline float CastPixel(Image::pixelRGB8 x)
{
	return static_cast<float>(x.x);
}

template<>
inline float CastPixel(Image::pixelRGBA8 x)
{
	return static_cast<float>(x.x);
}

template<>
inline float CastPixel(Image::pixelRGF x)
{
	return static_cast<float>(x.x);
}

template<>
inline float CastPixel(Image::pixelRGBF x)
{
	return static_cast<float>(x.x);
}

template<>
inline float CastPixel(Image::pixelRGBAF x)
{
	return static_cast<float>(x.x);
}

template<>
inline Image::pixelRGBF CastPixel(Image::pixelR32 x)
{
	return Image::pixelRGBF((float)x);
}

template<>
inline Image::pixelRGBF CastPixel(Image::pixelRG16 x)
{
	return Image::pixelRGBF(x.x, x.y, 0.0f);
}

template<>
inline Image::pixelRGBF CastPixel(Image::pixelRGF x)
{
	return Image::pixelRGBF(x.x, x.y, 0.0f);
}

template<>
inline Image::pixelRGB8 CastPixel(Image::pixelR16 x)
{
	return Image::pixelRGB8((Image::byte)x);
}

template<>
inline Image::pixelRGB8 CastPixel(Image::pixelRGF x)
{
	return Image::pixelRGB8((Image::byte)x.x, (Image::byte)x.y, 0);
}

template<>
inline Image::pixelRGB8 CastPixel(Image::pixelRF x)
{
	return Image::pixelRGB8((Image::byte)x);
}

template<>
inline Image::pixelRGB8 CastPixel(Image::pixelRG16 x)
{
	return Image::pixelRGB8((Image::byte)x.x, (Image::byte)x.y, 0);
}

template<>
inline Image::byte CastPixel<Image::pixelRG16, Image::byte>(Image::pixelRG16 x)
{
	return Image::byte(x.x);
}

template<>
inline Image::byte CastPixel<Image::pixelRGB8, Image::byte>(Image::pixelRGB8 x)
{
	return Image::byte(x.x);
}

template<>
inline Image::byte CastPixel<Image::pixelRGBA8, Image::byte>(Image::pixelRGBA8 x)
{
	return Image::byte(x.x);
}

template<>
inline Image::byte CastPixel<Image::pixelRGF, Image::byte>(Image::pixelRGF x)
{
	return Image::byte(x.x);
}

template<>
inline Image::byte CastPixel<Image::pixelRGBF, Image::byte>(Image::pixelRGBF x)
{
	return Image::byte(x.x);
}

template<>
inline Image::byte CastPixel<Image::pixelRGBAF, Image::byte>(Image::pixelRGBAF x)
{
	return static_cast<Image::byte>(x.x);
}

template<typename T1, typename T2>
Image Image::_Cast(Image& out) const
{
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T1* src = GetRow<T1>(j);
		T2* dst = out.GetRow<T2>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = CastPixel<T1, T2>(src[i]);
		}
	}
	return out;
}

template<typename T>
inline Image Image::Cast() const
{
	Image out = Empty<T>(GetSize());
	switch (dataType)
	{
	case R8: return _Cast<pixelR8, T>(out);
	case R16: return _Cast<pixelR16, T>(out);
	case R32: return _Cast<pixelR32, T>(out);
	case RG16: return _Cast<pixelRG16, T>(out);
	case RGB8: return _Cast<pixelRGB8, T>(out);
	case RGBA8: return _Cast<pixelRGBA8, T>(out);
	case RF: return _Cast<pixelRF, T>(out);
	case RGF: return _Cast<pixelRGF, T>(out);
	case RGBF: return _Cast<pixelRGBF, T>(out);
	case RGBAF: return _Cast<pixelRGBAF, T>(out);
	case COMPRESSED:
	default:
		throw utils::runtime_error("Can not handle compressed types");
	}
	return out;
}

inline size_t Image::GetBPP(DataType d)
{
	switch (d)
	{
	case R8: return sizeof(pixelR8);
	case R16: return sizeof(pixelR16);
	case R32: return sizeof(pixelR32);
	case RG16: return sizeof(pixelRG16);
	case RGB8: return sizeof(pixelRGB8);
	case RGBA8: return sizeof(pixelRGBA8);
	case RF: return sizeof(pixelRF);
	case RGF: return sizeof(pixelRGF);
	case RGBF: return sizeof(pixelRGBF);
	case RGBAF: return sizeof(pixelRGBAF);
	default: return 0;
	}
}

inline size_t Image::GetBPP() const
{
	return GetBPP(dataType);
}

template<typename V, typename D>
inline Image Image::_Mul(V x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const D* src = GetRow<D>(j);
		D* dst = out.GetRow<D>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = src[i] * x;
		}
	}
	return out;
}

template<typename V, typename D>
inline Image Image::_Add(V x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const D* src = GetRow<D>(j);
		D* dst = out.GetRow<D>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = src[i] + x;
		}
	}
	return out;
}

template<typename V, typename D>
inline Image Image::_Sub(V x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const D* src = GetRow<D>(j);
		D* dst = out.GetRow<D>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = src[i] - x;
		}
	}
	return out;
}

template<typename V, typename D>
inline Image Image::_Div(V x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const D* src = GetRow<D>(j);
		D* dst = out.GetRow<D>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = src[i] / x;
		}
	}
	return out;
}

template<typename V, typename D>
inline Image Image::_Step(V x) const
{
	Image out = Image::Empty(GetSize(), R8);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const D* src = GetRow<D>(j);
		byte* dst = out.GetRow<byte>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = (src[i] > D(x)) * 255;
		}
	}
	return out;
}

template<typename V, typename D>
inline Image Image::_StepV(V x) const
{
	Image out = Image::Empty(GetSize(), R8);
	byte ar[2] = { (byte)0, (byte)255 };
	for (int j = 0; j < GetSize().y; ++j)
	{
		const D* src = GetRow<D>(j);
		byte* dst = out.GetRow<byte>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = ar[glm::all(glm::greaterThan(src[i], D(x)))];
		}
	}
	return out;
}

template<typename T>
inline Image Image::_Add(Image x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* srcA = GetRow<T>(j);
		const T* srcB = x.GetRow<T>(j);
		T* dst = out.GetRow<T>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = srcA[i] + srcB[i];
		}
	}
	return out;
}

template<typename T>
inline Image Image::_Sub(Image x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* srcA = GetRow<T>(j);
		const T* srcB = x.GetRow<T>(j);
		T* dst = out.GetRow<T>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = srcA[i] - srcB[i];
		}
	}
	return out;
}

template<typename T>
inline Image Image::_Mul(Image x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* srcA = GetRow<T>(j);
		const T* srcB = x.GetRow<T>(j);
		T* dst = out.GetRow<T>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = srcA[i] * srcB[i];
		}
	}
	return out;
}

template<typename T>
inline Image Image::_Div(Image x) const
{
	Image out = Image::Empty(*this);
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* srcA = GetRow<T>(j);
		const T* srcB = x.GetRow<T>(j);
		T* dst = out.GetRow<T>(j);
		for (int i = 0; i < GetSize().x; ++i)
		{
			dst[i] = srcA[i] / srcB[i];
		}
	}
	return out;
}


inline int Image::GetChannelCount() const
{
	switch (dataType)
	{
		case R8:
		case R16:
		case R32:
		case RF:
			return 1;
		case RG16:
		case RGF:
			return 2;
		case RGB8:
		case RGBF:
			return 3;
		case RGBA8:
		case RGBAF:
			return 4;
		default:
			return 0;
	}
}


template<typename T, int size>
int GetArrLength(T(&)[size]) { return size; }

#define POINT_BEGIN(I, T) {for (int j = 1; j < (I).GetSize().y - 1; ++j){ for (int i = 1; i < (I).GetSize().x - 1; ++i) { T& x = *((I).GetRow<T>(j) + i);
#define POINT_END }}}

#define POINT_BEGIN_2(IN1, IN2, OUT, Tin, Tout) {for (int j = 1; j < (IN1).GetSize().y - 1; ++j){ for (int i = 1; i < (IN1).GetSize().x - 1; ++i)\
{ Tin& in1 = *((IN1).GetRow<Tin>(j) + i); Tin& in2 = *((IN1).GetRow<Tin>(j) + i); Tout& out = *((OUT).GetRow<Tout>(j) + i);


#define CONVOLVE_2D_1D_X(IN, OUT, H, Tin, Tout) \
OUT = Image::Empty(IN.GetSize() - glm::ivec2(GetArrLength(H) - 1, 0), IN.GetType());\
{int l = GetArrLength(H); for (int j = 0; j < (OUT).GetSize().y; ++j){for (int i = 0; i < (OUT).GetSize().x; ++i){ \
		Tout& out = *((OUT).GetRow< Tout >(j) + i);	out = Tout(0); \
		for (int ki = 0; ki < l; ++ki)	{ \
			const Tin& in = *((IN).GetRow< Tin >(j) + i + ki); \
			out += H[ki] * Tout(in); \
}}}}

#define CONVOLVE_2D_1D_Y(IN, OUT, H, Tin, Tout) \
OUT = Image::Empty(IN.GetSize() - glm::ivec2(0, GetArrLength(H) - 1), IN.GetType());\
{int l = GetArrLength(H); for (int j = 0; j < (OUT).GetSize().y; ++j){for (int i = 0; i < (OUT).GetSize().x; ++i){ \
		Tout& out = *((OUT).GetRow< Tout >(j) + i);	out = Tout(0); \
		for (int kj = 0; kj < l; ++kj)	{ \
			const Tin& in = *((IN).GetRow< Tin >(j + kj) + i); \
			out += H[kj] * Tout(in); \
}}}}

inline int inline_abs(int i)
{
	const int mask = i >> (sizeof(int) * CHAR_BIT - 1);
	return (i + mask) ^ mask;
}

inline Image FitIn(Image im, glm::ivec2 size)
{
	float rx = im.GetSize().x / float(size.x);
	float ry = im.GetSize().y / float(size.y);

	Image back = Image::Empty(im);

	if (ry > rx)
	{
		im = im.Resample(glm::ivec2(im.GetSize().x / ry, size.y));
		back.OpenView(glm::ivec2((back.GetSize().x - im.GetSize().x) / 2, 0), im.GetSize()) = im;
	}
	else
	{
		im = im.Resample(glm::ivec2(size.x, im.GetSize().y / rx));
		back.OpenView(glm::ivec2(0, (back.GetSize().y - im.GetSize().y) / 2), im.GetSize()) = im;
	}
	return back;
}
