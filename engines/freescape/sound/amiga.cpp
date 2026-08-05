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

#include "common/file.h"
#include "common/memstream.h"
#include "common/mutex.h"
#include "common/ptr.h"

#include "freescape/freescape.h"

namespace Freescape {

struct AmigaSfxEntry {
	byte priority;
	Common::Array<uint16> commands;
};

struct AmigaDmaSample {
	Common::Array<int8> data;
};

// Priority of the sound currently playing (DAT_25E6). Shared with the stream,
// which releases it from the mixer thread.
struct AmigaSfxPriority {
	AmigaSfxPriority() : value(0) {}
	int value;
};

/**
 * Amiga (and Atari ST) Castle Master sound engine: a 50Hz command interpreter
 * driving the four Paula channels.
 *
 * All channels start out playing a shared 64-byte square wave (+64/-64 bytes),
 * so a bare period command gives a tone at 3546895 / (period * 2) Hz. AUD0
 * doubles as the sample channel: 0x5NNN points it at a PCM sample of the
 * external `cmsnds2` bank and the audio interrupt counts buffer repeats.
 *
 * Commands are 16-bit big-endian words: type in bits 15-12, parameter below.
 *
 *   0x0xxx-0x3xxx  period of AUD1/AUD2/AUD3/AUD0 = xxx, and enable it
 *                  (xxx = 0 only disables the channel)
 *   0x8xxx-0xBxxx  same channels, period += sign_extend_12(xxx)
 *   0x4Yxx         volume = xx (Y=1: AUD1, Y=2: AUD2, else AUD0+AUD3)
 *   0xCYxx         volume += sign_extend_8(xx), same channel mapping
 *   0x5NNN         play sample NNN on AUD0, consuming three more words:
 *                  start offset, end trim and repeat count (NNN = 0: no-op)
 *   0x6xxx         wait xxx ticks
 *   0x7000         stop everything, 0x7001 wait for the sample to end,
 *                  0x7002 loop back while the counter lasts
 *   0xDxxx         mark loop position, loop counter = xxx
 *   0xFxxx         end, leaving the channels running
 */
class AmigaSfxStream : public Audio::Paula {
public:
	AmigaSfxStream(const uint16 *commands, int numCommands, const Common::Array<AmigaDmaSample> *dmaSamples,
				   const Common::SharedPtr<AmigaSfxPriority> &priority, int rate = 44100)
		: Audio::Paula(false, rate, rate / 50),
		  _dmaSamples(dmaSamples), _priority(priority),
		  _cmdPos(0), _delay(0), _paused(false),
		  _dmaActive(false), _dmaDone(true), _dmaStopPending(false), _dmaRepeats(0),
		  _loopPos(0), _loopCounter(0), _graceCounter(0) {

		_commands.resize(numCommands);
		for (int i = 0; i < numCommands; i++)
			_commands[i] = commands[i];

		for (int i = 0; i < 64; i += 2) {
			_squareWave[i] = 64;
			_squareWave[i + 1] = -64;
		}

		// Init periods, slightly detuned per channel
		static const uint16 initPeriods[4] = { 0x1A1, 0x1AB, 0x1B5, 0x1BF };
		for (int ch = 0; ch < 4; ch++) {
			_periodShadow[ch] = initPeriods[ch];
			_volumeShadow[ch] = 0;
			_channelEnabled[ch] = false;

			setChannelSampleStart(ch, _squareWave);
			setChannelSampleLen(ch, 0x20); // 32 words = 64 bytes
			setChannelPeriod(ch, _periodShadow[ch]);
			setChannelVolume(ch, 0);
			// Configured but silent until a command enables the channel
			disableChannel(ch);
		}
		// Count AUD0 buffer repeats, as the audio interrupt does
		setChannelInterrupt(0, true);
		startPaula();
	}

private:
	// Tail given to a sound that ends with its channels still running: the
	// original would keep them looping until the next sound is triggered.
	static const int kEndGraceTicks = 25;

