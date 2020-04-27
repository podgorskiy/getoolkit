#include "TextureFormat.h"
#include <spdlog/spdlog.h>
#include <GL/gl3w.h>

using namespace Render;


uint64_t Render::ParsePixelType(const char* s)
{
	char buff[255];
	strncpy(buff, s, 255);

	int i = 0;
	while (buff[i] && i < 255)
	{
		buff[i] = (tolower(buff[i]));
		i++;
	}

	uint64_t type = 0;
	uint64_t mul = 1;
	for (i = 0; i < 4; i++)
	{
		if (buff[i] == 'r' || buff[i] == 'g' || buff[i] == 'b' || buff[i] == 'a' || buff[i] == 'l' ||
		    buff[i] == 'd')
		{
			type += buff[i] * mul;
			mul = mul << 8u;
		}
		else
		{
			break;
		}
	}
	if (i == 0)
	{
		throw runtime_error("Format should start from channel names, such as: r, g, b, a, l, d. Got %c", buff[0]);
	}
	for (int _i = i; _i < 4; _i++)
	{
		mul = mul << 8u;
	}

	if (!(buff[i] >= '0' && buff[i] <= '9'))
	{
		throw runtime_error("Unexpected symbol after channel names: %c. Expected digits", buff[i]);
	}

	int j = 0;
	const char* p = buff + i;
	for (; j < 4; j++)
	{
		if (*p >= '0' && *p <= '9')
		{
			int v = *p - '0';
			if (*p == '1' && *(p + 1) == '0')
			{
				v = 10;
				++p;
			}
			if (*p == '1' && *(p + 1) == '1')
			{
				v = 11;
				++p;
			}
			if (*p == '1' && *(p + 1) == '2')
			{
				v = 12;
				++p;
			}
			if (*p == '1' && *(p + 1) == '6')
			{
				v = 16;
				++p;
			}
			if (*p == '3' && *(p + 1) == '2')
			{
				v = 32;
				++p;
			}
			++p;
			type += v * mul;
			mul = mul << 8u;
		}
		else
		{
			break;
		}
	}

	if (*p != '\0')
	{
		throw runtime_error("Unexpected symbol at the end of the format: %c", *p);
	}

	if (i != j)
	{
		throw runtime_error("Number of channel names and sizes do not match, got %d and %d", i, j);
	}
	return type;
}


DecodedType Render::DecodePixelType(uint64_t pixel_format)
{
	bool compressed = (pixel_format & 0xFFFFFFFF00000000) == 0;
	if (compressed)
	{
		return {compressed, {}, {}};
	}
	std::vector<char> channels;
	std::vector<uint8_t> sizes;
	for (int i = 0; i < 4; ++i)
	{
		char v = pixel_format & 0xffu;
		if (v != 0)
		{
			channels.push_back(v);
		}
		pixel_format = pixel_format >> 8u;
	}
	for (int i = 0; i < 4; ++i)
	{
		uint8_t v = pixel_format & 0xffu;
		if (v != 0)
		{
			sizes.push_back(v);
		}
		pixel_format = pixel_format >> 8u;
	}

	return {false, channels, sizes};
}


size_t TextureFormat::GetBitsPerPixel(Format f)
{
	switch (f)
	{
		case R8:
		case A8:
		case L8:
			return 8;

		case R16:
		case RG88:
		case LA88:
		case RGB565:
		case RGBA4444:
		case RGBA5551:
			return 16;

		case RGB888:
			return 24;

		case RG1616:
		case RGBA8888:
		case RGBA1010102:
		case BGRA8888:
		case BGR101111:
			return 32;

		case RGB161616:
			return 48;

		case RGBA16161616:
		case RG3232:
			return 64;

		case RGB323232:
			return 96;

		case RGBA32323232:
			return 128;

		case PVRTCI_2bpp_RGB:
		case PVRTCI_2bpp_RGBA:
		case PVRTCII_2bpp:
			return 2;

		case PVRTCI_4bpp_RGB:
		case PVRTCI_4bpp_RGBA:
		case PVRTCII_4bpp:
		case ETC1:
		case EAC_R11:
		case ETC2_RGB:
		case ETC2_RGB_A1:
		case DXT1:
			return 4;

		case DXT2:
		case DXT3:
		case DXT4:
		case DXT5:
		case EAC_RG11:
		case ETC2_RGBA:
			return 8;
		case RGBG8888:
		case GRGB8888:
			return 16;
	}
	spdlog::error("Unsupported texture format {}", f);
	return 0;
}


