#include "Image.h"
#include "utils/gaussiun_kernel.h"
#include "opencv_cc.h"
#include "parallelisation.h"
#include <assert.h>
#include <math.h>
#include <stb_image_resize.h>
#include <thread>
#include <algorithm>
#include <string.h>

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#endif

template<typename Ty>
Ty sqr(Ty a)
{
	return a * a;
}

bool Image::IsValid() const
{
	return (data_size != 0) && (misc::align(size.x * GetBPP(), Alignment) * size.y == data_size);
}

Image Image::Empty(glm::ivec2 size, Image::DataType d)
{
	Image im;
	im.size = size;
	im.dataType = d;
	im.row_stride = misc::align(im.GetRowSize(), Alignment);
	im.row_size = im.GetRowSize();
	im.data_size = im.row_stride * (size_t)im.size.y;
	im._ptr.reset(new byte[im.data_size]);
	memset(im._ptr.get(), 0, im.data_size);
	return im;
}

Image Image::Empty(const Image& sameAs)
{
	Image im = Empty(sameAs.size, sameAs.dataType);
	memset(im._ptr.get(), 0, im.data_size);
	return im;
}

Image Image::FromRawData(const void* data, DataType d, glm::ivec2 size)
{
	Image im = Empty(size, d);
	//size_t bpp = im.GetBPP();

	for (int j = 0; j < im.size.y; ++j)
	{
		memcpy(
			im.GetRow<byte>(j),
			(const char*)data + im.row_size * j,
			im.row_size);
	}
	return im;
}

void* Image::ToRawData()
{
	size_t bpp = GetBPP();
	void* p = new uint8_t[bpp * size.x * size.y];
	for (int j = 0; j < size.y; ++j)
	{
		memcpy(
			(char*)p + row_size * j,
			GetRow<byte>(j),
			row_size);
	}
	return p;
}

void Image::Assign(const Image& x)
{
	assert(x.dataType == dataType);
	assert(x.size == size || (size.x % x.size.x == 0 && size.y % x.size.y == 0));
	auto lessoreq = glm::lessThanEqual(x.size, size);
	assert(glm::all(lessoreq));

	size_t bpp = GetBPP();

	int rx = size.x / x.size.x;
	int ry = size.y / x.size.y;
	if (rx == 1 && ry == 1)
	{
		for (int j = 0; j < x.size.y; ++j)
		{
			memcpy(
				GetRow<byte>(j),
				x.GetRow<byte>(j),
				x.row_size);
		}
	}
	else
	{
		for (int j = 0; j < size.y; ++j)
		{
			for (int ri = 0; ri < rx; ++ri)
			{
				memcpy(
					GetRow<byte>(j) + ri * x.size.x * bpp,
					x.GetRow<byte>(j % x.size.y),
					x.row_size);
			}
		}
	}
}

Image Image::OpenView(const glm::ivec2 pos, const glm::ivec2 size)
{
	glm::ivec2 max_point = size + pos;
	auto lessoreq = glm::lessThanEqual(max_point, GetSize());
	assert(glm::all(lessoreq));

	Image view;
	view._ptr = _ptr;
	size_t bpp = GetBPP();
	view.dataType = dataType;
	view.data_size = data_size;
	view.size = size;
	view.offset = pos.y * row_stride + bpp * pos.x;
	view.row_size = bpp * size.x;
	view.row_stride = row_stride;

	return view;
}

Image Image::Pad(int padding_x, int padding_y) const
{
	Image im = Empty(GetSize() + glm::ivec2(padding_x, padding_y) * 2, dataType);

	size_t bpp = GetBPP();

	for (int j = 0; j < im.size.y; ++j)
	{
		int j_ = inline_abs(padding_y - j - 1);
		j_ = inline_abs(size.y - j_ - 1);
		j_ = size.y - j_ - 1;

		const uint8_t* src = GetRow<uint8_t>(j_);
		uint8_t* dst = im.GetRow<uint8_t>(j);

		memcpy(
			dst + bpp * padding_x,
			src,
			row_size);

		for (int i = 0; i < padding_x; ++i)
		{
			int i_ = padding_x - i - 1;
			for (int k = 0; k < (int)bpp; ++k)
			{
				*(dst + bpp * i + k) = *(src + bpp * i_ + k);
			}
		}
		for (int i = im.size.x - padding_x; i < im.size.x; ++i)
		{
			int i_ = -padding_x + i + 1;
			i_ = inline_abs(size.x - i_ - 1);
			i_ = size.x - i_ - 1;
			for (int k = 0; k < (int)bpp; ++k)
			{
				*(dst + bpp * i + k) = *(src + bpp * i_ + k);
			}
		}
	}
	return im;
}

Image Image::Transpose() const
{
	Image im = Empty(glm::ivec2(GetSize().y, GetSize().x), GetType());
	size_t bpp = GetBPP();

	for (int j = 0; j < im.size.y; ++j)
	{
		for (int i= 0; i < im.size.x; ++i)
		{
			uint8_t* dst = im.GetRow<uint8_t>(j) + i * bpp;
			const uint8_t* src = GetRow<uint8_t>(i) + j * bpp;
			memcpy(dst, src, bpp);
		}
	}
	return im;
}

template<typename T>
T WeightedSum(const T& ll, const T& lh, const T& hl, const T& hh, int _ll, int _lh, int _hl, int _hh);


template<>
uint8_t WeightedSum(const uint8_t& ll, const uint8_t& lh, const uint8_t& hl, const uint8_t& hh, int _ll, int _lh, int _hl, int _hh)
{
	int result = ll * _ll + lh * _lh + hl * _hl + hh * _hh;
	return (uint8_t)(result / 0x500);
}

template<>
uint16_t WeightedSum(const uint16_t& ll, const uint16_t& lh, const uint16_t& hl, const uint16_t& hh, int _ll, int _lh, int _hl, int _hh)
{
	int result = ll * _ll + lh * _lh + hl * _hl + hh * _hh;
	return (uint16_t)(result / 0x500);
}

template<>
float WeightedSum(const float& ll, const float& lh, const float& hl, const float& hh, int _ll, int _lh, int _hl, int _hh)
{
	float result = ll * _ll + lh * _lh + hl * _hl + hh * _hh;
	return result / float(0x500);
}

template<int N>
glm::vec<N, uint8_t> WeightedSum(const glm::vec<N, uint8_t>& ll, const glm::vec<N, uint8_t>& lh, const glm::vec<N, uint8_t>& hl, const glm::vec<N, uint8_t>& hh, int _ll, int _lh, int _hl, int _hh)
{
	glm::vec<N, int> result = glm::vec<N, int>(ll) * _ll + glm::vec<N, int>(lh) * _lh + glm::vec<N, int>(hl) * _hl + glm::vec<N, int>(hh) * _hh;
	return glm::vec<N, uint8_t>(result / 0x500);
}

