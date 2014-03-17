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
#include "illusions/camera.h"
#include "illusions/screen.h"
#include "common/str.h"

namespace Illusions {

// BackgroundResourceLoader

void BackgroundResourceLoader::load(Resource *resource) {
	// TODO
	debug("BackgroundResourceLoader::load() Loading background %08X from %s...", resource->_resId, resource->_filename.c_str());

	BackgroundResource *backgroundResource = new BackgroundResource();
	backgroundResource->load(resource->_data, resource->_dataSize);

	BackgroundItem *backgroundItem = _vm->_backgroundItems->allocBackgroundItem();
	backgroundItem->_bgRes = backgroundResource;
	backgroundItem->_tag = resource->_tag;
	
	backgroundItem->initSurface();
	
	// TODO Insert objects from item44s
	// TODO Insert IDs from item48s

	// TODO camera_fadeClear();
	int index = backgroundItem->_bgRes->findMasterBgIndex();
	_vm->_camera->set(backgroundItem->_bgRes->_bgInfos[index - 1]._panPoint, backgroundItem->_bgRes->_bgInfos[index - 1]._surfInfo._dimensions);
	
	// NOTE Skipped palette loading (not used in BBDOU)
}

void BackgroundResourceLoader::unload(Resource *resource) {
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
	
	debug("TileMap::load() _width: %d; _height: %d",
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
	
	debug("BgInfo::load() _flags: %08X; _priorityBase: %d; tileMapOffs: %08X; tilePixelsOffs: %08X",
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
	
	debug("PriorityLayer::load() _width: %d; _height: %d; mapOffs: %08X; valuesOffs: %08X; _mapWidth: %d; _mapHeight: %d",
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
	
	debug("ScaleLayer::load() _height: %d; valuesOffs: %08X",
		_height, valuesOffs);
}

int ScaleLayer::getScale(Common::Point pos) {
	pos.y = CLIP<int16>(pos.y, 0, _height - 1);
	return _values[pos.y];
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
	debug("_scaleLayersCount: %d", _scaleLayersCount);
	for (uint i = 0; i < _scaleLayersCount; ++i) {
		stream.seek(scaleLayersOffs + i * 8);
		_scaleLayers[i].load(data, stream);
	}

}

int BackgroundResource::findMasterBgIndex() {
	int index = 1;
	while (!_bgInfos[index - 1]._flags & 1)
		++index;
	return index;
}

// BackgroundItem

BackgroundItem::BackgroundItem(IllusionsEngine *vm) : _vm(vm), _tag(0), _pauseCtr(0), _bgRes(0) {
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
	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i) {
		_surfaces[i]->free();
		delete _surfaces[i];
		_surfaces[i] = 0;
	}
}

void BackgroundItem::drawTiles(Graphics::Surface *surface, TileMap &tileMap, byte *tilePixels) {
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
		/* TODO
		for (uint i = 0; i < _bgRes->_item48sCount; ++i)
			krndictRemoveID(_bgRes->_item48s[i].id);
		*/
		// TODO _vm->setDefPointDimensions1();
		// TODO memcpy(&_savedCamera, &_vm->camera, sizeof(backgroundItem->savedCamera));
		/* Unused
		_savedPalette = malloc(1024);
		savePalette(_savedPalette);
		*/
		freeSurface();
	}
}

void BackgroundItem::unpause() {
	// TODO
	--_pauseCtr;
	if (_pauseCtr <= 0) {
		/* TODO
		for (uint i = 0; i < _bgRes->_item48sCount; ++i)
			krndictAddID(_bgRes->_item48s[i].id, _bgRes->_item48s[i]);
		*/
		initSurface();
		/* Unused
		restorePalette(_savedPalette, 1, 256);
		free(_savedPalette);
		_savedPalette = 0;
		*/
		// TODO _vm->_screen->_fadeClear();
		// TODO memcpy(&_vm->camera, &_savedCamera, sizeof(SavedCamera));
		/* TODO
		currTime = krnxxxGetCurrentTime();
		_vm->_camera.panStartTime = currTime;
		*/
		_vm->_backgroundItems->refreshPan();
	}
}

void BackgroundItem::refreshPan(WidthHeight &dimensions) {
	Common::Point screenOffs = _vm->_camera->getScreenOffset();
	int x = dimensions._width - 640;
	int y = dimensions._height - 480;
	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i) {
		const BgInfo &bgInfo = _bgRes->_bgInfos[i];
		if (bgInfo._flags & 1) {
			_panPoints[i] = screenOffs;
		} else {
			Common::Point newOffs(0, 0);
			if (x > 0 && bgInfo._surfInfo._dimensions._width - 640 > 0)
				newOffs.x = screenOffs.x * (bgInfo._surfInfo._dimensions._width - 640) / x;
			if (y > 0 && bgInfo._surfInfo._dimensions._height - 480 > 0)
				newOffs.y = screenOffs.y * (bgInfo._surfInfo._dimensions._height - 480) / y;
			_panPoints[i] = newOffs;
		}
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
		backgroundItem->refreshPan(dimensions);
	}
}

BackgroundItem *BackgroundItems::debugFirst() {
	return *(_items.begin());
}

} // End of namespace Illusions
