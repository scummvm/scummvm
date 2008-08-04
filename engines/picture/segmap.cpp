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
		maskRect.ybottom = READ_LE_UINT16(source + 10);

		debug(0, "SegmentMap::load() (%d, %d, %d, %d, %04X, %d)",
			maskRect.x, maskRect.y, maskRect.width, maskRect.height, maskRect.maskOffset, maskRect.ybottom);

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
		pathRect.y = READ_LE_UINT16(source);
		pathRect.x = READ_LE_UINT16(source + 2);
		pathRect.height = READ_LE_UINT16(source + 4);
		pathRect.width = READ_LE_UINT16(source + 6);
		
		debug(0, "SegmentMap::load() (%d, %d, %d, %d)", pathRect.x, pathRect.y, pathRect.width, pathRect.height);

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

int SegmentMap::findPathRectAtPoint(int x, int y) {
	for (uint rectIndex = 0; rectIndex < _pathRects.size(); rectIndex++) {
		if (y >= _pathRects[rectIndex].y && y <= _pathRects[rectIndex].y + _pathRects[rectIndex].height &&
			x >= _pathRects[rectIndex].x && x <= _pathRects[rectIndex].x + _pathRects[rectIndex].width) {
			return rectIndex;
		}
	}
	return -1;
}

void SegmentMap::adjustPathPoint(int x, int y) {

	if (findPathRectAtPoint(x, y) != -1)
		return;

	uint32 minDistance = 0xFFFFFFFF, distance;
	int x2, y2;
	
	for (uint rectIndex = 0; rectIndex < _pathRects.size(); rectIndex++) {

		if (ABS(x - _pathRects[rectIndex].x) >= ABS((x - (_pathRects[rectIndex].x + _pathRects[rectIndex].width)))) {
			x2 = _pathRects[rectIndex].x + _pathRects[rectIndex].width;
		} else {
			x2 = _pathRects[rectIndex].x;
		}

		if (ABS(y - _pathRects[rectIndex].y) >= ABS((y - (_pathRects[rectIndex].y + _pathRects[rectIndex].height)))) {
			y2 = _pathRects[rectIndex].y + _pathRects[rectIndex].height;
		} else {
			y2 = _pathRects[rectIndex].y;
		}

		if (x >= _pathRects[rectIndex].x && x < _pathRects[rectIndex].x + _pathRects[rectIndex].width) {
			x2 = x;
		}

		distance = ABS(y - y2) + ABS(x - x2);
		if (distance < minDistance) {
			if (y >= _pathRects[rectIndex].y && y <= _pathRects[rectIndex].y + _pathRects[rectIndex].height)
				_y = y;
			else
				_y = y2;
			if (x >= _pathRects[rectIndex].x && x <= _pathRects[rectIndex].x + _pathRects[rectIndex].width)
				_x = x;
			else
				_x = x2;
			minDistance = distance;
		}

	}

}