template<int N>
glm::vec<N, uint16_t> WeightedSum(const glm::vec<N, uint16_t>& ll, const glm::vec<N, uint16_t>& lh, const glm::vec<N, uint16_t>& hl, const glm::vec<N, uint16_t>& hh, int _ll, int _lh, int _hl, int _hh)
{
	glm::vec<N, uint16_t> result = ll * _ll + lh * _lh + hl * _hl + hh * _hh;
	return result / 0x500;
}

template<int N>
glm::vec<N, float> WeightedSum(const glm::vec<N, float>& ll, const glm::vec<N, float>& lh, const glm::vec<N, float>& hl, const glm::vec<N, float>& hh, int _ll, int _lh, int _hl, int _hh)
{
	glm::vec<N, float> result = ll * float(_ll) + lh * float(_lh) + hl * float(_hl) + hh * float(_hh);
	return result / float(0x500);
}


template<typename T>
T WeightedSum(const T& l, const T& h, int _l, int _h);


template<>
uint8_t WeightedSum(const uint8_t& l, const uint8_t& h, int _l, int _h)
{
	int result = l * _l + h * _h;
	return (uint8_t)(result / 0x500);
}

template<>
uint16_t WeightedSum(const uint16_t& l, const uint16_t& h, int _l, int _h)
{
	int result = l * _l + h * _h;
	return (uint16_t)(result / 0x500);
}

template<>
float WeightedSum(const float& l, const float& h, int _l, int _h)
{
	float result = l * _l + h * _h;
	return result / float(0x500);
}

template<int N>
glm::vec<N, uint8_t> WeightedSum(const glm::vec<N, uint8_t>& l, const glm::vec<N, uint8_t>& h, int _l, int _h)
{
	glm::vec<N, int> result = glm::vec<N, int>(l) * _l + glm::vec<N, int>(h) * _h;
	return glm::vec<N, uint8_t>(result / 0x500);
}

template<int N>
glm::vec<N, uint16_t> WeightedSum(const glm::vec<N, uint16_t>& l, const glm::vec<N, uint16_t>& h, int _l, int _h)
{
	glm::vec<N, uint16_t> result = l * _l + h * _h;
	return result / 0x500;
}

template<int N>
glm::vec<N, float> WeightedSum(const glm::vec<N, float>& l, const glm::vec<N, float>& h, int _l, int _h)
{
	glm::vec<N, float> result = l * float(_l) + h * float(_h);
	return result / float(0x500);
}


typedef int fint;


template<typename T>
inline void ResampleRow(T* __restrict dst, const T* __restrict src, fint _uk, int outSize)
{
	for (int i = 0; i < outSize; ++i)
	{
		fint fj = i * _uk;
		int jl = fj / 0x500;
		fint jlf = jl * 0x500;
		int jh = jl + ((fj > jlf + 10) & 1);
		fint wj = fj - jlf;
		fint _1_wj = 0x500 - wj;

		T l = src[jl];
		T h = src[jh];
		dst[i] = WeightedSum(l, h, _1_wj, wj);
	}

}

template<typename T>
void ResampleInternal(const Image& in, Image& out)
{
	PARALLEL_BEGIN(out.GetSize().y)
	{
		auto outSize = out.GetSize();
		auto inSize = in.GetSize();

		T * __restrict iR_dA = (T*)malloc(sizeof(T) * outSize.x);
		T * __restrict iR_dB = (T*)malloc(sizeof(T) * outSize.x);

		fint _ukx = (inSize.x - 1) * 0x500 / (outSize.x - 1);
		fint _uky = (inSize.y - 1) * 0x500 / (outSize.y - 1);

		ResampleRow(iR_dA, in.GetRow<T>(0), _ukx, outSize.x);
		ResampleRow(iR_dB, in.GetRow<T>(1), _ukx, outSize.x);

		int rL = 0;
		int rH = 1;

		T * __restrict iR_dL = iR_dA;
		T * __restrict iR_dH = iR_dB;

		for (int j = p_begin; j < p_end; ++j)
		{
			fint fi = j * _uky;
			int il = fi / 0x500;
			fint ilf = il * 0x500;
			int ih = il + ((fi > ilf + 10) & 1);
			fint wi = fi - ilf;
			fint _1_wi = 0x500 - wi;

			if (il != rL)
			{
				if (il == rH)
				{
					auto tmp = iR_dL;
					iR_dL = iR_dH;
					iR_dH = tmp;
					rL = rH;
				}
				else
				{
					ResampleRow(iR_dL, in.GetRow<T>(rL), _ukx, outSize.x);
					rL = il;
				}
			}
			if (ih != rH)
			{
				ResampleRow(iR_dH, in.GetRow<T>(rH), _ukx, outSize.x);
				rH = ih;
			}

			T* __restrict scanlineDst = out.GetRow<T>(j);

			for (int i = 0; i < outSize.x; ++i)
			{
				T l = iR_dL[i];
				T h = iR_dH[i];

				scanlineDst[i] = WeightedSum(l, h, _1_wi, wi);
			}
		}

		free(iR_dA);
		free(iR_dB);
	}
	PARALLEL_END();
}

inline uint32_t interpolate(fint k, const uint8_t* __restrict src)
{
	fint fj = k;
	int jl = fj / 0x500;
	fint jlf = jl * 0x500;
	int jh = jl + 1;
	fint wj = fj - jlf;
	fint _1_wj = 0x500 - wj;

	uint8_t l = src[jl];
	uint8_t h = src[jh];
	return l * _1_wj + h * wj;
}

inline void ResampleRow(uint32_t* __restrict dst, const uint8_t* __restrict src, fint _uk, int outSize)
{
	int m = outSize - 3;
	int i;
	for (i = 0; i < m; i += 4)
	{
		dst[i] = interpolate(i * _uk, src);
		dst[i + 1] = interpolate((i + 1) * _uk, src);
		dst[i + 2] = interpolate((i + 2) * _uk, src);
		dst[i + 3] = interpolate((i + 3) * _uk, src);
	}
	for (; i < outSize; i++)
		dst[i] = interpolate(i * _uk, src);
}

