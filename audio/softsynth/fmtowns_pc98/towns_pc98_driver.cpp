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

#include "audio/softsynth/fmtowns_pc98/towns_pc98_driver.h"
#include "common/endian.h"
#include "common/textconsole.h"
#include "common/func.h"
#include "common/array.h"

class TownsPC98_MusicChannel {
public:
	TownsPC98_MusicChannel(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_MusicChannel();

	typedef enum channelState {
		CHS_RECALCFREQ = 0x01,
		CHS_KEYOFF = 0x02,
		CHS_SSGOFF = 0x04,
		CHS_VBROFF = 0x08,
		CHS_ALLOFF = 0x0f,
		CHS_PROTECT = 0x40,
		CHS_EOT = 0x80
	} ChannelState;

	virtual void reset();
	virtual void loadData(uint8 *data);
	virtual void processEvents();
	virtual void processFrequency();

	virtual void fadeStep();

	const uint8 _idFlag;

protected:
	void setupVibrato();
	bool processVibrato();

	uint8 readReg(uint8 part, uint8 reg);
	void writeReg(uint8 part, uint8 reg, uint8 val);

	bool control_dummy(uint8 para);
	bool control_f2_duration(uint8 para);
	bool control_f3_pitchBend(uint8 para);
	bool control_f5_tempo(uint8 para);
	bool control_f6_repeatSection(uint8 para);
	bool control_f7_setupVibrato(uint8 para);
	bool control_f8_toggleVibrato(uint8 para);
	bool control_fa_writeReg(uint8 para);
	bool control_fd_jump(uint8 para);

	uint8 _ticksLeft;
	uint8 _duration;
	uint8 _instr;
	uint8 _totalLevel;
	uint8 _frqBlockMSB;
	uint16 _frequency;
	uint8 _block;
	int8 _pitchBend;
	uint8 _regOffset;
	uint8 _flags;
	uint8 *_dataPtr;
	bool _sustain;
	bool _fading;

	TownsPC98_AudioDriver *_driver;
	const uint8 _chanNum;

	static const uint8 _controlEventSize[16];

private:
	void keyOn();
	void keyOff();

	void setOutputLevel();
	bool processControlEvent(uint8 cmd);

	bool control_f0_setPatch(uint8 para);
	bool control_f1_presetOutputLevel(uint8 para);
	bool control_f4_setOutputLevel(uint8 para);
	bool control_fb_incOutLevel(uint8 para);
	bool control_fc_decOutLevel(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	uint8 _vbrInitDelayHi;
	uint8 _vbrInitDelayLo;
	int16 _vbrModInitVal;
	uint8 _vbrDuration;
	uint8 _vbrCurDelay;
	int16 _vbrModCurVal;
	uint8 _vbrDurLeft;
	uint8 _algorithm;

	const uint8 _keyNum;
	const uint8 _part;

	typedef Common::Functor1Mem<uint8, bool, TownsPC98_MusicChannel> ControlEvent;
	Common::Array<const ControlEvent*> _controlEvents;
};

class TownsPC98_MusicChannelSSG : public TownsPC98_MusicChannel {
public:
	TownsPC98_MusicChannelSSG(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_MusicChannelSSG();

	virtual void reset();
	virtual void loadData(uint8 *data);
	void processEvents();
	void processFrequency();

	void protect();
	void restore();

	void fadeStep();

protected:
	void keyOn();
	void nextShape();

	void setOutputLevel(uint8 lvl);
	bool processControlEvent(uint8 cmd);

	bool control_f0_setPatch(uint8 para);
	bool control_f1_setTotalLevel(uint8 para);
	bool control_f4_setAlgorithm(uint8 para);
	bool control_f9_loadCustomPatch(uint8 para);
	bool control_fb_incOutLevel(uint8 para);
	bool control_fc_decOutLevel(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	uint8 _ssgTl;
	uint8 _ssgStep;
	uint8 _ssgTicksLeft;
	uint8 _ssgTargetLvl;
	uint8 _ssgStartLvl;
	uint8 _algorithm;

	static uint8 *_envPatchData;
	static const uint8 _envData[256];

	typedef Common::Functor1Mem<uint8, bool, TownsPC98_MusicChannelSSG> ControlEvent;
	Common::Array<const ControlEvent*> _controlEvents;
};

class TownsPC98_SfxChannel : public TownsPC98_MusicChannelSSG {
public:
	TownsPC98_SfxChannel(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
		TownsPC98_MusicChannelSSG(driver, regOffs, flgs, num, key, prt, id) {}
	virtual ~TownsPC98_SfxChannel() {}

	void reset();
	void loadData(uint8 *data);
};

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
class TownsPC98_MusicChannelPCM : public TownsPC98_MusicChannel {
public:
	TownsPC98_MusicChannelPCM(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id);
	virtual ~TownsPC98_MusicChannelPCM();

	void loadData(uint8 *data);
	void processEvents();

private:
	bool processControlEvent(uint8 cmd);
	bool control_f1_prcStart(uint8 para);
	bool control_ff_endOfTrack(uint8 para);

	typedef Common::Functor1Mem<uint8, bool, TownsPC98_MusicChannelPCM> ControlEvent;
	Common::Array<const ControlEvent*> _controlEvents;
};
#endif

#define CONTROL(x) _controlEvents.push_back(new ControlEvent(this, &TownsPC98_MusicChannel::control_##x))
TownsPC98_MusicChannel::TownsPC98_MusicChannel(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) : _driver(driver),
_regOffset(regOffs), _flags(flgs), _chanNum(num), _keyNum(key),	_part(prt), _idFlag(id), _ticksLeft(0), _algorithm(0), _instr(0), _totalLevel(0),
_frqBlockMSB(0), _duration(0), _block(0), _vbrInitDelayHi(0), _vbrInitDelayLo(0), _vbrDuration(0), _vbrCurDelay(0), _vbrDurLeft(0), _pitchBend(0),
_sustain(false), _fading(false), _dataPtr(0), _vbrModInitVal(0), _vbrModCurVal(0), _frequency(0) {
	CONTROL(f0_setPatch);
	CONTROL(f1_presetOutputLevel);
	CONTROL(f2_duration);
	CONTROL(f3_pitchBend);
	CONTROL(f4_setOutputLevel);
	CONTROL(f5_tempo);
	CONTROL(f6_repeatSection);
	CONTROL(f7_setupVibrato);
	CONTROL(f8_toggleVibrato);
	CONTROL(dummy);
	CONTROL(fa_writeReg);
	CONTROL(fb_incOutLevel);
	CONTROL(fc_decOutLevel);
	CONTROL(fd_jump);
	CONTROL(dummy);
	CONTROL(ff_endOfTrack);
}
#undef CONTROL

TownsPC98_MusicChannel::~TownsPC98_MusicChannel() {
	for (Common::Array<const ControlEvent*>::iterator i = _controlEvents.begin(); i != _controlEvents.end(); ++i)
		delete *i;
}

void TownsPC98_MusicChannel::reset() {
	_sustain = false;
	_duration = 0;
	_fading = false;
	_ticksLeft = 1;

	_flags = (_flags & ~CHS_EOT) | CHS_ALLOFF;

	_totalLevel = 0;
	_algorithm = 0;

	_block = 0;
	_frequency = 0;
	_frqBlockMSB = 0;
	_pitchBend = 0;

	_vbrInitDelayHi = 0;
	_vbrInitDelayLo = 0;
	_vbrModInitVal = 0;
	_vbrDuration = 0;
	_vbrCurDelay = 0;
	_vbrModCurVal = 0;
	_vbrDurLeft = 0;
}

void TownsPC98_MusicChannel::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALLOFF;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;

	uint8 *tmp = _dataPtr;
	for (bool loop = true; loop;) {
		uint8 cmd = *tmp++;
		if (cmd < 0xf0) {
			tmp++;
		} else if (cmd == 0xff) {
			if (READ_LE_UINT16(tmp)) {
				_driver->_looping |= _idFlag;
				tmp += _controlEventSize[cmd - 240];
			} else
				loop = false;
		} else if (cmd == 0xf6) {
			// reset repeat section countdown
			tmp[0] = tmp[1];
			tmp += 4;
		} else {
			tmp += _controlEventSize[cmd - 240];
		}
	}
}

void TownsPC98_MusicChannel::processEvents() {
	if (_flags & CHS_EOT)
		return;

	if (!_sustain && _ticksLeft == _duration)
		keyOff();

	if (--_ticksLeft)
		return;

	if (!_sustain)
		keyOff();

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd < 0xf0)
			loop = false;
		else if (!processControlEvent(cmd))
			return;
	}

	uint8 para = *_dataPtr++;

	if (cmd == 0x80) {
		keyOff();
		_sustain = false;
	} else {
		keyOn();

		if (_sustain == false || cmd != _frqBlockMSB)
			_flags |= CHS_RECALCFREQ;

		_sustain = (para & 0x80) ? true : false;
		_frqBlockMSB = cmd;
	}

	_ticksLeft = para & 0x7f;
}

void TownsPC98_MusicChannel::processFrequency() {
	static const uint16 noteFrequencies[] = { 0x26a, 0x28f, 0x2b6, 0x2df, 0x30b, 0x339, 0x36a, 0x39e, 0x3d5, 0x410, 0x44e, 0x48f };

	if (_flags & CHS_RECALCFREQ) {

		_frequency = (noteFrequencies[_frqBlockMSB & 0x0f] + _pitchBend) | (((_frqBlockMSB & 0x70) >> 1) << 8);

		writeReg(_part, _regOffset + 0xa4, (_frequency >> 8));
		writeReg(_part, _regOffset + 0xa0, (_frequency & 0xff));

		setupVibrato();
	}

	if (!(_flags & CHS_VBROFF)) {
		if (!processVibrato())
			return;

		writeReg(_part, _regOffset + 0xa4, (_frequency >> 8));
		writeReg(_part, _regOffset + 0xa0, (_frequency & 0xff));
	}
}

void TownsPC98_MusicChannel::fadeStep() {
	_fading = true;
	_totalLevel += 3;
	if (_totalLevel > 0x7f)
		_totalLevel = 0x7f;
	setOutputLevel();
}

void TownsPC98_MusicChannel::setupVibrato() {
	_vbrCurDelay = _vbrInitDelayHi;
	if (_flags & CHS_KEYOFF) {
		_vbrModCurVal = _vbrModInitVal;
		_vbrCurDelay += _vbrInitDelayLo;
	}
	_vbrDurLeft = (_vbrDuration >> 1);
	_flags &= ~(CHS_KEYOFF | CHS_RECALCFREQ);
}

bool TownsPC98_MusicChannel::processVibrato() {
	if (--_vbrCurDelay)
		return false;

	_vbrCurDelay = _vbrInitDelayHi;
	_frequency += _vbrModCurVal;

	if (!--_vbrDurLeft) {
		_vbrDurLeft = _vbrDuration;
		_vbrModCurVal = -_vbrModCurVal;
	}

	return true;
}

uint8 TownsPC98_MusicChannel::readReg(uint8 part, uint8 reg) {
	return _driver->readReg(part, reg);
}

void TownsPC98_MusicChannel::writeReg(uint8 part, uint8 reg, uint8 val) {
	_driver->writeReg(part, reg, val);
}

bool TownsPC98_MusicChannel::control_dummy(uint8 para) {
	_dataPtr--;
	return true;
}

bool TownsPC98_MusicChannel::control_f2_duration(uint8 para) {
	_duration = para;
	return true;
}

bool TownsPC98_MusicChannel::control_f3_pitchBend(uint8 para) {
	_pitchBend = (int8) para;
	return true;
}

bool TownsPC98_MusicChannel::control_f5_tempo(uint8 para) {
	_driver->setMusicTempo(para);
	return true;
}

bool TownsPC98_MusicChannel::control_f6_repeatSection(uint8 para) {
	_dataPtr--;
	_dataPtr[0]--;

	if (*_dataPtr) {
		// repeat section until counter has reached zero
		_dataPtr = _driver->_trackPtr + READ_LE_UINT16(_dataPtr + 2);
	} else {
		// reset counter, advance to next section
		_dataPtr[0] = _dataPtr[1];
		_dataPtr += 4;
	}
	return true;
}

bool TownsPC98_MusicChannel::control_f7_setupVibrato(uint8 para) {
	_vbrInitDelayHi = _dataPtr[0];
	_vbrInitDelayLo = para;
	_vbrModInitVal = (int16) READ_LE_UINT16(_dataPtr + 1);
	_vbrDuration = _dataPtr[3];
	_dataPtr += 4;
	_flags = (_flags & ~CHS_VBROFF) | CHS_KEYOFF | CHS_RECALCFREQ;
	return true;
}

bool TownsPC98_MusicChannel::control_f8_toggleVibrato(uint8 para) {
	if (para == 0x10) {
		if (*_dataPtr++) {
			_flags = (_flags & ~CHS_VBROFF) | CHS_KEYOFF;
		} else {
			_flags |= CHS_VBROFF;
		}
	} else {
		/* NOT IMPLEMENTED
		uint8 skipChannels = para / 36;
		uint8 entry = para % 36;
		TownsPC98_AudioDriver::TownsPC98_MusicChannel *t = &chan[skipChannels];

		t->unnamedEntries[entry] = *_dataPtr++;*/
	}
	return true;
}

bool TownsPC98_MusicChannel::control_fa_writeReg(uint8 para) {
	writeReg(_part, para, *_dataPtr++);
	return true;
}

bool TownsPC98_MusicChannel::control_fd_jump(uint8 para) {
	uint8 *tmp = _driver->_trackPtr + READ_LE_UINT16(_dataPtr - 1);
	_dataPtr = (tmp[1] == 1) ? tmp : (_dataPtr + 1);
	return true;
}

void TownsPC98_MusicChannel::keyOn() {
	// all operators on
	uint8 value = _keyNum | 0xf0;
	if (_part)
		value |= 4;
	uint8 regAddress = 0x28;
	writeReg(0, regAddress, value);
}

void TownsPC98_MusicChannel::keyOff() {
	// all operators off
	uint8 value = _keyNum & 0x0f;
	if (_part)
		value |= 4;
	uint8 regAddress = 0x28;
	writeReg(0, regAddress, value);
	_flags |= CHS_KEYOFF;
}

void TownsPC98_MusicChannel::setOutputLevel() {
	static const uint8 carrier[] = { 0x08, 0x08, 0x08, 0x08, 0x0C, 0x0E, 0x0E, 0x0F };
	uint8 outopr = carrier[_algorithm];
	uint8 reg = 0x40 + _regOffset;

	for (int i = 0; i < 4; i++) {
		if (outopr & 1)
			writeReg(_part, reg, _totalLevel);
		outopr >>= 1;
		reg += 4;
	}
}

bool TownsPC98_MusicChannel::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (*_controlEvents[cmd & 0x0f])(para);
}

bool TownsPC98_MusicChannel::control_f0_setPatch(uint8 para) {
	_instr = para;
	uint8 reg = _regOffset + 0x80;

	for (int i = 0; i < 4; i++) {
		// set release rate for each operator
		writeReg(_part, reg, 0x0f);
		reg += 4;
	}

	const uint8 *tptr = _driver->_patchData + ((uint32)_instr << 5);
	reg = _regOffset + 0x30;

	// write registers 0x30 to 0x8f
	for (int i = 0; i < 6; i++) {
		writeReg(_part, reg, tptr[0]);
		reg += 4;
		writeReg(_part, reg, tptr[2]);
		reg += 4;
		writeReg(_part, reg, tptr[1]);
		reg += 4;
		writeReg(_part, reg, tptr[3]);
		reg += 4;
		tptr += 4;
	}

	reg = _regOffset + 0xB0;
	_algorithm = tptr[0] & 7;
	// set feedback and algorithm
	writeReg(_part, reg, tptr[0]);

	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_f1_presetOutputLevel(uint8 para) {
	if (_fading)
		return true;

	_totalLevel = _driver->_levelPresets[para];
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_f4_setOutputLevel(uint8 para) {
	if (_fading)
		return true;

	_totalLevel = para;
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_fading)
		return true;

	uint8 val = (_totalLevel + 3);
	if (val > 0x7f)
		val = 0x7f;

	_totalLevel = val;
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_fading)
		return true;

	int8 val = (int8)(_totalLevel - 3);
	if (val < 0)
		val = 0;

	_totalLevel = (uint8)val;
	setOutputLevel();
	return true;
}

bool TownsPC98_MusicChannel::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _driver->_trackPtr + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_driver->_finishedChannelsFlag |= _idFlag;
		keyOff();
		return false;
	}
}

