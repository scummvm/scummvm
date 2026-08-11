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

#include "mads/core/pcspk_pit.h"

#include "common/hashmap.h"
#include "common/mutex.h"
#include "common/util.h"

#include <math.h>

namespace MADS {

namespace {

int32 saturateInt32(int64 value) {
	if (value > 0x7fffffffLL)
		return 0x7fffffff;
	if (value < -0x80000000LL)
		return -0x7fffffff - 1;
	return (int32)value;
}

int32 fixedFromDouble(double value, uint fracBits) {
	const double scale = (double)((uint64)1 << fracBits);
	const double scaled = value * scale;
	return saturateInt32((int64)(scaled < 0.0 ? scaled - 0.5 : scaled + 0.5));
}

int32 multiplyFixed(int32 value, int32 coefficient, uint fracBits) {
	const int64 product = (int64)value * coefficient;
	const int64 half = (int64)1 << (fracBits - 1);
	const int64 rounded = product < 0 ?
		-(((-product) + half) >> fracBits) :
		(product + half) >> fracBits;
	return saturateInt32(rounded);
}

} // namespace

class PCSpeakerPITRenderer::PCSpeakerOutputStage {
private:
	struct Section {
		int32 b0;
		int32 b1;
		int32 b2;
		int32 a1;
		int32 a2;
		int32 z1;
		int32 z2;

		Section() :
			b0(0), b1(0), b2(0), a1(0), a2(0), z1(0), z2(0) {
		}

		int32 process(int32 input) {
			const int32 output = saturateInt32((int64)
				multiplyFixed(input, b0, kCoefficientFracBits) + z1);
			z1 = saturateInt32((int64)
				multiplyFixed(input, b1, kCoefficientFracBits) -
				multiplyFixed(output, a1, kCoefficientFracBits) + z2);
			z2 = saturateInt32((int64)
				multiplyFixed(input, b2, kCoefficientFracBits) -
				multiplyFixed(output, a2, kCoefficientFracBits));
			return output;
		}

		void clear() {
			z1 = 0;
			z2 = 0;
		}
	};

	struct FilterConfiguration {
		Section highPassFirst;
		Section highPassSecond;
		Section lowPassFirst;
		Section lowPassSecond;
	};

	Section _highPassFirst;
	Section _highPassSecond;
	Section _lowPassFirst;
	Section _lowPassSecond;
	int32 _accumulatorDecay;

	static void configureFirstOrder(Section &section, double sampleRate,
			double cutoff, bool highPass) {
		const double k = tan(M_PI * cutoff / sampleRate);
		const double normalization = 1.0 / (1.0 + k);

		section.b0 = fixedFromDouble(
			(highPass ? 1.0 : k) * normalization, kCoefficientFracBits);
		section.b1 = fixedFromDouble(
			(highPass ? -1.0 : k) * normalization, kCoefficientFracBits);
		section.b2 = 0;
		section.a1 = fixedFromDouble(
			(k - 1.0) * normalization, kCoefficientFracBits);
		section.a2 = 0;
	}

	static void configureSecondOrder(Section &section, double sampleRate,
			double cutoff, bool highPass) {
		// The complex pole pair of a third-order Butterworth filter has Q=1.
		const double k = tan(M_PI * cutoff / sampleRate);
		const double normalization = 1.0 / (1.0 + k + k * k);
		const double b0 = (highPass ? 1.0 : k * k) * normalization;

		section.b0 = fixedFromDouble(b0, kCoefficientFracBits);
		section.b1 = fixedFromDouble(
			(highPass ? -2.0 : 2.0) * b0, kCoefficientFracBits);
		section.b2 = section.b0;
		section.a1 = fixedFromDouble(
			2.0 * (k * k - 1.0) * normalization,
			kCoefficientFracBits);
		section.a2 = fixedFromDouble(
			(1.0 - k + k * k) * normalization,
			kCoefficientFracBits);
	}

public:
	explicit PCSpeakerOutputStage(uint32 sampleRate) :
		_accumulatorDecay(fixedFromDouble(0.999, kCoefficientFracBits)) {
		static Common::Mutex filterCacheMutex;
		static Common::HashMap<uint32, FilterConfiguration> filterCache;
		{
			Common::StackLock lock(filterCacheMutex);
			if (!filterCache.contains(sampleRate)) {
				FilterConfiguration configuration;
				const double highPassCutoff =
					MIN<double>(120.0, sampleRate * 0.1);
				const double lowPassCutoff =
					MIN<double>(4300.0, sampleRate * 0.45);
				configureFirstOrder(configuration.highPassFirst,
					sampleRate, highPassCutoff, true);
				configureSecondOrder(configuration.highPassSecond,
					sampleRate, highPassCutoff, true);
				configureFirstOrder(configuration.lowPassFirst,
					sampleRate, lowPassCutoff, false);
				configureSecondOrder(configuration.lowPassSecond,
					sampleRate, lowPassCutoff, false);
				filterCache.setVal(sampleRate, configuration);
			}

			const FilterConfiguration &configuration =
				filterCache.getVal(sampleRate);
			_highPassFirst = configuration.highPassFirst;
			_highPassSecond = configuration.highPassSecond;
			_lowPassFirst = configuration.lowPassFirst;
			_lowPassSecond = configuration.lowPassSecond;
		}
		reset();
	}

