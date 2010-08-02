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

#include "sound/softsynth/fmtowns_pc98/towns_audio.h"
#include "common/endian.h"


class TownsAudio_PcmChannel {
friend class TownsAudioInterface;
public:
	TownsAudio_PcmChannel();
	~TownsAudio_PcmChannel();

private:
	void loadExtData(uint8 *buffer, uint32 size);
	void setupLoop(uint32 start, uint32 len);
	void clear();

	void envAttack();
	void envDecay();
	void envSustain();
	void envRelease();

	uint8 *curInstrument;
	uint8 note;
	uint8 velo;

	int8 *data;
	int8 *dataEnd;

	int8 *loopEnd;
	uint32 loopLen;

	uint16 stepNote;
	uint16 stepPitch;
	uint16 step;

	uint8 panLeft;
	uint8 panRight;

	uint32 pos;

	uint8 envTotalLevel;
	uint8 envAttackRate;
	uint8 envDecayRate;
	uint8 envSustainLevel;
	uint8 envSustainRate;
	uint8 envReleaseRate;

	int16 envStep;
	int16 envCurrentLevel;

	EnvelopeState envState;

	int8 *extData;
};

class TownsAudio_WaveTable {
friend class TownsAudioInterface;
public:
	TownsAudio_WaveTable();
	~TownsAudio_WaveTable();

private:
	void readHeader(const uint8 *buffer);
	void readData(const uint8 *buffer);
	void clear();

	char name[9];
	int32 id;
	uint32 size;
	uint32 loopStart;
	uint32 loopLen;
	uint16 rate;
	uint16 rateOffs;
	uint16 baseNote;
	int8 *data;
};

TownsAudioInterface::TownsAudioInterface(Audio::Mixer *mixer, TownsAudioInterfacePluginDriver *driver) : TownsPC98_FmSynth(mixer, kTypeTowns),
	_fmInstruments(0), _pcmInstruments(0), _pcmChan(0), _waveTables(0), _waveTablesTotalDataSize(0),
	_baserate(55125.0f / (float)mixer->getOutputRate()), _tickLength(0), _timer(0), _drv(driver), _ready(false) {

#define INTCB(x) &TownsAudioInterface::intf_##x
	static const TownsAudioIntfCallback intfCb[] = {
		// 0
		INTCB(reset),
		INTCB(keyOn),
		INTCB(keyOff),
		INTCB(setPanPos),
		// 4
		INTCB(setInstrument),
		INTCB(loadInstrument),
		INTCB(notImpl),
		INTCB(setPitch),
		// 8
		INTCB(setLevel),
		INTCB(chanOff),
		INTCB(notImpl),
		INTCB(notImpl),
		// 12
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 16
		INTCB(notImpl),
		INTCB(writeReg),
		INTCB(notImpl),
		INTCB(bufferedWriteReg),
		// 20
		INTCB(readRegBuffer),
		INTCB(setTimerA),
		INTCB(setTimerB),
		INTCB(enableTimerA),
		// 24
		INTCB(enableTimerB),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 28
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 32
		INTCB(notImpl),
		INTCB(reserveEffectChannels),
		INTCB(loadWaveTable),
		INTCB(unloadWaveTable),
		// 36
		INTCB(notImpl),
		INTCB(pcmPlayEffect),
		INTCB(notImpl),
		INTCB(pcmChanOff),
		// 40
		INTCB(pcmEffectPlaying),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 44
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 48
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(fmKeyOn),
		INTCB(fmKeyOff),
		// 52
		INTCB(fmSetPanPos),
		INTCB(fmSetInstrument),
		INTCB(fmLoadInstrument),
		INTCB(notImpl),
		// 56
		INTCB(fmSetPitch),
		INTCB(fmSetLevel),
		INTCB(fmReset),
		INTCB(notImpl),
		// 60
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 64
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(cdaSetVolume),
		// 68
		INTCB(cdaReset),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 72
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 76
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		INTCB(notImpl),
		// 80
		INTCB(pcmUpdateEnvelopeGenerator),
		INTCB(notImpl)

	};
#undef INTCB

	_intfOpcodes = intfCb;

	memset(_fmSaveReg, 0, sizeof(_fmSaveReg));
	_timerBase = (uint32)(_baserate * 1000000.0f);
	_tickLength = 2 * _timerBase;
}

TownsAudioInterface::~TownsAudioInterface() {
	delete[] _fmSaveReg[0];
	delete[] _fmSaveReg[1];
	delete[] _fmInstruments;
	delete[] _pcmInstruments;
	delete[] _waveTables;
	delete[] _pcmChan;
}

bool TownsAudioInterface::init() {
	if (_ready)
		return true;

	if (!_drv)
		return false;

	if (!TownsPC98_FmSynth::init())
		return false;

	_fmSaveReg[0] = new uint8[256];
	_fmSaveReg[1] = new uint8[256];
	_fmInstruments = new uint8[128 * 48];
	_pcmInstruments = new uint8[32 * 128];
	_waveTables = new TownsAudio_WaveTable[128];
	_pcmChan = new TownsAudio_PcmChannel[8];

	_timer = 0;

	callback(0);

	_ready = true;
	return true;
}

int TownsAudioInterface::callback(int command, ...) {
	va_list args;
	va_start(args, command);

	if (command > 81) {
		va_end(args);
		return 4;
	}

	Common::StackLock lock(_mutex);
	int res = (this->*_intfOpcodes[command])(args);

	va_end(args);
	return res;
}

