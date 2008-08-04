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

#ifndef PICTURE_SEGMAP_H
#define PICTURE_SEGMAP_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "graphics/surface.h"

#include "engines/engine.h"

#include "picture/screen.h"

namespace Picture {

struct ScriptWalk {
	int16 y, x;
	int16 y1, x1, y2, x2;
	int16 yerror, xerror;
	int16 mulValue;
	int16 scaling;
};

class SegmentMap {
public:
    SegmentMap(PictureEngine *vm);
    ~SegmentMap();

	void load(byte *source);
    
    int findPathRectAtPoint(int x, int y);
    void adjustPathPoint(int x, int y);
    
    void findPath(int16 *pointsArray, int destX, int destY, int x, int y);
    
    int8 getScalingAtPoint(int16 x, int16 y);
	void getRgbModifiertAtPoint(int16 x, int16 y, int16 id, byte &r, byte &g, byte &b);

	void restoreMasksBySprite(SpriteDrawItem *sprite);
	void restoreMask(int16 index);

	void debugDrawRects(Graphics::Surface *surf);

//protected:
public: // for debugging purposes

	struct SegmapMaskRect {
		int16 y, x;
		int16 height, width;
		int16 maskOffset;
		int16 ybottom;
	};

	struct SegmapPathRect {
		int16 y, x;
		int16 height, width;
	};
	
	struct SegmapInfoRect {
		int16 y, x;
		int16 height, width;
		byte id;
		byte a, b, c;
	};
	
	struct PathPoint {
	    int16 y, x;
	};
	
	typedef Common::Array<SegmapMaskRect> SegmapMaskRectArray;
	typedef Common::Array<SegmapPathRect> SegmapPathRectArray;
	typedef Common::Array<SegmapInfoRect> SegmapInfoRectArray;

    PictureEngine *_vm;
    
    SegmapMaskRectArray _maskRects;
    byte *_maskRectData;

    SegmapPathRectArray _pathRects;
    SegmapInfoRectArray _infoRects;

	int _rectIndexArray1[1000];
	uint _rectIndexArray1Count;

	int _rectIndexArray2[1000];
	uint _rectIndexArray2Count;

	PathPoint _pointsArray[1000];
	int16 _pointsCount;

    int _x, _y;
    
    int findNextPathRect(int srcRectIndex);
    
};

} // End of namespace Picture

#endif /* PICTURE_SEGMAP_H */
