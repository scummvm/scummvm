#ifndef DUNE_SPRITE_SHEET_H
#define DUNE_SPRITE_SHEET_H

#include "dune/archive.h"

#include "common/array.h"
#include "common/memstream.h"

namespace Dune {

class DuneEngine;

class SpriteSheet {
	DuneEngine *_vm;

	int indexOffset;
	int count;
	Resource r;
	Common::Array<uint16> offsets;

public:
	SpriteSheet(DuneEngine *_vm, Resource r);
	void draw(int index, int x, int y, byte flags);
};

} // End of namespace Dune

#endif
