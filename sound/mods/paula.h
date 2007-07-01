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

#ifndef SOUND_MODS_PAULA_H
#define SOUND_MODS_PAULA_H

#include "sound/audiostream.h"
#include "common/frac.h"
#include "common/mutex.h"

namespace Audio {

/**
 * Emulation of the "Paula" Amiga music chip
 * The interrupt frequency specifies the number of mixed wavesamples between
 * calls of the interrupt method
 */
class Paula : public AudioStream {
public:
	static const int NUM_VOICES = 4;

	Paula(bool stereo = false, int rate = 44100, int interruptFreq = 0);
	~Paula();

	bool playing() const { return _playing; }
	void setInterruptFreq(int freq) { _curInt = _intFreq = freq; }
	void clearVoice(byte voice);
	void clearVoices() { for (int i = 0; i < NUM_VOICES; ++i) clearVoice(i); }
	void startPlay(void) { _playing = true; }
	void stopPlay(void) { _playing = false; }
	void pausePlay(bool pause) { _playing = !pause; }

// AudioStream API
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return _stereo; }
	bool endOfData() const { return _end; }
	int getRate() const { return _rate; }

protected:
	struct Channel {
		const int8 *data;
		const int8 *dataRepeat;
		uint32 length;
		uint32 lengthRepeat;
		int16 period;
		byte volume;
		frac_t offset;
		byte panning; // For stereo mixing: 0 = far left, 255 = far right
	};

	bool _end;
	Common::Mutex _mutex;

	virtual void interrupt(void) = 0;

	void startPaula() {
		_playing = true;
		_end = false;
	}
	
	void stopPaula() {
		_playing = false;
		_end = true;
	}
	
	void setChannelPanning(byte channel, byte panning) {
		assert(channel < NUM_VOICES);
		_voice[channel].panning = panning;
	}

	void setChannelPeriod(byte channel, int16 period) {
		assert(channel < NUM_VOICES);
		_voice[channel].period = period;
	}

	void setChannelVolume(byte channel, byte volume) {
		assert(channel < NUM_VOICES);
		_voice[channel].volume = volume;
	}

	void setChannelData(uint8 channel, const int8 *data, const int8 *dataRepeat, uint32 length, uint32 lengthRepeat, int32 offset = 0) {
		assert(channel < NUM_VOICES);

		// For now, we only support 32k samples, as we use 16bit fixed point arithmetics.
		// If this ever turns out to be a problem, we can still enhance this code.
		assert(0 <= offset && offset < 32768);
		assert(length < 32768);
		assert(lengthRepeat < 32768);

		Channel &ch = _voice[channel];
		ch.data = data;
		ch.dataRepeat = dataRepeat;
		ch.length = length;
		ch.lengthRepeat = lengthRepeat;
		ch.offset = intToFrac(offset);
	}

	void setChannelOffset(byte channel, frac_t offset) {
		assert(channel < NUM_VOICES);
		assert(0 <= offset);
		_voice[channel].offset = offset;
	}

	frac_t getChannelOffset(byte channel) {
		assert(channel < NUM_VOICES);
		return _voice[channel].offset;
	}

private:
	Channel _voice[NUM_VOICES];

	const bool _stereo;
	const int _rate;
	int _intFreq;
	int _curInt;
	bool _playing;

	template<bool stereo>
	int readBufferIntern(int16 *buffer, const int numSamples);
};

} // End of namespace Audio

#endif
