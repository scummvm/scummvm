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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "audio/audiostream.h"
#include "audio/decoders/xa.h"
#include "audio/mixer.h"

#include "common/config-manager.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/util.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/psx/psx.h"

#include <math.h>

namespace Scumm {

class RA2PSXADSRStream : public Audio::RewindableAudioStream {
public:
	RA2PSXADSRStream(Audio::RewindableAudioStream *stream, uint16 attack,
			uint16 decay, uint16 sustain)
			: _stream(stream), _position(0) {
		const uint32 rate = _stream->getRate();
		_attackSamples = ((uint64)attack * rate + 999) / 1000;
		_decaySamples = ((uint64)decay * rate + 999) / 1000;
		_sustain = MIN<uint32>(sustain, 0x1000) * 0x10000 / 0x1000;
	}

	~RA2PSXADSRStream() override { delete _stream; }

	int readBuffer(int16 *buffer, const int numSamples) override {
		const int count = _stream->readBuffer(buffer, numSamples);
		if (count <= 0)
			return count;

		for (int i = 0; i < count; ++i, ++_position) {
			uint32 gain;
			if (_attackSamples && _position < _attackSamples) {
				gain = (uint32)(_position * 0x10000 / _attackSamples);
			} else if (_decaySamples && _position < _attackSamples + _decaySamples) {
				const uint64 decayPosition = _position - _attackSamples;
				gain = 0x10000 - (uint32)(decayPosition * (0x10000 - _sustain) /
						_decaySamples);
			} else {
				gain = _sustain;
			}
			buffer[i] = (int16)((int64)buffer[i] * gain / 0x10000);
		}
		return count;
	}

	bool isStereo() const override { return _stream->isStereo(); }
	bool endOfData() const override { return _stream->endOfData(); }
	int getRate() const override { return _stream->getRate(); }

	bool rewind() override {
		_position = 0;
		return _stream->rewind();
	}

private:
	Audio::RewindableAudioStream *_stream;
	uint64 _position;
	uint64 _attackSamples;
	uint64 _decaySamples;
	uint32 _sustain;
};

// A 14 bit bend centred on kNeutralBend, scaled into semitones by the range
// macro command 0x33 sets.
int bendToStep(int bend, int rangeUp, int rangeDown) {
	const int offset = CLIP(bend, 0, 0x3fff) - RA2PSXSoundPlayer::kNeutralBend;
	if (!offset)
		return 0x1000;
	const int range = offset < 0 ? rangeDown : rangeUp;
	const double semitones = (double)offset * range / 8192.0;
	return CLIP((int)(0x1000 * pow(2.0, semitones / 12.0)), 1, 0x8000);
}

// The SPU bends in place; the mixer cannot, so the step lives in a block the
// voice keeps a reference to and the stream reads.
struct RA2PSXPitch {
	RA2PSXPitch(int initial) : pitch(initial) {}
	int pitch;
};

typedef Common::SharedPtr<RA2PSXPitch> RA2PSXPitchRef;

// A sample whose ADPCM blocks ask the voice to repeat plays until something stops it.
class RA2PSXLoopStream : public Audio::RewindableAudioStream {
public:
	explicit RA2PSXLoopStream(Audio::RewindableAudioStream *stream) : _stream(stream) {}
	~RA2PSXLoopStream() override { delete _stream; }

	int readBuffer(int16 *buffer, const int numSamples) override {
		int produced = 0;
		while (produced < numSamples) {
			const int read = _stream->readBuffer(buffer + produced, numSamples - produced);
			if (read > 0) {
				produced += read;
				continue;
			}
			// An empty pass after rewinding means there is nothing to loop over.
			if (!_stream->rewind() || _stream->readBuffer(buffer + produced, 1) <= 0)
				break;
			++produced;
		}
		return produced;
	}

	bool isStereo() const override { return _stream->isStereo(); }
	int getRate() const override { return _stream->getRate(); }
	bool endOfData() const override { return false; }
	bool rewind() override { return _stream->rewind(); }

private:
	Audio::RewindableAudioStream *_stream;
};

class RA2PSXPitchStream : public Audio::RewindableAudioStream {
public:
	RA2PSXPitchStream(Audio::RewindableAudioStream *stream, const RA2PSXPitchRef &pitch)
			: _stream(stream), _pitch(pitch), _channels(stream->isStereo() ? 2 : 1),
			_fraction(0x1000), _ended(false) {
		for (int i = 0; i < 2; ++i)
			_previous[i] = _current[i] = 0;
	}