void TownsAudioInterface::nextTickEx(int32 *buffer, uint32 bufferSize) {
	if (!_ready)
		return;

	for (uint32 i = 0; i < bufferSize; i++) {
		_timer += _tickLength;
		while (_timer > 0x5B8D80) {
			_timer -= 0x5B8D80;

			for (int ii = 0; ii < 8; ii++) {
				if ((_pcmChanKeyPlaying & _chanFlags[ii]) || (_pcmChanEffectPlaying & _chanFlags[ii])) {
					TownsAudio_PcmChannel *s = &_pcmChan[ii];
					s->pos += s->step;

					if (&s->data[s->pos >> 11] >= s->loopEnd) {
						if (s->loopLen) {
							s->pos -= s->loopLen;
						} else {
							s->pos = 0;
							_pcmChanEffectPlaying &= ~_chanFlags[ii];
							_pcmChanKeyPlaying &= ~_chanFlags[ii];
						}
					}
				}
			}
		}

		int32 finOutL = 0;
		int32 finOutR = 0;

		for (int ii = 0; ii < 8; ii++) {
			if (_pcmChanOut & _chanFlags[ii]) {
				int32 o = _pcmChan[ii].data[_pcmChan[ii].pos >> 11] * _pcmChan[ii].velo;
				if (_pcmChan[ii].panLeft)
					finOutL += ((o * _pcmChan[ii].panLeft) >> 3);
				if (_pcmChan[ii].panRight)
					finOutR += ((o * _pcmChan[ii].panRight) >> 3);
				if (!((_pcmChanKeyPlaying & _chanFlags[ii]) || (_pcmChanEffectPlaying & _chanFlags[ii])))
					_pcmChanOut &= ~_chanFlags[ii];
			}
		}

		buffer[i << 1] += finOutL;
		buffer[(i << 1) + 1] += finOutR;
	}
}

void TownsAudioInterface::timerCallbackA() {
	Common::StackLock lock(_mutex);
	if (_drv && _ready)
		_drv->timerCallback(0);
}

void TownsAudioInterface::timerCallbackB() {
	Common::StackLock lock(_mutex);
	if (_ready) {
		if (_drv)
			_drv->timerCallback(1);
		for (int i = 0; i < 8; i++)
			pcmUpdateEnvelopeGenerator(i);
	}
}

int TownsAudioInterface::intf_reset(va_list &args) {
	Common::StackLock lock(_mutex);
	fmReset();
	pcmReset();
	cdaReset();
	return 0;
}

int TownsAudioInterface::intf_keyOn(va_list &args) {
	int chan = va_arg(args, int);
	int note = va_arg(args, int);
	int velo = va_arg(args, int);
	return (chan & 0x40) ? pcmKeyOn(chan, note, velo) : fmKeyOn(chan, note, velo);
}

int TownsAudioInterface::intf_keyOff(va_list &args) {
	int chan = va_arg(args, int);
	return (chan & 0x40) ? pcmKeyOff(chan) : fmKeyOff(chan);
}

int TownsAudioInterface::intf_setPanPos(va_list &args) {
	int chan = va_arg(args, int);
	int mode = va_arg(args, int);
	return (chan & 0x40) ? pcmSetPanPos(chan, mode) : fmSetPanPos(chan, mode);
}

int TownsAudioInterface::intf_setInstrument(va_list &args) {
	int chan = va_arg(args, int);
	int instrId = va_arg(args, int);
	return (chan & 0x40) ? pcmSetInstrument(chan, instrId) : fmSetInstrument(chan, instrId);
}

int TownsAudioInterface::intf_loadInstrument(va_list &args) {
	int chanType = va_arg(args, int);
	int instrId = va_arg(args, int);
	uint8 *instrData = va_arg(args, uint8 *);
	return (chanType & 0x40) ? pcmLoadInstrument(instrId, instrData) : fmLoadInstrument(instrId, instrData);
}

int TownsAudioInterface::intf_setPitch(va_list &args) {
	int chan = va_arg(args, int);
	int16 pitch = (int16)(va_arg(args, int) & 0xffff);
	return (chan & 0x40) ? pcmSetPitch(chan, pitch) : fmSetPitch(chan, pitch);
}

int TownsAudioInterface::intf_setLevel(va_list &args) {
	int chan = va_arg(args, int);
	int lvl = va_arg(args, int);
	return (chan & 0x40) ? pcmSetLevel(chan, lvl) : fmSetLevel(chan, lvl);
}

int TownsAudioInterface::intf_chanOff(va_list &args) {
	int chan = va_arg(args, int);
	return (chan & 0x40) ? pcmChanOff(chan) : fmChanOff(chan);
}

int TownsAudioInterface::intf_writeReg(va_list &args) {
	int part = va_arg(args, int) ? 1 : 0;
	int reg = va_arg(args, int);
	int val = va_arg(args, int);
	if ((!part && reg < 0x20) || (part && reg < 0x30) || (reg > 0xb6))
		return 3;

	bufferedWriteReg(part, reg, val);
	return 0;
}

int TownsAudioInterface::intf_bufferedWriteReg(va_list &args) {
	int part = va_arg(args, int) ? 1 : 0;
	int reg = va_arg(args, int);
	int val = va_arg(args, int);

	if ((!part && reg < 0x20) || (part && reg < 0x30) || (reg > 0xef))
		return 3;

	_fmSaveReg[part][reg] = val;
	return 0;
}

