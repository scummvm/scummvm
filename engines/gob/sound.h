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

#include "sound/audiostream.h"
#include "sound/mixer.h"

namespace Gob {

class Snd : public Audio::AudioStream {
public:
	struct SoundDesc {
		Audio::SoundHandle handle;
		char *data;
		uint32 size;
		int16 repCount;
		int16 timerTicks;
		int16 inClocks;
		int16 frequency;
		int16 flag;
		SoundDesc() : data(0), size(0), repCount(0), timerTicks(0),
					  inClocks(0), frequency(0), flag(0) {}
	};

	typedef void (*CleanupFuncPtr) (int16);

	char _playingSound;
	CleanupFuncPtr _cleanupFunc;

	Snd(GobEngine *vm);
	void speakerOn(int16 frequency, int32 length);
	void speakerOff(void);
	void speakerOnUpdate(uint32 milis);
	SoundDesc *loadSoundData(const char *path);
	void stopSound(int16 fadeLength);
	void playSample(SoundDesc *sndDesc, int16 repCount, int16 frequency, int16 fadeLength = 0);
	void playComposition(int16 *composition, int16 freqVal, SoundDesc **sndDescs = 0,
			int8 *sndTypes = 0, int8 sndCount = 60);
	void stopComposition(void);
	void waitEndPlay(bool interruptible = false, bool stopComp = true);

	// This deletes sndDesc and stops playing the sample.
	// If freedata is set, it also delete[]s the sample data.
	void freeSoundDesc(SoundDesc *sndDesc, bool freedata=true);

	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return false; }
	bool endOfData() const { return _end; }
	bool endOfStream() const { return false; }
	int getRate() const { return _rate; }

protected:
	// TODO: This is a very primitive square wave generator. The only thing is
	//       has in common with the PC speaker is that it sounds terrible.
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
		~SquareWaveStream() {}

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
	
	SoundDesc **_compositionSamples;
	int8 *_compositionSampleTypes;
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

	void cleanupFuncCallback() {;}
	int16 checkProAudio(void) {return 0;}
	int16 checkAdlib(void) {return 0;}
	int16 checkBlaster(void) {return 0;}

	void writeAdlib(int16 port, int16 data);
	void setBlasterPort(int16 port);
	void setResetTimerFlag(char flag){return;}
	void setSample(Snd::SoundDesc *sndDesc, int16 repCount, int16 frequency, int16 fadeLength);
	void checkEndSample(void);
	void nextCompositionPos(void);
};

}				// End of namespace Gob

#endif
