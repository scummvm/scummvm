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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADS_CORE_PCSPK_PIT_H
#define MADS_CORE_PCSPK_PIT_H

#include "common/array.h"
#include "common/scummsys.h"

namespace MADS {

/**
 * Renders the mode-3 PIT state driven by the MADS PC speaker sound driver.
 *
 * Unlike Audio::PCSpeakerStream, this class accepts the raw counter and
 * port-control writes made by the original DOS driver. It owns no mixer
 * stream or synchronization; the owning MADS sound driver serializes access.
 */
class PCSpeakerPITRenderer {
public:
	explicit PCSpeakerPITRenderer(uint32 sampleRate,
		uint32 pitClock = 1193182);
	~PCSpeakerPITRenderer();

	/** Advance the PIT to a fractional position in the current output sample. */
	void advanceToSampleFraction(uint32 numerator, uint32 denominator);
	void writeMode3Count(uint16 count);
	void setControl(bool timerGate, bool speakerEnabled);
	int16 generateSample(byte volume);

private:
	class PCSpeakerOutputStage;

	enum {
		kFractionalPhases = 32,
		kImpulseDurationUs = 3125,
		kSampleFracBits = 24,
		kCoefficientFracBits = 30
	};

	void initializeImpulse();
	void resetState();
	void addTransition(int level, uint64 elapsedPitClocks = 0);
	void advanceCounter(uint64 pitClockUnits);
	bool isUndersampled(uint16 count) const;
	int outputLevel() const;

	uint32 _sampleRate;
	uint32 _pitClock;
	uint64 _phase;
	uint64 _samplePhase;
	uint64 _sampleCounter;
	uint16 _count;
	uint16 _pendingCount;
	bool _hasPendingCount;
	bool _counterLoaded;
	bool _timerGate;
	bool _speakerEnabled;
	bool _high;
	bool _undersampled;
	bool _hasUndersampledReload;
	uint64 _lastUndersampledReloadSample;

	uint32 _impulseLength;
	uint32 _impulseHead;
	Common::Array<int32> _impulseLut;
	Common::Array<int32> _impulseBuffer;
	int32 _reconstructedLevel;
	int _targetLevel;
	PCSpeakerOutputStage *_outputStage;

	PCSpeakerPITRenderer(const PCSpeakerPITRenderer &);
	PCSpeakerPITRenderer &operator=(const PCSpeakerPITRenderer &);
};

} // End of namespace MADS

#endif // MADS_CORE_PCSPK_PIT_H
