/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_IMD_H
#define GOB_IMD_H

#include "gob/video.h"

namespace Gob {

class ImdPlayer {
public:

#include "common/pack-start.h"	// START STRUCT PACKING

	struct ImdCoord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	};

	struct Imd {
		int16 fileHandle;
		int16 verMin;
		int16 framesCount;
		int16 x;
		int16 y;
		int16 width;
		int16 height;
		int16 field_E;
		int16 curFrame;
		Video::Color *palette;
		SurfaceDesc *surfDesc;
		int32 *framesPos;
		int32 firstFramePos;
		int16 stdX;
		int16 stdY;
		int16 stdWidth;
		int16 stdHeight;
		int32 filePos;
		ImdCoord *frameCoords;
		int32 frameDataSize;
		int32 vidBufferSize;
	};

#include "common/pack-end.h"	// END STRUCT PACKING

	Imd *_curImd;
	byte _frontSurf;
	int8 _backSurf;
	byte *_frontMem;
	int32 _frameDelay;

	ImdPlayer(GobEngine *vm);
	virtual ~ImdPlayer();

	Imd *loadImdFile(const char *path, SurfaceDesc *surfDesc, int8 flags);
	void finishImd(Imd *imdPtr);
	int8 openImd(const char *path, int16 x, int16 y, int16 repeat, int16 flags);
	void closeImd(void);
	void setXY(Imd *imdPtr, int16 x, int16 y);

	void play(int16 arg_0, uint16 palCmd, int16 palStart,
			int16 playEnd, int16 palFrame, int16 arg_A);
	void play(const char *path, int16 x, int16 y, int16 startFrame,
			int16 frames, bool fade, bool interruptible);
	int16 view(ImdPlayer::Imd *imdPtr, int16 arg_4);
	void drawFrame(Imd *imdPtr, int16 frame, int16 x, int16 y,
			SurfaceDesc *dest = 0);
	void renderframe(Imd *imdPtr);
	void frameUncompressor(byte *dest, byte *src);
	int16 sub_2C825(Imd *imdPtr);

protected:
	char _curFile[15];

	int16 _curX;
	int16 _curY;

	uint16 _frameDataSize;
	uint16 _vidBufferSize;
	byte *_frameData;
	byte *_vidBuffer;

	GobEngine *_vm;
};

} // End of namespace Gob

#endif // GOB_IMD_H