	void reset() {
		_highPassFirst.clear();
		_highPassSecond.clear();
		_lowPassFirst.clear();
		_lowPassSecond.clear();
	}

	int32 process(int32 input) {
		int32 output = _highPassFirst.process(input);
		output = _highPassSecond.process(output);
		output = _lowPassFirst.process(output);
		return _lowPassSecond.process(output);
	}

	int32 decay(int32 input) const {
		return multiplyFixed(input, _accumulatorDecay,
			kCoefficientFracBits);
	}
};

PCSpeakerPITRenderer::PCSpeakerPITRenderer(uint32 sampleRate,
		uint32 pitClock) :
	_sampleRate(sampleRate),
	_pitClock(pitClock),
	_outputStage(nullptr) {
	assert(_sampleRate);
	assert(_pitClock);
	_outputStage = new PCSpeakerOutputStage(sampleRate);
	initializeImpulse();
	resetState();
}

PCSpeakerPITRenderer::~PCSpeakerPITRenderer() {
	delete _outputStage;
}

void PCSpeakerPITRenderer::initializeImpulse() {
	struct ImpulseConfiguration {
		uint32 length;
		Common::Array<int32> coefficients;

		ImpulseConfiguration() : length(0) {
		}
	};

	static Common::Mutex impulseCacheMutex;
	static Common::HashMap<uint32, ImpulseConfiguration> impulseCache;
	Common::StackLock cacheLock(impulseCacheMutex);

	if (impulseCache.contains(_sampleRate)) {
		const ImpulseConfiguration &configuration =
			impulseCache.getVal(_sampleRate);
		_impulseLength = configuration.length;
		_impulseLut = configuration.coefficients;
		_impulseBuffer.resize(_impulseLength + 2, 0);
		return;
	}

	_impulseLength = MAX<uint32>(
		2, ((uint64)_sampleRate * kImpulseDurationUs + 999999) / 1000000);
	_impulseLut.resize(_impulseLength * kFractionalPhases, 0);
	_impulseBuffer.resize(_impulseLength + 2, 0);

	// Coefficients are constructed once per mixer rate, then quantized. The
	// mixer hot path below contains no floating-point operations.
	const double cutoff = MIN<double>(14500.0, _sampleRate * 0.45);
	const double center = _impulseLength / 2.0;
	Common::Array<double> phaseCoefficients(_impulseLength, 0.0);

	for (uint32 phase = 0; phase < kFractionalPhases; ++phase) {
		const double fraction = (double)phase / kFractionalPhases;
		double sum = 0.0;

		for (uint32 tap = 0; tap < _impulseLength; ++tap) {
			// Samples are emitted at the end of their interval. A transition
			// occurring partway through the current sample is this far in the
			// past when tap zero is emitted.
			const double time = tap + 1.0 - fraction;
			double coefficient = 0.0;
			if (time > 0.0 && time < _impulseLength) {
				const double distance = time - center;
				const double window =
					0.5 * (1.0 + cos(2.0 * M_PI * distance /
						_impulseLength));
				const double argument =
					2.0 * M_PI * cutoff * distance / _sampleRate;
				const double sinc = argument == 0.0 ?
					1.0 : sin(argument) / argument;
				coefficient = window * sinc;
			}

			phaseCoefficients[tap] = coefficient;
			sum += coefficient;
		}

		if (fabs(sum) < 1e-12) {
			phaseCoefficients[0] = 1.0;
			sum = 1.0;
		}
		uint32 largestTap = 0;
		int32 largestCoefficient = 0;
		int64 fixedSum = 0;
		for (uint32 tap = 0; tap < _impulseLength; ++tap) {
			const uint32 index = phase * _impulseLength + tap;
			const int32 coefficient = fixedFromDouble(
				phaseCoefficients[tap] / sum, kSampleFracBits);
			_impulseLut[index] = coefficient;
			fixedSum += coefficient;
			if (ABS<int32>(coefficient) > ABS<int32>(largestCoefficient)) {
				largestCoefficient = coefficient;
				largestTap = tap;
			}
		}

		// Correct quantization residue at the strongest tap so every phase
		// has exactly unit integrated gain in Q8.24.
		const uint32 largestIndex = phase * _impulseLength + largestTap;
		const int32 previousCoefficient = _impulseLut[largestIndex];
		const int64 adjustedCoefficient = (int64)previousCoefficient +
			((int64)1 << kSampleFracBits) - fixedSum;
		assert(adjustedCoefficient >= -0x80000000LL &&
			adjustedCoefficient <= 0x7fffffffLL);
		_impulseLut[largestIndex] = (int32)adjustedCoefficient;
		fixedSum += adjustedCoefficient - previousCoefficient;
		assert(fixedSum == ((int64)1 << kSampleFracBits));
	}

	ImpulseConfiguration configuration;
	configuration.length = _impulseLength;
	configuration.coefficients = _impulseLut;
	impulseCache.setVal(_sampleRate, configuration);
}

void PCSpeakerPITRenderer::resetState() {
	_phase = 0;
	_samplePhase = 0;
	_sampleCounter = 0;
	_count = 0;
	_pendingCount = 0;
	_hasPendingCount = false;
	_counterLoaded = false;
	_timerGate = false;
	_speakerEnabled = false;
	_high = true;
	_undersampled = false;
	_hasUndersampledReload = false;
	_lastUndersampledReloadSample = 0;
	_impulseHead = 0;
	for (uint i = 0; i < _impulseBuffer.size(); ++i)
		_impulseBuffer[i] = 0;
	_reconstructedLevel = 0;
	_targetLevel = -1;
	_outputStage->reset();
}

void PCSpeakerPITRenderer::advanceToSampleFraction(uint32 numerator,
		uint32 denominator) {
	assert(denominator);
	assert(numerator <= denominator);

	const uint64 samplePhase = ((uint64)numerator * _pitClock +
		denominator / 2) / denominator;
	assert(samplePhase >= _samplePhase);
	advanceCounter(samplePhase - _samplePhase);
}

bool PCSpeakerPITRenderer::isUndersampled(uint16 count) const {
	const uint32 minimumCount =
		((uint64)2 * _pitClock + _sampleRate - 1) / _sampleRate;
	// Mode 3 formally requires count >= 2. Treat count 1 as undersampled
	// compatibility input rather than inventing specified 8254 behavior.
	return count && count < minimumCount;
}

int PCSpeakerPITRenderer::outputLevel() const {
	if (!_speakerEnabled)
		return -1;

	// Disabling the timer gate forces mode-3 OUT high. Port 0x61 bit 1
	// controls whether that output reaches the physical speaker separately.
	if (!_timerGate)
		return 1;

	return _high ? 1 : -1;
}

void PCSpeakerPITRenderer::addTransition(int level,
		uint64 elapsedPitClocks) {
	if (level == _targetLevel)
		return;

	const int delta = level - _targetLevel;
	_targetLevel = level;

	elapsedPitClocks = MIN<uint64>(elapsedPitClocks, _pitClock);
	uint32 phase = (uint32)((elapsedPitClocks * kFractionalPhases +
		_pitClock / 2) / _pitClock);
	uint32 sampleOffset = 0;
	if (phase == kFractionalPhases) {
		phase = 0;
		sampleOffset = 1;
	}

	for (uint32 tap = 0; tap < _impulseLength; ++tap) {
		const uint32 bufferIndex =
			(_impulseHead + sampleOffset + tap) % _impulseBuffer.size();
		_impulseBuffer[bufferIndex] = saturateInt32((int64)
			_impulseBuffer[bufferIndex] +
			(int64)delta * _impulseLut[phase * _impulseLength + tap]);
	}
}

void PCSpeakerPITRenderer::writeMode3Count(uint16 count) {
	if (isUndersampled(count)) {
		// Counts above Nyquist cannot be represented as ordinary oscillation.
		// Rapid reloads are nevertheless used as a noise source, so preserve
		// that compatibility behavior by toggling the last physical level.
		const uint64 sampleGap = _sampleCounter -
			_lastUndersampledReloadSample;
		const bool rapidReload = _hasUndersampledReload &&
			sampleGap * 1000 <= _sampleRate;
		if (!_undersampled)
			_high = false;
		else if (_timerGate && _speakerEnabled && rapidReload)
			_high = !_high;

		_count = count;
		_pendingCount = 0;
		_hasPendingCount = false;
		_counterLoaded = true;
		_phase = 0;
		_undersampled = true;
		_hasUndersampledReload = true;
		_lastUndersampledReloadSample = _sampleCounter;
		addTransition(outputLevel(), _samplePhase);
		return;
	}

	const bool wasUndersampled = _undersampled;
	_undersampled = false;
	_hasUndersampledReload = false;

	// A mode-3 count written while the counter is running is transferred at
	// the next half-cycle rather than restarting the current waveform.
	if (_timerGate && _counterLoaded && !wasUndersampled) {
		_pendingCount = count;
		_hasPendingCount = true;
	} else {
		_count = count;
		_pendingCount = 0;
		_hasPendingCount = false;
		_counterLoaded = true;
		_phase = 0;
		_high = true;
		addTransition(outputLevel(), _samplePhase);
	}
}

void PCSpeakerPITRenderer::setControl(bool timerGate,
		bool speakerEnabled) {
	const bool timerGateChanged = timerGate != _timerGate;
	_timerGate = timerGate;
	_speakerEnabled = speakerEnabled;

	if (timerGateChanged) {
		if (!timerGate && _hasPendingCount) {
			// A gate stop cancels the current half-cycle. Retain the newest
			// programmed count for the next start.
			_count = _pendingCount;
			_pendingCount = 0;
			_hasPendingCount = false;
		}

		_phase = 0;
		_high = !timerGate || !_undersampled;
	}

	// Evaluate both port-0x61 controls together so a combined write never
	// exposes an intermediate speaker level.
	addTransition(outputLevel(), _samplePhase);
}

void PCSpeakerPITRenderer::advanceCounter(uint64 pitClockUnits) {
	assert(_samplePhase + pitClockUnits <= _pitClock);

	if (!_timerGate || !_counterLoaded || _undersampled) {
		_samplePhase += pitClockUnits;
		return;
	}

	// Phase is expressed in PIT-clock/output-rate products. This avoids
	// timer drift and retains every edge's fractional sample position.
	uint64 phaseToAdvance = pitClockUnits;
	uint64 elapsed = _samplePhase;

	while (phaseToAdvance) {
		// A programmed PIT count of zero represents 65536.
		const uint32 effectiveCount = _count ? _count : 0x10000;
		const uint32 halfCount = _high ?
			(effectiveCount + 1) / 2 : effectiveCount / 2;
		const uint64 halfPeriod =
			(uint64)MAX<uint32>(halfCount, 1) * _sampleRate;
		const uint64 toTransition = halfPeriod - _phase;
		const uint64 advance = MIN<uint64>(phaseToAdvance, toTransition);

		_phase += advance;
		phaseToAdvance -= advance;
		elapsed += advance;

		if (_phase == halfPeriod) {
			_phase = 0;
			_high = !_high;
			if (_hasPendingCount) {
				_count = _pendingCount;
				_pendingCount = 0;
				_hasPendingCount = false;
			}
			addTransition(outputLevel(), elapsed);
		}
	}

	_samplePhase += pitClockUnits;
}

int16 PCSpeakerPITRenderer::generateSample(byte volume) {
	advanceCounter(_pitClock - _samplePhase);

	_reconstructedLevel = saturateInt32((int64)_reconstructedLevel +
		_impulseBuffer[_impulseHead]);
	_impulseBuffer[_impulseHead] = 0;
	_impulseHead = (_impulseHead + 1) % _impulseBuffer.size();
	_samplePhase = 0;
	++_sampleCounter;

	// Output coupling and coloration are device state, so they continue while
	// muted. Volume is applied only after both stages have advanced.
	const int32 output = _outputStage->process(_reconstructedLevel);
	_reconstructedLevel = _outputStage->decay(_reconstructedLevel);
	const int64 scaled = (int64)output * 127 * volume;
	const int64 half = (int64)1 << (kSampleFracBits - 1);
	const int32 rounded = saturateInt32(scaled < 0 ?
		-(((-scaled) + half) >> kSampleFracBits) :
		(scaled + half) >> kSampleFracBits);
	return (int16)CLIP<int32>(rounded, -32768, 32767);
}

} // End of namespace MADS