	Common::Array<uint16> _commands;
	const Common::Array<AmigaDmaSample> *_dmaSamples;
	Common::SharedPtr<AmigaSfxPriority> _priority;
	int _cmdPos;
	int _delay;            // -1 = interpreter stopped, 0 = execute next, >0 = waiting
	bool _paused;          // Waiting for sample playback to complete (0x7001)
	bool _dmaActive;       // AUD0 is playing a sample
	bool _dmaDone;         // Sample playback finished
	bool _dmaStopPending;  // Repeat counter expired inside the mixing loop
	int _dmaRepeats;       // Times the sample buffer must be played
	int _loopPos;
	int _loopCounter;
	int _graceCounter;     // Ticks left before winding down after END
	uint16 _periodShadow[4];
	int _volumeShadow[4];
	bool _channelEnabled[4];
	int8 _squareWave[64];

	// Period command 0 -> AUD1, 1 -> AUD2, 2 -> AUD3, 3 -> AUD0
	static int periodCmdToChannel(int nibble) {
		return (nibble + 1) & 3;
	}

	// AUDxVOL only implements bits 6-0 and caps at 64, so a fade running past
	// zero wraps around to full volume instead of going silent.
	static byte hardwareVolume(int value) {
		int vol = value & 0x7F;
		return (byte)MIN(vol, 64);
	}

	void releasePriority() {
		if (_priority)
			_priority->value = 0;
	}

	void setAbsolutePeriod(int ch, uint16 period) {
		_periodShadow[ch] = period;
		setChannelPeriod(ch, period);
		enableChannelDma(ch);
	}

	void enableChannelDma(int ch) {
		// Setting the DMACON bit of a running channel is a no-op, so only
		// restart the buffer when the channel was actually off.
		if (_channelEnabled[ch])
			return;
		enableChannel(ch);
		_channelEnabled[ch] = true;
	}

	void disableChannelDma(int ch) {
		disableChannel(ch);
		_channelEnabled[ch] = false;
	}

	void setRelativePeriod(int ch, int16 delta) {
		uint16 newPeriod = (uint16)(_periodShadow[ch] + delta);
		if (newPeriod == 0) {
			// The shadow keeps its previous value here
			disableChannelDma(ch);
			return;
		}
		_periodShadow[ch] = newPeriod;
		setChannelPeriod(ch, newPeriod);
		enableChannelDma(ch);
	}

	void setAbsoluteVolume(int sel, int vol) {
		if (sel == 1) {
			_volumeShadow[1] = vol;
			setChannelVolume(1, hardwareVolume(_volumeShadow[1]));
		} else if (sel == 2) {
			_volumeShadow[2] = vol;
			setChannelVolume(2, hardwareVolume(_volumeShadow[2]));
		} else {
			_volumeShadow[0] = vol;
			_volumeShadow[3] = vol;
			setChannelVolume(0, hardwareVolume(_volumeShadow[0]));
			setChannelVolume(3, hardwareVolume(_volumeShadow[3]));
		}
	}

	void addRelativeVolume(int sel, int8 delta) {
		if (sel == 1) {
			_volumeShadow[1] += delta;
			setChannelVolume(1, hardwareVolume(_volumeShadow[1]));
		} else if (sel == 2) {
			_volumeShadow[2] += delta;
			setChannelVolume(2, hardwareVolume(_volumeShadow[2]));
		} else {
			_volumeShadow[0] += delta;
			_volumeShadow[3] += delta;
			setChannelVolume(0, hardwareVolume(_volumeShadow[0]));
			setChannelVolume(3, hardwareVolume(_volumeShadow[3]));
		}
	}

