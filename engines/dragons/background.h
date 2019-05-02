/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef SCUMMVM_BACKGROUND_H
#define SCUMMVM_BACKGROUND_H

#include "common/rect.h"
#include "common/system.h"
#include "bigfile.h"
#include "dragonrms.h"

namespace Dragons {
class PriorityLayer;
class Background;

void drawTileToSurface(Graphics::Surface *surface, byte *palette, byte *tile, uint32 x, uint32 y);

class BackgroundResourceLoader {
private:
	BigfileArchive *_bigFileArchive;
	DragonRMS *_dragonRMS;
public:
	BackgroundResourceLoader(BigfileArchive *bigFileArchive, DragonRMS *dragonRMS);
	Background *load(uint32 sceneId);
	Background *load(const char *filename);
};

struct TileMap {
	uint16 w;
	uint16 h;
	uint32 size;
	byte *map;
	uint16 tileIndexOffset;
};

class Background {
private:
	byte *_data;
	byte *_tileDataOffset;
	TileMap _tileMap[4];
	PriorityLayer *_priorityLayer;
	byte _palette[512];
	Graphics::Surface *_layer[3];
	Common::Point *_points1;
	Common::Point *_points2;
public:
	Background();
	~Background();

	bool load(byte *dataStart, uint32 size);
	uint16 getWidth();
	uint16 getHeight();
	Graphics::Surface *getBgLayer() { return _layer[0]; }
	Graphics::Surface *getMgLayer() { return _layer[1]; }
	Graphics::Surface *getFgLayer() { return _layer[2]; }
	int16 getPriorityAtPoint(Common::Point pos);
	Common::Point getPoint2(uint32 pointIndex);
	byte *getPalette() { return _palette; }

	void overlayPriorityTileMap(byte *data, int16 x, int16 y, int16 w, int16 h);
	void restorePriorityTileMap(int16 x, int16 y, int16 w, int16 h);
	void overlayImage(uint16 layerNum, byte *data, int16 x, int16 y, int16 w, int16 h);
	void restoreTiles(uint16 layerNum, int16 x, int16 y, int16 w, int16 h);

private:
	Common::Point *loadPoints(Common::SeekableReadStream &stream);
	Graphics::Surface *loadGfxLayer(TileMap &tileMap, byte *tiles);
};

class PriorityLayer {
public:
	void load(TileMap &tileMap, byte *tiles);
	int16 getPriority(Common::Point pos);
	void overlayTileMap(byte *data, int16 x, int16 y, int16 w, int16 h);
	void restoreTileMap(int16 x, int16 y, int16 w, int16 h);
protected:
	int16 _width, _height;
	int16 _mapWidth, _mapHeight;
	byte *_map, *_values;
	byte *_mapBase;
};

} // End of namespace Dragons

#endif //SCUMMVM_BACKGROUND_H
