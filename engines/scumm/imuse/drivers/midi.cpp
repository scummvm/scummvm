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

#include "base/version.h"
#include "common/debug.h"
#include "common/system.h"
#include "scumm/imuse/drivers/midi.h"

// This makes older titles play new system style, with 16 virtual channels and
// dynamic allocation (instead of playing on fixed channels).
//#define FORCE_NEWSTYLE_CHANNEL_ALLOCATION

namespace IMSMidi {
using namespace Scumm;

/*******************************
*		General Midi driver
********************************/

struct ChannelNode;
class IMuseChannel_Midi : public MidiChannel {
public:
	IMuseChannel_Midi(IMuseDriver_GMidi *drv, int number);
	virtual ~IMuseChannel_Midi() override {}

	MidiDriver *device() override { return _drv; }
	byte getNumber() override {	return _number; }

	virtual bool allocate();
	void release() override { _allocated = false; }

	void send(uint32 b) override { if (_drv) _drv->send((b & ~0x0F) | _number); }

	// Regular messages
	void noteOff(byte note) override;
	void noteOn(byte note, byte velocity) override;
	void controlChange(byte control, byte value) override;
	virtual void programChange(byte program) override;
	void pitchBend(int16 bend) override;

	// Control Change and SCUMM specific functions
	void pitchBendFactor(byte value) override { pitchBend(0); _pitchBendSensitivity = value; }
	void transpose(int8 value) override { _transpose = value; pitchBend(_pitchBendTemp); }
	void detune(int16 value) override { _detune = value; pitchBend(_pitchBendTemp); }
	void priority(byte value) override { _prio = value; }
	void sustain(bool value) override;
	void allNotesOff() override;
	virtual void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override {}

	virtual void setOutput(ChannelNode*) {}

protected:
	virtual void sendMidi(byte evt, byte par1, byte par2) {
		if (_drv)
			_drv->send(((par2) << 16) | ((par1) << 8) | ((evt & 0xF0) | _number));
	}

	IMuseDriver_GMidi *_drv;
	const byte _number;
	const bool _newSystem;

	int16 _pitchBend;

	ChannelNode *_dummyNode;

private:
	void noteOffIntern(byte note);
	void noteOnIntern(byte note, byte velocity);
	virtual bool validateTransmission(byte note) const { return true; }

	void setNotePlaying(byte note) { _drv->setNoteFlag(_number, note); }
	void clearNotePlaying(byte note) { _drv->clearNoteFlag(_number, note); }
	bool isNotePlaying(byte note) const { return _drv->queryNoteFlag(_number, note); }
	void setNoteSustained(byte note) { _drv->setSustainFlag(_number, note); }
	void clearNoteSustained(byte note) { _drv->clearSustainFlag(_number, note); }
	bool isNoteSustained(byte note) const { return _drv->querySustainFlag(_number, note); }

	virtual void sendNoteOff(byte note);
	virtual void sendNoteOn(byte note, byte velocity);

	bool _allocated;

	byte _polyphony;
	byte _channelUsage;
	bool _exhaust;
	byte _prio;
	int16 _detune;
	int8 _transpose;
	int16 _pitchBendTemp;
	byte _pitchBendSensitivity;
	bool _sustain;

