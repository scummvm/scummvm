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
		kVoices = 32,
		kReverbConfigNr = 11,
		kMaxSysExSize = 264
	};

	MidiPlayer_Midi();
	virtual ~MidiPlayer_Midi();

	int open(ResourceManager *resMan);
	void close();
	void send(uint32 b);
	void sysEx(const byte *msg, uint16 length);
	bool hasRhythmChannel() const { return true; }
	byte getPlayId(SciVersion soundVersion);
	int getPolyphony() const { return kVoices; }
	void setVolume(byte volume);
	int getVolume();
	void setReverb(byte reverb);
	void playSwitch(bool play);

private:
	bool isMt32GmPatch(const byte *data, int size);
	void readMt32GmPatch(const byte *data, int size);
	void readMt32Patch(const byte *data, int size);
	void sendMt32SysEx(const uint32 addr, Common::MemoryReadStream *str, int len, bool noDelay);
	void sendMt32SysEx(const uint32 addr, const byte *buf, int len, bool noDelay);
	void setMt32Volume(byte volume);
	void resetMt32();

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
	bool _isOldPatchFormat;
	bool _hasReverb;
	bool _playSwitch;
	int _masterVolume;

	byte _reverbConfig[kReverbConfigNr][3];
	Channel _channels[16];	
	uint8 _percussionMap[128];
	int8 _keyShift[128];
	int8 _volAdjust[128];
	uint8 _patchMap[128];
	uint8 _velocityMapIdx[128];
	uint8 _velocityMap[4][128];
	byte _goodbyeMsg[20];
	byte _sysExBuf[kMaxSysExSize];
};