	~RA2PSXPitchStream() override { delete _stream; }

	int readBuffer(int16 *buffer, const int numSamples) override {
		int produced = 0;
		while (produced + _channels <= numSamples) {
			while (_fraction >= 0x1000) {
				int16 frame[2] = { 0, 0 };
				if (_stream->readBuffer(frame, _channels) != _channels) {
					_ended = true;
					return produced;
				}
				for (int i = 0; i < _channels; ++i) {
					_previous[i] = _current[i];
					_current[i] = frame[i];
				}
				_fraction -= 0x1000;
			}
			for (int i = 0; i < _channels; ++i)
				buffer[produced++] = (int16)((_previous[i] * (0x1000 - _fraction) +
						_current[i] * _fraction) >> 12);
			_fraction += CLIP(_pitch->pitch, 1, 0x3fff);
		}
		return produced;
	}

	bool isStereo() const override { return _channels == 2; }
	bool endOfData() const override { return _ended || _stream->endOfData(); }
	int getRate() const override { return _stream->getRate(); }

	bool rewind() override {
		_fraction = 0x1000;
		_ended = false;
		for (int i = 0; i < 2; ++i)
			_previous[i] = _current[i] = 0;
		return _stream->rewind();
	}

private:
	Audio::RewindableAudioStream *_stream;
	RA2PSXPitchRef _pitch;
	int _channels;
	int _fraction;
	bool _ended;
	int16 _previous[2];
	int16 _current[2];
};

bool matchesTag(const Common::Array<byte> &data, uint32 offset, const char *tag) {
	return offset + 4 <= data.size() && !memcmp(data.data() + offset, tag, 4);
}

bool timeReached(uint32 now, uint32 target) {
	return (int32)(now - target) >= 0;
}

int soundBalance(int pan) {
	// "sound mode: mono" in the options menu collapses the panning.
	if (ConfMan.hasKey("rebel2_mono") && ConfMan.getBool("rebel2_mono"))
		return 0;
	return CLIP((pan - 64) * 2, -127, 127);
}

bool RA2PSXSoundBank::load(const Common::Array<byte> &sampleData,
		const Common::Array<byte> &projectData) {
	Common::Array<Sample> samples;
	Common::Array<SFX> sfx;
	Common::Array<Macro> macros;
	Common::Array<ADSR> adsrs;

	if (sampleData.size() < 12 || projectData.size() < 32 ||
			!matchesTag(projectData, 0, "PROJ"))
		return false;

	const uint32 directorySize = READ_LE_UINT32(sampleData.data());
	if (!directorySize || directorySize % 12 || directorySize > sampleData.size())
		return false;

	for (uint32 offset = 0; offset < directorySize; offset += 12) {
		Sample sample;
		sample.offset = READ_LE_UINT32(sampleData.data() + offset);
		sample.id = READ_LE_UINT16(sampleData.data() + offset + 4);
		sample.blocks = READ_LE_UINT16(sampleData.data() + offset + 6);
		sample.rate = READ_LE_UINT16(sampleData.data() + offset + 8);
		const uint64 end = (uint64)sample.offset + (uint64)sample.blocks * 16;
		if (sample.offset < directorySize || !sample.blocks || !sample.rate ||
				end > sampleData.size())
			return false;
		samples.push_back(sample);
	}

	uint32 macroTag = 0xffffffff;
	uint32 sfxTag = 0xffffffff;
	for (uint32 offset = 0; offset + 4 <= projectData.size(); ++offset) {
		if (macroTag == 0xffffffff && matchesTag(projectData, offset, "MACR"))
			macroTag = offset;
		if (matchesTag(projectData, offset, "SFXG"))
			sfxTag = offset;
	}
	if (macroTag == 0xffffffff || macroTag + 16 > projectData.size() ||
			sfxTag == 0xffffffff || sfxTag + 24 > projectData.size())
		return false;

	const uint32 macroSection = READ_LE_UINT32(projectData.data() + macroTag + 12);
	if (macroSection > projectData.size() - 20)
		return false;
	const uint32 macroPointer = macroSection + 16;
	const uint64 macroBase64 = (uint64)macroPointer +
			READ_LE_UINT32(projectData.data() + macroPointer);
	if (macroBase64 < 4 || macroBase64 > projectData.size())
		return false;
	const uint32 macroBase = (uint32)macroBase64;
	// Records are (int32 offset relative to the table, uint32 id) pairs; swapping
	// the two pairs every macro with the previous entry's id.
	for (uint32 record = macroBase; record + 8 <= projectData.size() &&
			macros.size() < 256; record += 8) {
		const int32 relative = (int32)READ_LE_UINT32(projectData.data() + record);
		const uint32 id = READ_LE_UINT32(projectData.data() + record + 4);
		const int64 target = (int64)macroBase + relative;
		if (id > 0xffff || target < 0 || target + 8 > (int64)projectData.size() || (target & 3))
			break;
		Macro macro;
		macro.id = (uint32)id;
		macro.offset = (uint32)target;
		macros.push_back(macro);
	}
	if (macros.empty())
		return false;

	const uint32 sfxCount = READ_LE_UINT32(projectData.data() + sfxTag + 20);
	if (!sfxCount || sfxCount > 256 ||
			(uint64)sfxTag + 24 + (uint64)sfxCount * 10 > projectData.size())
		return false;
	for (uint32 i = 0; i < sfxCount; ++i) {
		const uint32 record = sfxTag + 24 + i * 10;
		SFX entry;
		entry.id = READ_LE_UINT16(projectData.data() + record);
		entry.macro = READ_LE_UINT16(projectData.data() + record + 2);
		entry.priority = projectData[record + 4];
		entry.maxVoices = projectData[record + 5];
		sfx.push_back(entry);
	}

	uint32 tableEnd = 0xffffffff;
	if (projectData.size() >= 8) {
		for (uint32 offset = projectData.size() - 8;; offset -= 4) {
			if (READ_LE_UINT32(projectData.data() + offset) == 0xffffffff &&
					READ_LE_UINT32(projectData.data() + offset + 4) == 0xffffffff) {
				tableEnd = offset;
				break;
			}
			if (offset < 4)
				break;
		}
	}
	if (tableEnd == 0xffffffff)
		return false;

	uint32 tableStart = tableEnd;
	int32 expectedRelative = -8;
	while (tableStart >= 8) {
		const uint32 record = tableStart - 8;
		const int32 relative = (int32)READ_LE_UINT32(projectData.data() + record);
		const uint32 id = READ_LE_UINT32(projectData.data() + record + 4);
		if (relative != expectedRelative || id > 0xffff)
			break;
		tableStart = record;
		expectedRelative -= 8;
	}
	for (uint32 record = tableStart; record < tableEnd; record += 8) {
		const int32 relative = (int32)READ_LE_UINT32(projectData.data() + record);
		const int64 target = (int64)tableStart + relative;
		if (target < 0 || target + 8 > tableStart)
			return false;
		ADSR adsr;
		adsr.id = (uint16)READ_LE_UINT32(projectData.data() + record + 4);
		adsr.attack = READ_LE_UINT16(projectData.data() + target);
		adsr.decay = READ_LE_UINT16(projectData.data() + target + 2);
		adsr.sustain = READ_LE_UINT16(projectData.data() + target + 4);
		adsr.release = READ_LE_UINT16(projectData.data() + target + 6);
		adsrs.push_back(adsr);
	}
	if (adsrs.empty())
		return false;

	_data = sampleData;
	_projectData = projectData;
	_samples = samples;
	_sfx = sfx;
	_macros = macros;
	_adsrs = adsrs;
	return true;
}

const RA2PSXSoundBank::Sample *RA2PSXSoundBank::findSample(uint16 id) const {
	for (uint i = 0; i < _samples.size(); ++i) {
		if (_samples[i].id == id)
			return &_samples[i];
	}
	return nullptr;
}

const RA2PSXSoundBank::ADSR *RA2PSXSoundBank::findADSR(uint16 id) const {
	for (uint i = 0; i < _adsrs.size(); ++i) {
		if (_adsrs[i].id == id)
			return &_adsrs[i];
	}
	return nullptr;
}

bool RA2PSXSoundBank::getSFX(uint16 id, uint16 &macro, byte &priority,
		byte &maxVoices) const {
	for (uint i = 0; i < _sfx.size(); ++i) {
		if (_sfx[i].id != id)
			continue;
		macro = _sfx[i].macro;
		priority = _sfx[i].priority;
		maxVoices = _sfx[i].maxVoices;
		return true;
	}
	return false;
}

bool RA2PSXSoundBank::getMacroCommand(uint16 macro, uint16 step, byte *command) const {
	for (uint i = 0; i < _macros.size(); ++i) {
		if (_macros[i].id != macro)
			continue;
		const uint64 offset = (uint64)_macros[i].offset + (uint64)step * 8;
		if (offset + 8 > _projectData.size())
			return false;
		memcpy(command, _projectData.data() + offset, 8);
		return true;
	}
	return false;
}

Audio::RewindableAudioStream *RA2PSXSoundBank::makeStream(uint16 id, uint32 macroRate,
		uint16 adsrId) const {
	const Sample *sample = findSample(id);
	if (!sample)
		return nullptr;

	const uint32 rate = macroRate ? macroRate : sample->rate;

	const uint32 size = (uint32)sample->blocks * 16;
	byte *copy = (byte *)malloc(size);
	if (!copy)
		return nullptr;
	memcpy(copy, _data.data() + sample->offset, size);

	// Every ADPCM block carries flags: bit 2 opens a loop, bit 1 asks the voice to repeat
	// at the end. Nearly every sample points that at its own final block, which is just a
	// sustain tail; only a genuine loop - the engine rumbles - opens earlier than that.
	uint32 loopStart = size;
	for (uint32 block = 0; block + 16 <= size; block += 16) {
		if (copy[block + 1] & 0x04) {
			loopStart = block;
			break;
		}
	}
	const bool loops = size >= 32 && (copy[size - 15] & 0x02) && loopStart + 16 < size;

	Common::SeekableReadStream *source =
			new Common::MemoryReadStream(copy, size, DisposeAfterUse::YES);
	Audio::RewindableAudioStream *stream = Audio::makeXAStream(source, rate);
	if (loops)
		stream = new RA2PSXLoopStream(stream);
	const ADSR *adsr = findADSR(adsrId);
	if (adsr)
		stream = new RA2PSXADSRStream(stream, adsr->attack, adsr->decay, adsr->sustain);
	return stream;
}

struct RA2PSXSoundPlayer::Impl {
	enum {
		kVoiceCount = 24,
		kGroupCount = 64,
		kMacroTicksPerSecond = 60,
		kVoiceStartupTicks = 4
	};

