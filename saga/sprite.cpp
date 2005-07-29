/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Sprite management module
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/scene.h"
#include "saga/rscfile.h"

#include "saga/font.h"

#include "saga/sprite.h"
#include "saga/stream.h"

namespace Saga {

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

	loadList(_vm->getResourceDescription()->mainSpritesResourceId, _mainSprites);
}

Sprite::~Sprite(void) {
	debug(8, "Shutting down sprite subsystem...");
	_mainSprites.freeMem();
	free(_decodeBuf);
}

void Sprite::loadList(int resourceId, SpriteList &spriteList) {
	SpriteInfo *spriteInfo;
	byte *spriteListData;
	size_t spriteListLength;
	uint16 oldSpriteCount;
	uint16 newSpriteCount;
	uint16 spriteCount;
	int i;
	int outputLength;
	uint32 offset;
	const byte *spritePointer;
	const byte *spriteDataPointer;

	_vm->_resource->loadResource(_spriteContext, resourceId, spriteListData, spriteListLength);

	if (spriteListLength == 0) {
		return;
	}

	MemoryReadStreamEndian readS(spriteListData, spriteListLength, _spriteContext->isBigEndian);

	spriteCount = readS.readUint16();

	oldSpriteCount = spriteList.spriteCount;
	newSpriteCount = spriteList.spriteCount + spriteCount;

	spriteList.infoList = (SpriteInfo *)realloc(spriteList.infoList, newSpriteCount * sizeof(*spriteList.infoList));
	if (spriteList.infoList == NULL) {
		memoryError("Sprite::loadList");
	}

	spriteList.spriteCount = newSpriteCount;

	for (i = oldSpriteCount; i < spriteList.spriteCount; i++) {
		spriteInfo = &spriteList.infoList[i];
		if (_vm->isMacResources())
			offset = readS.readUint32();
		else
			offset = readS.readUint16();

		if (offset >= spriteListLength) {
			error("Sprite::loadList offset exceed");
		}

		spritePointer = spriteListData;
		spritePointer += offset;

		MemoryReadStream readS2(spritePointer, (_vm->getFeatures() & GF_MAC_RESOURCES) ? 8 : 4);

		if (!(_vm->getFeatures() & GF_MAC_RESOURCES)) {
			spriteInfo->xAlign = readS2.readSByte();
			spriteInfo->yAlign = readS2.readSByte();

			spriteInfo->width = readS2.readByte();
			spriteInfo->height = readS2.readByte();
		} else {
			spriteInfo->xAlign = readS2.readSint16BE();
			spriteInfo->yAlign = readS2.readSint16BE();

			spriteInfo->width = readS2.readUint16BE();
			spriteInfo->height = readS2.readUint16BE();
		}
		spriteDataPointer = spritePointer + readS2.pos();
		outputLength = spriteInfo->width * spriteInfo->height;
		decodeRLEBuffer(spriteDataPointer, 64000, outputLength); //todo: 64000 - should be replace by real input length
		spriteInfo->decodedBuffer = (byte *) malloc(outputLength);
		if (spriteInfo->decodedBuffer == NULL) {
			memoryError("Sprite::loadList");
		}
		memcpy(spriteInfo->decodedBuffer, _decodeBuf, outputLength);
	}

	free(spriteListData);
}