	ChannelNode *&_idleChain;
	ChannelNode *&_activeChain;
};

struct ChannelNode {
	ChannelNode() : _prev(nullptr), _next(nullptr), _in(nullptr), _number(0), _note(0), _addr(0) {}
	ChannelNode *_prev;
	ChannelNode *_next;
	IMuseChannel_Midi *_in;
	byte _number;
	byte _note;
	// MT-32 only
	uint32 _addr;
};

void connect(ChannelNode *&chain, ChannelNode *node) {
	if (!node || node->_prev || node->_next)
		return;
	if ((node->_next = chain))
		chain->_prev = node;
	chain = node;
}

void disconnect(ChannelNode *&chain, ChannelNode *node) {
	if (!node || !chain)
		return;

	const ChannelNode *ch = chain;
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

IMuseChannel_Midi::IMuseChannel_Midi(IMuseDriver_GMidi *drv, int number) :MidiChannel(), _drv(drv), _number(number), _allocated(false), _sustain(false),
	_pitchBend(0x2000), _polyphony(1), _channelUsage(0), _exhaust(false), _prio(0x80), _detune(0), _transpose(0), _pitchBendTemp(0), _pitchBendSensitivity(2),
	_activeChain(drv ? _drv->_activeChain : _dummyNode), _idleChain(drv ? _drv->_idleChain : _dummyNode), _dummyNode(nullptr), _newSystem(drv ? drv->_newSystem : false) {
	assert(_drv);
}

bool IMuseChannel_Midi::allocate() {
	if (_allocated)
		return false;

	_channelUsage = 0;
	_exhaust = false;

	return (_allocated = true);
}

void IMuseChannel_Midi::noteOff(byte note)  {
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
		sendMidi(0x80, note, 0x40);
#endif
}

void IMuseChannel_Midi::noteOn(byte note, byte velocity)  {
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
		sendMidi(0x90, note, velocity);
#endif
}

void IMuseChannel_Midi::controlChange(byte control, byte value)  {
	switch (control) {
	case 1:
	case 7:
	case 10:
	case 91:
	case 93:
		sendMidi(0xB0, control, value);
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
		// The original SAMNMAX driver does not pass through "blindly". The
		// only controls that get sent are 1, 7, 10, 91 and 93.
		if (_newSystem)
			warning("Unhandled Control: %d", control);
		else
			sendMidi(0xB0, control, value);
		break;
	}
}

void IMuseChannel_Midi::programChange(byte program)  {
	sendMidi(0xC0, program, 0);
}

void IMuseChannel_Midi::pitchBend(int16 bend)  {
	_pitchBendTemp = bend;

	if (_newSystem) {
		// SAMNMAX formula (same for Roland MT-32 and GM)
		bend = (((bend * _pitchBendSensitivity) >> 5) + _detune + (_transpose << 8)) << 1;
	} else {
		// DOTT, INDY4 and MI2 formula (same for Roland MT-32 and GM)
		bend = CLIP<int>(((bend * _pitchBendSensitivity) >> 6) + _detune + (_transpose << 7), -2048, 2047) << 2;
	}

	_pitchBend = bend + 0x2000;
	sendMidi(0xE0, _pitchBend & 0x7F, (_pitchBend >> 7) & 0x7F);
}

void IMuseChannel_Midi::sustain(bool value) {
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
		sendMidi(0xB0, 0x40, value);
	}
}

void IMuseChannel_Midi::allNotesOff() {
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
		sendMidi(0xB0, 0x7B, 0);
	}
}

void IMuseChannel_Midi::noteOffIntern(byte note) {
	if (!_activeChain || !validateTransmission(note))
		return;

	ChannelNode *node = nullptr;
	for (ChannelNode *i = _activeChain; i; i = i->_next) {
		if (i->_number == _number && i->_note == note) {
			node = i;
			break;
		}
	}

	if (!node)
		return;

	sendNoteOff(note);

	if (_newSystem)
		_exhaust = (--_channelUsage > _polyphony);

	disconnect(_activeChain, node);
	connect(_idleChain, node);
}

void IMuseChannel_Midi::noteOnIntern(byte note, byte velocity) {
	if (!validateTransmission(note))
		return;

	ChannelNode *node = nullptr;

	if (_idleChain) {
		node = _idleChain;
		disconnect(_idleChain, node);
	} else {
		IMuseChannel_Midi *best = this;
		for (ChannelNode *i = _activeChain; i; i = i->_next) {
			assert (i->_in);
			if ((best->_exhaust == i->_in->_exhaust && best->_prio >= i->_in->_prio) || (!best->_exhaust && i->_in->_exhaust)) {
				best = i->_in;
				node = i;
			}
		}

		if (!node)
			return;

		IMuseChannel_Midi *prt = _drv->getPart(node->_number);
		if (prt)
			prt->sendMidi(0x80, node->_note, 0x40);

		if (_newSystem && prt)
			prt->_exhaust = (--prt->_channelUsage > prt->_polyphony);

		disconnect(_activeChain, node);
	}

	assert(node);
	node->_in = this;
	node->_number = _number;
	node->_note = note;

	connect(_activeChain, node);

	if (_newSystem)
		_exhaust = (++_channelUsage > _polyphony);

	sendNoteOn(note, velocity);
}

