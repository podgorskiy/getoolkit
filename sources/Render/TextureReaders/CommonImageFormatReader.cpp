#include "CommonImageFormatReader.h"
#include <spdlog/spdlog.h>
#include <stb_image.h>

using namespace Render;


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

static int read(void *user,char *data,int size)   // fill 'data' with 'size' bytes.  return number of bytes actually read
{
	auto* file = (fsal::File*)user;
	size_t readBytes = 0;
	file->Read((uint8_t*)data, size, &readBytes);
	return readBytes;
}

static void skip(void *user, int n)            // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
{
	auto* file = (fsal::File*)user;
	auto p = file->Tell();
	file->Seek(p + n);
}

static int eof(void *user)                       // returns nonzero if we are at end of file/data
{
	auto* file = (fsal::File*)user;
	return file->Tell() == file->GetSize();
}

bool CommonImageFormatReader::CheckIfCommonImage(fsal::File file)
{
	file.Seek(0);
	stbi_io_callbacks c = {read, skip, eof};
	int x; int y; int comp;
	return stbi_info_from_callbacks(&c, &file, &x, &y, &comp);
}


CommonImageFormatReader::CommonImageFormatReader(fsal::File file): file(file), channelType(TextureFormat::UnsignedByteNormalized),
	pixelFormat(TextureFormat::PVRTCI_2bpp_RGB)
{
	if (!file)
	{
		return;
	}

	stbi_io_callbacks c = {read, skip, eof};
	int x; int y; int comp;
	file.Seek(0);
	int res = stbi_info_from_callbacks(&c, &file, &x, &y, &comp);
	assert(res == 1);
	file.Seek(0);
	hdr = stbi_is_hdr_from_callbacks(&c, &file);
	is16 = stbi_is_16_bit_from_callbacks(&c, &file);

	m_size = glm::ivec2(x, y);

	uint8_t buff[8] = { 0 };
	const char* names = "rgba";
	for (int i = 0; i < comp; ++i)
	{
		buff[2 * i + 0] = names[i];
		buff[2 * i + 1] = hdr ? 32 : (is16 ? 16 : 8);
	}

	pixelFormat = (TextureFormat::Format)getPixelType(buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6], buff[7]);
	channelType = hdr ? TextureFormat::Float : (is16 ?  TextureFormat::UnsignedShortNormalized : TextureFormat::UnsignedByteNormalized);
}

CommonImageFormatReader::Blob CommonImageFormatReader::Read(int mipmap, int face)
{
	auto size = GetSize(mipmap);

	stbi_io_callbacks c = {read, skip, eof};
	int x; int y; int comp;
	file.Seek(0);

	uint8_t* data = nullptr;
	size_t data_size = 0;

	stbi_set_flip_vertically_on_load(true);
	stbi_set_unpremultiply_on_load(true);
	stbi_convert_iphone_png_to_rgb(true);

	if (hdr)
	{
		data = (uint8_t*)stbi_loadf_from_callbacks(&c, &file, &x, &y, &comp, 0);
		assert(size.x == x);
		assert(size.y == y);
		data_size = x * y * comp * sizeof(float);
	}
	else if (is16)
	{
		data = (uint8_t*)stbi_load_16_from_callbacks(&c, &file, &x, &y, &comp, 0);
		assert(size.x == x);
		assert(size.y == y);
		data_size = x * y * comp * sizeof(short);
	}
	else
	{
		data = (uint8_t*)stbi_load_from_callbacks(&c, &file, &x, &y, &comp, 0);
		assert(size.x == x);
		assert(size.y == y);
		data_size = x * y * comp * sizeof(uint8_t);
	}

	Blob blob;
	blob.size = data_size;
	blob.data = std::shared_ptr<uint8_t>(data, [](uint8_t* p){ free(p); });

	return blob;
}

glm::ivec3 CommonImageFormatReader::GetSize(int mipmap) const
{
	if (mipmap > 0)
	{
		return glm::ivec3(0);
	}
	return glm::ivec3(m_size, 1);
}

TextureFormat CommonImageFormatReader::GetFormat() const
{
	return { TextureFormat::lRGB, pixelFormat, channelType };
}


#include <doctest.h>

TEST_CASE("[Render] PVRReader")
{
	SUBCASE("Basic jpg")
	{
		fsal::FileSystem fs;
		auto f = fs.Open("test.jpg");
		CommonImageFormatReader reader(f);
		CHECK_EQ(reader.GetSize2D(0), glm::ivec2(1024));
		CHECK_EQ(reader.GetSize2D(2), glm::ivec2(0));
		CHECK_EQ(reader.GetFaceCount(), 1);
		CHECK_EQ(reader.GetMipmapCount(), 1);
		CHECK_EQ(reader.GetBitsPerPixel(), 24);
		CHECK_EQ(reader.GetBlockSize2D(), glm::ivec2(1));
	}
}

