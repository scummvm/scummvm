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

#include "illusions/illusions.h"
#include "illusions/resources/backgroundresource.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/resources/actorresource.h"
#include "illusions/screen.h"
#include "illusions/sequenceopcodes.h"
#include "common/str.h"

namespace Illusions {

// BackgroundResourceLoader

void BackgroundResourceLoader::load(Resource *resource) {
	resource->_instance = _vm->_backgroundInstances->createBackgroundInstance(resource);
}

bool BackgroundResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

// TileMap

void TileMap::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_width = stream.readSint16LE();
	_height = stream.readSint16LE();
	stream.skip(4); // Unknown
	uint32 mapOffs = stream.pos();
	_map = dataStart + mapOffs;
	debug(0, "TileMap::load() _width: %d; _height: %d",
		_width, _height);
}

// BgInfo

void BgInfo::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_flags = stream.readUint32LE();
	uint16 unknown = stream.readUint16LE(); //	TODO Unknown
	_priorityBase = stream.readSint16LE();
	_surfInfo.load(stream);
	loadPoint(stream, _panPoint);
	uint32 tileMapOffs = stream.readUint32LE();
	uint32 tilePixelsOffs = stream.readUint32LE();
	stream.seek(tileMapOffs);
	_tileMap.load(dataStart, stream);
	_tilePixels = dataStart + tilePixelsOffs;
	debug(0, "BgInfo::load() _flags: %08X; unknown: %04X; _priorityBase: %d; tileMapOffs: %08X; tilePixelsOffs: %08X",
		_flags, unknown, _priorityBase, tileMapOffs, tilePixelsOffs);
}

// PriorityLayer

void PriorityLayer::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	uint32 mapOffs = stream.readUint32LE();
	uint32 valuesOffs = stream.readUint32LE();
	_map = dataStart + mapOffs;
	_mapWidth = READ_LE_UINT16(_map + 0);
	_mapHeight = READ_LE_UINT16(_map + 2);
	_map += 8;
	_values = dataStart + valuesOffs;
	debug(0, "PriorityLayer::load() _width: %d; _height: %d; mapOffs: %08X; valuesOffs: %08X; _mapWidth: %d; _mapHeight: %d",
		_width, _height, mapOffs, valuesOffs, _mapWidth, _mapHeight);
}

int PriorityLayer::getPriority(Common::Point pos) {
	pos.x = CLIP<int16>(pos.x, 0, _width - 1);
	pos.y = CLIP<int16>(pos.y, 0, _height - 1);
	const int16 tx = pos.x / 32, sx = pos.x % 32;
	const int16 ty = pos.y / 8, sy = pos.y % 8;
	uint16 mapIndex = READ_LE_UINT16(_map + 2 * (tx + ty * _mapWidth)) - 1;
	return _values[mapIndex * 32 * 8 + sx + sy * 32];
}

// ScaleLayer

void ScaleLayer::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_height = stream.readUint16LE();
	stream.skip(2);
	uint32 valuesOffs = stream.readUint32LE();
	_values = dataStart + valuesOffs;
	debug(0, "ScaleLayer::load() _height: %d; valuesOffs: %08X",
		_height, valuesOffs);
}

int ScaleLayer::getScale(Common::Point pos) {
	pos.y = CLIP<int16>(pos.y, 0, _height - 1);
	return _values[pos.y];
}

// RegionLayer

void RegionLayer::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_unk = stream.readUint32LE();
	uint32 regionSequenceIdsOffs = stream.readUint32LE();
	_width = stream.readUint16LE();
	_height = stream.readUint16LE();
	uint32 mapOffs = stream.readUint32LE();
	uint32 valuesOffs = stream.readUint32LE();
	_regionSequenceIds = dataStart + regionSequenceIdsOffs;
	_map = dataStart + mapOffs;
	_values = dataStart + valuesOffs;
	_mapWidth = READ_LE_UINT16(_map + 0);
	_mapHeight = READ_LE_UINT16(_map + 2);
	_map += 8;
	debug(1, "RegionLayer::load() %d; regionSequenceIdsOffs: %08X; _width: %d; _height: %d; mapOffs: %08X; valuesOffs: %08X",
		_unk, regionSequenceIdsOffs, _width, _height, mapOffs, valuesOffs);
}