glm::ivec2 TextureFormat::GetMinBlockSize(Format f)
{
	if (uint64_t(f) >> 32u)
	{
		return glm::ivec2(1);
	}
	else
	{
		switch (f)
		{
			case DXT1:
			case DXT2:
			case DXT3:
			case DXT4:
			case DXT5:
			case ETC1:
			case ETC2_RGB:
			case ETC2_RGB_A1:
			case ETC2_RGBA:
			case EAC_R11:
			case EAC_RG11:
				return glm::ivec2(4);

			case PVRTCI_4bpp_RGB:
			case PVRTCI_4bpp_RGBA:
				return glm::ivec2(8);

			case PVRTCI_2bpp_RGB:
			case PVRTCI_2bpp_RGBA:
				return glm::ivec2(16, 8);

			case PVRTCII_4bpp:
				return glm::ivec2(4);

			case PVRTCII_2bpp:
				return glm::ivec2(8, 4);

			case RGBG8888:
			case GRGB8888:
				return glm::ivec2(2, 1);

#define _ASTC(W, H) case ASTC##_##W##x##H: return glm::ivec2(W, H);

			_ASTC(4, 4)
			_ASTC(5, 4)
			_ASTC(5, 5)
			_ASTC(6, 5)
			_ASTC(6, 6)
			_ASTC(8, 5)
			_ASTC(8, 6)
			_ASTC(8, 8)
			_ASTC(10, 5)
			_ASTC(10, 6)
			_ASTC(10, 8)
			_ASTC(10, 10)
			_ASTC(12, 10)
			_ASTC(12, 12)
#undef _ASTC
			default:
				spdlog::error("Unsupported texture format {}", f);
				return glm::ivec2(1);
		}
	}
}

std::string GetStringRepresentation(TextureFormat format)
{
	auto d = DecodePixelType(format.pixel_format);
	char buff [64];
	if (d.compressed)
	{
		sprintf(buff, "Compressed 0x%x", (int)format.pixel_format);
		return buff;
	}

	std::string str(d.channel_names.begin(), d.channel_names.end());

	for (auto s: d.channel_sizes)
	{
		sprintf(buff, "%d", s);
		str += buff;
	}
	return buff;
}

