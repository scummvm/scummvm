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

/*
 * The code in this file is based on code with Copyright 1998 Fabrice Bellard
 * Fabrice original code is part of SoX (http://sox.sourceforge.net).
 * Max Horn adapted that code to the needs of ScummVM and rewrote it partial,
 * in the process removing any use of floating point arithmetic. Various other
 * improvements over the original code were made.
 */

#define FORCE_TEXT_CONSOLE

#include "audio/audiostream.h"
#include "audio/rate.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"

namespace Audio {

/**
 * The default fractional type in frac.h (with 16 fractional bits) limits
 * the rate conversion code to 65536Hz audio: we need to able to handle
 * 192kHz audio, so we use fewer fractional bits in this code.
 */
enum {
	FRAC_BITS_LOW = 14,
	FRAC_ONE_LOW = (1L << FRAC_BITS_LOW),
	FRAC_HALF_LOW = (1L << (FRAC_BITS_LOW-1))
};

template<bool inStereo, bool outStereo, bool reverseStereo>
class RateConverter_Impl : public RateConverter {
private:
	/** Input and output rates */
	st_rate_t _inRate, _outRate;

	/**
	 * The intermediate input cache. Bigger values may increase performance,
	 * but only until some point (depends largely on cache size, target
	 * processor and various other factors), at which it will decrease again.
	 */
	int16 _buffer[512];

	/** Current position inside the buffer */
	const int16 *_bufferPos;

	/** Size of data currently loaded into the buffer */
	int _bufferSize;

	/** How far output is ahead of input when doing simple conversion */
	frac_t _outPos;

	/** Fractional position of the output stream in input stream unit */
	frac_t _outPosFrac;

	/** Last sample(s) in the input stream (left/right channel) */
	int16 _inLastL, _inLastR;

	/** Current sample(s) in the input stream (left/right channel) */
	int16 _inCurL, _inCurR;

	template<st_volume_t volL, st_volume_t volR, typename st_sample_t, MixMode mixMode>
	int commonConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val, int outputSamples);

	template<st_volume_t volL = static_cast<st_volume_t>(-1), st_volume_t volR = static_cast<st_volume_t>(-1), typename st_sample_t, MixMode mixMode>
	int copyConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val);
	template<st_volume_t volL = static_cast<st_volume_t>(-1), st_volume_t volR = static_cast<st_volume_t>(-1), typename st_sample_t, MixMode mixMode>
	int downsampleConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val);
	template<st_volume_t volL = static_cast<st_volume_t>(-1), st_volume_t volR = static_cast<st_volume_t>(-1), typename st_sample_t, MixMode mixMode>
	int upsampleConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val);
	template<st_volume_t volL = static_cast<st_volume_t>(-1), st_volume_t volR = static_cast<st_volume_t>(-1), typename st_sample_t, MixMode mixMode>
	int interpolateConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val);

	template<typename st_sample_t, MixMode mixMode>
	int convertForType(AudioStream &input, byte *outBuffer, st_size_t numSamples, st_volume_t volL, st_volume_t volR);

	// keep a single printConvertType shared across all RateConverter_Impl specializations.
	// PrintContext must be trivially destructible: it lives in a function-scope static and
	// is torn down after the OSystem (and its memory pool that backs Common::String) is gone.
	struct PrintContext {
		st_rate_t previousInRate = 0;
		char previousGameId[64] = { 0 };
	};
	void printConvertType(const char *name, PrintContext &ctx) const {
		const Common::String &activeDomain = ConfMan.getActiveDomainName();
		if (!activeDomain.empty() &&
			(ctx.previousInRate != _inRate ||
			 strncmp(ctx.previousGameId, activeDomain.c_str(), sizeof(ctx.previousGameId)) != 0)) {
			ctx.previousInRate = _inRate;
			Common::strlcpy(ctx.previousGameId, activeDomain.c_str(), sizeof(ctx.previousGameId));
			debugC(kDebugLevelGAudio, "RateConverter_Impl::%s[%s]: inRate %d Hz (%s) => outRate %d Hz (%s)",
				  name, activeDomain.c_str(),
				  _inRate, inStereo ? "stereo" : "mono", _outRate, outStereo ? "stereo" : "mono");
		}
	}
	#define PRINT_OUTPUT_RATE \
		do { static PrintContext _ctx; printConvertType(__FUNCTION__, _ctx); } while (0)

