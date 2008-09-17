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

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"
#include "common/stream.h"

#include "graphics/primitives.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/resource.h"
#include "picture/screen.h"
#include "picture/segmap.h"

namespace Picture {

SegmentMap::SegmentMap(PictureEngine *vm) : _vm(vm) {
	_maskRectData = new byte[32768];
}

SegmentMap::~SegmentMap() {
	delete[] _maskRectData;
}

void SegmentMap::load(byte *source) {

	// TODO: Use MemoryReadStream

	_maskRects.clear();
	_pathRects.clear();
	_infoRects.clear();

	// Load mask rects
	uint16 maskSize = READ_LE_UINT16(source);
	source += 2;
	uint16 maskRectCount = READ_LE_UINT16(source);
	source += 2;
	uint16 maskRectDataSize = maskRectCount * 12 + 2;
	
	debug(0, "SegmentMap::load() maskRectCount = %d", maskRectCount);

	for (uint16 i = 0; i < maskRectCount; i++) {
		SegmapMaskRect maskRect;
		maskRect.y = READ_LE_UINT16(source);
		maskRect.x = READ_LE_UINT16(source + 2);
		maskRect.height = READ_LE_UINT16(source + 4);
		maskRect.width = READ_LE_UINT16(source + 6);
		maskRect.maskOffset = READ_LE_UINT16(source + 8);
		maskRect.maskOffset -= maskRectDataSize;
		maskRect.priority = READ_LE_UINT16(source + 10);

		debug(0, "SegmentMap::load() (%d, %d, %d, %d, %04X, %d)",
			maskRect.x, maskRect.y, maskRect.width, maskRect.height, maskRect.maskOffset, maskRect.priority);

		source += 12;
		_maskRects.push_back(maskRect);
	}

	memcpy(_maskRectData, source, maskSize - maskRectDataSize);
	source += maskSize - maskRectDataSize;
	
	// Load path rects

	source += 2; // skip rects array size
	
	uint16 pathRectCount = READ_LE_UINT16(source);
	source += 2;
	
	debug(0, "SegmentMap::load() pathRectCount = %d", pathRectCount);
	
	for (uint16 i = 0; i < pathRectCount; i++) {
		SegmapPathRect pathRect;
		pathRect.y1 = READ_LE_UINT16(source);
		pathRect.x1 = READ_LE_UINT16(source + 2);
		pathRect.y2 = pathRect.y1 + READ_LE_UINT16(source + 4);
		pathRect.x2 = pathRect.x1 + READ_LE_UINT16(source + 6);
		
		debug(0, "SegmentMap::load() (%d, %d, %d, %d)", pathRect.x1, pathRect.y1, pathRect.x2, pathRect.y2);

		source += 8;
		_pathRects.push_back(pathRect);
	}
	
	// Load info rects

	source += 2; // skip rects array size

	uint16 infoRectCount = READ_LE_UINT16(source);
	source += 2;
	debug(0, "SegmentMap::load() infoRectCount = %d", infoRectCount);
	for (uint16 i = 0; i < infoRectCount; i++) {
		SegmapInfoRect infoRect;
		infoRect.y = READ_LE_UINT16(source);
		infoRect.x = READ_LE_UINT16(source + 2);
		infoRect.height = READ_LE_UINT16(source + 4);
		infoRect.width = READ_LE_UINT16(source + 6);
		infoRect.id = source[8];
		infoRect.a = source[9];
		infoRect.b = source[10];
		infoRect.c = source[11];

		debug(0, "SegmentMap::load() (%d, %d, %d, %d) (%d, %d, %d, %d)",
			infoRect.x, infoRect.y, infoRect.width, infoRect.height,
			infoRect.id, (int8)infoRect.a, (int8)infoRect.b, (int8)infoRect.c);

		source += 12;
		_infoRects.push_back(infoRect);
	}

	// TODO Other stuff


}

int16 SegmentMap::findPathRectAtPoint(int16 x, int16 y) {
	for (int16 rectIndex = 0; rectIndex < (int16)_pathRects.size(); rectIndex++) {
		if (y >= _pathRects[rectIndex].y1 && y <= _pathRects[rectIndex].y2 &&
			x >= _pathRects[rectIndex].x1 && x <= _pathRects[rectIndex].x2) {
			return rectIndex;
		}
	}
	return -1;
}

void SegmentMap::adjustPathPoint(int16 &x, int16 &y) {

	if (findPathRectAtPoint(x, y) != -1)
		return;

	uint32 minDistance = 0xFFFFFFFF, distance;
	int16 adjustedX, adjustedY, x2, y2;
	
	for (int16 rectIndex = 0; rectIndex < (int16)_pathRects.size(); rectIndex++) {

		if (x >= _pathRects[rectIndex].x1 && x < _pathRects[rectIndex].x2) {
			x2 = x;
		} else if (ABS(x - _pathRects[rectIndex].x1) >= ABS(x - _pathRects[rectIndex].x2)) {
			x2 = _pathRects[rectIndex].x2;
		} else {
			x2 = _pathRects[rectIndex].x1;
		}

		if (ABS(y - _pathRects[rectIndex].y1) >= ABS(y - _pathRects[rectIndex].y2)) {
			y2 = _pathRects[rectIndex].y2;
		} else {
			y2 = _pathRects[rectIndex].y1;
		}

		distance = ABS(y - y2) + ABS(x - x2);
		if (distance < minDistance) {
			if (x >= _pathRects[rectIndex].x1 && x <= _pathRects[rectIndex].x2) {
				adjustedX = x;
			} else {
				adjustedX = x2;
			}
			if (y >= _pathRects[rectIndex].y1 && y <= _pathRects[rectIndex].y2) {
				adjustedY = y;
			} else {
				adjustedY = y2;
			}
			minDistance = distance;
		}

	}
	
	x = adjustedX;
	y = adjustedY;

}

int16 SegmentMap::findNextPathRect(int16 srcRectIndex, int16 destX, int16 destY) {

	int16 result;
	uint16 minDistance, distance;
	int16 x1, y1, x2, y2;
	int16 xmin, xmax, ymax, ymin;

	result = -1;
	minDistance = 0xFFFF;

	x1 = _pathRects[srcRectIndex].x1;
	y1 = _pathRects[srcRectIndex].y1;
	x2 = _pathRects[srcRectIndex].x2;
	y2 = _pathRects[srcRectIndex].y2;

	for (int16 rectIndex = 0; rectIndex < (int16)_pathRects.size(); rectIndex++) {

		int16 nodeX = -1, nodeY = -1;

		// Check if the current rectangle is connected to the source rectangle
		if (x1 == _pathRects[rectIndex].x2 && y1 < _pathRects[rectIndex].y2 && y2 > _pathRects[rectIndex].y1) {
			nodeX = x1;
		} else if (x2 == _pathRects[rectIndex].x1 && y1 < _pathRects[rectIndex].y2 && y2 > _pathRects[rectIndex].y1) {
			nodeX = x2 - 1;
		} else if (y1 == _pathRects[rectIndex].y2 && x1 < _pathRects[rectIndex].x2 && x2 > _pathRects[rectIndex].x1) {
			nodeY = y1;
		} else if (y2 == _pathRects[rectIndex].y1 && x1 < _pathRects[rectIndex].x2 && x2 > _pathRects[rectIndex].x1) {
			nodeY = y2 - 1;
		} else
			continue;

		if (nodeX == -1) {
			xmin = MAX<int16>(x1, _pathRects[rectIndex].x1);
			xmax = MIN<int16>(x2, _pathRects[rectIndex].x2) - 1;
			if (destX > xmin && destX < xmax) {
				nodeX = destX;
			} else if (ABS(destX - xmin) >= ABS(destX - xmax)) {
				nodeX = xmax - 1;
			} else {
				nodeX = xmin;
			}
		}

		if (nodeY == -1) {
			ymin = MAX<int16>(y1, _pathRects[rectIndex].y1);
			ymax = MIN<int16>(y2, _pathRects[rectIndex].y2) - 1;
			if (destY > ymin && destY < ymax) {
				nodeY = destY;
			} else if (ABS(destY - ymin) >= ABS(destY - ymax)) {
				nodeY = ymax - 1;
			} else {
				nodeY = ymin;
			}
		}

		distance = ABS(destX - nodeX) + ABS(destY - nodeY);

		for (uint i = 0; i < _closedPathRectsCount; i++) {
			if (rectIndex == _closedPathRects[i]) {
				distance = minDistance;
				break;
			}
		}

		for (uint i = 0; i < _deadEndPathRectsCount; i++) {
			if (rectIndex == _deadEndPathRects[i]) {
				distance = minDistance;
				break;
			}
		}

		if (distance < minDistance) {
			result = rectIndex;
			minDistance = distance;
			_pathNodes[_pathNodesCount].x = nodeX;
			_pathNodes[_pathNodesCount].y = nodeY;
		}

	}

	return result;
}

struct LineData {
	int pitch;
	byte *surf;
};

void plotProc(int x, int y, int color, void *data) {
	LineData *ld = (LineData*)data;
	ld->surf[x + y * ld->pitch] = color;
}

void SegmentMap::findPath(int16 *pointsArray, int16 destX, int16 destY, int16 sourceX, int16 sourceY) {

	// TODO: Writes to pointsArray aren't endian-safe yet

	int16 currentRectIndex, destRectIndex;
	int16 pointsCount;

	debug(0, "SegmentMap::findPath(fromX: %d; fromY: %d; toX: %d; toY: %d)", sourceX, sourceY, destX, destY);

	_deadEndPathRectsCount = 0;
	_closedPathRectsCount = 0;
	_pathNodesCount = 0;

	pointsCount = 2;

    adjustPathPoint(sourceX, sourceY);
	currentRectIndex = findPathRectAtPoint(sourceX, sourceY);

	adjustPathPoint(destX, destY);
	destRectIndex = findPathRectAtPoint(destX, destY);

	if (currentRectIndex != -1) {
		if (destRectIndex != currentRectIndex) {
			while (1) {
				do {
					_closedPathRects[_closedPathRectsCount++] = currentRectIndex;
					currentRectIndex = findNextPathRect(currentRectIndex, destX, destY);
					_pathNodesCount++;
				} while (currentRectIndex != -1 && currentRectIndex != destRectIndex);
				if (currentRectIndex != -1 && currentRectIndex == destRectIndex)
					break;
				_deadEndPathRects[_deadEndPathRectsCount++] = _closedPathRects[--_closedPathRectsCount];
				_pathNodesCount -= 2;
				currentRectIndex = _closedPathRects[--_closedPathRectsCount];
			}
			for (int16 i = 0; i < _pathNodesCount; i++) {
				pointsArray[pointsCount++] = _pathNodes[i].y;
				pointsArray[pointsCount++] = _pathNodes[i].x;
			}
		}

		pointsArray[pointsCount++] = destY;
		pointsArray[pointsCount++] = destX;

		pointsArray[0] = 0;
		pointsArray[1] = _pathNodesCount + 1;
	}
	
	debug(0, "SegmentMap::findPath() count = %d", pointsArray[1]);

#if 0 // DEBUG: Draw the path we found
	int sx = sourceX, sy = sourceY;
	LineData ld;
	ld.pitch = _vm->_sceneWidth;
	ld.surf = _vm->_screen->_backScreen;
	for (int16 i = 0; i < pointsArray[1] * 2; i+=2) {
		debug(0, "x = %d; y = %d", pointsArray[3+i], pointsArray[2+i]);
		Graphics::drawLine(sx, sy, pointsArray[3+i], pointsArray[2+i], 0xFF, plotProc, &ld);
		sx = pointsArray[3+i];
		sy = pointsArray[2+i];
	}
#endif
	
}

int8 SegmentMap::getScalingAtPoint(int16 x, int16 y) {
	int8 scaling = 0;
	for (uint i = 0; i < _infoRects.size(); i++) {
		if (_infoRects[i].id == 0 &&
			y >= _infoRects[i].y && y <= _infoRects[i].y + _infoRects[i].height &&
			x >= _infoRects[i].x && x <= _infoRects[i].x + _infoRects[i].width) {

			int8 topScaling = (int8)_infoRects[i].b;
			int8 bottomScaling = (int8)_infoRects[i].c;
			if (y - _infoRects[i].y > 0) {
				scaling = (ABS(y - _infoRects[i].y) * (bottomScaling - topScaling) / _infoRects[i].height) + topScaling;
			}
		}
	}
	return scaling;
}

void SegmentMap::getRgbModifiertAtPoint(int16 x, int16 y, int16 id, byte &r, byte &g, byte &b) {
	r = 0;
	g = 0;
	b = 0;
	for (uint i = 0; i < _infoRects.size(); i++) {
		if (_infoRects[i].id == id &&
			y >= _infoRects[i].y && y <= _infoRects[i].y + _infoRects[i].height &&
			x >= _infoRects[i].x && x <= _infoRects[i].x + _infoRects[i].width) {

			r = _infoRects[i].a;
			g = _infoRects[i].b;
			b = _infoRects[i].c;
		}
	}
	debug(0, "SegmentMap::getRgbModifiertAtPoint() r: %d; g: %d; b: %d", r, g, b);
}

void SegmentMap::restoreMasksBySprite(SpriteDrawItem *sprite) {
	// TODO: This needs more optimization
	for (uint i = 0; i < _maskRects.size(); i++) {
	
#if 0
	if ( *(__int16 *)((char *)&spriteDrawList[0].y2 + v5) <= (unsigned __int16)v3->priority )
	{
	  if ( (unsigned __int16)(*(__int16 *)((char *)&spriteDrawList[0].height + v5)
							+ *(__int16 *)((char *)&spriteDrawList[0].y + v5)) > v3->y )
	  {
		if ( (unsigned __int16)(v3->height + v3->y) > *(__int16 *)((char *)&spriteDrawList[0].y + v5) )
		{
		  if ( (unsigned __int16)(*(__int16 *)((char *)&spriteDrawList[0].width + v5)
								+ *(__int16 *)((char *)&spriteDrawList[0].x + v5)) > v3->x )
		  {
			if ( (unsigned __int16)(v3->width + v3->x) > *(__int16 *)((char *)&spriteDrawList[0].x + v5) )
			{

#endif
	
		if (sprite->priority <= _maskRects[i].priority) {
			restoreMask(i);
		}
	}

}

void SegmentMap::restoreMask(int16 index) {
	// TODO: This needs more optimization
	SegmapMaskRect *maskRect = &_maskRects[index];

	int16 maskX = maskRect->x, maskY = maskRect->y;
	int16 skipX = 0;
	int16 x = maskRect->x - _vm->_cameraX;
	int16 y = maskRect->y - _vm->_cameraY;
	int16 width = maskRect->width;
	int16 height = maskRect->height;
	byte *mask = _maskRectData + maskRect->maskOffset;

	debug(0, "SegmentMap::restoreMask() screenX = %d; screenY = %d; maskX = %d; maskY = %d",
		x, y, maskRect->x, maskRect->y);

	// Not on screen, skip
	if (x + width < 0 || y + height < 0 || x >= 640 || y >= _vm->_cameraHeight)
		return;

	if (x < 0) {
		skipX = -x;
		x = 0;
	}

	if (y < 0) {
		int16 skipY = -y;
		for (int16 h = 0; h < skipY; h++) {
			int16 w = width;
			while (w > 0) {
				w -= (*mask++) & 0x7F;
			}
		}
		y = 0;
		height -= skipY;
		maskY += skipY;
	}

	if (x + width >= 640) {
		width -= x + width - 640;
	}

	if (y + height >= _vm->_cameraHeight) {
		height -= y + height - _vm->_cameraHeight;
	}

	byte *backScreen = _vm->_screen->_backScreen + maskX + (maskY * _vm->_sceneWidth);
	byte *frontScreen = _vm->_screen->_frontScreen + x + (y * 640);

	for (int16 h = 0; h < height; h++) {
		byte *src = backScreen;
		byte *dst = frontScreen;
		byte maskLine[640], *maskLineP = maskLine;

		int16 w = width;
		while (w > 0) {
			byte m = *mask++;
			byte count = m & 0x7F;
			if (m & 0x80)
				memset(maskLineP, 1, count);
			else
	   			memset(maskLineP, 0, count);
			maskLineP += count;
			w -= count;
		}

		src += skipX;
		for (int16 c = skipX; c < width; c++) {
			if (maskLine[c] == 1)
				*dst = *src;
			dst++;
			src++;
		}

		backScreen += _vm->_sceneWidth;
		frontScreen += 640;
	}
	
}

void SegmentMap::debugDrawRects(Graphics::Surface *surf) {
	for (uint16 i = 0; i < _pathRects.size(); i++) {
		SegmapPathRect pathRect = _pathRects[i];
		surf->frameRect(
			Common::Rect(pathRect.x1, pathRect.y1, pathRect.x2, pathRect.y2),
			255);
	}
}

} // End of namespace Picture
