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

#include "common/str.h"
#include "common/system.h"
#include "base/version.h"
#include "scumm/imuse/drivers/mt32.h"


// This makes older titles play new system style, with 16 virtual channels and
// dynamic allocation (instead of playing on fixed channels).
//#define FORCE_NEWSTYLE_CHANNEL_ALLOCATION

namespace Scumm {

class IMuseChannel_MT32 : public MidiChannel {
public:
	IMuseChannel_MT32(IMuseDriver_MT32 *drv, int number);
	~IMuseChannel_MT32() override {}

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
	void volume(byte value) override;
	void panPosition(byte value) override;
	void pitchBendFactor(byte value) override { _pitchBendSensitivity = value; }
	void transpose(int8 value) override { _transpose = value; }
	void detune(byte value) override { _detune = value; }
	void priority(byte value) override { _prio = value; }
	void modulationWheel(byte value) override;
	void sustain(bool value) override;
	void effectLevel(byte value) override;
	void chorusLevel(byte value) override {}
	void allNotesOff() override;
	void sysEx_customInstrument(uint32 type, const byte *instr) override;

	void setOutput(MT32RealChan *out) { _out = out; }

private:
	void noteOffIntern(byte note);
	void noteOnIntern(byte note, byte velocity);

	void sendSysexPatchData(byte offset, const byte *data, uint32 dataSize) const;
	void sendSysexTimbreData(const byte *data, uint32 dataSize) const;

	void setNotePlaying(byte note) { _drv->setNoteFlag(_number, note); }
	void clearNotePlaying(byte note) { _drv->clearNoteFlag(_number, note); }
	bool isNotePlaying(byte note) const { return _drv->queryNoteFlag(_number, note); }
	void setNoteSustained(byte note) { _drv->setSustainFlag(_number, note); }
	void clearNoteSustained(byte note) { _drv->clearSustainFlag(_number, note); }
	bool isNoteSustained(byte note) const { return _drv->querySustainFlag(_number, note); }

	IMuseDriver_MT32 *_drv;
	const byte _number;
	const bool _newSystem;
	bool _allocated;

	MT32RealChan *_out;
	byte _program;
	byte _timbre;
	byte _volume;
	byte _panPos;
	byte _polyphony;
	byte _channelUsage;
	bool _exhaust;
	byte _prio;
	int8 _detune;
	int8 _transpose;
	byte _reverbSwitch;
	byte _pitchBendSensitivity;
	int16 _pitchBend;
	bool _sustain;

	MT32ControlChan *&_idleChain;
	MT32RealChan *&_availHwChain;
	MT32ControlChan *&_activeChain;

