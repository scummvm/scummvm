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
#include "audio/softsynth/ay8912.h"

#include "common/mutex.h"
#include "common/ptr.h"

#include "freescape/freescape.h"

namespace Freescape {

struct AtariSfxEntry {
	byte priority;
	Common::Array<uint16> commands;
};

struct AtariPcmSample {
	Common::Array<int8> data;
};

struct AtariSfxPriority {
	AtariSfxPriority() : value(0) {}
	int value;
};

// YM2149 clock and MFP timer clock of a standard ST
const int kAtariYmClock = 2000000;
const int kAtariMfpClock = 2457600;
// Timer A runs with prescaler 3, and the driver refuses to go below 0x11
const int kAtariTimerPrescale = 16;
const int kAtariMinTimerData = 0x11;
// Register 7: tone A/B/C on (bits 0-2 clear), noise off everywhere
const byte kAtariMixerBase = 0xF8;

/**
 * Castle Master Atari ST sound engine: the same 50Hz command interpreter and the
 * same sound table as the Amiga release, over completely different hardware.
 *
 * The three YM2149 channels carry the tones. Sampled effects are played by an
 * MFP Timer A interrupt (prog $29fe) that walks the PCM byte by byte and writes
 * all three volume registers from a table at $2aee, which is how an 8-bit sample
 * comes out of three 4-bit DACs. That table is a linearisation of the signed
 * sample, so the samples are mixed here as the plain signed PCM they are.
 *
 * Commands are 16-bit big-endian words: type in bits 15-12, parameter below.
 *
 *   0x0xxx/0x1xxx/0x2xxx  tone period of channel A/B/C (0 silences it)
 *   0x8xxx/0x9xxx/0xAxxx  same channels, period += sign_extend_12(xxx)
 *   0x3xxx                sample rate: Timer A data = max(xxx & 0xff, 0x11),
 *                         so 2457600 / (16 * data), at most 9035 Hz
 *   0x4Yxx                volume = (xx & 0x3f) >> 2 (Y=1: A, Y=2: B, else C)
 *   0xCYxx                volume += sign_extend_8(xx), same channel mapping
 *   0x5NNN                play sample NNN, consuming three more words: start
 *                         offset, end trim (ignored here, as in the original)
 *                         and repeat count
 *   0x6xxx                wait xxx ticks
 *   0x7000                stop everything, 0x7001 wait for the sample to end,
 *                         0x7002 loop back while the counter lasts
 *   0xBxxx                noise period += sign_extend_12(xxx), and the sample
 *                         rate moves by the same amount
 *   0xDxxx                mark loop position, loop counter = xxx
 *   0xExxx                xx < 0 silences the noise, otherwise noise period =
 *                         xxx & 0x1f and noise is mixed into channel A
 *   0xFxxx                end, leaving the channels running
 */
class AtariSfxStream : public Audio::AudioStream {
public:
	AtariSfxStream(const uint16 *commands, int numCommands, const Common::Array<AtariPcmSample> *samples,
				   const Common::SharedPtr<AtariSfxPriority> &priority, int rate = 44100)
		: _samples(samples), _priority(priority), _rate(rate),
		  _cmdPos(0), _delay(0), _finished(false), _waitForSample(false),
		  _tickSamples(0), _loopPos(0), _loopCounter(0), _graceTicks(kEndGraceTicks),
		  _pcmData(nullptr), _pcmLength(0), _pcmPos(0), _pcmFrac(0), _pcmStep(0),
		  _pcmRepeats(0), _pcmActive(false), _mixer(kAtariMixerBase), _noisePeriod(0) {

		_commands.resize(numCommands);
		for (int i = 0; i < numCommands; i++)
			_commands[i] = commands[i];

		for (int i = 0; i < 3; i++) {
			_tonePeriod[i] = 0;
			_volume[i] = 0;
		}

		_ym = new Audio::AY8912Stream(rate, kAtariYmClock);
		_ym->setReg(7, _mixer);
		for (int i = 0; i < 3; i++)
			_ym->setReg(8 + i, 0);

		setTimerData(kAtariMinTimerData);
	}