const uint8 TownsPC98_MusicChannel::_controlEventSize[16] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x05, 0x02, 0x06, 0x02, 0x00, 0x00, 0x02, 0x00, 0x02 };

#define CONTROL(x) _controlEvents.push_back(new ControlEvent(this, &TownsPC98_MusicChannelSSG::control_##x))
TownsPC98_MusicChannelSSG::TownsPC98_MusicChannelSSG(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
TownsPC98_MusicChannel(driver, regOffs, flgs, num, key, prt, id), _algorithm(0x80),
	_ssgStartLvl(0), _ssgTl(0), _ssgStep(0), _ssgTicksLeft(0), _ssgTargetLvl(0) {
	CONTROL(f0_setPatch);
	CONTROL(f1_setTotalLevel);
	CONTROL(f2_duration);
	CONTROL(f3_pitchBend);
	CONTROL(f4_setAlgorithm);
	CONTROL(f5_tempo);
	CONTROL(f6_repeatSection);
	CONTROL(f7_setupVibrato);
	CONTROL(f8_toggleVibrato);
	CONTROL(f9_loadCustomPatch);
	CONTROL(fa_writeReg);
	CONTROL(fb_incOutLevel);
	CONTROL(fc_decOutLevel);
	CONTROL(fd_jump);
	CONTROL(dummy);
	CONTROL(ff_endOfTrack);

	if (!_envPatchData) {
		_envPatchData = new uint8[256];
		memcpy(_envPatchData, _envData, 256);
	}
}
#undef CONTROL

TownsPC98_MusicChannelSSG::~TownsPC98_MusicChannelSSG() {
	for (Common::Array<const ControlEvent*>::iterator i = _controlEvents.begin(); i != _controlEvents.end(); ++i)
		delete *i;
	delete[] _envPatchData;
	_envPatchData = 0;
}

void TownsPC98_MusicChannelSSG::reset() {
	TownsPC98_MusicChannel::reset();
	_ssgStartLvl = _ssgTl = _ssgStep = _ssgTicksLeft = _ssgTargetLvl = 0;

	// Unlike the original we restore the default patch data. This fixes a bug
	// where certain sound effects would bring each other out of tune (e.g. the
	// dragon's fire in Darm's house in Kyra 1 would sound different each time
	// you triggered another sfx by dropping an item etc.)
	uint8 i = (10 + _regOffset) << 4;
	const uint8 *src = _envData;
	_envPatchData[i] = src[i];
	_envPatchData[i + 3] = src[i + 3];
	_envPatchData[i + 4] = src[i + 4];
	_envPatchData[i + 6] = src[i + 6];
	_envPatchData[i + 8] = src[i + 8];
	_envPatchData[i + 12] = src[i + 12];
}

void TownsPC98_MusicChannelSSG::loadData(uint8 *data) {
	_driver->preventRegisterWrite(_flags & CHS_PROTECT ? true : false);
	TownsPC98_MusicChannel::loadData(data);
	setOutputLevel(0);
	_algorithm = 0x80;
}

void TownsPC98_MusicChannelSSG::processEvents() {
	if (_flags & CHS_EOT)
		return;

	_driver->preventRegisterWrite(_flags & CHS_PROTECT ? true : false);

	if (!_sustain && _ticksLeft == _duration)
		nextShape();

	if (!--_ticksLeft) {

		uint8 cmd = 0;
		bool loop = true;

		while (loop) {
			cmd = *_dataPtr++;
			if (cmd < 0xf0)
				loop = false;
			else if (!processControlEvent(cmd))
				return;
		}

		uint8 para = *_dataPtr++;

		if (cmd == 0x80) {
			nextShape();
			_sustain = false;
		} else {
			if (!_sustain) {
				_instr &= 0xf0;
				_ssgStep = _envPatchData[_instr];
				_ssgTicksLeft = _envPatchData[_instr + 1] & 0x7f;
				_ssgTargetLvl = _envPatchData[_instr + 2];
				_ssgStartLvl = _envPatchData[_instr + 3];
				_flags = (_flags & ~CHS_SSGOFF) | CHS_KEYOFF;
			}

			keyOn();

			if (_sustain == false || cmd != _frqBlockMSB)
				_flags |= CHS_RECALCFREQ;

			_sustain = (para & 0x80) ? true : false;
			_frqBlockMSB = cmd;
		}

		_ticksLeft = para & 0x7f;
	}

	if (!(_flags & CHS_SSGOFF)) {
		if (--_ssgTicksLeft) {
			if (!_driver->_fading)
				setOutputLevel(_ssgStartLvl);
			return;
		}

		_ssgTicksLeft = _envPatchData[_instr + 1] & 0x7f;

		if (_envPatchData[_instr + 1] & 0x80) {
			uint8 t = _ssgStartLvl - _ssgStep;

			if (_ssgStep <= _ssgStartLvl && _ssgTargetLvl < t) {
				if (!_driver->_fading)
					setOutputLevel(t);
				return;
			}
		} else {
			int t = _ssgStartLvl + _ssgStep;
			uint8 p = (uint8)(t & 0xff);

			if (t < 256 && _ssgTargetLvl > p) {
				if (!_driver->_fading)
					setOutputLevel(p);
				return;
			}
		}

		setOutputLevel(_ssgTargetLvl);
		if (_ssgStartLvl && !(_instr & 8)) {
			_instr += 4;
			_ssgStep = _envPatchData[_instr];
			_ssgTicksLeft = _envPatchData[_instr + 1] & 0x7f;
			_ssgTargetLvl = _envPatchData[_instr + 2];
		} else {
			_flags |= CHS_SSGOFF;
			setOutputLevel(0);
		}
	}
}

void TownsPC98_MusicChannelSSG::processFrequency() {
	static const uint16 noteFrequencies[] = { 0xee8, 0xe12, 0xd48, 0xc89, 0xbd5, 0xb2b, 0xa8a, 0x9f3, 0x964, 0x8dd, 0x85e, 0x7e6 };

	if (_algorithm & 0x40)
		return;

	if (_flags & CHS_RECALCFREQ) {
		_block = _frqBlockMSB >> 4;
		_frequency = noteFrequencies[_frqBlockMSB & 0x0f] + _pitchBend;

		uint16 f = _frequency >> _block;
		writeReg(0, _regOffset << 1, f & 0xff);
		writeReg(0, (_regOffset << 1) + 1, f >> 8);

		setupVibrato();
	}

	if (!(_flags & (CHS_EOT | CHS_VBROFF | CHS_SSGOFF))) {
		if (!processVibrato())
			return;

		uint16 f = _frequency >> _block;
		writeReg(0, _regOffset << 1, f & 0xff);
		writeReg(0, (_regOffset << 1) + 1, f >> 8);
	}
}

void TownsPC98_MusicChannelSSG::protect() {
	_flags |= CHS_PROTECT;
}

void TownsPC98_MusicChannelSSG::restore() {
	_flags &= ~CHS_PROTECT;
	keyOn();
	writeReg(0, 8 + _regOffset, _ssgTl);
	uint16 f = _frequency >> _block;
	writeReg(0, _regOffset << 1, f & 0xff);
	writeReg(0, (_regOffset << 1) + 1, f >> 8);
}

void TownsPC98_MusicChannelSSG::fadeStep() {
	_fading = true;
	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;
	setOutputLevel(_ssgStartLvl);
}

void TownsPC98_MusicChannelSSG::keyOn() {
	uint8 c = 0x7b;
	uint8 t = (_algorithm & 0xC0) << 1;
	if (_algorithm & 0x80)
		t |= 4;

	c = (c << (_regOffset + 1)) | (c >> (7 - _regOffset));
	t = (t << (_regOffset + 1)) | (t >> (7 - _regOffset));

	if (!(_algorithm & 0x80))
		writeReg(0, 6, _algorithm & 0x7f);

	uint8 e = (readReg(0, 7) & c) | t;
	writeReg(0, 7, e);
}

void TownsPC98_MusicChannelSSG::nextShape() {
	_instr = (_instr & 0xf0) + 0x0c;
	_ssgStep = _envPatchData[_instr];
	_ssgTicksLeft = _envPatchData[_instr + 1] & 0x7f;
	_ssgTargetLvl = _envPatchData[_instr + 2];
}

void TownsPC98_MusicChannelSSG::setOutputLevel(uint8 lvl) {
	_ssgStartLvl = lvl;
	uint16 newTl = (((uint16)_totalLevel + 1) * (uint16)lvl) >> 8;
	if (newTl == _ssgTl)
		return;
	_ssgTl = newTl;
	writeReg(0, 8 + _regOffset, _ssgTl);
}

bool TownsPC98_MusicChannelSSG::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (*_controlEvents[cmd & 0x0f])(para);
}

