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
 */

#include "audio/softsynth/fmtowns_pc98/towns_euphony.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/textconsole.h"

#define EUP_EVENT(x) _euphonyEvents.push_back(new EuphonyEvent(this, &EuphonyPlayer::event_##x))

#ifdef EUP_USE_MEMPOOL
#define EUP_EVENTS_DELETE(a)	_pendingEventsPool.deleteChunk(a)
#define EUP_EVENTS_NEW			new (_pendingEventsPool)
#else
#define EUP_EVENTS_DELETE(a)	delete a
#define EUP_EVENTS_NEW			new
#endif

EuphonyPlayer::EuphonyPlayer(Audio::Mixer *mixer) : _partConfig_enable(0), _partConfig_type(0), _partConfig_ordr(0), _partConfig_volume(0),
	_partConfig_transpose(0), _musicPos(0), _musicStart(0), _playing(false), _pendingEventsChain(0), _tempoModifier(0), _bar(0),
	_beat(0), _defaultBarLength(0), _barLength(0), _playerUpdatesLeft(0), _updatesPerPulseRemainder(0),	_updatesPerPulse(0),
	_deltaTicks(0), _defaultTempo(0), _trackTempo(0), _tempoControlMode(0), _timerSetting(0), _tempoMode1PulseCounter(0),
	_parseToBar(0), _tempoMode1UpdateF8(0), _loop(false), _endOfTrack(false), _paused(false), _musicTrackSize(0) {
	EUP_EVENT(notImpl);
	EUP_EVENT(noteOn);
	EUP_EVENT(polyphonicAftertouch);
	EUP_EVENT(controlChange_pitchWheel);
	EUP_EVENT(programChange_channelAftertouch);
	EUP_EVENT(programChange_channelAftertouch);
	EUP_EVENT(controlChange_pitchWheel);
	EUP_EVENT(sysex);
	EUP_EVENT(advanceBar);
	EUP_EVENT(notImpl);
	EUP_EVENT(notImpl);
	EUP_EVENT(setTempo);
	EUP_EVENT(notImpl);
	EUP_EVENT(typeOrdrChange);

	_drivers[0] = _eupDriver = new EuphonyDriver(mixer, this);
	_drivers[1] = new Type0Driver(this);
	_drivers[2] = 0;
	resetTempo();
}

#undef EUP_EVENT

EuphonyPlayer::~EuphonyPlayer() {
	for (int i = 0; i < 3; i++)
		delete _drivers[i];

	while (_pendingEventsChain) {
		PendingEvent *evt = _pendingEventsChain;
		_pendingEventsChain = _pendingEventsChain->next;
		EUP_EVENTS_DELETE(evt);
	}

	delete[] _partConfig_enable;
	delete[] _partConfig_type;
	delete[] _partConfig_ordr;
	delete[] _partConfig_volume;
	delete[] _partConfig_transpose;

	for (EuphonyEventsArray::iterator i = _euphonyEvents.begin(); i != _euphonyEvents.end(); ++i)
		delete *i;
}

bool EuphonyPlayer::init() {
	for (int i = 0; i < 3; i++) {
		if (_drivers[i]) {
			if (!_drivers[i]->init()) {
				warning("EuphonyPlayer:: Driver initialization failed: %d", i);
				delete _drivers[i];
				_drivers[i] = 0;
			}
		}
	}

	if (!_drivers[0] || !_drivers[1])
		return false;

	while (_pendingEventsChain) {
		PendingEvent *evt = _pendingEventsChain;
		_pendingEventsChain = _pendingEventsChain->next;
		EUP_EVENTS_DELETE(evt);
	}

	delete[] _partConfig_enable;
	delete[] _partConfig_type;
	delete[] _partConfig_ordr;
	delete[] _partConfig_volume;
	delete[] _partConfig_transpose;

	_partConfig_enable = new uint8[32];
	_partConfig_type = new uint8[32];
	_partConfig_ordr = new uint8[32];
	_partConfig_volume = new int8[32];
	_partConfig_transpose = new int8[32];

	reset();

	return true;
}