int RegionLayer::getRegionIndex(Common::Point pos) {
	pos.x = CLIP<int16>(pos.x, 0, _width - 1);
	pos.y = CLIP<int16>(pos.y, 0, _height - 1);
	const int16 tx = pos.x / 32, sx = pos.x % 32;
	const int16 ty = pos.y / 8, sy = pos.y % 8;
	uint16 mapIndex = READ_LE_UINT16(_map + 2 * (tx + ty * _mapWidth)) - 1;
	return _values[mapIndex * 32 * 8 + sx + sy * 32];
}

uint32 RegionLayer::getRegionSequenceId(int regionIndex) {
	return READ_LE_UINT32(_regionSequenceIds + 4 * regionIndex);
}

// Palette

void Palette::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_count = stream.readUint16LE();
	_unk = stream.readUint16LE();
	uint32 paletteOffs = stream.readUint32LE();
	_palette = dataStart + paletteOffs;
}

// BackgroundObject

void BackgroundObject::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_objectId = stream.readUint32LE();
	_flags = stream.readUint16LE();
	_priority = stream.readUint16LE();
	uint32 pointsConfigOffs = stream.readUint32LE();
	_pointsConfig = dataStart + pointsConfigOffs;
	debug(0, "BackgroundObject::load() _objectId: %08X; _flags: %04X; _priority: %d; pointsConfigOffs: %08X",
		_objectId, _flags, _priority, pointsConfigOffs);
}

// PathWalkPoints

void PathWalkPoints::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_points = new PointArray();
	uint count = stream.readUint32LE();
	uint32 pointsOffs = stream.readUint32LE();
	_points->reserve(count);
	stream.seek(pointsOffs);
	for (uint i = 0; i < count; ++i) {
		Common::Point pt;
		loadPoint(stream, pt);
		_points->push_back(pt);
	}
	debug(0, "PathWalkPoints::load() count: %d; pointsOffs: %08X",
		count, pointsOffs);
}

// PathWalkRects

void PathWalkRects::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_rects = new PathLines();
	uint count = stream.readUint32LE();
	uint32 rectsOffs = stream.readUint32LE();
	_rects->reserve(count);
	stream.seek(rectsOffs);
	for (uint i = 0; i < count; ++i) {
		PathLine rect;
		loadPoint(stream, rect.p0);
		loadPoint(stream, rect.p1);
		_rects->push_back(rect);
	}
	debug(0, "PathWalkRects::load() count: %d; rectsOffs: %08X",
		count, rectsOffs);
}

// BackgroundResource

BackgroundResource::BackgroundResource()
	: _bgInfos(nullptr), _scaleLayers(nullptr), _priorityLayers(nullptr), _regionLayers(nullptr),
	_regionSequences(nullptr), _backgroundObjects(nullptr), _pathWalkPoints(nullptr),
	_pathWalkRects(nullptr), _palettes(nullptr) {
}

BackgroundResource::~BackgroundResource() {
	delete[] _bgInfos;
	delete[] _scaleLayers;
	delete[] _priorityLayers;
	delete[] _regionLayers;
	delete[] _regionSequences;
	delete[] _backgroundObjects;
	delete[] _pathWalkPoints;
	delete[] _pathWalkRects;
	delete[] _palettes;
}

void BackgroundResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);

	stream.seek(8);
	_paletteIndex = stream.readUint16LE();

	// Load background pixels
	stream.seek(0x0A);
	_bgInfosCount = stream.readUint16LE();
	_bgInfos = new BgInfo[_bgInfosCount];
	stream.seek(0x20);
	uint32 bgInfosOffs = stream.readUint32LE();
	for (uint i = 0; i < _bgInfosCount; ++i) {
		stream.seek(bgInfosOffs + i * 0x1C);
		_bgInfos[i].load(data, stream);
	}

	// Load scale layers
	stream.seek(0x10);
	_scaleLayersCount = stream.readUint16LE();
	_scaleLayers = new ScaleLayer[_scaleLayersCount];
	stream.seek(0x2C);
	uint32 scaleLayersOffs = stream.readUint32LE();
	debug(0, "_scaleLayersCount: %d", _scaleLayersCount);
	for (uint i = 0; i < _scaleLayersCount; ++i) {
		stream.seek(scaleLayersOffs + i * 8);
		_scaleLayers[i].load(data, stream);
	}

	// Load priority layers
	stream.seek(0x14);
	_priorityLayersCount = stream.readUint16LE();
	_priorityLayers = new PriorityLayer[_priorityLayersCount];
	stream.seek(0x34);
	uint32 priorityLayersOffs = stream.readUint32LE();
	debug(1, "_priorityLayersCount: %d", _priorityLayersCount);
	for (uint i = 0; i < _priorityLayersCount; ++i) {
		stream.seek(priorityLayersOffs + i * 12);
		_priorityLayers[i].load(data, stream);
	}

	// Load region layers
	stream.seek(0x16);
	_regionLayersCount = stream.readUint16LE();
	_regionLayers = new RegionLayer[_regionLayersCount];
	stream.seek(0x38);
	uint32 regionLayersOffs = stream.readUint32LE();
	debug(1, "_regionLayersCount: %d", _regionLayersCount);
	for (uint i = 0; i < _regionLayersCount; ++i) {
		stream.seek(regionLayersOffs + i * 20);
		_regionLayers[i].load(data, stream);
	}

	// Load region sequences
	stream.seek(0x1E);
	_regionSequencesCount = stream.readUint16LE();
	_regionSequences = new Sequence[_regionSequencesCount];
	stream.seek(0x48);
	uint32 regionSequencesOffs = stream.readUint32LE();
	stream.seek(regionSequencesOffs);
	for (uint i = 0; i < _regionSequencesCount; ++i) {
		_regionSequences[i].load(data, stream);
	}

	// Load background objects
	stream.seek(0x1C);
	_backgroundObjectsCount = stream.readUint16LE();
	_backgroundObjects = new BackgroundObject[_backgroundObjectsCount];
	stream.seek(0x44);
	uint32 backgroundObjectsOffs = stream.readUint32LE();
	debug(0, "_backgroundObjectsCount: %d", _backgroundObjectsCount);
	for (uint i = 0; i < _backgroundObjectsCount; ++i) {
		stream.seek(backgroundObjectsOffs + i * 12);
		_backgroundObjects[i].load(data, stream);
	}

	// Load path walk points
	stream.seek(0x0E);
	_pathWalkPointsCount = stream.readUint16LE();
	debug(1, "_pathWalkPointsCount: %d", _pathWalkPointsCount);
	_pathWalkPoints = new PathWalkPoints[_pathWalkPointsCount];
	stream.seek(0x28);
	uint32 pathWalkPointsOffs = stream.readUint32LE();
	for (uint i = 0; i < _pathWalkPointsCount; ++i) {
		stream.seek(pathWalkPointsOffs + i * 8);
		_pathWalkPoints[i].load(data, stream);
	}

	// Load path walk rects
	stream.seek(0x12);
	_pathWalkRectsCount = stream.readUint16LE();
	debug(1, "_pathWalkRectsCount: %d", _pathWalkRectsCount);
	_pathWalkRects = new PathWalkRects[_pathWalkRectsCount];
	stream.seek(0x30);
	uint32 pathWalkRectsOffs = stream.readUint32LE();
	for (uint i = 0; i < _pathWalkRectsCount; ++i) {
		stream.seek(pathWalkRectsOffs + i * 8);
		_pathWalkRects[i].load(data, stream);
	}

	// Load named points
	stream.seek(0xC);
	uint namedPointsCount = stream.readUint16LE();
	stream.seek(0x24);
	uint32 namedPointsOffs = stream.readUint32LE();
	stream.seek(namedPointsOffs);
	_namedPoints.load(namedPointsCount, stream);

	// Load palettes
	stream.seek(0x18);
	_palettesCount = stream.readUint16LE();
	_palettes = new Palette[_palettesCount];
	stream.seek(0x3C);
	uint32 palettesOffs = stream.readUint32LE();
	debug(0, "_palettesCount: %d", _palettesCount);
	for (uint i = 0; i < _palettesCount; ++i) {
		stream.seek(palettesOffs + i * 8);
		_palettes[i].load(data, stream);
	}

}