bool TownsPC98_MusicChannelSSG::control_f0_setPatch(uint8 para) {
	_instr = para << 4;
	para = (para >> 3) & 0x1e;
	if (para)
		return control_f4_setAlgorithm(para | 0x40);
	return true;
}

bool TownsPC98_MusicChannelSSG::control_f1_setTotalLevel(uint8 para) {
	if (!_fading)
		_totalLevel = para;
	return true;
}

bool TownsPC98_MusicChannelSSG::control_f4_setAlgorithm(uint8 para) {
	_algorithm = para;
	return true;
}

bool TownsPC98_MusicChannelSSG::control_f9_loadCustomPatch(uint8 para) {
	_instr = (_driver->_sfxOffs + 10 + _regOffset) << 4;
	_envPatchData[_instr] = *_dataPtr++;
	_envPatchData[_instr + 3] = para;
	_envPatchData[_instr + 4] = *_dataPtr++;
	_envPatchData[_instr + 6] = *_dataPtr++;
	_envPatchData[_instr + 8] = *_dataPtr++;
	_envPatchData[_instr + 12] = *_dataPtr++;
	return true;
}

bool TownsPC98_MusicChannelSSG::control_fb_incOutLevel(uint8 para) {
	_dataPtr--;
	if (_fading)
		return true;

	_totalLevel--;
	if ((int8)_totalLevel < 0)
		_totalLevel = 0;

	return true;
}