public:
	RateConverter_Impl(st_rate_t inputRate, st_rate_t outputRate);
	virtual ~RateConverter_Impl() {}

	int convert(AudioStream &input, byte *outBuffer, uint outBytesPerSample, st_size_t numSamples, st_volume_t vol_l, st_volume_t vol_r, MixMode mixMode) override;

	void setInputRate(st_rate_t inputRate) override { _inRate = inputRate; }
	void setOutputRate(st_rate_t outputRate) override { _outRate = outputRate; }

	st_rate_t getInputRate() const override { return _inRate; }
	st_rate_t getOutputRate() const override { return _outRate; }

	bool needsDraining() const override { return _bufferSize != 0; }
};

template<bool inStereo, bool outStereo, bool reverseStereo>
template<st_volume_t volL, st_volume_t volR, typename st_sample_t, MixMode mixMode>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::commonConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val, int outputSamples) {
	const st_sample_t *outStart = outBuffer;
	const st_sample_t *outEnd = outBuffer + numSamples * (outStereo ? 2 : 1);

	while (outBuffer < outEnd) {
		// Check if we have to refill the buffer
		if (_bufferSize == 0) {
			_bufferPos = _buffer;
			_bufferSize = input.readBuffer(_buffer, ARRAYSIZE(_buffer));

			if (_bufferSize <= 0)
				return (outBuffer - outStart) / (outStereo ? 2 : 1);
		}

		// Process as many samples as we can from the current buffer
		const int count = MIN(
			_bufferSize / (inStereo ? 2 : 1),
			(int)(outEnd - outBuffer) / (outStereo ? 2 : 1) / outputSamples);
		_bufferSize -= count * (inStereo ? 2 : 1);

		if (volL | volR) {
			// Mix the data into the output buffer
			for (int i = 0; i < count; ++i) {
				int16 inL, inR;

				if (inStereo) {
					if (volL != 0)
						inL = *_bufferPos++;
					else
						_bufferPos++;

					if (volR != 0)
						inR = *_bufferPos++;
					else
						_bufferPos++;
				} else {
					if (volL != 0) {
						inL = *_bufferPos++;
						if (volR != 0)
							inR = inL;
					} else {
						inR = *_bufferPos++;
					}
				}

				st_sample_t outL, outR;

				if (volL != 0) {
					if (volL != Audio::Mixer::kMaxMixerVolume)
						outL = (inL * (int)volL_val) / Audio::Mixer::kMaxMixerVolume;
					else
						outL = inL;
				}

				if (volR != 0) {
					if (volR != Audio::Mixer::kMaxMixerVolume)
						outR = (inR * (int)volR_val) / Audio::Mixer::kMaxMixerVolume;
					else
						outR = inR;
				}

				// TODO: could be unrolled
				for (int j = 0; j < outputSamples; ++j) {
					if (outStereo) {
						// Output left channel
						if (volL != 0)
							processSample<mixMode>(outBuffer[reverseStereo    ], outL);

						// Output right channel
						if (volR != 0)
							processSample<mixMode>(outBuffer[reverseStereo ^ 1], outR);
					} else {
						// Output mono channel
						st_sample_t monoOut;
						if (volL != 0 && volR != 0)
							monoOut = (outL + outR) / 2;
						else if (volL != 0)
							monoOut = outL / 2;
						else if (volR != 0)
							monoOut = outR / 2;
						processSample<mixMode>(outBuffer[0], monoOut);
					}
					outBuffer += (outStereo ? 2 : 1);
				}
			}
		} else {
			_bufferPos += count * (inStereo ? 2 : 1);
			outBuffer += count * outputSamples * (outStereo ? 2 : 1);
		}
	}
	return (outBuffer - outStart) / (outStereo ? 2 : 1);
}

