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

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamBase::doBlocks() {
	uint16 dstOffset = data.word(kMapady) * 320 + data.word(kMapadx);
	uint16 mapOffset = kMap + data.byte(kMapy) * kMapwidth + data.byte(kMapx);
	const uint8 *mapData = getSegment(data.word(kMapdata)).ptr(mapOffset, 0);
	const uint8 *blocks = getSegment(data.word(kBackdrop)).ptr(kBlocks, 0);
	uint8 *dstBuffer = workspace() + dstOffset;

	for (size_t i = 0; i < 10; ++i) {
		for (size_t j = 0; j < 11; ++j) {
			uint16 blockType = mapData[j];
			if (blockType != 0) {
				uint8 *dst = dstBuffer + i * 320 * 16 + j * 16;
				const uint8 *block = blocks + blockType * 256;
				for (size_t k = 0; k < 4; ++k) {
					memcpy(dst, block, 16);
					block += 16;
					dst += 320;
				}
				for (size_t k = 0; k < 12; ++k) {
					memcpy(dst, block, 16);
					memset(dst + 16, 0xdf, 4);
					block += 16;
					dst += 320;
				}
				dst += 4;
				memset(dst, 0xdf, 16);
				dst += 320;
				memset(dst, 0xdf, 16);
				dst += 320;
				memset(dst, 0xdf, 16);
				dst += 320;
				memset(dst, 0xdf, 16);
			}
		}
		mapData += kMapwidth;
	}
}

uint8 DreamBase::getXAd(const uint8 *setData, uint8 *result) {
	uint8 v0 = setData[0];
	uint8 v1 = setData[1];
	uint8 v2 = setData[2];
	if (v0 != 0)
		return 0;
	if (v1 < data.byte(kMapx))
		return 0;
	v1 -= data.byte(kMapx);
	if (v1 >= 11)
		return 0;
	*result = (v1 << 4) | v2;
	return 1;
}

uint8 DreamBase::getYAd(const uint8 *setData, uint8 *result) {
	uint8 v0 = setData[3];
	uint8 v1 = setData[4];
	if (v0 < data.byte(kMapy))
		return 0;
	v0 -= data.byte(kMapy);
	if (v0 >= 10)
		return 0;
	*result = (v0 << 4) | v1;
	return 1;
}

uint8 DreamBase::getMapAd(const uint8 *setData) {
	uint8 xad, yad;
	if (getXAd(setData, &xad) == 0)
		return 0;
	data.word(kObjectx) = xad;
	if (getYAd(setData, &yad) == 0)
		return 0;
	data.word(kObjecty) = yad;
	return 1;
}

void DreamBase::calcFrFrame(uint16 frameSeg, uint16 frameNum, uint16 framesAd, uint8* width, uint8* height) {
	const Frame *frame = (const Frame *)getSegment(frameSeg).ptr(frameNum * sizeof(Frame), sizeof(Frame));
	data.word(kSavesource) = framesAd + frame->ptr();
	data.byte(kSavesize+0) = frame->width;
	data.byte(kSavesize+1) = frame->height;
	data.word(kOffsetx) = frame->x;
	data.word(kOffsety) = frame->y;
	*width = frame->width;
	*height = frame->height;
}

void DreamBase::finalFrame(uint16 *x, uint16 *y) {
	data.byte(kSavex) = (data.word(kObjectx) + data.word(kOffsetx)) & 0xff;
	data.byte(kSavey) = (data.word(kObjecty) + data.word(kOffsety)) & 0xff;
	*x = data.word(kObjectx);
	*y = data.word(kObjecty);
}

void DreamBase::makeBackOb(SetObject *objData) {
	if (data.byte(kNewobs) == 0)
		return;
	uint8 priority = objData->priority;
	uint8 type = objData->type;
	Sprite *sprite = makeSprite(data.word(kObjectx), data.word(kObjecty), addr_backobject, data.word(kSetframes), 0);

	uint16 objDataOffset = (uint8 *)objData - getSegment(data.word(kSetdat)).ptr(0, 0);
	assert(objDataOffset % sizeof(SetObject) == 0);
	assert(objDataOffset < 128 * sizeof(SetObject));
	sprite->setObjData(objDataOffset);
	if (priority == 255)
		priority = 0;
	sprite->priority = priority;
	sprite->type = type;
	sprite->b16 = 0;
	sprite->delay = 0;
	sprite->animFrame = 0;
}

void DreamBase::showAllObs() {
	const unsigned int count = 128;

	ObjPos *objPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(kSetlist, count * sizeof(ObjPos));
	memset(objPos, 0xff, count * sizeof(ObjPos));

	const Frame *frameBase = (const Frame *)getSegment(data.word(kSetframes)).ptr(0, 0);
	SetObject *setEntries = (SetObject *)getSegment(data.word(kSetdat)).ptr(0, count * sizeof(SetObject));
	for (size_t i = 0; i < count; ++i) {
		SetObject *setEntry = setEntries + i;
		if (getMapAd(setEntry->mapad) == 0)
			continue;
		uint8 currentFrame = setEntry->frames[0];
		if (currentFrame == 0xff)
			continue;
		uint8 width, height;
		calcFrFrame(data.word(kSetframes), currentFrame, kFrames, &width, &height);
		uint16 x, y;
		finalFrame(&x, &y);
		setEntry->index = setEntry->frames[0];
		if ((setEntry->type == 0) && (setEntry->priority != 5) && (setEntry->priority != 6)) {
			x += data.word(kMapadx);
			y += data.word(kMapady);
			showFrame(frameBase, x, y, currentFrame, 0);
		} else
			makeBackOb(setEntry);

		objPos->xMin = data.byte(kSavex);
		objPos->yMin = data.byte(kSavey);
		objPos->xMax = data.byte(kSavex) + data.byte(kSavesize+0);
		objPos->yMax = data.byte(kSavey) + data.byte(kSavesize+1);
		objPos->index = i;
		++objPos;
	}
}