std::vector<uint32_t> Render::GetGLMappedTypes(TextureFormat format)
{
	uint32_t internal_format = 0;
	uint32_t import_format = 0;
	// OpenGL 4    GL_RED, GL_RG, GL_RGB, GL_BGR, GL_RGBA, GL_BGRA, GL_RED_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_BGR_INTEGER, GL_RGBA_INTEGER, GL_BGRA_INTEGER, GL_STENCIL_INDEX, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
	// OpenGL ES3  GL_RED, GL_RED_INTEGER, GL_RG, GL_RG_INTEGER, GL_RGB, GL_RGB_INTEGER, GL_RGBA, GL_RGBA_INTEGER, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL, GL_LUMINANCE_ALPHA, GL_LUMINANCE, and GL_ALPHA.
	uint32_t channel_type = 0;
	// OpenGL 4    GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_HALF_FLOAT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2, and GL_UNSIGNED_INT_2_10_10_10_REV.
	// OpenGL ES3  GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_UNSIGNED_INT, GL_INT, GL_HALF_FLOAT, GL_FLOAT, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1, GL_UNSIGNED_INT_2_10_10_10_REV, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_UNSIGNED_INT_5_9_9_9_REV, GL_UNSIGNED_INT_24_8, and GL_FLOAT_32_UNSIGNED_INT_24_8_REV.

	bool _signed = Render::TextureFormat::IsSigned(format.type);
	bool _normalized = Render::TextureFormat::IsNormalized(format.type);
	bool _float = Render::TextureFormat::IsFloat(format.type);

	switch (format.pixel_format)
	{
		case Render::PixelType<'r', 8>::ID:
			import_format = _normalized ? GL_RED : GL_RED_INTEGER;
			channel_type = _signed ? GL_BYTE : GL_UNSIGNED_BYTE;
			internal_format = _normalized ? (_signed ? GL_R8_SNORM : GL_R8) : (_signed ? GL_R8I : GL_R8UI);
			break;

		case Render::PixelType<'r', 16>::ID:
			import_format = _normalized ? GL_RED : GL_RED_INTEGER;
			channel_type = _signed ? GL_SHORT : GL_UNSIGNED_SHORT;
			internal_format = _normalized ? (_signed ? GL_R16_SNORM : GL_R16) : (_signed ? GL_R16I : GL_R16UI);
			break;

		case Render::PixelType<'r', 32>::ID:
			import_format = _normalized ? GL_RED : GL_RED_INTEGER;
			channel_type = _signed ? (_float ? GL_FLOAT : GL_INT) : GL_UNSIGNED_INT;
			internal_format = _float ? GL_R32F : (_signed ? GL_R32I : GL_R32UI);
			break;

		case Render::PixelType<'r', 8, 'g', 8>::ID:
			import_format = _normalized ? GL_RG : GL_RG_INTEGER;
			channel_type = _signed ? GL_BYTE : GL_UNSIGNED_BYTE;
			internal_format = _normalized ? (_signed ? GL_RG8_SNORM : GL_RG8) : (_signed ? GL_RG8I : GL_RG8UI);
			break;

		case Render::PixelType<'r', 16, 'g', 16>::ID:
			import_format = _normalized ? GL_RG : GL_RG_INTEGER;
			channel_type = _signed ? GL_SHORT : GL_UNSIGNED_SHORT;
			internal_format = _normalized ? (_signed ? GL_RG16_SNORM : GL_RG16) : (_signed ? GL_RG16I : GL_RG16UI);
			break;

		case Render::PixelType<'r', 32, 'g', 32>::ID:
			import_format = _normalized ? GL_RG : GL_RG_INTEGER;
			channel_type = _signed ? (_float ? GL_FLOAT : GL_INT) : GL_UNSIGNED_INT;
			internal_format = _float ? GL_RG32F : (_signed ? GL_RG32I : GL_RG32UI);
			break;

		case Render::PixelType<'r', 8, 'g', 8, 'b', 8>::ID:
			import_format = _normalized ? GL_RGB : GL_RGB_INTEGER;
			channel_type = _signed ? GL_BYTE : GL_UNSIGNED_BYTE;
			internal_format = _normalized ? (_signed ? GL_RGB8_SNORM : GL_RGB8) : (_signed ? GL_RGB8I : GL_RGB8UI);
			break;

		case Render::PixelType<'r', 16, 'g', 16, 'b', 16>::ID:
			import_format = _normalized ? GL_RGB : GL_RGB_INTEGER;
			channel_type = _signed ? GL_SHORT : GL_UNSIGNED_SHORT;
			internal_format = _normalized ? (_signed ? GL_RGB16_SNORM : GL_RGB16) : (_signed ? GL_RGB16I : GL_RGB16UI);
			break;

		case Render::PixelType<'r', 32, 'g', 32, 'b', 32>::ID:
			import_format = _normalized ? GL_RGB : GL_RGB_INTEGER;
			channel_type = _signed ? (_float ? GL_FLOAT : GL_INT) : GL_UNSIGNED_INT;
			internal_format = _float ? GL_RGB32F : (_signed ? GL_RGB32I : GL_RGB32UI);
			break;

		case Render::PixelType<'r', 8, 'g', 8, 'b', 8, 'a', 8>::ID:
			import_format = _normalized ? GL_RGBA : GL_RGBA_INTEGER;
			channel_type = _signed ? GL_BYTE : GL_UNSIGNED_BYTE;
			internal_format = _normalized ? (_signed ? GL_RGBA8_SNORM : GL_RGBA8) : (_signed ? GL_RGBA8I : GL_RGBA8UI);
			break;

		case Render::PixelType<'r', 16, 'g', 16, 'b', 16, 'a', 16>::ID:
			import_format = _normalized ? GL_RGBA : GL_RGBA_INTEGER;
			channel_type = _signed ? GL_SHORT : GL_UNSIGNED_SHORT;
			internal_format = _normalized ? (_signed ? GL_RGBA16_SNORM : GL_RGBA16) : (_signed ? GL_RGBA16I
			                                                                                   : GL_RGBA16UI);
			break;

		case Render::PixelType<'r', 32, 'g', 32, 'b', 32, 'a', 32>::ID:
			import_format = _normalized ? GL_RGBA : GL_RGBA_INTEGER;
			channel_type = _signed ? (_float ? GL_FLOAT : GL_INT) : GL_UNSIGNED_INT;
			internal_format = _float ? GL_RGBA32F : (_signed ? GL_RGBA32I : GL_RGBA32UI);
			break;


		case Render::PixelType<'r', 5, 'g', 6, 'b', 5>::ID:
			assert(!_signed);
			assert(_normalized);
			import_format = GL_RGB;
			channel_type = GL_UNSIGNED_SHORT_5_6_5;
			internal_format = GL_RGB565;
			break;

		case Render::PixelType<'r', 4, 'g', 4, 'b', 4, 'a', 4>::ID:
			assert(!_signed);
			assert(_normalized);
			import_format = GL_RGBA;
			channel_type = GL_UNSIGNED_SHORT_4_4_4_4;
			internal_format = GL_RGBA4;
			break;

		case Render::PixelType<'r', 5, 'g', 5, 'b', 5, 'a', 1>::ID:
			assert(!_signed);
			assert(_normalized);
			import_format = GL_RGBA;
			channel_type = GL_UNSIGNED_SHORT_5_5_5_1;
			internal_format = GL_RGB5_A1;
			break;

		case Render::PixelType<'r', 10, 'g', 10, 'b', 10, 'a', 2>::ID:
			assert(!_signed);
			import_format = _normalized ? GL_RGBA : GL_RGBA_INTEGER;
			channel_type = GL_UNSIGNED_INT_10_10_10_2;
			internal_format = _normalized ? GL_RGB10_A2 : GL_RGB10_A2UI;
			break;

		default:
			spdlog::error("Could not find proper GL mapping of format: {}", GetStringRepresentation(format));
			throw runtime_error("Could not find proper GL mapping of format: %s", GetStringRepresentation(format).c_str());
	}
	return {internal_format, import_format, channel_type};
}

