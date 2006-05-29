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

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/fmopl.h"
#include "common/mutex.h"

#include "gob/gob.h"

namespace Gob {

class GobEngine;

class Music : public Audio::AudioStream {
public:
	Music(GobEngine *vm);
	virtual ~Music();

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
	bool playing() { return _playing; }
	bool getRepeating(void) { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }
	virtual void startPlay(void);
	virtual void stopPlay(void) { _mutex.lock(); _playing = false; _mutex.unlock(); }
	virtual void playTrack(const char *trackname);
	virtual void playBgMusic(void);
	virtual bool loadMusic(const char *filename);
	virtual void loadFromMemory(byte *data);
	virtual void unloadMusic(void);

// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples) {
		premixerCall(buffer, numSamples / 2);
		return numSamples;
	}
	bool isStereo() const { return true; }
	bool endOfData() const { return false; }
	int getRate() const { return _rate; }
	
protected:
	static const char *_tracks[][2];
	static const char *_tracksToFiles[][2];
	static const unsigned char _operators[];
	static const unsigned char _volRegNums [];
	FM_OPL *_opl;
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
	uint32 _samplesTillPoll;
	int32 _repCount;
	bool _playing;
	bool _first;
	bool _ended;
	Common::Mutex _mutex;
	GobEngine *_vm;

	void premixerCall(int16 *buf, uint len);
	void writeOPL(byte reg, byte val);
	void setFreqs(void);
	void reset(void);
	void setVoices();
	void setVoice(byte voice, byte instr, bool set);
	void setKey(byte voice, byte note, bool on, bool spec);
	void setVolume(byte voice, byte volume);
	void pollMusic(void);
};

class Music_Dummy: public Music {
public:
	Music_Dummy(GobEngine *vm) : Music(vm) {
		_vm->_mixer->setupPremix(0);
		OPLDestroy(_opl);
	}

	virtual void startPlay(void) {};
	virtual void stopPlay(void) {};
	virtual void playTrack(const char *trackname) {};
	virtual void playBgMusic(void) {};
	virtual bool loadMusic(const char *filename) { return true; };
	virtual void loadFromMemory(byte *data) {};
	virtual void unloadMusic(void) {};

	virtual ~Music_Dummy() {};
};

} // End of namespace Gob

#endif
