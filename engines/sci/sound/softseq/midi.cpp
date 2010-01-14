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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"

#include "common/config-manager.h"
#include "sound/fmopl.h"
#include "sound/softsynth/emumidi.h"

#include "sci/resource.h"
#include "sci/sound/softseq/mididriver.h"

namespace Sci {

class MidiPlayer_Midi : public MidiPlayer {
public:
	enum {
		kVoices = 32
	};

	MidiPlayer_Midi();
	virtual ~MidiPlayer_Midi();

	int open(ResourceManager *resMan);
	void send(uint32 b);
	bool hasRhythmChannel() const { return true; }
	byte getPlayId(SciVersion soundVersion);
	int getPolyphony() const { return kVoices; }
	void setVolume(byte volume);
	int getVolume();
	void playSwitch(bool play);

private:
	bool isMt32GmPatch(const byte *data, int size);
	void readMt32GmPatch(const byte *data, int size);

	void noteOn(int channel, int note, int velocity);
	void setPatch(int channel, int patch);
	void controlChange(int channel, int control, int value);

	struct Channel {
		byte mappedPatch;
		byte patch;
		int velocityMapIdx;
		bool playing;
		int8 keyShift;
		int8 volAdjust;
		uint8 pan;
		uint8 hold;
		uint8 volume;

		Channel() : mappedPatch(0xff), patch(0xff), velocityMapIdx(0), playing(false),
			keyShift(0), volAdjust(0), pan(0x80), hold(0), volume(0x7f) { }
	};

	bool _isMt32;
	bool _isHardwareMt32;
	bool _playSwitch;
	int _masterVolume;

	Channel _channels[16];	
	uint8 _percussionMap[128];
	int8 _keyShift[128];
	int8 _volAdjust[128];
	uint8 _patchMap[128];
	uint8 _velocityMapIdx[128];
	uint8 _velocityMap[4][128];
};

MidiPlayer_Midi::MidiPlayer_Midi() : _playSwitch(true), _masterVolume(15), _isMt32(false), _isHardwareMt32(false) {
	MidiDriverType midiType = MidiDriver::detectMusicDriver(MDT_MIDI);
	_driver = createMidi(midiType);

	if (midiType == MD_MT32)
		_isMt32 = true;
	else if ((midiType != MD_FLUIDSYNTH) && (midiType != MD_TIMIDITY) && ConfMan.getBool("native_mt32")) {
		_isMt32 = true;
		_isHardwareMt32 = true;
	}
}

MidiPlayer_Midi::~MidiPlayer_Midi() {
	delete _driver;
}

void MidiPlayer_Midi::noteOn(int channel, int note, int velocity) {
	if (channel == MIDI_RHYTHM_CHANNEL) {
		note = _percussionMap[note];
		if (note == 0xff)
			return;
	} else {
		if (_channels[channel].mappedPatch == 0xff)
			return;

		int8 keyshift = _keyShift[channel];

		int shiftNote = note + keyshift;

		if (keyshift > 0) {
			while (shiftNote > 127)
				shiftNote -= 12;
		} else {
			while (shiftNote < 0)
				shiftNote += 12;
		}

		note = shiftNote;

		// We assume that velocity 0 maps to 0 (for note off)
		int mapIndex = _channels[channel].velocityMapIdx;
		velocity = _velocityMap[mapIndex][velocity];
	}

	_channels[channel].playing = true;
	_driver->send(0x90 | channel, note, velocity);
}

void MidiPlayer_Midi::controlChange(int channel, int control, int value) {
	switch (control) {
	case 0x07:
		_channels[channel].volume = value;

		if (!_playSwitch)
			return;

		value += _channels[channel].volAdjust;

		if (value > 0x7f)
			value = 0x7f;

		if (value < 0)
			value = 1;

		value *= _masterVolume;

		if (value != 0) {
			value /= 15;

			if (value == 0)
				value = 1;
		}
		break;
	case 0x0a:
		if (_channels[channel].pan == value)
			return;

		_channels[channel].pan = value;
		break;
	case 0x40:
		if (_channels[channel].hold == value)
			return;

		_channels[channel].hold = value;
		break;
	case 0x7b:
		if (!_channels[channel].playing)
			return;

		_channels[channel].playing = false;
	}

	_driver->send(0xb0 | channel, control, value);
}

void MidiPlayer_Midi::setPatch(int channel, int patch) {
	bool resetVol = false;

	if ((channel == MIDI_RHYTHM_CHANNEL) || (_channels[channel].patch == patch))
		return;

	_channels[channel].patch = patch;
	_channels[channel].velocityMapIdx = _velocityMapIdx[patch];

	if (_channels[channel].mappedPatch == 0xff)
		resetVol = true;

	_channels[channel].mappedPatch = _patchMap[patch];

	if (_patchMap[patch] == 0xff) {
		_driver->send(0xb0 | channel, 0x7b, 0);
		_driver->send(0xb0 | channel, 0x40, 0);
		return;
	}

	if (_channels[channel].keyShift != _keyShift[patch]) {
		_channels[channel].keyShift = _keyShift[patch];
		_driver->send(0xb0 | channel, 0x7b, 0);
		_driver->send(0xb0 | channel, 0x40, 0);
		resetVol = true;
	}

	if (resetVol || (_channels[channel].volAdjust != _volAdjust[patch])) {
		_channels[channel].volAdjust = _volAdjust[patch];
		controlChange(channel, 0x07, _channels[channel].volume);
	}

	_driver->send(0xc0 | channel, _patchMap[patch], 0);
}

void MidiPlayer_Midi::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0x7f;
	byte op2 = (b >> 16) & 0x7f;