bool DreamBase::addAlong(const uint8 *mapFlags) {
	for (size_t i = 0; i < 11; ++i) {
		if (mapFlags[3 * i] != 0)
			return true;
	}
	return false;
}

bool DreamBase::addLength(const uint8 *mapFlags) {
	for (size_t i = 0; i < 10; ++i) {
		if (mapFlags[3 * 11 * i] != 0)
			return true;
	}
	return false;
}

void DreamBase::getDimension(uint8 *mapXstart, uint8 *mapYstart, uint8 *mapXsize, uint8 *mapYsize) {
	const uint8 *mapFlags = getSegment(data.word(kBuffers)).ptr(kMapflags, 0);

	uint8 yStart = 0;
	while (! addAlong(mapFlags + 3 * 11 * yStart))
		++yStart;

	uint8 xStart = 0;
	while (! addLength(mapFlags + 3 * xStart))
		++xStart;

	uint8 yEnd = 10;
	while (! addAlong(mapFlags + 3 * 11 * (yEnd - 1)))
		--yEnd;

	uint8 xEnd = 11;
	while (! addLength(mapFlags + 3 * (xEnd - 1)))
		--xEnd;

	*mapXstart = xStart;
	*mapYstart = yStart;
	*mapXsize = xEnd - xStart;
	*mapYsize = yEnd - yStart;
	data.word(kMapxstart) = xStart << 4;
	data.word(kMapystart) = yStart << 4;
	data.byte(kMapxsize) = *mapXsize << 4;
	data.byte(kMapysize) = *mapYsize << 4;
}

void DreamBase::calcMapAd() {
	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
	data.word(kMapadx) = data.word(kMapoffsetx) - 8 * (mapXsize + 2 * mapXstart - 11);
	data.word(kMapady) = data.word(kMapoffsety) - 8 * (mapYsize + 2 * mapYstart - 10);
}

void DreamBase::showAllFree() {
	const unsigned int count = 80;

	ObjPos *objPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(kFreelist, count * sizeof(ObjPos));
	memset(objPos, 0xff, count * sizeof(ObjPos));

	const DynObject *freeObjects = (const DynObject *)getSegment(data.word(kFreedat)).ptr(0, 0);
	const Frame *frameBase = (const Frame *)getSegment(data.word(kFreeframes)).ptr(0, 0);
	for (size_t i = 0; i < count; ++i) {
		uint8 mapAd = getMapAd(freeObjects[i].mapad);
		if (mapAd != 0) {
			uint8 width, height;
			uint16 currentFrame = 3 * i;
			calcFrFrame(data.word(kFreeframes), currentFrame, kFrframes, &width, &height);
			uint16 x, y;
			finalFrame(&x, &y);
			if ((width != 0) || (height != 0)) {
				x += data.word(kMapadx);
				y += data.word(kMapady);
				assert(currentFrame < 256);
				showFrame(frameBase, x, y, currentFrame, 0);
				objPos->xMin = data.byte(kSavex);
				objPos->yMin = data.byte(kSavey);
				objPos->xMax = data.byte(kSavex) + data.byte(kSavesize+0);
				objPos->yMax = data.byte(kSavey) + data.byte(kSavesize+1);
				objPos->index = i;
				++objPos;
			}
		}
	}
}

void DreamBase::drawFlags() {
	uint8 *mapFlags = getSegment(data.word(kBuffers)).ptr(kMapflags, 0);
	const uint8 *mapData = getSegment(data.word(kMapdata)).ptr(kMap + data.byte(kMapy) * kMapwidth + data.byte(kMapx), 0);
	const uint8 *backdropFlags = getSegment(data.word(kBackdrop)).ptr(kFlags, 0);

	for (size_t i = 0; i < 10; ++i) {
		for (size_t j = 0; j < 11; ++j) {
			uint8 tile = mapData[i * kMapwidth + j];
			mapFlags[0] = backdropFlags[2 * tile + 0];
			mapFlags[1] = backdropFlags[2 * tile + 1];
			mapFlags[2] = tile;
			mapFlags += 3;
		}
	}
}

void DreamBase::showAllEx() {
	const unsigned int count = 100;

	ObjPos *objPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(kExlist, count * sizeof(ObjPos));
	memset(objPos, 0xff, count * sizeof(ObjPos));

	DynObject *objects = (DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, sizeof(DynObject));
	const Frame *frameBase = (const Frame *)getSegment(data.word(kExtras)).ptr(0, 0);
	for (size_t i = 0; i < count; ++i) {
		DynObject *object = objects + i;
		if (object->mapad[0] == 0xff)
			continue;
		if (object->currentLocation != data.byte(kReallocation))
			continue;
		if (getMapAd(object->mapad) == 0)
			continue;
		uint8 width, height;
		uint16 currentFrame = 3 * i;
		calcFrFrame(data.word(kExtras), currentFrame, kExframes, &width, &height);
		uint16 x, y;
		finalFrame(&x, &y);
		if ((width != 0) || (height != 0)) {
			assert(currentFrame < 256);
			showFrame(frameBase, x + data.word(kMapadx), y + data.word(kMapady), currentFrame, 0);
			objPos->xMin = data.byte(kSavex);
			objPos->yMin = data.byte(kSavey);
			objPos->xMax = data.byte(kSavesize + 0) + data.byte(kSavex);
			objPos->yMax = data.byte(kSavesize + 1) + data.byte(kSavey);
			objPos->index = i;
			++objPos;
		}
	}
}

} // End of namespace DreamGen