	const byte *_programsMapping;
	const uint32 _sysexPatchAddrBase;
	const uint32 _sysexTimbreAddrBase;
};

class MT32Chan {
public:
	MT32Chan(int number, IMuseChannel_MT32 *in) : _prev(nullptr), _next(nullptr), _in(in), _number(number) {}
	MT32Chan *_prev;
	MT32Chan *_next;
	IMuseChannel_MT32 *_in;
	byte _number;
};

class MT32RealChan : public MT32Chan {
public:
	MT32RealChan(int number, IMuseChannel_MT32 *in, MidiChannel *out) : MT32Chan(number, in), _out(out), _sysexTempAddrBase(0xC000 + (number << 4)) {
		assert(_out && _number == out->getNumber());
		if (in)
			in->setOutput(this);
	}
	MidiChannel *_out;
	const uint32 _sysexTempAddrBase;
};

class MT32ControlChan : public MT32Chan{
public:
	MT32ControlChan() : MT32Chan(0, nullptr), _note(0) {}
	byte _note;
};

template <typename MT32ChanTmpl>
void connect(MT32ChanTmpl *&chain, MT32Chan *node) {
	if (!node || node->_prev || node->_next)
		return;
	if ((node->_next = chain))
		chain->_prev = node;
	chain = static_cast<MT32ChanTmpl*>(node);
}

template <typename MT32ChanTmpl>
void disconnect(MT32ChanTmpl *&chain, MT32Chan *node) {
	if (!node || !chain)
		return;

	const MT32Chan *ch = static_cast<MT32Chan*>(chain);
	while (ch && ch != node)
		ch = ch->_next;
	if (!ch)
		return;

	if (node->_next)
		node->_next->_prev = node->_prev;

	if (node->_prev)
		node->_prev->_next = node->_next;
	else
		chain = static_cast<MT32ChanTmpl*>(node->_next);

	node->_next = node->_prev = nullptr;
}

#define sendMidi(stat, par1, par2)	_drv->send(((par2) << 16) | ((par1) << 8) | (stat))

IMuseChannel_MT32::IMuseChannel_MT32(IMuseDriver_MT32 *drv, int number) :MidiChannel(), _drv(drv), _number(number), _allocated(false), _out(nullptr), _sustain(false),
	_program(0), _timbre(0xFF),	_volume(0x7F), _panPos(0x40), _pitchBend(0x2000), _polyphony(1), _channelUsage(0), _exhaust(false), _prio(0), _detune(0), _transpose(0),
	_pitchBendSensitivity(2), _reverbSwitch(1), _idleChain(_drv->_idleChain), _availHwChain(_drv->_hwChain), _activeChain(_drv->_activeChain),
	_sysexPatchAddrBase(0x14000 + (number << 3)), _sysexTimbreAddrBase(0x20000 + (number << 8)), _programsMapping(_drv->_programsMapping), _newSystem(_drv->_newSystem) {
	assert(_drv);
}

bool IMuseChannel_MT32::allocate() {
	if (_allocated)
		return false;

	if (!_newSystem) {
		byte msg[] = { (byte)(_timbre >> 6), (byte)(_timbre & 0x3F), 0x18, 0x32, 0x10, 0x00, _reverbSwitch};
		sendSysexPatchData(0, msg, sizeof(msg));
		_program = _number;
		_prio = 0x80;
	}

	return (_allocated = true);
}

void IMuseChannel_MT32::noteOff(byte note)  {
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
	 else if (_out || _number == 9)
		sendMidi(0x80 | _number, note, 0x40);
#endif
}

void IMuseChannel_MT32::noteOn(byte note, byte velocity)  {
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
	if (_newSystem) {
		noteOnIntern(note, velocity);
	} else if (_out) {
		sendMidi(0x90 | _out->_number, note, velocity);
	} else if (_number == 9) {
		sendMidi(0xB9, 0x07, _volume);
		sendMidi(0x99, note, velocity);
	}
#endif
}

void IMuseChannel_MT32::controlChange(byte control, byte value)  {
	switch (control) {
	case 7:
		volume(value);
		break;
	case 10:
		panPosition(value);
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
		warning("Unhandled Control: %d", control);
		break;
	}
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

	if (_out && _program < 128)
		sendMidi(0xC0 | _out->_number, _program, 0);
}

void IMuseChannel_MT32::pitchBend(int16 bend)  {
	_pitchBend = bend + 0x2000;
	if (_out)
		sendMidi(0xE0 | _out->_number, _pitchBend & 0x7F, (_pitchBend >> 7) & 0x7F);
}

void IMuseChannel_MT32::volume(byte value) {
	_volume = value;
	if (_out)
		sendMidi(0xB0 | _out->_number, 0x07, value);
}

void IMuseChannel_MT32::panPosition(byte value) {
	_panPos = value;
	if (_out)
		sendMidi(0xB0 | _out->_number, 0x0A, value);
}

void IMuseChannel_MT32::modulationWheel(byte value) {
	if (_out && !_newSystem)
		sendMidi(0xB0 | _out->_number, 0x01, value);
}

void IMuseChannel_MT32::sustain(bool value) {
	_sustain = value;

	if (_newSystem) {
		// For SAMNMAX, this is fully software controlled. No control change message gets sent.
		if (_sustain)
			return;

		for (int i = 0; i < 128; ++i) {
			if (isNoteSustained(i))
				noteOffIntern(i);
		}

	} else if (_out) {
		sendMidi(0xB0 | _out->_number, 0x40, value);
	}
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
		_drv->sendMT32Sysex(_out->_sysexTempAddrBase + 6, &_reverbSwitch, 1);
}

void IMuseChannel_MT32::allNotesOff() {
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

	} else if (_out) {
		sendMidi(0xB0 | _out->_number, 0x7B, 0);
	}
}

