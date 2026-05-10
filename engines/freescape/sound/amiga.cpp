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

#include "audio/audiostream.h"
#include "audio/mods/module.h"
#include "audio/mods/paula.h"

#include "common/memstream.h"

#include "freescape/freescape.h"

namespace Freescape {

/**
 * Amiga Sound Effect Synthesizer
 *
 * Synthesizes sound effects from a command stream, emulating the Castle Master
 * Amiga demo's custom sound engine. All 4 Amiga audio channels play the same
 * 64-byte square wave buffer (alternating +64/-64 signed bytes).
 *
 * Command format: 16-bit big-endian words.
 *   Bits 15-12: command type
 *   Bits 11-0:  parameter
 *
 * Period commands (set absolute frequency):
 *   0x0xxx: AUD1 period = xxx  (0 disables channel)
 *   0x1xxx: AUD2 period = xxx
 *   0x2xxx: AUD3 period = xxx
 *   0x3xxx: AUD0 period = xxx
 *
 * Relative period commands (pitch bend):
 *   0x8xxx: AUD1 period += sign_extend_12(xxx)
 *   0x9xxx: AUD2 period += sign_extend_12(xxx)
 *   0xAxxx: AUD3 period += sign_extend_12(xxx)
 *   0xBxxx: AUD0 period += sign_extend_12(xxx)
 *
 * Volume commands (channel select in bits 11-8):
 *   0x4Yxx: set volume = xx (Y=1: AUD1, Y=2: AUD2, else: AUD0+AUD3)
 *   0xCYxx: volume += sign_extend_8(xx) (same channel mapping)
 *
 * Control commands:
 *   0x5NNN: play note (reads 3 extra words; 3rd word = DMA repeat count)
 *   0x6xxx: delay xxx VBI ticks (50Hz)
 *   0x7000: full stop (silence all, end stream)
 *   0x7001: pause until DMA playback completes
 *   0x7002: loop (decrement counter, jump to saved position if > 0)
 *   0xDxxx: save loop position, set loop counter = xxx
 *   0xFxxx: end (stop interpreter)
 *
 * Tone frequency: 3,546,895 / (period * 2) Hz.
 * Paula plays bytes at 3,546,895 / period, and the 0x40/0xC0 waveform
 * alternates every byte (2 samples per cycle), adding a /2.
 */
class AmigaSfxStream : public Audio::Paula {
public:
	AmigaSfxStream(const uint16 *commands, int numCommands, const Common::Array<AmigaDmaSample> *dmaSamples, int rate = 44100)
		: Audio::Paula(false, rate, rate / 50),
		  _dmaSamples(dmaSamples),
		  _cmdPos(0), _delay(0), _paused(false),
		  _dmaCounter(0), _dmaAud0Active(false), _loopPos(0), _loopCounter(0),
		  _graceCounter(0) {

		_commands.resize(numCommands);
		for (int i = 0; i < numCommands; i++)
			_commands[i] = commands[i];

		for (int i = 0; i < 64; i += 2) {
			_squareWave[i] = 64;
			_squareWave[i + 1] = -64;
		}

		// FUN_2520 init values.
		static const uint16 initPeriods[4] = { 0x1A1, 0x1AB, 0x1B5, 0x1BF };
		for (int ch = 0; ch < 4; ch++) {
			_periodShadow[ch] = initPeriods[ch];
			_volumeShadow[ch] = 0;
			_channelEnabled[ch] = false;

			setChannelSampleStart(ch, _squareWave);
			setChannelSampleLen(ch, 0x20); // 32 words = 64 bytes
			setChannelPeriod(ch, _periodShadow[ch]);
			setChannelVolume(ch, 0);
			// FUN_2520 writes DMACON=0x000F (clear audio DMA bits), so channels
			// are configured but disabled until command handlers enable them.
			disableChannel(ch);
		}
		startPaula();
	}

private:
	void interrupt() override {
		tickUpdate();
	}

