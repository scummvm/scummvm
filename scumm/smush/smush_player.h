/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#if !defined(SMUSH_PLAYER_H) && !defined(DISABLE_SCUMM_7_8)
#define SMUSH_PLAYER_H

#include "common/util.h"
#include "scumm/smush/chunk.h"
#include "scumm/smush/codec37.h"
#include "scumm/smush/codec47.h"
#include "scumm/sound.h"

namespace Scumm {

class ScummEngine_v6;
class SmushFont;
class SmushMixer;
class StringResource;

class SmushPlayer {
	friend class Insane;
private:
	ScummEngine_v6 *_vm;
	int _version;
	int32 _nbframes;
	SmushMixer *_smixer;
	int16 _deltaPal[0x300];
	byte _pal[0x300];
	StringResource *_strings;
	Codec37Decoder _codec37;
	Codec47Decoder _codec47;
	FileChunk *_base;
	byte *_frameBuffer;
	byte *_specialBuffer;

	bool _skipNext;
	bool _subtitles;
	bool _skips[37];
	int32 _frame;

	Audio::SoundHandle _IACTchannel;
	AppendableAudioStream *_IACTstream;

	Audio::SoundHandle _compressedFileSoundHandle;
	bool _compressedFileMode;
	Common::File _compressedFile;
	byte _IACToutput[4096];
	int32 _IACTpos;
	bool _storeFrame;
	int _soundFrequency;
	bool _alreadyInit;
	bool _initDone;
	int _speed;
	bool _outputSound;

	byte *_dst;
	bool _updateNeeded;
	bool _warpNeeded;
	int _palDirtyMin, _palDirtyMax;
	int _warpX, _warpY;
	int _warpButtons;
	bool _insanity;
	bool _middleAudio;
	bool _skipPalette;
#ifdef _WIN32_WCE
	bool _inTimer;
	int16 _inTimerCount;
	int16 _inTimerCountRedraw;
#endif

	Common::Mutex _mutex;

public:
	SmushPlayer(ScummEngine_v6 *scumm, int speed);
	~SmushPlayer();

	void play(const char *filename, int32 offset = 0, int32 startFrame = 0);
	void warpMouse(int x, int y, int buttons);

protected:
	SmushFont *_sf[5];
	int _width, _height;
	
	int _origPitch, _origNumStrips;

	void insanity(bool);
	void setPalette(const byte *palette);
	void setPaletteValue(int n, byte r, byte g, byte b);
	void setDirtyColors(int min, int max);
	void seekSan(const char *file, int32 pos, int32 contFrame);
	const char *getString(int id);

private:
	void parseNextFrame();
	void init();
	void release();
	void setupAnim(const char *file);
	void updateScreen();
	void tryCmpFile(const char *filename);

	bool readString(const char *file);
	void checkBlock(const Chunk &, Chunk::type, uint32 = 0);
	void handleAnimHeader(Chunk &);
	void handleFrame(Chunk &);
	void handleNewPalette(Chunk &);
#ifdef USE_ZLIB
	void handleZlibFrameObject(Chunk &b);
#endif
	void handleFrameObject(Chunk &);
	void handleSoundBuffer(int32, int32, int32, int32, int32, int32, Chunk &, int32);
	void handleSoundFrame(Chunk &);
	void handleSkip(Chunk &);
	void handleStore(Chunk &);
	void handleFetch(Chunk &);
	void handleIACT(Chunk &);
	void handleTextResource(Chunk &);
	void handleDeltaPalette(Chunk &);
	void readPalette(byte *, Chunk &);
	
	static void timerCallback(void *ptr);
};

} // End of namespace Scumm

#endif