	// Start sample playback on AUD0. Paula interrupts once when DMA is switched
	// on plus once per completed buffer, so the sample is heard exactly
	// `repeats` times.
	void triggerSample(int sampleNum, uint16 startOffset, uint16 endTrim, uint16 repeats) {
		if (sampleNum <= 0 || repeats == 0)
			return;

		if (!_dmaSamples || sampleNum >= (int)_dmaSamples->size())
			return;

		const AmigaDmaSample &sample = (*_dmaSamples)[sampleNum];
		if (sample.data.empty())
			return;

		int size = sample.data.size();
		int start = MIN<int>(startOffset, size);
		int trim = MIN<int>(endTrim, size - start);
		int playLen = (size - start - trim) & ~1; // AUD0LEN counts words
		if (playLen <= 1)
			return;

		// The selected segment is reloaded on each buffer completion
		const int8 *src = sample.data.data() + start;
		setChannelData(0, src, src, playLen, playLen);
		setChannelDmaCount(0, 0);
		_dmaRepeats = repeats;
		_dmaActive = true;
		_dmaDone = false;
		_dmaStopPending = false;
		_channelEnabled[0] = true;
	}

	// AUD0 buffer wrap. Paula calls this from the middle of its mixing loop, so
	// the channel can only be muted here: disabling it would leave the mixer
	// dereferencing a null sample pointer for the rest of the buffer.
	void interruptChannel(byte channel) override {
		if (channel != 0 || !_dmaActive)
			return;
		if (getChannelDmaCount(0) < _dmaRepeats)
			return;
		setChannelVolume(0, 0);
		_dmaStopPending = true;
	}

	void finishDma() {
		disableChannelDma(0);
		setChannelVolume(0, hardwareVolume(_volumeShadow[0])); // Undo the mute
		_dmaActive = false;
		_dmaStopPending = false;
		_dmaDone = true;
		releasePriority();
	}

	// Interpreter tick, once per emulated VBI
	void interrupt() override {
		if (_dmaActive && (_dmaStopPending || getChannelDmaCount(0) >= _dmaRepeats))
			finishDma();

		// The original re-enters the whole routine after every command, so a
		// delay of N is decremented once on the tick that sets it: the next
		// command runs exactly N ticks later.
		for (;;) {
			if (_paused) {
				if (!_dmaDone)
					return;
				_paused = false;
			} else if (_delay < 0) {
				// Stopped, but the channels keep running: let the sample finish
				if (_dmaActive)
					return;
				if (_graceCounter < 0)
					_graceCounter = channelsAudible() ? kEndGraceTicks : 0;
				if (_graceCounter > 0) {
					_graceCounter--;
					return;
				}
				stopPaula();
				return;
			} else if (_delay > 0) {
				_delay--;
				return;
			}

			if (!executeCommand())
				return;
		}
	}

	// Runs one command word, returning false when the tick is over
	bool executeCommand() {
		if (_cmdPos >= (int)_commands.size()) {
			endInterpreter();
			return false;
		}

		uint16 cmd = _commands[_cmdPos++];
		int nibble = (cmd >> 12) & 0xF;
		int param = cmd & 0xFFF;

		switch (nibble) {
		case 0:
		case 1:
		case 2:
		case 3: {
			int ch = periodCmdToChannel(nibble);
			if (param == 0)
				disableChannelDma(ch);
			else
				setAbsolutePeriod(ch, (uint16)param);
			break;
		}

		case 4:
			setAbsoluteVolume((param >> 8) & 0xF, param & 0xFF);
			break;

		case 5: {
			if (_cmdPos + 3 > (int)_commands.size()) {
				endInterpreter();
				return false;
			}
			uint16 startOffset = _commands[_cmdPos++];
			uint16 endTrim = _commands[_cmdPos++];
			uint16 repeats = _commands[_cmdPos++];
			triggerSample(param, startOffset, endTrim, repeats);
			break;
		}

		case 6:
			_delay = param;
			break;

		case 7:
			if (param == 0x000) {
				for (int ch = 0; ch < 4; ch++) {
					_volumeShadow[ch] = 0;
					setChannelVolume(ch, 0);
					disableChannelDma(ch);
				}
				_dmaActive = false;
				_dmaStopPending = false;
				_dmaDone = true;
				_delay = -1;
				releasePriority();
				stopPaula();
				return false;
			} else if (param == 0x001) {
				_paused = true;
			} else if (param == 0x002) {
				_loopCounter--;
				if (_loopCounter > 0)
					_cmdPos = _loopPos;
			}
			break;

		case 8:
		case 9:
		case 0xA:
		case 0xB: {
			int ch = periodCmdToChannel(nibble - 8);
			int delta = (param & 0x800) ? param - 0x1000 : param; // Sign-extended
			setRelativePeriod(ch, (int16)delta);
			break;
		}

		case 0xC:
			addRelativeVolume((param >> 8) & 0xF, (int8)(param & 0xFF));
			break;

		case 0xD:
			_loopPos = _cmdPos;
			_loopCounter = param;
			break;

		case 0xF:
			endInterpreter();
			return false;

		default:
			// Unknown commands (0xE) are skipped by the original as well
			break;
		}

		return true;
	}