int TownsAudioInterface::intf_readRegBuffer(va_list &args) {
	int part = va_arg(args, int) ? 1 : 0;
	int reg = va_arg(args, int);
	uint8 *dst = va_arg(args, uint8 *);
	*dst = 0;

	if ((!part && reg < 0x20) || (part && reg < 0x30) || (reg > 0xef))
		return 3;

	*dst = _fmSaveReg[part][reg];
	return 0;
}

int TownsAudioInterface::intf_setTimerA(va_list &args) {
	int enable = va_arg(args, int);
	int tempo = va_arg(args, int);

	if (enable) {
		bufferedWriteReg(0, 0x25, tempo & 3);
		bufferedWriteReg(0, 0x24, (tempo >> 2) & 0xff);
		bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x05);
	} else {
		bufferedWriteReg(0, 0x27, (_fmSaveReg[0][0x27] & 0xfa) | 0x10);
	}

	return 0;
}

int TownsAudioInterface::intf_setTimerB(va_list &args) {
	int enable = va_arg(args, int);
	int tempo = va_arg(args, int);

	if (enable) {
		bufferedWriteReg(0, 0x26, tempo & 0xff);
		bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x0A);
	} else {
		bufferedWriteReg(0, 0x27, (_fmSaveReg[0][0x27] & 0xf5) | 0x20);
	}

	return 0;
}

int TownsAudioInterface::intf_enableTimerA(va_list &args) {
	bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x15);
	return 0;
}

int TownsAudioInterface::intf_enableTimerB(va_list &args) {
	bufferedWriteReg(0, 0x27, _fmSaveReg[0][0x27] | 0x2a);
	return 0;
}

int TownsAudioInterface::intf_reserveEffectChannels(va_list &args) {
	int numChan = va_arg(args, int);
	if (numChan > 8)
		return 3;
	if ((numChan << 13) + _waveTablesTotalDataSize > 65536)
		return 5;

	if (numChan == _numReservedChannels)
		return 0;

	if (numChan < _numReservedChannels) {
		int c = 8 - _numReservedChannels;
		for (int i = numChan; i; i--) {
			uint8 f = ~_chanFlags[c--];
			_pcmChanEffectPlaying &= f;
		}
	} else {
		int c = 7 - _numReservedChannels;
		for (int i = numChan - _numReservedChannels; i; i--) {
			uint8 f = ~_chanFlags[c--];
			_pcmChanKeyPressed &= f;
			_pcmChanKeyPlaying &= f;
		}
	}

	static const uint8 reserveChanFlags[] = { 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE, 0xFF };
	_numReservedChannels = numChan;
	_pcmChanReserved = reserveChanFlags[_numReservedChannels];

	return 0;
}

int TownsAudioInterface::intf_loadWaveTable(va_list &args) {
	uint8 *data = va_arg(args, uint8 *);
	if (_numWaveTables > 127)
		return 3;

	TownsAudio_WaveTable w;
	w.readHeader(data);
	if (!w.size)
		return 6;

	if (_waveTablesTotalDataSize + w.size > 65504)
		return 5;

	for (int i = 0; i < _numWaveTables; i++) {
		if (_waveTables[i].id == w.id)
			return 10;
	}

	TownsAudio_WaveTable *s = &_waveTables[_numWaveTables++];
	s->readHeader(data);
	s->readData(data + 32);

	_waveTablesTotalDataSize += w.size;

	return 0;
}

int TownsAudioInterface::intf_unloadWaveTable(va_list &args) {
	int id = va_arg(args, int);

	if (id == -1) {
		for (int i = 0; i < 128; i++)
			_waveTables[i].clear();
		_numWaveTables = 0;
		_waveTablesTotalDataSize = 0;
	} else {
		if (_waveTables) {
			for (int i = 0; i < _numWaveTables; i++) {
				if (_waveTables[i].id == id) {
					_numWaveTables--;
					_waveTablesTotalDataSize -= _waveTables[i].size;
					_waveTables[i].clear();
					for (; i < _numWaveTables; i++)
						memcpy(&_waveTables[i], &_waveTables[i + 1], sizeof(TownsAudio_WaveTable));
					return 0;
				}
				return 9;
			}
		}
	}

	return 0;
}

int TownsAudioInterface::intf_pcmPlayEffect(va_list &args) {
	int chan = va_arg(args, int);
	int note = va_arg(args, int);
	int velo = va_arg(args, int);
	uint8 *data = va_arg(args, uint8 *);

	if (chan < 0x40 || chan > 0x47)
		return 1;

	if (note & 0x80 || velo & 0x80)
		return 3;

	chan -= 0x40;

	if (!(_pcmChanReserved & _chanFlags[chan]))
		return 7;

	if ((_pcmChanEffectPlaying & _chanFlags[chan]))
		return 2;

	TownsAudio_WaveTable w;
	w.readHeader(data);

	if (!w.size < (w.loopStart + w.loopLen))
		return 13;

	if (!w.size)
		return 6;

	TownsAudio_PcmChannel *p = &_pcmChan[chan];

	_pcmChanNote[chan] = note;
	_pcmChanVelo[chan] = velo;

	p->note = note;
	p->velo = velo << 1;

	p->loadExtData(data + 32, w.size);
	p->setupLoop(w.loopStart, w.loopLen);

	pcmCalcPhaseStep(p, &w);
	if (p->step > 2048)
		p->step = 2048;

	_pcmChanEffectPlaying |= _chanFlags[chan];
	_pcmChanOut |= _chanFlags[chan];

	return 0;
}

