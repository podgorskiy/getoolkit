#include "TextureLoader.h"
#include "PVRReader.h"
#include "CommonImageFormatReader.h"
#include <spdlog/spdlog.h>


Render::TexturePtr Render::LoadTexture(fsal::Location path, fsal::FileSystem* fs)
{
	fsal::FileSystem _fs;
	if (fs == nullptr)
	{
		fs = &_fs;
	}

	auto file = fs->Open(path);

	if (!file)
	{
		spdlog::error("Could not load texture, no such file: {}", path.GetFullPath().string());
		return nullptr;
	}

	auto p = file.Tell();
	uint32_t w;
	file.Read(w);
	file.Seek(p);

	TextureReader reader;

	if (Render::PVRReader::CheckIfPVR(w))
	{
		reader = MakePVRReader(file);
	}
	else if (Render::CommonImageFormatReader::CheckIfCommonImage(file))
	{
		reader = MakeCommonImageFormatReader(file);
	}
	else
	{
		spdlog::error("Could not load texture (unknown format): {}", file.GetPath().string());
	}
	auto texture = Render::Texture::LoadTexture(Render::TextureReader(reader));
	return texture;
}