bool TownsPC98_MusicChannelSSG::control_fc_decOutLevel(uint8 para) {
	_dataPtr--;
	if (_fading)
		return true;

	if (_totalLevel + 1 < 0x10)
		_totalLevel++;

	return true;
}

bool TownsPC98_MusicChannelSSG::control_ff_endOfTrack(uint8 para) {
	if (!_driver->_sfxOffs) {
		uint16 val = READ_LE_UINT16(--_dataPtr);
		if (val) {
			// loop
			_dataPtr = _driver->_trackPtr + val;
			return true;
		} else {
			// stop parsing
			if (!_driver->_fading)
				setOutputLevel(0);
			--_dataPtr;
			_flags |= CHS_EOT;
			_driver->_finishedSSGFlag |= _idFlag;
		}
	} else {
		// end of sfx track - restore ssg music channel
		_flags |= CHS_EOT;
		_driver->_finishedSfxFlag |= _idFlag;
		_driver->_ssgChannels[_chanNum]->restore();
	}

	return false;
}

uint8 *TownsPC98_MusicChannelSSG::_envPatchData = 0;

const uint8 TownsPC98_MusicChannelSSG::_envData[256] = {
	0x00, 0x00, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x37, 0x81, 0xC8, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x37, 0x81, 0xC8, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x04, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xBE, 0x00,
	0x0A, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0x01, 0x00,
	0xFF, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0xFF, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xFF, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x64, 0x01, 0xFF, 0x64, 0xFF, 0x81, 0xFF, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,

	0x02, 0x01, 0xFF, 0x28, 0xFF, 0x81, 0xF0, 0x00,
	0x00, 0x81, 0x00, 0x00, 0x0A, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xC8, 0x00,
	0x01, 0x81, 0x00, 0x00, 0x28, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0x78, 0x5F, 0x81, 0xA0, 0x00,
	0x05, 0x81, 0x00, 0x00, 0x28, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00,
	0x00, 0x01, 0xFF, 0xFF, 0x00, 0x81, 0x00, 0x00,
	0x00, 0x81, 0x00, 0x00, 0xFF, 0x81, 0x00, 0x00
};