int TownsAudioInterface::intf_pcmChanOff(va_list &args) {
	int chan = va_arg(args, int);
	pcmChanOff(chan);
	return 0;
}

int TownsAudioInterface::intf_pcmEffectPlaying(va_list &args) {
	int chan = va_arg(args, int);
	if (chan < 0x40 || chan > 0x47)
		return 1;
	chan -= 0x40;
	return (_pcmChanEffectPlaying & _chanFlags[chan]) ? true : false;
}

int TownsAudioInterface::intf_fmKeyOn(va_list &args) {
	int chan = va_arg(args, int);
	int note = va_arg(args, int);
	int velo = va_arg(args, int);
	return fmKeyOn(chan, note, velo);
}

int TownsAudioInterface::intf_fmKeyOff(va_list &args) {
	int chan = va_arg(args, int);
	return fmKeyOff(chan);
}

int TownsAudioInterface::intf_fmSetPanPos(va_list &args) {
	int chan = va_arg(args, int);
	int mode = va_arg(args, int);
	return fmSetPanPos(chan, mode);
}

int TownsAudioInterface::intf_fmSetInstrument(va_list &args) {
	int chan = va_arg(args, int);
	int instrId = va_arg(args, int);
	return fmSetInstrument(chan, instrId);
}

int TownsAudioInterface::intf_fmLoadInstrument(va_list &args) {
	int instrId = va_arg(args, int);
	uint8 *instrData = va_arg(args, uint8 *);
	return fmLoadInstrument(instrId, instrData);
}

int TownsAudioInterface::intf_fmSetPitch(va_list &args) {
	int chan = va_arg(args, int);
	uint16 freq = va_arg(args, int) & 0xffff;
	return fmSetPitch(chan, freq);
}

int TownsAudioInterface::intf_fmSetLevel(va_list &args) {
	int chan = va_arg(args, int);
	int lvl = va_arg(args, int);
	return fmSetLevel(chan, lvl);
}

int TownsAudioInterface::intf_fmReset(va_list &args) {
	fmReset();
	return 0;
}

int TownsAudioInterface::intf_cdaReset(va_list &args) {
	cdaReset();
	return 0;
}

int TownsAudioInterface::intf_pcmUpdateEnvelopeGenerator(va_list &args) {
	for (int i = 0; i < 8; i++)
		pcmUpdateEnvelopeGenerator(i);
	return 0;
}

int TownsAudioInterface::intf_cdaSetVolume(va_list &args) {
	/*int unk = va_arg(args, int);
	int volume1 = va_arg(args, int);
	int volume2 = va_arg(args, int);*/
	return 0;
}

int TownsAudioInterface::intf_notImpl(va_list &args) {
	return 4;
}

void TownsAudioInterface::fmReset() {
	TownsPC98_FmSynth::reset();

	_fmChanPlaying = 0;
	memset(_fmChanNote, 0, sizeof(_fmChanNote));
	memset(_fmChanPitch, 0, sizeof(_fmChanPitch));

	memset(_fmSaveReg[0], 0, 240);
	memset(&_fmSaveReg[0][240], 0x7f, 16);
	memset(_fmSaveReg[1], 0, 256);
	memset(&_fmSaveReg[1][240], 0x7f, 16);
	_fmSaveReg[0][243] = _fmSaveReg[0][247] = _fmSaveReg[0][251] = _fmSaveReg[0][255] =
	                         _fmSaveReg[1][243] = _fmSaveReg[1][247] = _fmSaveReg[1][251] = _fmSaveReg[1][255] = 0xff;

	for (int i = 0; i < 128; i++)
		fmLoadInstrument(i, _fmDefaultInstrument);

	bufferedWriteReg(0, 0x21, 0);
	bufferedWriteReg(0, 0x2C, 0x80);
	bufferedWriteReg(0, 0x2B, 0);
	bufferedWriteReg(0, 0x27, 0x30);

	for (int i = 0; i < 6; i++) {
		fmKeyOff(i);
		fmSetInstrument(i, 0);
		fmSetLevel(i, 127);
	}
}

int TownsAudioInterface::fmKeyOn(int chan, int note, int velo) {
	if (chan > 5)
		return 1;
	if (note < 12 || note > 107 || (velo & 0x80))
		return 3;
	if (_fmChanPlaying & _chanFlags[chan])
		return 2;

	_fmChanPlaying |= _chanFlags[chan];
	note -= 12;

	_fmChanNote[chan] = note;
	int16 pitch = _fmChanPitch[chan];

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	int frq = 0;
	uint8 bl = 0;

	if (note) {
		frq = _frequency[(note - 1) % 12];
		bl = (note - 1) / 12;
	} else {
		frq = 616;
	}

	frq += pitch;

	if (frq < 616) {
		if (!bl) {
			frq = 616;
		} else {
			frq += 616;
			--bl;
		}
	} else if (frq > 1232) {
		if (bl == 7) {
			frq = 15500;
		} else {
			frq -= 616;
			++bl;
		}
	}

	frq |= (bl << 11);

	bufferedWriteReg(part, chan + 0xa4, (frq >> 8) & 0xff);
	bufferedWriteReg(part, chan + 0xa0, frq & 0xff);

	velo = (velo >> 2) + 96;
	uint16 c = _carrier[_fmSaveReg[part][0xb0 + chan] & 7];
	_fmSaveReg[part][0xe0 + chan] = velo;

	for (uint8 reg = 0x40 + chan; reg < 0x50; reg += 4) {
		c += c;
		if (c & 0x100) {
			c &= 0xff;
			bufferedWriteReg(part, reg, (((((((_fmSaveReg[part][0x80 + reg] ^ 0x7f) * velo) >> 7) + 1) * _fmSaveReg[part][0xd0 + chan]) >> 7) + 1) ^ 0x7f);
		}
	}

	uint8 v = chan;
	if (part)
		v |= 4;

	for (uint8 reg = 0x80 + chan; reg < 0x90; reg += 4)
		writeReg(part, reg,	_fmSaveReg[part][reg] | 0x0f);

	writeReg(0, 0x28, v);

	for (uint8 reg = 0x80 + chan; reg < 0x90; reg += 4)
		writeReg(part, reg,	_fmSaveReg[part][reg]);

	bufferedWriteReg(0, 0x28, v | 0xf0);

	return 0;
}

