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

#include "audio/softsynth/ay8912.h"

#include "freescape/freescape.h"

namespace Freescape {

/**
 * CPC Sound Implementation (shared by Driller, Dark Side, and other Freescape CPC games)
 *
 * Based on reverse engineering of DRILL.BIN and DARKCODE.BIN (both load at 0x1C62).
 * The sound engine is identical across games; only table contents and sizes differ.
 *
 * All sounds use the sub_4760h system:
 *   - Sound initialization loads 7-byte entry from the sound definition table
 *   - Volume envelope from "Tone" Table
 *   - Pitch sweep from "Envelope" Table
 *   - 300Hz interrupt-driven update
 *
 * AY-3-8912 PSG with 1MHz clock:
 *   Port 0xF4 = register select, Port 0xF6 = data
 *
 * ---- Sound Definition Table ----
 * N entries, 7 bytes each. Loaded with 1-based sound number.
 *   Byte 0: flags
 *     - Bits 0-1: channel number (1=A, 2=B, 3=C)
 *     - Bit 2: tone disable (0 = enable tone, 1 = disable)
 *     - Bit 3: noise disable (0 = enable noise, 1 = disable)
 *   Byte 1: "tone" table index (volume envelope)
 *   Byte 2: "envelope" table index (pitch sweep)
 *   Bytes 3-4: initial AY tone period (little-endian, 12-bit)
 *   Byte 5: initial AY volume (0-15)
 *   Byte 6: duration (repeat count; 0 = single play)
 *
 * ---- "Tone" Table - Volume Envelope ----
 * Despite the name, this table controls VOLUME modulation, not pitch.
 * Indexed by 4-byte stride: base = index * 4.
 *   Byte 0: number of triplets (N)
 *   Then N triplets of 3 bytes each:
 *     Byte 0: counter - how many times to apply the delta
 *     Byte 1: delta (signed) - added to volume each step
 *     Byte 2: limit - ticks between each application
 *
 * ---- "Envelope" Table - Pitch Sweep ----
 * Despite the name, this table controls PITCH modulation, not envelope.
 * Indexed by 4-byte stride: base = index * 4.
 *   Byte 0: number of triplets (N)
 *   Then N triplets of 3 bytes each:
 *     Byte 0: counter - how many times to apply the delta
 *     Byte 1: delta (signed) - added to period each step
 *     Byte 2: limit - ticks between each application
 */

class CPCSfxStream : public Audio::AY8912Stream {
public:
	CPCSfxStream(int index, const byte *soundDefTable, int soundDefTableSize,
	             const byte *toneTable, const byte *envelopeTable, int rate = 44100)
		: AY8912Stream(rate, 1000000),
		  _soundDefTable(soundDefTable), _soundDefTableSize(soundDefTableSize),
		  _toneTable(toneTable), _envelopeTable(envelopeTable) {
		_finished = false;
		_tickSampleCount = 0;

		// Reset all AY registers to match CPC init state
		for (int r = 0; r < 14; r++)
			setReg(r, 0);
		// Noise period from CPC init table (verified in binary)
		setReg(6, 0x07);

		memset(&_ch, 0, sizeof(_ch));
		setupSound(index);
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		if (_finished)
			return 0;

		int samplesGenerated = 0;
		// AY8912Stream is stereo: readBuffer counts int16 values (2 per frame).
		// CPC interrupts fire at 300Hz (6 per frame). The update routine is called
		// unconditionally at every interrupt, NOT inside the 50Hz divider.
		int samplesPerTick = (getRate() / 300) * 2;

		while (samplesGenerated < numSamples && !_finished) {
			// Generate samples until next tick
			int remaining = samplesPerTick - _tickSampleCount;
			int toGenerate = MIN(numSamples - samplesGenerated, remaining);

			if (toGenerate > 0) {
				generateSamples(buffer + samplesGenerated, toGenerate);
				samplesGenerated += toGenerate;
				_tickSampleCount += toGenerate;
			}

			// Run interrupt handler at 300Hz tick boundary
			if (_tickSampleCount >= samplesPerTick) {
				_tickSampleCount -= samplesPerTick;
				tickUpdate();
			}
		}

		return samplesGenerated;
	}