int SegmentMap::findNextPathRect(int srcRectIndex) {

	uint v28;
	int result;
	int minDistance, distance;
	int x1, y1, x2, y2;
	int nx1, nx2, nx3;
	int ny, ny2, ny3;

	result = -1;
	minDistance = 65535;

	x1 = _pathRects[srcRectIndex].x;
	y1 = _pathRects[srcRectIndex].y;

	x2 = x1 + _pathRects[srcRectIndex].width;
	y2 = y1 + _pathRects[srcRectIndex].height;

	for (uint rectIndex = 0; rectIndex < _pathRects.size(); ++rectIndex) {

		if ( y1 == _pathRects[rectIndex].height + _pathRects[rectIndex].y && x1 < _pathRects[rectIndex].x + _pathRects[rectIndex].width && x2 > _pathRects[rectIndex].x ) {
			ny = y1;
			
LABEL_28:
			if ( x1 >= _pathRects[rectIndex].x ) {
				nx1 = x1;
			} else {
				nx1 = _pathRects[rectIndex].x;
			}
			if ( x2 <= _pathRects[rectIndex].x + _pathRects[rectIndex].width ) {
				nx2 = x2 - 1;
			} else {
				nx2 = _pathRects[rectIndex].x + _pathRects[rectIndex].width - 1;
			}
			if ( ABS(_x - nx1) >= ABS(_x - nx2) ) {
				nx3 = nx2 - 1;
			} else {
				nx3 = nx1;
			}
			if ( _x > nx1 && _x < nx2 ) {
				nx3 = _x;
			}
			goto LABEL_55;
		}
		if ( y2 == _pathRects[rectIndex].y && x1 < _pathRects[rectIndex].x + _pathRects[rectIndex].width && x2 > _pathRects[rectIndex].x ) {
			ny = y2 - 1;
			goto LABEL_28;
		}
		if ( x1 == _pathRects[rectIndex].x + _pathRects[rectIndex].width && y1 < _pathRects[rectIndex].y + _pathRects[rectIndex].height && y2 > _pathRects[rectIndex].y ) {
			nx3 = x1;
		} else {
			if ( x2 != _pathRects[rectIndex].x || y1 >= _pathRects[rectIndex].y + _pathRects[rectIndex].height || y2 <= _pathRects[rectIndex].y )
				continue;
			nx3 = x2 - 1;
		}
		if ( y1 >= _pathRects[rectIndex].y ) {
			ny3 = y1;
		} else {
			ny3 = _pathRects[rectIndex].y;
		}
		if ( y2 <= _pathRects[rectIndex].y + _pathRects[rectIndex].height ) {
			ny2 = y2 - 1;
		} else {
			ny2 = _pathRects[rectIndex].y + _pathRects[rectIndex].height - 1;
		}
		if ( ABS(_y - ny3) >= ABS(_y - ny2) ) {
			ny = ny2 - 1;
		} else {
			ny = ny3;
		}
		if ( _y > ny3 && _y < ny2 ) {
			ny = _y;
		}
		
LABEL_55:
		distance = ABS(_x - nx3) + ABS(_y - ny);
		v28 = 0;
		while ( v28 < _rectIndexArray2Count ) {
			if ( rectIndex == _rectIndexArray2[v28] ) {
				distance = minDistance;
				break;
			}
			++v28;
		}

		v28 = 0;
		while ( v28 < _rectIndexArray1Count ) {
			if ( rectIndex == _rectIndexArray1[v28] ) {
				distance = minDistance;
				break;
			}
			++v28;
		}

		if ( distance < minDistance ) {
			result = rectIndex;
			minDistance = distance;
			_pointsArray[_pointsCount].y = ny;
			_pointsArray[_pointsCount].x = nx3;
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

void SegmentMap::findPath(int16 *pointsArray, int destX, int destY, int x, int y) {

	int index;
	int sourceRectIndex, destRectIndex;
	int pointsCount;

	pointsCount = 2;
	index = 0;

	debug(0, "SegmentMap::findPath(fromX: %d; fromY: %d; toX: %d; toY: %d)", x, y, destX, destY);

	sourceRectIndex = findPathRectAtPoint(x, y);
	if ( sourceRectIndex == -1 ) {
		adjustPathPoint(x, y);
		x = _x;
		y = _y;
	}

	_rectIndexArray1Count = 0;
	_rectIndexArray2Count = 0;
	_pointsCount = 0;

	_x = destX;
	_y = destY;

	adjustPathPoint(_x, _y);
	destRectIndex = findPathRectAtPoint(_x, _y);
	sourceRectIndex = findPathRectAtPoint(x, y);
	if ( sourceRectIndex != -1 ) {
		if ( destRectIndex != sourceRectIndex ) {
			while ( 1 ) {
				do {
					_rectIndexArray2[_rectIndexArray2Count++] = sourceRectIndex;
					sourceRectIndex = findNextPathRect(sourceRectIndex);
					_pointsCount++;
				} while ( sourceRectIndex != -1 && sourceRectIndex != destRectIndex );
				if ( sourceRectIndex != -1 && sourceRectIndex == destRectIndex )
					break;
				_rectIndexArray1[_rectIndexArray1Count++] = _rectIndexArray2[--_rectIndexArray2Count];
				_pointsCount -= 2;
				sourceRectIndex = _rectIndexArray2[--_rectIndexArray2Count];
			}
			sourceRectIndex = 0;
			while ( sourceRectIndex < _pointsCount ) {
				pointsArray[pointsCount++] = _pointsArray[sourceRectIndex].y;
				pointsArray[pointsCount++] = _pointsArray[sourceRectIndex].x;
				index++;
				sourceRectIndex++;
			}
		}

		pointsArray[pointsCount++] = _y;
		pointsArray[pointsCount++] = _x;

		pointsArray[0] = 0;
		pointsArray[1] = index + 1;
	}
	
	debug(0, "SegmentMap::findPath() count = %d", pointsArray[1]);

	/*
	int sx = x, sy = y;
	LineData ld;
	ld.pitch = _vm->_sceneWidth;
	ld.surf = _vm->_screen->_backScreen;
	for (int16 i = 0; i < pointsArray[1] * 2; i+=2) {
	    debug(0, "x = %d; y = %d", pointsArray[3+i], pointsArray[2+i]);
	    Graphics::drawLine(sx, sy, pointsArray[3+i], pointsArray[2+i], 0xFF, plotProc, &ld);
	    sx = pointsArray[3+i];
		sy = pointsArray[2+i];
	}
	*/
	
}

int8 SegmentMap::getScalingAtPoint(int16 x, int16 y) {
	int8 scaling = 0;
	for (uint i = 0; i < _infoRects.size(); i++) {
		if (_infoRects[i].id == 0 &&
			y >= _infoRects[i].y && y <= _infoRects[i].y + _infoRects[i].height &&
			x >= _infoRects[i].x && x <= _infoRects[i].x + _infoRects[i].width) {
			char topScaling = (char)_infoRects[i].b;
			char bottomScaling = (char)_infoRects[i].c;
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
    if ( *(__int16 *)((char *)&spriteDrawList[0].y2 + v5) <= (unsigned __int16)v3->ybottom )
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
	
	    if (sprite->ybottom <= _maskRects[i].ybottom) {
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
			Common::Rect(pathRect.x, pathRect.y, pathRect.x + pathRect.width, pathRect.y + pathRect.height),
			255);
	}
}

} // End of namespace Picture
