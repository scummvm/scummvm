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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ADL_SOUND_H
#define ADL_SOUND_H

#include "audio/audiostream.h"

#include "common/array.h"
#include "common/frac.h"

namespace Adl {

class Speaker;

struct Tone {
	double freq; // Hz
	double len; // ms

	Tone(double frequency, double length) : freq(frequency), len(length) { }
};

typedef Common::Array<Tone> Tones;

class Sound : public Audio::AudioStream {
public:
	Sound(const Tones &tones);
	~Sound() override;

	// AudioStream
	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return false; }
	bool endOfData() const override;
	int getRate() const override	{ return _rate; }

private:
	const Tones &_tones;

	Speaker *_speaker;
	int _rate;
	uint _toneIndex;
	int _samplesRem;
};

} // End of namespace Adl

#endif
