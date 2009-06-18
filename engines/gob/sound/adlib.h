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

#ifndef GOB_SOUND_ADLIB_H
#define GOB_SOUND_ADLIB_H

#include "common/mutex.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/fmopl.h"

namespace Gob {

class GobEngine;

class AdLib : public Audio::AudioStream {
public:
	AdLib(Audio::Mixer &mixer);
	virtual ~AdLib();

	bool isPlaying() const;
	int getIndex() const;
	bool getRepeating() const;

	void setRepeating(int32 repCount);

	void startPlay();
	void stopPlay();

	virtual void unload();

// AudioStream API
	int  readBuffer(int16 *buffer, const int numSamples);
	bool isStereo()    const { return false;     }
	bool endOfData()   const { return !_playing; }
	bool endOfStream() const { return false;     }
	int  getRate()     const { return _rate;     }

protected:
	static const unsigned char _operators[];
	static const unsigned char _volRegNums [];

	Audio::Mixer *_mixer;
	Audio::SoundHandle _handle;
	FM_OPL *_opl;

	Common::Mutex _mutex;

	uint32 _rate;

	byte *_data;
	byte *_playPos;
	uint32 _dataSize;

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

	bool _freeData;

	int _index;

	unsigned char _wait;
	uint8 _tickBeat;
	uint8 _beatMeasure;
	uint32 _totalTick;
	uint32 _nrCommand;
	uint16 _pitchBendRangeStep;
	uint16 _basicTempo, _tempo;

	void writeOPL(byte reg, byte val);
	void setFreqs();
	void setKey(byte voice, byte note, bool on, bool spec);
	void setVolume(byte voice, byte volume);
	void pollMusic();

	virtual void interpret() = 0;

	virtual void reset();
	virtual void rewind() = 0;
	virtual void setVoices() = 0;

private:
	void init();
};

class ADLPlayer : public AdLib {
public:
	ADLPlayer(Audio::Mixer &mixer);
	~ADLPlayer();

	bool load(const char *fileName);
	bool load(byte *data, uint32 size, int index = -1);

	void unload();

protected:
	void interpret();

	void reset();
	void rewind();

	void setVoices();
	void setVoice(byte voice, byte instr, bool set);
};

class MDYPlayer : public AdLib {
public:
	MDYPlayer(Audio::Mixer &mixer);
	~MDYPlayer();

	bool loadMDY(const char *fileName);
	bool loadTBR(const char *fileName);

	void unload();

protected:
	byte _soundMode;

	byte *_timbres;
	uint16 _tbrCount;
	uint16 _tbrStart;
	uint32 _timbresSize;

	void interpret();

	void reset();
	void rewind();

	void setVoices();
	void setVoice(byte voice, byte instr, bool set);

private:
	void init();
};

} // End of namespace Gob

#endif // GOB_SOUND_ADLIB_H