	void endInterpreter() {
		_delay = -1;
		_graceCounter = -1; // Decided once the sample playback is over
	}

	bool channelsAudible() const {
		for (int ch = 0; ch < 4; ch++) {
			if (_channelEnabled[ch] && hardwareVolume(_volumeShadow[ch]) > 0)
				return true;
		}
		return false;
	}
};

class SoundAmigaDemo final : public Sound {
public:
	SoundAmigaDemo(Audio::Mixer *mixer) : _priority(new AmigaSfxPriority()), _mixer(mixer) {}

	void loadSounds(Common::SeekableReadStream *file, int offset, int numSounds, const Common::Path &sampleBank, int modOffset);

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
		return index >= 0 && index < (int)_amigaSfxTable.size();
	}

private:
	void loadDmaSamples(Common::SeekableReadStream *file, const Common::Path &sampleBank, int modOffset);
	void dropSoundsWithoutSamples();

	Common::Array<AmigaSfxEntry> _amigaSfxTable;
	Common::Array<AmigaDmaSample> _amigaDmaSamples;
	Common::SharedPtr<AmigaSfxPriority> _priority;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _soundFxHandle;
};

void SoundAmigaDemo::loadSounds(Common::SeekableReadStream *file, int offset, int numSounds, const Common::Path &sampleBank, int modOffset) {
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

	loadDmaSamples(file, sampleBank, modOffset);
	dropSoundsWithoutSamples();
}

// A missing sample leaves AUD0 on the shared square wave, so the volume command
// that follows turns a sampled effect into a bare tone. The demo disc ships no
// bank, so drop those entries instead of beeping.
void SoundAmigaDemo::dropSoundsWithoutSamples() {
	int dropped = 0;
	for (uint i = 0; i < _amigaSfxTable.size(); i++) {
		AmigaSfxEntry &entry = _amigaSfxTable[i];
		bool missing = false;

		for (uint j = 0; j < entry.commands.size(); j++) {
			uint16 command = entry.commands[j];
			if ((command >> 12) != 5)
				continue;

			// 0x5NNN carries three parameter words of its own
			uint16 slot = command & 0x0fff;
			j += 3;
			if (slot == 0 || slot >= _amigaDmaSamples.size() || _amigaDmaSamples[slot].data.empty()) {
				missing = true;
				break;
			}
		}

		if (missing) {
			entry.commands.clear();
			dropped++;
			debugC(1, kFreescapeDebugParser, "Amiga SFX %d needs a sample that is not present, dropped", i);
		}
	}

	if (dropped > 0)
		debugC(1, kFreescapeDebugParser, "Dropped %d Amiga sound effects with no sample data", dropped);
}