void IMuseChannel_Midi::sendNoteOff(byte note) {
	sendMidi(0x80, note, 0x40);
}

void IMuseChannel_Midi::sendNoteOn(byte note, byte velocity) {
	sendMidi(0x90, note, velocity);
}

} // End of namespace IMSMidi

namespace Scumm {
using namespace IMSMidi;

IMuseDriver_GMidi::IMuseDriver_GMidi(MidiDriver::DeviceHandle dev, bool rolandGSMode, bool newSystem) : MidiDriver(), _drv(nullptr), _gsMode(rolandGSMode),
	_imsParts(nullptr), _newSystem(newSystem), _numChannels(16), _notesPlaying(nullptr), _notesSustained(nullptr), _idleChain(nullptr), _activeChain(nullptr), _numVoices(12) {
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
		initRolandGSMode();

	initDevice();

	return res;
}

void IMuseDriver_GMidi::close() {
	if (isOpen() && _drv) {
		deinitDevice();
		_drv->close();
	}

	releaseChannels();
}

MidiChannel *IMuseDriver_GMidi::allocateChannel() {
	if (!isOpen())
		return nullptr;

	for (int i = 0; i < _numChannels; ++i) {
		IMuseChannel_Midi *ch = _imsParts[i];
		if (ch && ch->getNumber() != 9 && ch->allocate())
			return ch;
	}

	return nullptr;
}

MidiChannel *IMuseDriver_GMidi::getPercussionChannel() {
	if (!isOpen())
		return nullptr;

	IMuseChannel_Midi *ch = getPart(9);
	return ch;
}

IMuseChannel_Midi *IMuseDriver_GMidi::getPart(int number) {
	for (int i = 0; i < _numChannels; ++i)
		if (_imsParts[i]->getNumber() == number)
			return _imsParts[i];
	return nullptr;
}

void IMuseDriver_GMidi::createChannels() {
	releaseChannels();
	createParts();

	for (int i = 0; i < _numVoices; ++i) {
		ChannelNode *node = new ChannelNode();
		assert(node);
		connect(_idleChain, node);
	}

	if (_newSystem) {
		_notesPlaying = new uint16[128]();
		_notesSustained = new uint16[128]();
	}
}

void IMuseDriver_GMidi::createParts() {
	_imsParts = new IMuseChannel_Midi*[_numChannels];
	assert(_imsParts);
	for (int i = 0; i < _numChannels; ++i)
		_imsParts[i] = new IMuseChannel_Midi(this, i);
}

void IMuseDriver_GMidi::releaseChannels() {
	if (_imsParts) {
		for (int i = 0; i < _numChannels; ++i)
			delete _imsParts[i];
		delete[] _imsParts;
		_imsParts = nullptr;
	}

	int released = 0;
	while (_idleChain) {
		ChannelNode *node = _idleChain;
		disconnect(_idleChain, node);
		delete node;
		released++;
	}

	while (_activeChain) {
		ChannelNode *node = _activeChain;
		disconnect(_activeChain, node);
		delete node;
		released++;
	}

	assert(released == 0 || released == _numVoices);

	delete[] _notesPlaying;
	_notesPlaying = nullptr;
	delete[] _notesSustained;
	_notesSustained = nullptr;
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

void IMuseDriver_GMidi::initRolandGSMode() {
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

void IMuseDriver_GMidi::deinitDevice() {
	for (int i = 0; i < 16; ++i) {
		send(0x0040B0 | i);
		send(0x007BB0 | i);
	}
}

} // End of namespace Scumm

namespace IMSMidi {

/**************************
*		MT-32 driver
***************************/

class IMuseChannel_MT32 : public IMuseChannel_Midi {
public:
	IMuseChannel_MT32(IMuseDriver_MT32 *drv, int number);
	~IMuseChannel_MT32() override {}

	bool allocate() override;
	void reset();

	// Regular messages
	void programChange(byte program) override;

	// Control Change and SCUMM specific functions
	void volume(byte value) override;
	void panPosition(byte value) override;
	void modulationWheel(byte value) override;
	void effectLevel(byte value) override;
	void chorusLevel(byte value) override {}
	void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override;

	void setOutput(ChannelNode *out) override { _out = out; }

private:
	bool validateTransmission(byte note) const override { return _program < 128 && (!_newSystem || !(_number == 9 && note > 75)); }

	void sendMidi(byte stat, byte par1, byte par2) override {
		if (_drv && (_out || _number == 9))
			_drv->send(((par2) << 16) | ((par1) << 8) | ((stat & 0xF0) | (_out ? _out->_number : 9)));
	}

	void sendNoteOff(byte note) override;
	void sendNoteOn(byte note, byte velocity) override;

	void sendSysexPatchData(byte offset, const byte *data, uint32 dataSize) const;
	void sendSysexTimbreData(const byte *data, uint32 dataSize) const;

	IMuseDriver_MT32 *_mt32Drv;
	ChannelNode *_out;
	byte _program;
	byte _timbre;
	byte _volume;
	byte _panPos;
	byte _reverbSwitch;

	ChannelNode *&_hwRealChain;

	const byte *_programsMapping;
	const uint32 _sysexPatchAddrBase;
	const uint32 _sysexTimbreAddrBase;

	enum SysexMessageSize {
		kSysexLengthTimbre = 254
	};
};

IMuseChannel_MT32::IMuseChannel_MT32(IMuseDriver_MT32 *drv, int number) : IMuseChannel_Midi(drv, number), _out(nullptr), _program(0), _timbre(0xFF),
	_volume(0x7F), _panPos(0x40), _reverbSwitch(1), _sysexPatchAddrBase(0x14000 + (number << 3)), _sysexTimbreAddrBase(0x20000 + (number << 8)),
	_mt32Drv(drv), _programsMapping(drv ? drv->_programsMapping : nullptr), _hwRealChain(drv ? drv->_hwRealChain : _dummyNode) {
}

bool IMuseChannel_MT32::allocate() {
	bool res = IMuseChannel_Midi::allocate();

	if (res && !_newSystem)
		_program = _number;

	return res;
}

void IMuseChannel_MT32::reset() {
	if (_newSystem)
		return;
	byte msg[] = { (byte)(_timbre >> 6), (byte)(_timbre & 0x3F), 0x18, 0x32, 0x10, 0x00, _reverbSwitch};
	sendSysexPatchData(0, msg, sizeof(msg));
}

void IMuseChannel_MT32::programChange(byte program)  {
	if (program > 127)
		return;

	if (_newSystem) {
		if (_programsMapping)
			program = _programsMapping[program];
		_program = program;
	} else if (_timbre != program) {
		_timbre = program;
		byte msg[2] = { (byte)(program >> 6), (byte)(program & 0x3F) };
		sendSysexPatchData(0, msg, sizeof(msg));
	}

	if (_program < 128)
		sendMidi(0xC0, _program, 0);
}

void IMuseChannel_MT32::volume(byte value) {
	_volume = value;
#ifdef FORCE_NEWSTYLE_CHANNEL_ALLOCATION
	if (_number != 9)
#else
	if (!_newSystem || _number != 9)
#endif
		sendMidi(0xB0, 0x07, value);
}

void IMuseChannel_MT32::panPosition(byte value) {
	_panPos = value;
	sendMidi(0xB0, 0x0A, value);
}

void IMuseChannel_MT32::modulationWheel(byte value) {
	if (!_newSystem)
		sendMidi(0xB0, 0x01, value);
}

void IMuseChannel_MT32::effectLevel(byte value) {
	// The SAMNMAX Roland MT-32 driver ignores this (same with most of the other
	// sysex magic that the older drivers did in several places).
	if (_newSystem)
		return;

	value = value ? 1 : 0;
	if (_reverbSwitch == value)
		return;

	_reverbSwitch = value;

	sendSysexPatchData(6, &_reverbSwitch, 1);
	if (_out)
		_mt32Drv->sendMT32Sysex(_out->_addr + 6, &_reverbSwitch, 1);
}

void IMuseChannel_MT32::sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize)  {
	if (type != 'ROL ') {
		warning("IMuseChannel_MT32: Receiving '%c%c%c%c' instrument data. Probably loading a savegame with that sound setting", (type >> 24) & 0xFF, (type >> 16) & 0xFF, (type >> 8) & 0xFF, type & 0xFF);
		return;
	}

	if (*instr++ != 0x41 || dataSize < 6) {
		warning("IMuseChannel_MT32::sysEx_customInstrument(): Invalid sysex message received");
		return;
	}

	byte partNo = *instr;
	uint32 addr = (instr[3] << 14) | (instr[4] << 7) | instr[5];

	if (dataSize == kSysexLengthTimbre) {
		if (!(addr & 0xFFFF) || partNo < 16) {
			sendSysexTimbreData(instr + 6, 246);
			_timbre = 0xFF;
			byte msg[2] = { 0x02, _program };
			sendSysexPatchData(0, msg, sizeof(msg));
			if (_out)
				sendMidi(0xC0, _program, 0);
		} else {
			_mt32Drv->sendMT32Sysex(0x22000 + (partNo << 8), instr + 6, 246);
		}
	} else {
		// We cannot arrive here, since our imuse code calls this function only for instruments.
		// So this is just a reminder that the original driver handles more things than we do,
		// (but these things are apparently never used and thus not needed).
		warning("IMuseChannel_MT32::sysEx_customInstrument(): Unsupported sysex message received");
	}
}

void IMuseChannel_MT32::sendNoteOff(byte note) {
	sendMidi(0x80, note, 0x40);
}

void IMuseChannel_MT32::sendNoteOn(byte note, byte velocity) {
	if (_number == 9) {
		sendMidi(0xB0, 0x07, _volume);
		sendMidi(0x90, note, velocity);
		return;
	}

	if (!_out) {
		ChannelNode *nodeReal = _hwRealChain;
		while (nodeReal && nodeReal->_next)
			nodeReal = nodeReal->_next;

		assert(nodeReal);
		assert(nodeReal->_in);

		nodeReal->_in->setOutput(nullptr);
		nodeReal->_in = this;
		_out = nodeReal;

		sendMidi(0xB0, 0x7B, 0);
		sendMidi(0xB0, 0x07, _volume);
		sendMidi(0xB0, 0x0A, _panPos);
		sendMidi(0xC0, _program, 0);
		sendMidi(0xE0, _pitchBend & 0x7F, (_pitchBend >> 7) & 0x7F);
	}

	disconnect(_hwRealChain, _out);
	connect(_hwRealChain, _out);

	sendMidi(0x90, note, velocity);
}

void IMuseChannel_MT32::sendSysexPatchData(byte offset, const byte *data, uint32 dataSize) const {
	assert(!_newSystem);
	_mt32Drv->sendMT32Sysex(_sysexPatchAddrBase + offset, data, dataSize);
}

void IMuseChannel_MT32::sendSysexTimbreData(const byte *data, uint32 dataSize) const {
	assert(!_newSystem);
	_mt32Drv->sendMT32Sysex(_sysexTimbreAddrBase, data, dataSize);
}

} // End of namespace IMSMidi

namespace Scumm {

IMuseDriver_MT32::IMuseDriver_MT32(MidiDriver::DeviceHandle dev, bool newSystem) : IMuseDriver_GMidi(dev, false, newSystem), _programsMapping(nullptr), _hwRealChain(nullptr) {
#ifdef FORCE_NEWSTYLE_CHANNEL_ALLOCATION
	_numChannels = 16;
#else
	_numChannels = newSystem ? 16 : 9;
#endif
	_numVoices = 9;

	assert(_drv);
	_drv->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	if (_newSystem)
		_programsMapping = MidiDriver::_gmToMt32;
}

void IMuseDriver_MT32::initDevice() {
	// Display a welcome message on MT-32 displays. Compute version string (truncated to 20 chars max.)
	Common::String infoStr = gScummVMVersion;
	infoStr = "ScummVM " + infoStr.substr(0, MIN<uint32>(infoStr.findFirstNotOf("0123456789."), 12));
	for (int i = (20 - (int)infoStr.size()) >> 1; i > 0; --i)
		infoStr = ' ' + infoStr + ' ';
	sendMT32Sysex(0x80000, (const byte*)infoStr.c_str(), MIN<uint32>(infoStr.size(), 20));

	// Reset the MT-32
	sendMT32Sysex(0x1FC000, 0, 0);

	g_system->delayMillis(250);

	// Setup master tune, reverb mode, reverb time, reverb level, channel mapping, partial reserve and master volume
	static const char initSysex1[] = "\x40\x00\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x64";
	sendMT32Sysex(0x40000, (const byte*)initSysex1, sizeof(initSysex1) - 1);
	g_system->delayMillis(40);

	if (!_newSystem) {
		// Map percussion to notes 24 - 34 without reverb. It still happens in the DOTT driver, but not in the SAMNMAX one.
		static const char initSysex2[] = "\x40\x64\x07\x00\x4a\x64\x06\x00\x41\x64\x07\x00\x4b\x64\x08\x00\x45\x64\x06\x00\x44\x64"
										 "\x0b\x00\x51\x64\x05\x00\x43\x64\x08\x00\x50\x64\x07\x00\x42\x64\x03\x00\x4c\x64\x07\x00";
		sendMT32Sysex(0xC090, (const byte*)initSysex2, sizeof(initSysex2) - 1);
		g_system->delayMillis(40);
	}

	const byte pbRange = 0x10;
	for (int i = 0; i < 128; ++i) {
		sendMT32Sysex(0x014004 + (i << 3), &pbRange, 1);
		g_system->delayMillis(5);
	}

	for (int i = 0; i < 16; ++i) {
		send(0x0000C0 | i);
		send(0x0040B0 | i);
		send(0x007BB0 | i);
		send(0x3F0AB0 | i);
		send(0x4000E0 | i);
	}

	for (int i = 0; i < _numChannels; ++i) {
		static_cast<IMuseChannel_MT32*>(_imsParts[i])->reset();
		g_system->delayMillis(5);
	}
}

void IMuseDriver_MT32::deinitDevice() {
	for (int i = 0; i < 16; ++i) {
		send(0x0040B0 | i);
		send(0x007BB0 | i);
	}
	// Reset the MT-32
	sendMT32Sysex(0x1FC000, 0, 0);
}

void IMuseDriver_MT32::createChannels() {
	releaseChannels();

	IMuseDriver_GMidi::createChannels();

	for (int i = 1; i < 9; ++i) {
		ChannelNode *node = new ChannelNode();
		assert(node);
		node->_number = i;
		node->_in = getPart(i);
		assert(node->_in);
		node->_in->setOutput(node);
		node->_addr = 0xC000 + (i << 4);
		connect(_hwRealChain, node);
	}
}

void IMuseDriver_MT32::createParts() {
	_imsParts = new IMuseChannel_Midi*[_numChannels];
	assert(_imsParts);
	for (int i = 0; i < _numChannels; ++i) {
		IMuseChannel_MT32 *prt = new IMuseChannel_MT32(this, (i + 1) & 0x0F);
		_imsParts[i] = prt;
	}
}

void IMuseDriver_MT32::releaseChannels() {
	IMuseDriver_GMidi::releaseChannels();

	int released = 0;
	while (_hwRealChain) {
		ChannelNode *node = _hwRealChain;
		disconnect(_hwRealChain, node);
		delete node;
		released++;
	}
	assert(released == 0 || released == 8);
}

void IMuseDriver_MT32::sendMT32Sysex(uint32 addr, const byte *data, uint32 dataSize) {
	static const byte header[] = { 0x41, 0x10, 0x16, 0x12 };

	byte *msg = new byte[sizeof(header) + 4 + dataSize];
	memcpy(msg, header, sizeof(header));
	byte *dst = msg + sizeof(header);
	const byte *src = dst;

	*dst++ = (addr >> 14) & 0x7F;
	*dst++ = (addr >> 7) & 0x7F;
	*dst++ = addr & 0x7F;

	while (dataSize) {
		*dst++ = *data++;
		--dataSize;
	}

	uint8 checkSum = 0;
	while (src < dst)
		checkSum -= *src++;

	*dst++ = checkSum & 0x7F;

	dataSize = dst - msg;
	sysEx(msg, dataSize);

	delete[] msg;
}

#undef FORCE_NEWSTYLE_CHANNEL_ALLOCATION

} // End of namespace Scumm