	struct Voice {
		Voice() : active(false), waiting(false), waitForSampleEnd(false), macroDone(false),
				root(false), sound(0), macro(0), step(0), loopsLeft(-1), adsr(0xffff), rate(0),
				sampleId(0), bendUp(2), bendDown(2),
				pitch(RA2PSXSoundPlayer::kNeutralBend), volume(0), pan(64), priority(0),
				born(0), readyTick(0),
				startedAt(0), waitUntil(0) {}

		bool active;
		bool waiting;
		bool waitForSampleEnd;
		bool macroDone;
		bool root;
		SoundId sound;
		uint16 macro;
		uint16 step;
		// Countdown for macro op 5; negative means it has not been armed yet.
		int loopsLeft;
		uint16 adsr;
		uint16 sampleId;
		RA2PSXPitchRef pitchRef;
		int bendUp;
		int bendDown;
		uint32 rate;
		int pitch;
		int volume;
		int pan;
		byte priority;
		uint32 born;
		uint32 readyTick;
		uint32 startedAt;
		uint32 waitUntil;
		Audio::SoundHandle handle;
	};

	struct Group {
		Group() : active(false), hasExpiry(false), sound(0), born(0), expiry(0) {}

		bool active;
		bool hasExpiry;
		SoundId sound;
		uint32 born;
		uint32 expiry;
	};

