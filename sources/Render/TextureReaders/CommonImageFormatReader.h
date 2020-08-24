#pragma once
#include "IReader.h"
#include "TextureFormat.h"
#include <inttypes.h>
#include <fsal.h>


namespace Render
{
	class CommonImageFormatReader: public IReader
	{
	public:
		explicit CommonImageFormatReader(fsal::File file);

		static bool CheckIfCommonImage(fsal::File file);

		Blob Read(int mipmap, int face) final;

		glm::ivec3 GetSize(int mipmap) const final;

		int GetFaceCount() const final { return 1; };

		TextureFormat GetFormat() const final;

		int GetMipmapCount() const final { return 1; };
	private:
		fsal::File file;
		TextureFormat::DataType channelType;
		TextureFormat::Format pixelFormat;
		glm::ivec2 m_size;
		bool hdr;
		bool is16;
	};

	inline TextureReader MakeCommonImageFormatReader(fsal::File file)
	{
		Render::IReaderPtr reader = std::make_shared<CommonImageFormatReader>(file);
		return Render::TextureReader(reader);
	}
}