int BackgroundResource::findMasterBgIndex() {
	int index = 1;
	while (!(_bgInfos[index - 1]._flags & 1)) { // TODO check if this is correct
		++index;
	}
	return index;
}

PriorityLayer *BackgroundResource::getPriorityLayer(uint index) {
	return &_priorityLayers[index];
}

ScaleLayer *BackgroundResource::getScaleLayer(uint index) {
	return &_scaleLayers[index];
}

RegionLayer *BackgroundResource::getRegionLayer(uint index) {
	return &_regionLayers[index];
}

PathWalkPoints *BackgroundResource::getPathWalkPoints(uint index) {
	return &_pathWalkPoints[index];
}

PathWalkRects *BackgroundResource::getPathWalkRects(uint index) {
	return &_pathWalkRects[index];
}

Palette *BackgroundResource::getPalette(uint index) {
	return &_palettes[index];
}

bool BackgroundResource::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	return _namedPoints.findNamedPoint(namedPointId, pt);
}

// BackgroundInstance

BackgroundInstance::BackgroundInstance(IllusionsEngine *vm)
	: _vm(vm), _sceneId(0), _pauseCtr(0), _bgRes(nullptr), _savedPalette(nullptr) {
}

BackgroundInstance::~BackgroundInstance() {
	delete[] _savedPalette;
}

void BackgroundInstance::load(Resource *resource) {
	debug(1, "BackgroundResourceLoader::load() Loading background %08X from %s...", resource->_resId, resource->_filename.c_str());

	BackgroundResource *backgroundResource = new BackgroundResource();
	backgroundResource->load(resource->_data, resource->_dataSize);

	_bgRes = backgroundResource;
	_sceneId = resource->_sceneId;
	initSurface();

	// Insert background objects
	for (uint i = 0; i < backgroundResource->_backgroundObjectsCount; ++i) {
		_vm->_controls->placeBackgroundObject(&backgroundResource->_backgroundObjects[i]);
	}

	registerResources();

	_vm->clearFader();

	int index = _bgRes->findMasterBgIndex();
	_vm->_camera->set(_bgRes->_bgInfos[index - 1]._panPoint, _bgRes->_bgInfos[index - 1]._surfInfo._dimensions);

	if (_bgRes->_palettesCount > 0) {
		Palette *palette = _bgRes->getPalette(_bgRes->_paletteIndex - 1);
		_vm->_screenPalette->setPalette(palette->_palette, 1, palette->_count);
	}

}

void BackgroundInstance::unload() {
	debug(1, "BackgroundInstance::unload()");
	freeSurface();
	unregisterResources();
	delete _bgRes;
	_vm->_backgroundInstances->removeBackgroundInstance(this);
	_vm->setDefaultTextCoords();
}

