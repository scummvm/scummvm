/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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

#ifndef SMUSH_PLAYER_H
#define SMUSH_PLAYER_H

#include "common/util.h"
#include "scumm/smush/chunk.h"
#include "scumm/smush/codec37.h"
#include "scumm/smush/codec47.h"
#include "sound/mixer.h"

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
	SmushFont *_sf[5];
	Codec37Decoder _codec37;
	Codec47Decoder _codec47;
	FileChunk *_base;
	byte *_frameBuffer;

	bool _skipNext;
	bool _subtitles;
	bool _skips[37];
	int32 _frame;

	PlayingSoundHandle _IACTchannel;
	byte _IACToutput[4096];
	int32 _IACTpos;
	bool _storeFrame;
	int _soundFrequency;
	bool _alreadyInit;
	int _speed;
	bool _outputSound;

	int _width, _height;
	byte *_dst;
	bool _updateNeeded;
	bool _insanity;
	bool _middleAudio;

public:
	SmushPlayer(ScummEngine_v6 *scumm, int speed);
	~SmushPlayer();

	void play(const char *filename, const char *directory);

protected:
	void insanity(bool);
	void setPalette(const byte *palette);
	void seekSan(const char *file, const char *directory, int32 pos, int32 contFrame);

private:
	void updatePalette(void);
	void parseNextFrame();
	void init();
	void deinit();
	void setupAnim(const char *file, const char *directory);
	void updateScreen();

	bool readString(const char *file, const char *directory);
	void checkBlock(const Chunk &, Chunk::type, uint32 = 0);
	void handleAnimHeader(Chunk &);
	void handleFrame(Chunk &);
	void handleNewPalette(Chunk &);
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