int EuphonyPlayer::startTrack(const uint8 *data, int trackSize, int barLen) {
	if (_playing)
		return 2;

	_musicPos = _musicStart = data;
	_defaultBarLength = _barLength = barLen;
	_musicTrackSize = trackSize;
	_parseToBar = _bar = 0;
	_beat = 0;
	_playing = true;

	return 0;
}

void EuphonyPlayer::stop() {
	if (_playing) {
		_playing = false;
		_playerUpdatesLeft = 0;
		_endOfTrack = false;
		clearHangingNotes();
		resetAllControls();
	}
}

void EuphonyPlayer::pause() {
	_paused = true;
	clearHangingNotes();
	allPartsOff();
}

void EuphonyPlayer::resume() {
	_paused = false;
}

int EuphonyPlayer::setTempo(int tempo) {
	if (tempo > 250)
		return 3;
	_defaultTempo = tempo;
	_trackTempo = tempo;
	sendTempo(tempo);
	return 0;
}

void EuphonyPlayer::setLoopStatus(bool loop) {
	_loop = loop;
}

int EuphonyPlayer::configPart_enable(int part, int val) {
	uint8 enable = val & 0xff;
	if (part > 31 || ((enable + 1) & 0xff) > 1)
		return 3;
	_partConfig_enable[part] = enable;
	return 0;
}

int EuphonyPlayer::configPart_setType(int part, int val) {
	uint8 type = val & 0xff;
	if (part > 31 || ((type + 1) & 0xff) > 8)
		return 3;
	_partConfig_type[part] = type;
	return 0;
}

int EuphonyPlayer::configPart_remap(int part, int val) {
	uint8 remap = val & 0xff;
	if (part > 31 || ((remap + 1) & 0xff) > 16)
		return 3;
	_partConfig_ordr[part] = remap;
	return 0;
}

int EuphonyPlayer::configPart_adjustVolume(int part, int val) {
	int8 adjvol = val & 0xff;
	if (part > 31 || adjvol < -40 || adjvol > 40)
		return 3;
	_partConfig_volume[part] = adjvol;
	return 0;
}

int EuphonyPlayer::configPart_setTranspose(int part, int val) {
	int8 trans = val & 0xff;
	if (part > 31 || trans < -40 || trans > 40)
		return 3;
	_partConfig_transpose[part] = trans;
	return 0;
}

void EuphonyPlayer::timerCallback(int timerId) {
	switch (timerId) {
	case 0:
		updatePulseCounters();
		while (_playerUpdatesLeft) {
			--_playerUpdatesLeft;
			updateBeat();
			if (!_playing)
				continue;
			updateHangingNotes();
			updateParser();
			updateCheckEot();
		}
		break;
	default:
		break;
	}
}

void EuphonyPlayer::reset() {
	_eupDriver->reset();
	_eupDriver->setTimerA(true, 1);
	_eupDriver->setTimerA(false, 1);
	_eupDriver->setTimerB(true, 221);

	resetPartConfig();

	while (_pendingEventsChain) {
		PendingEvent *evt = _pendingEventsChain;
		_pendingEventsChain = _pendingEventsChain->next;
		EUP_EVENTS_DELETE(evt);
	}

	_playing = _endOfTrack = _paused = _loop = false;
	_tempoMode1UpdateF8 = 0;
	_tempoMode1PulseCounter = 0;

	resetTempo();

	if (_tempoControlMode == 1) {
		//if (///)
		//  return;
		sendTempo(_defaultTempo);
	} else {
		sendTempo(_defaultTempo);
	}

	resetAllControls();
}

void EuphonyPlayer::resetPartConfig() {
	memset(_partConfig_enable, 0xff, 32);
	memset(_partConfig_type, 0xff, 16);
	memset(_partConfig_type + 16, 0, 16);
	for (int i = 0; i < 32; i++)
		_partConfig_ordr[i] = i & 0x0f;
	memset(_partConfig_volume, 0, 32);
	memset(_partConfig_transpose, 0, 32);
}

