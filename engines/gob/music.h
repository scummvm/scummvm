/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 * Original ADL-Player source Copyright (C) 2004 by Patrick Combet aka Dorian Gray
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

#ifndef GOB_MUSIC_H
#define GOB_MUSIC_H

#include "common/mutex.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/fmopl.h"

namespace Gob {

class GobEngine;

class Adlib : public Audio::AudioStream {
public:
	Adlib(GobEngine *vm);
	~Adlib();

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
	bool playing() const { return _playing; }
	bool getRepeating() const { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }
	int getIndex() const { return _index; }
	void startPlay() { if (_data) _playing = true; }
	void stopPlay()
	{
		Common::StackLock slock(_mutex);
		_playing = false;
	}
	void playTrack(const char *trackname);
	void playBgMusic();
	bool load(const char *fileName);
	void load(byte *data, uint32 size, int index = -1);
	void unload();

// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return !_playing; }
	bool endOfStream() const { return false; }
	int getRate() const { return _rate; }
	
protected:
	static const char *_tracks[][2];
	static const char *_trackFiles[];
	static const unsigned char _operators[];
	static const unsigned char _volRegNums [];
	Audio::SoundHandle _handle;
	FM_OPL *_opl;
	int _index;
	byte *_data;
	byte *_playPos;
	uint32 _dataSize;
	uint32 _rate;
	short _freqs[25][12];
	byte _notes[11];
	byte _notCol[11];
	byte _notLin[11];
	bool _notOn[11];
	byte _pollNotes[16];
	int _samplesTillPoll;
	int32 _repCount;
	bool _playing;
	bool _first;
	bool _ended;
	bool _needFree;
	Common::Mutex _mutex;
	GobEngine *_vm;

	void writeOPL(byte reg, byte val);
	void setFreqs();
	void reset();
	void setVoices();
	void setVoice(byte voice, byte instr, bool set);
	void setKey(byte voice, byte note, bool on, bool spec);
	void setVolume(byte voice, byte volume);
	void pollMusic();
};

} // End of namespace Gob

#endif // GOB_MUSIC_H