void TownsPC98_SfxChannel::loadData(uint8 *data) {
	_flags = CHS_ALLOFF;
	_ticksLeft = 1;
	_dataPtr = data;
	_ssgTl = 0xff;
	_algorithm = 0x80;

	uint8 *tmp = _dataPtr;
	for (bool loop = true; loop;) {
		uint8 cmd = *tmp++;
		if (cmd < 0xf0) {
			tmp++;
		} else if (cmd == 0xff) {
			loop = false;
		} else if (cmd == 0xf6) {
			// reset repeat section countdown
			tmp[0] = tmp[1];
			tmp += 4;
		} else {
			tmp += _controlEventSize[cmd - 240];
		}
	}
}

void TownsPC98_SfxChannel::reset() {
	TownsPC98_MusicChannel::reset();
	_ssgStartLvl = _ssgTl = _ssgStep = _ssgTicksLeft = _ssgTargetLvl = 0;

	// Unlike the original we restore the default patch data. This fixes a bug
	// where certain sound effects would bring each other out of tune (e.g. the
	// dragon's fire in Darm's house in Kyra 1 would sound different each time
	// you triggered another sfx by dropping an item etc.)
	uint8 i = (13 + _regOffset) << 4;
	const uint8 *src = _envData;
	_envPatchData[i] = src[i];
	_envPatchData[i + 3] = src[i + 3];
	_envPatchData[i + 4] = src[i + 4];
	_envPatchData[i + 6] = src[i + 6];
	_envPatchData[i + 8] = src[i + 8];
	_envPatchData[i + 12] = src[i + 12];
}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
#define CONTROL(x) _controlEvents.push_back(new ControlEvent(this, &TownsPC98_MusicChannelPCM::control_##x))
TownsPC98_MusicChannelPCM::TownsPC98_MusicChannelPCM(TownsPC98_AudioDriver *driver, uint8 regOffs, uint8 flgs, uint8 num, uint8 key, uint8 prt, uint8 id) :
TownsPC98_MusicChannel(driver, regOffs, flgs, num, key, prt, id) {
	CONTROL(dummy);
	CONTROL(f1_prcStart);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(f6_repeatSection);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(fa_writeReg);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(dummy);
	CONTROL(ff_endOfTrack);
}
#undef CONTROL

