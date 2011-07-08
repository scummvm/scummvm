/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "scumm/imuse/pcspk.h"

#include "common/debug.h"
#include "common/textconsole.h"

namespace Scumm {

PcSpkDriver::PcSpkDriver(Audio::Mixer *mixer)
	: MidiDriver_Emulated(mixer), _pcSpk(mixer->getOutputRate()) {
}

PcSpkDriver::~PcSpkDriver() {
	close();
}

int PcSpkDriver::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	MidiDriver_Emulated::open();

	for (uint i = 0; i < 6; ++i)
		_channels[i].init(this, i);
	_activeChannel = 0;

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);
	return 0;
}

void PcSpkDriver::close() {
	if (!_isOpen)
		return;
	_isOpen = false;

	_mixer->stopHandle(_mixerSoundHandle);
}

void PcSpkDriver::send(uint32 d) {
	assert((d & 0x0F) < 6);
	debug("send");
	_channels[(d & 0x0F)].send(d);
}

void PcSpkDriver::sysEx_customInstrument(byte channel, uint32 type, const byte *instr) {
	assert(channel < 6);
	if (type == 'SPK ')
		_channels[channel].sysEx_customInstrument(type, instr);
}

MidiChannel *PcSpkDriver::allocateChannel() {
	for (uint i = 0; i < 6; ++i) {
		if (_channels[i].allocate())
			return &_channels[i];
	}

	return 0;
}

void PcSpkDriver::generateSamples(int16 *buf, int len) {
	_pcSpk.readBuffer(buf, len);
}

void PcSpkDriver::onTimer() {
	if (!_activeChannel)
		return;

	for (uint i = 0; i < 6; ++i) {
		OutputChannel &out = _channels[i]._out;

		if (!out.active)
			continue;

		if (out.length == 0 || --out.length != 0) {
			if (out.unkB && out.unkC) {
				// TODO
			}
		} else {
			out.active = 0;
			updateNote();
			return;
		}
	}

	if (_activeChannel->_tl) {
		//output((_activeChannel->_out.note << 7) + _activeChannel->_pitchBend + _activeChannel->_out.unk60 + _activeChannel->_out.unkE);
	} else {
		_pcSpk.stop();
	}
}

void PcSpkDriver::updateNote() {
	uint8 priority = 0;
	_activeChannel = 0;
	for (uint i = 0; i < 6; ++i) {
		if (_channels[i]._allocated && _channels[i]._out.active && _channels[i]._priority >= priority) {
			priority = _channels[i]._priority;
			_activeChannel = &_channels[i];
		}
	}

	if (_activeChannel == 0 || _activeChannel->_tl == 0) {
		_pcSpk.stop();
	} else {
		output(_activeChannel->_pitchBend + (_activeChannel->_out.note << 7));
	}
}

void PcSpkDriver::output(uint16 out) {
	byte v1 = (out >> 7) & 0xFF;
	byte v2 = (out >> 2) & 0x1E;

	byte shift = _outputTable1[v1];
	uint16 indexBase = _outputTable2[v1] << 5;
	uint16 frequency = _frequencyTable[(indexBase + v2) / 2] >> shift;
	_pcSpk.play(Audio::PCSpeaker::kWaveFormSquare, 1193180 / frequency, -1);
}

void PcSpkDriver::MidiChannel_PcSpk::init(PcSpkDriver *owner, byte channel) {
	_owner = owner;
	_channel = channel;
	_allocated = false;
	memset(&_out, 0, sizeof(_out));
}

bool PcSpkDriver::MidiChannel_PcSpk::allocate() {
	if (_allocated)
		return false;

	memset(&_out, 0, sizeof(_out));
	memset(_instrument, 0, sizeof(_instrument));
	_out.effectDefA.envelope = &_out.effectEnvelopeA;
	_out.effectDefB.envelope = &_out.effectEnvelopeB;

	_allocated = true;
	return true;
}

MidiDriver *PcSpkDriver::MidiChannel_PcSpk::device() {
	return _owner;
}

byte PcSpkDriver::MidiChannel_PcSpk::getNumber() {
	return _channel;
}

void PcSpkDriver::MidiChannel_PcSpk::release() {
	_out.active = 0;
	_allocated = false;
	_owner->updateNote();
}

void PcSpkDriver::MidiChannel_PcSpk::send(uint32 b) {
	uint8 type = b & 0xF0;
	uint8 p1 = (b >> 8) & 0xFF;
	uint8 p2 = (b >> 16) & 0xFF;

	switch (type) {
	case 0x80:
		noteOff(p1);
		break;

	case 0x90:
		if (p2)
			noteOn(p1, p2);
		else
			noteOff(p1);
		break;

	case 0xB0:
		controlChange(p1, p2);
		break;

	case 0xE0:
		pitchBend((p1 | (p2 << 7)) - 0x2000);
		break;

	default:
		break;
	}
}

void PcSpkDriver::MidiChannel_PcSpk::noteOff(byte note) {
	if (!_allocated)
		return;

	if (_sustain) {
		if (_out.note == note)
			_out.sustainNoteOff = 1;
	} else {
		if (_out.note == note) {
			_out.active = 0;
			_owner->updateNote();
		}
	}
}

void PcSpkDriver::MidiChannel_PcSpk::noteOn(byte note, byte velocity) {
	if (!_allocated)
		return;

	_out.note = note;
	_out.sustainNoteOff = 0;
	_out.length = _instrument[0];
	//_out.instrument = _owner->_outInstrumentData + note;
	_out.unkA = 0;
	_out.unkB = _instrument[1];
	_out.unkC = _instrument[2];
	_out.unkE = 0;
	_out.unk60 = 0;
	_out.active = 1;

	_owner->updateNote();

	if ((_instrument[5] & 0x80) != 0) {
		warning("Effect envelope 1 used");
	}

	if ((_instrument[14] & 0x80) != 0) {
		warning("Effect envelope 2 used");
	}
}