	~AtariSfxStream() override {
		delete _ym;
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		int done = 0;
		while (done < numSamples) {
			if (_tickSamples <= 0) {
				runTick();
				_tickSamples = _rate / 50;
			}

			int chunk = MIN<int>(numSamples - done, _tickSamples);
			chunk &= ~1; // stereo pairs
			if (chunk <= 0)
				chunk = MIN<int>(numSamples - done, 2);

			_ym->readBuffer(buffer + done, chunk);
			mixSample(buffer + done, chunk);

			done += chunk;
			_tickSamples -= chunk;
		}
		return done;
	}

	bool isStereo() const override { return true; }
	int getRate() const override { return _rate; }
	bool endOfData() const override { return _finished && !_pcmActive && _graceTicks <= 0; }

private:
	// Ticks a finished program keeps its channels running for
	static const int kEndGraceTicks = 25;

	Common::Array<uint16> _commands;
	const Common::Array<AtariPcmSample> *_samples;
	Common::SharedPtr<AtariSfxPriority> _priority;
	Audio::AY8912Stream *_ym;
	int _rate;

	int _cmdPos;
	int _delay;
	bool _finished;
	bool _waitForSample;
	int _tickSamples;
	int _loopPos;
	int _loopCounter;
	int _graceTicks;

	const int8 *_pcmData;
	int _pcmLength;
	int _pcmPos;
	uint32 _pcmFrac;
	uint32 _pcmStep;
	int _pcmRepeats;
	bool _pcmActive;

	byte _mixer;
	uint16 _tonePeriod[3];
	int _volume[3];
	uint16 _noisePeriod;
	uint16 _timerData;

	void releasePriority() {
		if (_priority)
			_priority->value = 0;
	}

	// The driver clamps the data register at 0x11, which is what makes 9035 Hz
	// the fastest these effects can play
	void setTimerData(uint16 data) {
		_timerData = MAX<uint16>(data & 0xff, kAtariMinTimerData);
		int pcmRate = kAtariMfpClock / (kAtariTimerPrescale * _timerData);
		_pcmStep = (uint32)((((uint64)pcmRate) << 16) / _rate);
	}

	void setTonePeriod(int channel, uint16 period) {
		_tonePeriod[channel] = period;
		_ym->setReg(2 * channel, period & 0xff);
		_ym->setReg(2 * channel + 1, (period >> 8) & 0x0f);
	}

	void setVolume(int channel, int value) {
		_volume[channel] = value;
		_ym->setReg(8 + channel, (value & 0x3f) >> 2);
	}

	// 0x4Yxx and 0xCYxx: Y=1 is channel A, Y=2 channel B, anything else C
	static int volumeSelectToChannel(uint16 param) {
		uint16 sel = param & 0x0f00;
		if (sel == 0x0100)
			return 0;
		if (sel == 0x0200)
			return 1;
		return 2;
	}

	static int16 signExtend12(uint16 value) {
		return (int16)((value & 0x0fff) << 4) >> 4;
	}

	void setNoiseEnabled(bool enabled) {
		if (enabled)
			_mixer &= ~0x08;
		else
			_mixer |= 0x08;
		_ym->setReg(7, _mixer);
	}

	void triggerSample(int sampleNum, uint16 startOffset, uint16 repeats) {
		_pcmActive = false;
		if (sampleNum <= 0 || !_samples || sampleNum >= (int)_samples->size())
			return;

		const AtariPcmSample &sample = (*_samples)[sampleNum];
		if (sample.data.empty())
			return;

		int start = (startOffset < sample.data.size()) ? startOffset : 0;
		_pcmData = sample.data.data() + start;
		_pcmLength = sample.data.size() - start;
		if (_pcmLength <= 0)
			return;

		_pcmPos = 0;
		_pcmFrac = 0;
		// The counter is pre-decremented, so a repeat word of N plays N times
		_pcmRepeats = (repeats > 0) ? repeats - 1 : 0;
		_pcmActive = true;
	}

	void silenceTones() {
		for (int i = 0; i < 3; i++) {
			setVolume(i, 0);
			setTonePeriod(i, 0);
		}
		_mixer = kAtariMixerBase;
		_ym->setReg(7, _mixer);
		_ym->setReg(6, 0);
	}

	void stopEverything() {
		silenceTones();
		_pcmActive = false;
	}

	void endInterpreter() {
		_finished = true;
		_delay = -1;
		releasePriority();
	}

