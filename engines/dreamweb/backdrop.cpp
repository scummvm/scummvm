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
#include "engines/util.h"
#include "graphics/surface.h"

namespace DreamGen {

void DreamGenContext::doblocks() {
	uint16 dstOffset = data.word(kMapady) * 320 + data.word(kMapadx);
	uint16 mapOffset = kMap + data.byte(kMapy) * kMapwidth + data.byte(kMapx);
	ds = data.word(kMapdata);
	const uint8 *mapData = ds.ptr(mapOffset, 0);
	ds = data.word(kBackdrop);
	const uint8 *blocks = ds.ptr(kBlocks, 0);
	es = data.word(kWorkspace);
	uint8 *dstBuffer = es.ptr(dstOffset, 0);

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

uint8 DreamGenContext::getxad(const uint8 *setData, uint8 *result) {
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

uint8 DreamGenContext::getyad(const uint8 *setData, uint8 *result) {
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

void DreamGenContext::getmapad() {
	ch = getmapad((const uint8 *)es.ptr(si, 5));
}

uint8 DreamGenContext::getmapad(const uint8 *setData) {
	uint8 xad, yad;
	if (getxad(setData, &xad) == 0)
		return 0;
	data.word(kObjectx) = xad;
	if (getyad(setData, &yad) == 0)
		return 0;
	data.word(kObjecty) = yad;
	return 1;
}

void DreamGenContext::calcfrframe() {
	const Frame *frame = (const Frame *)segRef(data.word(kFrsegment)).ptr(data.word(kCurrentframe) * sizeof(Frame), sizeof(Frame));
	data.word(kSavesource) = data.word(kFramesad) + frame->ptr();
	data.byte(kSavesize+0) = frame->width;
	data.byte(kSavesize+1) = frame->height;
	data.word(kOffsetx) = frame->x;
	data.word(kOffsety) = frame->y;
}

void DreamGenContext::finalframe() {
	uint16 x, y;
	finalframe(&x, &y);
	di = x;
	bx = y;
}

void DreamGenContext::finalframe(uint16 *x, uint16 *y) {
	data.byte(kSavex) = (data.word(kObjectx) + data.word(kOffsetx)) & 0xff;
	data.byte(kSavey) = (data.word(kObjecty) + data.word(kOffsety)) & 0xff;
	*x = data.word(kObjectx);
	*y = data.word(kObjecty);
}

void DreamGenContext::showallobs() {
	data.word(kListpos) = kSetlist;
	memset(segRef(data.word(kBuffers)).ptr(kSetlist, 0), 0xff, 128 * 5);
	data.word(kFrsegment) = data.word(kSetframes);
	data.word(kDataad) = kFramedata;
	data.word(kFramesad) = kFrames;

	const Frame *frames = (const Frame *)segRef(data.word(kFrsegment)).ptr(0, 0);
	ObjData *setEntries = (ObjData *)segRef(data.word(kSetdat)).ptr(0, 128 * sizeof(ObjData));
	for (size_t i = 0; i < 128; ++i) {
		ObjData *setEntry = setEntries + i;
		if (getmapad(setEntry->b58) == 0)
			continue;
		uint8 currentFrame = setEntry->b18[0];
		data.word(kCurrentframe) = currentFrame;
		if (currentFrame == 0xff)
			continue;
		calcfrframe();
		uint16 x, y;
		finalframe(&x, &y);
		setEntry->b17 = setEntry->b18[0];
		if ((setEntry->type == 0) && (setEntry->priority != 5) && (setEntry->priority != 6)) {
			x += data.word(kMapadx);
			y += data.word(kMapady);
			uint8 width, height;
			showframe(frames, x, y, data.word(kCurrentframe), 0, &width, &height);
		} else
			makebackob(setEntry);

		ObjPos *objPos = (ObjPos *)segRef(data.word(kBuffers)).ptr(data.word(kListpos), sizeof(ObjPos));
		objPos->xMin = data.byte(kSavex);
		objPos->yMin = data.byte(kSavey);
		objPos->xMax = data.byte(kSavex) + data.byte(kSavesize+0);
		objPos->yMax = data.byte(kSavey) + data.byte(kSavesize+1);
		objPos->index = i;
		data.word(kListpos) += sizeof(ObjPos);
	}
}

} /*namespace dreamgen */