int TownsAudioInterface::fmKeyOff(int chan) {
	if (chan > 5)
		return 1;
	_fmChanPlaying &= ~_chanFlags[chan];
	if (chan > 2)
		chan++;
	bufferedWriteReg(0, 0x28, chan);
	return 0;
}

int TownsAudioInterface::fmChanOff(int chan) {
	if (chan > 5)
		return 1;
	_fmChanPlaying &= ~_chanFlags[chan];

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	for (uint8 reg = 0x80 + chan; reg < 0x90; reg += 4)
		writeReg(part, reg, _fmSaveReg[part][reg] | 0x0f);

	if (part)
		chan += 4;
	writeReg(0, 0x28, chan);
	return 0;
}

int TownsAudioInterface::fmSetPanPos(int chan, int value) {
	if (chan > 5)
		return 1;

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	if (value > 0x40)
		value = 0x40;
	else if (value < 0x40)
		value = 0x80;
	else
		value = 0xC0;

	bufferedWriteReg(part, 0xb4 + chan, (_fmSaveReg[part][0xb4 + chan] & 0x3f) | value);
	return 0;
}

int TownsAudioInterface::fmSetInstrument(int chan, int instrId) {
	if (chan > 5)
		return 1;
	if (instrId > 127)
		return 3;

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	uint8 *src = &_fmInstruments[instrId * 48 + 8];

	uint16 c = _carrier[src[24] & 7];
	uint8 reg = 0x30 + chan;

	for (; reg < 0x40; reg += 4)
		bufferedWriteReg(part, reg, *src++);

	for (; reg < 0x50; reg += 4) {
		uint8 v = *src++;
		_fmSaveReg[part][0x80 + reg] = _fmSaveReg[part][reg] = v;
		c += c;
		if (c & 0x100) {
			c &= 0xff;
			v = 127;
		}
		writeReg(part, reg, v);
	}

	for (; reg < 0x90; reg += 4)
		bufferedWriteReg(part, reg, *src++);

	reg += 0x20;
	bufferedWriteReg(part, reg, *src++);

	uint8 v = *src++;
	reg += 4;
	if (v < 64)
		v |= (_fmSaveReg[part][reg] & 0xc0);
	bufferedWriteReg(part, reg, v);

	return 0;
}

int TownsAudioInterface::fmLoadInstrument(int instrId, const uint8 *data) {
	if (instrId > 127)
		return 3;
	assert(data);
	memcpy(&_fmInstruments[instrId * 48], data, 48);
	return 0;
}

int TownsAudioInterface::fmSetPitch(int chan, int pitch) {
	if (chan > 5)
		return 1;

	uint8 bl = _fmChanNote[chan];
	int frq = 0;

	if (pitch < 0) {
		if (bl) {
			if (pitch < -8008)
				pitch = -8008;
			pitch *= -1;
			pitch /= 13;
			frq = _frequency[(bl - 1) % 12] - pitch;
			bl = (bl - 1) / 12;
			_fmChanPitch[chan] = -pitch;

			if (frq < 616) {
				if (bl) {
					frq += 616;
					bl--;
				} else {
					frq = 616;
					bl = 0;
				}
			}
		} else {
			frq = 616;
			bl = 0;
		}

	} else if (pitch > 0) {
		if (bl < 96) {
			if (pitch > 8008)
				pitch = 8008;
			pitch /= 13;

			if (bl) {
				frq = _frequency[(bl - 1) % 12] + pitch;
				bl = (bl - 1) / 12;
			} else {
				frq = 616;
				bl = 0;
			}

			_fmChanPitch[chan] = pitch;

			if (frq > 1232) {
				if (bl < 7) {
					frq -= 616;
					bl++;
				} else {
					frq = 1164;
					bl = 7;
				}
			} else {
				if (bl >= 7 && frq > 1164)
					frq = 1164;
			}

		} else {
			frq = 1164;
			bl = 7;
		}
	} else {
		_fmChanPitch[chan] = 0;
		if (bl) {
			frq = _frequency[(bl - 1) % 12];
			bl = (bl - 1) / 12;
		} else {
			frq = 616;
			bl = 0;
		}
	}

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	frq |= (bl << 11);

	bufferedWriteReg(part, chan + 0xa4, (frq >> 8));
	bufferedWriteReg(part, chan + 0xa0, (frq & 0xff));

	return 0;
}

