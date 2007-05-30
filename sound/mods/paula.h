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
	void setPanning(byte voice, byte panning) {
		assert(voice < NUM_VOICES);
		_voice[voice].panning = panning;
	}
	void clearVoice(byte voice);
	void clearVoices() { for (int i = 0; i < NUM_VOICES; ++i) clearVoice(i); }
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
		const int8 *data;
		const int8 *dataRepeat;
		uint32 length;
		uint32 lengthRepeat;
		int16 period;
		byte volume;
		double offset;
		byte panning; // For stereo mixing: 0 = far left, 255 = far right
	} _voice[NUM_VOICES];

	int _rate;
	int _intFreq;
	int _curInt;
	bool _stereo;
	bool _end;
	bool _playing;
	Common::Mutex _mutex;

	void mix(int16 *&buf, int8 data, int voice) {
		const int32 tmp = ((int32) data) * _voice[voice].volume;
		if (_stereo) {
			*buf++ += (tmp * (255 - _voice[voice].panning)) >> 7;
			*buf++ += (tmp * (_voice[voice].panning)) >> 7;
		} else
			*buf++ += tmp;
	}
	virtual void interrupt(void) {}
};

} // End of namespace Audio

#endif
