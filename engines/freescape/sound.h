/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FREESCAPE_SOUND_H
#define FREESCAPE_SOUND_H

#include "audio/softsynth/pcspk.h"
#include "common/array.h"

namespace Freescape {

struct soundFx {
	int size;
	float sampleRate;
	int repetitions;
	byte *data;
};

struct soundUnitZX {
	bool isRaw;
	uint16 freqTimesSeconds;
	uint16 tStates;
	float rawFreq;
	uint32 rawLengthus;
	float multiplier;
};

struct soundSpeakerFx {
	uint16 frequencyStart;
	uint8 frequencyDuration;
	uint8 frequencyStepsNumber;
	uint16 frequencyStep;
	uint8 repetitions;
	Common::Array<struct soundSpeakerFx *>additionalSteps;
};

// TODO: Migrate to Audio::PCSpeaker
class SizedPCSpeaker : public Audio::PCSpeakerStream {
public:
	bool endOfStream() const override { return !isPlaying(); }
};

} // End of namespace Freescape

#endif // FREESCAPE_SOUND_H
