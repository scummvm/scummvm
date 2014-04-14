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

#include "illusions/illusions.h"
#include "illusions/backgroundresource.h"
#include "illusions/actor.h"
#include "illusions/actorresource.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/screen.h"
#include "common/str.h"

namespace Illusions {

// BackgroundResourceLoader

void BackgroundResourceLoader::load(Resource *resource) {
	debug("BackgroundResourceLoader::load() Loading background %08X from %s...", resource->_resId, resource->_filename.c_str());

	BackgroundResource *backgroundResource = new BackgroundResource();
	backgroundResource->load(resource->_data, resource->_dataSize);
	resource->_refId = backgroundResource;

	// TODO Move to BackgroundItems
	BackgroundItem *backgroundItem = _vm->_backgroundItems->allocBackgroundItem();
	backgroundItem->_bgRes = backgroundResource;
	backgroundItem->_tag = resource->_tag;
	backgroundItem->initSurface();
	
	// Insert background objects
	for (uint i = 0; i < backgroundResource->_backgroundObjectsCount; ++i)
		_vm->_controls->placeBackgroundObject(&backgroundResource->_backgroundObjects[i]);

	// Insert region sequences
	for (uint i = 0; i < backgroundResource->_regionSequencesCount; ++i) {
		Sequence *sequence = &backgroundResource->_regionSequences[i];
		_vm->_dict->addSequence(sequence->_sequenceId, sequence);
	}

	// TODO camera_fadeClear();
	int index = backgroundItem->_bgRes->findMasterBgIndex();
	_vm->_camera->set(backgroundItem->_bgRes->_bgInfos[index - 1]._panPoint, backgroundItem->_bgRes->_bgInfos[index - 1]._surfInfo._dimensions);

	if (backgroundItem->_bgRes->_palettesCount > 0) {
		Palette *palette = &backgroundItem->_bgRes->_palettes[backgroundItem->_bgRes->_paletteIndex - 1];
		_vm->_screen->setPalette(palette->_palette, 1, palette->_count);
	}
	
}

void BackgroundResourceLoader::unload(Resource *resource) {
	debug("BackgroundResourceLoader::unload() Unloading background %08X...", resource->_resId);
	// TODO Move to BackgroundItems
	BackgroundItem *backgroundItem = _vm->_backgroundItems->findBackgroundByResource((BackgroundResource*)resource->_refId);
	backgroundItem->freeSurface();
	for (uint i = 0; i < backgroundItem->_bgRes->_regionSequencesCount; ++i) {
		Sequence *sequence = &backgroundItem->_bgRes->_regionSequences[i];
		_vm->_dict->removeSequence(sequence->_sequenceId);
	}
	delete backgroundItem->_bgRes;
	_vm->_backgroundItems->freeBackgroundItem(backgroundItem);
	_vm->setDefaultTextCoords();
	debug("BackgroundResourceLoader::unload() Unloading background %08X OK", resource->_resId);
}

void BackgroundResourceLoader::buildFilename(Resource *resource) {
	resource->_filename = Common::String::format("%08X.bg", resource->_resId);
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
	stream.skip(2); // Unknown
	_priorityBase = stream.readSint16LE();
	_surfInfo.load(stream);
	loadPoint(stream, _panPoint);
	uint32 tileMapOffs = stream.readUint32LE();
	uint32 tilePixelsOffs = stream.readUint32LE();
	stream.seek(tileMapOffs);
	_tileMap.load(dataStart, stream);
	_tilePixels = dataStart + tilePixelsOffs;
	
	debug(0, "BgInfo::load() _flags: %08X; _priorityBase: %d; tileMapOffs: %08X; tilePixelsOffs: %08X",
		_flags, _priorityBase, tileMapOffs, tilePixelsOffs);
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

	debug("RegionLayer::load() %d; regionSequenceIdsOffs: %08X; _width: %d; _height: %d; mapOffs: %08X; valuesOffs: %08X",
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

// BackgroundResource

BackgroundResource::BackgroundResource() {
}

BackgroundResource::~BackgroundResource() {
	// TODO Free stuff
}

void BackgroundResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);
	// TODO A lot
	
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
	debug(0, "_priorityLayersCount: %d", _priorityLayersCount);
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
	debug("_regionLayersCount: %d", _regionLayersCount);
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
	for (uint i = 0; i < _regionSequencesCount; ++i)
		_regionSequences[i].load(data, stream);

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
	while (!_bgInfos[index - 1]._flags & 1)
		++index;
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

bool BackgroundResource::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	return _namedPoints.findNamedPoint(namedPointId, pt);
}

// BackgroundItem

BackgroundItem::BackgroundItem(IllusionsEngine *vm) : _vm(vm), _tag(0), _pauseCtr(0), _bgRes(0), _savedPalette(0) {
}

BackgroundItem::~BackgroundItem() {
}

void BackgroundItem::initSurface() {
	for (uint i = 0; i < kMaxBackgroundItemSurfaces; ++i)
		_surfaces[i] = 0;
	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i) {
		BgInfo *bgInfo = &_bgRes->_bgInfos[i];
		_panPoints[i] = bgInfo->_panPoint;
		_surfaces[i] = _vm->_screen->allocSurface(bgInfo->_surfInfo);
		drawTiles(_surfaces[i], bgInfo->_tileMap, bgInfo->_tilePixels);
	}
}

