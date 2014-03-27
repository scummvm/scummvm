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

#ifndef ILLUSIONS_BACKGROUNDRESOURCE_H
#define ILLUSIONS_BACKGROUNDRESOURCE_H

#include "illusions/graphics.h"
#include "illusions/resourcesystem.h"
#include "graphics/surface.h"

#include "common/array.h"
#include "common/file.h"
#include "common/list.h"
#include "common/memstream.h"
#include "common/rect.h"
#include "common/substream.h"
#include "common/system.h"

namespace Illusions {

class IllusionsEngine;

class BackgroundResourceLoader : public BaseResourceLoader {
public:
	BackgroundResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	virtual ~BackgroundResourceLoader() {}
	virtual void load(Resource *resource);
	virtual void unload(Resource *resource);
	virtual void buildFilename(Resource *resource);
	virtual bool isFlag(int flag);
protected:
	IllusionsEngine *_vm;
};

struct TileMap {
	int16 _width, _height;
	//field_4 dd
	byte *_map;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

struct BgInfo {
	uint32 _flags;
	//field_4 dw
	int16 _priorityBase;
	SurfInfo _surfInfo;
	Common::Point _panPoint;
	TileMap _tileMap;
	byte *_tilePixels;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

class PriorityLayer {
public:
	void load(byte *dataStart, Common::SeekableReadStream &stream);
	int getPriority(Common::Point pos);
protected:
	int16 _width, _height;
	int16 _mapWidth, _mapHeight;
	byte *_map, *_values;
};

class ScaleLayer {
public:
	void load(byte *dataStart, Common::SeekableReadStream &stream);
	int getScale(Common::Point pos);
protected:
	int16 _height;
	byte *_values;
};

#if 0
BgResource_PathWalkRects struc ; (sizeof=0x8)
count dd ?
rects dd ?
BgResource_PathWalkRects ends

BgResource_PathWalkPoints struc ; (sizeof=0x8)
count dd ?
points dd ?
BgResource_PathWalkPoints ends
#endif

struct BackgroundObject {
	uint32 _objectId;
	uint16 _flags;
	int16 _priority;
	byte *_pointsConfig;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

class BackgroundResource {
public:
	BackgroundResource();
	~BackgroundResource();
	void load(byte *data, uint32 dataSize);
	int findMasterBgIndex();
	PriorityLayer *getPriorityLayer(uint index);
	ScaleLayer *getScaleLayer(uint index);
	bool findNamedPoint(uint32 namedPointId, Common::Point &pt);
public:

	uint _bgInfosCount;
	BgInfo *_bgInfos;
	
	uint _priorityLayersCount;
	PriorityLayer *_priorityLayers;

	uint _scaleLayersCount;
	ScaleLayer *_scaleLayers;
	
	uint _backgroundObjectsCount;
	BackgroundObject *_backgroundObjects;
	
	NamedPoints _namedPoints;

};

const uint kMaxBackgroundItemSurfaces = 3;

class BackgroundItem {
public:
	BackgroundItem(IllusionsEngine *vm);
	~BackgroundItem();
	void initSurface();
	void freeSurface();
	void drawTiles(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels);
	void pause();
	void unpause();
	void refreshPan(WidthHeight &dimensions);
public:
	IllusionsEngine *_vm;
	uint32 _tag;
	int _pauseCtr;
	BackgroundResource *_bgRes;
	Common::Point _panPoints[kMaxBackgroundItemSurfaces];
	Graphics::Surface *_surfaces[kMaxBackgroundItemSurfaces];
	// TODO SavedCamera savedCamera;
	// TODO? byte *savedPalette;
};

class BackgroundItems {
public:
	BackgroundItems(IllusionsEngine *vm);
	~BackgroundItems();
	BackgroundItem *allocBackgroundItem();
	void freeBackgroundItem(BackgroundItem *backgroundItem);
	void pauseByTag(uint32 tag);
	void unpauseByTag(uint32 tag);
	BackgroundItem *findActiveBackground();
	BackgroundItem *findBackgroundByResource(BackgroundResource *backgroundResource);
	BackgroundResource *getActiveBgResource();
	WidthHeight getMasterBgDimensions();
	void refreshPan();
	bool findActiveBackgroundNamedPoint(uint32 namedPointId, Common::Point &pt);
	BackgroundItem *debugFirst();
//protected:
public:
	typedef Common::List<BackgroundItem*> Items;
	typedef Items::iterator ItemsIterator;
	IllusionsEngine *_vm;
	Items _items;
};

} // End of namespace Illusions

#endif // ILLUSIONS_BACKGROUNDRESOURCE_H