MidiPlayer_Midi::MidiPlayer_Midi() : _playSwitch(true), _masterVolume(15), _isMt32(false), _hasReverb(false), _isOldPatchFormat(false) {
	MidiDriverType midiType = MidiDriver::detectMusicDriver(MDT_MIDI);
	_driver = createMidi(midiType);

	if (midiType == MD_MT32 || ConfMan.getBool("native_mt32"))
		_isMt32 = true;

	_sysExBuf[0] = 0x41;
	_sysExBuf[1] = 0x10;
	_sysExBuf[2] = 0x16;
	_sysExBuf[3] = 0x12;
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

void MidiPlayer_Midi::setReverb(byte reverb) {
	_reverb = CLIP<byte>(reverb, 0, kReverbConfigNr - 1);
	if (_hasReverb)
		sendMt32SysEx(0x100001, _reverbConfig[_reverb], 3, true);
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

void MidiPlayer_Midi::sendMt32SysEx(const uint32 addr, Common::MemoryReadStream *str, int len, bool noDelay = false) {
	if (len + 8 > kMaxSysExSize) {
		warning("SysEx message exceed maximum size; ignoring");
		return;
	}

	uint16 chk = 0;

	_sysExBuf[4] = (addr >> 16) & 0xff;
	_sysExBuf[5] = (addr >> 8) & 0xff;
	_sysExBuf[6] = addr & 0xff;

	for (int i = 0; i < len; i++)
		_sysExBuf[7 + i] = str->readByte();

	for (int i = 4; i < 7 + len; i++)
		chk += _sysExBuf[i];

	_sysExBuf[7 + len] = 128 - chk % 128;

	if (noDelay)
		_driver->sysEx(_sysExBuf, len + 8);
	else
		sysEx(_sysExBuf, len + 8);
}

void MidiPlayer_Midi::sendMt32SysEx(const uint32 addr, const byte *buf, int len, bool noDelay = false) {
	Common::MemoryReadStream *str = new Common::MemoryReadStream(buf, len);
	sendMt32SysEx(addr, str, len, noDelay);
	delete str;
}

void MidiPlayer_Midi::readMt32Patch(const byte *data, int size) {
	Common::MemoryReadStream *str = new Common::MemoryReadStream(data, size);

	// Send before-SysEx text
	str->seek(0x14);
	sendMt32SysEx(0x200000, str, 20);

	// Save goodbye message
	str->read(_goodbyeMsg, 20);

	// Skip volume, we leave the MT-32 volume alone
	str->seek(2, SEEK_CUR);

	// Reverb default only used in (roughly) SCI0/SCI01
	_reverb = str->readByte();
	_hasReverb = true;

	// Skip reverb SysEx message
	str->seek(11, SEEK_CUR);

	// Read reverb data
	for (int i = 0; i < kReverbConfigNr; i++) {
		_reverbConfig[i][0] = str->readByte();
		_reverbConfig[i][1] = str->readByte();
		_reverbConfig[i][2] = str->readByte();
	}

	// Patches 1-48
	sendMt32SysEx(0x50000, str, 256);
	sendMt32SysEx(0x50200, str, 128);

	// Timbres
	byte timbresNr = str->readByte();
	for (int i = 0; i < timbresNr; i++)
		sendMt32SysEx(0x80000 + (i << 9), str, 246);

	uint16 flag = str->readUint16BE();

	if (!str->eos() && (flag == 0xabcd)) {
		// Patches 49-96
		sendMt32SysEx(0x50300, str, 256);
		sendMt32SysEx(0x50500, str, 128);
		flag = str->readUint16BE();
	}

	if (!str->eos() && (flag == 0xdcba)) {
		// Rhythm key map
		sendMt32SysEx(0x30110, str, 256);
		// Partial reserve
		sendMt32SysEx(0x100004, str, 9);
	}

	// Send after-SysEx text
	str->seek(0);
	sendMt32SysEx(0x200000, str, 20);

	// Send the mystery SysEx
	sendMt32SysEx(0x52000a, (const byte *)"\x16\x16\x16\x16\x16\x16", 6);

	delete str;
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
				sysEx(midi + i, len);

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

void MidiPlayer_Midi::setMt32Volume(byte volume) {
	sendMt32SysEx(0x100016, &volume, 1);
}

void MidiPlayer_Midi::resetMt32() {
	sendMt32SysEx(0x7f0000, (const byte *)"\x01\x00", 2, true);

	// This seems to require a longer delay than usual
	g_system->delayMillis(150);
}

int MidiPlayer_Midi::open(ResourceManager *resMan) {
	assert(resMan != NULL);

	int retval = _driver->open();
	if (retval != 0) {
		warning("Failed to open MIDI driver");
		return retval;
	}

	if (_isMt32) {
		resetMt32();
		setMt32Volume(80);
	}

	Resource *res = NULL;

	if (!_isMt32) {
		res = resMan->findResource(ResourceId(kResourceTypePatch, 4), 0);
		if (!res)
			warning("Failed to locate GM patch, attempting to load MT-32 patch");

		// Detect the format of patch 1, so that we know what play mask to use
		Resource *resPatch1 = resMan->findResource(ResourceId(kResourceTypePatch, 1), 0);
		if (!resPatch1)
			_isOldPatchFormat = false;
		else
			_isOldPatchFormat = !isMt32GmPatch(resPatch1->data, resPatch1->size);
	}

	if (!res) {
		res = resMan->findResource(ResourceId(kResourceTypePatch, 1), 0);
		if (!res)
			error("Failed to load MT-32 patch");
	}

	if (isMt32GmPatch(res->data, res->size)) {
		readMt32GmPatch(res->data, res->size);
		strncpy((char *)_goodbyeMsg, "      ScummVM       ", 20);
	} else {
		if (!_isMt32) {
			warning("MT-32 to GM translation not yet supported");
		}

		readMt32Patch(res->data, res->size);

		// No mapping
		for (uint i = 0; i < 128; i++) {
			_percussionMap[i] = i;
			_patchMap[i] = i;
			_velocityMap[0][i] = i;
			_keyShift[i] = 0;
			_volAdjust[i] = 0;
			_velocityMapIdx[i] = 0;
		}
	}

	return 0;
}

void MidiPlayer_Midi::close() {
	if (_isMt32) {
		// Send goodbye message
		sendMt32SysEx(0x200000, _goodbyeMsg, 20);
	}

	_driver->close();
}

void MidiPlayer_Midi::sysEx(const byte *msg, uint16 length) {
	_driver->sysEx(msg, length);

	// Wait the time it takes to send the SysEx data
	uint32 delay = (length + 2) * 1000 / 3125;

	// Plus an additional delay for the MT-32 rev00
	if (_isMt32)
		delay += 40;

	g_system->delayMillis(delay);
	g_system->updateScreen();
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
			return _isOldPatchFormat ? 0x0c : 0x07;
	}
}

MidiPlayer *MidiPlayer_Midi_create() {
	return new MidiPlayer_Midi();
}

} // End of namespace Sci
