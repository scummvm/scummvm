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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "common/memstream.h"
#include "neverhood/resource.h"
#include "neverhood/resourceman.h"

namespace Neverhood {

// SpriteResource

SpriteResource::SpriteResource(NeverhoodEngine *vm)
	 : _vm(vm), _pixels(NULL) {
}

SpriteResource::~SpriteResource() {
	unload();
}

void SpriteResource::draw(Graphics::Surface *destSurface, bool flipX, bool flipY) {
	if (_pixels) {
		byte *dest = (byte*)destSurface->getPixels();
		const int destPitch = destSurface->pitch;
		if (_rle)
			unpackSpriteRle(_pixels, _dimensions.width, _dimensions.height, dest, destPitch, flipX, flipY);
		else
			unpackSpriteNormal(_pixels, _dimensions.width, _dimensions.height, dest, destPitch, flipX, flipY);
	}
}

bool SpriteResource::load(uint32 fileHash, bool doLoadPosition) {
	debug(2, "SpriteResource::load(%08X)", fileHash);
	unload();
	_vm->_res->queryResource(fileHash, _resourceHandle);
	if (_resourceHandle.isValid() && _resourceHandle.type() == kResTypeBitmap) {
		_vm->_res->loadResource(_resourceHandle, _vm->applyResourceFixes());
		const byte *spriteData = _resourceHandle.data();
		NPoint *position = doLoadPosition ? &_position : NULL;
		parseBitmapResource(spriteData, &_rle, &_dimensions, position, NULL, &_pixels);
	}
	return _pixels != NULL;
}

void SpriteResource::unload() {
	_vm->_res->unloadResource(_resourceHandle);
	_pixels = NULL;
	_rle = false;
}

// PaletteResource

PaletteResource::PaletteResource(NeverhoodEngine *vm)
	: _vm(vm), _palette(NULL) {
}

PaletteResource::~PaletteResource() {
	unload();
}

bool PaletteResource::load(uint32 fileHash) {
	debug(2, "PaletteResource::load(%08X)", fileHash);
	unload();
	_vm->_res->queryResource(fileHash, _resourceHandle);
	if (_resourceHandle.isValid() &&
		(_resourceHandle.type() == kResTypeBitmap || _resourceHandle.type() == kResTypePalette)) {
		_vm->_res->loadResource(_resourceHandle, _vm->applyResourceFixes());
		_palette = _resourceHandle.data();
		// Check if the palette is stored in a bitmap
		if (_resourceHandle.type() == kResTypeBitmap)
			parseBitmapResource(_palette, NULL, NULL, NULL, &_palette, NULL);

	}
	return _palette != NULL;
}

void PaletteResource::unload() {
	_vm->_res->unloadResource(_resourceHandle);
	_palette = NULL;
}

void PaletteResource::copyPalette(byte *destPalette) {
	if (_palette)
		memcpy(destPalette, _palette, 1024);
}

// AnimResource

AnimResource::AnimResource(NeverhoodEngine *vm)
	: _vm(vm), _width(0), _height(0), _currSpriteData(NULL), _fileHash(0), _paletteData(NULL),
	_spriteData(NULL), _replEnabled(false), _replOldColor(0), _replNewColor(0) {
}

AnimResource::~AnimResource() {
	unload();
}

void AnimResource::draw(uint frameIndex, Graphics::Surface *destSurface, bool flipX, bool flipY) {
	const AnimFrameInfo frameInfo = _frames[frameIndex];
	byte *dest = (byte*)destSurface->getPixels();
	const int destPitch = destSurface->pitch;
	_currSpriteData = _spriteData + frameInfo.spriteDataOffs;
	_width = frameInfo.drawOffset.width;
	_height = frameInfo.drawOffset.height;
	if (_replEnabled && _replOldColor != _replNewColor)
		unpackSpriteRle(_currSpriteData, _width, _height, dest, destPitch, flipX, flipY, _replOldColor, _replNewColor);
	else
		unpackSpriteRle(_currSpriteData, _width, _height, dest, destPitch, flipX, flipY);
}

bool AnimResource::load(uint32 fileHash) {
	debug(2, "AnimResource::load(%08X)", fileHash);

	if (fileHash == _fileHash)
		return true;

	unload();

	_vm->_res->queryResource(fileHash, _resourceHandle);
	if (!_resourceHandle.isValid() || _resourceHandle.type() != kResTypeAnimation)
		return false;

	const byte *resourceData, *animList, *frameList;
	uint16 animInfoStartOfs, animListIndex, animListCount;
	uint16 frameListStartOfs, frameCount;
	uint32 spriteDataOfs, paletteDataOfs;

	_vm->_res->loadResource(_resourceHandle, _vm->applyResourceFixes());
	resourceData = _resourceHandle.data();

	animListCount = READ_LE_UINT16(resourceData);
	animInfoStartOfs = READ_LE_UINT16(resourceData + 2);
	spriteDataOfs = READ_LE_UINT32(resourceData + 4);
	paletteDataOfs = READ_LE_UINT32(resourceData + 8);

	animList = resourceData + 12;
	for (animListIndex = 0; animListIndex < animListCount; animListIndex++) {
		debug(8, "hash: %08X", READ_LE_UINT32(animList));
		if (READ_LE_UINT32(animList) == fileHash)
			break;
		animList += 8;
	}

	if (animListIndex >= animListCount) {
		_vm->_res->unloadResource(_resourceHandle);
		return false;
	}

	_spriteData = resourceData + spriteDataOfs;
	if (paletteDataOfs > 0)
		_paletteData = resourceData + paletteDataOfs;

	frameCount = READ_LE_UINT16(animList + 4);
	frameListStartOfs = READ_LE_UINT16(animList + 6);

	debug(8, "frameCount = %d; frameListStartOfs = %04X; animInfoStartOfs = %04X", frameCount, frameListStartOfs, animInfoStartOfs);

	frameList = resourceData + animInfoStartOfs + frameListStartOfs;

	_frames.clear();
	_frames.reserve(frameCount);

	for (uint16 frameIndex = 0; frameIndex < frameCount; frameIndex++) {
		AnimFrameInfo frameInfo;
		frameInfo.frameHash = READ_LE_UINT32(frameList);
		frameInfo.counter = READ_LE_UINT16(frameList + 4);
		frameInfo.drawOffset.x = READ_LE_UINT16(frameList + 6);
		frameInfo.drawOffset.y = READ_LE_UINT16(frameList + 8);
		frameInfo.drawOffset.width = READ_LE_UINT16(frameList + 10);
		frameInfo.drawOffset.height = READ_LE_UINT16(frameList + 12);
		frameInfo.deltaX = READ_LE_UINT16(frameList + 14);
		frameInfo.deltaY = READ_LE_UINT16(frameList + 16);
		frameInfo.collisionBoundsOffset.x = READ_LE_UINT16(frameList + 18);
		frameInfo.collisionBoundsOffset.y = READ_LE_UINT16(frameList + 20);
		frameInfo.collisionBoundsOffset.width = READ_LE_UINT16(frameList + 22);
		frameInfo.collisionBoundsOffset.height = READ_LE_UINT16(frameList + 24);
		frameInfo.spriteDataOffs = READ_LE_UINT32(frameList + 28);
		debug(8, "frameHash = %08X; counter = %d; rect = (%d,%d,%d,%d); deltaX = %d; deltaY = %d; collisionBoundsOffset = (%d,%d,%d,%d); spriteDataOffs = %08X",
			frameInfo.frameHash, frameInfo.counter,
			frameInfo.drawOffset.x, frameInfo.drawOffset.y, frameInfo.drawOffset.width, frameInfo.drawOffset.height,
			frameInfo.deltaX, frameInfo.deltaY,
			frameInfo.collisionBoundsOffset.x, frameInfo.collisionBoundsOffset.y, frameInfo.collisionBoundsOffset.width, frameInfo.collisionBoundsOffset.height,
			frameInfo.spriteDataOffs);
		frameList += 32;
		_frames.push_back(frameInfo);
	}

	_fileHash = fileHash;

	return true;

}

void AnimResource::unload() {
	_vm->_res->unloadResource(_resourceHandle);
	_currSpriteData = NULL;
	_fileHash = 0;
	_paletteData = NULL;
	_spriteData = NULL;
	_replEnabled = true;
	_replOldColor = 0;
	_replNewColor = 0;
}

int16 AnimResource::getFrameIndex(uint32 frameHash) {
	int16 frameIndex = -1;
	for (uint i = 0; i < _frames.size(); i++)
		if (_frames[i].frameHash == frameHash) {
			frameIndex = (int16)i;
			break;
		}
	debug(2, "AnimResource::getFrameIndex(%08X) -> %d", frameHash, frameIndex);
	return frameIndex;
}

void AnimResource::setRepl(byte oldColor, byte newColor) {
	_replOldColor = oldColor;
	_replNewColor = newColor;
}

NDimensions AnimResource::loadSpriteDimensions(uint32 fileHash) {
	ResourceHandle resourceHandle;
	NDimensions dimensions;
	_vm->_res->queryResource(fileHash, resourceHandle);
	const byte *resDimensions = resourceHandle.extData();
	if (resDimensions) {
		dimensions.width = READ_LE_UINT16(resDimensions + 0);
		dimensions.height = READ_LE_UINT16(resDimensions + 2);
	} else {
		dimensions.width = 0;
		dimensions.height = 0;
	}
	return dimensions;
}

// MouseCursorResource

MouseCursorResource::MouseCursorResource(NeverhoodEngine *vm)
	: _cursorSprite(vm), _cursorNum(4), _currFileHash(0) {

	_rect.width = 32;
	_rect.height = 32;
}

void MouseCursorResource::load(uint32 fileHash) {
	if (_currFileHash != fileHash) {
		if (_cursorSprite.load(fileHash) && !_cursorSprite.isRle() &&
			_cursorSprite.getDimensions().width == 96 && _cursorSprite.getDimensions().height == 224) {
			_currFileHash = fileHash;
		} else {
			unload();
		}
	}
}

void MouseCursorResource::unload() {
	_cursorSprite.unload();
	_currFileHash = 0;
	_cursorNum = 4;
}

NDrawRect& MouseCursorResource::getRect() {
	static const NPoint kCursorHotSpots[] = {
		{-15, -5},
		{-17, -25},
		{-17, -30},
		{-14, -1},
		{-3, -7},
		{-30, -18},
		{-1, -18}
	};
	_rect.x = kCursorHotSpots[_cursorNum].x;
	_rect.y = kCursorHotSpots[_cursorNum].y;
	return _rect;
}

void MouseCursorResource::draw(int frameNum, Graphics::Surface *destSurface) {
	if (_cursorSprite.getPixels()) {
		const int sourcePitch = (_cursorSprite.getDimensions().width + 3) & 0xFFFC; // 4 byte alignment
		const int destPitch = destSurface->pitch;
		const byte *source = _cursorSprite.getPixels() + _cursorNum * (sourcePitch * 32) + frameNum * 32;
		byte *dest = (byte*)destSurface->getPixels();
		for (int16 yc = 0; yc < 32; yc++) {
			memcpy(dest, source, 32);
			source += sourcePitch;
			dest += destPitch;
		}
	}
}

// TextResource

TextResource::TextResource(NeverhoodEngine *vm)
	: _vm(vm), _textData(NULL), _count(0) {

}

TextResource::~TextResource() {
	unload();
}

void TextResource::load(uint32 fileHash) {
	debug(2, "TextResource::load(%08X)", fileHash);
	unload();
	_vm->_res->queryResource(fileHash, _resourceHandle);
	if (_resourceHandle.isValid() && _resourceHandle.type() == kResTypeText) {
		_vm->_res->loadResource(_resourceHandle, _vm->applyResourceFixes());
		_textData = _resourceHandle.data();
		_count = READ_LE_UINT32(_textData);
	}
}

void TextResource::unload() {
	_vm->_res->unloadResource(_resourceHandle);
	_textData = NULL;
	_count = 0;
}

const char *TextResource::getString(uint index, const char *&textEnd) {
	const char *textStart = (const char*)(_textData + 4 + _count * 4 + READ_LE_UINT32(_textData + (index + 1) * 4));
	textEnd = (const char*)(_textData + 4 + _count * 4 + READ_LE_UINT32(_textData + (index + 2) * 4));
	return textStart;
}

// DataResource

DataResource::DataResource(NeverhoodEngine *vm)
	: _vm(vm) {
}

DataResource::~DataResource() {
	unload();
}

void DataResource::load(uint32 fileHash) {
	if (_resourceHandle.fileHash() == fileHash)
		return;
	const byte *data = NULL;
	uint32 dataSize = 0;
	unload();
	_vm->_res->queryResource(fileHash, _resourceHandle);
	if (_resourceHandle.isValid() && _resourceHandle.type() == kResTypeData) {
		_vm->_res->loadResource(_resourceHandle, _vm->applyResourceFixes());
		data = _resourceHandle.data();
		dataSize = _resourceHandle.size();
	}
	if (data && dataSize) {
		Common::MemoryReadStream dataS(data, dataSize);
		uint itemCount = dataS.readUint16LE();
		uint32 itemStartOffs = 2 + itemCount * 8;
		debug(2, "itemCount = %d", itemCount);
		for (uint i = 0; i < itemCount; i++) {
			dataS.seek(2 + i * 8);
			DRDirectoryItem drDirectoryItem;
			drDirectoryItem.nameHash = dataS.readUint32LE();
			drDirectoryItem.offset = dataS.readUint16LE();
			drDirectoryItem.type = dataS.readUint16LE();
			debug(2, "%03d nameHash = %08X; offset = %04X; type = %d", i, drDirectoryItem.nameHash, drDirectoryItem.offset, drDirectoryItem.type);
			dataS.seek(itemStartOffs + drDirectoryItem.offset);
			switch (drDirectoryItem.type) {
			case 1:
				{
					debug(3, "NPoint");
					NPoint point;
					point.x = dataS.readUint16LE();
					point.y = dataS.readUint16LE();
					debug(3, "(%d, %d)", point.x, point.y);
					drDirectoryItem.offset = _points.size();
					_points.push_back(point);
					break;
				}
			case 2:
				{
					uint count = dataS.readUint16LE();
					NPointArray *pointArray = new NPointArray();
					debug(3, "NPointArray; count = %d", count);
					for (uint j = 0; j < count; j++) {
						NPoint point;
						point.x = dataS.readUint16LE();
						point.y = dataS.readUint16LE();
						debug(3, "(%d, %d)", point.x, point.y);
						pointArray->push_back(point);
					}
					drDirectoryItem.offset = _pointArrays.size();
					_pointArrays.push_back(pointArray);
					break;
				}
			case 3:
				{
					uint count = dataS.readUint16LE();
					HitRectList *hitRectList = new HitRectList();
					debug(3, "HitRectList; count = %d", count);
					for (uint j = 0; j < count; j++) {
						HitRect hitRect;
						hitRect.rect.x1 = dataS.readUint16LE();
						hitRect.rect.y1 = dataS.readUint16LE();
						hitRect.rect.x2 = dataS.readUint16LE();
						hitRect.rect.y2 = dataS.readUint16LE();
						hitRect.type = dataS.readUint16LE() + 0x5001;
						debug(3, "(%d, %d, %d, %d) -> %04d", hitRect.rect.x1, hitRect.rect.y1, hitRect.rect.x2, hitRect.rect.y2, hitRect.type);
						hitRectList->push_back(hitRect);
					}
					drDirectoryItem.offset = _hitRectLists.size();
					_hitRectLists.push_back(hitRectList);
					break;
				}
			case 4:
				{
					uint count = dataS.readUint16LE();
					MessageList *messageList = new MessageList();
					debug(3, "MessageList; count = %d", count);
					for (uint j = 0; j < count; j++) {
						MessageItem messageItem;
						messageItem.messageNum = dataS.readUint32LE();
						messageItem.messageValue = dataS.readUint32LE();
						debug(3, "(%08X, %08X)", messageItem.messageNum, messageItem.messageValue);
						messageList->push_back(messageItem);
					}
					drDirectoryItem.offset = _messageLists.size();
					_messageLists.push_back(messageList);
					break;
				}
			case 5:
				{
					uint count = dataS.readUint16LE();
					DRSubRectList *drSubRectList = new DRSubRectList();
					debug(3, "SubRectList; count = %d", count);
					for (uint j = 0; j < count; j++) {
						DRSubRect drSubRect;
						drSubRect.rect.x1 = dataS.readUint16LE();
						drSubRect.rect.y1 = dataS.readUint16LE();
						drSubRect.rect.x2 = dataS.readUint16LE();
						drSubRect.rect.y2 = dataS.readUint16LE();
						drSubRect.messageListHash = dataS.readUint32LE();
						drSubRect.messageListItemIndex = dataS.readUint16LE();
						debug(3, "(%d, %d, %d, %d) -> %08X (%d)", drSubRect.rect.x1, drSubRect.rect.y1, drSubRect.rect.x2, drSubRect.rect.y2, drSubRect.messageListHash, drSubRect.messageListItemIndex);
						drSubRectList->push_back(drSubRect);
					}
					drDirectoryItem.offset = _drSubRectLists.size();
					_drSubRectLists.push_back(drSubRectList);
					break;
				}
			case 6:
				{
					DRRect drRect;
					drRect.rect.x1 = dataS.readUint16LE();
					drRect.rect.y1 = dataS.readUint16LE();
					drRect.rect.x2 = dataS.readUint16LE();
					drRect.rect.y2 = dataS.readUint16LE();
					drRect.subRectIndex = dataS.readUint16LE();
					debug(3, "(%d, %d, %d, %d) -> %d", drRect.rect.x1, drRect.rect.y1, drRect.rect.x2, drRect.rect.y2, drRect.subRectIndex);
					drDirectoryItem.offset = _drRects.size();
					_drRects.push_back(drRect);
					break;
				}
			case 7:
				{
					uint count = dataS.readUint16LE();
					NRectArray *rectArray = new NRectArray();
					debug(3, "NRectArray; count = %d", count);
					for (uint j = 0; j < count; j++) {
						NRect rect;
						rect.x1 = dataS.readUint16LE();
						rect.y1 = dataS.readUint16LE();
						rect.x2 = dataS.readUint16LE();
						rect.y2 = dataS.readUint16LE();
						debug(3, "(%d, %d, %d, %d)", rect.x1, rect.y1, rect.x2, rect.y2);
						rectArray->push_back(rect);
					}
					drDirectoryItem.offset = _rectArrays.size();
					_rectArrays.push_back(rectArray);
					break;
				}
			default:
				break;
			}
			_directory.push_back(drDirectoryItem);
		}
	}
}

void DataResource::unload() {
	_directory.clear();
	_points.clear();
	for (Common::Array<NPointArray*>::iterator it = _pointArrays.begin(); it != _pointArrays.end(); ++it)
		delete (*it);
	_pointArrays.clear();
	for (Common::Array<NRectArray*>::iterator it = _rectArrays.begin(); it != _rectArrays.end(); ++it)
		delete (*it);
	_rectArrays.clear();
	for (Common::Array<HitRectList*>::iterator it = _hitRectLists.begin(); it != _hitRectLists.end(); ++it)
		delete (*it);
	_hitRectLists.clear();
	for (Common::Array<MessageList*>::iterator it = _messageLists.begin(); it != _messageLists.end(); ++it)
		delete (*it);
	_messageLists.clear();
	_drRects.clear();
	for (Common::Array<DRSubRectList*>::iterator it = _drSubRectLists.begin(); it != _drSubRectLists.end(); ++it)
		delete (*it);
	_drSubRectLists.clear();
	_vm->_res->unloadResource(_resourceHandle);
}

NPoint DataResource::getPoint(uint32 nameHash) {
	DataResource::DRDirectoryItem *drDirectoryItem = findDRDirectoryItem(nameHash, 1);
	return drDirectoryItem ? _points[drDirectoryItem->offset] : NPoint();
}

NPointArray *DataResource::getPointArray(uint32 nameHash) {
	DataResource::DRDirectoryItem *drDirectoryItem = findDRDirectoryItem(nameHash, 2);
	return drDirectoryItem ? _pointArrays[drDirectoryItem->offset] : NULL;
}

NRectArray *DataResource::getRectArray(uint32 nameHash) {
	DataResource::DRDirectoryItem *drDirectoryItem = findDRDirectoryItem(nameHash, 3);
	return drDirectoryItem ? _rectArrays[drDirectoryItem->offset] : NULL;
}

HitRectList *DataResource::getHitRectList() {
	DataResource::DRDirectoryItem *drDirectoryItem = findDRDirectoryItem(calcHash("HitArray"), 3);
	return drDirectoryItem ? _hitRectLists[drDirectoryItem->offset] : NULL;
}

MessageList *DataResource::getMessageListAtPos(int16 klaymenX, int16 klaymenY, int16 mouseX, int16 mouseY) {
	for (uint i = 0; i < _drRects.size(); i++) {
		if (klaymenX >= _drRects[i].rect.x1 && klaymenX <= _drRects[i].rect.x2 &&
			klaymenY >= _drRects[i].rect.y1 && klaymenY <= _drRects[i].rect.y2) {
			DRSubRectList *drSubRectList = _drSubRectLists[_drRects[i].subRectIndex];
			for (uint j = 0; j < drSubRectList->size(); j++) {
				DRSubRect &subRect = (*drSubRectList)[j];
				if (mouseX >= subRect.rect.x1 && mouseX <= subRect.rect.x2 &&
					mouseY >= subRect.rect.y1 && mouseY <= subRect.rect.y2) {
					return _messageLists[subRect.messageListItemIndex];
				}
			}
		}
	}
	return NULL;
}

DataResource::DRDirectoryItem *DataResource::findDRDirectoryItem(uint32 nameHash, uint16 type) {
	for (Common::Array<DRDirectoryItem>::iterator it = _directory.begin(); it != _directory.end(); it++)
		if ((*it).nameHash == nameHash && (*it).type == type)
			return &(*it);
	return NULL;
}

uint32 calcHash(const char *value) {
	uint32 hash = 0, shiftValue = 0;
	while (*value != 0) {
		char ch = *value++;
		if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
			if (ch >= 'a' && ch <= 'z')
				ch -= 32;
			else if (ch >= '0' && ch <= '9')
				ch += 22;
			shiftValue += ch - 64;
			if (shiftValue >= 32)
				shiftValue -= 32;
			hash ^= 1 << shiftValue;
		}
	}
	return hash;
}

} // End of namespace Neverhood