TownsPC98_MusicChannelPCM::~TownsPC98_MusicChannelPCM() {
	for (Common::Array<const ControlEvent*>::iterator i = _controlEvents.begin(); i != _controlEvents.end(); ++i)
		delete *i;
}

void TownsPC98_MusicChannelPCM::loadData(uint8 *data) {
	_flags = (_flags & ~CHS_EOT) | CHS_ALLOFF;
	_ticksLeft = 1;
	_dataPtr = data;
	_totalLevel = 0x7F;
}

void TownsPC98_MusicChannelPCM::processEvents()  {
	if (_flags & CHS_EOT)
		return;

	if (--_ticksLeft)
		return;

	uint8 cmd = 0;
	bool loop = true;

	while (loop) {
		cmd = *_dataPtr++;
		if (cmd == 0x80) {
			loop = false;
		} else if (cmd < 0xf0) {
			writeReg(0, 0x10, cmd);
		} else if (!processControlEvent(cmd)) {
			return;
		}
	}

	_ticksLeft = *_dataPtr++;
}

bool TownsPC98_MusicChannelPCM::processControlEvent(uint8 cmd) {
	uint8 para = *_dataPtr++;
	return (*_controlEvents[cmd & 0x0f])(para);
}

bool TownsPC98_MusicChannelPCM::control_f1_prcStart(uint8 para) {
	_totalLevel = para;
	writeReg(0, 0x11, para);
	return true;
}

bool TownsPC98_MusicChannelPCM::control_ff_endOfTrack(uint8 para) {
	uint16 val = READ_LE_UINT16(--_dataPtr);
	if (val) {
		// loop
		_dataPtr = _driver->_trackPtr + val;
		return true;
	} else {
		// quit parsing for active channel
		--_dataPtr;
		_flags |= CHS_EOT;
		_driver->_finishedRhythmFlag |= _idFlag;
		return false;
	}
}
#endif // DISABLE_PC98_RHYTHM_CHANNEL

TownsPC98_AudioDriver::TownsPC98_AudioDriver(Audio::Mixer *mixer, EmuType type) :
	_channels(0), _ssgChannels(0), _sfxChannels(0),
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	_rhythmChannel(0),
#endif
	_sfxData(0), _sfxOffs(0), _patchData(0), _sfxBuffer(0), _musicBuffer(0), _trackPtr(0),
	_levelPresets(type == kTypeTowns ? _levelPresetFMTOWNS : _levelPresetPC98),
	_updateChannelsFlag(type == kType26 ? 0x07 : 0x3F), _finishedChannelsFlag(0),
	_updateSSGFlag(type == kTypeTowns ? 0x00 : 0x07), _finishedSSGFlag(0),
	_updateRhythmFlag(type == kType86 ?
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	0x01
#else
	0x00
#endif
	: 0x00),
	_numChanFM(type == kType26 ? 3 : 6), _numChanSSG(type == kTypeTowns ? 0 : 3), _numChanRHY(type == kType86 ? 1 : 0),
	_finishedRhythmFlag(0), _updateSfxFlag(0), _finishedSfxFlag(0),
	_musicTickCounter(0), _regWriteProtect(false),
	_musicPlaying(false), _sfxPlaying(false), _fading(false), _looping(0), _ready(false) {
	_sfxOffsets[0] = _sfxOffsets[1] = 0;
	_pc98a = new PC98AudioCore(mixer, this, type);
}