	void runTick() {
		// 0xFxxx leaves the channels running, so wind the tones down after a
		// grace; a sample started just before it keeps its repeat count.
		if (_finished) {
			if (_graceTicks > 0 && --_graceTicks == 0)
				silenceTones();
			return;
		}

		if (_waitForSample) {
			if (_pcmActive)
				return;
			_waitForSample = false;
		}

		if (_delay > 0) {
			_delay--;
			return;
		}

		while (executeCommand())
			;
	}

	// One command per call; returns true while the interpreter should keep
	// running commands within this tick
	bool executeCommand() {
		if (_cmdPos >= (int)_commands.size()) {
			endInterpreter();
			return false;
		}

		uint16 command = _commands[_cmdPos++];
		uint16 type = command & 0xf000;
		uint16 param = command & 0x0fff;

		switch (type >> 12) {
		case 0:
		case 1:
		case 2:
			setTonePeriod(type >> 12, param);
			break;

		case 8:
		case 9:
		case 0xa: {
			int channel = (type >> 12) - 8;
			setTonePeriod(channel, (uint16)(_tonePeriod[channel] + signExtend12(param)));
			break;
		}

		case 3:
			setTimerData(param);
			break;

		case 4:
			setVolume(volumeSelectToChannel(param), param & 0x3f);
			break;

		case 0xc: {
			int channel = volumeSelectToChannel(param);
			setVolume(channel, _volume[channel] + (int8)(param & 0xff));
			break;
		}

		case 5: {
			if (_cmdPos + 3 > (int)_commands.size()) {
				endInterpreter();
				return false;
			}
			uint16 startOffset = _commands[_cmdPos++];
			_cmdPos++; // end trim, which the Atari driver never reads
			uint16 repeats = _commands[_cmdPos++];

			// The same volume registers carry the PCM, so everything else goes
			// quiet first
			setNoiseEnabled(false);
			for (int i = 0; i < 3; i++) {
				setTonePeriod(i, 0);
				setVolume(i, 0);
			}
			_ym->setReg(6, 0);
			triggerSample(param, startOffset, repeats);
			break;
		}

		case 6:
			_delay = param;
			return false;

		case 7:
			if (param == 0x000) {
				stopEverything();
				endInterpreter();
				return false;
			} else if (param == 0x001) {
				_waitForSample = true;
				return false;
			} else if (param == 0x002) {
				if (--_loopCounter > 0)
					_cmdPos = _loopPos;
			}
			break;

		case 0xb: {
			int16 delta = signExtend12(param);
			_noisePeriod = (uint16)(_noisePeriod + delta);
			_ym->setReg(6, _noisePeriod & 0x1f);
			setTimerData((uint16)(_timerData + delta));
			break;
		}

		case 0xd:
			_loopCounter = param;
			_loopPos = _cmdPos;
			break;

		case 0xe:
			if ((param & 0x80) != 0) {
				setNoiseEnabled(false);
			} else {
				_noisePeriod = param & 0x1f;
				_ym->setReg(6, _noisePeriod);
				setNoiseEnabled(true);
			}
			break;

		case 0xf:
			endInterpreter();
			return false;

		default:
			break;
		}

		return true;
	}

	void mixSample(int16 *buffer, int numSamples) {
		if (!_pcmActive)
			return;

		for (int i = 0; i < numSamples; i += 2) {
			if (_pcmPos >= _pcmLength) {
				if (_pcmRepeats <= 0) {
					_pcmActive = false;
					return;
				}
				_pcmRepeats--;
				_pcmPos = 0;
				_pcmFrac = 0;
			}

			int mixed = buffer[i] + (int)_pcmData[_pcmPos] * 128;
			int16 value = (int16)CLIP<int>(mixed, -32768, 32767);
			buffer[i] = value;
			buffer[i + 1] = value;

			_pcmFrac += _pcmStep;
			_pcmPos += _pcmFrac >> 16;
			_pcmFrac &= 0xffff;
		}
	}
};

class SoundAtariCastle final : public Sound {
public:
	SoundAtariCastle(Audio::Mixer *mixer) : _mixer(mixer), _priority(new AtariSfxPriority()) {}

	~SoundAtariCastle() override {
		_mixer->stopHandle(_soundFxHandle);
	}

	void loadSounds(Common::SeekableReadStream *file, int offset, int numSounds, int bankOffset);

