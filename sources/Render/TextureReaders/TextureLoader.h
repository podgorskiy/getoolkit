#pragma once
#include "Render/TextureReaders/IReader.h"
#include "Render/Texture.h"
#include <fsal.h>

namespace Render
{
	TexturePtr LoadTexture(fsal::Location path, fsal::FileSystem* fs = nullptr);
}
