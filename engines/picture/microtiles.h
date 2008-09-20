#ifndef PICTURE_MICROTILES_H
#define PICTURE_MICROTILES_H

#include "common/scummsys.h"
#include "common/util.h"
#include "common/rect.h"

namespace Picture {

typedef uint32 BoundingBox;

const BoundingBox FullBoundingBox  = 0x00001F1F;
const BoundingBox EmptyBoundingBox = 0x00000000;
const int TileSize = 32;

class MicroTileArray {
public:
	MicroTileArray(int16 width, int16 height);
	~MicroTileArray();
	void addRect(Common::Rect r);
	void clear();
	Common::Rect *getRectangles(int *num_rects, int min_x, int min_y, int max_x, int max_y);
protected:
	BoundingBox *_tiles;
	int16 _tilesW, _tilesH;
	byte TileX0(const BoundingBox &boundingBox);
	byte TileY0(const BoundingBox &boundingBox);
	byte TileX1(const BoundingBox &boundingBox);
	byte TileY1(const BoundingBox &boundingBox);
	bool isBoundingBoxEmpty(const BoundingBox &boundingBox);
	bool isBoundingBoxFull(const BoundingBox &boundingBox);
	void setBoundingBox(BoundingBox &boundingBox, byte x0, byte y0, byte x1, byte y1);
	void updateBoundingBox(BoundingBox &boundingBox, byte x0, byte y0, byte x1, byte y1);
};

} // namespace Picture

#endif // PICTURE_MICROTILES_H
