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
#include "gob/sound.h"

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
		int16 handle;
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
		ImdCoord *frameCoords;
		int32 frameDataSize;
		int32 vidBufferSize;
		Video::Color *extraPalette;
	};

#include "common/pack-end.h"	// END STRUCT PACKING

	Imd *_curImd;
	byte _frontSurf;
	int8 _backSurf;
	byte *_frontMem;
	int32 _frameDelay;

	uint8 _soundStage; // (0: no sound, 1: loaded, 2: playing)

	ImdPlayer(GobEngine *vm);
	virtual ~ImdPlayer();

	Imd *loadImdFile(const char *path, SurfaceDesc *surfDesc, int8 flags);
	void finishImd(Imd *&imdPtr);

	int8 openImd(const char *path, int16 x, int16 y,
			int16 startFrame, int16 flags);
	void closeImd(void);

	void play(int16 frame, uint16 palCmd, int16 palStart, int16 palEnd,
			int16 palFrame, int16 lastFrame);
	void play(const char *path, int16 x, int16 y, bool interruptible);
	void play(const char *path, int16 x, int16 y, int16 startFrame,
			int16 frames, bool fade, bool interruptible);

protected:
	char _curFile[18];

	int16 _curX;
	int16 _curY;
	int16 _left;
	int16 _top;
	int16 _right;
	int16 _bottom;

	byte *_frameData;
	byte *_vidBuffer;

	bool _noSound;

	uint32 _soundStartTime;
	uint32 _skipFrames;

	int16 _soundFreq;
	uint16 _soundSliceSize;
	int16 _soundSlicesCount;
	uint16 _soundSliceLength;

	Audio::AppendableAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	GobEngine *_vm;

	void copyPalette(int16 palStart, int16 palEnd);
	void flipFrontMem();
	void drawFrame(int16 frame);
	void setXY(Imd *imdPtr, int16 x, int16 y);

	void seekFrame(Imd *imdPtr, int16 frame, int16 from, bool restart = false);
	uint16 checkFrameType(Imd *imdPtr, int16 frame);
	void drawFrame(Imd *imdPtr, int16 frame, int16 x, int16 y,
			SurfaceDesc *dest = 0);

	uint32 view(ImdPlayer::Imd *imdPtr, int16 arg_4);
	void renderFrame(Imd *imdPtr);
	void frameUncompressor(byte *dest, byte *src);

	void waitEndSoundSlice();
};

} // End of namespace Gob

#endif // GOB_IMD_H
