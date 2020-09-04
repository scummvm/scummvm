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

#ifdef ENABLE_EOB

#include "kyra/kyra_v1.h"
#include "kyra/sound/drivers/segacd.h"

#include "audio/softsynth/fmtowns_pc98/sega_audio.h"

#include "common/endian.h"
#include "common/ptr.h"

#define SEGA_SND_DEBUG_EXT	0

namespace Kyra {

class SegaAudioDriverInternal;
class SegaAudioChannel {
public:
	SegaAudioChannel(uint8 id, SegaAudioInterface *sai);
	virtual ~SegaAudioChannel() {}

	void initTrack();
	bool update();

	void lock() { _lock = true;  }
	void unlock() { _lock = false; }

protected:
	uint8 setCountDown();
	void startVbr();
	uint16 getFrequency(uint8 note);

	void cmd_setTempo();
	void cmd_programChange();
	void cmd_setVolume();
	void cmd_incVolume();
	void cmd_decVolume();
	void cmd_modVolume();
	void cmd_setOctave();
	void cmd_incOctave();
	void cmd_decOctave();
	void cmd_modOctave();
	void cmd_setNoteLen();
	void cmd_setReleaseTimer();
	void cmd_writeReg();
	void cmd_setRepeatMarker();
	void cmd_repeatFromMarker();
	void cmd_removeRepeatMarker();
	void cmd_beginRepeatSection();
	void cmd_jump();
	void cmd_jumpToSubroutine();
	void cmd_returnFromSubroutine();
	void cmd_initVbr();
	void cmd_pitchBend();
	void cmd_initCstVbr();
	void cmd_enableTwoChanMode();
	void cmd_disableTwoChanMode();
	void cmd_panCenter();
	void cmd_panLeft();
	void cmd_panRight();
	void cmd_UNK29();
	void cmd_void() {}
	void cmd_transpose();

#if SEGA_SND_DEBUG_EXT
	typedef Common::Functor0Mem<void, SegaAudioChannel> SegaSndFunc;
	struct SegaSndOpcode {
		SegaSndOpcode(SegaSndFunc *func, const char *desc, int dataLen) : _func(func), _desc(desc), _dataLen(dataLen) {}
		~SegaSndOpcode() { delete _func; }
		bool isValid() { return _func->isValid(); }
		void operator()(int dbgInfoChan, const uint8 *dbgInfoData) const {
			Common::String dstr = "";
			for (int i = 0; i < _dataLen; ++i)
				dstr += Common::String::format("0x%02x ", dbgInfoData[i]);
			debugC(3, kDebugLevelSound, "Channel %d: %s() [ %s]", dbgInfoChan, _desc, dstr.c_str());
			(*_func)();
		}
		SegaSndFunc *_func;
		const char *_desc;
		const int _dataLen;
	};
#else
	typedef Common::Functor0Mem<void, SegaAudioChannel> SegaSndOpcode;
#endif
	typedef Common::SharedPtr<SegaSndOpcode> PSegaSndOpcode;
	Common::Array<PSegaSndOpcode> _opcodes;

	uint8 _id;
	uint8 _para1;
	uint8 _countDown;
	uint8 _releaseTimer;
	uint8 _noteLen;
	uint8 _volume;
	uint8 _octave;

	uint16 _frequency;
	int16 _pitchBend;
	int8 _transpose;

	uint8 _vbrTempo;
	int16 _vbrState;
	int16 _vbrIncStart;
	uint8 _vbrSteps;
	uint8 _vbrDelay;
	uint8 _vbrTempoCurState;
	int16 _vbrInc;
	uint8 _vbrStepsCounter;
	uint8 _vbrDelayCountDown;

	uint8 _cstVbrEnable;
	uint8 _cstVbrDelay;
	uint8 _cstVbrDelayCountDown;
	const uint8 *_cstVbrData;
	const uint8 *_cstVbrDataCur;
	const uint8 *_cstVbrDataTmp;

	bool _lock;

	const uint8 *_dataPtr;

	SegaAudioInterface *_sga;

private:
	virtual void keyOff() = 0;
	virtual void restoreTone() {}
	virtual void fadeUpdate() {}
	virtual bool setupTone() = 0;
	virtual void setVolume(uint8 vol) = 0;
	virtual void sendVolume(uint8 vol) = 0;
	virtual void programChange() = 0;
	virtual void setPanPos(uint8 pan) {}
	virtual void sendFrequency(uint16 freq) = 0;
	virtual void toggleSpecialMode(bool enable) {}
	virtual void updateEnvelope() {}
	virtual const uint16 *freqTable() = 0;

