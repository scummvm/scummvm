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
#include "saga/rscfile_mod.h"

#include "saga/text.h"
#include "saga/font.h"

#include "saga/sprite.h"
#include "saga/stream.h"

namespace Saga {

Sprite::Sprite(SagaEngine *vm) : _vm(vm), _initialized(false) {
	debug(0, "Initializing sprite subsystem...");

	// Load sprite module resource context
	_spriteContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_spriteContext == NULL) {
		return;
	}

	_decodeBufLen = DECODE_BUF_LEN;

	_decodeBuf = (byte *)malloc(_decodeBufLen);
	if (_decodeBuf == NULL) {
		return;
	}

	loadList(RID_ITE_MAIN_SPRITES, _mainSprites); //fixme: IHNM may have no such list

	_initialized = true;
}

Sprite::~Sprite(void) {
	if (!_initialized) {
		return;
	}

	debug(0, "Shutting down sprite subsystem...");
	_mainSprites.freeMem();
	free(_decodeBuf);
}

int Sprite::loadList(int resourceId, SpriteList &spriteList) {
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

	if (RSC_LoadResource(_spriteContext, resourceId, &spriteListData, &spriteListLength) != SUCCESS) {
		warning("Sprite::loadList RSC_LoadResource FAILURE");
		return FAILURE;
	}

	if (spriteListLength == 0) {
		warning("Sprite::loadList spriteListLength == 0");
		return FAILURE;
	}

	MemoryReadStreamEndian readS(spriteListData, spriteListLength, IS_BIG_ENDIAN);

	spriteCount = readS.readUint16();

	oldSpriteCount = spriteList.spriteCount;
	newSpriteCount = spriteList.spriteCount + spriteCount;

	spriteList.infoList = (SpriteInfo *)realloc(spriteList.infoList, newSpriteCount * sizeof(*spriteList.infoList));
	if (spriteList.infoList == NULL) {
		error("Sprite::loadList Not enough memory");
	}

	spriteList.spriteCount = newSpriteCount;

	for (i = oldSpriteCount; i < spriteList.spriteCount; i++) {
		spriteInfo = &spriteList.infoList[i];
		if (_vm->_features & GF_MAC_RESOURCES)
			offset = readS.readUint32();
		else
			offset = readS.readUint16();

		if (offset >= spriteListLength) {
			error("Sprite::loadList offset exceed");
		}

		spritePointer = spriteListData;
		spritePointer += offset;

		MemoryReadStream readS(spritePointer, (_vm->_features & GF_MAC_RESOURCES) ? 8 : 4);

		if (!(_vm->_features & GF_MAC_RESOURCES)) {
			spriteInfo->xAlign = readS.readSByte();
			spriteInfo->yAlign = readS.readSByte();

			spriteInfo->width = readS.readByte();
			spriteInfo->height = readS.readByte();
		} else {
			spriteInfo->xAlign = readS.readSint16BE();
			spriteInfo->yAlign = readS.readSint16BE();

			spriteInfo->width = readS.readUint16BE();
			spriteInfo->height = readS.readUint16BE();
		}
		spriteDataPointer = spritePointer + readS.pos();
		outputLength = spriteInfo->width * spriteInfo->height;
		decodeRLEBuffer(spriteDataPointer, 64000, outputLength); //todo: 64000 - should be replace by real input length
		spriteInfo->decodedBuffer = (byte *) malloc(outputLength);
		if (spriteInfo->decodedBuffer == NULL) {
			error("Sprite::loadList Not enough memory");
		}
		memcpy(spriteInfo->decodedBuffer, _decodeBuf, outputLength);
	}

	RSC_FreeResource(spriteListData);

	return SUCCESS;
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

int Sprite::draw(SURFACE *ds, SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale) {
	const byte *spriteBuffer;
	int i, j;
	byte *buf_row_p;
	const byte *src_row_p;
	int clip_width;
	int clip_height;
	int width;
	int height;
	int xAlign;
	int yAlign;
	Point spritePointer;

	assert(_initialized);

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);
	
	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;

	if (spritePointer.x < 0) {
		return 0;
	}
	if (spritePointer.y < 0) {
		return 0;
	}

	buf_row_p = (byte *)ds->pixels + ds->pitch * spritePointer.y;
	src_row_p = spriteBuffer;

	// Clip to right side of surface
	clip_width = width;
	if (width > (ds->w - spritePointer.x)) {
		clip_width = (ds->w - spritePointer.x);
	}

	// Clip to bottom side of surface
	clip_height = height;
	if (height > (ds->h - spritePointer.y)) {
		clip_height = (ds->h - spritePointer.y);
	}

	for (i = 0; i < clip_height; i++) {
		for (j = 0; j < clip_width; j++) {
			if (*(src_row_p + j) != 0) {
				*(buf_row_p + j + spritePointer.x) = *(src_row_p + j);
			}
		}
		buf_row_p += ds->pitch;
		src_row_p += width;
	}

	return SUCCESS;
}