	Impl(ScummEngine_v7 *engine, const RA2PSXSoundBank &soundBank)
			: vm(engine), bank(soundBank), nextSound(1), tick(1), serial(1),
			nextUpdate(0), tickRemainder(0), randomState(0x13579bdf) {}

	ScummEngine_v7 *vm;
	const RA2PSXSoundBank &bank;
	Voice voices[kVoiceCount];
	Group groups[kGroupCount];
	SoundId nextSound;
	uint32 tick;
	uint32 serial;
	uint32 nextUpdate;
	uint32 tickRemainder;
	uint32 randomState;

	int findGroup(SoundId sound) const {
		for (int i = 0; i < kGroupCount; ++i) {
			if (groups[i].active && groups[i].sound == sound)
				return i;
		}
		return -1;
	}

	void clearVoice(int index) {
		if (voices[index].active)
			vm->_mixer->stopHandle(voices[index].handle);
		voices[index] = Voice();
	}

	// Keyed on the sound, not the group: the group can be gone while voices play.
	void stopSound(SoundId sound) {
		for (int i = 0; i < kVoiceCount; ++i) {
			if (voices[i].active && voices[i].sound == sound)
				clearVoice(i);
		}
		const int group = findGroup(sound);
		if (group >= 0)
			groups[group] = Group();
	}