void IMuseChannel_MT32::sysEx_customInstrument(uint32 type, const byte *instr)  {
	if (*instr++ != 0x41) {
		warning("IMuseChannel_MT32::sysEx_customInstrument(): Invalid (non-Roland) sysex message received");
		return;
	}

	byte partNo = *instr;
	uint32 addr = (instr[3] << 14) | (instr[4] << 7) | instr[5];

	if (!(addr & 0xFFFF) || partNo < 16) {
		sendSysexTimbreData(instr + 6, 246);
		_timbre = 0xFF;
		byte msg[2] = { 0x02, _program };
		sendSysexPatchData(0, msg, sizeof(msg));
		if (_out)
			sendMidi(0xC0 | _out->_number, _program, 0);
	} else {
		_drv->sendMT32Sysex(0x22000 + (partNo << 8), instr + 6, 246);
	}
}

void IMuseChannel_MT32::noteOffIntern(byte note) {
	if (_program > 127 || _activeChain == nullptr)
		return;

	MT32Chan *node = nullptr;
	for (MT32ControlChan *i = _activeChain; i; i = static_cast<MT32ControlChan*>(i->_next)) {
		if (i->_number == _number && i->_note == note) {
			node = i;
			break;
		}
	}

	if (!node)
		return;

	if (_out)
		sendMidi(0x80 | _out->_number, note, 0x40);
	else
		sendMidi(0x89, note, 0x40);

	if (_newSystem)
		_exhaust = (--_channelUsage > _polyphony);

	disconnect(_activeChain, node);
	connect(_idleChain, node);
}