	bool endOfData() const override { return _finished; }
	bool endOfStream() const override { return _finished; }

private:
	bool _finished;
	int _tickSampleCount; // Samples generated in current tick

	// Pointers to table data loaded from game binary (owned by FreescapeEngine)
	const byte *_soundDefTable;
	int _soundDefTableSize;      // Size in bytes (numSounds * 7)
	const byte *_toneTable;      // Volume envelope data
	const byte *_envelopeTable;  // Pitch sweep data

	/**
	 * Channel state - mirrors the 23-byte per-channel structure
	 * as populated by the init routine and updated at 300Hz.
	 *
	 * "vol" fields come from the "tone" table - controls volume envelope
	 * "pitch" fields come from the "envelope" table - controls pitch sweep
	 */
	struct ChannelState {
		// Volume modulation (from "tone" table)
		byte volCounter;        // ix+000h: initial counter value
		int8 volDelta;          // ix+001h: signed delta added to volume
		byte volLimit;          // ix+002h: initial limit value
		byte volCounterCur;     // ix+003h: current counter (decremented)
		byte volLimitCur;       // ix+004h: current limit countdown
		byte volume;            // ix+005h: current AY volume (0-15)
		byte volTripletTotal;   // ix+006h: total number of volume triplets
		byte volCurrentStep;    // ix+007h: current triplet index
		byte duration;          // ix+008h: repeat count
		byte volToneIdx;        // tone table index (to recompute data pointer)

		// Pitch modulation (from "envelope" table)
		byte pitchCounter;      // ix+00Bh: initial counter value
		int8 pitchDelta;        // ix+00Ch: signed delta added to period
		byte pitchLimit;        // ix+00Dh: initial limit value
		byte pitchCounterCur;   // ix+00Eh: current counter (decremented)
		byte pitchLimitCur;     // ix+00Fh: current limit countdown
		uint16 period;          // ix+010h-011h: current 16-bit AY tone period
		byte pitchTripletTotal; // ix+012h: total number of pitch triplets
		byte pitchCurrentStep;  // ix+013h: current triplet index
		byte pitchEnvIdx;       // envelope table index (to recompute data pointer)

		byte finishedFlag;      // ix+016h: set when volume envelope exhausted

		// AY register mapping for this channel
		byte channelNum;        // 1=A, 2=B, 3=C
		byte toneRegLo;         // AY register for tone fine
		byte toneRegHi;         // AY register for tone coarse
		byte volReg;            // AY register for volume
		bool active;             // Channel is producing sound
	} _ch;

	void writeReg(int reg, byte val) {
		setReg(reg, val);
	}

	void setupSound(int index) {
		int maxSounds = _soundDefTableSize / 7;
		if (index >= 1 && index <= maxSounds) {
			setupSub4760h(index);
		} else {
			_finished = true;
		}
	}