TownsPC98_AudioDriver::~TownsPC98_AudioDriver() {
	_ready = false;
	delete _pc98a;

	if (_channels) {
		for (int i = 0; i < _numChanFM; i++)
			delete _channels[i];
		delete[] _channels;
	}

	if (_ssgChannels) {
		for (int i = 0; i < _numChanSSG; i++)
			delete _ssgChannels[i];
		delete[] _ssgChannels;
	}

	if (_sfxChannels) {
		for (int i = 0; i < 2; i++)
			delete _sfxChannels[i];
		delete[] _sfxChannels;
	}
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	delete _rhythmChannel;
#endif
}

bool TownsPC98_AudioDriver::init() {
	if (_ready) {
		reset();
		return true;
	}

	if (!_pc98a->init())
		return false;

	_pc98a->setSoundEffectChanMask(0);

	_channels = new TownsPC98_MusicChannel *[_numChanFM];
	for (int i = 0; i < _numChanFM; i++) {
		int ii = i * 6;
		_channels[i] = new TownsPC98_MusicChannel(this, _channelPreset[ii], _channelPreset[ii + 1],
		        _channelPreset[ii + 2], _channelPreset[ii + 3], _channelPreset[ii + 4], _channelPreset[ii + 5]);
	}

	if (_numChanSSG) {
		_ssgChannels = new TownsPC98_MusicChannelSSG *[_numChanSSG];
		for (int i = 0; i < _numChanSSG; i++) {
			int ii = i * 6;
			_ssgChannels[i] = new TownsPC98_MusicChannelSSG(this, _channelPreset[ii], _channelPreset[ii + 1],
			        _channelPreset[ii + 2], _channelPreset[ii + 3], _channelPreset[ii + 4], _channelPreset[ii + 5]);
		}

		_sfxChannels = new TownsPC98_SfxChannel *[2];
		for (int i = 0; i < 2; i++) {
			int ii = (i + 1) * 6;
			_sfxChannels[i] = new TownsPC98_SfxChannel(this, _channelPreset[ii], _channelPreset[ii + 1],
			        _channelPreset[ii + 2], _channelPreset[ii + 3], _channelPreset[ii + 4], _channelPreset[ii + 5]);
		}
	}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_numChanRHY)
		_rhythmChannel = new TownsPC98_MusicChannelPCM(this, 0, 0, 0, 0, 0, 1);
#endif

	setMusicTempo(84);
	setSfxTempo(654);

	_ready = true;

	return true;
}

