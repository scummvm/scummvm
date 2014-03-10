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
#include "common/str.h"

namespace Illusions {

// BackgroundResourceLoader

void BackgroundResourceLoader::load(Resource *resource) {
	// TODO
	debug("BackgroundResourceLoader::load() Loading background %08X from %s...", resource->_resId, resource->_filename.c_str());

	BackgroundResource *backgroundResource = new BackgroundResource();
	backgroundResource->load(resource->_data, resource->_dataSize);

	BackgroundItem *backgroundItem = _vm->allocBackgroundItem();
	backgroundItem->_bgRes = backgroundResource;
	backgroundItem->_tag = resource->_tag;
	
	backgroundItem->initSurface();
	
	// TODO Insert objects from item44s
	// TODO Insert IDs from item48s

	// TODO camera_fadeClear();
	// TODO bgInfo = &bgResourceb->bgInfos[(unsigned __int16)BgResource_findMasterBgIndex(bgResourceb)];
	// TODO camera_set(bgInfo[-1].panPoint, bgInfo[-1].surfInfo.dimensions);
	
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

// BackgroundItem

BackgroundItem::BackgroundItem(IllusionsEngine *vm) : _vm(vm), _tag(0), _pauseCtr(0), _bgRes(0) {
}

void BackgroundItem::initSurface() {

	for (uint i = 0; i < kMaxBackgroundItemSurfaces; ++i)
		_surfaces[i] = 0;

	for (uint i = 0; i < _bgRes->_bgInfosCount; ++i) {
		BgInfo *bgInfo = &_bgRes->_bgInfos[i];
		_panPoints[i] = bgInfo->_panPoint;
		_surfaces[i] = _vm->allocSurface(bgInfo->_surfInfo);
		drawTiles(_surfaces[i], bgInfo->_tileMap, bgInfo->_tilePixels);
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

	/*	
	Common::DumpFile d;
	d.open("dump.000");
	d.write(surface->getPixels(), surface->h * surface->pitch);
	d.close();
	*/

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

}

int BackgroundResource::findMasterBgIndex() {
	int index = 1;
	while (!_bgInfos[index - 1]._flags & 1)
		++index;
	return index;
}

} // End of namespace Illusions