	/**
	 * Sound initialization - loads 7-byte entry and configures AY registers.
	 */
	void setupSub4760h(int soundNum) {
		int maxSounds = _soundDefTableSize / 7;
		if (soundNum < 1 || soundNum > maxSounds) {
			_finished = true;
			return;
		}

		const byte *entry = &_soundDefTable[(soundNum - 1) * 7];
		byte flags = entry[0];
		byte toneIdx = entry[1];
		byte envIdx = entry[2];
		uint16 period = entry[3] | (entry[4] << 8);
		byte volume = entry[5];
		byte duration = entry[6];

		// Channel number (1-based): 1=A, 2=B, 3=C
		byte channelNum = flags & 0x03;
		if (channelNum < 1 || channelNum > 3) {
			_finished = true;
			return;
		}

		// AY register mapping
		_ch.channelNum = channelNum;
		_ch.toneRegLo = (channelNum - 1) * 2;       // A=0, B=2, C=4
		_ch.toneRegHi = (channelNum - 1) * 2 + 1;   // A=1, B=3, C=5
		_ch.volReg = channelNum + 7;                 // A=8, B=9, C=10

		// Configure mixer (register 7)
		// Start with all disabled (0xFF), selectively enable per flags
		// Bit 2 set in flags = DISABLE tone, Bit 3 set = DISABLE noise
		byte mixer = 0xFF;
		if (!(flags & 0x04))
			mixer &= ~(1 << (channelNum - 1));        // Enable tone
		if (!(flags & 0x08))
			mixer &= ~(1 << (channelNum - 1 + 3));    // Enable noise
		writeReg(7, mixer);

		// Set AY tone period from entry[3-4]
		_ch.period = period;
		writeReg(_ch.toneRegLo, period & 0xFF);
		writeReg(_ch.toneRegHi, period >> 8);

		// Set AY volume from entry[5]
		_ch.volume = volume;
		writeReg(_ch.volReg, volume);

		// Duration from entry[6]
		_ch.duration = duration;

		// Load volume envelope from "tone" table
		// index * 4 stride, byte[0]=triplet_count, then {counter, delta, limit}
		int toneBase = toneIdx * 4;
		_ch.volTripletTotal = _toneTable[toneBase];
		_ch.volCurrentStep = 0;
		_ch.volToneIdx = toneIdx;

		// Load first volume triplet
		int volOff = toneBase + 1;
		_ch.volCounter = _toneTable[volOff];
		_ch.volDelta = static_cast<int8>(_toneTable[volOff + 1]);
		_ch.volLimit = _toneTable[volOff + 2];
		_ch.volCounterCur = _ch.volCounter;
		_ch.volLimitCur = _ch.volLimit;

		// Load pitch sweep from "envelope" table
		// index * 4 stride, byte[0]=triplet_count, then {counter, delta, limit}
		int envBase = envIdx * 4;
		_ch.pitchTripletTotal = _envelopeTable[envBase];
		_ch.pitchCurrentStep = 0;
		_ch.pitchEnvIdx = envIdx;

		// Load first pitch triplet
		int pitchOff = envBase + 1;
		_ch.pitchCounter = _envelopeTable[pitchOff];
		_ch.pitchDelta = static_cast<int8>(_envelopeTable[pitchOff + 1]);
		_ch.pitchLimit = _envelopeTable[pitchOff + 2];
		_ch.pitchCounterCur = _ch.pitchCounter;
		_ch.pitchLimitCur = _ch.pitchLimit;

		_ch.finishedFlag = 0;
		_ch.active = true;

		debugC(1, kFreescapeDebugMedia, "CPC sound init: sound %d ch=%d mixer=0x%02x period=%d vol=%d dur=%d tone[%d] env[%d]",
			soundNum, channelNum, mixer, period, volume, duration, toneIdx, envIdx);
		debugC(1, kFreescapeDebugMedia, "  vol envelope: triplets=%d counter=%d delta=%d limit=%d",
			_ch.volTripletTotal, _ch.volCounter, _ch.volDelta, _ch.volLimit);
		debugC(1, kFreescapeDebugMedia, "  pitch sweep:  triplets=%d counter=%d delta=%d limit=%d",
			_ch.pitchTripletTotal, _ch.pitchCounter, _ch.pitchDelta, _ch.pitchLimit);
	}