// The samples played by 0x5NNN come from an external bank, which the original
// loads over the memory holding the ProTracker module: that is why music and
// sound effects are mutually exclusive there, and why the rolling demo, whose
// disk carries no bank at all, is music only. Each of the 10 entries is a
// 4-byte big endian length, a 2-byte sample rate and that many signed 8-bit
// samples.
void SoundAmigaDemo::loadDmaSamples(Common::SeekableReadStream *file, const Common::Path &sampleBank, int modOffset) {
	// Parameter N uses index N, so index 0 stays empty; 11 is the extra slot
	_amigaDmaSamples.clear();
	_amigaDmaSamples.resize(12);

	Common::File bank;
	if (!sampleBank.empty() && bank.open(sampleBank)) {
		int index = 1;
		while (index <= 10 && bank.pos() + 6 <= bank.size()) {
			uint32 length = bank.readUint32BE();
			bank.readUint16BE(); // Nominal rate, unused: 0x3xxx sets the period
			if (length == 0 || bank.pos() + (int64)length > bank.size())
				break;

			_amigaDmaSamples[index].data.resize(length);
			bank.read(_amigaDmaSamples[index].data.data(), length);
			debugC(1, kFreescapeDebugParser, "Amiga DMA sample %d: %d bytes", index, length);
			index++;
		}
		bank.close();
	} else if (!sampleBank.empty()) {
		warning("Freescape: '%s' is missing from the game data, so the sampled part "
				"of the Amiga sound effects will not play", sampleBank.toString().c_str());
	}

	if (modOffset < 0)
		return;

	// The demo points its extra sample slot at the third instrument of the
	// music module; the full game leaves that pointer uninitialized
	int64 fileSize = file->size();
	if (fileSize <= modOffset + 1084)
		return;

	int modSize = fileSize - modOffset;
	Common::Array<byte> modBytes;
	modBytes.resize(modSize);
	file->seek(modOffset);
	file->read(modBytes.data(), modSize);

	Common::MemoryReadStream modStream(modBytes.data(), modBytes.size());
	Modules::Module module;
	if (!module.load(modStream, 0))
		return;

	const Modules::sample_t &sample = module.sample[2];
	if (sample.len > 0 && sample.data) {
		_amigaDmaSamples[11].data.resize(sample.len);
		memcpy(_amigaDmaSamples[11].data.data(), sample.data, sample.len);
	}
}

void SoundAmigaDemo::playSound(int index, Type type) {
	if (index < 0 || index >= (int)_amigaSfxTable.size()) {
		debugC(1, kFreescapeDebugMedia, "Amiga sound %d out of range (have %d)", index, (int)_amigaSfxTable.size());
		return;
	}

	const AmigaSfxEntry &entry = _amigaSfxTable[index];
	if (entry.commands.empty()) {
		debugC(1, kFreescapeDebugMedia, "Amiga sound %d has no commands", index);
		return;
	}

	// A sound still holding the priority slot cannot be replaced by a lesser one
	if (_mixer->isSoundHandleActive(_soundFxHandle)) {
		Common::StackLock lock(_mixer->mutex());
		if (entry.priority < _priority->value) {
			debugC(1, kFreescapeDebugMedia, "Amiga sound %d skipped (priority %d < %d)",
				index, entry.priority, _priority->value);
			return;
		}
	}

	debugC(1, kFreescapeDebugMedia, "Playing Amiga sound %d (priority=%d, commands=%d)",
		index, entry.priority, (int)entry.commands.size());

	AmigaSfxStream *stream = new AmigaSfxStream(entry.commands.data(), entry.commands.size(), &_amigaDmaSamples, _priority);
	// Claim the slot only once the previous stream is gone, so that its clean
	// up cannot release the new claim
	_mixer->stopHandle(_soundFxHandle);
	{
		Common::StackLock lock(_mixer->mutex());
		_priority->value = entry.priority;
	}
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream, -1,
		Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
}

Sound *FreescapeEngine::loadSoundsAmiga(Common::SeekableReadStream *file, int offset, int numSounds, const Common::Path &sampleBank, int modOffset) {
	SoundAmigaDemo *sound = new SoundAmigaDemo(_mixer);
	sound->loadSounds(file, offset, numSounds, sampleBank, modOffset);
	return sound;
}

} // namespace Freescape
