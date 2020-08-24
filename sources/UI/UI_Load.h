#pragma once
#include "Block.h"
#include <fsal.h>


namespace UI
{
	BlockPtr Load(const fsal::File& f);
}
