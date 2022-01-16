/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Plays films within a scene, takes into account the actor in each 'column'.
 */

#ifndef TINSEL_BMV_H
#define TINSEL_BMV_H

#include "common/coroutines.h"
#include "common/file.h"

#include "audio/mixer.h"

#include "tinsel/object.h"
#include "tinsel/palette.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Tinsel {

typedef enum
{
	BMV_OP_DELTA = 0,
	BMV_OP_RAW   = 1,
	BMV_OP_RUN   = 2,
	BMV_OP_COUNT
} BMV_OP;

class BMVPlayer {

	bool bOldAudio;

	/// Set when a movie is on
	bool bMovieOn;

	/// Set to kill one off
	bool bAbort;

	/// For escaping out of movies
	int bmvEscape;

	/// Movie file pointer
	Common::File stream;

	/// Movie file name
	char szMovieFile[14];

	/// Pointers to buffers
	byte *bigBuffer;

	/// Next data to use to extract a frame
	int nextUseOffset;

	/// Next data to use to extract sound data
	int nextSoundOffset;

	/// When above offset gets to what this is set at, rewind
	int wrapUseOffset;

	/// The offset of the most future packet
	int mostFutureOffset;

	/// The current frame
	int currentFrame;
	int currentSoundFrame;

	/// Number of packets currently in RAM
	int numAdvancePackets;

	/// Next slot that will be read from disc
	int nextReadSlot;

	/// Set when the whole file has been read
	bool bFileEnd;

	/// Palette
	COLORREF moviePal[256 * 8]; // TinselV1 & V2 need 256, TinselV3 needs 2048

	int blobsInBuffer;

	struct {
		OBJECT *pText;
		int	dieFrame;
	} texts[2];

	COLORREF talkColor;

	/// TinselV3 header fields
	int slotSize;
	int frames;
	int prefetchSlots;
	int numSlots;
	int frameRate;
	int audioMaxSize;
	int audioBlobSize;
	int width;
	int height;

	/// TinselV3
	int frameTime;
	int bpp;

	int bigProblemCount;

	bool bIsText;

	int movieTick;
	int startTick;
	uint32 nextMovieTime;

	uint16 Au_Prev1;
	uint16 Au_Prev2;
	byte *ScreenBeg;
	byte *screenBuffer;

	bool audioStarted;

	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	int nextMaintain;
public:
	BMVPlayer();

	void PlayBMV(CORO_PARAM, SCNHANDLE hFileStem, int myEscape);
	void FinishBMV();
	void CopyMovieToScreen();
	void FettleBMV();

	bool MoviePlaying();

	int32 MovieAudioLag();

	uint32 NextMovieTime();

	void AbortMovie();

private:
	void ReadHeader();

	void InitBMV(byte *memoryBuffer);
	void PrepAudio(const byte *sourceData, int blobCount, byte *destPtr);
	void PrepBMV(const byte *sourceData, int length, short deltaFetchDisp);
	void t3DoOperation(BMV_OP op, uint32 len, const byte **src, byte **dst, int32 deltaOffset);
	void t3PrepBMV(const byte *src, uint32 len, int32 deltaOffset);
	void MoviePalette(int paletteOffset);
	void InitializeMovieSound();
	void StartMovieSound();
	void FinishMovieSound();
	void MovieAudio(int audioOffset, int blobs);
	void FettleMovieText();
	void BmvDrawText(bool bDraw);
	void MovieText(CORO_PARAM, int stringId, int x, int y, int fontId, COLORREF *pTalkColor, int duration);
	int MovieCommand(char cmd, int commandOffset);
	int FollowingPacket(int thisPacket, bool bReallyImportant);
	void LoadSlots(int number);
	void InitializeBMV();
	bool MaintainBuffer();
	bool DoBMVFrame();
	bool DoSoundFrame();
};


} // End of namespace Tinsel

#endif