template<>
void ResampleInternal<uint8_t>(const Image& in, Image& out)
{
	{
		auto outSize = out.GetSize();
		auto inSize = in.GetSize();

		uint32_t * __restrict iR_dA = (uint32_t*)malloc(sizeof(uint32_t) * outSize.x);
		uint32_t * __restrict iR_dB = (uint32_t*)malloc(sizeof(uint32_t) * outSize.x);

		fint _ukx = (inSize.x - 1) * 0x500 / (outSize.x - 1);
		fint _uky = (inSize.y - 1) * 0x500 / (outSize.y - 1);

		ResampleRow(iR_dA, in.GetRow<uint8_t>(0), _ukx, outSize.x);
		ResampleRow(iR_dB, in.GetRow<uint8_t>(1), _ukx, outSize.x);

		int rL = 0;
		int rH = 1;

		uint32_t* __restrict iR_dL = iR_dA;
		uint32_t* __restrict iR_dH = iR_dB;

		for (int j = 0; j < out.GetSize().y; ++j)
		{
			fint fi = j * _uky;
			int il = fi / 0x500;
			fint ilf = il * 0x500;
			int ih = il + 1;
			fint wi = fi - ilf;
			fint _1_wi = 0x500 - wi;

			if (il != rL)
			{
				if (il == rH)
				{
					auto tmp = iR_dL;
					iR_dL = iR_dH;
					iR_dH = tmp;
					rL = rH;
				}
				else
				{
					ResampleRow(iR_dL, in.GetRow<uint8_t>(rL), _ukx, outSize.x);
					rL = il;
				}
			}
			uint32_t* __restrict iR_dH_f = iR_dH;
			if (ih == il)
			{
				iR_dH_f = iR_dL;
			}
			else if (ih != rH)
			{
				ResampleRow(iR_dH, in.GetRow<uint8_t>(rH), _ukx, outSize.x);
				rH = ih;
				iR_dH_f = iR_dH;
			}

			uint8_t* __restrict scanlineDst = out.GetRow<uint8_t>(j);

			/*
			int m = outSize.x - 3;
			int i;
			for (i = 0; i < m; i += 4)
			{
				scanlineDst[i] = ((iR_dL[i] * _1_wi + iR_dH_f[i] * wi) / (0x500 * 0x500) > 127) * 255;
				scanlineDst[i + 1] = ((iR_dL[i + 1] * _1_wi + iR_dH_f[i + 1] * wi) / (0x500 * 0x500) > 127) * 255;
				scanlineDst[i + 2] = ((iR_dL[i + 2] * _1_wi + iR_dH_f[i + 2] * wi) / (0x500 * 0x500) > 127) * 255;
				scanlineDst[i + 3] = ((iR_dL[i + 3] * _1_wi + iR_dH_f[i + 3] * wi) / (0x500 * 0x500) > 127) * 255;
			}
			for (; i < outSize.x; i++)
				scanlineDst[i] = ((iR_dL[i] * _1_wi + iR_dH_f[i] * wi) / (0x500 * 0x500) > 127) * 255;
				*/
			int m = outSize.x - 3;
			int i;
			for (i = 0; i < m; i += 4)
			{
				scanlineDst[i] = (iR_dL[i] * _1_wi + iR_dH_f[i] * wi) / (0x500 * 0x500);
				scanlineDst[i + 1] = (iR_dL[i + 1] * _1_wi + iR_dH_f[i + 1] * wi) / (0x500 * 0x500);
				scanlineDst[i + 2] = (iR_dL[i + 2] * _1_wi + iR_dH_f[i + 2] * wi) / (0x500 * 0x500);
				scanlineDst[i + 3] = (iR_dL[i + 3] * _1_wi + iR_dH_f[i + 3] * wi) / (0x500 * 0x500);
			}
			for (; i < outSize.x; i++)
				scanlineDst[i] = (iR_dL[i] * _1_wi + iR_dH_f[i] * wi) / (0x500 * 0x500);
		}

		free(iR_dA);
		free(iR_dB);
	}
}

Image Image::Resample(glm::ivec2 size, bool fast) const
{
	Image im = Empty(size, dataType);
	Image source;
	source = *this;

	if (dataType == R8)
	{
		if (fast)
		{
			ResampleInternal<pixelR8>(source, im);
		}
		else
		{
			stbir_resize_uint8(source.GetRow<unsigned char>(0), source.GetSize().x, source.GetSize().y, (int)source.GetRowSizeAligned(), im.GetRow<unsigned char>(0), size.x, size.y, (int)im.GetRowSizeAligned(), 1);
		}
	}
	else if (dataType == RF)
	{
		if (fast)
		{
			ResampleInternal<pixelRF>(source, im);
		}
		else
		{
			stbir_resize_float((float*)source._ptr.get(), source.size.x, source.size.y, (int)source.GetRowSizeAligned(), (float*)im._ptr.get(), im.size.x, im.size.y, (int)im.GetRowSizeAligned(), 1);
		}
	}
	else if (dataType == RGB8)
	{
		if (fast)
		{
			ResampleInternal<pixelRGB8>(source, im);
		}
		else
		{
			stbir_resize_uint8(source.GetRow<unsigned char>(0), source.GetSize().x, source.GetSize().y, (int)source.GetRowSizeAligned(), im.GetRow<unsigned char>(0), size.x, size.y, (int)im.GetRowSizeAligned(), 3);
		}
	}
	else if (dataType == RGBA8)
	{
		if (fast)
		{
			ResampleInternal<pixelRGBA8>(source, im);
		}
		else
		{
			stbir_resize_uint8(source.GetRow<unsigned char>(0), source.GetSize().x, source.GetSize().y, (int)source.GetRowSizeAligned(), im.GetRow<unsigned char>(0), size.x, size.y, (int)im.GetRowSizeAligned(), 4);
		}
	}
	else if (dataType == RGBF)
	{
		if (fast)
		{
			ResampleInternal<pixelRGBF>(source, im);
		}
		else
		{
			stbir_resize_float((float*)source._ptr.get(), source.size.x, source.size.y, (int)source.GetRowSizeAligned(), (float*)im._ptr.get(), im.size.x, im.size.y, (int)im.GetRowSizeAligned(), 3);
		}
	}
	else if (dataType == RGBAF)
	{
		if (fast)
		{
			ResampleInternal<pixelRGBAF>(source, im);
		}
		else
		{
			stbir_resize_float((float*)source._ptr.get(), source.size.x, source.size.y, (int)source.GetRowSizeAligned(), (float*)im._ptr.get(), im.size.x, im.size.y, (int)im.GetRowSizeAligned(), 4);
		}
	}

	return im;
}

Image Image::CarveVertical(const std::vector<short>& seam) const
{
	Image out = Image::Empty(glm::ivec2(GetSize().x - 1, GetSize().y), GetType());
	int bpp = (int)GetBPP();
	for (int j = 0; j < GetSize().y; ++j)
	{
		const char* src = GetRow<char>(j);
		char* dst = out.GetRow<char>(j);
		memcpy(dst, src, seam[j] * bpp);
		memcpy(dst + seam[j] * bpp, src + seam[j] * bpp + bpp, row_size - seam[j] * bpp - bpp);
	}
	return out;
}

