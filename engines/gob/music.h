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
#include "gob/util.h"

namespace Gob {

class GobEngine;

/**
 * Emulation of the "Paula" Amiga music chip
 * The interrupt frequency specifies the number of mixed wavesamples between
 * calls of the interrupt method
 */
class Paula : public Audio::AudioStream {
public:
	Paula(bool stereo = false, int rate = 44100, int interruptFreq = 0);
	~Paula();
	
	bool playing() const { return _playing; }
	void setInterruptFreq(int freq) { _intFreq = freq; }
	void setPanning(byte voice, byte panning)
	{
		if (voice < 4)
			_voice[voice].panning = panning;
	}
	void clearVoice(byte voice);
	void clearVoices() { int i; for (i = 0; i < 4; i++) clearVoice(i); }
	virtual void startPlay(void) {}
	virtual void stopPlay(void) {}
	virtual void pausePlay(bool pause) {}

// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return _stereo; }
	bool endOfData() const { return _end; }
	int getRate() const { return _rate; }

protected:
	struct Channel {
		int8 *data;
		int8 *dataRepeat;
		uint32 length;
		uint32 lengthRepeat;
		int16 period;
		byte volume;
		double offset;
		byte panning; // For stereo mixing: 0 = far left, 255 = far right
	} _voice[4];

	int _rate;
	int _intFreq;
	int _curInt;
	bool _stereo;
	bool _end;
	bool _playing;
	Common::Mutex _mutex;

	void mix(int16 *&buf, int8 data, int voice) {
		if (_stereo) {
			*buf++ += (((int32) data) * _voice[voice].volume *
				 (255 - _voice[voice].panning)) >> 7;
			*buf++ += (((int32) data) * _voice[voice].volume *
				 (_voice[voice].panning)) >> 7;
		} else
			*buf++ += _voice[voice].volume * data;
	}
	virtual void interrupt(void) {};
};

/** A player for the Infogrames/RobHubbard2 format */
class Infogrames : public Paula {
public:
	class Instruments {
	public:
		Instruments();
		template<typename T> Instruments(T ins) {
			init();
			bool result = load(ins);
			assert(result);
		}
		~Instruments();

		bool load(Common::SeekableReadStream &ins);
		bool load(const char *ins) {
			Common::File f;

			if (f.open(ins))
				return load(f);
			return false;
		}
		void unload(void);

		uint8 getCount(void) const { return _count; }

	protected:
		struct Sample {
			int8 *data;
			int8 *dataRepeat;
			uint32 length;
			uint32 lengthRepeat;
		} _samples[32];

		uint8 _count;
		int8 *_sampleData;

		void init();

		friend class Infogrames;
	};

	Infogrames(Instruments &ins, bool stereo = false, int rate = 44100);
	~Infogrames();

	Instruments *getInstruments(void) const { return _instruments; }
	bool getRepeating(void) const { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }
	virtual void startPlay(void) { _playing = true;}
	virtual void stopPlay(void)
	{
		_mutex.lock();
		_playing = false;
		_mutex.unlock();
	}
	virtual void pausePlay(bool pause) { _playing = !pause; }

	bool load(Common::SeekableReadStream &dum);
	bool load(const char *dum) {
		Common::File f;

		if (f.open(dum))
			return load(f);
		return false;
	}
	void unload(void);
	void restart(void) { if (_data) { stopPlay(); init(); startPlay(); } }

protected:
	Instruments *_instruments;

	static const uint8 tickCount[];
	static const uint16 periods[];
	byte *_data;
	int32 _repCount;

	byte *_subSong;
	byte *_cmdBlocks;
	byte *_volSlideBlocks;
	byte *_periodSlideBlocks;
	uint8 _speedCounter;
	uint8 _speed;

	uint16 _volume;
	int16 _period;
	uint8 _sample;

	struct Slide {
		byte *data;
		int8 amount;
		uint8 dataOffset;
		int16 finetuneNeg;
		int16 finetunePos;
		uint8 curDelay1;
		uint8 curDelay2;
		uint8 flags; // 0: Apply finetune modifier, 2: Don't slide, 7: Continuous
	};
	struct Channel {
		byte *cmdBlockIndices;
		byte *cmdBlocks;
		byte *cmds;
		uint8 ticks;
		uint8 tickCount;
		Slide volSlide;
		Slide periodSlide;
		int16 period;
		int8 periodMod;
		uint8 flags; // 0: Need init, 5: Loop cmdBlocks, 6: Ignore channel
	} _chn[4];

	void init(void);
	void reset(void);
	void getNextSample(Channel &chn);
	int16 tune(Slide &slide, int16 start) const;
	virtual void interrupt(void);
};

class Adlib : public Audio::AudioStream {
public:
	Adlib(GobEngine *vm);
	virtual ~Adlib();

	void lock() { _mutex.lock(); }
	void unlock() { _mutex.unlock(); }
	bool playing() const { return _playing; }
	bool getRepeating(void) const { return _repCount != 0; }
	void setRepeating (int32 repCount) { _repCount = repCount; }
	int getIndex(void) const { return _index; }
	virtual void startPlay(void) { if (_data) _playing = true; }
	virtual void stopPlay(void)
	{
		_mutex.lock();
		_playing = false;
		_mutex.unlock();
	}
	virtual void playTrack(const char *trackname);
	virtual void playBgMusic(void);
	virtual bool load(const char *filename);
	virtual void load(byte *data, int index=-1);
	virtual void unload(void);

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
	static const char *_trackFiles[];
	static const unsigned char _operators[];
	static const unsigned char _volRegNums [];
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
	uint32 _samplesTillPoll;
	int32 _repCount;
	bool _playing;
	bool _first;
	bool _ended;
	bool _needFree;
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

/**
 * A dummy class for the "null" sound driver
 */
class Adlib_Dummy: public Adlib {
public:
	Adlib_Dummy(GobEngine *vm) : Adlib(vm) {}

	virtual void startPlay(void) {};
	virtual void stopPlay(void) {};
	virtual void playTrack(const char *trackname) {};
	virtual void playBgAdlib(void) {};
	virtual bool load(const char *filename) { return true; }
	virtual void load(byte *data, int index=-1) {}
	virtual void unload(void) {};

	virtual ~Adlib_Dummy() {};
};

} // End of namespace Gob

#endif