	void stopGroup(int index) {
		stopSound(groups[index].sound);
	}

	int allocateGroup(SoundId sound) {
		int slot = -1;
		for (int i = 0; i < kGroupCount; ++i) {
			if (!groups[i].active) {
				slot = i;
				break;
			}
		}
		if (slot < 0) {
			slot = 0;
			for (int i = 1; i < kGroupCount; ++i) {
				if (groups[i].born < groups[slot].born)
					slot = i;
			}
			stopGroup(slot);
		}
		groups[slot].active = true;
		groups[slot].sound = sound;
		groups[slot].born = serial++;
		return slot;
	}

	int allocateVoice(uint16 macro, byte priority, byte maxVoices, int excluded) {
		int sameMacro = 0;
		int oldestSame = -1;
		for (int i = 0; i < kVoiceCount; ++i) {
			if (!voices[i].active || voices[i].macro != macro || i == excluded)
				continue;
			++sameMacro;
			if (oldestSame < 0 || voices[i].born < voices[oldestSame].born)
				oldestSame = i;
		}
		if (maxVoices < kVoiceCount && sameMacro >= maxVoices) {
			if (oldestSame < 0)
				return -1;
			clearVoice(oldestSame);
			return oldestSame;
		}

		for (int i = 0; i < kVoiceCount; ++i) {
			if (!voices[i].active)
				return i;
		}

		int victim = -1;
		for (int i = 0; i < kVoiceCount; ++i) {
			if (i == excluded || voices[i].priority > priority)
				continue;
			if (victim < 0 || voices[i].priority < voices[victim].priority ||
					(voices[i].priority == voices[victim].priority &&
					 voices[i].born < voices[victim].born))
				victim = i;
		}
		if (victim >= 0)
			clearVoice(victim);
		return victim;
	}

	int startVoice(SoundId sound, uint16 macro, uint16 step, byte priority,
			byte maxVoices, int volume, int pan, int pitch, bool root,
			uint32 now, int excluded) {
		const int slot = allocateVoice(macro, priority, maxVoices, excluded);
		if (slot < 0)
			return -1;

		Voice &voice = voices[slot];
		voice.active = true;
		voice.root = root;
		voice.sound = sound;
		voice.macro = macro;
		voice.step = step;
		voice.pitch = pitch;
		voice.volume = CLIP(volume, 0, 127);
		voice.pan = CLIP(pan, 0, 127);
		voice.priority = priority;
		voice.born = serial++;
		voice.readyTick = tick + kVoiceStartupTicks;
		voice.startedAt = now;
		return slot;
	}

	void setGroupExpiry(SoundId sound, uint32 expiry) {
		const int group = findGroup(sound);
		if (group >= 0) {
			groups[group].hasExpiry = true;
			groups[group].expiry = expiry;
		}
	}

	bool nextCommandEnds(const Voice &voice) const {
		byte command[8];
		return bank.getMacroCommand(voice.macro, voice.step, command) &&
				((command[0] & 0x3f) == 0 || (command[0] & 0x3f) == 1);
	}

	void finishVoice(int index) {
		if (voices[index].root) {
			stopSound(voices[index].sound);
			return;
		}
		voices[index].macroDone = true;
		if (!vm->_mixer->isSoundHandleActive(voices[index].handle))
			clearVoice(index);
	}

