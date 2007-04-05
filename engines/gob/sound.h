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

#ifndef GOB_SOUND_H
#define GOB_SOUND_H

#include "common/mutex.h"
#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Gob {

enum SoundType {
	SOUND_SND,
	SOUND_ADL
};

enum SoundSource {
	SOUND_FILE,
	SOUND_TOT,
	SOUND_EXT
};

class SoundDesc {
public:
	int16 _repCount;
	int16 _frequency;
	int16 _flag;
	int16 _id;

	byte *getData() { return _dataPtr; }
	uint32 size() { return _size; }
	bool empty() { return !_dataPtr; }
	bool isId(int16 id) { return _dataPtr && _id == id; };
	SoundType getType() { return _type; }

	void set(SoundType type, SoundSource src, byte *data, uint32 dSize);
	void load(SoundType type, SoundSource src, byte *data, uint32 dSize);
	void free();
	void convToSigned();

	// Which fade out length to use when the fade starts half-way through?
	int16 calcFadeOutLength(int16 frequency) {
		return (10 * (_size / 2)) / frequency;
	}
	uint32 calcLength(int16 repCount, int16 frequency, bool fade) {
		uint32 fadeSize = fade ? _size / 2 : 0;
		return ((_size * repCount - fadeSize) * 1000) / frequency;
	}
	
	SoundDesc() : _data(0), _dataPtr(0), _size(0), _type(SOUND_SND),
			_source(SOUND_FILE), _repCount(0), _frequency(0),
			_flag(0), _id(0) {}
	~SoundDesc() { free(); }

private:
	byte *_data;
	byte *_dataPtr;
	uint32 _size;

	SoundType _type;
	SoundSource _source;

	void loadSND(byte *data, uint32 dSize);
	void loadADL(byte *data, uint32 dSize);
};

class Snd : public Audio::AudioStream {
public:
	char _playingSound;

	Snd(GobEngine *vm);
	~Snd();

	void speakerOn(int16 frequency, int32 length);
	void speakerOff();
	void speakerOnUpdate(uint32 milis);
	void stopSound(int16 fadeLength, SoundDesc *sndDesc = 0);

	bool loadSample(SoundDesc &sndDesc, const char *fileName);
	void freeSample(SoundDesc &sndDesc);
	void playSample(SoundDesc &sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength = 0);

	void playComposition(int16 *composition, int16 freqVal,
			SoundDesc *sndDescs = 0, int8 sndCount = 60);
	void stopComposition();
	void setRepeating(int32 repCount);
	void waitEndPlay(bool interruptible = false, bool stopComp = true);

	static void convToSigned(byte *buffer, int length) {
		while (length-- > 0)
			*buffer++ ^= 0x80;
	}

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return _end; }
	bool endOfStream() const { return false; }
	int getRate() const { return _rate; }

protected:
	// TODO: This is a very primitive square wave generator. The only thing it
	//       has in common with the PC speaker is that it sounds terrible.
	// Note: The SCUMM code has a PC speaker implementations; maybe it could be
	//       refactored to be reusable by all engines. And DosBox also has code
	//       for emulating the PC speaker.
	class SquareWaveStream : public Audio::AudioStream {
	private:
		uint _rate;
		bool _beepForever;
		uint32 _periodLength;
		uint32 _periodSamples;
		uint32 _remainingSamples;
		uint32 _mixedSamples;
		int16 _sampleValue;

	public:
		SquareWaveStream();

		void playNote(int freq, int32 ms, uint rate);
		void update(uint32 milis);
		void stop(uint32 milis);

		int readBuffer(int16 *buffer, const int numSamples);

		bool isStereo() const	{ return false; }
		bool endOfData() const	{ return false; }
		bool endOfStream() const { return false; }
		int getRate() const	{ return _rate; }
	};

	SquareWaveStream _speakerStream;
	Audio::SoundHandle _speakerHandle;
	uint32 _speakerStartTimeKey;

	Audio::SoundHandle *_activeHandle;
	Audio::SoundHandle _compositionHandle;
	
	SoundDesc *_compositionSamples;
	int8 _compositionSampleCount;
	int16 _composition[50];
	int8 _compositionPos;

	Audio::SoundHandle _handle;
	Common::Mutex _mutex;
	SoundDesc *_curSoundDesc;
	bool _end;
	int8 *_data;
	uint32 _length;
	uint32 _rate;
	int32 _freq;
	int32 _repCount;
	double _offset;
	double _ratio;

	double _frac;
	int16 _cur;
	int16 _last;

	bool _fade;
	double _fadeVol;
	double _fadeVolStep;
	uint8 _fadeLength;
	uint32 _fadeSamples;
	uint32 _curFadeSamples;

	GobEngine *_vm;

	void setSample(SoundDesc &sndDesc, int16 repCount,
			int16 frequency, int16 fadeLength);
	void checkEndSample();
	void nextCompositionPos();
};

} // End of namespace Gob

#endif // GOB_SOUND_H
