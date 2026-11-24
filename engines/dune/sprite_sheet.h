#ifndef DUNE_SPRITE_SHEET_H
#define DUNE_SPRITE_SHEET_H

#include "dune/archive.h"

#include "common/array.h"
#include "common/memstream.h"
#include "common/rect.h"

namespace Dune {

class DuneEngine;

class SpriteSheet {
	DuneEngine *_vm;

	int indexOffset;
	int count;
	Resource r;
	Common::Array<uint16> offsets;

public:
	SpriteSheet(DuneEngine *_vm, Resource r, bool takeItsPalette = true);
	// C13E reinstalls a resident bank's colours when returning from PERS.
	void applyPalette();

	// The word the caller holds a sprite in, which is what 0xC22F is handed:
	// the sprite's own number in its low nine bits, the two flips in 0x2000
	// and 0x4000, and which of the eight sizes to draw it at in 0x1C00.  The
	// palette offset stands apart because a scene keeps it in a byte of its
	// own, and where it is not nought it stands in place of the sprite's.
	void draw(int index, int x, int y, uint16 word, byte palOverride);

	// The bytes of one entry, which is not always a picture: the two banks of
	// the sky keep their palettes as entries of their own, and 0x3988 reads
	// one six bytes in.  Answers false where the entry does not reach that
	// far.
	bool read(int index, uint32 skip, byte *into, uint32 size);

	// How big one sprite is, which the caller needs to put it down by its
	// middle.
	bool size(int index, int *width, int *height);
	// C30D draws map actors through the driver's rectangle-clipped path.
	void drawMapSprite(int index, int x, int y, const Common::Rect &clip);
	int entryCount() const { return count; }
};

} // End of namespace Dune

#endif
