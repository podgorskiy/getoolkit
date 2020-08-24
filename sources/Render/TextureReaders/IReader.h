#pragma once
#include "Render/TextureReaders/TextureFormat.h"
#include <glm/glm.hpp>
#include <memory>


namespace Render
{
	class IReader;
	typedef std::shared_ptr<IReader> IReaderPtr;

	class IReader
	{
	public:
		struct Blob
		{
			std::shared_ptr<uint8_t> data;
			size_t size;
		};

		virtual Blob Read(int mipmap, int face) = 0;

		virtual glm::ivec3 GetSize(int mipmap) const = 0;

		virtual glm::ivec2 GetSize2D(int mipmap) const
		{
			return glm::ivec2(GetSize(mipmap));
		}

		virtual int GetFaceCount() const = 0;

		virtual int GetMipmapCount() const = 0;

		virtual TextureFormat GetFormat() const = 0;

		virtual size_t GetBitsPerPixel() const
		{
			return TextureFormat::GetBitsPerPixel(GetFormat().pixel_format);
		}

		virtual glm::ivec3 GetBlockSize() const
		{
			return glm::ivec3(TextureFormat::GetMinBlockSize(GetFormat().pixel_format), 1);
		}

		virtual glm::ivec2 GetBlockSize2D() const
		{
			return TextureFormat::GetMinBlockSize(GetFormat().pixel_format);
		}

		virtual ~IReader() = default;
	};


	class TextureReader: private IReader
	{
	public:
		TextureReader(IReaderPtr reader): m_reader(reader) {}
		TextureReader() = default;
		TextureReader(const TextureReader& other) = default;

		IReader::Blob Read(int mipmap, int face) final { return m_reader->Read(mipmap, face); }

		glm::ivec3 GetSize(int mipmap) const final  { return m_reader->GetSize(mipmap); }

		int GetFaceCount() const final   { return m_reader->GetFaceCount(); }

		int GetMipmapCount() const final { return m_reader->GetMipmapCount(); }

		TextureFormat GetFormat() const final { return m_reader->GetFormat(); }

		size_t GetBitsPerPixel() const final  { return m_reader->GetBitsPerPixel(); }

		glm::ivec3 GetBlockSize() const final  { return m_reader->GetBlockSize(); }

	private:
		IReaderPtr m_reader;
	};
}