	switch (command) {
	case 0x80:
		noteOn(channel, op1, 0);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		controlChange(channel, op1, op2);
		break;
	case 0xc0:
		setPatch(channel, op1);
		break;
	case 0xe0:
		_driver->send(b);
		break;
	default:
		warning("Ignoring MIDI event %02x", command);
	}
}

void MidiPlayer_Midi::setVolume(byte volume) {
	_masterVolume = volume;

	if (!_playSwitch)
		return;

	for (uint i = 1; i < 10; i++) {
		if (_channels[i].volume != 0xff)
			controlChange(i, 0x07, _channels[i].volume & 0x7f);
	}
}

int MidiPlayer_Midi::getVolume() {
	return _masterVolume;
}

void MidiPlayer_Midi::playSwitch(bool play) {
	_playSwitch = play;
	if (play)
		setVolume(_masterVolume);
	else {
		for (uint i = 1; i < 10; i++)
			_driver->send(0xb0 | i, 7, 0);
	}
}

bool MidiPlayer_Midi::isMt32GmPatch(const byte *data, int size)
{
	if (size < 1155)
		return false;
	if (size > 16889)
		return true;

	bool isMt32 = false;
	bool isMt32Gm = false;

	if (READ_LE_UINT16(data + 1153) + 1155 == size)
		isMt32Gm = true;

	int pos = 492 + 246 * data[491];

	if ((size >= (pos + 386)) && (READ_BE_UINT16(data + pos) == 0xabcd))
		pos += 386;

	if ((size >= (pos + 267)) && (READ_BE_UINT16(data + pos) == 0xdcba))
		pos += 267;

	if (size == pos)
		isMt32 = true;

	if (isMt32 == isMt32Gm)
		error("Failed to detect MT-32 patch format");

	return isMt32Gm;
}

void MidiPlayer_Midi::readMt32GmPatch(const byte *data, int size) {
	memcpy(_patchMap, data, 0x80);
	memcpy(_keyShift, data + 0x80, 0x80);
	memcpy(_volAdjust, data + 0x100, 0x80);
	memcpy(_percussionMap, data + 0x180, 0x80);
	memcpy(_velocityMapIdx, data + 0x201, 0x80);
	memcpy(_velocityMap, data + 0x281, 0x200);

	uint16 midiSize = READ_LE_UINT16(data + 0x481);

	if (midiSize > 0) {
		if (size < midiSize + 1155)
			error("Failed to read MIDI data");

		const byte *midi = data + 1155;
		byte command = 0;
		uint i = 0;

		while (i < midiSize) {
			byte op1, op2;

			if (midi[i] & 0x80)
				command = midi[i++];

			switch (command & 0xf0) {
			case 0xf0: {
				byte *sysExEnd = (byte *)memchr(midi + i, 0xf7, midiSize - i);

				if (!sysExEnd)
					error("Failed to find end of sysEx");

				int len = sysExEnd - (midi + i);
				_driver->sysEx(midi + i, len);
				if (_isHardwareMt32)
					g_system->delayMillis(100);

				i += len + 1; // One more for the 0x7f
				break;
			}
			case 0x80:
			case 0x90:
			case 0xa0:
			case 0xb0:
			case 0xe0:
				if (i + 1 >= midiSize)
					error("MIDI command exceeds data size");

				op1 = midi[i++];
				op2 = midi[i++];
				_driver->send(command, op1, op2);
				break;
			case 0xc0:
			case 0xd0:
				if (i >= midiSize)
					error("MIDI command exceeds data size");

				op1 = midi[i++];
				_driver->send(command, op1, 0);
				break;
			default:
				error("Failed to find MIDI command byte");
			}
		}
	}
}

int MidiPlayer_Midi::open(ResourceManager *resMan) {
	assert(resMan != NULL);

	int retval = _driver->open();
	if (retval != 0) {
		warning("Failed to open MIDI driver");
		return retval;
	}

	Resource *res = NULL;

	if (!_isMt32) {
		res = resMan->findResource(ResourceId(kResourceTypePatch, 4), 0);
		if (!res)
			warning("Failed to locate GM patch, attempting to load MT-32 patch");
	}

	if (!res) {
		res = resMan->findResource(ResourceId(kResourceTypePatch, 1), 0);
		if (!res)
			error("Failed to load MT-32 patch");
	}

	if (isMt32GmPatch(res->data, res->size)) {
		readMt32GmPatch(res->data, res->size);
	} else {
		// TODO
		warning("Old MT-32 patch format currently not supported");
		for (uint i = 0; i < 127; i++) {
			_percussionMap[i] = i;
			_patchMap[i] = i;
			for (uint j = 0; j < 4; j++)
				_velocityMap[j][i] = i;
			_keyShift[i] = 0;
			_volAdjust[i] = 0;
			_velocityMapIdx[i] = 0;
		}
	}

	return 0;
}

byte MidiPlayer_Midi::getPlayId(SciVersion soundVersion) {
	switch (soundVersion) {
	case SCI_VERSION_0_EARLY:
		return 0x00; // TODO
	case SCI_VERSION_0_LATE:
		return 0x01;
	default:
		if (_isMt32)
			return 0x0c;
		else
			return 0x07;
	}
}

MidiPlayer *MidiPlayer_Midi_create() {
	return new MidiPlayer_Midi();
}

} // End of namespace Sci