	/**
	 * 300Hz interrupt-driven update. Updates pitch first, then volume.
	 */
	void tickUpdate() {
		if (!_ch.active) {
			_finished = true;
			return;
		}

		const byte *toneRaw = _toneTable;
		const byte *envRaw = _envelopeTable;

		// === PITCH UPDATE ===
		_ch.pitchLimitCur--;
		if (_ch.pitchLimitCur == 0) {
			// Reload limit countdown
			_ch.pitchLimitCur = _ch.pitchLimit;

			// period += sign_extend(pitchDelta) with natural 16-bit wrapping
			_ch.period += static_cast<int8>(_ch.pitchDelta);

			// Write period to AY tone registers (AY masks coarse to 4 bits)
			writeReg(_ch.toneRegLo, _ch.period & 0xFF);
			writeReg(_ch.toneRegHi, _ch.period >> 8);

			// Decrement pitch counter
			_ch.pitchCounterCur--;
			if (_ch.pitchCounterCur == 0) {
				// Advance to next pitch triplet
				_ch.pitchCurrentStep++;
				if (_ch.pitchCurrentStep >= _ch.pitchTripletTotal) {
					// All pitch triplets exhausted -> check duration
					_ch.duration--;
					if (_ch.duration == 0) {
						// SHUTDOWN: silence and deactivate
						writeReg(_ch.volReg, 0);
						_ch.active = false;
						_finished = true;
						return;
					}
					// Duration > 0: restart BOTH volume and pitch from beginning

					// Reload first volume triplet (from tone table)
					int volOff = _ch.volToneIdx * 4 + 1;
					_ch.volCounter = toneRaw[volOff];
					_ch.volDelta = static_cast<int8>(toneRaw[volOff + 1]);
					_ch.volLimit = toneRaw[volOff + 2];
					_ch.volCounterCur = _ch.volCounter;
					_ch.volLimitCur = _ch.volLimit;

					// Reset both position indices and done flag
					_ch.volCurrentStep = 0;
					_ch.pitchCurrentStep = 0;
					_ch.finishedFlag = 0;

					// Reload first pitch triplet (from envelope table)
					int off = _ch.pitchEnvIdx * 4 + 1;
					_ch.pitchCounter = envRaw[off];
					_ch.pitchDelta = static_cast<int8>(envRaw[off + 1]);
					_ch.pitchLimit = envRaw[off + 2];
					_ch.pitchCounterCur = _ch.pitchCounter;
					_ch.pitchLimitCur = _ch.pitchLimit;
				} else {
					// Load next pitch triplet
					int off = _ch.pitchEnvIdx * 4 + 1 + _ch.pitchCurrentStep * 3;
					_ch.pitchCounter = envRaw[off];
					_ch.pitchDelta = static_cast<int8>(envRaw[off + 1]);
					_ch.pitchLimit = envRaw[off + 2];
					_ch.pitchCounterCur = _ch.pitchCounter;
					_ch.pitchLimitCur = _ch.pitchLimit;
				}
			}
		}

		// === VOLUME UPDATE ===
		if (!_ch.finishedFlag) {
			_ch.volLimitCur--;
			if (_ch.volLimitCur == 0) {
				// Reload limit countdown
				_ch.volLimitCur = _ch.volLimit;

				// volume = (volume + volDelta) & 0x0F
				_ch.volume = (_ch.volume + _ch.volDelta) & 0x0F;
				writeReg(_ch.volReg, _ch.volume);

				// Decrement volume counter
				_ch.volCounterCur--;
				if (_ch.volCounterCur == 0) {
					// Advance to next volume triplet
					_ch.volCurrentStep++;
					if (_ch.volCurrentStep >= _ch.volTripletTotal) {
						// All volume triplets exhausted -> set finished flag
						// NOTE: Does NOT shutdown channel - pitch continues
						_ch.finishedFlag = 1;
					} else {
						// Load next volume triplet
						int off = _ch.volToneIdx * 4 + 1 + _ch.volCurrentStep * 3;
						_ch.volCounter = toneRaw[off];
						_ch.volDelta = static_cast<int8>(toneRaw[off + 1]);
						_ch.volLimit = toneRaw[off + 2];
						_ch.volCounterCur = _ch.volCounter;
						_ch.volLimitCur = _ch.volLimit;
					}
				}
			}
		}
	}
};

void FreescapeEngine::playSoundCPC(int index, Audio::SoundHandle &handle) {
	if (_soundsCPCSoundDefTable.empty()) {
		debugC(1, kFreescapeDebugMedia, "CPC sound tables not loaded");
		return;
	}
	debugC(1, kFreescapeDebugMedia, "Playing CPC sound %d", index);
	CPCSfxStream *stream = new CPCSfxStream(index,
		_soundsCPCSoundDefTable.data(), _soundsCPCSoundDefTable.size(),
		_soundsCPCToneTable.data(), _soundsCPCEnvelopeTable.data());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream->toAudioStream(), -1, kFreescapeDefaultVolume, 0, DisposeAfterUse::YES);
}

} // namespace Freescape
