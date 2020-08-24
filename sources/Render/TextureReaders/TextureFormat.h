#pragma once
#include "runtime_error.h"
#include <glm/glm.hpp>
#include <inttypes.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <string.h>
#include <ctype.h>


namespace Render
{
	template<char C1Name, uint8_t C1Bits, char C2Name = 0, uint8_t C2Bits = 0, char C3Name = 0, uint8_t C3Bits = 0, char C4Name = 0, uint8_t C4Bits = 0>
	class PixelType
	{
	public:
		enum : uint64_t
		{
			ID = (static_cast<uint64_t>(C1Name) + (static_cast<uint64_t>(C2Name) << 8u) +
			      (static_cast<uint64_t>(C3Name) << 16u) + (static_cast<uint64_t>(C4Name) << 24u) +
			      (static_cast<uint64_t>(C1Bits) << 32u) + (static_cast<uint64_t>(C2Bits) << 40u) +
			      (static_cast<uint64_t>(C3Bits) << 48u) + (static_cast<uint64_t>(C4Bits) << 56u))
		};
	};

	inline uint64_t
	getPixelType(char C1Name, uint8_t C1Bits, char C2Name = 0, uint8_t C2Bits = 0, char C3Name = 0, uint8_t C3Bits = 0,
	             char C4Name = 0, uint8_t C4Bits = 0)
	{
		return (static_cast<uint64_t>(C1Name) + (static_cast<uint64_t>(C2Name) << 8u) +
		        (static_cast<uint64_t>(C3Name) << 16u) + (static_cast<uint64_t>(C4Name) << 24u) +
		        (static_cast<uint64_t>(C1Bits) << 32u) + (static_cast<uint64_t>(C2Bits) << 40u) +
		        (static_cast<uint64_t>(C3Bits) << 48u) + (static_cast<uint64_t>(C4Bits) << 56u));
	}

	uint64_t ParsePixelType(const char* s);

	struct DecodedType
	{
		bool compressed;
		std::vector<char> channel_names;
		std::vector<uint8_t> channel_sizes;
	};

	DecodedType DecodePixelType(uint64_t pixel_format);


	struct TextureFormat
	{
		enum ColourSpace: uint32_t
		{
			lRGB = 0,
			sRGB = 1
		};

		enum Format: uint64_t
		{
			PVRTCI_2bpp_RGB,
			PVRTCI_2bpp_RGBA,
			PVRTCI_4bpp_RGB,
			PVRTCI_4bpp_RGBA,

			PVRTCII_2bpp,
			PVRTCII_4bpp,

			ETC1,

			DXT1,
			DXT2,
			DXT3,
			DXT4,
			DXT5,
			BC1 = DXT1,
			BC2 = DXT3,
			BC3 = DXT5,

			RGBG8888 = 20,
			GRGB8888,

			ETC2_RGB = 22,
			ETC2_RGBA,
			ETC2_RGB_A1,

			EAC_R11,
			EAC_RG11,

			ASTC_4x4,
			ASTC_5x4,
			ASTC_5x5,
			ASTC_6x5,
			ASTC_6x6,
			ASTC_8x5,
			ASTC_8x6,
			ASTC_8x8,
			ASTC_10x5,
			ASTC_10x6,
			ASTC_10x8,
			ASTC_10x10,
			ASTC_12x10,
			ASTC_12x12,

			RGBA8888 = PixelType<'r', 8, 'g', 8, 'b', 8, 'a', 8>::ID,
			RGBA1010102 = PixelType<'r', 10, 'g', 10, 'b', 10, 'a', 2>::ID,
			RGBA4444 = PixelType<'r', 4, 'g', 4, 'b', 4, 'a', 4>::ID,
			RGBA5551 = PixelType<'r', 5, 'g', 5, 'b', 5, 'a', 1>::ID,
			BGRA8888 = PixelType<'b', 8, 'g', 8, 'r', 8, 'a', 8>::ID,
			RGBA16161616 = PixelType<'r', 16, 'g', 16, 'b', 16, 'a', 16>::ID,
			RGBA32323232 = PixelType<'r', 32, 'g', 32, 'b', 32, 'a', 32>::ID,

			RGB888 = PixelType<'r', 8, 'g', 8, 'b', 8>::ID,
			RGB161616 = PixelType<'r', 16, 'g', 16, 'b', 16>::ID,
			RGB565 = PixelType<'r', 5, 'g', 6, 'b', 5>::ID,
			RGB323232 = PixelType<'r', 32, 'g', 32, 'b', 32>::ID,
			BGR101111 = PixelType<'b', 10, 'g', 11, 'r', 11>::ID,

			RG88 = PixelType<'r', 8, 'g', 8>::ID,
			LA88 = PixelType<'l', 8, 'a', 8>::ID,
			RG1616 = PixelType<'r', 16, 'g', 16>::ID,
			RG3232 = PixelType<'r', 32, 'g', 32>::ID,

			R8 = PixelType<'r', 8>::ID,
			A8 = PixelType<'a', 8>::ID,
			L8 = PixelType<'l', 8>::ID,
			R16 = PixelType<'r', 16>::ID,
			R32 = PixelType<'r', 32>::ID,
		};

		enum DataType: uint32_t
		{
			Signed   = 0b00001,
			Norm     = 0b00010,

			WidthMask= 0b11100,

			Byte     = 0b00000,
			Short    = 0b00100,
			Integer  = 0b01000,
			Float    = 0b01100,

			UnsignedByteNormalized = Byte,
			SignedByteNormalized = Byte | Signed,
			UnsignedByte = Byte | Norm,
			SignedByte = Byte | Norm | Signed,
			UnsignedShortNormalized = Short,
			SignedShortNormalized = Short | Signed,
			UnsignedShort = Short | Norm,
			SignedShort = Short | Norm | Signed,
			UnsignedIntegerNormalized = Integer,
			SignedIntegerNormalized = Integer | Signed,
			UnsignedInteger = Integer | Norm,
			SignedInteger = Integer | Norm | Signed,
		};

		static bool IsNormalized(DataType t) { return !(t & Norm); }
		static bool IsSigned(DataType t)     { return t & Signed; }
		static bool IsByte(DataType t)       { return (t & WidthMask) == Byte; }
		static bool IsShort(DataType t)      { return (t & WidthMask) == Short; }
		static bool IsInteger(DataType t)    { return (t & WidthMask) == Integer; }
		static bool IsFloat(DataType t)      { return (t & WidthMask) == Float; }

		ColourSpace colorspace;
		Format pixel_format;
		DataType type;

		static size_t GetBitsPerPixel(Format f);
		static glm::ivec2 GetMinBlockSize(Format f);
		static size_t GetChannelCount(Format f)
		{
			auto decoded = DecodePixelType(f);
			return decoded.channel_names.size();
		}
	};

	std::vector<uint32_t> GetGLMappedTypes(TextureFormat format);
}