void EuphonyPlayer::resetTempo() {
	_defaultBarLength = _barLength = 0x33;
	_playerUpdatesLeft = 0;
	_updatesPerPulseRemainder = 0;
	_updatesPerPulse = 0x10;
	_tempoModifier = 0;
	_bar = 0;
	_deltaTicks = 0;
	_beat = 0;
	_defaultTempo = 90;
	_trackTempo = 90;
}

void EuphonyPlayer::updatePulseCounters() {
	int tc = _updatesPerPulse + _updatesPerPulseRemainder;
	_updatesPerPulseRemainder = tc & 0x0f;
	tc >>= 4;
	_tempoMode1PulseCounter -= tc;

	while (_tempoMode1PulseCounter < 0) {
		_tempoMode1UpdateF8++;
		_tempoMode1PulseCounter += 4;
	}

	if (_playing && !_paused)
		_playerUpdatesLeft += tc;
}

void EuphonyPlayer::updateBeat() {
	static const uint16 beatLengthTable[] = { 0x180, 0xC0, 0x80, 0x60, 0x40, 0x30, 0x20, 0x18 };
	uint8 beatsPersBar = (_barLength & 0x0f) + 1;
	uint8 beatNoteValue = _barLength >> 4;

	if ((uint32)(beatLengthTable[beatNoteValue] * beatsPersBar) > ++_beat)
		return;

	++_bar;
	_beat = 0;
	_deltaTicks = 0;
}

void EuphonyPlayer::updateParser() {
	for (bool loop = true; loop;) {
		uint8 cmd = _musicPos[0];

		if (cmd == 0xff || cmd == 0xf7) {
			proceedToNextEvent();

		} else if (cmd < 0x90) {
			_endOfTrack = true;
			clearHangingNotes();
			loop = false;

		} else if (_parseToBar > _bar) {
			loop = false;

		} else {
			if (_parseToBar == _bar) {
				uint16 parseToBeat = ((_musicPos[3] << 8) | ((_musicPos[2] << 1) & 0xff)) >> 1;
				if (parseToBeat > _beat)
					loop = false;
			}

			if (loop) {
				if (parseEvent())
					loop = false;
			}
		}
	}
}

void EuphonyPlayer::updateCheckEot() {
	if (!_endOfTrack || _pendingEventsChain)
		return;
	stop();
}

bool EuphonyPlayer::parseEvent() {
	uint cmd = _musicPos[0];
	if (cmd != 0xfe && cmd != 0xfd) {
		bool result = (cmd >= 0xf0) ? (*_euphonyEvents[((cmd - 0xf0) >> 1) + 7])() : (*_euphonyEvents[(cmd - 0x80) >> 4])();
		if (!result) {
			proceedToNextEvent();
			return false;
		}
	}

	if (cmd == 0xfd) {
		_paused = true;
		return true;
	}

	if (!_loop) {
		_endOfTrack = true;
		return true;
	}

	_endOfTrack = false;
	_musicPos = _musicStart;
	_parseToBar = _bar = _beat = 0;
	_barLength = _defaultBarLength;

	return false;
}

void EuphonyPlayer::proceedToNextEvent() {
	_musicPos += 6;
	if (_musicPos >= _musicStart + _musicTrackSize)
		_musicPos = _musicStart;
}

void EuphonyPlayer::updateHangingNotes() {
	PendingEvent *l = 0;
	PendingEvent *e = _pendingEventsChain;

	while (e) {
		if (--e->len) {
			l = e;
			e = e->next;
			continue;
		}

		PendingEvent *n = e->next;
		if (l)
			l->next = n;
		if (_pendingEventsChain == e)
			_pendingEventsChain = n;

		sendPendingEvent(e->type, e->evt, e->note, e->velo);
		EUP_EVENTS_DELETE(e);

		e = n;
	}
}

void EuphonyPlayer::clearHangingNotes() {
	while (_pendingEventsChain) {
		PendingEvent *e = _pendingEventsChain;
		_pendingEventsChain = _pendingEventsChain->next;
		sendPendingEvent(e->type, e->evt, e->note, e->velo);
		EUP_EVENTS_DELETE(e);
	}
}