template<bool inStereo, bool outStereo, bool reverseStereo>
template<st_volume_t volL, st_volume_t volR, typename st_sample_t, MixMode mixMode>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::copyConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val) {
	PRINT_OUTPUT_RATE;

	return commonConvert<volL, volR, st_sample_t, mixMode>(input, outBuffer, numSamples, volL_val, volR_val, 1);
}

template<bool inStereo, bool outStereo, bool reverseStereo>
template<st_volume_t volL, st_volume_t volR, typename st_sample_t, MixMode mixMode>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::downsampleConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val) {
	PRINT_OUTPUT_RATE;

	// How much to increment _outPos by
	const frac_t outPos_inc = _inRate / _outRate;

	const st_sample_t *outStart = outBuffer;
	const st_sample_t *outEnd = outBuffer + numSamples * (outStereo ? 2 : 1);

	while (outBuffer < outEnd) {
		// Read enough input samples so that _outPos >= 0
		while (_outPos >= 0) {
			const int skip = MIN((int)_outPos + 1, _bufferSize / (inStereo ? 2 : 1));
			_bufferPos += skip * (inStereo ? 2 : 1);
			_bufferSize -= skip * (inStereo ? 2 : 1);
			_outPos -= skip;

			// Check if we have to refill the buffer
			if (_bufferSize == 0) {
				_bufferPos = _buffer;
				_bufferSize = input.readBuffer(_buffer, ARRAYSIZE(_buffer));

				if (_bufferSize <= 0)
					return (outBuffer - outStart) / (outStereo ? 2 : 1);
			}
		}

		// Process as many samples as we can from the current buffer
		const int count = MIN(
			_bufferSize / (inStereo ? 2 : 1) / outPos_inc,
			(int)(outEnd - outBuffer) / (outStereo ? 2 : 1));
		_bufferSize -= count * outPos_inc * (inStereo ? 2 : 1);
		_outPos = outPos_inc - 1;

		// Frame stride remaining after reading one frame
		const int stride = (outPos_inc - 1) * (inStereo ? 2 : 1);

		if (volL | volR) {
			for (int i = 0; i < count; ++i) {
				int16 inL, inR;

				if (inStereo) {
					if (volL != 0)
						inL = *_bufferPos++;
					else
						_bufferPos++;

					if (volR != 0)
						inR = *_bufferPos++;
					else
						_bufferPos++;
				} else {
					if (volL != 0) {
						inL = *_bufferPos++;
						if (volR != 0)
							inR = inL;
					} else {
						inR = *_bufferPos++;
					}
				}

				_bufferPos += stride;

				st_sample_t outL, outR;
				if (volL != 0) {
					if (volL != Audio::Mixer::kMaxMixerVolume)
						outL = (inL * (int)volL_val) / Audio::Mixer::kMaxMixerVolume;
					else
						outL = inL;
				}
				if (volR != 0) {
					if (volR != Audio::Mixer::kMaxMixerVolume)
						outR = (inR * (int)volR_val) / Audio::Mixer::kMaxMixerVolume;
					else
						outR = inR;
				}

				if (outStereo) {
					// Output left channel
					if (volL != 0)
						processSample<mixMode>(outBuffer[reverseStereo    ], outL);

					// Output right channel
					if (volR != 0)
						processSample<mixMode>(outBuffer[reverseStereo ^ 1], outR);
				} else {
					// Output mono channel
					st_sample_t monoOut;
					if (volL != 0 && volR != 0)
						monoOut = (outL + outR) / 2;
					else if (volL != 0)
						monoOut = outL / 2;
					else if (volR != 0)
						monoOut = outR / 2;
					processSample<mixMode>(outBuffer[0], monoOut);
				}

				outBuffer += (outStereo ? 2 : 1);
			}
		} else {
			_bufferPos += count * outPos_inc * (inStereo ? 2 : 1);
			outBuffer += count * (outStereo ? 2 : 1);
		}
	}
	return (outBuffer - outStart) / (outStereo ? 2 : 1);
}

