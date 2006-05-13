/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 * Original ADL-Player source Copyright (C) 2004 by Dorian Gray
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
#include "sound/fmopl.h"
#include "common/mutex.h"

#include "gob/gob.h"

namespace Gob {

class GobEngine;

class Music : public Audio::AudioStream {
public:
	Music(GobEngine *vm);
	~Music();

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
	bool playing() { return _playing; }
	bool getRepeating(void) { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }
	void startPlay(void);
	void stopPlay(void) { _mutex.lock(); _playing = false; _mutex.unlock(); }
	void playTrack(const char *trackname);
	void playBgMusic(void);
	bool loadMusic(const char *filename);
	void loadFromMemory(byte *data);
	void unloadMusic(void);

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

} // End of namespace Gob

#endif