	void runVoice(int index, uint32 now) {
		for (int commandCount = 0; commandCount < 64; ++commandCount) {
			if (!voices[index].active)
				return;
			Voice &voice = voices[index];
			byte command[8];
			if (!bank.getMacroCommand(voice.macro, voice.step++, command)) {
				finishVoice(index);
				return;
			}

			switch (command[0] & 0x3f) {
			case 0:
			case 1:
				finishVoice(index);
				return;
			case 7: {
				uint32 milliseconds = READ_LE_UINT16(command + 6);
				if (command[2] && milliseconds) {
					randomState = randomState * 1103515245 + 12345;
					milliseconds = randomState % milliseconds;
				}
				voice.waitUntil = command[4] ? voice.startedAt + milliseconds : now + milliseconds;
				// Byte 1 bit 0 also ends on the sample; 0xffff means no limit.
				voice.waitForSampleEnd = (command[1] & 1) != 0;
				voice.waiting = milliseconds != 0;
				if (milliseconds == 0xffff)
					voice.waitUntil = now + 0xffffff;
				if (voice.root && nextCommandEnds(voice))
					setGroupExpiry(voice.sound, voice.waitUntil);
				if (voice.waiting)
					return;
				break;
			}
			case 8: {
				const uint16 macro = READ_LE_UINT16(command + 2);
				const uint16 step = READ_LE_UINT16(command + 4);
				if (macro != voice.macro) {
					startVoice(voice.sound, macro, step, command[6], command[7],
							voice.volume, voice.pan, voice.pitch, false, now, index);
				}
				break;
			}
			case 5:
				// Jump back to a step, repeating the count in bytes 6-7; 0xffff never
				// runs out. Only the low shield alarm uses it: 05 01 .. ff ff, so the
				// beep-pause-beep body repeats for as long as the sound is held.
				if (voice.loopsLeft == 0)
					break;
				if (voice.loopsLeft > 0)
					--voice.loopsLeft;
				else if (READ_LE_UINT16(command + 6) != 0xffff)
					voice.loopsLeft = READ_LE_UINT16(command + 6);
				voice.step = command[1];
				break;
			case 0xc:
				voice.adsr = READ_LE_UINT16(command + 1);
				break;
			case 0x33:
				voice.bendUp = command[1] ? command[1] : 1;
				voice.bendDown = command[2] ? command[2] : 1;
				break;
			case 0x10: {
				vm->_mixer->stopHandle(voice.handle);
				voice.sampleId = READ_LE_UINT16(command + 1);
				startSample(index);
				break;
			}
			case 0x1f:
				voice.rate = (uint32)command[1] | ((uint32)command[2] << 8) |
						((uint32)command[3] << 16);
				break;
			default:
				break;
			}
		}
		finishVoice(index);
	}

	void startSample(int index) {
		Voice &voice = voices[index];
		if (!voice.sampleId)
			return;
		Audio::RewindableAudioStream *stream = bank.makeStream(voice.sampleId, voice.rate,
				voice.adsr);
		if (!stream)
			return;
		voice.pitchRef = RA2PSXPitchRef(new RA2PSXPitch(
				bendToStep(voice.pitch, voice.bendUp, voice.bendDown)));
		stream = new RA2PSXPitchStream(stream, voice.pitchRef);
		vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &voice.handle, stream, -1,
				voice.volume * Audio::Mixer::kMaxChannelVolume / 127, soundBalance(voice.pan));
	}

	SoundId play(uint16 sfxId, int volume, int pan, int pitch) {
		uint16 macro;
		byte priority;
		byte maxVoices;
		if (!bank.getSFX(sfxId, macro, priority, maxVoices))
			return kInvalidSoundId;

		SoundId sound = nextSound++;
		if (sound == kInvalidSoundId)
			sound = nextSound++;
		const int group = allocateGroup(sound);
		const uint32 now = g_system->getMillis();
		const int voice = startVoice(sound, macro, 0, priority, maxVoices,
				volume, pan, pitch, true, now, -1);
		if (voice < 0) {
			groups[group] = Group();
			return kInvalidSoundId;
		}
		return sound;
	}

