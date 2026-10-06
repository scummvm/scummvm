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

#include "audio/fmopl.h"
#include "audio/mixer.h"
#include "common/endian.h"
#include "common/mutex.h"
#include "common/queue.h"
#include "common/textconsole.h"
#include "freescape/sound.h"
#include "freescape/sound/3dck.h"

namespace Freescape {

// The RUN driver uses early Westwood AdLib bytecode with absolute offsets.
class KitAdLibSound : public Sound {
public:
	KitAdLibSound(Audio::Mixer *mixer, const Common::Array<byte> &data);
	~KitAdLibSound() override;
	void playSound(int index, Type type) override;
	void stopSound(Type type) override;
	bool isPlayingSound(Type type) const override;
	bool isSoundAvailable(int index) const override;

private:
	enum Command {
		kRepeat = 0, kCheckRepeat, kStartProgram, kNoteSpacing, kJump, kCall, kReturn,
		kBaseOctave, kStop, kRest, kWriteRegister, kNoteDuration, kBaseNote,
		kSecondaryEffect, kStopOther,
		kInstrument = 16, kSlide, kRemoveSlide, kBaseFrequency,
		kVibrato = 21, kPriority = 26, kBeat = 28, kWaitBeat, kLevel1,
		kDuration = 32, kNoteOn, kFractionalSpacing = 36, kTempo = 38,
		kRemoveSecondary, kChannelTempo = 41, kLevel3 = 43, kLevel2, kChangeLevel2,
		kAMDepth, kVibratoDepth, kChangeLevel1, kClearChannel = 51,
		kRandomNote = 53, kRemoveVibrato, kPitchBend = 57, kResetTempo, kNop,
		kRandomDuration, kChangeTempo, kKeyModulation = 63, kRemoveKeyModulation,
		kSetupRhythm, kPlayRhythm, kRemoveRhythm, kRhythmLevel2, kChangeRhythmLevel1,
		kRhythmLevel1, kTempoReset = 72, kNoteLevels
	};
	enum Effect { kNoEffect, kSlideEffect, kVibratoEffect };
	struct Channel {
		uint32 ip = 0;
		uint16 stack[4] = {};
		byte stackSize = 0, duration = 0, repeat = 0;
		int8 priority = 0, baseOctave = 0, baseNote = 0, pitchBend = 0;
		byte baseFrequency = 0, rawNote = 0;
		byte tempo = 255, timer = 255, tempoReset = 0;
		byte spacing = 1, fractionalSpacing = 0, gateDuration = 0, durationRandomness = 0;
		byte regA = 0, regB = 0;
		byte level1 = 0, level2 = 0, level3 = 0;
		byte opLevel[2] = {}, noteLevelScale[2] = {}, noteLevel = 0;
		bool additive = false;
		Effect effect = kNoEffect;
		byte slideTempo = 0, slideTimer = 255;
		int16 slideStep = 0, vibratoStep = 0;
		byte vibratoTempo = 0, vibratoTimer = 0, vibratoRange = 0;
		byte vibratoSteps = 0, vibratoCount = 0, vibratoDelay = 0, vibratoWait = 0;
		uint16 secondaryData = 0;
		byte secondaryTempo = 0, secondaryTimer = 0, secondaryRegister = 0;
		int8 secondarySize = 0, secondaryPosition = 0;
	};

	bool contains(uint32 offset, uint size) const { return offset <= _data.size() && size <= _data.size() - offset; }
	uint16 word(uint offset) const { return contains(offset, 2) ? READ_LE_UINT16(&_data[offset]) : 0; }
	uint16 program(uint16 bank, byte index) const;
	uint16 mapSound(uint16 index) const { return index <= 20 ? word(0x2f6 + index * 2) : index; }
	static bool advance(byte &timer, byte tempo) {
		uint sum = timer + tempo;
		timer = sum;
		return sum > 255;
	}
	void onTimer();
	void tick();
	void startProgram(byte index);
	void stopChannel(uint index, bool clear);
	bool executeChannel(uint index);
	bool executeCommand(uint index, byte command, const byte *values);
	void updateEffects(uint index);
	void setupNote(uint index, byte note);
	void noteOn(uint index);
	void noteOff(uint index);
	bool setupDuration(Channel &channel, byte duration);
	void setupInstrument(uint index, byte instrument, Channel &channel);
	void setupRhythm(Channel &channel, const byte *values);
	void rhythmLevel(byte command, byte mask, byte value);
	void adjustLevel(uint index);
	byte operatorLevel(const Channel &channel, uint op) const;
	void writeRegister(byte reg, byte value);
	void writeVolume(uint index);
	void updateVolume();
	uint16 getRandomNumber();

