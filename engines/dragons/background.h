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

struct TileMap;

class BackgroundResourceLoader {
private:
	BigfileArchive *_bigFileArchive;
	DragonRMS *_dragonRMS;
public:
	BackgroundResourceLoader(BigfileArchive *bigFileArchive, DragonRMS *dragonRMS);
	Background *load(uint32 sceneId);
};

class Background {
private:
	PriorityLayer *_priorityLayer;
	byte _palette[512];
	Graphics::Surface *_bgLayer;
	Graphics::Surface *_mgLayer;
	Graphics::Surface *_fgLayer;
	Common::Point *_points1;
	Common::Point *_points2;
public:
	Background() : _priorityLayer(0), _points1(0), _points2(0), _bgLayer(0), _fgLayer(0) {}
	bool load(byte *dataStart, Common::SeekableReadStream &stream);
	uint16 getWidth();
	uint16 getHeight();
	Graphics::Surface *getBgLayer() { return _bgLayer; }
	Graphics::Surface *getMgLayer() { return _mgLayer; }
	Graphics::Surface *getFgLayer() { return _fgLayer; }
	Common::Point getPoint2(uint32 pointIndex);
	byte *getPalette() { return _palette; }

private:
	Common::Point *loadPoints(Common::SeekableReadStream &stream);
	Graphics::Surface *loadGfxLayer(TileMap &tileMap, byte *tiles);
	void drawTileToSurface(Graphics::Surface *surface, byte *tile, uint32 x, uint32 y);
};

class PriorityLayer {
public:
	void load(TileMap &tileMap, byte *tiles);
	int getPriority(Common::Point pos);
protected:
	int16 _width, _height;
	int16 _mapWidth, _mapHeight;
	byte *_map, *_values;
};

} // End of namespace Dragons

#endif //SCUMMVM_BACKGROUND_H
