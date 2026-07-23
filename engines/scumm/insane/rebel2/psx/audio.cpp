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

#include "common/endian.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/util.h"

#include "scumm/scumm_v7.h"
#include "scumm/insane/rebel2/psx/psx.h"

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

static bool matchesTag(const Common::Array<byte> &data, uint32 offset, const char *tag) {
	return offset + 4 <= data.size() && !memcmp(data.data() + offset, tag, 4);
}

static bool timeReached(uint32 now, uint32 target) {
	return (int32)(now - target) >= 0;
}

static int soundBalance(int pan) {
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
	for (uint32 record = macroBase - 4; record + 8 <= projectData.size() &&
			macros.size() < 256; record += 8) {
		const int32 relative = (int32)READ_LE_UINT32(projectData.data() + record + 4);
		const int64 target = (int64)macroBase + relative;
		if (target < 0 || target + 8 > (int64)projectData.size() || (target & 3))
			break;
		Macro macro;
		macro.id = READ_LE_UINT32(projectData.data() + record);
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

Audio::RewindableAudioStream *RA2PSXSoundBank::makeStream(uint16 id, uint32 rate,
		uint16 adsrId) const {
	const Sample *sample = findSample(id);
	if (!sample)
		return nullptr;

	const uint32 size = (uint32)sample->blocks * 16;
	byte *copy = (byte *)malloc(size);
	if (!copy)
		return nullptr;
	memcpy(copy, _data.data() + sample->offset, size);

	Common::SeekableReadStream *source =
			new Common::MemoryReadStream(copy, size, DisposeAfterUse::YES);
	Audio::RewindableAudioStream *stream =
			Audio::makeXAStream(source, rate ? rate : sample->rate);
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
				root(false), sound(0), macro(0), step(0), adsr(0xffff), rate(0),
				rateOverride(-1), volume(0), pan(64), priority(0), born(0), readyTick(0),
				startedAt(0), waitUntil(0) {}

		bool active;
		bool waiting;
		bool waitForSampleEnd;
		bool macroDone;
		bool root;
		SoundId sound;
		uint16 macro;
		uint16 step;
		uint16 adsr;
		uint32 rate;
		int rateOverride;
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

	void stopGroup(int index) {
		const SoundId sound = groups[index].sound;
		for (int i = 0; i < kVoiceCount; ++i) {
			if (voices[i].active && voices[i].sound == sound)
				clearVoice(i);
		}
		groups[index] = Group();
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
			byte maxVoices, int volume, int pan, int rateOverride, bool root,
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
		voice.rateOverride = rateOverride;
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
		const SoundId sound = voices[index].sound;
		if (voices[index].root) {
			const int group = findGroup(sound);
			if (group >= 0)
				stopGroup(group);
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
				voice.waitForSampleEnd = command[3] != 0;
				voice.waiting = milliseconds != 0;
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
							voice.volume, voice.pan, -1, false, now, index);
				}
				break;
			}
			case 0xc:
				voice.adsr = READ_LE_UINT16(command + 1);
				break;
			case 0x10: {
				vm->_mixer->stopHandle(voice.handle);
				const uint32 rate = voice.rateOverride >= 0 ?
						(uint32)voice.rateOverride : voice.rate;
				if (rate != 0 || voice.rateOverride < 0) {
					Audio::RewindableAudioStream *stream = bank.makeStream(
							READ_LE_UINT16(command + 1), rate, voice.adsr);
					if (stream) {
						vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &voice.handle,
								stream, -1,
								voice.volume * Audio::Mixer::kMaxChannelVolume / 127,
								soundBalance(voice.pan));
					}
				}
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

	SoundId play(uint16 sfxId, int volume, int pan, int rate) {
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
				volume, pan, rate, true, now, -1);
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

	void stop(SoundId sound) {
		const int group = findGroup(sound);
		if (group >= 0)
			stopGroup(group);
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
		int pan, int rate) {
	return _impl->play(sfx, volume, pan, rate);
}

void RA2PSXSoundPlayer::update() {
	_impl->update();
}

void RA2PSXSoundPlayer::setPan(SoundId sound, int pan) {
	_impl->setPan(sound, pan);
}

void RA2PSXSoundPlayer::stop(SoundId sound) {
	_impl->stop(sound);
}

void RA2PSXSoundPlayer::stopAll() {
	_impl->stopAll();
}

} // End of namespace Scumm