void BackgroundInstance::pause() {
	++_pauseCtr;
	if (_pauseCtr <= 1) {
		unregisterResources();
		_vm->setDefaultTextCoords();
		_vm->_camera->getActiveState(_savedCameraState);
		_savedPalette = new byte[1024];
		_vm->_screenPalette->getPalette(_savedPalette);
		freeSurface();
	}
}

void BackgroundInstance::unpause() {
	--_pauseCtr;
	if (_pauseCtr <= 0) {
		registerResources();
		initSurface();
		_vm->_screenPalette->setPalette(_savedPalette, 1, 256);
		delete[] _savedPalette;
		_savedPalette = nullptr;
		_vm->clearFader();
		_vm->_camera->setActiveState(_savedCameraState);
		_vm->_backgroundInstances->refreshPan();
	}
}

void BackgroundInstance::registerResources() {
	for (uint i = 0; i < _bgRes->_regionSequencesCount; ++i) {
		Sequence *sequence = &_bgRes->_regionSequences[i];
		_vm->_dict->addSequence(sequence->_sequenceId, sequence);
	}
}

void BackgroundInstance::unregisterResources() {
	for (uint i = 0; i < _bgRes->_regionSequencesCount; ++i) {
		Sequence *sequence = &_bgRes->_regionSequences[i];
		_vm->_dict->removeSequence(sequence->_sequenceId);
	}
}

void BackgroundInstance::initSurface() {
	for (uint i = 0; i < kMaxBackgroundItemSurfaces; ++i) {
		_surfaces[i] = nullptr;
	}
	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i) {
		BgInfo *bgInfo = &_bgRes->_bgInfos[i];
		_panPoints[i] = bgInfo->_panPoint;
		_surfaces[i] = _vm->_screen->allocSurface(bgInfo->_surfInfo);
		drawTiles(_surfaces[i], bgInfo->_tileMap, bgInfo->_tilePixels);
#if 0
		if (_bgRes->_pathWalkRectsCount > 0) {
			PathLines *pl = _bgRes->_pathWalkRects->_rects;
			for (int j=0; j < pl->size(); j++) {
				PathLine pathLine = (*pl)[j];
				debug(0, "walk path rect line[%d]. (%d,%d)->(%d,%d)", j, pathLine.p0.x, pathLine.p0.y, pathLine.p1.x, pathLine.p1.y);
				_surfaces[i]->drawLine(pathLine.p0.x, pathLine.p0.y, pathLine.p1.x, pathLine.p1.y, 5);
			}
		}
#endif
	}
}

void BackgroundInstance::freeSurface() {
	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i) {
		if (_surfaces[i]) {
			_surfaces[i]->free();
			delete _surfaces[i];
			_surfaces[i] = nullptr;
		}
	}
}

void BackgroundInstance::drawTiles(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
	switch (_vm->getGameId()) {
	case kGameIdDuckman:
		drawTiles8(surface, tileMap, tilePixels);
		break;
	case kGameIdBBDOU:
		drawTiles16(surface, tileMap, tilePixels);
		break;
	default:
		break;
	}
}

void BackgroundInstance::drawTiles8(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
	const int kTileWidth = 32;
	const int kTileHeight = 8;
	const int kTileSize = kTileWidth * kTileHeight;
	uint tileMapIndex = 0;
	for (int tileY = 0; tileY < tileMap._height; ++tileY) {
		int tileDestY = tileY * kTileHeight;
		int tileDestH = MIN(kTileHeight, surface->h - tileDestY);
		for (int tileX = 0; tileX < tileMap._width; ++tileX) {
			int tileDestX = tileX * kTileWidth;
			int tileDestW = MIN(kTileWidth, surface->w - tileDestX);
			uint16 tileIndex = READ_LE_UINT16(tileMap._map + 2 * tileMapIndex);
			++tileMapIndex;
			byte *src = tilePixels + (tileIndex - 1) * kTileSize;
			byte *dst = (byte*)surface->getBasePtr(tileDestX, tileDestY);
			for (int h = 0; h < tileDestH; ++h) {
				memcpy(dst, src, tileDestW);
				dst += surface->pitch;
				src += kTileWidth;
			}
		}
	}
}

