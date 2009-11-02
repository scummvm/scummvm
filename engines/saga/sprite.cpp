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
 * $URL$
 * $Id$
 *
 */

// Sprite management module

#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/scene.h"
#include "saga/resource.h"
#include "saga/font.h"

#include "saga/sprite.h"
#include "saga/render.h"

namespace Saga {

#define RID_IHNM_ARROW_SPRITES 13
#define RID_IHNM_SAVEREMINDER_SPRITES 14
#define RID_IHNMDEMO_ARROW_SPRITES 8
#define RID_IHNMDEMO_SAVEREMINDER_SPRITES 9

Sprite::Sprite(SagaEngine *vm) : _vm(vm) {
	debug(8, "Initializing sprite subsystem...");

	// Load sprite module resource context
	_spriteContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (_spriteContext == NULL) {
		error("Sprite::Sprite resource context not found");
	}

	_decodeBufLen = DECODE_BUF_LEN;

	_decodeBuf = (byte *)malloc(_decodeBufLen);
	if (_decodeBuf == NULL) {
		memoryError("Sprite::Sprite");
	}

	if (_vm->getGameId() == GID_ITE) {
		loadList(_vm->getResourceDescription()->mainSpritesResourceId, _mainSprites);
		_arrowSprites = _saveReminderSprites = _inventorySprites = _mainSprites;
#ifdef ENABLE_IHNM
	} else if (_vm->getGameId() == GID_IHNM) {
		if (_vm->getFeatures() & GF_IHNM_DEMO) {
			loadList(RID_IHNMDEMO_ARROW_SPRITES, _arrowSprites);
			loadList(RID_IHNMDEMO_SAVEREMINDER_SPRITES, _saveReminderSprites);
		} else {
			loadList(RID_IHNM_ARROW_SPRITES, _arrowSprites);
			loadList(RID_IHNM_SAVEREMINDER_SPRITES, _saveReminderSprites);
		}
#endif
	} else {
		error("Sprite: unknown game type");
	}
}

Sprite::~Sprite() {
	debug(8, "Shutting down sprite subsystem...");
	_mainSprites.freeMem();
	if (_vm->getGameId() == GID_IHNM) {
		_inventorySprites.freeMem();
		_arrowSprites.freeMem();
		_saveReminderSprites.freeMem();
	}
	free(_decodeBuf);
}

void Sprite::loadList(int resourceId, SpriteList &spriteList) {
	SpriteInfo *spriteInfo;
	byte *spriteListData = 0;
	size_t spriteListLength = 0;
	uint16 oldSpriteCount;
	uint16 newSpriteCount;
	uint16 spriteCount;
	int i;
	int outputLength, inputLength;
	uint32 offset;
	const byte *spritePointer;
	const byte *spriteDataPointer;

	_vm->_resource->loadResource(_spriteContext, resourceId, spriteListData, spriteListLength);

	if (spriteListLength == 0) {
		return;
	}

	MemoryReadStreamEndian readS(spriteListData, spriteListLength, _spriteContext->isBigEndian);

	spriteCount = readS.readUint16();

	debug(9, "Sprites: %d", spriteCount);

	oldSpriteCount = spriteList.spriteCount;
	newSpriteCount = spriteList.spriteCount + spriteCount;

	spriteList.infoList = (SpriteInfo *)realloc(spriteList.infoList, newSpriteCount * sizeof(*spriteList.infoList));
	if (spriteList.infoList == NULL) {
		memoryError("Sprite::loadList");
	}

	spriteList.spriteCount = newSpriteCount;

	bool bigHeader = _vm->getGameId() == GID_IHNM || _vm->isMacResources();

	for (i = oldSpriteCount; i < spriteList.spriteCount; i++) {
		spriteInfo = &spriteList.infoList[i];
		if (bigHeader)
			offset = readS.readUint32();
		else
			offset = readS.readUint16();

		if (offset >= spriteListLength) {
			// ITE Mac demos throw this warning
			warning("Sprite::loadList offset exceeded");
			spriteList.spriteCount = i;
			return;
		}

		spritePointer = spriteListData;
		spritePointer += offset;

		if (bigHeader) {
			MemoryReadStreamEndian readS2(spritePointer, 8, _spriteContext->isBigEndian);

			spriteInfo->xAlign = readS2.readSint16();
			spriteInfo->yAlign = readS2.readSint16();

			spriteInfo->width = readS2.readUint16();
			spriteInfo->height = readS2.readUint16();

			spriteDataPointer = spritePointer + readS2.pos();
		} else {
			MemoryReadStreamEndian readS2(spritePointer, 4);

			spriteInfo->xAlign = readS2.readSByte();
			spriteInfo->yAlign = readS2.readSByte();

			spriteInfo->width = readS2.readByte();
			spriteInfo->height = readS2.readByte();
			spriteDataPointer = spritePointer + readS2.pos();
		}

		outputLength = spriteInfo->width * spriteInfo->height;
		inputLength = spriteListLength - (spriteDataPointer - spriteListData);
		decodeRLEBuffer(spriteDataPointer, inputLength, outputLength);
		spriteInfo->decodedBuffer = (byte *) malloc(outputLength);
		if (spriteInfo->decodedBuffer == NULL) {
			memoryError("Sprite::loadList");
		}

#ifdef ENABLE_IHNM
		// IHNM sprites are upside-down, for reasons which i can only
		// assume are perverse. To simplify things, flip them now. Not
		// at drawing time.

		if (_vm->getGameId() == GID_IHNM) {
			byte *src = _decodeBuf + spriteInfo->width * (spriteInfo->height - 1);
			byte *dst = spriteInfo->decodedBuffer;

			for (int j = 0; j < spriteInfo->height; j++) {
				memcpy(dst, src, spriteInfo->width);
				src -= spriteInfo->width;
				dst += spriteInfo->width;
			}
		} else
#endif
			memcpy(spriteInfo->decodedBuffer, _decodeBuf, outputLength);
	}

	free(spriteListData);
}

void Sprite::getScaledSpriteBuffer(SpriteList &spriteList, int spriteNumber, int scale, int &width, int &height, int &xAlign, int &yAlign, const byte *&buffer) {
	SpriteInfo *spriteInfo;

	if (spriteList.spriteCount <= spriteNumber) {
		// this can occur in IHNM while loading a saved game from chapter 1-5 when being in the end chapter
		warning("spriteList.spriteCount <= spriteNumber");
		return;
	}

	spriteInfo = &spriteList.infoList[spriteNumber];

	if (scale < 256) {
		xAlign = (spriteInfo->xAlign * scale) >> 8;
		yAlign = (spriteInfo->yAlign * scale) >> 8;
		height = (spriteInfo->height * scale + 0x7f) >> 8;
		width = (spriteInfo->width * scale + 0x7f) >> 8;
		scaleBuffer(spriteInfo->decodedBuffer, spriteInfo->width, spriteInfo->height, scale);
		buffer = _decodeBuf;
	} else {
		xAlign = spriteInfo->xAlign;
		yAlign = spriteInfo->yAlign;
		height = spriteInfo->height;
		width = spriteInfo->width;
		buffer = spriteInfo->decodedBuffer;
	}
}

void Sprite::drawClip(const Point &spritePointer, int width, int height, const byte *spriteBuffer, bool clipToScene) {
	int clipWidth;
	int clipHeight;
	Common::Rect clipRect = clipToScene ? _vm->_scene->getSceneClip() : _vm->getDisplayClip();

	int i, j, jo, io;
	byte *bufRowPointer;
	const byte *srcRowPointer;

	bufRowPointer = _vm->_gfx->getBackBufferPixels() + _vm->_gfx->getBackBufferPitch() * spritePointer.y;
	srcRowPointer = spriteBuffer;

	clipWidth = CLIP(width, 0, clipRect.right - spritePointer.x);
	clipHeight = CLIP(height, 0, clipRect.bottom - spritePointer.y);

	jo = 0;
	io = 0;
	if (spritePointer.x < clipRect.left) {
		jo = clipRect.left - spritePointer.x;
	}
	if (spritePointer.y < clipRect.top) {
		io = clipRect.top - spritePointer.y;
		bufRowPointer += _vm->_gfx->getBackBufferPitch() * io;
		srcRowPointer += width * io;
	}

	for (i = io; i < clipHeight; i++) {
		for (j = jo; j < clipWidth; j++) {
			assert(_vm->_gfx->getBackBufferPixels() <= (byte *)(bufRowPointer + j + spritePointer.x));
			assert((_vm->_gfx->getBackBufferPixels() + (_vm->getDisplayInfo().width *
				 _vm->getDisplayInfo().height)) > (byte *)(bufRowPointer + j + spritePointer.x));
			assert((const byte *)spriteBuffer <= (const byte *)(srcRowPointer + j));
			assert(((const byte *)spriteBuffer + (width * height)) > (const byte *)(srcRowPointer + j));

			if (*(srcRowPointer + j) != 0) {
				*(bufRowPointer + j + spritePointer.x) = *(srcRowPointer + j);
			}
		}
		bufRowPointer += _vm->_gfx->getBackBufferPitch();
		srcRowPointer += width;
	}

	int x1 = MAX<int>(spritePointer.x, 0);
	int y1 = MAX<int>(spritePointer.y, 0);
	int x2 = MIN<int>(MAX<int>(spritePointer.x + clipWidth, 0), clipRect.right);
	int y2 = MIN<int>(MAX<int>(spritePointer.y + clipHeight, 0), clipRect.bottom);

	if (x2 > x1 && y2 > y1)
		_vm->_render->addDirtyRect(Common::Rect(x1, y1, x2, y2));
}

void Sprite::draw(SpriteList &spriteList, int32 spriteNumber, const Point &screenCoord, int scale, bool clipToScene) {
	const byte *spriteBuffer = NULL;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int yAlign = 0;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;

	drawClip(spritePointer, width, height, spriteBuffer, clipToScene);
}

void Sprite::draw(SpriteList &spriteList, int32 spriteNumber, const Rect &screenRect, int scale, bool clipToScene) {
	const byte *spriteBuffer = NULL;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int spw;
	int yAlign = 0;
	int sph;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);
	spw = (screenRect.width() - width) / 2;
	sph = (screenRect.height() - height) / 2;
	if (spw < 0) {
		spw = 0;
	}
	if (sph < 0) {
		sph = 0;
	}
	spritePointer.x = screenRect.left + xAlign + spw;
	spritePointer.y = screenRect.top + yAlign + sph;
	drawClip(spritePointer, width, height, spriteBuffer, clipToScene);
}