int TownsAudioInterface::fmSetLevel(int chan, int lvl) {
	if (chan > 5)
		return 1;
	if (lvl > 127)
		return 3;

	uint8 part = chan > 2 ? 1 : 0;
	if (chan > 2)
		chan -= 3;

	uint16 c = _carrier[_fmSaveReg[part][0xb0 + chan] & 7];
	_fmSaveReg[part][0xd0 + chan] = lvl;

	for (uint8 reg = 0x40 + chan; reg < 0x50; reg += 4) {
		c += c;
		if (c & 0x100) {
			c &= 0xff;
			bufferedWriteReg(part, reg, (((((((_fmSaveReg[part][0x80 + reg] ^ 0x7f) * lvl) >> 7) + 1) * _fmSaveReg[part][0xe0 + chan]) >> 7) + 1) ^ 0x7f);
		}
	}
	return 0;
}

void TownsAudioInterface::bufferedWriteReg(uint8 part, uint8 regAddress, uint8 value) {
	_fmSaveReg[part][regAddress] = value;
	writeReg(part, regAddress, value);
}

void TownsAudioInterface::pcmReset() {
	_pcmChanOut = 0;
	_pcmChanReserved = _pcmChanKeyPressed = _pcmChanEffectPlaying = _pcmChanKeyPlaying = 0;
	_numReservedChannels = 0;

	memset(_pcmChanNote, 0, 8);
	memset(_pcmChanVelo, 0, 8);
	memset(_pcmChanLevel, 0, 8);

	for (int i = 0; i < 8; i++)
		_pcmChan[i].clear();

	memset(_pcmInstruments, 0, 128 * 32);
	static uint8 name[] = { 0x4E, 0x6F, 0x20, 0x44, 0x61, 0x74, 0x61, 0x21 };
	for (int i = 0; i < 32; i++)
		memcpy(_pcmInstruments + i * 128, name, 8);

	for (int i = 0; i < 128; i++)
		_waveTables[i].clear();
	_numWaveTables = 0;
	_waveTablesTotalDataSize = 0;

	for (int i = 0x40; i < 0x48; i++) {
		pcmSetInstrument(i, 0);
		pcmSetLevel(i, 127);
	}
}

int TownsAudioInterface::pcmKeyOn(int chan, int note, int velo) {
	if (chan < 0x40 || chan > 0x47)
		return 1;

	if (note & 0x80 || velo & 0x80)
		return 3;

	chan -= 0x40;

	if ((_pcmChanReserved & _chanFlags[chan]) || (_pcmChanKeyPressed & _chanFlags[chan]))
		return 2;

	_pcmChanNote[chan] = note;
	_pcmChanVelo[chan] = velo;

	TownsAudio_PcmChannel *p = &_pcmChan[chan];
	p->note = note;

	uint8 *instr = _pcmChan[chan].curInstrument;
	int i = 0;
	for (; i < 8; i++) {
		if (note <= instr[16 + 2 * i])
			break;
	}

	if (i == 8)
		return 8;

	int il = i << 3;
	p->note += instr[il + 70];

	p->envTotalLevel = instr[il + 64];
	p->envAttackRate = instr[il + 65];
	p->envDecayRate = instr[il + 66];
	p->envSustainLevel = instr[il + 67];
	p->envSustainRate = instr[il + 68];
	p->envReleaseRate = instr[il + 69];
	p->envStep = 0;

	int32 id = (int32)READ_LE_UINT32(&instr[i * 4 + 32]);

	for (i = 0; i < _numWaveTables; i++) {
		if (id == _waveTables[i].id)
			break;
	}

	if (i == _numWaveTables)
		return 9;

	TownsAudio_WaveTable *w = &_waveTables[i];

	p->data = w->data;
	p->dataEnd = w->data + w->size;
	p->setupLoop(w->loopStart, w->loopLen);

	pcmCalcPhaseStep(p, w);

	uint32 lvl = _pcmChanLevel[chan] * _pcmChanVelo[chan];
	p->envTotalLevel = ((p->envTotalLevel * lvl) >> 14) & 0xff;
	p->envSustainLevel = ((p->envSustainLevel * lvl) >> 14) & 0xff;

	p->envAttack();
	p->velo = (p->envCurrentLevel >> 8) << 1;

	_pcmChanKeyPressed |= _chanFlags[chan];
	_pcmChanKeyPlaying |= _chanFlags[chan];
	_pcmChanOut |= _chanFlags[chan];

	return 0;
}

int TownsAudioInterface::pcmKeyOff(int chan) {
	if (chan < 0x40 || chan > 0x47)
		return 1;

	chan -= 0x40;
	_pcmChanKeyPressed &= ~_chanFlags[chan];
	_pcmChan[chan].envRelease();
	return 0;
}

int TownsAudioInterface::pcmChanOff(int chan) {
	if (chan < 0x40 || chan > 0x47)
		return 1;

	chan -= 0x40;

	_pcmChanKeyPressed &= ~_chanFlags[chan];
	_pcmChanEffectPlaying &= ~_chanFlags[chan];
	_pcmChanKeyPlaying &= ~_chanFlags[chan];
	_pcmChanOut &= ~_chanFlags[chan];

	return 0;
}