template<bool inStereo, bool outStereo, bool reverseStereo>
template<st_volume_t volL, st_volume_t volR, typename st_sample_t, MixMode mixMode>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::upsampleConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val) {
	PRINT_OUTPUT_RATE;

	return commonConvert<volL, volR, st_sample_t, mixMode>(input, outBuffer, numSamples, volL_val, volR_val, _outRate / _inRate);
}

template<bool inStereo, bool outStereo, bool reverseStereo>
template<st_volume_t volL, st_volume_t volR, typename st_sample_t, MixMode mixMode>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::interpolateConvert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t volL_val, st_volume_t volR_val) {
	PRINT_OUTPUT_RATE;

	// How much to increment _outPosFrac by
	const frac_t outPos_inc = (_inRate << FRAC_BITS_LOW) / _outRate;

	const st_sample_t *outStart = outBuffer;
	const st_sample_t *outEnd = outBuffer + numSamples * (outStereo ? 2 : 1);

	while (outBuffer < outEnd) {
		// Read enough input samples so that _outPosFrac < 0
		while ((frac_t)FRAC_ONE_LOW <= _outPosFrac) {
			// Check if we have to refill the buffer
			if (_bufferSize == 0) {
				_bufferPos = _buffer;
				_bufferSize = input.readBuffer(_buffer, ARRAYSIZE(_buffer));

				if (_bufferSize <= 0)
					return (outBuffer - outStart) / (outStereo ? 2 : 1);
			}

			_bufferSize -= (inStereo ? 2 : 1);

			if (volL != 0 || (!inStereo && volR != 0)) {
				_inLastL = _inCurL;
				_inCurL = *_bufferPos++;
			} else {
				_bufferPos++;
			}

			if (inStereo) {
				if (volR != 0) {
					_inLastR = _inCurR;
					_inCurR = *_bufferPos++;
				} else {
					_bufferPos++;
				}
			}

			_outPosFrac -= FRAC_ONE_LOW;
		}

		// Loop as long as the _outPos trails behind, and as long as there is
		// still space in the output buffer.
		while (_outPosFrac < (frac_t)FRAC_ONE_LOW && outBuffer < outEnd) {
			if (volL | volR) {
				// Interpolate
				int16 inL, inR;

				if (volL != 0 || (!inStereo && volR != 0)) {
					inL = (int16)(_inLastL + (((_inCurL - _inLastL) * _outPosFrac + FRAC_HALF_LOW) >> FRAC_BITS_LOW));
				}

				if (volR != 0) {
					inR = (inStereo ?
						(int16)(_inLastR + (((_inCurR - _inLastR) * _outPosFrac + FRAC_HALF_LOW) >> FRAC_BITS_LOW)) :
						inL);
				}

				st_sample_t outL, outR;
				if (volL != 0) {
					if (volL != Audio::Mixer::kMaxMixerVolume)
						outL = (inL * (int)volL_val) / Audio::Mixer::kMaxMixerVolume;
					else
						outL = inL;
				}

				if (volR != 0) {
					if (volR != Audio::Mixer::kMaxMixerVolume)
						outR = (inR * (int)volR_val) / Audio::Mixer::kMaxMixerVolume;
					else
						outR = inR;
				}

				if (outStereo) {
					// Output left channel
					if (volL != 0)
						processSample<mixMode>(outBuffer[reverseStereo    ], outL);

					// Output right channel
					if (volR != 0)
						processSample<mixMode>(outBuffer[reverseStereo ^ 1], outR);
				} else {
					// Output mono channel
					st_sample_t monoOut;
					if (volL != 0 && volR != 0)
						monoOut = (outL + outR) / 2;
					else if (volL != 0)
						monoOut = outL / 2;
					else
						monoOut = outR / 2;
					processSample<mixMode>(outBuffer[0], monoOut);
				}
			}

			outBuffer += (outStereo ? 2 : 1);

			// Increment output position
			_outPosFrac += outPos_inc;
		}
	}
	return (outBuffer - outStart) / (outStereo ? 2 : 1);
}