	void update() {
		const uint32 now = g_system->getMillis();
		if (nextUpdate && !timeReached(now, nextUpdate))
			return;
		uint32 tickMillis = 1000 / kMacroTicksPerSecond;
		tickRemainder += 1000 % kMacroTicksPerSecond;
		if (tickRemainder >= kMacroTicksPerSecond) {
			++tickMillis;
			tickRemainder -= kMacroTicksPerSecond;
		}
		nextUpdate = now + tickMillis;
		++tick;

		for (int i = 0; i < kGroupCount; ++i) {
			if (groups[i].active && groups[i].hasExpiry && timeReached(now, groups[i].expiry))
				stopGroup(i);
		}

		for (int i = 0; i < kVoiceCount; ++i) {
			Voice &voice = voices[i];
			if (!voice.active || voice.readyTick > tick)
				continue;
			if (voice.macroDone) {
				if (!vm->_mixer->isSoundHandleActive(voice.handle))
					clearVoice(i);
				continue;
			}
			if (voice.waiting) {
				const bool sampleEnded = voice.waitForSampleEnd &&
						!vm->_mixer->isSoundHandleActive(voice.handle);
				if (!sampleEnded && !timeReached(now, voice.waitUntil))
					continue;
				voice.waiting = false;
			}
			runVoice(i, now);
		}

		for (int i = 0; i < kGroupCount; ++i) {
			if (!groups[i].active || groups[i].hasExpiry)
				continue;
			bool hasVoice = false;
			for (int j = 0; j < kVoiceCount; ++j)
				hasVoice |= voices[j].active && voices[j].sound == groups[i].sound;
			if (!hasVoice)
				groups[i] = Group();
		}
	}

	void setPan(SoundId sound, int pan) {
		pan = CLIP(pan, 0, 127);
		for (int i = 0; i < kVoiceCount; ++i) {
			if (!voices[i].active || voices[i].sound != sound)
				continue;
			voices[i].pan = pan;
			if (vm->_mixer->isSoundHandleActive(voices[i].handle))
				vm->_mixer->setChannelBalance(voices[i].handle, soundBalance(pan));
		}
	}

	void setVolume(SoundId sound, int volume) {
		volume = CLIP(volume, 0, 127);
		for (int i = 0; i < kVoiceCount; ++i) {
			if (!voices[i].active || voices[i].sound != sound)
				continue;
			voices[i].volume = volume;
			if (vm->_mixer->isSoundHandleActive(voices[i].handle))
				vm->_mixer->setChannelVolume(voices[i].handle,
						volume * Audio::Mixer::kMaxChannelVolume / 127);
		}
	}

	void setPitch(SoundId sound, int pitch) {
		pitch = CLIP(pitch, 0, 0x3fff);
		for (int i = 0; i < kVoiceCount; ++i) {
			if (!voices[i].active || voices[i].sound != sound)
				continue;
			voices[i].pitch = pitch;
			if (voices[i].pitchRef)
				voices[i].pitchRef->pitch =
						bendToStep(pitch, voices[i].bendUp, voices[i].bendDown);
		}
	}

	void stop(SoundId sound) {
		stopSound(sound);
	}

	void stopAll() {
		for (int i = 0; i < kGroupCount; ++i) {
			if (groups[i].active)
				stopGroup(i);
		}
	}
};

RA2PSXSoundPlayer::RA2PSXSoundPlayer(ScummEngine_v7 *vm, const RA2PSXSoundBank &bank)
		: _impl(new Impl(vm, bank)) {
}

RA2PSXSoundPlayer::~RA2PSXSoundPlayer() {
	_impl->stopAll();
	delete _impl;
}

RA2PSXSoundPlayer::SoundId RA2PSXSoundPlayer::play(uint16 sfx, int volume,
		int pan, int pitch) {
	return _impl->play(sfx, volume, pan, pitch);
}

void RA2PSXSoundPlayer::update() {
	_impl->update();
}

void RA2PSXSoundPlayer::setPan(SoundId sound, int pan) {
	_impl->setPan(sound, pan);
}

void RA2PSXSoundPlayer::setVolume(SoundId sound, int volume) {
	_impl->setVolume(sound, volume);
}

void RA2PSXSoundPlayer::setPitch(SoundId sound, int pitch) {
	_impl->setPitch(sound, pitch);
}

void RA2PSXSoundPlayer::stop(SoundId sound) {
	_impl->stop(sound);
}

void RA2PSXSoundPlayer::stopAll() {
	_impl->stopAll();
}

} // End of namespace Scumm
