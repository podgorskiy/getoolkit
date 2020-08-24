#pragma once
#include "IReader.h"
#include "TextureFormat.h"
#include <inttypes.h>
#include <fsal.h>


namespace Render
{
	class PVRReader: public IReader
	{

		enum{
			HeaderSize = 52,
			MagicNumber = 0x03525650,
			PreMultiplied = 0x02,
		};

	public:
		explicit PVRReader(fsal::File file);

		static bool CheckIfPVR(fsal::File file);
		static bool CheckIfPVR(uint32_t firtWord);

		Blob Read(int mipmap, int face) final;

		glm::ivec3 GetSize(int mipmap) const final;

		int GetFaceCount() const final;

		TextureFormat GetFormat() const final;

		int GetMipmapCount() const final;
	private:
		fsal::File file;
		int version;
		int flags;
		int channelType;
		int numSurfaces;
		int numFaces;
		uint32_t metaDataSize;
		TextureFormat::Format pixelFormat;
		TextureFormat::ColourSpace colourSpace;
		int width;
		int height;
		int depth;
		int MIPMapCount;
		bool x_axis_flipped;
		bool y_axis_flipped;
	};

	inline TextureReader MakePVRReader(fsal::File file)
	{
		Render::IReaderPtr reader = std::make_shared<PVRReader>(file);
		return Render::TextureReader(reader);
	}
}