bool Sprite::hitTest(SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, const Point &testPoint) {
	const byte *spriteBuffer = NULL;
	int i, j;
	const byte *srcRowPointer;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int yAlign = 0;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;

	if ((testPoint.y < spritePointer.y) || (testPoint.y >= spritePointer.y + height)) {
		return false;
	}
	if ((testPoint.x < spritePointer.x) || (testPoint.x >= spritePointer.x + width)) {
		return false;
	}
	i = testPoint.y - spritePointer.y;
	j = testPoint.x - spritePointer.x;
	srcRowPointer = spriteBuffer + j + i * width;
	return *srcRowPointer != 0;
}

void Sprite::drawOccluded(SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, int depth) {
	const byte *spriteBuffer = NULL;
	int x, y;
	byte *destRowPointer;
	const byte *sourceRowPointer;
	const byte *sourcePointer;
	byte *destPointer;
	byte *maskPointer;
	int width  = 0;
	int height = 0;
	int xAlign = 0;
	int yAlign = 0;

	ClipData clipData;

	// BG mask variables
	int maskWidth;
	int maskHeight;
	byte *maskBuffer;
	size_t maskBufferLength;
	byte *maskRowPointer;
	int maskZ;

	if (!_vm->_scene->isBGMaskPresent()) {
		draw(spriteList, spriteNumber, screenCoord, scale);
		return;
	}

	_vm->_scene->getBGMaskInfo(maskWidth, maskHeight, maskBuffer, maskBufferLength);

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	clipData.destPoint.x = screenCoord.x + xAlign;
	clipData.destPoint.y = screenCoord.y + yAlign;

	clipData.sourceRect.left = 0;
	clipData.sourceRect.top = 0;
	clipData.sourceRect.right = width;
	clipData.sourceRect.bottom = height;

	clipData.destRect = _vm->_scene->getSceneClip();

	if (!clipData.calcClip()) {
		return;
	}

	// Finally, draw the occluded sprite

	sourceRowPointer = spriteBuffer + clipData.drawSource.x + (clipData.drawSource.y * width);
	destRowPointer = _vm->_gfx->getBackBufferPixels() + clipData.drawDest.x + (clipData.drawDest.y * _vm->_gfx->getBackBufferPitch());
	maskRowPointer = maskBuffer + clipData.drawDest.x + (clipData.drawDest.y * maskWidth);

	for (y = 0; y < clipData.drawHeight; y++) {
		sourcePointer = sourceRowPointer;
		destPointer = destRowPointer;
		maskPointer = maskRowPointer;
		for (x = 0; x < clipData.drawWidth; x++) {
			if (*sourcePointer != 0) {
				maskZ = *maskPointer & SPRITE_ZMASK;
				if (maskZ > depth) {
					*destPointer = *sourcePointer;
				}
			}
			sourcePointer++;
			destPointer++;
			maskPointer++;
		}
		destRowPointer += _vm->_gfx->getBackBufferPitch();
		maskRowPointer += maskWidth;
		sourceRowPointer += width;
	}

	_vm->_render->addDirtyRect(Common::Rect(clipData.drawSource.x, clipData.drawSource.y,
								clipData.drawSource.x + width, clipData.drawSource.y + height));
}