template<bool inStereo, bool outStereo, bool reverseStereo>
RateConverter_Impl<inStereo, outStereo, reverseStereo>::RateConverter_Impl(st_rate_t inputRate, st_rate_t outputRate) :
	_inRate(inputRate),
	_outRate(outputRate),
	_outPos(1),
	_outPosFrac(FRAC_ONE_LOW),
	_inLastL(0),
	_inLastR(0),
	_inCurL(0),
	_inCurR(0),
	_bufferSize(0),
	_bufferPos(nullptr) {}

template<bool inStereo, bool outStereo, bool reverseStereo>
template<typename st_sample_t, MixMode mixMode>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::convertForType(AudioStream &input, byte *outBuffer, st_size_t numSamples, st_volume_t volL, st_volume_t volR) {
	assert(input.isStereo() == inStereo);

	constexpr auto kMax = Audio::Mixer::kMaxMixerVolume;

	if (_inRate == _outRate) {
		if (volL == 0 && volR == 0)
			return copyConvert<0, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == 0 && volR == kMax)
			return copyConvert<0, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == 0)
			return copyConvert<kMax, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == kMax)
			return copyConvert<kMax, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else
			return copyConvert<static_cast<st_volume_t>(-1), static_cast<st_volume_t>(-1), st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
	} else if ((_inRate % _outRate) == 0 && (_inRate < 65536)) {
		if (volL == 0 && volR == 0)
			return downsampleConvert<0, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == 0 && volR == kMax)
			return downsampleConvert<0, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == 0)
			return downsampleConvert<kMax, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == kMax)
			return downsampleConvert<kMax, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else
			return downsampleConvert<static_cast<st_volume_t>(-1), static_cast<st_volume_t>(-1), st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
	} else if ((_outRate % _inRate) == 0) {
		if (volL == 0 && volR == 0)
			return upsampleConvert<0, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == 0 && volR == kMax)
			return upsampleConvert<0, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == 0)
			return upsampleConvert<kMax, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == kMax)
			return upsampleConvert<kMax, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else
			return upsampleConvert<static_cast<st_volume_t>(-1), static_cast<st_volume_t>(-1), st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
	} else {
		if (volL == 0 && volR == 0)
			return interpolateConvert<0, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == 0 && volR == kMax)
			return interpolateConvert<0, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == 0)
			return interpolateConvert<kMax, 0, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else if (volL == kMax && volR == kMax)
			return interpolateConvert<kMax, kMax, st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
		else
			return interpolateConvert<static_cast<st_volume_t>(-1), static_cast<st_volume_t>(-1), st_sample_t, mixMode>(input, (st_sample_t *)outBuffer, numSamples, volL, volR);
	}
}

template<bool inStereo, bool outStereo, bool reverseStereo>
int RateConverter_Impl<inStereo, outStereo, reverseStereo>::convert(AudioStream &input, byte *outBuffer, uint outBytesPerSample, st_size_t numSamples, st_volume_t volL, st_volume_t volR, MixMode mixMode) {
	if (outBytesPerSample == sizeof(int32)) {
		if (mixMode == MIX_ADD)
			return convertForType<int32, MIX_ADD>(input, outBuffer, numSamples, volL, volR);
		else
			return convertForType<int32, MIX_CLAMPED_ADD>(input, outBuffer, numSamples, volL, volR);
	} else {
		if (mixMode == MIX_ADD)
			return convertForType<int16, MIX_ADD>(input, outBuffer, numSamples, volL, volR);
		else
			return convertForType<int16, MIX_CLAMPED_ADD>(input, outBuffer, numSamples, volL, volR);
	}
}

RateConverter *makeRateConverter(st_rate_t inRate, st_rate_t outRate, bool inStereo, bool outStereo, bool reverseStereo) {
	if (inStereo) {
		if (outStereo) {
			if (reverseStereo)
				return new RateConverter_Impl<true, true, true>(inRate, outRate);
			else
				return new RateConverter_Impl<true, true, false>(inRate, outRate);
		} else
			return new RateConverter_Impl<true, false, false>(inRate, outRate);
	} else {
		if (outStereo) {
			return new RateConverter_Impl<false, true, false>(inRate, outRate);
		} else
			return new RateConverter_Impl<false, false, false>(inRate, outRate);
	}
}

} // End of namespace Audio