int TownsAudioInterface::pcmSetPanPos(int chan, int mode) {
	if (chan > 0x47)
		return 1;
	if (mode & 0x80)
		return 3;

	chan -= 0x40;
	uint8 blc = 0x77;

	if (mode > 64) {
		mode -= 64;
		blc = ((blc ^ (mode >> 3)) + (mode << 4)) & 0xff;
	} else if (mode < 64) {
		mode = (mode >> 3) ^ 7;
		blc = ((119 + mode) ^ (mode << 4)) & 0xff;
	}

	_pcmChan[chan].panLeft = blc & 0x0f;
	_pcmChan[chan].panRight = blc >> 4;

	return 0;
}

int TownsAudioInterface::pcmSetInstrument(int chan, int instrId) {
	if (chan > 0x47)
		return 1;
	if (instrId > 31)
		return 3;
	chan -= 0x40;
	_pcmChan[chan].curInstrument = &_pcmInstruments[instrId * 128];
	return 0;
}

int TownsAudioInterface::pcmLoadInstrument(int instrId, const uint8 *data) {
	if (instrId > 31)
		return 3;
	assert(data);
	memcpy(&_pcmInstruments[instrId * 128], data, 128);
	return 0;
}

int TownsAudioInterface::pcmSetPitch(int chan, int pitch) {
	if (chan > 0x47)
		return 1;

	if (pitch < -8192 || pitch > 8191)
		return 3;

	chan -= 0x40;
	TownsAudio_PcmChannel *p = &_pcmChan[chan];

	uint32 pts = 0x4000;

	if (pitch < 0)
		pts = (0x20000000 / (-pitch + 0x2001)) >> 2;
	else if (pitch > 0)
		pts = (((pitch + 0x2001) << 16) / 0x2000) >> 2;

	p->stepPitch = pts & 0xffff;
	p->step = (p->stepNote * p->stepPitch) >> 14;

//	if (_pcmChanUnkFlag & _chanFlags[chan])
//		 unk[chan] = (((p->step * 1000) << 11) / 98) / 20833;

	/*else*/
	if ((_pcmChanEffectPlaying & _chanFlags[chan]) && (p->step > 2048))
		p->step = 2048;

	return 0;
}

int TownsAudioInterface::pcmSetLevel(int chan, int lvl) {
	if (chan > 0x47)
		return 1;

	if (lvl & 0x80)
		return 3;

	chan -= 0x40;
	TownsAudio_PcmChannel *p = &_pcmChan[chan];

	if (_pcmChanReserved & _chanFlags[chan]) {
		_pcmChanVelo[chan] = lvl;
		p->velo = lvl << 1;
	} else {
		int32 t = p->envStep * lvl;
		if (_pcmChanLevel[chan])
			t /= _pcmChanLevel[chan];
		p->envStep = t;
		t = p->envCurrentLevel * lvl;
		if (_pcmChanLevel[chan])
			t /= _pcmChanLevel[chan];
		p->envCurrentLevel = t;
		_pcmChanLevel[chan] = lvl;
		p->velo = p->envCurrentLevel >> 8;
	}

	return 0;
}

void TownsAudioInterface::pcmUpdateEnvelopeGenerator(int chan) {
	TownsAudio_PcmChannel *p = &_pcmChan[chan];
	if (!p->envCurrentLevel) {
		_pcmChanKeyPlaying &= ~_chanFlags[chan];
		p->envState = kEnvReady;
	}

	if (!(_pcmChanKeyPlaying & _chanFlags[chan]))
		return;

	switch (p->envState) {
	case kEnvAttacking:
		if (((p->envCurrentLevel + p->envStep) >> 8) > p->envTotalLevel) {
			p->envDecay();
			return;
		} else {
			p->envCurrentLevel += p->envStep;
		}
		break;

	case kEnvDecaying:
		if (((p->envCurrentLevel - p->envStep) >> 8) < p->envSustainLevel) {
			p->envSustain();
			return;
		} else {
			p->envCurrentLevel -= p->envStep;
		}
		break;

	case kEnvSustaining:
	case kEnvReleasing:
		p->envCurrentLevel -= p->envStep;
		if (p->envCurrentLevel <= 0)
			p->envCurrentLevel = 0;
		break;

	default:
		break;
	}
	p->velo = (p->envCurrentLevel >> 8) << 1;
}

void TownsAudioInterface::pcmCalcPhaseStep(TownsAudio_PcmChannel *p, TownsAudio_WaveTable *w) {
	int8 diff = p->note - w->baseNote;
	uint16 r = w->rate + w->rateOffs;
	uint16 bl = 0;
	uint32 s = 0;

	if (diff < 0) {
		diff -= 1;
		bl = diff % 12;
		diff /= 12;
		s = (r >> diff);
		if (bl)
			s = (s * _pcmPhase2[bl]) >> 16;

	} else if (diff > 0) {
		bl = diff % 12;
		diff /= 12;
		s = (r << diff);
		if (bl)
			s += ((s * _pcmPhase1[bl]) >> 16);

	} else {
		s = r;
	}

	p->stepNote = s & 0xffff;
	p->step = (s * p->stepPitch) >> 14;
}

void TownsAudioInterface::cdaReset() {

}