	Common::Array<uint16> _commands;
	const Common::Array<AmigaDmaSample> *_dmaSamples;
	int _cmdPos;
	int _delay;         // -1 = stopped, 0 = execute next, >0 = waiting
	bool _paused;       // Waiting for DMA completion
	int _dmaCounter;    // DMA ticks remaining (approximate)
	bool _dmaAud0Active;
	int _loopPos;       // Saved command position for looping
	int _loopCounter;   // Loop iterations remaining
	int _graceCounter;  // Ticks to keep playing after END before finishing
	uint16 _periodShadow[4];
	int _volumeShadow[4];
	bool _channelEnabled[4]; // Tracks DMA enable state per channel
	int8 _squareWave[64];

	/**
	 * Map period command nibble (0-3) to internal channel index.
	 * Command 0 -> AUD1 (ch 1), 1 -> AUD2 (ch 2), 2 -> AUD3 (ch 3), 3 -> AUD0 (ch 0)
	 */
	static int periodCmdToChannel(int nibble) {
		return (nibble + 1) & 3;
	}

	uint8 clampVolume(int value) const {
		return (uint8)CLIP<int>(value, 0, 64);
	}

	void setAbsolutePeriod(int ch, uint16 period) {
		_periodShadow[ch] = period;
		setChannelPeriod(ch, period);
		if (!_channelEnabled[ch]) {
			// Channel was off -> enable DMA (like writing DMACON with SET bit).
			// Restore square wave for AUD0 if not playing a DMA sample.
			if (ch == 0 && !_dmaAud0Active) {
				setChannelSampleStart(0, _squareWave);
				setChannelSampleLen(0, 0x20);
			}
			enableChannel(ch);
			_channelEnabled[ch] = true;
		}
		// If already enabled, just the period register update above is
		// sufficient. On real hardware, writing AUDxPER only changes the
		// DMA fetch rate without restarting the buffer position.
	}

	void setRelativePeriod(int ch, int16 delta) {
		// Original only writes to shadow + period register.
		// Does NOT touch DMACON - channel retains its current enable state.
		uint16 newPeriod = (uint16)(_periodShadow[ch] + delta);
		_periodShadow[ch] = newPeriod;
		if (newPeriod == 0) {
			disableChannel(ch);
			_channelEnabled[ch] = false;
			return;
		}
		setChannelPeriod(ch, newPeriod);
	}

	void setAbsoluteVolume(int sel, uint8 vol) {
		if (sel == 1) {
			_volumeShadow[1] = vol;
			setChannelVolume(1, clampVolume(_volumeShadow[1]));
		} else if (sel == 2) {
			_volumeShadow[2] = vol;
			setChannelVolume(2, clampVolume(_volumeShadow[2]));
		} else {
			_volumeShadow[0] = vol;
			_volumeShadow[3] = vol;
			setChannelVolume(0, clampVolume(_volumeShadow[0]));
			setChannelVolume(3, clampVolume(_volumeShadow[3]));
		}
	}

	void addRelativeVolume(int sel, int8 delta) {
		if (sel == 1) {
			_volumeShadow[1] += delta;
			setChannelVolume(1, clampVolume(_volumeShadow[1]));
		} else if (sel == 2) {
			_volumeShadow[2] += delta;
			setChannelVolume(2, clampVolume(_volumeShadow[2]));
		} else {
			_volumeShadow[0] += delta;
			_volumeShadow[3] += delta;
			setChannelVolume(0, clampVolume(_volumeShadow[0]));
			setChannelVolume(3, clampVolume(_volumeShadow[3]));
		}
	}

	void tickUpdate() {
		if (_dmaCounter > 0) {
			_dmaCounter--;
			if (_dmaCounter == 0 && _dmaAud0Active) {
				disableChannel(0);
				_channelEnabled[0] = false;
				setChannelSampleStart(0, _squareWave);
				setChannelSampleLen(0, 0x20);
				setChannelOffset(0, Audio::Paula::Offset(0));
				_dmaAud0Active = false;
			}
		}

		if (_paused) {
			if (_dmaCounter <= 0)
				_paused = false;
			else
				return;
		}

		if (_delay < 0) {
			// After END command, allow a grace period so short sounds
			// remain audible (original hardware keeps channels playing
			// until next sound trigger silences them).
			if (_graceCounter > 0) {
				_graceCounter--;
				return;
			}
			_dmaAud0Active = false;
			setChannelSampleStart(0, _squareWave);
			setChannelSampleLen(0, 0x20);
			setChannelOffset(0, Audio::Paula::Offset(0));
			stopPaula();
			return;
		}

		if (_delay > 0) {
			_delay--;
			return;
		}

		// _delay == 0: execute commands
		executeCommands();
	}