	struct Marker {
		Marker() : counter(0), pos(0) {}
		Marker(uint8 c, const uint8 *p) : counter(c), pos(p) {}
		bool operator==(const uint8 *p) const { return (pos == p); }
		uint8 counter;
		const uint8 *pos;
	};

	Common::Array<Marker> _repeatMarkers;
	Common::Array<Marker> _sectionMarkers;
	Common::Array<const uint8*> _returnMarkers;
};

class SegaAudioChannel_FM : public SegaAudioChannel {
public:
	SegaAudioChannel_FM(uint8 id, SegaAudioInterface *sai, uint8 part, uint8 regOffs);
	~SegaAudioChannel_FM() override {}

private:
	void keyOff() override;
	void fadeUpdate() override;
	bool setupTone() override;
	void setVolume(uint8 vol) override;
	void sendVolume(uint8 vol) override;
	void programChange() override;
	void setPanPos(uint8 pan) override;
	void sendFrequency(uint16 freq) override;
	void toggleSpecialMode(bool enable) override;

	const uint16 *freqTable() override { return _freqTable; }

	uint8 _algorithm;
	uint8 _program;
	uint8 _cfreqReg;

	const uint8 _part;
	const uint8 _regOffs;
	const uint8 _regKeyOn;
	const uint8 _regKeyOff;

	static const uint16 _freqTable[97];
};

class SegaAudioChannel_SG : public SegaAudioChannel {
public:
	SegaAudioChannel_SG(uint8 id, SegaAudioInterface *sai, uint8 regOffs);
	~SegaAudioChannel_SG() override {}

private:
	void keyOff() override;
	void restoreTone() override;
	bool setupTone() override;
	void setVolume(uint8 vol) override {}
	void sendVolume(uint8 vol) override;
	void programChange() override;
	void sendFrequency(uint16 freq) override;
	void updateEnvelope() override;

	const uint16 *freqTable() override { return _freqTable; }

	uint8 _envDelay;
	uint8 _volume2;
	const uint8 *_envDataAtt;
	const uint8 *_envDataRel;
	const uint8 *_envDataCur;
	const uint8 _regOffs;

	enum EnvState {
		kDisabled = 0,
		kAttack,
		kSustain,
		kRelease,
		kReady
	};

	int _envState;

	static const uint16 _freqTable[97];
};

class SegaAudioChannel_NG : public SegaAudioChannel_FM {
public:
	SegaAudioChannel_NG(uint8 id, SegaAudioInterface *sai);
	~SegaAudioChannel_NG() override {}

private:
};

class SegaAudioDriverInternal : public SegaAudioPluginDriver {
public:
	SegaAudioDriverInternal(Audio::Mixer *mixer);
	~SegaAudioDriverInternal();

	static SegaAudioDriverInternal *open(Audio::Mixer *mixer);
	static void close();

	void startFMSound(const uint8 *trackData, uint8 volume, uint8 prioFlags);

	void loadPCMData(uint16 address, const uint8 *data, uint16 dataSize);
	void startPCMSound(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 step, uint8 pan, uint8 vol);

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	void timerCallbackA() override;
	void timerCallbackB() override;

	static uint8 calcVolume(int vol);
	static int getFadeState();
	static const uint8 *getTrack();
	static const uint8 *getTrack(int channel);
	static const uint8 *getProgram(int instrument);

private:
	void start();
	void stop();
	void fade();

	void update();

	SegaAudioChannel **_channels;

	bool _priority;
	uint8 _isPlaying;
	uint8 _sfxInternal_a;

	static const uint8 *_trackData;
	static uint8 _fadeTicker;
	static uint8 _attenuation;
	static uint8 _fadeAttenuation;

	SegaAudioInterface *_sga;

	static SegaAudioDriverInternal *_refInstance;
	static int _refCount;