void EuphonyPlayer::resetAllControls() {
	for (int i = 0; i < 32; i++) {
		if (_partConfig_ordr[i] > 15) {
			for (int ii = 0; ii < 16; ii++)
				sendControllerReset(_partConfig_type[i], ii);
		} else {
			sendControllerReset(_partConfig_type[i], _partConfig_ordr[i]);
		}
	}
}

void EuphonyPlayer::allPartsOff() {
	for (int i = 0; i < 32; i++) {
		if (_partConfig_ordr[i] > 15) {
			for (int ii = 0; ii < 16; ii++)
				sendAllNotesOff(_partConfig_type[i], ii);
		} else {
			sendAllNotesOff(_partConfig_type[i], _partConfig_ordr[i]);
		}
	}
}

uint8 EuphonyPlayer::appendEvent(uint8 evt, uint8 chan) {
	if (evt >= 0x80 && evt < 0xf0 && _partConfig_ordr[chan] < 16)
		return (evt & 0xf0) | _partConfig_ordr[chan];
	return evt;
}

bool EuphonyPlayer::event_notImpl() {
	return false;
}

bool EuphonyPlayer::event_noteOn() {
	if (_musicPos[1] > 31)
		return false;
	if (!_partConfig_enable[_musicPos[1]]) {
		proceedToNextEvent();
		return (_musicPos[0] == 0xfe || _musicPos[0] == 0xfd) ? true : false;
	}
	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 type = _partConfig_type[_musicPos[1]];
	uint8 note = _musicPos[4];
	uint8 velo = _musicPos[5];

	sendByte(type, evt);
	sendByte(type, applyTranspose(note));
	sendByte(type, applyVolumeAdjust(velo));

	proceedToNextEvent();
	if (_musicPos[0] == 0xfe || _musicPos[0] == 0xfd)
		return true;

	velo = _musicPos[5];
	uint16 len = (_musicPos[1] & 0x0f) | ((_musicPos[2] & 0x0f) << 4) | ((_musicPos[3] & 0x0f) << 8) | ((_musicPos[4] & 0x0f) << 12);

	_pendingEventsChain = EUP_EVENTS_NEW PendingEvent(evt, type, note, velo, len ? len : 1, _pendingEventsChain);

	return false;
}

bool EuphonyPlayer::event_polyphonicAftertouch() {
	if (_musicPos[1] > 31)
		return false;
	if (!_partConfig_enable[_musicPos[1]])
		return false;

	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 type = _partConfig_type[_musicPos[1]];

	sendByte(type, evt);
	sendByte(type, applyTranspose(_musicPos[4]));
	sendByte(type, _musicPos[5]);

	return false;
}

bool EuphonyPlayer::event_controlChange_pitchWheel() {
	if (_musicPos[1] > 31)
		return false;
	if (!_partConfig_enable[_musicPos[1]])
		return false;

	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 type = _partConfig_type[_musicPos[1]];

	sendByte(type, evt);
	sendByte(type, _musicPos[4]);
	sendByte(type, _musicPos[5]);

	return false;
}

bool EuphonyPlayer::event_programChange_channelAftertouch() {
	if (_musicPos[1] > 31)
		return false;
	if (!_partConfig_enable[_musicPos[1]])
		return false;

	uint8 evt = appendEvent(_musicPos[0], _musicPos[1]);
	uint8 type = _partConfig_type[_musicPos[1]];

	sendByte(type, evt);
	sendByte(type, _musicPos[4]);

	return false;
}

bool EuphonyPlayer::event_sysex() {
	uint8 type = _partConfig_type[_musicPos[1]];
	sendByte(type, 0xF0);
	proceedToNextEvent();

	for (bool loop = true; loop; ) {
		for (int i = 0; i < 6; i++) {
			if (_musicPos[i] != 0xFF) {
				sendByte(type, _musicPos[i]);
				if (_musicPos[i] >= 0x80) {
					loop = false;
					break;
				}
			}
		}
		if (loop)
			proceedToNextEvent();
	}

	return false;
}