void TownsPC98_AudioDriver::loadMusicData(uint8 *data, bool loadPaused) {
	if (!_ready) {
		warning("TownsPC98_AudioDriver: Driver must be initialized before loading data");
		return;
	}

	if (!data) {
		warning("TownsPC98_AudioDriver: Invalid music file data");
		return;
	}

	reset();

	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	const uint8 *src_a = _trackPtr = _musicBuffer = data;

	for (uint8 i = 0; i < 3; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (int i = 0; i < _numChanSSG; i++) {
		_ssgChannels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	for (uint8 i = 3; i < _numChanFM; i++) {
		_channels[i]->loadData(data + READ_LE_UINT16(src_a));
		src_a += 2;
	}

	if (_numChanRHY) {
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		_rhythmChannel->loadData(data + READ_LE_UINT16(src_a));
#endif
		src_a += 2;
	}

	preventRegisterWrite(false);

	_patchData = src_a + 4;
	_finishedChannelsFlag = _finishedSSGFlag = _finishedRhythmFlag = 0;

	_musicPlaying = !loadPaused;
}

void TownsPC98_AudioDriver::loadSoundEffectData(uint8 *data, uint8 trackNum) {
	if (!_ready) {
		warning("TownsPC98_AudioDriver: Driver must be initialized before loading data");
		return;
	}

	if (!_sfxChannels) {
		warning("TownsPC98_AudioDriver: Sound effects not supported by this configuration");
		return;
	}

	if (!data) {
		warning("TownsPC98_AudioDriver: Invalid sound effects file data");
		return;
	}

	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	_sfxData = _sfxBuffer = data;
	_sfxOffsets[0] = READ_LE_UINT16(&_sfxData[(trackNum << 2)]);
	_sfxOffsets[1] = READ_LE_UINT16(&_sfxData[(trackNum << 2) + 2]);
	_sfxPlaying = true;
	_finishedSfxFlag = 0;
}

void TownsPC98_AudioDriver::reset() {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();

	_musicPlaying = false;
	_sfxPlaying = false;
	_fading = 0;
	_looping = 0;
	_musicTickCounter = 0;
	_sfxData = 0;

	_pc98a->reset();
	setMusicTempo(84);
	setSfxTempo(654);

	for (int i = 0; i < _numChanFM; i++)
		_channels[i]->reset();
	for (int i = 0; i < _numChanSSG; i++)
		_ssgChannels[i]->reset();

	if (_numChanSSG) {
		for (int i = 0; i < 2; i++)
			_sfxChannels[i]->reset();
	}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
	if (_rhythmChannel)
		_rhythmChannel->reset();
#endif
}

void TownsPC98_AudioDriver::fadeStep() {
	if (!_musicPlaying)
		return;

	for (int j = 0; j < _numChanFM; j++) {
		if (_updateChannelsFlag & _channels[j]->_idFlag)
			_channels[j]->fadeStep();
	}

	for (int j = 0; j < _numChanSSG; j++) {
		if (_updateSSGFlag & _ssgChannels[j]->_idFlag)
			_ssgChannels[j]->fadeStep();
	}

	if (!_fading) {
		_fading = 19;
#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		if (_numChanRHY) {
			if (_updateRhythmFlag & _rhythmChannel->_idFlag)
				_rhythmChannel->reset();
		}
#endif
	} else {
		if (!--_fading)
			reset();
	}
}

void TownsPC98_AudioDriver::pause() {
	_musicPlaying = false;
}

void TownsPC98_AudioDriver::cont() {
	_musicPlaying = true;
}

bool TownsPC98_AudioDriver::looping() const {
	return _looping == _updateChannelsFlag ? true : false;
}

bool TownsPC98_AudioDriver::musicPlaying() const {
	return _musicPlaying;
}

void TownsPC98_AudioDriver::setMusicVolume(int volume) {
	_pc98a->setMusicVolume(volume);
}

void TownsPC98_AudioDriver::setSoundEffectVolume(int volume) {
	_pc98a->setSoundEffectVolume(volume);
}

uint8 TownsPC98_AudioDriver::readReg(uint8 part, uint8 reg) {
	return _pc98a->readReg(part, reg);
}

void TownsPC98_AudioDriver::writeReg(uint8 part, uint8 reg, uint8 val) {
	if (!_regWriteProtect)
		_pc98a->writeReg(part, reg, val);
}

void TownsPC98_AudioDriver::preventRegisterWrite(bool prevent) {
	_regWriteProtect = prevent;
}

void TownsPC98_AudioDriver::timerCallbackA() {
	if (_sfxChannels && _sfxPlaying) {
		if (_sfxData)
			startSoundEffect();

		_sfxOffs = 3;
		_trackPtr = _sfxBuffer;

		for (int i = 0; i < 2; i++) {
			if (_updateSfxFlag & _sfxChannels[i]->_idFlag) {
				_sfxChannels[i]->processEvents();
				_sfxChannels[i]->processFrequency();
			}
		}

		_trackPtr = _musicBuffer;
	}

	if (_updateSfxFlag && _finishedSfxFlag == _updateSfxFlag) {
		_sfxPlaying = false;
		_updateSfxFlag = 0;
		_pc98a->setSoundEffectChanMask(0);
	}
}

void TownsPC98_AudioDriver::timerCallbackB() {
	_sfxOffs = 0;

	if (_musicPlaying) {
		_musicTickCounter++;

		for (int i = 0; i < _numChanFM; i++) {
			if (_updateChannelsFlag & _channels[i]->_idFlag) {
				_channels[i]->processEvents();
				_channels[i]->processFrequency();
			}
		}

		for (int i = 0; i < _numChanSSG; i++) {
			if (_updateSSGFlag & _ssgChannels[i]->_idFlag) {
				_ssgChannels[i]->processEvents();
				_ssgChannels[i]->processFrequency();
			}
		}

#ifndef DISABLE_PC98_RHYTHM_CHANNEL
		if (_numChanRHY)
			if (_updateRhythmFlag & _rhythmChannel->_idFlag)
				_rhythmChannel->processEvents();
#endif
	}

	preventRegisterWrite(false);

	if (_finishedChannelsFlag == _updateChannelsFlag && _finishedSSGFlag == _updateSSGFlag && _finishedRhythmFlag == _updateRhythmFlag)
		_musicPlaying = false;
}

void TownsPC98_AudioDriver::startSoundEffect() {
	int volFlags = 0;

	for (int i = 0; i < 2; i++) {
		if (_sfxOffsets[i]) {
			_ssgChannels[i + 1]->protect();
			_sfxChannels[i]->reset();
			_sfxChannels[i]->loadData(_sfxData + _sfxOffsets[i]);
			_updateSfxFlag |= _sfxChannels[i]->_idFlag;
			volFlags |= (_sfxChannels[i]->_idFlag << _numChanFM);
		} else {
			_ssgChannels[i + 1]->restore();
			_updateSfxFlag &= ~_sfxChannels[i]->_idFlag;
		}
	}

	_pc98a->setSoundEffectChanMask(volFlags);
	_sfxData = 0;
}

void TownsPC98_AudioDriver::setMusicTempo(uint8 tempo) {
	writeReg(0, 0x26, tempo);
	writeReg(0, 0x27, 0x3D);
	writeReg(0, 0x27, 0x3F);
}

void TownsPC98_AudioDriver::setSfxTempo(uint16 tempo) {
	writeReg(0, 0x24, tempo & 0xff);
	writeReg(0, 0x25, tempo >> 8);
	writeReg(0, 0x27, 0x3E);
	writeReg(0, 0x27, 0x3F);
}

const uint8 TownsPC98_AudioDriver::_channelPreset[36] = {
	0x00, 0x80, 0x00, 0x00, 0x00, 0x01,
	0x01, 0x80, 0x01, 0x01, 0x00, 0x02,
	0x02, 0x80, 0x02, 0x02, 0x00, 0x04,
	0x00, 0x80, 0x03, 0x04, 0x01, 0x08,
	0x01, 0x80, 0x04, 0x05, 0x01, 0x10,
	0x02, 0x80, 0x05, 0x06, 0x01, 0x20
};

const uint8 TownsPC98_AudioDriver::_levelPresetFMTOWNS[24] = {
	0x54, 0x50, 0x4C, 0x48, 0x44, 0x40, 0x3C, 0x38,
	0x34, 0x30, 0x2C, 0x28, 0x24, 0x20, 0x1C, 0x18,
	0x14, 0x10, 0x0C, 0x08, 0x04, 0x90, 0x90, 0x90
};

const uint8 TownsPC98_AudioDriver::_levelPresetPC98[24] = {
	0x40, 0x3B, 0x38, 0x34, 0x30, 0x2A, 0x28, 0x25,
	0x22, 0x20, 0x1D, 0x1A, 0x18, 0x15, 0x12, 0x10,
	0x0D, 0x0A, 0x08, 0x05, 0x02, 0x90, 0x90, 0x90
};