void Sprite::getScaledSpriteBuffer(SpriteList &spriteList, int spriteNumber, int scale, int &width, int &height, int &xAlign, int &yAlign, const byte *&buffer) {
	SpriteInfo *spriteInfo;
	assert(spriteList.spriteCount>spriteNumber);
	spriteInfo = &spriteList.infoList[spriteNumber];

	if (scale < 256) {
		xAlign = (spriteInfo->xAlign * scale) >> 8;
		yAlign = (spriteInfo->yAlign * scale) >> 8;
		height = (spriteInfo->height * scale + 0x80) >> 8;
		width = (spriteInfo->width * scale + 0x80) >> 8;
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

void Sprite::drawClip(Surface *ds, const Rect &clipRect, const Point &spritePointer, int width, int height, const byte *spriteBuffer) {
	int clipWidth;
	int clipHeight;

	int i, j, jo, io;
	byte *bufRowPointer;
	const byte *srcRowPointer;

	bufRowPointer = (byte *)ds->pixels + ds->pitch * spritePointer.y;
	srcRowPointer = spriteBuffer;

	clipWidth = width;
	if (width > (clipRect.right - spritePointer.x)) {
		clipWidth = (clipRect.right - spritePointer.x);
	}

	clipHeight = height;
	if (height > (clipRect.bottom - spritePointer.y)) {
		clipHeight = (clipRect.bottom - spritePointer.y);
	}

	jo = 0;
	io = 0;
	if (spritePointer.x < clipRect.left) {
		jo = clipRect.left - spritePointer.x;
	}
	if (spritePointer.y < clipRect.top) {
		io = clipRect.top - spritePointer.y;
		bufRowPointer += ds->pitch * io;
		srcRowPointer += width * io;
	}
	for (i = io; i < clipHeight; i++) {
		for (j = jo; j < clipWidth; j++) {
			assert((byte *)ds->pixels <= (byte *)(bufRowPointer + j + spritePointer.x));
			assert(((byte *)ds->pixels + (_vm->getDisplayWidth() *
				 _vm->getDisplayHeight())) > (byte *)(bufRowPointer + j + spritePointer.x));
			assert((const byte *)spriteBuffer <= (const byte *)(srcRowPointer + j));
			assert(((const byte *)spriteBuffer + (width * height)) > (const byte *)(srcRowPointer + j));

			if (*(srcRowPointer + j) != 0) {
				*(bufRowPointer + j + spritePointer.x) = *(srcRowPointer + j);
			}
		}
		bufRowPointer += ds->pitch;
		srcRowPointer += width;
	}
}

void Sprite::draw(Surface *ds, const Rect &clipRect, SpriteList &spriteList, int32 spriteNumber, const Point &screenCoord, int scale) {
	const byte *spriteBuffer;
	int width;
	int height;
	int xAlign;
	int yAlign;
	Point spritePointer;

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;
	drawClip(ds, clipRect, spritePointer, width, height, spriteBuffer);
}

void Sprite::draw(Surface *ds, const Rect &clipRect, SpriteList &spriteList, int32 spriteNumber, const Rect &screenRect, int scale) {
	const byte *spriteBuffer;
	int width;
	int height;
	int xAlign, spw;
	int yAlign, sph;
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
	drawClip(ds, clipRect, spritePointer, width, height, spriteBuffer);
}

bool Sprite::hitTest(SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, const Point &testPoint) {
	const byte *spriteBuffer;
	int i, j;
	const byte *srcRowPointer;
	int width;
	int height;
	int xAlign;
	int yAlign;
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

void Sprite::drawOccluded(Surface *ds, const Rect &clipRect, SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, int depth) {
	const byte *spriteBuffer;
	int x, y;
	byte *destRowPointer;
	const byte *sourceRowPointer;
	const byte *sourcePointer;
	byte *destPointer;
	byte *maskPointer;
	int width;
	int height;
	int xAlign;
	int yAlign;

	ClipData clipData;

	// BG mask variables
	int maskWidth;
	int maskHeight;
	byte *maskBuffer;
	size_t maskBufferLength;
	byte *maskRowPointer;
	int maskZ;


	if (!_vm->_scene->isBGMaskPresent()) {
		draw(ds, clipRect, spriteList, spriteNumber, screenCoord, scale);
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

	clipData.destRect = clipRect;

	if (!clipData.calcClip()) {
		return;
	}

	// Finally, draw the occluded sprite
	sourceRowPointer = spriteBuffer + clipData.drawSource.x + (clipData.drawSource.y * width);

	destRowPointer = (byte *)ds->pixels + clipData.drawDest.x + (clipData.drawDest.y * ds->pitch);
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
		destRowPointer += ds->pitch;
		maskRowPointer += maskWidth;
		sourceRowPointer += width;
	}
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