bool EuphonyPlayer::event_advanceBar() {
	++_parseToBar;
	_barLength = _musicPos[1];
	return false;
}

bool EuphonyPlayer::event_setTempo() {
	_trackTempo = ((_musicPos[5] << 8) | ((_musicPos[4] << 1) & 0xff)) >> 1;
	sendTempo(_trackTempo);
	return false;
}

bool EuphonyPlayer::event_typeOrdrChange() {
	if (_musicPos[1] > 31)
		return false;
	if (!_partConfig_enable[_musicPos[1]])
		return false;

	if (_musicPos[4] == 1)
		_partConfig_type[_musicPos[1]] = _musicPos[5];
	else if (_musicPos[4] == 2)
		_partConfig_ordr[_musicPos[1]] = _musicPos[5];

	return false;
}

uint8 EuphonyPlayer::applyTranspose(uint8 in) {
	int out = _partConfig_transpose[_musicPos[1]];
	if (!out)
		return in;
	out += (in & 0x7f);

	if (out > 127)
		out -= 12;

	if (out < 0)
		out += 12;

	return out & 0xff;
}

uint8 EuphonyPlayer::applyVolumeAdjust(uint8 in) {
	int out = _partConfig_volume[_musicPos[1]];
	out += (in & 0x7f);
	out = CLIP(out, 1, 127);

	return out & 0xff;
}

void EuphonyPlayer::sendByte(uint8 type, uint8 command) {
	int drv = ((type >> 4) + 1) & 3;
	if (_drivers[drv])
		_drivers[drv]->send(command);
}

void EuphonyPlayer::sendPendingEvent(int type, int evt, int note, int velo) {
	if (velo)
		evt &= 0x8f;
	sendByte(type, evt);
	sendByte(type, note);
	sendByte(type, velo);
}

void EuphonyPlayer::sendControllerReset(int type, int part) {
	sendByte(type, 0xb0 | part);
	sendByte(type, 0x40);
	sendByte(type, 0);
	sendByte(type, 0xb0 | part);
	sendByte(type, 0x7b);
	sendByte(type, 0);
	sendByte(type, 0xb0 | part);
	sendByte(type, 0x79);
	sendByte(type, 0x40);
}

void EuphonyPlayer::sendAllNotesOff(int type, int part) {
	sendByte(type, 0xb0 | part);
	sendByte(type, 0x40);
	sendByte(type, 0);
}

void EuphonyPlayer::sendTempo(int tempo) {
	tempo = CLIP(tempo + _tempoModifier, 0, 500);
	if (_tempoControlMode == 0) {
		_timerSetting = 34750 / (tempo + 30);
		_updatesPerPulse = 0x10;

		while (_timerSetting < 126) {
			_timerSetting <<= 1;
			_updatesPerPulse <<= 1;
		}

		while (_timerSetting > 383) {
			_timerSetting >>= 1;
			_updatesPerPulse >>= 1;
		}

		_eupDriver->setTimerA(true, -(_timerSetting - 2));

	} else if (_tempoControlMode == 1) {
		_timerSetting = 312500 / (tempo + 30);
		_updatesPerPulse = 0x10;
		while (_timerSetting < 1105) {
			_timerSetting <<= 1;
			_updatesPerPulse <<= 1;
		}

	} else if (_tempoControlMode == 2) {
		_timerSetting = 625000 / (tempo + 30);
		_updatesPerPulseRemainder = 0;
	}
}

EuphonyDriver::EuphonyDriver(Audio::Mixer *mixer, EuphonyPlayer *pl) : EuphonyBaseDriver(), _channels(0), _partToChanMapping(0), _sustainChannels(0) {
	_intf = new TownsAudioInterface(mixer, pl);
}

EuphonyDriver::~EuphonyDriver() {
	delete _intf;
	delete[] _partToChanMapping;
	delete[] _sustainChannels;
	delete[] _channels;
}


bool EuphonyDriver::init() {
	if (!_intf->init())
		return false;

	delete[] _channels;
	delete[] _partToChanMapping;
	delete[] _sustainChannels;

	_channels = new Channel[128];
	_partToChanMapping = new int8[16];
	_sustainChannels = new int8[16];

	return true;
}