const uint8 TownsAudioInterface::_chanFlags[] = {
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

const uint16 TownsAudioInterface::_frequency[] = {
	0x028C, 0x02B4, 0x02DC, 0x030A, 0x0338, 0x0368, 0x039C, 0x03D4, 0x040E, 0x044A, 0x048C, 0x04D0
};

const uint8 TownsAudioInterface::_carrier[] = {
	0x10, 0x10, 0x10, 0x10, 0x30, 0x70, 0x70, 0xF0
};

const uint8 TownsAudioInterface::_fmDefaultInstrument[] = {
	0x45, 0x4C, 0x45, 0x50, 0x49, 0x41, 0x4E, 0x4F, 0x01, 0x0A, 0x02, 0x01,
	0x1E, 0x32, 0x05, 0x00, 0x9C, 0xDC, 0x9C, 0xDC, 0x07, 0x03, 0x14, 0x08,
	0x00, 0x03, 0x05, 0x05, 0x55, 0x45, 0x27, 0xA7, 0x04, 0xC0, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint16 TownsAudioInterface::_pcmPhase1[] =  {
	0x879B, 0x0F37, 0x1F58, 0x306E, 0x4288, 0x55B6, 0x6A08, 0x7F8F, 0x965E, 0xAE88, 0xC882, 0xE341
};

const uint16 TownsAudioInterface::_pcmPhase2[] =  {
	0xFEFE, 0xF1A0, 0xE411, 0xD744, 0xCB2F, 0xBFC7, 0xB504, 0xAAE2, 0xA144, 0x9827, 0x8FAC
};

TownsAudio_PcmChannel::TownsAudio_PcmChannel() {
	extData = 0;
	clear();
}

TownsAudio_PcmChannel::~TownsAudio_PcmChannel() {
	clear();
}

void TownsAudio_PcmChannel::loadExtData(uint8 *buffer, uint32 size) {
	delete[] extData;
	extData = new int8[size];
	int8 *src = (int8 *)buffer;
	int8 *dst = extData;
	for (uint32 i = 0; i < size; i++)
		*dst++ = *src & 0x80 ? (*src++ & 0x7f) : -*src++;

	data = extData;
	dataEnd = extData + size;
	pos = 0;
}

void TownsAudio_PcmChannel::setupLoop(uint32 start, uint32 len) {
	loopLen = len << 11;
	loopEnd = loopLen ? &data[(start + loopLen) >> 11] : dataEnd;
	pos = start;
}

void TownsAudio_PcmChannel::clear() {
	curInstrument = 0;
	note = 0;
	velo = 0;

	data = 0;
	dataEnd = 0;
	loopLen = 0;

	pos = 0;
	loopEnd = 0;

	step = 0;
	stepNote = 0x4000;
	stepPitch = 0x4000;

	panLeft = panRight = 0;

	envTotalLevel = envAttackRate = envDecayRate = envSustainLevel =
	                                    envSustainRate = envReleaseRate = 0;
	envStep = envCurrentLevel = 0;

	envState = kEnvReady;

	delete[] extData;
	extData = 0;
}

void TownsAudio_PcmChannel::envAttack() {
	envState = kEnvAttacking;
	int16 t = envTotalLevel << 8;
	if (envAttackRate == 127) {
		envStep = 0;
	} else if (envAttackRate) {
		envStep = t / envAttackRate;
		envCurrentLevel = 1;
	} else {
		envCurrentLevel = t;
		envDecay();
	}
}

void TownsAudio_PcmChannel::envDecay() {
	envState = kEnvDecaying;
	int16 t = envTotalLevel - envSustainLevel;
	if (t < 0 || envDecayRate == 127) {
		envStep = 0;
	} else if (envDecayRate) {
		envStep = (t << 8) / envDecayRate;
	} else {
		envCurrentLevel = envSustainLevel << 8;
		envSustain();
	}
}

void TownsAudio_PcmChannel::envSustain() {
	envState = kEnvSustaining;
	if (envSustainLevel && envSustainRate)
		envStep = (envSustainRate == 127) ? 0 : (envCurrentLevel / envSustainRate) >> 1;
	else
		envStep = envCurrentLevel = 1;
}

void TownsAudio_PcmChannel::envRelease() {
	envState = kEnvReleasing;
	if (envReleaseRate == 127)
		envStep = 0;
	else if (envReleaseRate)
		envStep = envCurrentLevel / envReleaseRate;
	else
		envStep = envCurrentLevel = 1;
}

TownsAudio_WaveTable::TownsAudio_WaveTable() {
	data = 0;
	clear();
}

TownsAudio_WaveTable::~TownsAudio_WaveTable() {
	clear();
}

void TownsAudio_WaveTable::readHeader(const uint8 *buffer) {
	memcpy(name, buffer, 8);
	name[8] = 0;
	id = READ_LE_UINT32(&buffer[8]);
	size = READ_LE_UINT32(&buffer[12]);
	loopStart = READ_LE_UINT32(&buffer[16]);
	loopLen = READ_LE_UINT32(&buffer[20]);
	rate = READ_LE_UINT16(&buffer[24]);
	rateOffs = READ_LE_UINT16(&buffer[26]);
	baseNote = READ_LE_UINT32(&buffer[28]);
}

void TownsAudio_WaveTable::readData(const uint8 *buffer) {
	if (!size)
		return;

	delete[] data;
	data = new int8[size];

	const int8 *src = (const int8 *)buffer;
	int8 *dst = data;
	for (uint32 i = 0; i < size; i++)
		*dst++ = *src & 0x80 ? (*src++ & 0x7f) : -*src++;
}

void TownsAudio_WaveTable::clear() {
	name[0] = name[8] = 0;
	id = -1;
	size = 0;
	loopStart = 0;
	loopLen = 0;
	rate = 0;
	rateOffs = 0;
	baseNote = 0;
	delete[] data;
	data = 0;
}

