/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "stdafx.h"
#include "sound/audiostream.h"
#include "sound/mididrv.h"
#include "sound/mixer.h"

#define BASE_FREQ 250
#define FIXP_SHIFT 16

class MidiDriver_Emulated : public AudioStream, public MidiDriver {
protected:
	bool _isOpen;
	SoundMixer *_mixer;

private:
	Timer::TimerProc _timer_proc;
	void *_timer_param;

	int _next_tick;
	int _samples_per_tick;

protected:
	virtual void generate_samples(int16 *buf, int len) = 0;

public:
	MidiDriver_Emulated(SoundMixer *mixer) : _mixer(mixer) {
		_isOpen = false;
	
		_timer_proc = 0;
		_timer_param = 0;
	
		_next_tick = 0;
		_samples_per_tick = 0;
	}

	int open() {
		_isOpen = true;
		_samples_per_tick = (getRate() << FIXP_SHIFT) / BASE_FREQ;
	}

	void setTimerCallback(void *timer_param, Timer::TimerProc timer_proc) {
		_timer_proc = timer_proc;
		_timer_param = timer_param;
	}

	uint32 getBaseTempo() { return 1000000 / BASE_FREQ; }


	// AudioStream API
	int readBuffer(int16 *data, const int numSamples) {
		const int stereoFactor = isStereo() ? 2 : 1;
		int len = numSamples / stereoFactor;
		int step;
	
		do {
			step = len;
			if (step > (_next_tick >> FIXP_SHIFT))
				step = (_next_tick >> FIXP_SHIFT);
			generate_samples(data, step);
	
			_next_tick -= step << FIXP_SHIFT;
			if (!(_next_tick >> FIXP_SHIFT)) {
				if (_timer_proc)
					(*_timer_proc)(_timer_param);
				_next_tick += _samples_per_tick;
			}
			data += step * stereoFactor;
			len -= step;
		} while (len);

		return numSamples;
	}
	int16 read() {
		error("ProcInputStream::read not supported");
	}
	bool endOfData() const { return false; }
};