void EuphonyDriver::reset() {
	_intf->callback(0);
	_intf->callback(74);
	_intf->callback(70, 0);
	_intf->callback(75, 3);

	_currentEvent.clear();
	memset(_sustainChannels, 0, 16);
	memset(_partToChanMapping, -1, 16);

	for (int i = 0; i < 128; i++) {
		_channels[i].part = _channels[i].next = -1;
		_channels[i].note = _channels[i].pri = 0;
	}

	int e = 0;
	for (int i = 0; i < 6; i++)
		assignPartToChannel(i, e++);
	for (int i = 0x40; i < 0x48; i++)
		assignPartToChannel(i, e++);
}

int EuphonyDriver::assignPartToChannel(int chan, int part) {
	if (part > 15 || chan > 127 || chan < 0)
		return 3;

	Channel *a = &_channels[chan];
	if (a->part == part)
		return 0;

	if (a->part != -1) {
		int8 *b = &_partToChanMapping[a->part];
		while (*b != chan) {
			b = &_channels[*b].next;
			if (*b == -1 && *b != chan)
				return 3;
		}

		*b = a->next;

		if (a->note)
			_intf->callback(2, chan);

		a->part = a->next = -1;
		a->note = 0;
	}

	a->next = _partToChanMapping[part];
	_partToChanMapping[part] = chan;
	a->part = part;
	a->note = a->pri = 0;

	return 0;
}

void EuphonyDriver::send(uint8 command) {
	if (command >= 0x80) {
		_currentEvent.clear();
		_currentEvent.push_back(command >= 0xf0 ? 0 : command);
	} else if (_currentEvent[0] >= 0x80) {
		uint8 cmd = (_currentEvent[0] - 0x80) >> 4;
		_currentEvent.push_back(command);

		static const uint8 eventSize[] = { 3, 3, 3, 3, 2, 2, 3 };
		if (_currentEvent.size() != eventSize[cmd])
			return;

		switch (cmd) {
		case 0:
			noteOff();
			break;
		case 1:
			if (_currentEvent[2])
				noteOn();
			else
				noteOff();
			break;
		case 3:
			if (_currentEvent[1] == 7)
				controlChange_volume();
			else if (_currentEvent[1] == 10)
				controlChange_panPos();
			else if (_currentEvent[1] == 64)
				controlChange_allNotesOff();
			break;
		case 4:
			programChange();
			break;
		case 6:
			pitchWheel();
			break;
		default:
			break;
		}
	}
}

void EuphonyDriver::setTimerA(bool enable, int tempo) {
	_intf->callback(21, enable ? 255 : 0, tempo);
}

void EuphonyDriver::setTimerB(bool enable, int tempo) {
	_intf->callback(22, enable ? 255 : 0, tempo);
}

void EuphonyDriver::loadInstrument(int chanType, int id, const uint8 *data) {
	_intf->callback(5, chanType, id, data);
}

void EuphonyDriver::setInstrument(int chan, int instrID) {
	_intf->callback(4, chan, instrID);
}

void EuphonyDriver::loadWaveTable(const uint8 *data) {
	_intf->callback(34, data);
}

void EuphonyDriver::unloadWaveTable(int id) {
	_intf->callback(35, id);
}

void EuphonyDriver::reserveSoundEffectChannels(int num) {
	_intf->callback(33, num);
	uint32 volMask = 0;

	if (num > 8)
		return;

	for (uint32 v = 1 << 13; num; num--) {
		volMask |= v;
		v >>= 1;
	}

	_intf->setSoundEffectChanMask(volMask);
}

void EuphonyDriver::playSoundEffect(int chan, int note, int velo, const uint8 *data) {
	_intf->callback(37, chan, note, velo, data);
}

void EuphonyDriver::stopSoundEffect(int chan) {
	_intf->callback(39, chan);
}

bool EuphonyDriver::soundEffectIsPlaying(int chan) {
	return _intf->callback(40, chan) ? true : false;
}

void EuphonyDriver::channelPan(int chan, int mode) {
	_intf->callback(3, chan, mode);
}