	void executeCommands() {
		// Process commands until we hit a delay, stop, or end
		while (_cmdPos < (int)_commands.size()) {
			uint16 cmd = _commands[_cmdPos++];
			int nibble = (cmd >> 12) & 0xF;
			int param = cmd & 0xFFF;

			switch (nibble) {
			case 0: case 1: case 2: case 3: {
				// Set absolute period
				int ch = periodCmdToChannel(nibble);
				if (param == 0) {
					_periodShadow[ch] = 0;
					disableChannel(ch);
					_channelEnabled[ch] = false;
				} else {
					setAbsolutePeriod(ch, (uint16)param);
				}
				break;
			}

			case 4: {
				// Set volume
				int sel = (param >> 8) & 0xF;
				int vol = param & 0xFF;
				setAbsoluteVolume(sel, (uint8)vol);
				break;
			}

			case 5: {
				// Play note: NNN selects a sample, 3 extra words follow.
				// FUN_26C2 does SUBQ #1, D0 (D0=NNN). If D0<0 (NNN=0) -> NO-OP.
				// NNN>0: triggers DMA playback of a sample buffer on AUD0.
				// Extra words: D2=start offset, D4=end trim, D3=repeat count.
				// DMA plays buffer (D3+1) times total (SUBQ #1 + BPL counting).
				if (_cmdPos + 3 <= (int)_commands.size()) {
					uint16 startOffset = _commands[_cmdPos++]; // D2
					uint16 endTrim = _commands[_cmdPos++];     // D4
					uint16 dmaCount = _commands[_cmdPos++];    // D3
					if (param > 0 && dmaCount > 0 && _periodShadow[0] > 0) {
						int bufSize = 256;
						if (_dmaSamples && param < (int)_dmaSamples->size()) {
							const AmigaDmaSample &sample = (*_dmaSamples)[param];
							if (!sample.data.empty()) {
								int start = MIN<int>(startOffset, sample.data.size());
								int trim = MIN<int>(endTrim, sample.data.size() - start);
									int playLen = sample.data.size() - start - trim;
									if (playLen > 1) {
										const int8 *src = sample.data.data() + start;
										// AUD0LC/AUD0LEN are reloaded on each DMA completion,
										// so the selected segment repeats in full.
										setChannelData(0, src, src, playLen, playLen);
										bufSize = playLen;
									}
								}
							}

						double durationSec = (dmaCount + 1) * bufSize * _periodShadow[0] / Audio::Paula::kPalPaulaClock;
						_dmaCounter = (int)(durationSec * 50.0) + 1;
						_dmaAud0Active = true;
						enableChannel(0);
						_channelEnabled[0] = true;
					}
				}
				break;
			}

			case 6:
				// Delay
				_delay = param;
				return;

			case 7:
				if (param == 0x000) {
					// Full stop: silence all channels
					for (int ch = 0; ch < 4; ch++) {
						_volumeShadow[ch] = 0;
						_channelEnabled[ch] = false;
						setChannelVolume(ch, 0);
						disableChannel(ch);
					}
					setChannelSampleStart(0, _squareWave);
					setChannelSampleLen(0, 0x20);
					setChannelOffset(0, Audio::Paula::Offset(0));
					_dmaAud0Active = false;
					_delay = -1;
					stopPaula();
					return;
				} else if (param == 0x001) {
					// Pause: wait for DMA completion
					_paused = true;
					return;
				} else if (param == 0x002) {
					// Loop: decrement counter, jump back if > 0
					_loopCounter--;
					if (_loopCounter > 0)
						_cmdPos = _loopPos;
					break;
				}
				break;

			case 8: case 9: case 0xA: case 0xB: {
				// Relative period (pitch bend)
				int ch = periodCmdToChannel(nibble - 8);
				// Sign-extend 12-bit parameter
				int16 delta = (int16)(param << 4) >> 4;
				setRelativePeriod(ch, delta);
				break;
			}

			case 0xC: {
				// Relative volume
				int sel = (param >> 8) & 0xF;
				int8 delta = (int8)(param & 0xFF);
				addRelativeVolume(sel, delta);
				break;
			}

			case 0xD:
				// Save loop position and set counter
				_loopPos = _cmdPos;
				_loopCounter = param;
				break;

			case 0xF:
				// End: stop interpreter but let channels keep playing.
				// On real Amiga hardware, audio DMA channels loop their
				// waveform buffer continuously until the next FUN_2652 call
				// silences them. playSoundAmiga() calls stopHandle() before
				// playing a new sound, matching this behavior.
				// Grace period of 25 ticks (500ms) approximates the typical
				// inter-sound gap during gameplay.
				_delay = -1;
				_graceCounter = 25;
				return;

			default:
				break;
			}
		}

		// Ran out of commands
		_delay = -1;
		_dmaAud0Active = false;
		setChannelSampleStart(0, _squareWave);
		setChannelSampleLen(0, 0x20);
		setChannelOffset(0, Audio::Paula::Offset(0));
		stopPaula();
	}
};

void FreescapeEngine::loadSoundsAmigaDemo(Common::SeekableReadStream *file, int offset, int numSounds) {
	file->seek(offset);
	_amigaSfxTable.clear();
	for (int i = 0; i < numSounds; i++) {
		AmigaSfxEntry entry;
		uint16 header = file->readUint16BE();
		entry.priority = header >> 8;
		int numWords = header & 0xFF;
		entry.commands.resize(numWords);
		for (int j = 0; j < numWords; j++)
			entry.commands[j] = file->readUint16BE();
		_amigaSfxTable.push_back(entry);
		debugC(1, kFreescapeDebugParser, "Amiga SFX %d: priority=%d, commands=%d", i, entry.priority, numWords);
	}
	debugC(1, kFreescapeDebugParser, "Loaded %d Amiga sound effects", numSounds);

	// Prepare DMA sample set for 0x5 commands from the embedded ProTracker module.
	// Parameter N uses index N (1-based), so keep index 0 empty.
	_amigaDmaSamples.clear();
	_amigaDmaSamples.resize(12);

	static const int kModOffset = 0x3D5A6;
	if (file->size() > kModOffset + 1084) {
		int modSize = file->size() - kModOffset;
		Common::Array<byte> modBytes;
		modBytes.resize(modSize);
		file->seek(kModOffset);
		file->read(modBytes.data(), modSize);

		Common::MemoryReadStream modStream(modBytes.data(), modBytes.size());
		Modules::Module module;
		if (module.load(modStream, 0)) {
			for (int i = 1; i <= 10; i++) {
				const Modules::sample_t &sample = module.sample[i - 1];
				if (sample.len > 0 && sample.data) {
					_amigaDmaSamples[i].data.resize(sample.len);
					memcpy(_amigaDmaSamples[i].data.data(), sample.data, sample.len);
				}
			}
		}
	}
}

void FreescapeEngine::playSoundAmiga(int index, Audio::SoundHandle &handle) {
	if (index < 0 || index >= (int)_amigaSfxTable.size()) {
		debugC(1, kFreescapeDebugMedia, "Amiga sound %d out of range (have %d)", index, (int)_amigaSfxTable.size());
		return;
	}

	const AmigaSfxEntry &entry = _amigaSfxTable[index];
	if (entry.commands.empty()) {
		debugC(1, kFreescapeDebugMedia, "Amiga sound %d has no commands", index);
		return;
	}

	debugC(1, kFreescapeDebugMedia, "Playing Amiga sound %d (priority=%d, commands=%d)",
		index, entry.priority, (int)entry.commands.size());

	AmigaSfxStream *stream = new AmigaSfxStream(entry.commands.data(), entry.commands.size(), &_amigaDmaSamples);
	_mixer->stopHandle(handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
}

} // namespace Freescape