int Sprite::drawOccluded(SURFACE *ds, SpriteList &spriteList, int spriteNumber, const Point &screenCoord, int scale, int depth) {
	const byte *spriteBuffer;
	int x, y;
	byte *dst_row_p;
	const byte *src_row_p;
	const byte *src_p;
	byte *dst_p;
	byte *mask_p;
	int width;
	int height;
	int xAlign;
	int yAlign;
	Point spritePointer;

	// Clipinfo variables
	Rect spriteSourceRect;
	Rect spriteDestRect;
	CLIPINFO ci;

	// BG mask variables
	int maskWidth;
	int maskHeight;
	byte *maskBuffer;
	size_t maskBufferLength;
	byte *mask_row_p;
	int mask_z;


	assert(_initialized);

	if (!_vm->_scene->isBGMaskPresent()) {
		return draw(ds, spriteList, spriteNumber, screenCoord, scale);
	}

	_vm->_scene->getBGMaskInfo(maskWidth, maskHeight, maskBuffer, maskBufferLength);

	getScaledSpriteBuffer(spriteList, spriteNumber, scale, width, height, xAlign, yAlign, spriteBuffer);

	spritePointer.x = screenCoord.x + xAlign;
	spritePointer.y = screenCoord.y + yAlign;

	spriteSourceRect.left = 0;
	spriteSourceRect.top = 0;
	spriteSourceRect.right = width;
	spriteSourceRect.bottom = height;

	spriteDestRect.left = 0;
	spriteDestRect.top = 0;
	spriteDestRect.right = ds->clip_rect.right;
	spriteDestRect.bottom = MIN(ds->clip_rect.bottom, (int16)maskHeight);
	
	ci.dst_rect = &spriteDestRect;
	ci.src_rect = &spriteSourceRect;
	ci.dst_pt = &spritePointer;

	getClipInfo(&ci);

	if (ci.nodraw) {
		return SUCCESS;
	}


	// Finally, draw the occluded sprite
	src_row_p = spriteBuffer + ci.src_draw_x + (ci.src_draw_y * width);

	dst_row_p = (byte *)ds->pixels + ci.dst_draw_x + (ci.dst_draw_y * ds->pitch);
	mask_row_p = maskBuffer + ci.dst_draw_x + (ci.dst_draw_y * maskWidth);

	for (y = 0; y < ci.draw_h; y++) {
		src_p = src_row_p;
		dst_p = dst_row_p;
		mask_p = mask_row_p;
		for (x = 0; x < ci.draw_w; x++) {
			if (*src_p != 0) {
				mask_z = *mask_p & SPRITE_ZMASK;
				if (mask_z > depth) {
					*dst_p = *src_p;
				}
			}
			src_p++;
			dst_p++;
			mask_p++;
		}
		dst_row_p += ds->pitch;
		mask_row_p += maskWidth;
		src_row_p += width;
	}

	return SUCCESS;
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

	while (!readS.eof() && (outPointer < outPointerEnd)) {
		bg_runcount = readS.readByte();
		fg_runcount = readS.readByte();

		for (c = 0; c < bg_runcount; c++) {
			*outPointer = (byte) 0;
			if (outPointer < outPointerEnd)
				outPointer++;
			else
				return;
		}

		for (c = 0; c < fg_runcount; c++) {
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

		if(vskip < skip) { // We had an overflow
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
