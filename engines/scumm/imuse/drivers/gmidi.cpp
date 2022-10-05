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

#include "common/debug.h"
#include "common/system.h"
#include "scumm/imuse/drivers/gmidi.h"


// This makes older titles play new system style, with 16 virtual channels and
// dynamic allocation (instead of playing on fixed channels).
//#define FORCE_NEWSTYLE_CHANNEL_ALLOCATION

namespace Scumm {

class IMuseChannel_GMidi : public MidiChannel {
public:
	IMuseChannel_GMidi(IMuseDriver_GMidi *drv, int number);
	~IMuseChannel_GMidi() override {}

	MidiDriver *device() override { return _drv; }
	byte getNumber() override {	return _number; }

	bool allocate();
	void release() override { _allocated = false; }

	void send(uint32 b) override { if (_drv) _drv->send((b & ~0x0F) | _number); }

	// Regular messages
	void noteOff(byte note) override;
	void noteOn(byte note, byte velocity) override;
	void controlChange(byte control, byte value) override;
	void programChange(byte program) override;
	void pitchBend(int16 bend) override;

	// Control Change and SCUMM specific functions
	void pitchBendFactor(byte value) override { _pitchBendSensitivity = value; }
	void transpose(int8 value) override { _transpose = value; }
	void detune(byte value) override { _detune = value; }
	void priority(byte value) override { _prio = value; }
	void sustain(bool value) override;
	void allNotesOff() override;
	void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override {}

private:
	void noteOffIntern(byte note);
	void noteOnIntern(byte note, byte velocity);

	void setNotePlaying(byte note) { _drv->setNoteFlag(_number, note); }
	void clearNotePlaying(byte note) { _drv->clearNoteFlag(_number, note); }
	bool isNotePlaying(byte note) const { return _drv->queryNoteFlag(_number, note); }
	void setNoteSustained(byte note) { _drv->setSustainFlag(_number, note); }
	void clearNoteSustained(byte note) { _drv->clearSustainFlag(_number, note); }
	bool isNoteSustained(byte note) const { return _drv->querySustainFlag(_number, note); }

	IMuseDriver_GMidi *_drv;
	const byte _number;
	const bool _newSystem;
	bool _allocated;

	byte _polyphony;
	byte _channelUsage;
	bool _exhaust;
	byte _prio;
	int8 _detune;
	int8 _transpose;
	byte _pitchBendSensitivity;
	int16 _pitchBend;
	bool _sustain;