void Image::CarveVerticalInplace(const std::vector<short>& seam)
{
	//Profile(CarveVerticalInplace);
	int bpp = (int)GetBPP();
	int targetRowSize = misc::align((size.x - 1) * bpp, Alignment);

	for (int j = 0; j < GetSize().y; ++j)
	{
		const char* src = GetRow<char>(j);
		char* dst = reinterpret_cast<char*>(_ptr.get() + targetRowSize * j);
		memmove(dst, src, seam[j] * bpp);
		memmove(dst + seam[j] * bpp, src + seam[j] * bpp + bpp, row_size - seam[j] * bpp - bpp);
	}
	--size.x;
}

Image Image::Copy() const
{
	Image im = Empty(size, dataType);
	// size_t bpp = GetBPP();

	for (int j = 0; j < im.size.y; ++j)
	{
		memcpy(
			im.GetRow<byte>(j),
			GetRow<byte>(j),
			im.row_size);
	}
	return im;
}

Image Image::Add(const Image& x) const
{
	if (dataType == R8)
	{
		return _Add<byte>(x);
	}
	else if (dataType == R16)
	{
		return _Add<uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Add<uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Add<pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Add<pixelRGB8>(x);
	}
	else if (dataType == RGBA8)
	{
		return _Add<pixelRGBA8>(x);
	}
	else if (dataType == RF)
	{
		return _Add<float>(x);
	}
	else if (dataType == RGBF)
	{
		return _Add<pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _Add<pixelRGBAF>(x);
	}
	assert(false);
	return Image();
}

Image Image::Sub(const Image& x) const
{
	if (dataType == R8)
	{
		return _Sub<byte>(x);
	}
	else if (dataType == R16)
	{
		return _Sub<uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Sub<uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Sub<pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Sub<pixelRGB8>(x);
	}
	else if (dataType == RGBA8)
	{
		return _Sub<pixelRGBA8>(x);
	}
	else if (dataType == RF)
	{
		return _Sub<float>(x);
	}
	else if (dataType == RGBF)
	{
		return _Sub<pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _Sub<pixelRGBAF>(x);
	}
	assert(false);
	return Image();
}

Image Image::Div(const Image& x) const
{
	if (dataType == R8)
	{
		return _Div<byte>(x);
	}
	else if (dataType == R16)
	{
		return _Div<uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Div<uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Div<pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Div<pixelRGB8>(x);
	}
	else if (dataType == RGBA8)
	{
		return _Div<pixelRGBA8>(x);
	}
	else if (dataType == RF)
	{
		return _Div<float>(x);
	}
	else if (dataType == RGBF)
	{
		return _Div<pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _Div<pixelRGBAF>(x);
	}
	assert(false);
	return Image();
}

Image Image::Mul(const Image& x) const
{
	if (dataType == R8)
	{
		return _Mul<byte>(x);
	}
	else if (dataType == R16)
	{
		return _Mul<uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Mul<uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Mul<pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Mul<pixelRGB8>(x);
	}
	else if (dataType == RGBA8)
	{
		return _Mul<pixelRGBA8>(x);
	}
	else if (dataType == RF)
	{
		return _Mul<float>(x);
	}
	else if (dataType == RGBF)
	{
		return _Mul<pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _Mul<pixelRGBAF>(x);
	}
	assert(false);
	return Image();
}

Image Image::Mul(float x) const
{
	if (dataType == RF)
	{
		return _Mul<float, float>(x);
	}
	else if (dataType == RGBF)
	{
		return _Mul<float, pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _Mul<float, pixelRGBAF>(x);
	}
	assert(false);
	return Image();
}

Image Image::Mul(int x) const
{
	if (dataType == R8)
	{
		return _Mul<byte, byte>((byte)x);
	}
	else if (dataType == R16)
	{
		return _Mul<uint16_t, uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Mul<uint32_t, uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Mul<int16_t, pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Mul<byte, pixelRGB8>((byte)x);
	}
	else if (dataType == RGBA8)
	{
		return _Mul<byte, pixelRGBA8>((byte)x);
	}

	assert(false);
	return Image();
}

Image Image::Div(int x) const
{
	if (dataType == R8)
	{
		return _Div<byte, byte>((byte)x);
	}
	else if (dataType == R16)
	{
		return _Div<uint16_t, uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Div<uint32_t, uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Div<int16_t, pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Div<byte, pixelRGB8>((byte)x);
	}
	else if (dataType == RGBA8)
	{
		return _Div<byte, pixelRGBA8>((byte)x);
	}

	assert(false);
	return Image();
}

Image Image::Add(float x) const
{
	if (dataType == RF)
	{
		return _Add<float, float>(x);
	}
	else if (dataType == RGBF)
	{
		return _Add<float, pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _Add<float, pixelRGBAF>(x);
	}
	return Image();
}

Image Image::Add(int x) const
{
	if (dataType == R8)
	{
		return _Add<byte, byte>((byte)x);
	}
	else if (dataType == R16)
	{
		return _Add<uint16_t, uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Add<uint32_t, uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _Add<int16_t, pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _Add<byte, pixelRGB8>((byte)x);
	}
	else if (dataType == RGBA8)
	{
		return _Add<byte, pixelRGBA8>((byte)x);
	}
	assert(false);
	return Image();
}

Image Image::Step(float x) const
{
	if (dataType == R8)
	{
		return _Step<uint8_t, uint8_t>(x * 127.0f);
	}
	else if (dataType == RF)
	{
		return _Step<float, float>(x);
	}
	else if (dataType == RGBF)
	{
		return _StepV<float, pixelRGBF>(x);
	}
	else if (dataType == RGBAF)
	{
		return _StepV<float, pixelRGBAF>(x);
	}
	assert(false);
	return Image();
}

Image Image::Step(int x) const
{
	if (dataType == R8)
	{
		return _Step<byte, byte>((byte)x);
	}
	else if (dataType == R16)
	{
		return _Step<uint16_t, uint16_t>(x);
	}
	else if (dataType == R32)
	{
		return _Step<uint32_t, uint32_t>(x);
	}
	else if (dataType == RG16)
	{
		return _StepV<int16_t, pixelRG16>(x);
	}
	else if (dataType == RGB8)
	{
		return _StepV<byte, pixelRGB8>((byte)x);
	}
	else if (dataType == RGBA8)
	{
		return _StepV<byte, pixelRGBA8>((byte)x);
	}
	assert(false);
	return Image();
}

int Image::rows() const
{
	return GetSize().y;
}

int Image::cols() const
{
	return GetSize().x;
}