void PcSpkDriver::MidiChannel_PcSpk::programChange(byte program) {
	// Nothing to implement here, the iMuse code takes care of passing us the
	// instrument data.
}

void PcSpkDriver::MidiChannel_PcSpk::pitchBend(int16 bend) {
	_pitchBend = (bend * _pitchBendFactor) >> 6;
}

void PcSpkDriver::MidiChannel_PcSpk::controlChange(byte control, byte value) {
	switch (control) {
	case 1:
		if (_out.effectEnvelopeA.state && _out.effectDefA.enabled)
			_out.effectEnvelopeA.modWheelState = (value >> 2);
		if (_out.effectEnvelopeB.state && _out.effectDefB.enabled)
			_out.effectEnvelopeB.modWheelState = (value >> 2);
		break;

	case 7:
		_tl = value;
		// TODO: Properly implement this
		_owner->updateNote();
		break;

	case 64:
		_sustain = value;
		if (!value && _out.sustainNoteOff) {
			_out.active = 0;
			_owner->updateNote();
		}
		break;

	case 123:
		_out.active = 0;
		_owner->updateNote();
		break;

	default:
		break;
	}
}

void PcSpkDriver::MidiChannel_PcSpk::pitchBendFactor(byte value) {
	_pitchBendFactor = value;
}

void PcSpkDriver::MidiChannel_PcSpk::priority(byte value) {
	_priority = value;
}

void PcSpkDriver::MidiChannel_PcSpk::sysEx_customInstrument(uint32 type, const byte *instr) {
	memcpy(_instrument, instr, sizeof(_instrument));
}

/*const byte PcSpkDriver::_outInstrumentData[] = {
	0
};*/

const byte PcSpkDriver::_outputTable1[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 3, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7
};

const byte PcSpkDriver::_outputTable2[] = {
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7,
	8,  9, 10, 11,
	0,  1,  2,  3,
	4,  5,  6,  7
};

const uint16 PcSpkDriver::_frequencyTable[] = {
	0x8E84, 0x8E00, 0x8D7D, 0x8CFA,
	0x8C78, 0x8BF7, 0x8B76, 0x8AF5,
	0x8A75, 0x89F5, 0x8976, 0x88F7,
	0x8879, 0x87FB, 0x877D, 0x8700,
	0x8684, 0x8608, 0x858C, 0x8511,
	0x8496, 0x841C, 0x83A2, 0x8328,
	0x82AF, 0x8237, 0x81BF, 0x8147,
	0x80D0, 0x8059, 0x7FE3, 0x7F6D,
	0x7EF7, 0x7E82, 0x7E0D, 0x7D99,
	0x7D25, 0x7CB2, 0x7C3F, 0x7BCC,
	0x7B5A, 0x7AE8, 0x7A77, 0x7A06,
	0x7995, 0x7925, 0x78B5, 0x7846,
	0x77D7, 0x7768, 0x76FA, 0x768C,
	0x761F, 0x75B2, 0x7545, 0x74D9,
	0x746D, 0x7402, 0x7397, 0x732C,
	0x72C2, 0x7258, 0x71EF, 0x7186,
	0x711D, 0x70B5, 0x704D, 0x6FE5,
	0x6F7E, 0x6F17, 0x6EB0, 0x6E4A,
	0x6DE5, 0x6D7F, 0x6D1A, 0x6CB5,
	0x6C51, 0x6BED, 0x6B8A, 0x6B26,
	0x6AC4, 0x6A61, 0x69FF, 0x699D,
	0x693C, 0x68DB, 0x687A, 0x681A,
	0x67BA, 0x675A, 0x66FA, 0x669B,
	0x663D, 0x65DF, 0x6581, 0x6523,
	0x64C6, 0x6469, 0x640C, 0x63B0,
	0x6354, 0x62F8, 0x629D, 0x6242,
	0x61E7, 0x618D, 0x6133, 0x60D9,
	0x6080, 0x6027, 0x5FCE, 0x5F76,
	0x5F1E, 0x5EC6, 0x5E6E, 0x5E17,
	0x5DC1, 0x5D6A, 0x5D14, 0x5CBE,
	0x5C68, 0x5C13, 0x5BBE, 0x5B6A,
	0x5B15, 0x5AC1, 0x5A6E, 0x5A1A,
	0x59C7, 0x5974, 0x5922, 0x58CF,
	0x587D, 0x582C, 0x57DA, 0x5789,
	0x5739, 0x56E8, 0x5698, 0x5648,
	0x55F9, 0x55A9, 0x555A, 0x550B,
	0x54BD, 0x546F, 0x5421, 0x53D3,
	0x5386, 0x5339, 0x52EC, 0x52A0,
	0x5253, 0x5207, 0x51BC, 0x5170,
	0x5125, 0x50DA, 0x5090, 0x5046,
	0x4FFB, 0x4FB2, 0x4F68, 0x4F1F,
	0x4ED6, 0x4E8D, 0x4E45, 0x4DFC,
	0x4DB5, 0x4D6D, 0x4D25, 0x4CDE,
	0x4C97, 0x4C51, 0x4C0A, 0x4BC4,
	0x4B7E, 0x4B39, 0x4AF3, 0x4AAE,
	0x4A69, 0x4A24, 0x49E0, 0x499C,
	0x4958, 0x4914, 0x48D1, 0x488E,
	0x484B, 0x4808, 0x47C6, 0x4783
};

} // End of namespace Scumm

