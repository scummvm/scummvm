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

void DreamGenContext::doBlocks() {
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
				ax = 0x0dfdf;
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

uint8 DreamGenContext::getXAd(const uint8 *setData, uint8 *result) {
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

uint8 DreamGenContext::getYAd(const uint8 *setData, uint8 *result) {
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

void DreamGenContext::getMapAd() {
	ch = getMapAd((const uint8 *)es.ptr(si, 5));
}

uint8 DreamGenContext::getMapAd(const uint8 *setData) {
	uint8 xad, yad;
	if (getXAd(setData, &xad) == 0)
		return 0;
	data.word(kObjectx) = xad;
	if (getYAd(setData, &yad) == 0)
		return 0;
	data.word(kObjecty) = yad;
	return 1;
}

void DreamGenContext::calcFrFrame(uint16 frame) {
	uint8 width, height;
	calcFrFrame(frame, &width, &height);
	cl = width;
	ch = height;
}

void DreamGenContext::calcFrFrame(uint16 frameNum, uint8* width, uint8* height) {
	const Frame *frame = (const Frame *)getSegment(data.word(kFrsegment)).ptr(frameNum * sizeof(Frame), sizeof(Frame));
	data.word(kSavesource) = data.word(kFramesad) + frame->ptr();
	data.byte(kSavesize+0) = frame->width;
	data.byte(kSavesize+1) = frame->height;
	data.word(kOffsetx) = frame->x;
	data.word(kOffsety) = frame->y;
	*width = frame->width;
	*height = frame->height;
}

void DreamGenContext::finalFrame() {
	uint16 x, y;
	finalFrame(&x, &y);
	di = x;
	bx = y;
}

void DreamGenContext::finalFrame(uint16 *x, uint16 *y) {
	data.byte(kSavex) = (data.word(kObjectx) + data.word(kOffsetx)) & 0xff;
	data.byte(kSavey) = (data.word(kObjecty) + data.word(kOffsety)) & 0xff;
	*x = data.word(kObjectx);
	*y = data.word(kObjecty);
}

void DreamGenContext::showAllObs() {
	data.word(kListpos) = kSetlist;
	memset(getSegment(data.word(kBuffers)).ptr(kSetlist, 0), 0xff, 128 * 5);
	data.word(kFrsegment) = data.word(kSetframes);
	data.word(kDataad) = kFramedata;
	data.word(kFramesad) = kFrames;

	const Frame *frames = (const Frame *)getSegment(data.word(kFrsegment)).ptr(0, 0);
	SetObject *setEntries = (SetObject *)getSegment(data.word(kSetdat)).ptr(0, 128 * sizeof(SetObject));
	for (size_t i = 0; i < 128; ++i) {
		SetObject *setEntry = setEntries + i;
		if (getMapAd(setEntry->mapad) == 0)
			continue;
		uint8 currentFrame = setEntry->frames[0];
		if (currentFrame == 0xff)
			continue;
		calcFrFrame(currentFrame);
		uint16 x, y;
		finalFrame(&x, &y);
		setEntry->index = setEntry->frames[0];
		if ((setEntry->type == 0) && (setEntry->priority != 5) && (setEntry->priority != 6)) {
			x += data.word(kMapadx);
			y += data.word(kMapady);
			showFrame(frames, x, y, currentFrame, 0);
		} else
			makeBackOb(setEntry);

		ObjPos *objPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(ObjPos));
		objPos->xMin = data.byte(kSavex);
		objPos->yMin = data.byte(kSavey);
		objPos->xMax = data.byte(kSavex) + data.byte(kSavesize+0);
		objPos->yMax = data.byte(kSavey) + data.byte(kSavesize+1);
		objPos->index = i;
		data.word(kListpos) += sizeof(ObjPos);
	}
}

void DreamGenContext::getDimension() {
	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
	cl = mapXstart;
	ch = mapYstart;
	dl = mapXsize;
	dh = mapYsize;
}

bool DreamGenContext::addAlong(const uint8 *mapFlags) {
	for (size_t i = 0; i < 11; ++i) {
		if (mapFlags[3 * i] != 0)
			return true;
	}
	return false;
}

bool DreamGenContext::addLength(const uint8 *mapFlags) {
	for (size_t i = 0; i < 10; ++i) {
		if (mapFlags[3 * 11 * i] != 0)
			return true;
	}
	return false;
}

void DreamGenContext::getDimension(uint8 *mapXstart, uint8 *mapYstart, uint8 *mapXsize, uint8 *mapYsize) {
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

void DreamGenContext::calcMapAd() {
	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
	data.word(kMapadx) = data.word(kMapoffsetx) - 8 * (mapXsize + 2 * mapXstart - 11);
	data.word(kMapady) = data.word(kMapoffsety) - 8 * (mapYsize + 2 * mapYstart - 10);
}

void DreamGenContext::showAllFree() {
	data.word(kListpos) = kFreelist;
	ObjPos *listPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(kFreelist, 80 * sizeof(ObjPos));
	memset(listPos, 0xff, 80 * sizeof(ObjPos));

	data.word(kFrsegment) = data.word(kFreeframes);
	data.word(kDataad) = kFrframedata;
	data.word(kFramesad) = kFrframes;
	data.byte(kCurrentfree) = 0;
	const DynObject *freeObjects = (const DynObject *)getSegment(data.word(kFreedat)).ptr(0, 0);
	for (size_t i = 0; i < 80; ++i) {
		uint8 mapAd = getMapAd(freeObjects[i].mapad);
		if (mapAd != 0) {
			uint8 width, height;
			uint16 currentFrame = 3 * data.byte(kCurrentfree);
			calcFrFrame(currentFrame, &width, &height);
			uint16 x, y;
			finalFrame(&x, &y);
			if ((width != 0) || (height != 0)) {
				x += data.word(kMapadx);
				y += data.word(kMapady);
				assert(currentFrame < 256);
				showFrame((Frame *)getSegment(data.word(kFrsegment)).ptr(0, 0), x, y, currentFrame, 0);
				ObjPos *objPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(ObjPos));
				objPos->xMin = data.byte(kSavex);
				objPos->yMin = data.byte(kSavey);
				objPos->xMax = data.byte(kSavex) + data.byte(kSavesize+0);
				objPos->yMax = data.byte(kSavey) + data.byte(kSavesize+1);
				objPos->index = i;
				data.word(kListpos) += sizeof(ObjPos);
			}
		}

		++data.byte(kCurrentfree);
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

void DreamGenContext::showAllEx() {
	data.word(kListpos) = kExlist;
	memset(getSegment(data.word(kBuffers)).ptr(kExlist, 100 * 5), 0xff, 100 * 5);

	data.word(kFrsegment) = data.word(kExtras);
	data.word(kDataad) = kExframedata;
	data.word(kFramesad) = kExframes;
	data.byte(kCurrentex) = 0;
	DynObject *objects = (DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, sizeof(DynObject));
	for (size_t i = 0; i < 100; ++i, ++data.byte(kCurrentex)) {
		DynObject *object = objects + i;
		if (object->mapad[0] == 0xff)
			continue;
		if (object->currentLocation != data.byte(kReallocation))
			continue;
		if (getMapAd(object->mapad) == 0)
			continue;
		uint8 width, height;
		uint16 currentFrame = 3 * data.byte(kCurrentex);
		calcFrFrame(currentFrame, &width, &height);
		uint16 x, y;
		finalFrame(&x, &y);
		if ((width != 0) || (height != 0)) {
			assert(currentFrame < 256);
			showFrame((Frame *)getSegment(data.word(kFrsegment)).ptr(0, 0), x + data.word(kMapadx), y + data.word(kMapady), currentFrame, 0);
			ObjPos *objPos = (ObjPos *)getSegment(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(ObjPos));
			objPos->xMin = data.byte(kSavex);
			objPos->yMin = data.byte(kSavey);
			objPos->xMax = data.byte(kSavesize + 0) + data.byte(kSavex);
			objPos->yMax = data.byte(kSavesize + 1) + data.byte(kSavey);
			objPos->index = i;
			data.word(kListpos) += sizeof(ObjPos);
		}
	}
}

} // End of namespace DreamGen
