#include "PVRReader.h"
#include <spdlog/spdlog.h>

using namespace Render;


// http://cdn.imgtec.com/sdk-documentation/PVR+File+Format.Specification.pdf

template<char C1Name, char C2Name, char C3Name, char C4Name>
class FourCC
{
public:
	enum : uint32_t
	{
		Value = (static_cast<uint32_t>(C1Name) + (static_cast<uint32_t>(C2Name) << 8u) +
		      (static_cast<uint32_t>(C3Name) << 16u) + (static_cast<uint32_t>(C4Name) << 24u))
	};
};


bool PVRReader::CheckIfPVR(fsal::File file)
{
	auto p = file.Tell();
	uint32_t w;
	file.Read(w);
	file.Seek(p);
	return CheckIfPVR(w);
}


bool PVRReader::CheckIfPVR(uint32_t firtWord)
{
	return MagicNumber == firtWord;
}


PVRReader::PVRReader(fsal::File file): file(file), version(-1), flags(0), channelType(0), numSurfaces(0),
	numFaces(0), metaDataSize(0), pixelFormat(TextureFormat::PVRTCI_2bpp_RGB), colourSpace(TextureFormat::sRGB),
	width(0), height(0), depth(0), MIPMapCount(0), x_axis_flipped(false), y_axis_flipped(false)
{
	if (!file)
	{
		return;
	}

	file.Seek(0);

	file.Read(version);

	file.Read(flags);
	file.Read((uint64_t&)pixelFormat);
	file.Read((uint32_t&)colourSpace);
	file.Read(channelType);
	file.Read(height);
	file.Read(width);
	file.Read(depth);
	file.Read(numSurfaces);
	file.Read(numFaces);
	file.Read(MIPMapCount);
	file.Read(metaDataSize);

	assert(file.Tell() == HeaderSize);

	if (version != MagicNumber)
	{
		spdlog::error("Error reading PVR texture, magic number mismatch\n");
		assert(version != MagicNumber);
		version = -1;
		return;
	}

	assert(flags == 0 || flags == 2);
	assert(colourSpace == TextureFormat::lRGB || colourSpace == TextureFormat::sRGB);
	assert(numSurfaces == 1);
	assert(numFaces == 1 || numFaces == 6);

	while(file.Tell() < HeaderSize + metaDataSize)
	{
		int datatype = 0;
		int key = 0;
		int datasize = 0;
		file.Read(datatype);

		if (datatype == FourCC<'P', 'V', 'R', 3>::Value)
		{
			file.Read(key);
			file.Read(datasize);
			switch (key)
			{
				case 3:
					{
						uint8_t axis[3];
						file.Read(axis, 3);
						spdlog::info("{}", axis[0] ? "X values increase to the left" : "X values increase to the right");
						spdlog::info("{}", axis[1] ? "Y values increase to the upwards" : "Y values increase to the downwards");
						spdlog::info("{}", axis[2] ? "Z values increase to the outwards" : "Z values increase to the inwards");
						x_axis_flipped = axis[0];
						y_axis_flipped = axis[1];
					}
					break;

				case 0:
				case 1:
				case 2:
				case 4:
				case 5:
				default:
					file.Seek(datasize, fsal::File::CurrentPosition);
			}
		}
		else
		{
			break;
		}
	}
}

static int sizeof_4_ary_tree(int h)
{
	return ((1u << uint32_t(2 * (h + 1))) - 1) / (4 - 1);
}

template<typename T, int D>
inline T prod(const glm::vec<D, T>& x)
{
	T v = 1;
	for (int i = 0; i < D; ++i)
		v *= x[i];
	return v;
}

PVRReader::Blob PVRReader::Read(int mipmap, int face)
{
	auto size = GetSize(mipmap);

	// assert(glm::all(glm::lessThanEqual(GetBlockSize(), size)));

	int offset = HeaderSize + metaDataSize;

	if (glm::all(glm::equal(GetBlockSize(), glm::ivec3(1))))
	{
		int offset_smallest_mipmap = sizeof_4_ary_tree(MIPMapCount - 1) - sizeof_4_ary_tree(MIPMapCount - 1 - mipmap);
		auto smallest_mipmap_size = GetSize(MIPMapCount - 1);
		offset += (offset_smallest_mipmap * prod(smallest_mipmap_size) * GetBitsPerPixel()) / 8 * GetFaceCount();
	}
	else
	{
		for (int i = 0; i <= mipmap; ++i)
		{
			auto s = GetSize(mipmap);
			offset += (prod(s) * GetBitsPerPixel()) / 8 * GetFaceCount();
		}
	}
	int face_size = (prod(size) * GetBitsPerPixel()) / 8;
	offset += face_size * face;

	Blob blob;
	blob.size = face_size;
	blob.data.reset(new uint8_t[blob.size]);

	file.Seek(offset);
	file.Read(blob.data.get(), blob.size);
	return blob;
}