void IMuseChannel_MT32::noteOnIntern(byte note, byte velocity) {
	if (_program > 127 || (_newSystem && _number == 9 && note > 75))
		return;

	MT32ControlChan *node = nullptr;

	if (_idleChain) {
		node = _idleChain;
		disconnect(_idleChain, node);
	} else {
		MT32Chan *foundChan = nullptr;
		IMuseChannel_MT32 *best = this;
		for (MT32Chan *i = _activeChain; i; i = i->_next) {
			assert (i->_in);
			if ((best->_exhaust == i->_in->_exhaust && best->_prio >= i->_in->_prio) || (!best->_exhaust && i->_in->_exhaust)) {
				best = i->_in;
				foundChan = i;
			}
		}

		if (!foundChan)
			return;

		node = static_cast<MT32ControlChan*>(foundChan);

		IMuseChannel_MT32 *prt = _drv->getPart(node->_number);
		if (prt && prt->_out)
			sendMidi(0x80 | prt->_out->_number, node->_note, 0x40);
		else if (node->_number == 9)
			sendMidi(0x89, node->_note, 0x40);

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

	if (_number == 9) {
		sendMidi(0xB9, 0x07, _volume);
		sendMidi(0x99, note, velocity);
		return;
	}

	if (!_out) {
		MT32RealChan *nodeReal = _availHwChain;
		while (nodeReal && nodeReal->_next)
			nodeReal = static_cast<MT32RealChan*>(nodeReal->_next);

		assert(nodeReal);
		assert(nodeReal->_in);

		nodeReal->_in->_out = nullptr;
		nodeReal->_in = this;
		_out = nodeReal;

		sendMidi(0xB0 | _out->_number, 0x7B, 0);
		sendMidi(0xB0 | _out->_number, 0x07, _volume);
		sendMidi(0xB0 | _out->_number, 0x0A, _panPos);
		sendMidi(0xC0 | _out->_number, _program, 0);
		sendMidi(0xE0 | _out->_number, _pitchBend & 0x7F, (_pitchBend >> 7) & 0x7F);
	}

	disconnect(_availHwChain, _out);
	connect(_availHwChain, _out);

	sendMidi(0x90 | _out->_number, note, velocity);
}

void IMuseChannel_MT32::sendSysexPatchData(byte offset, const byte *data, uint32 dataSize) const {
	assert(!_newSystem);
	_drv->sendMT32Sysex(_sysexPatchAddrBase + offset, data, dataSize);
}

void IMuseChannel_MT32::sendSysexTimbreData(const byte *data, uint32 dataSize) const {
	assert(!_newSystem);
	_drv->sendMT32Sysex(_sysexTimbreAddrBase, data, dataSize);
}

#undef sendMidi

IMuseDriver_MT32::IMuseDriver_MT32(MidiDriver::DeviceHandle dev, bool newSystem) : MidiDriver(), _newSystem(newSystem), _programsMapping(nullptr), _notesPlaying(nullptr),
	_notesSustained(nullptr), _drv(nullptr), _imsParts(nullptr), _hwOutputChan(nullptr), _controlChan(nullptr), _idleChain(nullptr), _hwChain(nullptr), _activeChain(nullptr),
#ifdef FORCE_NEWSTYLE_CHANNEL_ALLOCATION
	_numChannels(16) {
#else
	_numChannels(newSystem ? 16 : 9) {
#endif
	_drv = MidiDriver::createMidi(dev);
	assert(_drv);

	_drv->property(MidiDriver::PROP_CHANNEL_MASK, 0x03FE);

	if (newSystem)
		_programsMapping = MidiDriver::_gmToMt32;
}

IMuseDriver_MT32::~IMuseDriver_MT32() {
	close();
	delete _drv;
}

int IMuseDriver_MT32::open() {
	if (!_drv)
		return MERR_CANNOT_CONNECT;

	int res = _drv->open();
	if (res)
		return res;

	initDevice();
	createChannels();

	return res;
}

void IMuseDriver_MT32::close() {
	if (isOpen()) {
		// Reset the MT-32
		sendMT32Sysex(0x1FC000, 0, 0);
		g_system->delayMillis(250);

		if (_drv)
			_drv->close();
	}

	releaseChannels();
}

MidiChannel *IMuseDriver_MT32::allocateChannel() {
	if (!isOpen())
		return nullptr;

	for (int i = 0; i < _numChannels; ++i) {
		IMuseChannel_MT32 *ch = _imsParts[i];
		if (ch && ch->getNumber() != 9 && ch->allocate())
			return ch;
	}

	return nullptr;
}

MidiChannel *IMuseDriver_MT32::getPercussionChannel() {
	if (!isOpen())
		return nullptr;

	IMuseChannel_MT32 *ch = getPart(9);
	if (ch) {
		ch->release();
		ch->allocate();
	}

	return ch;
}

void IMuseDriver_MT32::initDevice() {
	// Reset the MT-32
	sendMT32Sysex(0x1FC000, 0, 0);
	g_system->delayMillis(250);

	// Setup master tune, reverb mode, reverb time, reverb level,
	// channel mapping, partial reserve and master volume
	static const char initSysex1[] = "\x40\x00\x04\x04\x04\x04\x04\x04\x04\x04\x04\x04\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x64";
	sendMT32Sysex(0x40000, (const byte*)initSysex1, sizeof(initSysex1) - 1);
	g_system->delayMillis(250);

	if (!_newSystem) {
		// Map percussion to notes 24 - 34 without reverb. It still happens in the DOTT driver, but not in the SAMNMAX one.
		static const char initSysex2[] = "\x40\x64\x07\x00\x4a\x64\x06\x00\x41\x64\x07\x00\x4b\x64\x08\x00\x45\x64\x06\x00\x44\x64\x0b\x00\x51\x64\x05\x00\x43\x64\x08\x00\x50\x64\x07\x00\x42\x64\x03\x00\x4c\x64\x07\x00";
		sendMT32Sysex(0xC090, (const byte*)initSysex2, sizeof(initSysex2) - 1);
		g_system->delayMillis(250);
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
	}

	// Compute version string (truncated to 20 chars max.)
	Common::String infoStr = "ScummVM ";
	infoStr += gScummVMVersion;
	int len = infoStr.size();
	if (len > 20)
		len = 20;

	// Display a welcome message on MT-32 displays.
	byte buffer[28];
	memcpy(&buffer[0], "\x41\x10\x16\x12\x20\x00\x00", 7);
	memcpy(&buffer[7], "                    ", 20);
	memcpy(buffer + 7 + (20 - len) / 2, infoStr.c_str(), len);
	byte checksum = 0;
	for (int i = 4; i < 27; ++i)
		checksum -= buffer[i];
	buffer[27] = checksum & 0x7F;
	sysEx(buffer, 28);
	g_system->delayMillis(1000);
}

void IMuseDriver_MT32::createChannels() {
	releaseChannels();

	_imsParts = new IMuseChannel_MT32*[_numChannels];
	_hwOutputChan = new MT32RealChan*[8];
	_controlChan = new MT32ControlChan*[9];

	assert(_imsParts);
	assert(_hwOutputChan);
	assert(_controlChan);

	for (int i = 0; i < _numChannels; ++i)
		_imsParts[i] = new IMuseChannel_MT32(this, (i + 1) & 0x0F);

	MidiChannel *driverChannels[16];
	memset(driverChannels, 0, sizeof(driverChannels));
	for (int i = 0; i < ARRAYSIZE(driverChannels); ++i)
		driverChannels[i] = _drv->allocateChannel();

	for (int i = 1; i < 9; ++i) {
		MidiChannel *m = nullptr;
		for (int ii = 0; m == nullptr && ii < ARRAYSIZE(driverChannels); ++ii) {
			if (driverChannels[ii] && driverChannels[ii]->getNumber() == i)
				SWAP(m, driverChannels[ii]);
		}
		if (!m)
			error("IMuseDriver_MT32::createChannels(): Failed to create channels.");
		_hwOutputChan[i - 1] = new MT32RealChan(i, getPart(i), m);
		connect(_hwChain, _hwOutputChan[i - 1]);
	}

	for (int i = 0; i < ARRAYSIZE(driverChannels); ++i) {
		if (driverChannels[i])
			driverChannels[i]->release();
	}

	for (int i = 0; i < 9; ++i) {
		_controlChan[i] = new MT32ControlChan();
		connect(_idleChain, _controlChan[i]);
	}

	if (_newSystem) {
		_notesPlaying = new uint16[128]();
		_notesSustained = new uint16[128]();
	}
}

void IMuseDriver_MT32::releaseChannels() {
	if (_imsParts) {
		for (int i = 0; i < _numChannels; ++i)
			delete _imsParts[i];
		delete[] _imsParts;
		_imsParts = nullptr;
	}

	if (_hwOutputChan) {
		for (int i = 0; i < 8; ++i)
			delete _hwOutputChan[i];
		delete[] _hwOutputChan;
		_hwOutputChan = nullptr;
	}

	if (_controlChan) {
		for (int i = 0; i < 9; ++i)
			delete _controlChan[i];
		delete[] _controlChan;
		_controlChan = nullptr;
	}

	delete[] _notesPlaying;
	_notesPlaying = nullptr;
	delete[] _notesSustained;
	_notesSustained = nullptr;
}

IMuseChannel_MT32 *IMuseDriver_MT32::getPart(int number) const {
	for (int i = 0; i < _numChannels; ++i)
		if (_imsParts[i]->getNumber() == number)
			return _imsParts[i];
	return nullptr;
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

	sysEx(msg, dst - msg);

	delete[] msg;
}

#undef FORCE_NEWSTYLE_CHANNEL_ALLOCATION

} // End of namespace Scumm