	bool _ready;
};

SegaAudioChannel::SegaAudioChannel(uint8 id, SegaAudioInterface *sai) : _id(id), _sga(sai), _para1(0), _countDown(0), _dataPtr(0),
	_volume(0), _octave(0), _releaseTimer(0), _noteLen(0), _vbrTempo(0), _vbrState(0), _vbrIncStart(0), _vbrSteps(0),
	_vbrDelay(0), _vbrTempoCurState(0), _vbrInc(0), _vbrStepsCounter(0), _vbrDelayCountDown(0), _frequency(0), _pitchBend(0),
	_transpose(0), _cstVbrEnable(0), _cstVbrDelay(0), _cstVbrDelayCountDown(0), _cstVbrData(0), _cstVbrDataCur(0), _lock(false),
	_cstVbrDataTmp(0) {
#if SEGA_SND_DEBUG_EXT
#define SGC(x, y)	_opcodes.push_back(PSegaSndOpcode(new SegaSndOpcode(new SegaSndFunc(this, &SegaAudioChannel::x), #x, y)))
#else
#define SGC(x, y)	_opcodes.push_back(PSegaSndOpcode(new SegaSndOpcode(this, &SegaAudioChannel::x)))
#endif
#define SGCR(x)	_opcodes.push_back(_opcodes[x])
	SGC(cmd_setTempo, 1);
	SGC(cmd_programChange, 1);
	SGCR(1);
	SGC(cmd_setVolume, 1);
	SGC(cmd_incVolume, 0);
	SGC(cmd_decVolume, 0);
	SGC(cmd_modVolume, 1);
	SGC(cmd_setOctave, 1);
	SGC(cmd_incOctave, 0);
	SGC(cmd_decOctave, 0);
	SGC(cmd_modOctave, 1);
	SGC(cmd_setNoteLen, 1);
	SGC(cmd_setReleaseTimer, 1);
	SGC(cmd_writeReg, 2);
	SGC(cmd_setRepeatMarker, 1);
	SGC(cmd_repeatFromMarker, 0);
	SGC(cmd_removeRepeatMarker, 0);
	SGC(cmd_beginRepeatSection, 2);
	SGC(cmd_jump, 2);
	SGC(cmd_jumpToSubroutine, 2);
	SGC(cmd_returnFromSubroutine, 0);
	SGC(cmd_initVbr, 5);
	SGC(cmd_pitchBend, 1);
	SGC(cmd_initCstVbr, 2);
	SGC(cmd_enableTwoChanMode, 0);
	SGC(cmd_disableTwoChanMode, 0);
	SGC(cmd_panCenter, 0);
	SGC(cmd_panLeft, 0);
	SGC(cmd_panRight, 0);
	SGC(cmd_UNK29, 1);
	SGC(cmd_void, 0);
	SGC(cmd_transpose, 1);
#undef SGC
#undef SGCR
}

void SegaAudioChannel::initTrack() {
	_dataPtr = SegaAudioDriverInternal::getTrack(_id);
	_para1 = 0;
	_countDown = 1;
	_repeatMarkers.clear();
	_returnMarkers.clear();
	_sectionMarkers.clear();
	_cstVbrData = _cstVbrDataCur = _cstVbrDataTmp = 0;
	_cstVbrEnable = _vbrTempo = 0;
	_pitchBend = _vbrState = 0;
	_transpose = 0;
	_volume = 0;
	_lock = false;
}

bool SegaAudioChannel::update() {
	/*
	not implemented/required: sfx channel lock handling
	restoreTone();
	*/

	fadeUpdate();

	if (!_dataPtr)
		return true;

	if (!--_countDown) {
		do {
			uint8 cmd = *_dataPtr;
			while (!(cmd & 0x80)) {
				++_dataPtr;
				if (_opcodes[cmd]->isValid())
#if SEGA_SND_DEBUG_EXT
					(*_opcodes[cmd])(_id, _dataPtr);
#else
					(*_opcodes[cmd])();
#endif
				cmd = *_dataPtr;
			}

			if (cmd == 0xFF)
				return false;

		} while (setupTone());
	}

	//if (_type != 2) {
	if (!(_para1 & 0x10) && _countDown == _releaseTimer) {
		keyOff();
	}
	//}

	uint8 vbrSet = 0;
	if (_vbrTempo) {
		if (_vbrDelayCountDown) {
			--_vbrDelayCountDown;
		} else if (!--_vbrTempoCurState) {
			vbrSet = _vbrTempoCurState = _vbrTempo;
			_vbrState += _vbrInc;
			if (!--_vbrStepsCounter) {
				_vbrStepsCounter += (_vbrSteps + _vbrSteps);
				_vbrInc = -_vbrInc;
			}
		}
	}

	if (_cstVbrEnable & 1) {
		if (_cstVbrDelayCountDown) {
			--_cstVbrDelayCountDown;
		} else {
			assert(_cstVbrDataCur);
			const uint8 *in = _cstVbrDataCur;
			for (bool cont = true; cont; ) {
				uint8 lo = *in++;
				uint8 hi = *in++;

				if ((hi & 0xF8) != 0x80) {
					_vbrState += (int16)((hi << 8) | lo);
					cont = false;
					vbrSet = 1;
				} else {
					switch (hi & 0x0F) {
					case 0:
						_cstVbrDataTmp = in;
						break;
					case 1:
						assert(_cstVbrDataTmp);
						in = _cstVbrDataTmp;
						break;
					case 2:
						_cstVbrDelayCountDown = lo;
						cont = false;
						break;
					case 3:
						_cstVbrEnable += _para1;
						cont = false;
						break;
					default:
						error("SegaAudioChannel::update(): Unknown error");
						break;
					}
				}
			}
			_cstVbrDataCur = in;
		}
	}

	if (vbrSet)
		sendFrequency(_frequency + _vbrState);

	updateEnvelope();

	_lock = false;

	return true;
}

uint8 SegaAudioChannel::setCountDown() {
	_para1 = *_dataPtr++;
	_countDown = (_para1 & 0x20) ? *_dataPtr++ : _noteLen;
	debugC(3, kDebugLevelSound, "Channel %d: Note %d, Duration %d", _id, _octave * 12 + (_para1 & 0x0F), _countDown);
	return _para1;
}

void SegaAudioChannel::startVbr() {
	if (_cstVbrEnable) {
		_cstVbrEnable = 1;
		_cstVbrDataCur = _cstVbrData;
		_cstVbrDelayCountDown = _cstVbrDelay;
	}

	if (_vbrTempo) {
		_vbrTempoCurState = _vbrTempo;
		_vbrInc = _vbrIncStart;
		_vbrStepsCounter = _vbrSteps;
		_vbrDelayCountDown = _vbrDelay;
	}
	_vbrState = 0;
}

uint16 SegaAudioChannel::getFrequency(uint8 note) {
	_frequency = freqTable()[_octave * 12 + note + _transpose] + _pitchBend;
	return _frequency;
}

void SegaAudioChannel::cmd_setTempo() {
	_sga->writeReg(0, 0x26, *_dataPtr++);
}

void SegaAudioChannel::cmd_programChange() {
	programChange();
}

void SegaAudioChannel::cmd_setVolume() {
	_volume = *_dataPtr++;
	setVolume(_volume);
}

void SegaAudioChannel::cmd_incVolume() {
	setVolume(++_volume);
}

void SegaAudioChannel::cmd_decVolume() {
	setVolume(--_volume);
}

void SegaAudioChannel::cmd_modVolume() {
	_volume += *_dataPtr++;
	setVolume(_volume);
}

void SegaAudioChannel::cmd_setOctave() {
	_octave = *_dataPtr++ - 1;
	_transpose = 0;
}

void SegaAudioChannel::cmd_incOctave() {
	_octave++;
}

void SegaAudioChannel::cmd_decOctave() {
	_octave--;
}

void SegaAudioChannel::cmd_modOctave() {
	_octave += *_dataPtr++;
}

void SegaAudioChannel::cmd_setNoteLen() {
	_noteLen = *_dataPtr++;
}

void SegaAudioChannel::cmd_setReleaseTimer() {
	_releaseTimer = *_dataPtr++;
}

void SegaAudioChannel::cmd_writeReg() {
	uint8 reg = *_dataPtr++;
	uint8 val = *_dataPtr++;
	if (!_lock)
		_sga->writeReg(_id >= 7 ? 1 : 0, reg, val);
}

void SegaAudioChannel::cmd_setRepeatMarker() {
	_repeatMarkers.push_back(Marker(*_dataPtr, _dataPtr + 1));
	_dataPtr++;
}

void SegaAudioChannel::cmd_repeatFromMarker() {
	assert(!_repeatMarkers.empty());
	if (--_repeatMarkers.back().counter)
		_dataPtr = _repeatMarkers.back().pos;
	else
		_repeatMarkers.pop_back();
}

void SegaAudioChannel::cmd_removeRepeatMarker() {
	assert(!_repeatMarkers.empty());
	_repeatMarkers.pop_back();
}

void SegaAudioChannel::cmd_beginRepeatSection() {
	uint8 totalReps = *_dataPtr++;

	Common::Array<Marker>::iterator i = Common::find(_sectionMarkers.begin(), _sectionMarkers.end(), _dataPtr);
	if (i == _sectionMarkers.end()) {
		_sectionMarkers.push_back(Marker(*_dataPtr, _dataPtr));
		i = _sectionMarkers.end() - 1;
	}
	_dataPtr++;

	if (++i->counter == totalReps) {
		_sectionMarkers.erase(i);
		cmd_jump();
	} else {
		_dataPtr += 2;
	}
}

void SegaAudioChannel::cmd_jump() {
	uint16 offset = READ_LE_UINT16(_dataPtr);
	_dataPtr = SegaAudioDriverInternal::getTrack() + offset;
}

void SegaAudioChannel::cmd_jumpToSubroutine() {
	_returnMarkers.push_back(_dataPtr + 2);
	cmd_jump();
}

void SegaAudioChannel::cmd_returnFromSubroutine() {
	_dataPtr = _returnMarkers.back();
	assert(_dataPtr);
	_returnMarkers.pop_back();
}

void SegaAudioChannel::cmd_initVbr() {
	_vbrTempo = *_dataPtr++;
	if (!_vbrTempo)
		return;
	_vbrIncStart = READ_LE_INT16(_dataPtr);
	_dataPtr += 2;
	_vbrInc = 0;
	_vbrSteps = *_dataPtr++;
	_vbrDelay = *_dataPtr++;
}

void SegaAudioChannel::cmd_pitchBend() {
	_pitchBend = (int8)*_dataPtr++;
	if (_id == 10)
		_pitchBend <<= 4;
}

void SegaAudioChannel::cmd_initCstVbr() {
	_cstVbrEnable = *_dataPtr++;
	if (!_cstVbrEnable)
		return;
	_cstVbrData = SegaAudioDriverInternal::getProgram(_cstVbrEnable);
	_cstVbrEnable = 2;
	_cstVbrDelay = *_dataPtr++;
}

void SegaAudioChannel::cmd_enableTwoChanMode() {
	toggleSpecialMode(true);
}

void SegaAudioChannel::cmd_disableTwoChanMode() {
	toggleSpecialMode(false);
}

void SegaAudioChannel::cmd_panCenter() {
	setPanPos(0xC0);
}

void SegaAudioChannel::cmd_panLeft() {
	setPanPos(0x80);
}

void SegaAudioChannel::cmd_panRight() {
	setPanPos(0x40);
}

void SegaAudioChannel::cmd_UNK29() {
	_dataPtr++;
}

void SegaAudioChannel::cmd_transpose() {
	_transpose += (int8)*_dataPtr++;
}

SegaAudioChannel_FM::SegaAudioChannel_FM(uint8 id, SegaAudioInterface *sai, uint8 part, uint8 regOffs) : SegaAudioChannel(id, sai), _part(part), _regOffs(regOffs),
	_regKeyOn(0xF0 | regOffs | (part ? 4 : 0)), _regKeyOff(regOffs | (part ? 4 : 0)), _algorithm(0), _program(0), _cfreqReg(0) {
}

void SegaAudioChannel_FM::keyOff() {
	debugC(5, kDebugLevelSound, "Channel %d: Key Off", _id);
	if (!_lock)
		_sga->writeReg(0, 0x28, _regKeyOff);
}

void SegaAudioChannel_FM::fadeUpdate() {
	if (SegaAudioDriverInternal::getFadeState() == _id)
		sendVolume(_volume);
}

bool SegaAudioChannel_FM::setupTone() {
	if (_cfreqReg > 0 && _cfreqReg < 4) {
		uint8 note = *_dataPtr++ & 0x0F;

		if (note) {
			static uint8 frqreg[] = { 0xAD, 0xA9, 0xAE, 0xAA, 0xAC, 0xA8 };
			uint16 f = _freqTable[_octave * 12 + note + _transpose];
			if (!_lock) {
				_sga->writeReg(_part, frqreg[(_cfreqReg - 1) << 1], f >> 8);
				_sga->writeReg(_part, frqreg[((_cfreqReg - 1) << 1) + 1], f & 0xFF);
			}
		}

		_cfreqReg++;
		return true;

	} else if (_cfreqReg == 4) {
		_cfreqReg = 1;
	}

	uint8 para = _para1;
	if (!(_para1 & 0x10))
		keyOff();

	uint8 note = setCountDown() & 0x0F;
	if (!note)
		return false;

	sendFrequency(getFrequency(note));

	if (!(para & 0x10)) {
		if (!_lock)
			_sga->writeReg(0, 0x28, _regKeyOn);
		startVbr();
	}

	return false;
}

void SegaAudioChannel_FM::setVolume(uint8 vol) {
	sendVolume(vol);
}

void SegaAudioChannel_FM::sendVolume(uint8 vol) {
	if (_lock)
		return;

	static const uint8 carrier[8] = { 1,  1,  1,  1,  2,  3,  3,  4 };
	vol = SegaAudioDriverInternal::calcVolume(vol);

	const int8 *in = (const int8*)SegaAudioDriverInternal::getProgram(_program) + 7;
	for (uint8 c = 0; c < carrier[_algorithm]; ++c)
		_sga->writeReg(_part, 0x4C + _regOffs - (c << 2), vol + *in--);
}

void SegaAudioChannel_FM::programChange() {
	_program = *_dataPtr++;
	const uint8 *in = SegaAudioDriverInternal::getProgram(_program);
	if (!_lock) {
		for (uint8 reg = 0x30 + _regOffs; reg < 0x8F; reg += 4)
			_sga->writeReg(_part, reg, *in++);
	} else {
		in += 24;
	}

	_algorithm = *in & 7;
	if (!_lock)
		_sga->writeReg(_part, 0xB0 + _regOffs, *in++);
	sendVolume(_volume);
}

void SegaAudioChannel_FM::setPanPos(uint8 pan) {
	if (!_lock)
		_sga->writeReg(_part, 0xB4 + _regOffs, pan);
}

void SegaAudioChannel_FM::sendFrequency(uint16 freq) {
	if (_lock)
		return;
	_sga->writeReg(_part, 0xA4 + _regOffs, freq >> 8);
	_sga->writeReg(_part, 0xA0 + _regOffs, freq & 0xFF);
}

void SegaAudioChannel_FM::toggleSpecialMode(bool enable) {
	_cfreqReg = enable ? 1 : 0;
	_sga->writeReg(0, 0x27, (_cfreqReg << 6) | 0x0F);
}

const uint16 SegaAudioChannel_FM::_freqTable[97] = {
	0x0000,
	0x0284, 0x02ab, 0x02d3, 0x02fe, 0x032d, 0x035c, 0x038f, 0x03c5,	0x03ff, 0x043c, 0x047c, 0x04c0,
	0x0a84, 0x0aab, 0x0ad3, 0x0afe,	0x0b2d, 0x0b5c, 0x0b8f, 0x0bc5, 0x0bff, 0x0c3c, 0x0c7c, 0x0cc0,
	0x1284, 0x12ab, 0x12d3, 0x12fe, 0x132d, 0x135c, 0x138f, 0x13c5,	0x13ff, 0x143c, 0x147c, 0x14c0,
	0x1a84, 0x1aab, 0x1ad3, 0x1afe, 0x1b2d, 0x1b5c, 0x1b8f, 0x1bc5, 0x1bff, 0x1c3c, 0x1c7c, 0x1cc0,
	0x2284, 0x22ab, 0x22d3, 0x22fe, 0x232d, 0x235c, 0x238f, 0x23c5, 0x23ff, 0x243c, 0x247c, 0x24c0,
	0x2a84, 0x2aab, 0x2ad3, 0x2afe, 0x2b2d, 0x2b5c, 0x2b8f, 0x2bc5, 0x2bff, 0x2c3c, 0x2c7c, 0x2cc0,
	0x3284, 0x32ab, 0x32d3, 0x32fe, 0x332d, 0x335c, 0x338f, 0x33c5, 0x33ff, 0x343c, 0x347c, 0x34c0,
	0x3a84, 0x3aab, 0x3ad3, 0x3afe, 0x3b2d, 0x3b5c, 0x3b8f, 0x3bc5, 0x3bff, 0x3c3c, 0x3c7c, 0x3cc0
};

SegaAudioChannel_SG::SegaAudioChannel_SG(uint8 id, SegaAudioInterface *sai, uint8 regOffs) : SegaAudioChannel(id, sai), _regOffs(regOffs), _envDataAtt(0), _envDataRel(0),
	_envDataCur(0), _envDelay(0), _envState(0), _volume2(0) {
}

void SegaAudioChannel_SG::keyOff() {
	if (_envState == kDisabled) {
		sendVolume(0);
	} else if (_envState < kRelease) {
		_envState = kRelease;
		_envDelay = 1;
		_envDataCur = _envDataRel;
	}
}

void SegaAudioChannel_SG::restoreTone() {
	_sga->psgWrite(0x80 | _regOffs | (_frequency & 0x0F));
	_sga->psgWrite((_frequency >> 4) & 0x3F);
	_sga->psgWrite((0x90 | _regOffs | (_volume2 & 0x0F)) ^ 0x0F);
}

bool SegaAudioChannel_SG::setupTone() {
	uint8 para = _para1;
	uint8 note = setCountDown() & 0x0F;
	if (!note) {
		keyOff();
		return false;
	}

	sendFrequency(getFrequency(note));

	if (!(para & 0x10)) {
		startVbr();
		if (_envState == kDisabled) {
			sendVolume(SegaAudioDriverInternal::calcVolume(_volume));
		} else {
			_envState = kAttack;
			_envDelay = 1;
			_envDataCur = _envDataAtt;
		}
	}

	return false;
}

void SegaAudioChannel_SG::sendVolume(uint8 vol) {
	_volume2 = vol;
	if (!_lock)
		_sga->psgWrite((0x90 | _regOffs | (vol & 0x0F)) ^ 0x0F);
}

void SegaAudioChannel_SG::programChange() {
	uint8 prg = *_dataPtr++;
	if (!prg) {
		_envState = kDisabled;
		return;
	}

	_envState = kReady;
	_envDataAtt = SegaAudioDriverInternal::getProgram(prg);
	_envDataRel = SegaAudioDriverInternal::getProgram(*_dataPtr++);
	sendVolume(0);
}

void SegaAudioChannel_SG::sendFrequency(uint16 freq) {
	if (_lock)
		return;
	_sga->psgWrite(0x80 | _regOffs | (freq & 0x0F));
	_sga->psgWrite((freq >> 4) & 0x3F);
}

void SegaAudioChannel_SG::updateEnvelope() {
	if (_envState != kAttack && _envState != kRelease)
		return;

	if (--_envDelay)
		return;

	if (*_envDataCur == 0xFF) {
		_envState++;
		return;
	}

	sendVolume(SegaAudioDriverInternal::calcVolume(_volume + (int8)*_envDataCur++));
	_envDelay = *_envDataCur++;
}

const uint16 SegaAudioChannel_SG::_freqTable[97] = {
	0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x03f7, 0x03be, 0x0388,
	0x0356, 0x0326, 0x02f9, 0x02ce, 0x02a5, 0x0280, 0x025c, 0x023a, 0x021a, 0x01fb, 0x01df, 0x01c4,
	0x01ab, 0x0193, 0x017d, 0x0167, 0x0153, 0x0140, 0x012e, 0x011d, 0x010d, 0x00fe, 0x00ef, 0x00e2,
	0x00d6, 0x00c9, 0x00be, 0x00b4, 0x00a9, 0x00a0, 0x0097, 0x008f, 0x0087, 0x007f, 0x0078, 0x0071,
	0x006b, 0x0065, 0x005f, 0x005a, 0x0055, 0x0050, 0x004b, 0x0047, 0x0043, 0x0040, 0x003c, 0x0039,
	0x0036, 0x0033, 0x0030, 0x002d, 0x002b, 0x0028, 0x0026, 0x0024, 0x0022, 0x0020, 0x001f, 0x001d,
	0x001b, 0x001a, 0x0018, 0x0017, 0x0016, 0x0015, 0x0013, 0x0012, 0x0011, 0x0010, 0x000f, 0x000e
};

SegaAudioChannel_NG::SegaAudioChannel_NG(uint8 id, SegaAudioInterface *sai) : SegaAudioChannel_FM(id, sai, 0, 0) {
}

SegaAudioDriverInternal *SegaAudioDriverInternal::_refInstance = 0;
int SegaAudioDriverInternal::_refCount = 0;
uint8 SegaAudioDriverInternal::_attenuation = 0;
uint8 SegaAudioDriverInternal::_fadeTicker = 0;
uint8 SegaAudioDriverInternal::_fadeAttenuation = 0;
const uint8 *SegaAudioDriverInternal::_trackData = 0;

SegaAudioDriverInternal::SegaAudioDriverInternal(Audio::Mixer *mixer) : SegaAudioPluginDriver(), _sga(0), _priority(false), _channels(0), _isPlaying(0), _sfxInternal_a(0), _ready(false) {
	_sga = new SegaAudioInterface(mixer, this);
	_sga->init();
	// Setup all channels as sound effect channels. Some FM tunes (like the gameover tune) are actual music pieces and not just sfx.
	// Unfortunately there isn't any good way to distinguish the tune types. Most tracks are classical sfx. So for the volume control
	// we treat the CD Audio as music and all FM and PCM output as sound effects.
	_sga->setSoundEffectChanMask(-1);

	_channels = new SegaAudioChannel*[10];
	for (int i = 0; i < 3; ++i)
		_channels[i] = new SegaAudioChannel_FM(i, _sga, 0, i);
	for (int i = 3; i < 6; ++i)
		_channels[i] = new SegaAudioChannel_SG(i, _sga, (i - 3) << 5);
	_channels[6] = new SegaAudioChannel_NG(6, _sga);
	for (int i = 7; i < 10; ++i)
		_channels[i] = new SegaAudioChannel_FM(i, _sga, 1, i - 7);

	_sga->writeReg(0, 0x27, 0x3F);
	_ready = true;
}

SegaAudioDriverInternal::~SegaAudioDriverInternal() {
	_ready = false;
	delete _sga;

	if (_channels) {
		for (int i = 0; i < 10; ++i)
			delete _channels[i];
		delete[] _channels;
	}
}

SegaAudioDriverInternal *SegaAudioDriverInternal::open(Audio::Mixer *mixer) {
	_refCount++;

	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new SegaAudioDriverInternal(mixer);
	else if (_refCount < 2 || _refInstance == 0)
		error("SegaAudioDriverInternal::open(): Internal instance management failure");

	return _refInstance;
}

void SegaAudioDriverInternal::close() {
	if (!_refCount)
		return;

	_refCount--;

	if (!_refCount) {
		delete _refInstance;
		_refInstance = 0;
	}
}

void SegaAudioDriverInternal::startFMSound(const uint8 *trackData, uint8 volume, uint8 prioFlags) {
	if (!_isPlaying)
		_priority = false;

	if (prioFlags & SegaAudioDriver::kPrioHigh)
		_priority = true;
	else if (_isPlaying && (_priority || (prioFlags & SegaAudioDriver::kPrioLow)))
		return;

	SegaAudioInterface::MutexLock lock = _sga->stackLockMutex();
	stop();

	_trackData = trackData;
	_attenuation = (uint8)-((volume & 0x0F) - 0x0F);
	_sfxInternal_a = 0;

	start();
}

void SegaAudioDriverInternal::loadPCMData(uint16 address, const uint8 *data, uint16 dataSize) {
	_sga->loadPCMData(address, data, dataSize);
}

void SegaAudioDriverInternal::startPCMSound(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 rate, uint8 pan, uint8 vol) {
	_sga->playPCMChannel(channel, dataStart, loopStart, rate, pan, vol);
}

void SegaAudioDriverInternal::setMusicVolume(int volume) {
	_sga->setMusicVolume(volume);
}

void SegaAudioDriverInternal::setSoundEffectVolume(int volume) {
	_sga->setSoundEffectVolume(volume);
}

void SegaAudioDriverInternal::timerCallbackA() {
	if (_ready && _isPlaying != 0xFF)
		update();
}

void SegaAudioDriverInternal::timerCallbackB() {

}

uint8 SegaAudioDriverInternal::calcVolume(int vol) {
	static const uint8 volTable[18] = { 0x5F, 0x2A, 0x28, 0x25, 0x22, 0x20, 0x1D, 0x1A, 0x18, 0x15, 0x12, 0x10, 0x0D, 0x0A, 0x08, 0x05, 0x02, 0x00 };
	vol = MAX<int>((int8)(vol & 0xFF) - _attenuation - _fadeAttenuation, 0) & 0xFF;
	assert(vol < ARRAYSIZE(volTable));
	return volTable[vol];
}

int SegaAudioDriverInternal::getFadeState() {
	return _fadeAttenuation ? _fadeTicker - 1 : -1;
}

const uint8 *SegaAudioDriverInternal::getTrack() {
	return _trackData;
}

const uint8 *SegaAudioDriverInternal::getTrack(int channel) {
	return _trackData ? _trackData + READ_LE_UINT16(_trackData + 1 + (channel << 1)) : 0;
}

const uint8 *SegaAudioDriverInternal::getProgram(int instrument) {
	return getTrack(12 + instrument);
}

void SegaAudioDriverInternal::start() {
	if (_isPlaying)
		stop();

	debugC(3, kDebugLevelSound, "%s", "\nStarting sound...");

	_isPlaying = 1;
	for (int i = 0; i < 10; ++i)
		_channels[i]->initTrack();

	_sga->writeReg(0, 0x26, 0xE9);
}

void SegaAudioDriverInternal::stop() {
	_fadeAttenuation = 15;
	_channels[0]->unlock();
	/*
	*/
	update();
	_fadeAttenuation = 0;
	_fadeTicker = 0;
	_isPlaying = 0;
	//_unkByte = 0;
	//_sai->writeReg(0, 0x27, 0);
}

void SegaAudioDriverInternal::fade() {
	if (!_isPlaying || _fadeAttenuation)
		return;
	_fadeAttenuation = 1;
	_fadeTicker = 12;
}

void SegaAudioDriverInternal::update() {
	if (_fadeAttenuation) {
		if (!--_fadeTicker) {
			_fadeTicker = 12;
			if (++_fadeAttenuation == 14)
				stop();
		}
	}

	for (int i = 0; i < 10; ++i) {
		if (!_channels[i]->update())
			stop();
	}
}

SegaAudioDriver::SegaAudioDriver(Audio::Mixer *mixer) {
	_drv = SegaAudioDriverInternal::open(mixer);
}

SegaAudioDriver::~SegaAudioDriver() {
	SegaAudioDriverInternal::close();
	_drv = 0;
}

void SegaAudioDriver::startFMSound(const uint8 *trackData, uint8 volume, PrioFlags prioFlags) {
	_drv->startFMSound(trackData, volume, (uint8)prioFlags);
}

void SegaAudioDriver::loadPCMData(uint16 address, const uint8 *data, uint16 dataLen) {
	_drv->loadPCMData(address, data, dataLen);
}

void SegaAudioDriver::startPCMSound(uint8 channel, uint8 dataStart, uint16 loopStart, uint16 rate, uint8 pan, uint8 vol) {
	_drv->startPCMSound(channel, dataStart, loopStart, rate, pan, vol);
}

void SegaAudioDriver::setMusicVolume(int volume) {
	_drv->setMusicVolume(volume);
}

void SegaAudioDriver::setSoundEffectVolume(int volume) {
	_drv->setSoundEffectVolume(volume);
}

} // End of namespace Kyra

#endif