	GMidiControlChan *&_idleChain;
	GMidiControlChan *&_activeChain;
};

class GMidiControlChan {
public:
	GMidiControlChan() : _prev(nullptr), _next(nullptr), _in(nullptr), _number(0), _note(0) {}
	GMidiControlChan *_prev;
	GMidiControlChan *_next;
	IMuseChannel_GMidi *_in;
	byte _number;
	byte _note;
};

void connect(GMidiControlChan *&chain, GMidiControlChan *node) {
	if (!node || node->_prev || node->_next)
		return;
	if ((node->_next = chain))
		chain->_prev = node;
	chain = node;
}

void disconnect(GMidiControlChan *&chain, GMidiControlChan *node) {
	if (!node || !chain)
		return;

	const GMidiControlChan *ch = chain;
	while (ch && ch != node)
		ch = ch->_next;
	if (!ch)
		return;

	if (node->_next)
		node->_next->_prev = node->_prev;

	if (node->_prev)
		node->_prev->_next = node->_next;
	else
		chain = node->_next;

	node->_next = node->_prev = nullptr;
}

#define sendMidi(stat, par1, par2)	_drv->send(((par2) << 16) | ((par1) << 8) | (stat))

IMuseChannel_GMidi::IMuseChannel_GMidi(IMuseDriver_GMidi *drv, int number) :MidiChannel(), _drv(drv), _number(number), _allocated(false), _sustain(false),
	_pitchBend(0x2000), _polyphony(1), _channelUsage(0), _exhaust(false), _prio(0), _detune(0), _transpose(0),
	_pitchBendSensitivity(2), _idleChain(_drv->_idleChain), _activeChain(_drv->_activeChain), _newSystem(_drv->_newSystem) {
	assert(_drv);
}

bool IMuseChannel_GMidi::allocate() {
	if (_allocated)
		return false;

	if (!_newSystem) {
		_prio = 0x80;
	}

	return (_allocated = true);
}

void IMuseChannel_GMidi::noteOff(byte note)  {
	if (_newSystem) {
		if (!isNotePlaying(note))
			return;

		clearNotePlaying(note);
		if (_sustain) {
			setNoteSustained(note);
			return;
		}
	}

#ifdef FORCE_NEWSTYLE_CHANNEL_ALLOCATION
	noteOffIntern(note);
#else
	if (_newSystem)
		noteOffIntern(note);
	else
		sendMidi(0x80 | _number, note, 0x40);
#endif
}

void IMuseChannel_GMidi::noteOn(byte note, byte velocity)  {
	if (_newSystem) {
		if (isNotePlaying(note)) {
			noteOffIntern(note);
		} else if (isNoteSustained(note)) {
			setNotePlaying(note);
			clearNoteSustained(note);
			noteOffIntern(note);
		} else {
			setNotePlaying(note);
		}
	}

#ifdef FORCE_NEWSTYLE_CHANNEL_ALLOCATION
	noteOnIntern(note, velocity);
#else
	if (_newSystem)
		noteOnIntern(note, velocity);
	else 
		sendMidi(0x90 | _number, note, velocity);
#endif
}

void IMuseChannel_GMidi::controlChange(byte control, byte value)  {
	switch (control) {
	case 1:
	case 7:
	case 10:
	case 91:
	case 93:
		sendMidi(0xB0 | _number, control, value);
		break;
	case 17:
		if (_newSystem)
			_polyphony = value;
		else
			detune(value);
		break;
	case 18:
		priority(value);
		break;
	case 123:
		allNotesOff();
		break;
	default:
		// The original driver does not pass through "blindly". The
		// only controls that get sent are 1, 7, 10, 91 and 93.
		warning("Unhandled Control: %d", control);
		break;
	}
}

void IMuseChannel_GMidi::programChange(byte program)  {
	sendMidi(0xC0 | _number, program, 0);
}

void IMuseChannel_GMidi::pitchBend(int16 bend)  {
	bend = bend + 0x2000;
	sendMidi(0xE0 | _number, _pitchBend & 0x7F, (_pitchBend >> 7) & 0x7F);
}

void IMuseChannel_GMidi::sustain(bool value) {
	_sustain = value;

	if (_newSystem) {
		// For SAMNMAX, this is fully software controlled. No control change message gets sent.
		if (_sustain)
			return;

		for (int i = 0; i < 128; ++i) {
			if (isNoteSustained(i))
				noteOffIntern(i);
		}

	} else {
		sendMidi(0xB0 | _number, 0x40, value);
	}
}

void IMuseChannel_GMidi::allNotesOff() {
	if (_newSystem) {
		// For SAMNMAX, this is fully software controlled. No control change message gets sent.
		if (_sustain)
			return;

		for (int i = 0; i < 128; ++i) {
			if (isNotePlaying(i)) {
				noteOffIntern(i);
				clearNotePlaying(i);
			} else if (isNoteSustained(i)) {
				noteOffIntern(i);
				clearNoteSustained(i);
			}
		}

	} else {
		sendMidi(0xB0 | _number, 0x7B, 0);
	}
}

void IMuseChannel_GMidi::noteOffIntern(byte note) {
	if (_activeChain == nullptr)
		return;

	GMidiControlChan *node = nullptr;
	for (GMidiControlChan *i = _activeChain; i; i = i->_next) {
		if (i->_number == _number && i->_note == note) {
			node = i;
			break;
		}
	}

	if (!node)
		return;

	sendMidi(0x80 | _number, note, 0x40);

	if (_newSystem)
		_exhaust = (--_channelUsage > _polyphony);

	disconnect(_activeChain, node);
	connect(_idleChain, node);
}

void IMuseChannel_GMidi::noteOnIntern(byte note, byte velocity) {
	GMidiControlChan *node = nullptr;

	if (_idleChain) {
		node = _idleChain;
		disconnect(_idleChain, node);
	} else {
		IMuseChannel_GMidi *best = this;
		for (GMidiControlChan *i = _activeChain; i; i = i->_next) {
			assert (i->_in);
			if ((best->_exhaust == i->_in->_exhaust && best->_prio >= i->_in->_prio) || (!best->_exhaust && i->_in->_exhaust)) {
				best = i->_in;
				node = i;
			}
		}

		if (!node)
			return;

		IMuseChannel_GMidi *prt = _drv->_imsParts[node->_number];
		if (prt) {
			sendMidi(0x80 | prt->_number, node->_note, 0x40);
			if (_newSystem)
				prt->_exhaust = (--prt->_channelUsage > prt->_polyphony);
		}

		disconnect(_activeChain, node);
	}

	assert(node);
	node->_in = this;
	node->_number = _number;
	node->_note = note;

	connect(_activeChain, node);

	if (_newSystem)
		_exhaust = (++_channelUsage > _polyphony);

	sendMidi(0x90 | _number, note, velocity);
}

#undef sendMidi

IMuseDriver_GMidi::IMuseDriver_GMidi(MidiDriver::DeviceHandle dev, bool rolandGSMode, bool newSystem) : MidiDriver(), _drv(nullptr), _gsMode(rolandGSMode), _imsParts(nullptr),
	_newSystem(newSystem), _numChannels(16), _notesPlaying(nullptr), _notesSustained(nullptr),  _controlChan(nullptr), _idleChain(nullptr), _activeChain(nullptr) {
	_drv = MidiDriver::createMidi(dev);
	assert(_drv);
}

IMuseDriver_GMidi::~IMuseDriver_GMidi() {
	close();
	delete _drv;
}

int IMuseDriver_GMidi::open() {
	if (!_drv)
		return MERR_CANNOT_CONNECT;

	int res = _drv->open();
	if (res)
		return res;

	createChannels();

	if (_gsMode)
		initDeviceAsRolandGS();
	else
		initDevice();	

	return res;
}

void IMuseDriver_GMidi::close() {
	if (isOpen() && _drv) {
		for (int i = 0; i < 16; ++i) {
			send(0x0040B0 | i);
			send(0x007BB0 | i);
		}
		_drv->close();
	}

	releaseChannels();
}

MidiChannel *IMuseDriver_GMidi::allocateChannel() {
	if (!isOpen())
		return nullptr;

	for (int i = 0; i < _numChannels; ++i) {
		IMuseChannel_GMidi *ch = _imsParts[i];
		if (ch && i != 9 && ch->allocate())
			return ch;
	}

	return nullptr;
}

MidiChannel *IMuseDriver_GMidi::getPercussionChannel() {
	if (!isOpen())
		return nullptr;

	return _imsParts[9];
}

void IMuseDriver_GMidi::initDevice() {
	// These are the init messages from the DOTT General Midi driver. This is the major part of the bug fix for bug
	// no. 13460 ("DOTT: Incorrect MIDI pitch bending"). SAMNMAX has some less of the default settings (since
	// the driver works a bit different), but it uses the same values for the pitch bend range.
	for (int i = 0; i < 16; ++i) {
		send(0x0064B0 | i);
		send(0x0065B0 | i);
		send(0x1006B0 | i);
		send(0x7F07B0 | i);
		send(0x3F0AB0 | i);
		send(0x0000C0 | i);
		send(0x4000E0 | i);
		send(0x0001B0 | i);
		send(0x0040B0 | i);
		send(0x405BB0 | i);
		send(0x005DB0 | i);
		send(0x0000B0 | i);
		send(0x007BB0 | i);
	}
}

void IMuseDriver_GMidi::initDeviceAsRolandGS() {
	byte buffer[12];
	int i;

	// General MIDI System On message
	// Resets all GM devices to default settings
	memcpy(&buffer[0], "\x7E\x7F\x09\x01", 4);
	sysEx(buffer, 4);
	debug(2, "GM SysEx: GM System On");
	g_system->delayMillis(200);

	// All GS devices recognize the GS Reset command,
	// even using Roland's ID. It is impractical to
	// support other manufacturers' devices for
	// further GS settings, as there are limitless
	// numbers of them out there that would each
	// require individual SysEx commands with unique IDs.

	// Roland GS SysEx ID
	memcpy(&buffer[0], "\x41\x10\x42\x12", 4);

	// GS Reset
	memcpy(&buffer[4], "\x40\x00\x7F\x00\x41", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: GS Reset");
	g_system->delayMillis(200);

	// Set global Master Tune to 442.0kHz, as on the MT-32
	memcpy(&buffer[4], "\x40\x00\x00\x00\x04\x04\x0F\x29", 8);
	sysEx(buffer, 12);
	debug(2, "GS SysEx: Master Tune set to 442.0kHz");

	// Note: All Roland GS devices support CM-64/32L maps

	// Set Channels 1-16 to SC-55 Map, then CM-64/32L Variation
	for (i = 0; i < 16; ++i) {
		_drv->send((127 << 16) | (0  << 8) | (0xB0 | i));
		_drv->send((1   << 16) | (32 << 8) | (0xB0 | i));
		_drv->send((0   << 16) | (0  << 8) | (0xC0 | i));
	}
	debug(2, "GS Program Change: CM-64/32L Map Selected");

	// Set Percussion Channel to SC-55 Map (CC#32, 01H), then
	// Switch Drum Map to CM-64/32L (MT-32 Compatible Drums)
	getPercussionChannel()->controlChange(0, 0);
	getPercussionChannel()->controlChange(32, 1);
	send(127 << 8 | 0xC0 | 9);
	debug(2, "GS Program Change: Drum Map is CM-64/32L");

	// Set Master Chorus to 0. The MT-32 has no chorus capability.
	memcpy(&buffer[4], "\x40\x01\x3A\x00\x05", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Master Chorus Level is 0");

	// Set Channels 1-16 Reverb to 64, which is the
	// equivalent of MT-32 default Reverb Level 5
	for (i = 0; i < 16; ++i)
		send((64 << 16) | (91 << 8) | (0xB0 | i));
	debug(2, "GM Controller 91 Change: Channels 1-16 Reverb Level is 64");

	// Set Channels 1-16 Pitch Bend Sensitivity to
	// 12 semitones; then lock the RPN by setting null.
	for (i = 0; i < 16; ++i)
		setPitchBendRange(i, 12);
	debug(2, "GM Controller 6 Change: Channels 1-16 Pitch Bend Sensitivity is 12 semitones");

	// Set channels 1-16 Mod. LFO1 Pitch Depth to 4
	memcpy(&buffer[4], "\x40\x20\x04\x04\x18", 5);
	for (i = 0; i < 16; ++i) {
		buffer[5] = 0x20 + i;
		buffer[8] = 0x18 - i;
		sysEx(buffer, 9);
	}

	debug(2, "GS SysEx: Channels 1-16 Mod. LFO1 Pitch Depth Level is 4");

	// Set Percussion Channel Expression to 80
	getPercussionChannel()->controlChange(11, 80);
	debug(2, "GM Controller 11 Change: Percussion Channel Expression Level is 80");

	// Turn off Percussion Channel Rx. Expression so that
	// Expression cannot be modified. I don't know why, but
	// Roland does it this way.
	memcpy(&buffer[4], "\x40\x10\x0E\x00\x22", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Percussion Channel Rx. Expression is OFF");

	// Change Reverb Character to 0. I don't think this
	// sounds most like MT-32, but apparently Roland does.
	memcpy(&buffer[4], "\x40\x01\x31\x00\x0E", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Reverb Character is 0");

	// Change Reverb Pre-LF to 4, which is similar to
	// what MT-32 reverb does.
	memcpy(&buffer[4], "\x40\x01\x32\x04\x09", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Reverb Pre-LF is 4");

	// Change Reverb Time to 106; the decay on Hall 2
	// Reverb is too fast compared to the MT-32's
	memcpy(&buffer[4], "\x40\x01\x34\x6A\x21", 5);
	sysEx(buffer, 9);
	debug(2, "GS SysEx: Reverb Time is 106");
}

void IMuseDriver_GMidi::createChannels() {
	releaseChannels();

	_imsParts = new IMuseChannel_GMidi*[_numChannels];
	_controlChan = new GMidiControlChan*[12];

	assert(_imsParts);
	assert(_controlChan);

	for (int i = 0; i < _numChannels; ++i)
		_imsParts[i] = new IMuseChannel_GMidi(this, i);

	for (int i = 0; i < 12; ++i) {
		_controlChan[i] = new GMidiControlChan();
		connect(_idleChain, _controlChan[i]);
	}

	if (_newSystem) {
		_notesPlaying = new uint16[128]();
		_notesSustained = new uint16[128]();
	}
}

void IMuseDriver_GMidi::releaseChannels() {
	if (_imsParts) {
		for (int i = 0; i < _numChannels; ++i)
			delete _imsParts[i];
		delete[] _imsParts;
		_imsParts = nullptr;
	}

	if (_controlChan) {
		for (int i = 0; i < 12; ++i)
			delete _controlChan[i];
		delete[] _controlChan;
		_controlChan = nullptr;
	}

	delete[] _notesPlaying;
	_notesPlaying = nullptr;
	delete[] _notesSustained;
	_notesSustained = nullptr;
}

#undef FORCE_NEWSTYLE_CHANNEL_ALLOCATION

} // End of namespace Scumm