	Common::Array<byte> _data;
	Common::Queue<byte> _queue;
	Channel _channels[10];
	Audio::Mixer *_mixer;
	OPL::OPL *_opl;
	mutable Common::Mutex _mutex;
	uint32 _timer = 0;
	uint16 _bank = 0, _random = 0x1234;
	byte _divider = 12, _tempo = 0, _beatTimer = 255;
	byte _beatDivider = 0, _beatCount = 0, _beat = 0, _beatWaiting = 0;
	byte _depth = 0, _rhythm = 0;
	byte _rhythmLevel[5] = {}, _rhythmLevel1[5] = {}, _rhythmLevel2[5] = {};
	byte _keyModulation = 0, _keyPeriod = 0, _keyTimer = 0, _keyState = 0;
	byte _registers[256] = {};
	int _volume = 255;
	static const byte kOperators[9];
	static const byte kArgumentCounts[74];
};

const byte KitAdLibSound::kOperators[9] = {0, 1, 2, 8, 9, 10, 16, 17, 18};
const byte KitAdLibSound::kArgumentCounts[74] = {
	1, 2, 1, 1, 2, 2, 0, 1, 0, 1, 2, 2, 1, 5, 1, 0,
	1, 3, 0, 1, 0, 4, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0,
	1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 2, 2, 1, 1,
	1, 0, 0, 1, 0, 2, 0, 0, 0, 1, 0, 0, 1, 1, 0, 2,
	0, 9, 1, 0, 2, 2, 2, 0, 1, 2
};

KitAdLibSound::KitAdLibSound(Audio::Mixer *mixer, const Common::Array<byte> &data) : _data(data), _mixer(mixer) {
	_opl = OPL::Config::create();
	if (!_opl || !_opl->init())
		error("Unable to initialize 3D Construction Kit AdLib sound");
	_bank = word(0x447c);
	writeRegister(1, 0x20);
	writeRegister(8, 0);
	stopSound(kTypeNormal);
	_opl->start(new Common::Functor0Mem<void, KitAdLibSound>(this, &KitAdLibSound::onTimer), 1000);
}

KitAdLibSound::~KitAdLibSound() {
	_opl->stop();
	delete _opl;
}

uint16 KitAdLibSound::program(uint16 bank, byte index) const {
	uint16 offset = word(bank + index * 2);
	return offset && contains(offset, 2) && _data[offset] < 10 ? offset : 0;
}

bool KitAdLibSound::isSoundAvailable(int index) const {
	if (index < 0 || index > 0xffff)
		return false;
	uint16 sound = mapSound(index);
	return program(word(0x447c + (sound > 255 ? 2 : 0)), sound & 255) != 0;
}

void KitAdLibSound::playSound(int index, Type type) {
	if (!isSoundAvailable(index))
		return;
	Common::StackLock lock(_mutex);
	uint16 sound = mapSound(index);
	_bank = word(0x447c + (sound > 255 ? 2 : 0));
	if (_queue.size() == 16)
		_queue.pop();
	_queue.push(sound & 255);
}

void KitAdLibSound::stopSound(Type type) {
	if (type == kTypeMovement)
		return;
	Common::StackLock lock(_mutex);
	_queue.clear();
	_keyModulation = _rhythm = _depth = 0;
	writeRegister(0xbd, 0);
	for (uint i = 0; i < 10; i++) {
		_channels[i] = Channel();
		stopChannel(i, true);
	}
}

bool KitAdLibSound::isPlayingSound(Type type) const {
	Common::StackLock lock(_mutex);
	if (!_queue.empty())
		return true;
	for (const Channel &channel : _channels)
		if (channel.ip)
			return true;
	return false;
}

void KitAdLibSound::onTimer() {
	Common::StackLock lock(_mutex);
	updateVolume();
	// PIT reload 1365, with one sequencer update every twelve interrupts.
	_timer += kKitPITClock;
	while (_timer >= 1365 * 1000) {
		_timer -= 1365 * 1000;
		if (_keyModulation && !--_keyTimer) {
			_keyState ^= 0x20;
			_channels[0].regB = (_channels[0].regB & 0x1f) | _keyState;
			writeRegister(0xb0, _channels[0].regB);
			_keyTimer = _keyPeriod;
		}
		if (!--_divider) {
			_divider = 12;
			tick();
		}
	}
}

void KitAdLibSound::startProgram(byte index) {
	uint16 offset = program(_bank, index);
	if (!offset)
		return;
	byte channelIndex = _data[offset];
	int8 priority = _data[offset + 1];
	Channel &channel = _channels[channelIndex];
	if (!channelIndex)
		_keyModulation = 0;
	if (priority < channel.priority)
		return;
	byte level = channel.level2;
	channel = Channel();
	channel.level2 = level;
	channel.priority = priority;
	channel.ip = offset + 2;
	channel.duration = 1;
	if (channelIndex < 9 && (!_rhythm || channelIndex < 6)) {
		byte op = kOperators[channelIndex];
		writeRegister(0x60 + op, 255);
		writeRegister(0x63 + op, 255);
		writeRegister(0x80 + op, 255);
		writeRegister(0x83 + op, 255);
		writeRegister(0xb0 + channelIndex, 0);
		writeRegister(0xb0 + channelIndex, 0x20);
	}
}

void KitAdLibSound::tick() {
	while (!_queue.empty())
		startProgram(_queue.pop());
	for (int i = 9; i >= 0; i--) {
		Channel &channel = _channels[i];
		if (!channel.ip)
			continue;
		if (channel.tempoReset)
			channel.tempo = _tempo;
		if (advance(channel.timer, channel.tempo)) {
			if (!--channel.duration) {
				if (!executeChannel(i))
					continue;
			} else if (channel.duration == channel.spacing || channel.duration == channel.gateDuration) {
				noteOff(i);
			}
		}
		if (channel.ip)
			updateEffects(i);
	}
	if (advance(_beatTimer, _tempo) && !--_beatCount) {
		_beatCount = _beatDivider;
		_beat++;
	}
}

bool KitAdLibSound::executeChannel(uint index) {
	Channel &channel = _channels[index];
	for (uint budget = 4096; budget && channel.ip; budget--) {
		if (!contains(channel.ip, 1))
			break;
		byte opcode = _data[channel.ip++];
		if (opcode & 0x80) {
			opcode &= 0x7f;
			if (opcode >= ARRAYSIZE(kArgumentCounts) || !contains(channel.ip, kArgumentCounts[opcode]))
				break;
			const byte *values = _data.data() + channel.ip;
			channel.ip += kArgumentCounts[opcode];
			if (executeCommand(index, opcode, values))
				return channel.ip != 0 && opcode != kWaitBeat;
		} else {
			uint size = 1 + ((channel.noteLevelScale[0] || channel.noteLevelScale[1]) ? 1 : 0);
			if (!contains(channel.ip, size))
				break;
			setupNote(index, opcode);
			noteOn(index);
			bool wait = setupDuration(channel, _data[channel.ip++]);
			if (size == 2) {
				channel.noteLevel = _data[channel.ip++];
				adjustLevel(index);
			}
			if (wait)
				return true;
		}
	}
	if (channel.ip)
		warning("Invalid 3D Construction Kit AdLib program at %x", channel.ip);
	stopChannel(index, false);
	return false;
}

bool KitAdLibSound::executeCommand(uint index, byte command, const byte *v) {
	Channel &channel = _channels[index];
	switch (command) {
	case kRepeat: channel.repeat = v[0]; break;
	case kCheckRepeat:
		if (!--channel.repeat)
			break;
		// fall through
	case kJump: channel.ip = READ_LE_UINT16(v); break;
	case kCall:
		if (channel.stackSize == ARRAYSIZE(channel.stack)) {
			stopChannel(index, false);
			return true;
		}
		channel.stack[channel.stackSize++] = channel.ip;
		channel.ip = READ_LE_UINT16(v);
		break;
	case kReturn:
		if (!channel.stackSize) {
			stopChannel(index, false);
			return true;
		}
		channel.ip = channel.stack[--channel.stackSize];
		break;
	case kStartProgram:
		if (v[0] != 255)
			startProgram(v[0]);
		break;
	case kNoteSpacing: channel.spacing = v[0]; break;
	case kBaseOctave: channel.baseOctave = v[0]; break;
	case kRest:
		noteOff(index);
		return setupDuration(channel, v[0]);
	case kWriteRegister: writeRegister(v[0], v[1]); break;
	case kNoteDuration:
		setupNote(index, v[0]);
		return setupDuration(channel, v[1]);
	case kBaseNote: channel.baseNote = v[0]; break;
	case kSecondaryEffect:
		channel.secondaryTempo = channel.secondaryTimer = v[0];
		channel.secondarySize = channel.secondaryPosition = v[1];
		channel.secondaryRegister = v[2];
		channel.secondaryData = READ_LE_UINT16(v + 3);
		break;
	case kStopOther:
		if (v[0] < 10) {
			_channels[v[0]].ip = 0;
			_channels[v[0]].priority = 0;
		}
		break;
	case kInstrument: setupInstrument(index, v[0], channel); break;
	case kSlide:
		channel.slideTempo = v[0];
		channel.slideTimer = 255;
		channel.slideStep = READ_BE_UINT16(v + 1);
		channel.effect = kSlideEffect;
		break;
	case kRemoveSlide:
		channel.slideStep = 0;
		// fall through
	case kRemoveVibrato: channel.effect = kNoEffect; break;
	case kBaseFrequency: channel.baseFrequency = v[0]; break;
	case kVibrato:
		channel.vibratoTempo = v[0];
		channel.vibratoRange = v[1];
		channel.vibratoCount = v[2] + 1;
		channel.vibratoSteps = v[2] * 2;
		channel.vibratoDelay = v[3];
		channel.effect = kVibratoEffect;
		break;
	case kPriority: channel.priority = v[0]; break;
	case kBeat:
		_beatDivider = _beatCount = v[0] >> 1;
		_beatTimer = 255;
		_beat = _beatWaiting = 0;
		break;
	case kWaitBeat:
		if (_beatWaiting && (_beat & v[0])) {
			_beatWaiting = 0;
			break;
		}
		if (!_beatWaiting && !(_beat & v[0]))
			_beatWaiting = 1;
		channel.ip -= 2;
		channel.duration = 1;
		return true;
	case kLevel1: channel.level1 = v[0]; adjustLevel(index); break;
	case kDuration: return setupDuration(channel, v[0]);
	case kNoteOn:
		noteOn(index);
		return setupDuration(channel, v[0]);
	case kFractionalSpacing: channel.fractionalSpacing = v[0] & 7; break;
	case kTempo: _tempo = v[0]; break;
	case kRemoveSecondary: channel.secondaryData = 0; break;
	case kChannelTempo: channel.tempo = v[0]; break;
	case kLevel3: channel.level3 = v[0]; break;
	case kLevel2:
	case kChangeLevel2:
		if (v[0] < 10) {
			_channels[v[0]].level2 = v[1] + (command == kChangeLevel2 ? _channels[v[0]].level2 : 0);
			adjustLevel(v[0]);
		}
		break;
	case kAMDepth:
		_depth = (_depth & 0x7f) | ((v[0] & 1) << 7);
		writeRegister(0xbd, _depth);
		break;
	case kVibratoDepth:
		_depth = (_depth & 0xbf) | ((v[0] & 1) << 6);
		writeRegister(0xbd, _depth);
		break;
	case kChangeLevel1: channel.level1 += v[0]; adjustLevel(index); break;
	case kClearChannel:
		if (v[0] < 10)
			stopChannel(v[0], true);
		break;
	case kRandomNote: {
		uint16 note = (((channel.regB & 0x1f) << 8) | channel.regA) + (getRandomNumber() & READ_BE_UINT16(v));
		if (index < 9) {
			writeRegister(0xa0 + index, note & 255);
			writeRegister(0xb0 + index, (note >> 8) | (channel.regB & 0x20));
		}
		break;
	}
	case kPitchBend: channel.pitchBend = v[0]; setupNote(index, channel.rawNote); break;
	case kResetTempo: channel.tempo = _tempo; break;
	case kNop: break;
	case kRandomDuration: channel.durationRandomness = v[0]; break;
	case kChangeTempo: channel.tempo = CLIP<int>(channel.tempo + int8(v[0]), 1, 255); break;
	case kKeyModulation:
		if (v[1] < 5) {
			_keyModulation = v[0];
			if (v[0] == 2) {
				uint16 period = word(0x21b5 + v[1] * 2);
				uint16 frequency = word(0x21b5 + (v[1] + 1) * 2);
				if (contains(period, 1) && contains(frequency, 1)) {
					_keyPeriod = _data[period];
					writeRegister(0xa0, _data[frequency]);
				}
			}
		}
		break;
	case kRemoveKeyModulation: _keyModulation = 0; break;
	case kSetupRhythm: setupRhythm(channel, v); break;
	case kPlayRhythm:
		writeRegister(0xbd, (_rhythm & ~(v[0] & 31)) | 0x20);
		_rhythm |= v[0];
		writeRegister(0xbd, _depth | 0x20 | _rhythm);
		break;
	case kRemoveRhythm:
		_rhythm = 0;
		writeRegister(0xbd, _depth & 0xc0);
		break;
	case kRhythmLevel2:
	case kChangeRhythmLevel1:
	case kRhythmLevel1: rhythmLevel(command, v[0], v[1]); break;
	case kTempoReset: channel.tempoReset = v[0]; break;
	case kNoteLevels:
		channel.noteLevelScale[0] = v[0];
		channel.noteLevelScale[1] = v[1];
		break;
	default:
		stopChannel(index, false);
		return true;
	}
	return false;
}

void KitAdLibSound::stopChannel(uint index, bool clear) {
	Channel &channel = _channels[index];
	channel.ip = 0;
	channel.duration = 0;
	channel.priority = 0;
	if (!clear) {
		noteOff(index);
		return;
	}
	channel.level2 = 0;
	if (!index)
		_keyModulation = 0;
	if (index < 9) {
		writeRegister(0xc0 + index, 0);
		writeRegister(0x43 + kOperators[index], 0x3f);
		writeRegister(0x83 + kOperators[index], 0xff);
		writeRegister(0xb0 + index, 0);
	}
}

void KitAdLibSound::setupNote(uint index, byte note) {
	if (index >= 9)
		return;
	Channel &channel = _channels[index];
	channel.rawNote = note;
	int pitch = int8((note & 15) + channel.baseNote);
	int octave = byte((note & 0xf0) + channel.baseOctave);
	while (pitch < 0) { pitch += 12; octave -= 16; }
	while (pitch >= 12) { pitch -= 12; octave += 16; }
	uint16 frequency = word(0x63e + pitch * 2) + channel.baseFrequency;
	uint16 value = frequency | (((octave >> 2) & 0x1c) << 8);
	if (channel.pitchBend) {
		uint table = (note & 15) + (channel.pitchBend > 0 ? 2 : 0);
		uint offset = word(0x11a + table * 2) + ABS(int(channel.pitchBend));
		if (contains(offset, 1))
			value += channel.pitchBend > 0 ? _data[offset] : -_data[offset];
	}
	channel.regA = value;
	channel.regB = (value >> 8) | (channel.regB & 0x20);
	writeRegister(0xa0 + index, channel.regA);
	writeRegister(0xb0 + index, channel.regB);
}

void KitAdLibSound::noteOn(uint index) {
	if (index >= 9)
		return;
	Channel &channel = _channels[index];
	channel.regB |= 0x20;
	writeRegister(0xb0 + index, channel.regB);
	uint frequency = ((channel.regB << 8) | channel.regA) & 0x3ff;
	channel.vibratoStep = (frequency >> (9 - MIN<uint>(channel.vibratoRange, 9))) & 255;
	channel.vibratoWait = channel.vibratoDelay;
}

void KitAdLibSound::noteOff(uint index) {
	if (index >= 9 || (_rhythm && index >= 6))
		return;
	_channels[index].regB &= ~0x20;
	writeRegister(0xb0 + index, _channels[index].regB);
}

bool KitAdLibSound::setupDuration(Channel &channel, byte duration) {
	channel.duration = duration;
	if (channel.durationRandomness)
		channel.duration += getRandomNumber() & channel.durationRandomness;
	else if (channel.fractionalSpacing)
		channel.gateDuration = (duration >> 3) * channel.fractionalSpacing;
	return duration != 0;
}

void KitAdLibSound::setupInstrument(uint index, byte instrument, Channel &channel) {
	uint offset = word(0x1892 + instrument * 2);
	if (index >= 9 || !offset || !contains(offset, 11))
		return;
	const byte *data = &_data[offset];
	byte op = kOperators[index];
	writeRegister(0x20 + op, data[0]);
	writeRegister(0x23 + op, data[1]);
	writeRegister(0xc0 + index, data[2]);
	channel.additive = data[2] & 1;
	writeRegister(0xe0 + op, data[3]);
	writeRegister(0xe3 + op, data[4]);
	channel.opLevel[0] = data[5];
	channel.opLevel[1] = data[6];
	writeRegister(0x40 + op, operatorLevel(channel, 0));
	writeRegister(0x43 + op, operatorLevel(channel, 1));
	writeRegister(0x60 + op, data[7]);
	writeRegister(0x63 + op, data[8]);
	writeRegister(0x80 + op, data[9]);
	writeRegister(0x83 + op, data[10]);
}

byte KitAdLibSound::operatorLevel(const Channel &channel, uint op) const {
	byte level = channel.opLevel[op] & 63;
	if (op || channel.additive)
		level += channel.level1 + channel.level2 + channel.level3;
	if (channel.noteLevelScale[op]) {
		uint shift = channel.noteLevelScale[op] + 1;
		if (shift < 16)
			level -= uint16(channel.noteLevel << shift) >> 8;
	}
	return (channel.opLevel[op] & 0xc0) | (int8(level) < 0 ? 0 : MIN<int>(level, 63));
}

void KitAdLibSound::adjustLevel(uint index) {
	if (index >= 9)
		return;
	Channel &channel = _channels[index];
	writeRegister(0x43 + kOperators[index], operatorLevel(channel, 1));
	writeRegister(0x40 + kOperators[index], operatorLevel(channel, 0));
}

void KitAdLibSound::updateEffects(uint index) {
	if (index >= 9)
		return;
	Channel &channel = _channels[index];
	bool changed = false;
	uint16 frequency = ((channel.regB << 8) | channel.regA) & 0x3ff;
	if (channel.effect == kSlideEffect && advance(channel.slideTimer, channel.slideTempo)) {
		byte octave = channel.regB & 0x1c;
		frequency += channel.slideStep;
		if (channel.slideStep >= 0 && int16(frequency) >= 0x2de) {
			frequency >>= 1;
			octave = (octave + 4) & 0x1c;
		} else if (channel.slideStep < 0 && int16(frequency) <= 0x184) {
			frequency <<= 1;
			octave = (octave - 4) & 0x1c;
		}
		frequency &= 0x3ff;
		channel.regB = (channel.regB & 0x20) | octave;
		changed = true;
	} else if (channel.effect == kVibratoEffect) {
		if (channel.vibratoWait) {
			channel.vibratoWait--;
		} else if (advance(channel.vibratoTimer, channel.vibratoTempo)) {
			if (!--channel.vibratoCount) {
				channel.vibratoStep = -channel.vibratoStep;
				channel.vibratoCount = channel.vibratoSteps;
			}
			frequency += channel.vibratoStep;
			changed = true;
		}
	}
	if (changed) {
		channel.regA = frequency;
		channel.regB = (channel.regB & 0xfc) | (frequency >> 8);
		writeRegister(0xa0 + index, channel.regA);
		writeRegister(0xb0 + index, channel.regB);
	}
	if (channel.secondaryData && advance(channel.secondaryTimer, channel.secondaryTempo)) {
		if (--channel.secondaryPosition < 0)
			channel.secondaryPosition = channel.secondarySize;
		uint offset = channel.secondaryData + byte(channel.secondaryPosition);
		if (contains(offset, 1))
			writeRegister(channel.secondaryRegister + kOperators[index], _data[offset]);
	}
}

void KitAdLibSound::setupRhythm(Channel &channel, const byte *values) {
	for (uint i = 0; i < 3; i++) {
		setupInstrument(6 + i, values[i], channel);
		if (i == 0)
			_rhythmLevel[4] = channel.opLevel[1];
		else if (i == 1) {
			_rhythmLevel[0] = channel.opLevel[0];
			_rhythmLevel[3] = channel.opLevel[1];
		} else {
			_rhythmLevel[2] = channel.opLevel[0];
			_rhythmLevel[1] = channel.opLevel[1];
		}
	}
	for (uint i = 0; i < 3; i++) {
		_channels[6 + i].regB = values[3 + i * 2] & 0x2f;
		writeRegister(0xb6 + i, _channels[6 + i].regB);
		writeRegister(0xa6 + i, values[4 + i * 2]);
	}
	_rhythm = 0x20;
}

void KitAdLibSound::rhythmLevel(byte command, byte mask, byte value) {
	static const byte registers[5] = {0x51, 0x55, 0x52, 0x54, 0x53};
	for (uint i = 0; i < 5; i++) {
		if (!(mask & (1 << i)))
			continue;
		byte level = value + _rhythmLevel[i] + _rhythmLevel2[i];
		if (command == kRhythmLevel2) {
			_rhythmLevel2[i] = value;
			level = value + _rhythmLevel[i] + _rhythmLevel1[i] + value;
		} else if (command == kChangeRhythmLevel1) {
			level += _rhythmLevel1[i];
			_rhythmLevel1[i] = MIN<int>(level, 63);
		} else {
			_rhythmLevel1[i] = value;
		}
		writeRegister(registers[i], MIN<int>(level, 63));
	}
}

void KitAdLibSound::writeVolume(uint index) {
	byte op = kOperators[index];
	for (uint i = 0; i < 2; i++) {
		byte reg = 0x40 + op + i * 3;
		byte value = _registers[reg];
		if (i || (_registers[0xc0 + index] & 1) || ((_registers[0xbd] & 0x20) && index >= 7))
			value = (value & 0xc0) | (63 - (63 - (value & 63)) * _volume / 255);
		_opl->writeReg(reg, value);
	}
}

void KitAdLibSound::writeRegister(byte reg, byte value) {
	_registers[reg] = value;
	if (reg >= 0x40 && reg <= 0x55) {
		for (uint i = 0; i < 9; i++) {
			if (reg == 0x40 + kOperators[i] || reg == 0x43 + kOperators[i]) {
				writeVolume(i);
				return;
			}
		}
	}
	_opl->writeReg(reg, value);
	if (reg >= 0xc0 && reg <= 0xc8)
		writeVolume(reg - 0xc0);
	else if (reg == 0xbd)
		for (uint i = 6; i < 9; i++)
			writeVolume(i);
}

void KitAdLibSound::updateVolume() {
	int volume = _mixer->isSoundTypeMuted(Audio::Mixer::kSFXSoundType) ? 0 :
		_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
	if (_volume == volume)
		return;
	_volume = volume;
	for (uint i = 0; i < 9; i++)
		writeVolume(i);
}

uint16 KitAdLibSound::getRandomNumber() {
	_random += 0x9248;
	_random = (_random >> 3) | (_random << 13);
	return _random;
}

Sound *createKitAdLibSound(Audio::Mixer *mixer, const Common::Array<byte> &data) {
	return new KitAdLibSound(mixer, data);
}

} // namespace Freescape