void BackgroundItem::freeSurface() {
	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i)
		if (_surfaces[i]) {
			_surfaces[i]->free();
			delete _surfaces[i];
			_surfaces[i] = 0;
		}
}

void BackgroundItem::drawTiles(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
	switch (_vm->getGameId()) {
	case kGameIdDuckman:
		drawTiles8(surface, tileMap, tilePixels);
		break;
	case kGameIdBBDOU:
		drawTiles16(surface, tileMap, tilePixels);
		break;
	}
}

void BackgroundItem::drawTiles8(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
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

void BackgroundItem::drawTiles16(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
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

void BackgroundItem::pause() {
	// TODO
	++_pauseCtr;
	if (_pauseCtr <= 1) {
		for (uint i = 0; i < _bgRes->_regionSequencesCount; ++i) {
			Sequence *sequence = &_bgRes->_regionSequences[i];
			_vm->_dict->removeSequence(sequence->_sequenceId);
		}
		_vm->setDefaultTextCoords();
		_vm->_camera->getActiveState(_savedCameraState);
		_savedPalette = new byte[1024];
		_vm->_screen->getPalette(_savedPalette);
		freeSurface();
	}
}

void BackgroundItem::unpause() {
	// TODO
	--_pauseCtr;
	if (_pauseCtr <= 0) {
		for (uint i = 0; i < _bgRes->_regionSequencesCount; ++i) {
			Sequence *sequence = &_bgRes->_regionSequences[i];
			_vm->_dict->addSequence(sequence->_sequenceId, sequence);
		}
		initSurface();
		_vm->_screen->setPalette(_savedPalette, 1, 256);
		delete[] _savedPalette;
		_savedPalette = 0;
		// TODO _vm->_screen->_fadeClear();
		_vm->_camera->setActiveState(_savedCameraState);
		_vm->_backgroundItems->refreshPan();
	}
}

// BackgroundItems

BackgroundItems::BackgroundItems(IllusionsEngine *vm)
	: _vm(vm) {
}

BackgroundItems::~BackgroundItems() {
}

BackgroundItem *BackgroundItems::allocBackgroundItem() {
	BackgroundItem *backgroundItem = new BackgroundItem(_vm);
	_items.push_back(backgroundItem);
	return backgroundItem;
}

void BackgroundItems::freeBackgroundItem(BackgroundItem *backgroundItem) {
	_items.remove(backgroundItem);
	delete backgroundItem;
}

void BackgroundItems::pauseByTag(uint32 tag) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_tag == tag)
			(*it)->pause();
}

void BackgroundItems::unpauseByTag(uint32 tag) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_tag == tag)
			(*it)->unpause();
}

BackgroundItem *BackgroundItems::findActiveBackground() {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_pauseCtr == 0)
			return (*it);
	return 0;
}

BackgroundItem *BackgroundItems::findBackgroundByResource(BackgroundResource *backgroundResource) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_bgRes == backgroundResource)
			return (*it);
	return 0;
}

BackgroundResource *BackgroundItems::getActiveBgResource() {
	BackgroundItem *background = findActiveBackground();
	if (background)
		return background->_bgRes;
	return 0;
}

WidthHeight BackgroundItems::getMasterBgDimensions() {
	BackgroundItem *backgroundItem = findActiveBackground();
	int16 index = backgroundItem->_bgRes->findMasterBgIndex();
	return backgroundItem->_bgRes->_bgInfos[index - 1]._surfInfo._dimensions;
}

void BackgroundItems::refreshPan() {
	BackgroundItem *backgroundItem = findActiveBackground();
	if (backgroundItem) {
		WidthHeight dimensions = getMasterBgDimensions();
		_vm->_camera->refreshPan(backgroundItem, dimensions);
	}
}

bool BackgroundItems::findActiveBackgroundNamedPoint(uint32 namedPointId, Common::Point &pt) {
	BackgroundResource *backgroundResource = getActiveBgResource();
	return backgroundResource ? backgroundResource->findNamedPoint(namedPointId, pt) : false;
}

} // End of namespace Illusions