	void playSound(int index, Type type) override;

	void stopSound(Type type) override {
		_mixer->stopHandle(_soundFxHandle);
		Common::StackLock lock(_mixer->mutex());
		_priority->value = 0;
	}

	bool isPlayingSound(Type type) const override {
		return _mixer->isSoundHandleActive(_soundFxHandle);
	}

	bool isSoundAvailable(int index) const override {
		return index >= 0 && index < (int)_sfxTable.size();
	}

private:
	void loadPcmSamples(Common::SeekableReadStream *file, int bankOffset);

	Common::Array<AtariSfxEntry> _sfxTable;
	Common::Array<AtariPcmSample> _pcmSamples;
	Common::SharedPtr<AtariSfxPriority> _priority;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundFxHandle;
};

void SoundAtariCastle::loadSounds(Common::SeekableReadStream *file, int offset, int numSounds, int bankOffset) {
	file->seek(offset);
	_sfxTable.clear();
	for (int i = 0; i < numSounds; i++) {
		AtariSfxEntry entry;
		uint16 header = file->readUint16BE();
		entry.priority = header >> 8;
		int numWords = header & 0xFF;
		entry.commands.resize(numWords);
		for (int j = 0; j < numWords; j++)
			entry.commands[j] = file->readUint16BE();
		_sfxTable.push_back(entry);
		debugC(1, kFreescapeDebugParser, "Atari SFX %d: priority=%d, commands=%d", i, entry.priority, numWords);
	}

	loadPcmSamples(file, bankOffset);
}

// The bank the Amiga ships as the external `cmsnds2`, embedded byte for byte:
// ten entries of a 4-byte big endian length, a 2-byte nominal rate and that many
// signed 8-bit samples, each ending in the zero byte the handler stops on.
void SoundAtariCastle::loadPcmSamples(Common::SeekableReadStream *file, int bankOffset) {
	// Parameter N uses index N, so index 0 stays empty
	_pcmSamples.clear();
	_pcmSamples.resize(11);

	if (bankOffset <= 0)
		return;

	file->seek(bankOffset);
	for (int index = 1; index <= 10; index++) {
		if (file->pos() + 6 > file->size())
			break;

		uint32 length = file->readUint32BE();
		file->readUint16BE(); // Nominal rate, unused: 0x3xxx sets the timer
		if (length == 0 || file->pos() + (int64)length > file->size())
			break;

		_pcmSamples[index].data.resize(length);
		file->read(_pcmSamples[index].data.data(), length);
		debugC(1, kFreescapeDebugParser, "Atari PCM sample %d: %d bytes", index, length);
	}
}

void SoundAtariCastle::playSound(int index, Type type) {
	if (index < 0 || index >= (int)_sfxTable.size()) {
		debugC(1, kFreescapeDebugMedia, "Atari sound %d out of range (have %d)", index, (int)_sfxTable.size());
		return;
	}

	const AtariSfxEntry &entry = _sfxTable[index];
	if (entry.commands.empty()) {
		debugC(1, kFreescapeDebugMedia, "Atari sound %d has no commands", index);
		return;
	}

	// A sound still holding the priority slot cannot be replaced by a lesser one
	if (_mixer->isSoundHandleActive(_soundFxHandle)) {
		Common::StackLock lock(_mixer->mutex());
		if (entry.priority < _priority->value) {
			debugC(1, kFreescapeDebugMedia, "Atari sound %d skipped (priority %d < %d)",
				index, entry.priority, _priority->value);
			return;
		}
	}

	debugC(1, kFreescapeDebugMedia, "Playing Atari sound %d (priority=%d, commands=%d)",
		index, entry.priority, (int)entry.commands.size());

	AtariSfxStream *stream = new AtariSfxStream(entry.commands.data(), entry.commands.size(), &_pcmSamples, _priority);
	_mixer->stopHandle(_soundFxHandle);
	{
		Common::StackLock lock(_mixer->mutex());
		_priority->value = entry.priority;
	}
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
}

Sound *FreescapeEngine::loadSoundsAtariCastle(Common::SeekableReadStream *file, int offset, int numSounds, int bankOffset) {
	SoundAtariCastle *sound = new SoundAtariCastle(_mixer);
	sound->loadSounds(file, offset, numSounds, bankOffset);
	return sound;
}

} // namespace Freescape
