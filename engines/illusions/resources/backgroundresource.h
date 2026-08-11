/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ILLUSIONS_BACKGROUNDRESOURCE_H
#define ILLUSIONS_BACKGROUNDRESOURCE_H

#include "illusions/camera.h"
#include "illusions/graphics.h"
#include "illusions/pathfinder.h"
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
struct Sequence;

class BackgroundResourceLoader : public BaseResourceLoader {
public:
	BackgroundResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	~BackgroundResourceLoader() override {}
	void load(Resource *resource) override;
	bool isFlag(int flag) override;
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

class RegionLayer {
public:
	void load(byte *dataStart, Common::SeekableReadStream &stream);
	int getRegionIndex(Common::Point pos);
	uint32 getRegionSequenceId(int regionIndex);
protected:
	uint32 _unk;
	byte *_regionSequenceIds;
	int16 _width, _height;
	int16 _mapWidth, _mapHeight;
	byte *_map, *_values;
};

struct Palette {
	uint16 _count;
	uint16 _unk;
	byte *_palette;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

struct BackgroundObject {
	uint32 _objectId;
	uint16 _flags;
	int16 _priority;
	byte *_pointsConfig;
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

struct PathWalkPoints {
	PointArray *_points;
	PathWalkPoints() : _points(0) {}
	~PathWalkPoints() { delete _points; }
	void load(byte *dataStart, Common::SeekableReadStream &stream);
};

struct PathWalkRects {
	PathLines *_rects;
	PathWalkRects() : _rects(0) {}
	~PathWalkRects() { delete _rects; }
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
	RegionLayer *getRegionLayer(uint index);
	PathWalkPoints *getPathWalkPoints(uint index);
	PathWalkRects *getPathWalkRects(uint index);
	Palette *getPalette(uint index);
	bool findNamedPoint(uint32 namedPointId, Common::Point &pt);
public:

	uint _paletteIndex;

	uint _bgInfosCount;
	BgInfo *_bgInfos;

	uint _priorityLayersCount;
	PriorityLayer *_priorityLayers;

	uint _scaleLayersCount;
	ScaleLayer *_scaleLayers;

	uint _regionLayersCount;
	RegionLayer *_regionLayers;

	uint _regionSequencesCount;
	Sequence *_regionSequences;

	uint _backgroundObjectsCount;
	BackgroundObject *_backgroundObjects;

	uint _pathWalkPointsCount;
	PathWalkPoints *_pathWalkPoints;

	uint _pathWalkRectsCount;
	PathWalkRects *_pathWalkRects;

	NamedPoints _namedPoints;

	uint _palettesCount;
	Palette *_palettes;

};

const uint kMaxBackgroundItemSurfaces = 3;

class BackgroundInstance : public ResourceInstance {
public:
	BackgroundInstance(IllusionsEngine *vm);
	~BackgroundInstance();
	void load(Resource *resource) override;
	void unload() override;
	void pause() override;
	void unpause() override;
public:
	IllusionsEngine *_vm;
	uint32 _sceneId;
	int _pauseCtr;
	BackgroundResource *_bgRes;
	Common::Point _panPoints[kMaxBackgroundItemSurfaces];
	Graphics::Surface *_surfaces[kMaxBackgroundItemSurfaces];
	CameraState _savedCameraState;
	byte *_savedPalette;
	void registerResources();
	void unregisterResources();
	void initSurface();
	void freeSurface();
	void drawTiles(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels);
	void drawTiles8(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels);
	void drawTiles16(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels);
};

class BackgroundInstanceList {
public:
	BackgroundInstanceList(IllusionsEngine *vm);
	~BackgroundInstanceList();
	BackgroundInstance *createBackgroundInstance(Resource *resource);
	void removeBackgroundInstance(BackgroundInstance *backgroundInstance);
	void pauseBySceneId(uint32 sceneId);
	void unpauseBySceneId(uint32 sceneId);
	BackgroundInstance *findActiveBackgroundInstance();
	BackgroundInstance *findBackgroundByResource(BackgroundResource *backgroundResource);
	BackgroundResource *getActiveBgResource();
	WidthHeight getMasterBgDimensions();
	void refreshPan();
	bool findActiveBackgroundNamedPoint(uint32 namedPointId, Common::Point &pt);
//protected:
public:
	typedef Common::List<BackgroundInstance*> Items;
	typedef Items::iterator ItemsIterator;
	IllusionsEngine *_vm;
	Items _items;
};

} // End of namespace Illusions

#endif // ILLUSIONS_BACKGROUNDRESOURCE_H