/*
@article{grevera2004dead,
title={The �dead reckoning� signed distance transform},
author={Grevera, George J},
journal={Computer Vision and Image Understanding},
volume={95},
number={3},
pages={317--333},
year={2004},
publisher={Elsevier}

Algorithm: Chamfer
Inputs: I - a 2D binary image of size X by Y
       d1 - distance between two adjacent pixels in either the x or y direction 
       d2 - diagonal distance between two diagonally adjacent pixels

Output: d - a 2D grey image of size X and Y representing the distance image

//initialize d
for y = 1 to Y do begin
   for x = 1 to X do begin 
       d(x,y) = inf
   end
end

//initialize immediate interior & exterior elements
for y = 1 to Y do begin
   for x = 1 to X do begin
       //if ( I(x,y) ) then //uncomment to disable symmetry under complement property 
       if ( I(x-1,y) != I(x,y) or I(x+1,y) != I(x,y) or
            I(x,y-1) != I(x,y) or I(x,y+1) != I(x,Y) ) then d(x,y) = 0
   end
end

//perform the first (forward) pass 
for y = 1 to Y do begin
   for x = 1 to X do begin
       if 61(x-1,y-1)+d2 < d(x,y)) then   d(x,y) = d(x-1,y-1)+d2 
       if 61(x,y-1) +dl < d(x,y)) then    d(x,y) = d(x,y-1) +dl 
       if 61(x+1,y-1)+d2 < d(x,y)) then   d(x,y) = d(x+1,y-1)+d2 
       if 61(x-1,y) +dl < d(x,y)) then    d(x,y) = d(x-1,y) +dl
   end
end

//perform the final (backward) pass 
for y = Y to 1 do begin
   for x = X to 1 do begin
       if 61(x+1,y) +d1 < d (x, y)) then d(x,y) = d(x+1,y) +dl 
       if (d(x-1,y+1)+d2 < d(x,y)) then  d(x,y) = d(x-1,y+1)+d2 
       if 61(x,y+1) +4:11 < d(x,y)) then d(x,y) = d(x,y+1) +dl 
       if 61(x+1,y+1)+d2 < d(x,y)) then  d(x,y) = d(x+1,y+1)+d2
   end
end

//indicate inside & outside 
for y = Y to 1 do begin
   for x = X to 1 do begin
       if (I(x,y) == 0) then   d(x,y) = -d(x,y)
   end
end
}*/
Image Image::ComputeDF(DFType type) const
{
	Image d = Image::Empty(GetSize(), RF);
	Image p = Image::Empty(GetSize(), RG16);
	Image binary = Image::Empty(GetSize(), R8);

	int width = d.GetSize().x;
	for (int j = 1; j < GetSize().y - 1; ++j)
	{
		byte* b_ptr = binary.GetRow<byte>(j);
		const pixelRGB8* ptr = GetRow<pixelRGB8>(j);

		for (int i = 1; i < width - 1; ++i)
		{
			int av = ptr[i].x + ptr[i].y + ptr[i].z;
			if (av > 128 * 3)
			{
				b_ptr[i] = (byte)255;
			}
			else
			{
				b_ptr[i] = (byte)0;
			}
		}
	}

	constexpr float d1 = 1.f;
	constexpr float d2 = 1.41421356237309504880f;
	constexpr float d3 = 2.23606797749978969641f;

	//initialize immediate interior & exterior elements
	for (int j = 1; j < GetSize().y-1; ++j)
	{
		float* d_ptr = d.GetRow<float>(j);
		pixelRG16* p_ptr = p.GetRow<pixelRG16>(j);
		const byte* ptr = binary.GetRow<byte>(j);
		const byte* ptr_yp = binary.GetRow<byte>(j+1);
		const byte* ptr_ym = binary.GetRow<byte>(j - 1);

		for (int i = 1; i < width -1; ++i)
		{
			if (ptr[i - 1] != ptr[i] || ptr[i + 1] != ptr[i] || ptr_ym[i] != ptr[i] || ptr_yp[i] != ptr[i])
			{
				d_ptr[i] = 0.0f;
				p_ptr[i] = pixelRG16(i, j);
			}
			else
			{
				d_ptr[i] = 1e8f;
				p_ptr[i] = pixelRG16(-100);
			}	
		}
	}

	if (type == Chamfer3x3)
	{
		//perform the first (forward) pass 
		for (int j = 1; j < GetSize().y; ++j)
		{
			float* d_ptr = d.GetRow<float>(j);
			float* d_ptr_ym = d.GetRow<float>(j - 1);
			for (int i = 1; i < width - 1; ++i)
			{
				float& d_xy = d_ptr[i];
				if (d_ptr_ym[i - 1] + d2 < d_xy)  d_xy = d_ptr_ym[i - 1] + d2;
				if (d_ptr_ym[i] + d1 < d_xy)  d_xy = d_ptr_ym[i] + d1;
				if (d_ptr_ym[i + 1] + d2 < d_xy)  d_xy = d_ptr_ym[i + 1] + d2;
				if (d_ptr[i - 1] + d1 < d_xy)  d_xy = d_ptr[i - 1] + d1;
			}
		}

		//perform the final (backward) pass 
		for (int j = GetSize().y - 2; j >= 0; --j)
		{
			float* d_ptr = d.GetRow<float>(j);
			float* d_ptr_yp = d.GetRow<float>(j + 1);
			for (int i = width - 2; i > 0; --i)
			{
				float& d_xy = d_ptr[i];
				if (d_ptr[i + 1] + d1 < d_xy)  d_xy = d_ptr[i + 1] + d1;
				if (d_ptr_yp[i - 1] + d2 < d_xy)  d_xy = d_ptr_yp[i - 1] + d2;
				if (d_ptr_yp[i] + d1 < d_xy)  d_xy = d_ptr_yp[i] + d1;
				if (d_ptr_yp[i + 1] + d2 < d_xy)  d_xy = d_ptr_yp[i + 1] + d2;
			}
		}
	}
	else if (type == DeadReckoning3x3)
	{
		//perform the first (forward) pass 
		for (int j = 1; j < GetSize().y; ++j)
		{
			float* d_ptr = d.GetRow<float>(j);
			float* d_ptr_ym = d.GetRow<float>(j - 1);
			pixelRG16* p_ptr = p.GetRow<pixelRG16>(j);
			// pixelRG16* p_ptr_yp = p.GetRow<pixelRG16>(j + 1);
			pixelRG16* p_ptr_ym = p.GetRow<pixelRG16>(j - 1);
			for (int i = 1; i < width - 1; ++i)
			{
				float& d_xy = d_ptr[i];

				if (d_ptr_ym[i - 1] + d2 < d_xy) 
				{
					p_ptr[i] = p_ptr_ym[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym[i] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym[i + 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr[i - 1] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
			}
		}

		//perform the final (backward) pass 
		for (int j = GetSize().y - 2; j >= 0; --j)
		{
			float* d_ptr = d.GetRow<float>(j);
			float* d_ptr_yp = d.GetRow<float>(j + 1);
			pixelRG16* p_ptr = p.GetRow<pixelRG16>(j);
			pixelRG16* p_ptr_yp = p.GetRow<pixelRG16>(j + 1);
			// pixelRG16* p_ptr_ym = p.GetRow<pixelRG16>(j - 1);
			for (int i = width - 2; i > 0; --i)
			{
				float& d_xy = d_ptr[i];

				if (d_ptr[i + 1] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i - 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i + 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
			}
		}
	}
	else if (type == DeadReckoning5x5)
	{
		//perform the first (forward) pass 
		for (int j = 2; j < GetSize().y; ++j)
		{
			float* d_ptr = d.GetRow<float>(j);
			float* d_ptr_ym = d.GetRow<float>(j - 1);
			float* d_ptr_ym_ym = d.GetRow<float>(j - 2);
			pixelRG16* p_ptr = p.GetRow<pixelRG16>(j);
			// pixelRG16* p_ptr_yp = p.GetRow<pixelRG16>(j + 1);
			pixelRG16* p_ptr_ym = p.GetRow<pixelRG16>(j - 1);
			pixelRG16* p_ptr_ym_ym = p.GetRow<pixelRG16>(j - 2);
			for (int i = 2; i < width - 2; ++i)
			{
				float& d_xy = d_ptr[i];

				if (d_ptr_ym[i - 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym[i] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym[i + 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr[i - 1] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym[i - 2] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i - 2];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym[i + 2] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_ym[i + 2];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym_ym[i - 1] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_ym_ym[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_ym_ym[i + 1] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_ym_ym[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
			}
		}

		//perform the final (backward) pass 
		for (int j = GetSize().y - 3; j >= 0; --j)
		{
			float* d_ptr = d.GetRow<float>(j);
			float* d_ptr_yp = d.GetRow<float>(j + 1);
			float* d_ptr_yp_yp = d.GetRow<float>(j + 2);
			pixelRG16* p_ptr = p.GetRow<pixelRG16>(j);
			pixelRG16* p_ptr_yp = p.GetRow<pixelRG16>(j + 1);
			pixelRG16* p_ptr_yp_yp = p.GetRow<pixelRG16>(j + 2);
			// pixelRG16* p_ptr_ym = p.GetRow<pixelRG16>(j - 1);
			for (int i = width - 3; i > 1; --i)
			{
				float& d_xy = d_ptr[i];

				if (d_ptr[i + 1] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i - 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i] + d1 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i + 1] + d2 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i + 2] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i + 2];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp[i - 2] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_yp[i - 2];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp_yp[i + 1] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_yp_yp[i + 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
				if (d_ptr_yp_yp[i - 1] + d3 < d_xy)
				{
					p_ptr[i] = p_ptr_yp_yp[i - 1];
					d_xy = sqrt(float(sqr(i - p_ptr[i].x) + sqr(j - p_ptr[i].y)));
				}
			}
		}
	}

	//indicate inside & outside 
	for (int j = 0; j < GetSize().y; ++j)
	{
		float* d_ptr = d.GetRow<float>(j);
		const byte* ptr = binary.GetRow<byte>(j);
		for (int i = 0; i < width - 0; ++i)
		{
			if (ptr[i] == (byte)0)
			{
				d_ptr[i] = -d_ptr[i];
			}
		}
	}

	return d;
}

Image Image::GaussBlur(float r) const
{
	return GaussBlurX(r).Transpose().GaussBlurX(r).Transpose();
}

template<typename T, typename I>
Image Image::_GaussBlurX(float r) const
{
	int half = int(r);
	std::vector<float> kv = misc::GaussKernel(r / 3.0f, half * 2 + 1);
	const float* __restrict kernel = &kv[0];
	// const int s = (int)kv.size();
	Image out = Image::Empty(*this);

	int width = GetSize().x;
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* __restrict src = GetRow<T>(j);
		T* __restrict dst = out.GetRow<T>(j);
		for (int i = 0; i < half; ++i)
		{
			I s = (I)src[i] * kernel[half];
			for (int k = 1; k <= half; ++k)
			{
				s += (I)src[i + k] * kernel[k + half] + (I)src[abs(i - k)] * kernel[k + half];
			}
			dst[i] = (T)s;
		}
		for (int i = half; i < width - half; ++i)
		{
			I s = (I)src[i] * kernel[half];
			for (int k = 1; k <= half; ++k)
			{
				s += (I)src[i + k] * kernel[k + half] + (I)src[i - k] * kernel[k + half];
			}
			dst[i] = (T)s;
		}
		for (int i = width - half; i < width; ++i)
		{
			I s = (I)src[i] * kernel[half];
			for (int k = 1; k <= half; ++k)
			{
				s += (I)src[width - 1 - abs(width - 1 - i - k)] * kernel[k + half] + (I)src[i - k] * kernel[k + half];
			}
			dst[i] = (T)s;
		}
	}
	return out;
}

Image Image::GaussBlurX(float r) const
{
	if (dataType == R8)
	{
		return _GaussBlurX<byte, float>(r);
	}
	else if (dataType == R16)
	{
		return _GaussBlurX<uint16_t, float>(r);
	}
	else if (dataType == R32)
	{
		return _GaussBlurX<uint32_t, float>(r);
	}
	else if (dataType == RG16)
	{
		return _GaussBlurX<pixelRG16, pixelRGF>(r);
	}
	else if (dataType == RGB8)
	{
		return _GaussBlurX<pixelRGB8, pixelRGBF>(r);
	}
	else if (dataType == RGBA8)
	{
		return _GaussBlurX<pixelRGBA8, pixelRGBAF>(r);
	}
	else if (dataType == RF)
	{
		return _GaussBlurX<float, float>(r);
	}
	else if (dataType == RGF)
	{
		return _GaussBlurX<pixelRGF, pixelRGF>(r);
	}
	else if (dataType == RGBF)
	{
		return _GaussBlurX<pixelRGBF, pixelRGBF>(r);
	}
	else if (dataType == RGBAF)
	{
		return _GaussBlurX<pixelRGBAF, pixelRGBAF>(r);
	}
	assert(false);
	return Image();
}

Image Image::ConnectedComponents(int* count) const
{
	Image L = Image::Empty(GetSize(), R32);

	assert(dataType == Image::R8);

	int cc = cv::connectedComponents(*this, L, 8);

	if (count != nullptr)
	{
		*count = cc;
	}

	return L;
}

template<typename T>
inline Image Image::ResizeX(Image seamMap, int reduction, bool fixCollisions) const
{
	Image out = Image::Empty(glm::ivec2(GetSize().x - reduction, GetSize().y), GetType());

	byte* counter = new byte[GetSize().x];

	int width = out.GetSize().x;
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* src = GetRow<T>(j);
		const short* srcIndex = seamMap.GetRow<short>(j);
		T* dst = out.GetRow<T>(j);
		int reduction_corrected = reduction;

		if (fixCollisions)
		{
			memset(counter, 0, GetSize().x);

			for (int i = 0; i < GetSize().x; ++i)
			{
				counter[srcIndex[i] % GetSize().x]++;
			}
			int red = 0;
			int abs_red = abs(reduction);
			for (int i = 0; i < GetSize().x; ++i)
			{
				red += counter[i];
				if (red > abs_red)
				{
					if (reduction < 0)
					{
						reduction_corrected = -i + 1;
					}
					else
					{
						reduction_corrected = i - 1;
					}
					break;
				}
			}
			memset(counter, 0, GetSize().x);
		}

		if (reduction_corrected > 0)
		{
			int ii = 0;
			for (int i = 0; i < GetSize().x; ++i)
			{
				if (((srcIndex[i] >= reduction_corrected)) && ii < width)
				{
					dst[ii] = src[i];
					++ii;
				}
			}
		}
		else
		{
			int ii = 0;
			for (int i = 0; i < GetSize().x; ++i)
			{
				if (ii < width)
				{
					dst[ii] = src[i];
					++ii;
					if ((srcIndex[i] < -reduction_corrected) && ii < width)
					{
						dst[ii] = src[i];
						++ii;
					}
				}
			}
		}
	}
	delete[] counter;
	return out;
}

template<typename T>
inline Image Image::ResizeX(const std::vector<uint16_t>& col, int reduction) const
{
	Image out = Image::Empty(glm::ivec2(GetSize().x - reduction, GetSize().y), GetType());

	int width = out.GetSize().x;
	for (int j = 0; j < GetSize().y; ++j)
	{
		const T* src = GetRow<T>(j);
		T* dst = out.GetRow<T>(j);

		if (reduction > 0)
		{
			int ii = 0;
			for (int i = 0; i < GetSize().x; ++i)
			{
				if (col[i] >= reduction && ii < width)
				{
					dst[ii] = src[i];
					++ii;
				}
			}
		}
		else
		{
			int ii = 0;
			for (int i = 0; i < GetSize().x; ++i)
			{
				if (ii < width)
				{
					dst[ii] = src[i];
					++ii;
					if (col[i] < -reduction && ii < width)
					{
						dst[ii] = src[i];
						++ii;
					}
				}
			}
		}
	}
	return out;
}

template<typename T>
inline Image Image::ResizeY(Image seamMap, int reduction, bool fixCollisions) const
{
	byte* counter = new byte[GetSize().y];

	Image out = Image::Empty(glm::ivec2(GetSize().x, GetSize().y - reduction), GetType());

	if (reduction > 0)
	{
		int height = out.GetSize().y;
		for (int i = 0; i < GetSize().x; ++i)
		{
			int reduction_corrected = reduction;

			if (fixCollisions)
			{
				memset(counter, 0, GetSize().y);

				for (int j = 0; j < GetSize().y; ++j)
				{
					counter[seamMap.GetRow<short>(j)[i] % GetSize().y]++;
				}
				int red = 0;
				int abs_red = abs(reduction);
				for (int j = 0; j < GetSize().y; ++j)
				{
					red += counter[j];
					if (red > abs_red)
					{
						if (reduction < 0)
						{
							reduction_corrected = -j + 1;
						}
						else
						{
							reduction_corrected = j - 1;
						}
						break;
					}
				}
			}

			int jj = 0;
			for (int j = 0; j < GetSize().y; ++j)
			{
				const T* src = GetRow<T>(j);
				const short* srcIndex = seamMap.GetRow<short>(j);
				T* dst = out.GetRow<T>(jj % height);

				if (srcIndex[i] >= reduction_corrected && jj < height)
				{
					dst[i] = src[i];
					++jj;
				}
			}
		}
	}
	else
	{
		int height = out.GetSize().y;
		for (int i = 0; i < GetSize().x; ++i)
		{
			int reduction_corrected = reduction;

			if (fixCollisions)
			{
				memset(counter, 0, GetSize().y);

				for (int j = 0; j < GetSize().y; ++j)
				{
					counter[seamMap.GetRow<short>(j)[i] % GetSize().y]++;
				}
				int red = 0;
				int abs_red = abs(reduction);
				for (int j = 0; j < GetSize().y; ++j)
				{
					red += counter[j];
					if (red > abs_red)
					{
						if (reduction < 0)
						{
							reduction_corrected = -j + 1;
						}
						else
						{
							reduction_corrected = j - 1;
						}
						break;
					}
				}
			}

			int jj = 0;
			for (int j = 0; j < GetSize().y; ++j)
			{
				const T* src = GetRow<T>(j);
				const short* srcIndex = seamMap.GetRow<short>(j);
				T* dst = out.GetRow<T>(jj % height);

				if (jj < height)
				{
					dst[i] = src[i];
					++jj;
					if (srcIndex[i] < -reduction_corrected && jj < height)
					{
						dst = out.GetRow<T>(jj % height);
						dst[i] = src[i];
						++jj;
					}
				}
			}
		}
	}
	delete[] counter;
	return out;
}

template<typename T>
inline Image Image::ResizeY(const std::vector<uint16_t>& rows, int reduction) const
{
	Image out = Image::Empty(glm::ivec2(GetSize().x, GetSize().y - reduction), GetType());

	if (reduction > 0)
	{
		int height = out.GetSize().y;
		for (int i = 0; i < GetSize().x; ++i)
		{
			int jj = 0;
			for (int j = 0; j < GetSize().y; ++j)
			{
				const T* src = GetRow<T>(j);
				T* dst = out.GetRow<T>(jj % height);

				if (rows[j] >= reduction)
				{
					dst[i] = src[i];
					++jj;
				}
			}
		}
	}
	else
	{
		int height = out.GetSize().y;
		for (int i = 0; i < GetSize().x; ++i)
		{
			int jj = 0;
			for (int j = 0; j < GetSize().y; ++j)
			{
				const T* src = GetRow<T>(j);
				T* dst = out.GetRow<T>(jj % height);
				dst[i] = src[i];
				++jj;
				if (rows[j] < -reduction)
				{
					dst = out.GetRow<T>(jj % height);
					dst[i] = src[i];
					++jj;
				}
			}
		}
	}
	return out;
}

template<typename T>
inline Image Image::ResizeNaive(Image seamMapX, Image seamMapY, glm::ivec2 newSize) const
{
	int reductionx = GetSize().x - newSize.x;
	int reductiony = GetSize().y - newSize.y;

	if (abs(reductionx) > abs(reductiony))
	{
		Image seammapY = seamMapY.ResizeY<uint16_t>(seamMapX, reductiony);
		return ResizeY<T>(seamMapX, reductiony).template ResizeX<T>(seammapY, reductionx);
	}
	else
	{
		Image seammapX = seamMapX.ResizeX<uint16_t>(seamMapY, reductionx);
		return ResizeX<T>(seamMapY, reductionx).template ResizeY<T>(seammapX, reductiony);
	}
}

template<typename T>
inline Image Image::ResizeCollisionFix(Image seamMapX, Image seamMapY, glm::ivec2 newSize) const
{
	int reductionx = GetSize().x - newSize.x;
	int reductiony = GetSize().y - newSize.y;

	if (abs(reductionx) > abs(reductiony))
	{
		Image seammapY = seamMapY.ResizeY<uint16_t>(seamMapX, reductiony);
		return ResizeY<T>(seamMapX, reductiony).template ResizeX<T>(seammapY, reductionx, true);
	}
	else
	{
		Image seammapX = seamMapX.ResizeX<uint16_t>(seamMapY, reductionx);
		return ResizeX<T>(seamMapY, reductionx).template ResizeY<T>(seammapX, reductiony, true);
	}
}

template<typename T>
inline Image Image::ResizeSimple(Image seamMapX, Image seamMapY, glm::ivec2 newSize) const
{
	int reductionx = GetSize().x - newSize.x;
	int reductiony = GetSize().y - newSize.y;

	if (abs(reductionx) > abs(reductiony))
	{
		return ResizeX<T>(seamMapY, reductionx);
	}
	else
	{
		return ResizeY<T>(seamMapX, reductiony);
	}
}

template<typename T>
inline Image Image::ResizeSemi(Image seamMapX, Image seamMapY, const std::vector<uint16_t>& col, const std::vector<uint16_t>& rows, glm::ivec2 newSize) const
{
	int reductionx = GetSize().x - newSize.x;
	int reductiony = GetSize().y - newSize.y;

	if (abs(reductionx) > abs(reductiony))
	{
		return ResizeX<T>(seamMapY, reductionx).template ResizeY<T>(rows, reductiony);
	}
	else
	{
		return ResizeY<T>(seamMapX, reductiony).template ResizeX<T>(col, reductionx);
	}
}

template<> Image Image::Empty<Image::byte>(glm::ivec2 size) { return Empty(size, R8); }
template<> Image Image::Empty<Image::pixelR16>(glm::ivec2 size) { return Empty(size, R16); }
template<> Image Image::Empty<Image::pixelR32>(glm::ivec2 size) { return Empty(size, R32); }
template<> Image Image::Empty<Image::pixelRG16>(glm::ivec2 size) { return Empty(size, RG16); }
template<> Image Image::Empty<Image::pixelRGB8>(glm::ivec2 size) { return Empty(size, RGB8); }
template<> Image Image::Empty<Image::pixelRGBA8>(glm::ivec2 size) { return Empty(size, RGBA8); }
template<> Image Image::Empty<Image::pixelRF>(glm::ivec2 size) { return Empty(size, RF); }
template<> Image Image::Empty<Image::pixelRGF>(glm::ivec2 size) { return Empty(size, RGF); }
template<> Image Image::Empty<Image::pixelRGBF>(glm::ivec2 size) { return Empty(size, RGBF); }
template<> Image Image::Empty<Image::pixelRGBAF>(glm::ivec2 size) { return Empty(size, RGBAF); }

template Image Image::ResizeSemi<Image::pixelR8>(Image, Image, const std::vector<uint16_t>&, const std::vector<uint16_t>&, glm::ivec2) const;
template Image Image::ResizeSemi<Image::pixelR16>(Image, Image, const std::vector<uint16_t>&, const std::vector<uint16_t>&, glm::ivec2) const;
template Image Image::ResizeSemi<Image::pixelRF>(Image, Image, const std::vector<uint16_t>&, const std::vector<uint16_t>&, glm::ivec2) const;
template Image Image::ResizeSemi<Image::pixelRGB8>(Image, Image, const std::vector<uint16_t>&, const std::vector<uint16_t>&, glm::ivec2) const;
template Image Image::ResizeSemi<Image::pixelRGBF>(Image, Image, const std::vector<uint16_t>&, const std::vector<uint16_t>&, glm::ivec2) const;

template Image Image::ResizeSimple<Image::pixelR8>(Image, Image, glm::ivec2) const;
template Image Image::ResizeSimple<Image::pixelR16>(Image, Image, glm::ivec2) const;
template Image Image::ResizeSimple<Image::pixelRF>(Image, Image, glm::ivec2) const;
template Image Image::ResizeSimple<Image::pixelRGB8>(Image, Image, glm::ivec2) const;
template Image Image::ResizeSimple<Image::pixelRGBF>(Image, Image, glm::ivec2) const;

template Image Image::ResizeNaive<Image::pixelR8>(Image, Image, glm::ivec2) const;
template Image Image::ResizeNaive<Image::pixelR16>(Image, Image, glm::ivec2) const;
template Image Image::ResizeNaive<Image::pixelRF>(Image, Image, glm::ivec2) const;
template Image Image::ResizeNaive<Image::pixelRGB8>(Image, Image, glm::ivec2) const;
template Image Image::ResizeNaive<Image::pixelRGBF>(Image, Image, glm::ivec2) const;

template Image Image::ResizeCollisionFix<Image::pixelR8>(Image, Image, glm::ivec2) const;
template Image Image::ResizeCollisionFix<Image::pixelR16>(Image, Image, glm::ivec2) const;
template Image Image::ResizeCollisionFix<Image::pixelRF>(Image, Image, glm::ivec2) const;
template Image Image::ResizeCollisionFix<Image::pixelRGB8>(Image, Image, glm::ivec2) const;
template Image Image::ResizeCollisionFix<Image::pixelRGBF>(Image, Image, glm::ivec2) const;


void Image::SaveToTGA(const char * filename)
{
	FILE* file = fopen(filename, "wb");
	char buff[18];
	int headerSize = sizeof(buff);
	memset(buff, 0, headerSize);
	buff[2] = 2;
	int height = GetSize().y;
	int width = GetSize().x;

	buff[0xc] = width % 256;
	buff[0xd] = width / 256;
	buff[0xe] = height % 256;
	buff[0xf] = height / 256;
	buff[0x10] = 24;
	fwrite(buff, headerSize, 1, file);

	int channelCount = GetChannelCount();

	uint8_t* row = new uint8_t[GetSize().x * 3];

	for (int j = size.y - 1; j >= 0; --j)
	{
		uint8_t* p = GetRow<uint8_t>(j);
		for (int j = 0; j < GetSize().x; ++j)
		{
			uint8_t rgb[3] = { 0, 0, 0 };

			for (int k = 0; k < channelCount; ++k)
			{
				rgb[k] = *(p + j * channelCount + k);
			}

			row[j * 3 + 0] = rgb[2];
			row[j * 3 + 1] = rgb[1];
			row[j * 3 + 2] = rgb[0];
		}
		fwrite(row, GetSize().x * 3, 1, file);
	}

	delete[] row;
	fclose(file);
}
