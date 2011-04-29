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
 *
 * $URL:  $
 * $Id:  $
 */

#include "audio/softsynth/fmtowns_pc98/towns_midi.h"
#include "common/textconsole.h"

class MidiChannel_TOWNS : public MidiChannel {
public:
	MidiChannel_TOWNS(MidiDriver_TOWNS *driver);
	~MidiChannel_TOWNS();

	MidiDriver *device() { return _driver; }
	byte getNumber() { return 0; }
	void release();

	void send(uint32 b);

	void noteOff(byte note);
	void noteOn(byte note, byte velocity);
	void programChange(byte program);
	void pitchBend(int16 bend);
	void controlChange(byte control, byte value);
	void pitchBendFactor(byte value);
	void priority(byte value);

	void sysEx_customInstrument(uint32 type, const byte *instr);

private:
	MidiDriver_TOWNS *_driver;
};

MidiChannel_TOWNS::MidiChannel_TOWNS(MidiDriver_TOWNS *driver) : MidiChannel(), _driver(driver) {

}

MidiChannel_TOWNS::~MidiChannel_TOWNS() {

}

void MidiChannel_TOWNS::release() {

}

void MidiChannel_TOWNS::send(uint32 b) {

}

void MidiChannel_TOWNS::noteOff(byte note) {

}

void MidiChannel_TOWNS::noteOn(byte note, byte velocity) {

}

void MidiChannel_TOWNS::programChange(byte program) {

}

void MidiChannel_TOWNS::pitchBend(int16 bend) {

}

void MidiChannel_TOWNS::controlChange(byte control, byte value) {

}

void MidiChannel_TOWNS::pitchBendFactor(byte value) {

}

void MidiChannel_TOWNS::priority(byte value) {

}

void MidiChannel_TOWNS::sysEx_customInstrument(uint32 type, const byte *instr) {

}

MidiDriver_TOWNS::MidiDriver_TOWNS(Audio::Mixer *mixer) : _timerBproc(0), _timerBpara(0), _open(false) {
	_intf = new TownsAudioInterface(mixer, this);
	_channels = new MidiChannel_TOWNS*[16];
	for (int i = 0; i < 16; i++)
		_channels[i] = new MidiChannel_TOWNS(this);

	_tickCounter = 0;
	_curChan = 0;
	//unbuffered write:	_intf->callback(17, part, reg, val);
	//buffered write:	_intf->callback(19, part, reg, val);
}

MidiDriver_TOWNS::~MidiDriver_TOWNS() {
	close();
	delete _intf;
	setTimerCallback(0, 0);

	for (int i = 0; i < 16; i++)
		delete _channels[i];
	delete[] _channels;
}

int MidiDriver_TOWNS::open() {
	if (_open)
		return MERR_ALREADY_OPEN;

	if (!_intf->init())
		return MERR_CANNOT_CONNECT;

	_intf->callback(0);

	_intf->callback(21, 255, 1);
	_intf->callback(21, 0, 1);
	_intf->callback(22, 255, 221);

	_intf->callback(33, 8);
	_intf->setSoundEffectChanMask(~0x3f);

	_open = true;

	return 0;
}

void MidiDriver_TOWNS::close() {
	_open = false;
}

void MidiDriver_TOWNS::send(uint32 b) {
	byte param2 = (b >> 16) & 0xFF;
	byte param1 = (b >> 8) & 0xFF;
	byte cmd = b & 0xF0;

	/*AdLibPart *part;
	if (chan == 9)
		part = &_percussion;
	else**/
	MidiChannel_TOWNS *c = _channels[b & 0x0F];

	switch (cmd) {
	case 0x80:
		//part->noteOff(param1);
		break;
	case 0x90:
		//part->noteOn(param1, param2);
		if (param2)
			c->noteOn(param1, param2);
		else
			c->noteOff(param1);
		break;
	case 0xB0:
		// supported: 1, 7, 0x40
		c->controlChange(param1, param2);
		break;
	case 0xC0:
		c->programChange(param1);
		break;
	case 0xE0:
		//part->pitchBend((param1 | (param2 << 7)) - 0x2000);
		c->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0:
		warning("MidiDriver_ADLIB: Receiving SysEx command on a send() call");
		break;

	default:
		break;
	}
}

void MidiDriver_TOWNS::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerBproc = timer_proc;
	_timerBpara = timer_param;
}

uint32 MidiDriver_TOWNS::getBaseTempo() {
	return 0;
}

MidiChannel *MidiDriver_TOWNS::allocateChannel() {
	MidiChannel *res = 0;

	for (int i = 0; i < 6; i++) {
		if (++_curChan == 6)
			_curChan = 0;

		//if (_channels[i]->   //// )
		//	return _channels[i];

	}

	//if (res)
	//	res->noteOff();

	return res;
}

MidiChannel *MidiDriver_TOWNS::getPercussionChannel() {
	return 0;
}

void MidiDriver_TOWNS::timerCallback(int timerId) {
	if (!_open)
		return;

	switch (timerId) {
	case 1:
		if (_timerBproc) {
			_timerBproc(_timerBpara);
			_tickCounter += 10000;
			while (_tickCounter >= 4167) {
				_tickCounter -= 4167;
				//_timerBproc(_timerBpara);
			}
		}
		break;
	default:
		break;
	}
}