void Sprite::decodeRLEBuffer(const byte *inputBuffer, size_t inLength, size_t outLength) {
	int bg_runcount;
	int fg_runcount;
	byte *outPointer;
	byte *outPointerEnd;
	int c;

	if (outLength > _decodeBufLen) { // TODO: may we should make dynamic growing?
		error("Sprite::decodeRLEBuffer outLength > _decodeBufLen");
	}

	outPointer = _decodeBuf;
	outPointerEnd = _decodeBuf + outLength;
	outPointerEnd--;

	memset(outPointer, 0, outLength);

	MemoryReadStream readS(inputBuffer, inLength);

	while (!readS.eos() && (outPointer < outPointerEnd)) {
		bg_runcount = readS.readByte();
		if (readS.eos())
			break;
		fg_runcount = readS.readByte();

		for (c = 0; c < bg_runcount && !readS.eos(); c++) {
			*outPointer = (byte) 0;
			if (outPointer < outPointerEnd)
				outPointer++;
			else
				return;
		}

		for (c = 0; c < fg_runcount && !readS.eos(); c++) {
			*outPointer = readS.readByte();
			if (readS.eos())
				break;
			if (outPointer < outPointerEnd)
				outPointer++;
			else
				return;
		}
	}
}

void Sprite::scaleBuffer(const byte *src, int width, int height, int scale) {
	byte skip = 256 - scale; // skip factor
	byte vskip = 0x80, hskip;
	byte *dst = _decodeBuf;

	for (int i = 0; i < height; i++) {
		vskip += skip;

		if (vskip < skip) { // We had an overflow
			src += width;
		} else {
			hskip = 0x80;

			for (int j = 0; j < width; j++) {
				*dst++ = *src++;

				hskip += skip;
				if (hskip < skip) // overflow
					dst--;
			}
		}
	}
}


} // End of namespace Saga