void BackgroundInstance::drawTiles16(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
	const int kTileWidth = 32;
	const int kTileHeight = 8;
	const int kTileSize = kTileWidth * kTileHeight * 2;
	uint tileMapIndex = 0;
	for (int tileY = 0; tileY < tileMap._height; ++tileY) {
		int tileDestY = tileY * kTileHeight;
		int tileDestH = MIN(kTileHeight, surface->h - tileDestY);
		for (int tileX = 0; tileX < tileMap._width; ++tileX) {
			int tileDestX = tileX * kTileWidth;
			int tileDestW = MIN(kTileWidth, surface->w - tileDestX);
			uint16 tileIndex = READ_LE_UINT16(tileMap._map + 2 * tileMapIndex);
			++tileMapIndex;
			byte *src = tilePixels + (tileIndex - 1) * kTileSize;
			byte *dst = (byte*)surface->getBasePtr(tileDestX, tileDestY);
			for (int h = 0; h < tileDestH; ++h) {
				for (int w = 0; w < tileDestW; ++w) {
					uint16 pixel = READ_LE_UINT16(src + w * 2);
					WRITE_LE_UINT16(dst + w * 2, pixel);
				}
				dst += surface->pitch;
				src += kTileWidth * 2;
			}
		}
	}
}

// BackgroundInstanceList

BackgroundInstanceList::BackgroundInstanceList(IllusionsEngine *vm)
	: _vm(vm) {
}

BackgroundInstanceList::~BackgroundInstanceList() {
}

BackgroundInstance *BackgroundInstanceList::createBackgroundInstance(Resource *resource) {
	BackgroundInstance *backgroundInstance = new BackgroundInstance(_vm);
	backgroundInstance->load(resource);
	_items.push_back(backgroundInstance);
	return backgroundInstance;
}

void BackgroundInstanceList::removeBackgroundInstance(BackgroundInstance *backgroundInstance) {
	_items.remove(backgroundInstance);
}

void BackgroundInstanceList::pauseBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_sceneId == sceneId)
			(*it)->pause();
	}
}

void BackgroundInstanceList::unpauseBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_sceneId == sceneId)
			(*it)->unpause();
	}
}

BackgroundInstance *BackgroundInstanceList::findActiveBackgroundInstance() {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_pauseCtr == 0)
			return (*it);
	}
	return nullptr;
}

BackgroundInstance *BackgroundInstanceList::findBackgroundByResource(BackgroundResource *backgroundResource) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_bgRes == backgroundResource)
			return (*it);
	}
	return nullptr;
}

BackgroundResource *BackgroundInstanceList::getActiveBgResource() {
	BackgroundInstance *background = findActiveBackgroundInstance();
	if (background)
		return background->_bgRes;
	return nullptr;
}

WidthHeight BackgroundInstanceList::getMasterBgDimensions() {
	BackgroundInstance *backgroundInstance = findActiveBackgroundInstance();
	int16 index = backgroundInstance->_bgRes->findMasterBgIndex();
	return backgroundInstance->_bgRes->_bgInfos[index - 1]._surfInfo._dimensions;
}

void BackgroundInstanceList::refreshPan() {
	BackgroundInstance *backgroundInstance = findActiveBackgroundInstance();
	if (backgroundInstance) {
		WidthHeight dimensions = getMasterBgDimensions();
		_vm->_camera->refreshPan(backgroundInstance, dimensions);
	}
}

bool BackgroundInstanceList::findActiveBackgroundNamedPoint(uint32 namedPointId, Common::Point &pt) {
	BackgroundResource *backgroundResource = getActiveBgResource();
	return backgroundResource ? backgroundResource->findNamedPoint(namedPointId, pt) : false;
}

} // End of namespace Illusions