#include <doctest.h>

TEST_CASE("[Render] PVRReader")
{
	CHECK_EQ(Render::PixelType<'r', 8, 'g', 8, 'b', 8, 'a', 8>::ID, Render::TextureFormat::RGBA8888);
	CHECK_EQ(Render::PixelType<'r', 8, 'g', 8, 'b', 8, 'a', 8>::ID, Render::getPixelType('r', 8, 'g', 8, 'b', 8, 'a', 8));
	CHECK_EQ(Render::PixelType<'r', 8, 'g', 8, 'b', 8, 'a', 8>::ID, Render::ParsePixelType("RGBA8888"));
	CHECK_EQ(Render::PixelType<'r', 8, 'g', 8, 'b', 8>::ID, Render::ParsePixelType("RGB888"));
	CHECK_EQ(Render::PixelType<'r', 5, 'g', 6, 'b', 5>::ID, Render::ParsePixelType("RGB565"));
	CHECK_EQ(Render::PixelType<'r', 32, 'g', 32, 'b', 32, 'a', 32>::ID, Render::ParsePixelType("RGBA32323232"));
	CHECK_NE(Render::PixelType<'r', 8, 'g', 8, 'b', 8, 'a', 8>::ID, Render::ParsePixelType("RGB888"));
	REQUIRE_THROWS(Render::ParsePixelType("RGBA888"));
	REQUIRE_THROWS(Render::ParsePixelType("RGB8888"));
	REQUIRE_THROWS(Render::ParsePixelType(" RGBA8888"));
	REQUIRE_THROWS(Render::ParsePixelType("RGBA8888 "));


	CHECK_EQ(Render::TextureFormat::GetChannelCount(Render::TextureFormat::RGBA8888), 4);
	CHECK_EQ(Render::TextureFormat::GetChannelCount(Render::TextureFormat::RGB888), 3);
	CHECK_EQ(Render::TextureFormat::GetChannelCount(Render::TextureFormat::RG88), 2);
	CHECK_EQ(Render::TextureFormat::GetChannelCount(Render::TextureFormat::R8), 1);
}