glm::ivec3 PVRReader::GetSize(int mipmap) const
{
	uint32_t div = 1u << uint32_t(mipmap);
	auto size = glm::ivec3(width / div, height / div, depth / div);
	if (glm::all(glm::equal(size, glm::ivec3(0))))
	{
		return glm::ivec3(0);
	}
	size = glm::max(size, glm::ivec3(1));
	auto block_size = GetBlockSize();
	size.x = ((size.x + block_size.x - 1) / block_size.x) * block_size.x;
	size.y = ((size.y + block_size.y - 1) / block_size.y) * block_size.y;
	size.z = ((size.z + block_size.z - 1) / block_size.z) * block_size.z;
	return size;
}

int PVRReader::GetFaceCount() const
{
	return numFaces;
}

TextureFormat PVRReader::GetFormat() const
{
	return { colourSpace, pixelFormat, TextureFormat::DataType(channelType) };
}

int PVRReader::GetMipmapCount() const
{
	return MIPMapCount;
}


#include <doctest.h>

TEST_CASE("[Render] PVRReader")
{
	SUBCASE("Basic rgb565")
	{
		fsal::FileSystem fs;
		auto f = fs.Open("test_rgb565.pvr");
		PVRReader reader(f);
		CHECK_EQ(reader.GetSize2D(0), glm::ivec2(1024));
		CHECK_EQ(reader.GetSize2D(2), glm::ivec2(256));
		CHECK_EQ(reader.GetFaceCount(), 1);
		CHECK_EQ(reader.GetMipmapCount(), 11);
		CHECK_EQ(reader.GetBitsPerPixel(), 16);
		CHECK_EQ(reader.GetBlockSize2D(), glm::ivec2(1));
	}

	SUBCASE("Basic etc2")
	{
		fsal::FileSystem fs;
		auto f = fs.Open("test_etc2.pvr");
		PVRReader reader(f);
		CHECK_EQ(reader.GetSize2D(0), glm::ivec2(1024));
		CHECK_EQ(reader.GetSize2D(2), glm::ivec2(256));
		CHECK_EQ(reader.GetFaceCount(), 1);
		CHECK_EQ(reader.GetMipmapCount(), 11);
		CHECK_EQ(reader.GetBitsPerPixel(), 4);
		CHECK_EQ(reader.GetBlockSize2D(), glm::ivec2(4));
	}
	SUBCASE("Basic pvrtc 2bpp")
	{
		fsal::FileSystem fs;
		auto f = fs.Open("test_pvrtc_2bpp.pvr");
		PVRReader reader(f);
		CHECK_EQ(reader.GetSize2D(0), glm::ivec2(1024));
		CHECK_EQ(reader.GetSize2D(2), glm::ivec2(256));
		CHECK_EQ(reader.GetFaceCount(), 1);
		CHECK_EQ(reader.GetMipmapCount(), 11);
		CHECK_EQ(reader.GetBitsPerPixel(), 2);
		CHECK_EQ(reader.GetBlockSize2D(), glm::ivec2(16, 8));
	}

	SUBCASE("Basic pvrtc 4bpp")
	{
		fsal::FileSystem fs;
		auto f = fs.Open("test_pvrtc_4bpp.pvr");
		PVRReader reader(f);
		CHECK_EQ(reader.GetSize2D(0), glm::ivec2(1024));
		CHECK_EQ(reader.GetSize2D(2), glm::ivec2(256));
		CHECK_EQ(reader.GetFaceCount(), 1);
		CHECK_EQ(reader.GetMipmapCount(), 11);
		CHECK_EQ(reader.GetBitsPerPixel(), 4);
		CHECK_EQ(reader.GetBlockSize2D(), glm::ivec2(8));
	}

	SUBCASE("Basic rgb888")
	{
		fsal::FileSystem fs;
		auto f = fs.Open("test_rgb888.pvr");
		PVRReader reader(f);
		CHECK_EQ(reader.GetSize2D(0), glm::ivec2(1024));
		CHECK_EQ(reader.GetSize2D(2), glm::ivec2(256));
		CHECK_EQ(reader.GetFaceCount(), 1);
		CHECK_EQ(reader.GetMipmapCount(), 11);
		CHECK_EQ(reader.GetBitsPerPixel(), 24);
		CHECK_EQ(reader.GetBlockSize2D(), glm::ivec2(1));

//		Image im;
//		im = Image::FromRawData(reader.Read(0, 0).data.get(), Image::DataType::RGB8, reader.GetSize(0));
//		im.SaveToTGA("test_0.tga");
//		im = Image::FromRawData(reader.Read(1, 0).data.get(), Image::DataType::RGB8, reader.GetSize(1));
//		im.SaveToTGA("test_1.tga");
//		im = Image::FromRawData(reader.Read(2, 0).data.get(), Image::DataType::RGB8, reader.GetSize(2));
//		im.SaveToTGA("test_2.tga");
	}
}