void EuphonyDriver::channelPitch(int chan, int pitch) {
	_intf->callback(7, chan, pitch);
}

void EuphonyDriver::channelVolume(int chan, int vol) {
	_intf->callback(8, chan, vol);
}

void EuphonyDriver::setOutputVolume(int mode, int volLeft, int volRight) {
	_intf->callback(67, mode, volLeft, volRight);
}

void EuphonyDriver::cdaToggle(int a) {
	_intf->callback(73, a);
}

void EuphonyDriver::setMusicVolume(int volume) {
	_intf->setMusicVolume(volume);
}

void EuphonyDriver::setSoundEffectVolume(int volume) {
	_intf->setSoundEffectVolume(volume);
}

void EuphonyDriver::noteOff() {
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	if (*chan == -1)
		return;

	while (_channels[*chan].note != _currentEvent[1]) {
		chan = &_channels[*chan].next;
		if (*chan == -1)
			return;
	}

	if (_sustainChannels[_currentEvent[0] & 0x0f]) {
		_channels[*chan].note |= 0x80;
	} else {
		_channels[*chan].note = 0;
		_intf->callback(2, *chan);
	}
}

void EuphonyDriver::noteOn() {
	if (!_currentEvent[1])
		return;
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	if (*chan == -1)
		return;

	do {
		_channels[*chan].pri++;
		chan = &_channels[*chan].next;
	} while (*chan != -1);

	chan = &_partToChanMapping[_currentEvent[0] & 0x0f];

	int d = 0;
	int c = 0;
	bool found = false;

	do {
		if (!_channels[*chan].note) {
			found = true;
			break;
		}
		if (d <= _channels[*chan].pri) {
			c = *chan;
			d = _channels[*chan].pri;
		}
		chan = &_channels[*chan].next;
	} while (*chan != -1);

	if (found)
		c = *chan;
	else
		_intf->callback(2, c);

	_channels[c].note = _currentEvent[1];
	_channels[c].pri = 0;
	_intf->callback(1, c, _currentEvent[1], _currentEvent[2]);
}

void EuphonyDriver::controlChange_volume() {
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	while (*chan != -1) {
		_intf->callback(8, *chan, _currentEvent[2] & 0x7f);
		chan = &_channels[*chan].next;
	}
}

void EuphonyDriver::controlChange_panPos() {
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	while (*chan != -1) {
		_intf->callback(3, *chan, _currentEvent[2] & 0x7f);
		chan = &_channels[*chan].next;
	}
}

void EuphonyDriver::controlChange_allNotesOff() {
	if (_currentEvent[2] > 63) {
		_sustainChannels[_currentEvent[0] & 0x0f] = -1;
		return;
	}

	_sustainChannels[_currentEvent[0] & 0x0f] = 0;
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	while (*chan != -1) {
		if (_channels[*chan].note & 0x80) {
			_channels[*chan].note = 0;
			_intf->callback(2, *chan);
		}
		chan = &_channels[*chan].next;
	}
}

void EuphonyDriver::programChange() {
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	while (*chan != -1) {
		_intf->callback(4, *chan, _currentEvent[1]);
		_intf->callback(7, *chan, 0);
		chan = &_channels[*chan].next;
	}
}

void EuphonyDriver::pitchWheel() {
	int8 *chan = &_partToChanMapping[_currentEvent[0] & 0x0f];
	while (*chan != -1) {
		_currentEvent[1] += _currentEvent[1];
		int16 pitch = ((((_currentEvent[2] << 8) | _currentEvent[1]) >> 1) & 0x3fff) - 0x2000;
		_intf->callback(7, *chan, pitch);
		chan = &_channels[*chan].next;
	}
}

Type0Driver::Type0Driver(EuphonyPlayer *pl) : EuphonyBaseDriver() {
}

Type0Driver::~Type0Driver() {
}

bool Type0Driver::init() {
	return true;
}

void Type0Driver::send(uint8 command) {
}

#undef EUP_EVENTS_DELETE
#undef EUP_EVENTS_NEW
