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


#include "audio/softsynth/fmtowns_pc98/sega_audio.h"
#include "backends/audiocd/audiocd.h"
#include "common/array.h"
#include "common/func.h"
#include "common/ptr.h"
#include "common/memstream.h"
#include "common/system.h"
#include "snatcher/resource.h"
#include "snatcher/sound_device.h"
#include "snatcher/staticres.h"
#include "snatcher/util.h"


namespace Snatcher {

class MessageHandler {
public:
	MessageHandler() : _mutex(), _msgCntW(0), _msgCntR(0) {
		_msg[0] = _msg[1] = _msg[2] = 0;
	}

	void write(uint8 a, uint8 b, uint8 c) {
		Common::StackLock lock(_mutex);
		_msg[0] = a;
		_msg[1] = b;
		_msg[2] = c;
		++_msgCntW;
	}

	void read(uint8 &a, uint8 &b, uint8 &c) {
		Common::StackLock lock(_mutex);
		a = _msg[0];
		b = _msg[1];
		c = _msg[2];
		_msgCntR = _msgCntW;
	}

	void clear() {
		Common::StackLock lock(_mutex);
		_msg[0] = _msg[1] = _msg[2] = _msgCntW = _msgCntR = 0;
	}

	bool hasNewMsg() const {
		Common::StackLock lock(_mutex);
		return (_msgCntR != _msgCntW);
	}

private:
	uint8 _msg[3];
	uint8 _msgCntW;
	uint8 _msgCntR;
	Common::Mutex _mutex;
};

class KonamiMCDAudioDriver;
class SegaSoundDevice : public SoundDevice {
public:
	SegaSoundDevice(FIO *fio);
	~SegaSoundDevice() override;

	bool init(Audio::Mixer *mixer) override;

	void cdaPlay(int track) override;
	void cdaStop() override;
	bool cdaIsPlaying() const override;
	uint32 cdaGetTime() const override;

	void fmSendCommand(int cmd, int restoreVolume) override;
	uint8 fmGetStatus() const override;
	void pcmSendCommand(int cmd, int arg) override;
	void pcmInitSound(int sndId) override;
	uint8 pcmGetStatus() const override;
	int16 pcmGetResourceId() const override;

	void pause(bool toggle) override;
	void update() override;

	void reduceVolume2(bool enable) override;

	void setMusicVolume(int vol) override;
	void setSoundEffectVolume(int vol) override;

private:
	bool fmLoadData();
	void fmUpdateVolumeFader();
	void fmHandleCmdQueue();

	uint8 *_fmCmdQueue;
	uint8 _fmCmdQueueWPos;
	uint8 _fmCmdQueueRPos;
	uint8 _fmVolume1;
	uint8 _fmLastVolume1;
	uint8 _fmVolume2;
	uint8 _fmLastVolume2;

	MessageHandler _driverMsg;
	KonamiMCDAudioDriver *_fmDriver;

private:
	bool pcmLoadData();
	void pcmDisableStereoMode(bool disable);
	void pcmStop();
	void pcmStartSound(uint8 sndId, int8 unit, uint8 d2 = 0);

	void pcmCheckPlayStatus();
	void pcmHandleDriverMessage();
	void pcmDelayedStart();

	typedef Common::SharedPtr<Common::SeekableReadStream> SmpStrPtr;
	struct PCMCacheEntry {
		PCMCacheEntry(uint32 ad, int16 no, Common::SharedPtr<uint8> ptr, uint32 sz) : addr(ad), resNo(no), data(ptr), dataSize(sz) {}
		uint32 addr;
		int16 resNo;
		Common::SharedPtr<const uint8> data;
		uint32 dataSize;
	};
	Common::Array<PCMCacheEntry> _pcmCache;
	SmpStrPtr createSamplesStream(uint32 addr);

	struct PCMResourceInfo {
		uint8 prio;
		uint8 mode;
		uint8 vol;
		uint8 pan;
		uint16 sector;
		uint16 numSectors;
	};

	struct PCMResidentDataInfo {
		uint8 prio;
		uint8 unit;
		uint8 vol;
		uint8 pan;
		uint16 rate;
		uint32 addr;
	};

	struct PCMInstrumentInfo {
		uint32 addr;
		uint32 loopst;
	};

	struct PCMSound {
		uint8 flags;
		uint8 id;
		uint8 prio;
		uint8 unit;
		uint32 addr;
		uint8 volBase;
		uint8 volEff;
		uint8 pan;
		uint16 rate;
	};

	void pcmResetUnit(uint8 unit, PCMSound &s);
	void pcmStartSoundInternal(uint8 unit, uint8 chan, PCMSound &s);
	void pcmStartInstrument(uint8 unit, uint8 sndId, uint8 vol);

	uint8 calcVolume(uint8 unit, PCMSound &s) const;
	uint8 calcPan(PCMSound &s) const;

	PCMResourceInfo *_pcmResourceInfo;
	PCMResidentDataInfo *_pcmResidentDataInfo;
	PCMInstrumentInfo *_pcmInstrumentInfo;
	PCMSound *_pcmSounds;

	bool _pcmDisableStereo;
	int16 _pcmResourceNumber;
	uint8 _pcmState;
	uint8 _pcmMode;
	uint8 _faderFlags;
	uint8 _faderTimer1;
	uint8 _faderTimer2;
	bool _reduceVolume2;
	uint8 _pcmMsgFromDriver[4];
	uint8 *_temp;

private:
	int _lastTrack;
	uint32 _pauseStartTime;
	FIO *_fio;
	SegaAudioInterface *_sai;
};

struct SndDriverState {
	SndDriverState() : flags(0), volume1(0), volume2(0), volUpdate(0), chanUse(0), progress(0), result(0) {
		msg[0] = msg[1] = msg[2] = 0;
		sfxId[0] = sfxId[1] = 0;
		restore[0] = restore[1] = 0;
	}

	void clear() {
		flags = 0;
		volume1 = volume2 = 0;
		volUpdate = 0;
		chanUse = 0;
		progress = result = 0;
		msg[0] = msg[1] = msg[2] = 0;
		sfxId[0] = sfxId[1] = 0;
		restore[0] = restore[1] = 0;

	}

	enum Flags : uint16 {
		kSndLockSfxChan1 =		1		<<		0,
		kSndLockSfxChan2 =		1		<<		1,
		kSndFadeoutDone =		1		<<		3,
		kSndKeepPanPos =		1		<<		5,
		kSndSfxMode =			1		<<		6,
		kSndSfxIsPlaying =		1		<<		7,
		kSndPitchAdjust =		1		<<		10,
		kSndDisableStereo =		1		<<		11,
		kSndParseBreak =		1		<<		12,
		kSndSfxStarting =		1		<<		13,
		kSndBlockChannels =		1		<<		14
	};

	uint8 volume1, volume2;
	uint16 flags;
	uint16 volUpdate;
	uint16 chanUse;
	uint8 msg[3];
	uint8 sfxId[2];
	uint8 restore[2];
	uint8 progress;
	uint8 result;
};

class KonamiMCDSndChannel {
public:
	KonamiMCDSndChannel(int id, SegaAudioInterface *sai, SndDriverState &state);
	virtual ~KonamiMCDSndChannel() {}

	void clear();

	void loadInstrumentData(const uint8 *data, uint32 dataSize);
	void initSound(const uint8 *data, const uint8 *dataEnd, uint8 sndId, int16 prio);
	void setVolume(int vol);
	void writeReg(uint8 reg, uint8 val);

	void stop();
	void off();

	bool busy() const { return _sndId != 0; }
	uint8 getId() const { return _id; }
	uint8 getNumber() const { return _num; }
	uint8 getPrio() const { return _prio; }

	virtual bool checkTicker() = 0;
	virtual void update() = 0;

protected:
	void nextTick(const uint8 *&data);
	void updateVolumeAndFrequency();

	void chanOff();
	virtual void noteOn(uint8 note, const uint8 *&data);
	void runOpcode(uint8 opcode, const uint8 *&data);
	void addToMessage(uint8 cmd, const uint8 *&data);

	void panEffect();
	void applyLFOSensitivity();
	void applyAmpModSensitivity();
	void checkNoteTimer();
	void applyVolumeSlide();

	void keyOff();
	void keyOn();

	void programChange(const uint8 *&data);
	void setPitchBend(const uint8 *&data);
	void calcPitchBend(uint16 lastFreq);
	void jump(const uint8 *&data);
	void checkPtr(const uint8 *data, uint8 bytesNeeded, const char *caller);
	void finish();

	void applyPitchBend();
	int16 getVbrModifier();
	void applyFreqModifiers(int16 vbrMod);

	void recalcAndSetChannelVolume();
	void setParameter(uint8 para, uint8 val);
	void setFrequency(uint16 frq);
	void setProgram(uint8 prg);

	bool blockedBySfx() const {
		return ((_state.flags & SndDriverState::kSndSfxIsPlaying) &&
			((_id == 4 && ((_state.flags & (SndDriverState::kSndSfxMode | SndDriverState::kSndLockSfxChan1)) == SndDriverState::kSndLockSfxChan1)) ||
				(_id == 5 && ((_state.flags & (SndDriverState::kSndSfxMode | SndDriverState::kSndLockSfxChan2)) == SndDriverState::kSndLockSfxChan2))));
	}

protected:
	enum Flags : uint8 {
		KChSubroutine		=		1		<<		2,
		KChPortamento		=		1		<<		4,
		kChBlockVolume		=		1		<<		5,
		kChFinishedTheme	=		1		<<		6,
		kChRepeatingTheme	=		1		<<		7
	};

	uint8 _sndId, _prio, _section1RepeatCounter, _section2RepeatCounter, _speed, _ticker, _pitchBendSteps, _pitchBendDuration, _pitchBendDelayTimer, _pitchBendDelay, _vbrDelayTimer, _vbrState,
		_vbrRate, _vbrDepthModulationRateCounter, _vbrDelay, _vbrDepthModulationRate, _vbrDepth, _velocity, _volSlideDuration, _volSlideDest, _panEffectRateCounter, _panEffectRate, _panPosition,
		_panEffectType, _panEffectState, _noteTimer, _eventDelay, _noteDuration, _lastNoteDuration, _eventDelayTimer, _pbExtTimer, _pbExtSpeed, _flags, _program, _enableDelayedEffects, _algorithm,
		_lfoSensitivity, _ampModSensitivity, _lfoSensitivityDelay, _ampModSensitivityDelay, _lfoSensitivityDelayTimer, _ampModSensitivityDelayTimer, _regB4, _resetFlag, _signalNoteOff, _volTotal;
	uint8 _tl[4];
	int8 _transposeMSB;
	int16 _section1VolAdjust, _section2VolAdjust, _section1Transpose, _section2Transpose, _volSlideRate, _pitchBendRate, _pitchBendDist, _vbrDepthRate, _transposeLSB;
	uint16 _volume, _freqTotal, _pitchBendDest, _vbrDepthCur, _pbExtPos, _pbExtMask, _decayRate, _decayPhase, _freqLast;
	const uint8 *_returnPos, *_themeStart, *_resumeAfterThemePos, *_sectionMarker1, *_sectionMarker2, *_trackCurPos, *_trackStart, *_dataEnd;
	const uint8 _id, _num, _part, _regOffs;
	SegaAudioInterface *_sai;

	const uint8 *_instrumentData;
	uint32 _instrumentDataSize;

	SndDriverState &_state;

private:
	void makeFunctions();

	void op_nop(const uint8*&);
	void op_break_toneOff(const uint8 *&data);
	void op_break_keepTone(const uint8 *&data);
	void op_programChange(const uint8 *&data);
	void op_panPos(const uint8 *&data);
	void op_vibrato(const uint8 *&data);
	void op_pitchBendSpecial(const uint8 *&data);
	void op_setRepeatSectionMarker1(const uint8 *&data);
	void op_repeatSection1(const uint8 *&data);
	void op_setRepeatSectionMarker2(const uint8 *&data);
	void op_repeatSection2(const uint8 *&data);
	void op_setSpeed(const uint8 *&data);
	void op_panEffect(const uint8 *&data);
	void op_transposeCr(const uint8 *&data);
	void op_toggleLFO(const uint8 *&data);
	void op_setVolume(const uint8 *&data);
	void op_volumeSlide(const uint8 *&data);
	void op_portamento(const uint8 *&data);
	void op_pitchWheel(const uint8 *&data);
	void op_transposeFn(const uint8 *&data);
	void op_lfoSensivity(const uint8 *&data);
	void op_ampModSensitivity(const uint8 *&data);
	void op_setThemeMarker(const uint8 *&data);
	void op_repeatTheme(const uint8 *&data);
	void op_vibratoDepthModulation(const uint8 *&data);
	void op_envRate(const uint8 *&data);
	void op_changePitchBendMod(const uint8 *&data);
	void op_jump(const uint8 *&data);
	void op_jumpToSubroutine(const uint8 *&data);
	void op_return(const uint8 *&data);

#ifdef SNDDRV_DEBUG
	typedef Common::Functor1Mem<const uint8*&, void, KonamiMCDSndChannel> ChanOpcodeFunc;
	struct ChanOpcode {
		ChanOpcode(ChanOpcodeFunc *func, const char *desc, int chan, int dataLen) : _func(func), _desc(desc), _chan(chan), _dataLen(dataLen) {}
		~ChanOpcode() { delete _func; }
		bool isValid() { return _func->isValid(); }
		void operator()(const uint8*& data) const {
			Common::String dstr = "";
			for (int i = 0; i < _dataLen; ++i)
				dstr += Common::String::format("%02X ", data[i]);
			debug( "Channel %d: %s() [ %s]", _chan, _desc, dstr.c_str());
			(*_func)(data);
		}
		ChanOpcodeFunc *_func;
		const char *_desc;
		const int _dataLen;
		const int _chan;
	};
#else
	typedef Common::Functor1Mem<const uint8*&, void, KonamiMCDSndChannel> ChanOpcode;
#endif
	Common::Array<Common::SharedPtr<ChanOpcode> > _opcodes;

	static const uint8 _carrier[8];
};

class KonamiMCDSndChannelDef : public KonamiMCDSndChannel {
public:
	KonamiMCDSndChannelDef(int id, SegaAudioInterface *sai, SndDriverState &state) : KonamiMCDSndChannel(id, sai, state) {}
	~KonamiMCDSndChannelDef() override {}
	bool checkTicker() override;
	void update() override;
};

class KonamiMCDSndChannelRtm : public KonamiMCDSndChannel {
public:
	KonamiMCDSndChannelRtm(int id, SegaAudioInterface *sai, SndDriverState &state) : KonamiMCDSndChannel(id, sai, state) {}
	~KonamiMCDSndChannelRtm() override {}
	bool checkTicker() override;
	void update() override;
};

class KonamiMCDAudioDriver : public SegaAudioPluginDriver {
public:
	KonamiMCDAudioDriver(Audio::Mixer *mixer, MessageHandler *msg = nullptr);
	~KonamiMCDAudioDriver();

	bool init();
	bool loadData(const uint8 *data, uint32 dataSize);

	void doCommand(int cmd, int arg);
	void setVolume(uint8 vol1, uint8 vol2);

	uint8 getStatus() const;

	void timerCallbackA() override;

private:
	void reset();
	void stopChannels(int scope);
	void disableStereoMode(bool disable);

	void handleComList();
	void nextTick();

	void startSound(uint8 sndId);

	void updateRtmChannels();
	void updateSfxChannels();

	const uint8 *getTrack(uint32 index) const;

	SndDriverState _state;

	uint8 _curSfxPrio;
	uint8 _curTrackPrio;

	uint8 _comList[32];
	uint8 _comListWritePos;
	uint8 _comListWritePosPrev;
	uint8 _comListReadPos;

	MessageHandler *_msg;

	KonamiMCDSndChannel **_channels;
	SegaAudioInterface *_sai;

	uint8 *_instrumentData;
	const uint16 _instrumentDataSize;
	uint8 *_trackData;
	const uint16 _trackDataSize;
	uint16 _trackDataSize2;
	uint8 *_trackDataEnd;
};

KonamiMCDSndChannel::KonamiMCDSndChannel(int id, SegaAudioInterface *sai, SndDriverState &state) : _num(id), _id(id > 7 ? id - 4 : id), _sai(sai), _state(state),
	_regOffs((id < 7 ? id : id - 1) % 3), _part(id < 3 ? 0 : 1), _prio(0), _freqLast(0), _sndId(0), _section1RepeatCounter(0), _speed(0), _ticker(0),
	_ampModSensitivity(0), _pitchBendSteps(0), _vbrDepthModulationRate(0), _vbrDepth(0), _velocity(0), _volSlideDuration(0), _panEffectRateCounter(0), _noteTimer(0), _eventDelayTimer(0),
	_panEffectRate(0), _panPosition(0), _panEffectType(0),	_panEffectState(0), _pbExtSpeed(0), _transposeMSB(0), _flags(0), _program(0), _enableDelayedEffects(0), _algorithm(0),
	_lfoSensitivityDelay(0), _regB4(0), _resetFlag(0), _section1VolAdjust(0), _section2VolAdjust(0), _section1Transpose(0), _section2Transpose(0), _pitchBendDuration(0), _volume(0),
	_vbrDepthCur(0), _transposeLSB(0), _decayRate(0), _decayPhase(0), _pbExtTimer(0), _pbExtPos(0), _pbExtMask(0), _themeStart(nullptr), _resumeAfterThemePos(nullptr),
	_sectionMarker1(nullptr), _sectionMarker2(nullptr), _pitchBendDelay(0), _trackCurPos(nullptr), _volTotal(0), _returnPos(nullptr), _lfoSensitivity(0), _eventDelay(0),
	_noteDuration(0), _lastNoteDuration(0), _pitchBendDelayTimer(0), _freqTotal(0), _pitchBendRate(0), _pitchBendDest(0), _vbrDelayTimer(0), _vbrDepthModulationRateCounter(0),
	_vbrState(0), _vbrRate(0), _vbrDepthRate(0), _volSlideRate(0), _pitchBendDist(0), _ampModSensitivityDelay(0), _instrumentData(nullptr), _vbrDelay(0),
	_lfoSensitivityDelayTimer(0), _ampModSensitivityDelayTimer(0), _signalNoteOff(0), _section2RepeatCounter(0), _instrumentDataSize(0) {
	_tl[0] = _tl[1] = _tl[2] = _tl[3] = 0;
	makeFunctions();
}

void KonamiMCDSndChannel::clear() {
	_sndId = _prio = _speed = _ticker = _pitchBendSteps = _vbrDepthModulationRate = _vbrDepth = _velocity = _volSlideDuration = _panEffectRateCounter = _noteTimer = _pbExtTimer = _pbExtSpeed = 0;
	_flags = _program = _enableDelayedEffects = _tl[0] = _tl[1] = _tl[2] = _tl[3] = _vbrDelay = _lfoSensitivityDelayTimer = _ampModSensitivityDelayTimer = _signalNoteOff = _algorithm = 0;
	_lfoSensitivityDelay = _resetFlag = _regB4 = _eventDelayTimer = _pitchBendDuration = _pitchBendDelay = _ampModSensitivity = _lfoSensitivity = 0;
	_transposeMSB = 0;
	_noteTimer = _eventDelay = _noteDuration = _lastNoteDuration = _volTotal = _panEffectRate = _panPosition = _panEffectType = _panEffectState = _pitchBendDelayTimer = _vbrDelayTimer = 0;
	_vbrDepthModulationRateCounter = _vbrState = _vbrRate = _ampModSensitivityDelay = 0;
	_section1RepeatCounter = _section2RepeatCounter = _section1VolAdjust = _section2VolAdjust = _decayPhase = _section1Transpose = _vbrDepthCur = _decayRate = _freqTotal = 0;
	_pitchBendRate = _pitchBendDest = _vbrDepthRate = _freqLast = _pbExtPos = _pbExtMask = _volume = _volSlideRate = _pitchBendDist = 0;
	_section1Transpose = _section2Transpose = _transposeLSB = 0;
	_themeStart = _resumeAfterThemePos = _sectionMarker1 = _sectionMarker2 = _trackCurPos = _returnPos = nullptr;
	_state.volUpdate &= ~(1 << _id);
}

void KonamiMCDSndChannel::loadInstrumentData(const uint8 *data, uint32 dataSize) {
	_instrumentData = data;
	_instrumentDataSize = dataSize;
}

void KonamiMCDSndChannel::initSound(const uint8 *data, const uint8 *dataEnd, uint8 sndId, int16 prio) {
	_sndId = sndId;
	_trackStart = _themeStart = _sectionMarker1 = _sectionMarker2 = _trackCurPos = data;
	_dataEnd = dataEnd;
	_speed = _program = 0xFF;
	_eventDelayTimer = _ticker = 1;
	_volume = 0x7F00;
	_velocity = 0x7F;
	_flags = 0;

	if (prio != -1) {
		_prio = prio;
		_flags |= 1;
	}

	_pbExtSpeed = _volSlideDuration = _panEffectRateCounter = _section2RepeatCounter = _section1RepeatCounter = _noteTimer = _eventDelay = _noteDuration =
		_lastNoteDuration = _regB4 = _lfoSensitivityDelay = _ampModSensitivityDelay = _pitchBendDuration = _vbrDepthModulationRate = _vbrDepth = _pitchBendSteps = _volTotal = 0;
	_transposeMSB = 0;
	_section1VolAdjust = _section2VolAdjust = _section1Transpose = _vbrDepthCur = _decayRate = _freqLast = 0;
	_section1Transpose = _section2Transpose = _transposeLSB = 0;
	_state.volUpdate &= ~(1 << _id);

	if (_id < 6)
		_resetFlag = 1;
}

void KonamiMCDSndChannel::setVolume(int vol) {
	if (blockedBySfx())
		return;

	static const uint8 volReg[] = { 0x40, 0x48, 0x44, 0x4C };
	uint8 cr = _carrier[_algorithm];

	for (int i = 3; i >= 0; --i) {
		if (cr & 0x80)
			writeReg(volReg[i], MIN<uint16>(_tl[i] + 127 - vol, 127));
		cr <<= 1;
	}
}

void KonamiMCDSndChannel::writeReg(uint8 reg, uint8 val) {
	_sai->writeReg(_part, reg + _regOffs, val);
}

void KonamiMCDSndChannel::stop() {
	_sndId = _prio = 0;
	if (_id < 6)
		_resetFlag = 1;
}

void KonamiMCDSndChannel::off() {
	for (uint8 r = 0x80; r < 0x90; r += 4)
		writeReg(r, 0xFF);
	keyOff();
}

bool KonamiMCDSndChannelDef::checkTicker() {
	bool oflw = (_ticker + _speed) > 0xFF;
	_ticker += _speed;
	return oflw;
}

bool KonamiMCDSndChannelRtm::checkTicker() {
	bool oflw = (_ticker + _speed) > 0xFF;
	_ticker += _speed;
	if (!oflw || --_eventDelayTimer)
		return false;
	return true;
}

void KonamiMCDSndChannelDef::update() {
	if ((_num == 4 || _num == 5) && _state.restore[_num - 4]) {
		_state.flags |= SndDriverState::kSndKeepPanPos;
		setProgram(_program);
		_state.flags &= ~SndDriverState::kSndKeepPanPos;
		_decayPhase = 0xFFFF;
		recalcAndSetChannelVolume();
		_state.restore[_num - 4] = 0;
	}

	if (_resetFlag) {
		setParameter(0, 0);
		chanOff();
		_resetFlag = 0;
	}

	if (_sndId == 0)
		return;

	if (checkTicker()) {
		_enableDelayedEffects = 1;
		nextTick(_trackCurPos);
	} else {
		_enableDelayedEffects = 0;
		updateVolumeAndFrequency();
	}
}

void KonamiMCDSndChannelRtm::update() {
	for (bool lp = true; lp; ) {
		checkPtr(_trackCurPos, 1, __FUNCTION__);
		uint8 in = *_trackCurPos++;
		if (in < 96)
			continue;
		if (in < 224) {
			addToMessage(in - 96, _trackCurPos);
			lp = false;
		} else {
			_state.flags &= ~SndDriverState::kSndParseBreak;
			runOpcode(in - 224, _trackCurPos);
			lp = !(_state.flags & SndDriverState::kSndParseBreak);
		}
	}
}

void KonamiMCDSndChannel::nextTick(const uint8 *&data) {
	if (--_eventDelayTimer == 0) {
		for (bool lp = true; lp; ) {
			checkPtr(data, 1, __FUNCTION__);
			uint8 in = *data++;
			if (in > 223) {
				_state.flags &= ~SndDriverState::kSndParseBreak;
				runOpcode(in - 224, data);
				lp = !(_state.flags & SndDriverState::kSndParseBreak);
			} else if ((in & 0x7F) > 95) {
				continue;
			} else if (in) {
				noteOn(in, data);
				lp = false;
			}
		}
	} else {
		if (_panEffectRate != 0)
			panEffect();

		if (_lfoSensitivityDelay)
			applyLFOSensitivity();

		if (_ampModSensitivityDelay)
			applyAmpModSensitivity();

		if (_noteDuration != 0 && _noteDuration != 127)
			checkNoteTimer();

		if (_eventDelayTimer == 1 && (_id & 1) && *data == 0xE2)
			programChange(++data);

		updateVolumeAndFrequency();
	}
}

void KonamiMCDSndChannel::updateVolumeAndFrequency() {
	_state.flags &= ~SndDriverState::kSndPitchAdjust;
	if (_pitchBendSteps)
		applyPitchBend();

	int16 v = getVbrModifier();
	applyFreqModifiers(v);

	if (_flags & kChBlockVolume) {
		_flags &= ~kChBlockVolume;
		return;
	}

	if (_volSlideDuration)
		applyVolumeSlide();

	if (_state.volume1 != 0xFF || _state.volume2 != 0xFF)
		_state.volUpdate = 0x33F;

	if (_state.volUpdate & (1 << _id))
		recalcAndSetChannelVolume();
}

void KonamiMCDSndChannel::chanOff() {
	if (!blockedBySfx()) {
		for (uint8 r = 0x80; r < 0x90; r += 4)
			writeReg(r, 0xFF);
	}
	keyOff();
}

void KonamiMCDSndChannel::noteOn(uint8 note, const uint8 *&data) {
#ifdef SNDDRV_DEBUG
	const uint8 *dbgStart = data;
#endif
	if (_signalNoteOff) {
		_signalNoteOff = 0;
		keyOff();
	}
	_state.volUpdate |= (1 << _id);

	if (!(note & 0x80)) {
		checkPtr(data, 1, __FUNCTION__);
		_eventDelay = *data++;
	}

	_eventDelayTimer = _eventDelay;

	checkPtr(data, 1, __FUNCTION__);
	uint8 a = *data++; // if flagged with 0x80, this is the new duration, otherwise it's the velocity
	uint8 b = (a & 0x80) ? _noteDuration : a;

	if (b != 0 && b != 127 && _eventDelay == 1)
		_signalNoteOff = 1;
	_noteDuration = b;

	if (a & 0x80) {
		_velocity = a & 0x7F;
	} else {
		checkPtr(data, 1, __FUNCTION__);
		_velocity = (*data++) & 0x7F;
	}

	_noteTimer = (_noteDuration != 0 && _noteDuration != 127) ? MAX<uint8>(1, (_noteDuration * 2 * _eventDelay) >> 8) : _eventDelay;

	uint16 fn = _freqTotal;
	_freqTotal = (((note + _transposeMSB) << 8) + _section1Transpose + _section2Transpose + _transposeLSB) & 0x7FFF;

	_vbrDelayTimer = _vbrDelay;
	_lfoSensitivityDelayTimer = _lfoSensitivityDelay;
	_ampModSensitivityDelayTimer = _ampModSensitivityDelay;
	_vbrDepthModulationRateCounter = _vbrDepthModulationRate;
	if (_vbrDepthModulationRateCounter != 0)
		_vbrDepthCur = 0;
	_vbrState = 0;
	_decayPhase = 0;

	checkPtr(data, 1, __FUNCTION__);
	if (!*data)
		++data;

	if (*data == 0xF3) {
		setPitchBend(data);
	} else {
		_pitchBendSteps = _pitchBendDuration;
		if (_pitchBendDuration)
			calcPitchBend(fn);
	}

	setFrequency(_freqTotal);

	if (_lfoSensitivityDelay)
		setParameter(1, 0);

	if (_ampModSensitivityDelay)
		setParameter(2, 0);

	if (_flags & kChBlockVolume) {
		_flags &= ~kChBlockVolume;
	} else {
		if (_volSlideDuration)
			applyVolumeSlide();

		if (_state.volume1 != 0xFF || _state.volume2 != 0xFF)
			_state.volUpdate = 0x33F;

		if (_state.volUpdate & (1 << _id))
			recalcAndSetChannelVolume();
	}

	if (_lastNoteDuration != 127)
		keyOn();

	_lastNoteDuration = _noteDuration;

#ifdef SNDDRV_DEBUG
	Common::String dbgMsg = Common::String::format("%02X ", note);
	for (int i = 0; i < data - dbgStart; ++i)
		dbgMsg += Common::String::format("%02X ", dbgStart[i]);
	debug("Channel %d: noteOn(): [ %s]", _num, dbgMsg.c_str());
#endif
}

void KonamiMCDSndChannel::runOpcode(uint8 opcode, const uint8 *&data) {
	if (opcode < _opcodes.size() && _opcodes[opcode]->isValid())
		(*_opcodes[opcode])(data);
	else
		error("%s(): Invalid opcode %d", __FUNCTION__, opcode);
}

void KonamiMCDSndChannel::addToMessage(uint8 cmd, const uint8 *&data) {
	if (_id == 7)
		_state.msg[1] |= (cmd << 3);
	else
		_state.msg[0] |= cmd;

	uint16 v = 0;
	checkPtr(data, 3, __FUNCTION__);
	_eventDelayTimer = _eventDelay = *data++;
	_lastNoteDuration = *data++;
	v = ((*data++) * (_volume >> 8)) >> 9;

	if (_id == 7) {
		_state.msg[2] |= (v & 0xFF);
	} else {
		_state.msg[1] |= (v & 7);
		_state.msg[2] |= ((v << 2) & 0x60);
	}
}

void KonamiMCDSndChannel::panEffect() {
	static const uint8 panTable[4][12] = {
		{ 0x40, 0x80, 0x40, 0x80, 0x40, 0x80, 0x40, 0x80, 0x40, 0x80, 0x40, 0x80 },
		{ 0xc0, 0x40, 0xc0, 0x80, 0xc0, 0x40, 0xc0, 0x80, 0xc0, 0x40, 0xc0, 0x80 },
		{ 0xc0, 0x40, 0x80, 0xc0, 0x40, 0x80, 0xc0, 0x40, 0x80, 0xc0, 0x40, 0x80 },
		{ 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40, 0xc0, 0x80, 0x40 }
	};

	if (--_panEffectRateCounter)
		return;
	_panEffectRateCounter = _panEffectRate;

	assert(_panEffectType < 4);
	assert(_panEffectState < 12);
	_panPosition = panTable[_panEffectType][_panEffectState];
	_panEffectState = (_panEffectState + 1) % 12;

	setParameter(3, _panPosition);
}

void KonamiMCDSndChannel::applyLFOSensitivity() {
	if (_lfoSensitivityDelayTimer && !--_lfoSensitivityDelayTimer)
		setParameter(1, _lfoSensitivity);
}

void KonamiMCDSndChannel::applyAmpModSensitivity() {
	if (_ampModSensitivityDelayTimer && !--_ampModSensitivityDelayTimer)
		setParameter(2, _ampModSensitivity);
}

void KonamiMCDSndChannel::checkNoteTimer() {
	if (_noteTimer != 0) {
		if (--_noteTimer)
			return;
		keyOff();
	}

	if (_decayRate != 0) {
		_state.volUpdate |= (1 << _id);
		_decayPhase = MIN<uint32>(_decayPhase + _decayRate, 0xFFFF);
	}
}

void KonamiMCDSndChannel::applyPitchBend() {
	if (_pitchBendDelayTimer) {
		if (_enableDelayedEffects)
			--_pitchBendDelayTimer;
		return;
	}

	if (--_pitchBendSteps)
		_freqTotal += _pitchBendRate;
	else
		_freqTotal = _pitchBendDest;

	_state.flags |= SndDriverState::kSndPitchAdjust;
}

void KonamiMCDSndChannel::applyVolumeSlide() {
	if (--_volSlideDuration)
		_volume += _volSlideRate;
	else
		_volume = _volSlideDest << 8;
	_state.volUpdate |= (1 << _id);
}

void KonamiMCDSndChannel::keyOff() {
	if (blockedBySfx() || _id > 5)
		return;
	_sai->writeReg(0, 0x28, _id + (_id > 2 ? 1 : 0));
}

void KonamiMCDSndChannel::keyOn() {
	if (blockedBySfx() || _id > 5)
		return;
	_sai->writeReg(0, 0x28, 0xF0 + _id + (_id > 2 ? 1 : 0));
}

void KonamiMCDSndChannel::programChange(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	uint8 prg = *data++;
	if (prg == _program)
		return;
	_program = prg;
	_decayRate = 0;
	setProgram(prg);
}

void KonamiMCDSndChannel::setPitchBend(const uint8 *&data) {
	++data;
	checkPtr(data, 3, __FUNCTION__);
	_pitchBendDelayTimer = *data++;
	_pitchBendSteps = *data++;
	_pitchBendDest = (_transposeMSB + *data++) << 8;
	if (!_pitchBendSteps)
		return;
	checkPtr(data, 3, __FUNCTION__);
	++data;
	_pitchBendRate = READ_LE_INT16(data); // really LE
	data += 2;
}

void KonamiMCDSndChannel::calcPitchBend(uint16 lastFreq) {
	_pitchBendDelayTimer = _pitchBendDelay;
	if (_flags & KChPortamento) {
		SWAP(_freqTotal, lastFreq);
		if (lastFreq >= 0x8000)
			lastFreq = 0;
		else if (lastFreq >= 0x6000)
			lastFreq = 0x5FFF;
		_pitchBendDest = lastFreq;

		int16 diff = lastFreq - _freqTotal;
		if (diff == 0)
			diff = 1;

		_pitchBendRate = diff / _pitchBendSteps;

	} else {
		_pitchBendDest = _freqTotal;
		_freqTotal -= _pitchBendDist;
	}
}

void KonamiMCDSndChannel::jump(const uint8 *&data) {
	checkPtr(data, 4, __FUNCTION__);
	data += READ_BE_INT32(data);
	checkPtr(data, 1, __FUNCTION__);
}

void KonamiMCDSndChannel::checkPtr(const uint8 *data, uint8 bytesNeeded, const char *caller) {
	if (data < _trackStart || data > _dataEnd - bytesNeeded)
		error("%s(): Invalid memory access attempt", caller);
}

void KonamiMCDSndChannel::finish() {
	if ((_state.flags & SndDriverState::kSndSfxIsPlaying) && (_id == 4 || _id == 5) && (_state.flags & (_id - 3))) {
		_state.flags &= ~(_id - 3);
		_state.sfxId[_id - 4] = 0;
		_state.restore[_id - 4] = 1;
		_prio = 0;
		_sai->setSoundEffectChanMask(0x1E00 | ((_state.flags & (SndDriverState::kSndLockSfxChan1 | SndDriverState::kSndLockSfxChan2)) << 4));
	}

	if (!(_state.flags & (SndDriverState::kSndLockSfxChan1 | SndDriverState::kSndLockSfxChan2))) {
		_state.flags &= ~SndDriverState::kSndSfxIsPlaying;
		_state.progress |= 0x40;
	}
}

int16 KonamiMCDSndChannel::getVbrModifier() {
	if (!_vbrDepth || _vbrDelayTimer) {
		if (_vbrDepth && _enableDelayedEffects)
			--_vbrDelayTimer;
		return 0;
	}

	if (_vbrDepthModulationRateCounter && _enableDelayedEffects) {
		if (--_vbrDepthModulationRateCounter) {
			_vbrDepthCur += _vbrDepthRate;
		} else {
			_vbrDepthCur = _vbrDepth << 8;
		}
	}

	_vbrState += _vbrRate;
	_state.flags |= SndDriverState::kSndPitchAdjust;

	static const uint8 vbrTable[] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11,
		0x10, 0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01
	};

	int16 res = vbrTable[(_vbrState >> 1) & 0x3F] * (_vbrDepthCur >> 8);
	if (!(_vbrDepth & 0x80))
		res >>= 2;

	return (_vbrState & 0x80) ? -res : res;
}

void KonamiMCDSndChannel::applyFreqModifiers(int16 vbrMod) {
	int16 pbExtMod = 0;
	if (_pbExtSpeed) {
		bool ovflw = (_pbExtTimer + _pbExtSpeed > 0xFF);
		_pbExtTimer += _pbExtSpeed;
		if (!ovflw)
			return;
		pbExtMod = READ_BE_INT16(StaticRes_SCD::_fmSndPitchSpecialTable + _pbExtPos) & _pbExtMask;
		_pbExtPos = (_pbExtPos + 1) & 0x1FF;
	}

	if (!pbExtMod && !(_state.flags & SndDriverState::kSndPitchAdjust))
		return;

	setFrequency(vbrMod + pbExtMod + _freqTotal);
}

void KonamiMCDSndChannel::recalcAndSetChannelVolume() {
	int vol = MAX<int>(0, _velocity - (_decayPhase >> 8) - (127 - (_volume >> 8)) + _section1VolAdjust + _section2VolAdjust);
	if (vol < 128)
		vol = 64 + (vol >> 1);
	else
		vol = 127;
	_volTotal = vol;

	if (_state.volume2 != 0xFF)
		vol = (vol * (_state.volume2 >> 1)) >> 7;

	if (_state.volume1 != 0xFF) {
		if (!(_state.flags & SndDriverState::kSndSfxMode) || (_id != 4 && _id != 5) || !(_state.flags & (_id - 3)))
			vol = (vol * (_state.volume1 >> 1)) >> 7;
	}

	setVolume(vol & 0xFF);
	_state.volUpdate &= ~(1 << _id);
}

void KonamiMCDSndChannel::setParameter(uint8 para, uint8 val) {
	static const uint8 mask[4] = { 0xC0, 0xF0, 0xCF, 0x3F };
	assert(para < ARRAYSIZE(mask));
	_regB4 &= mask[para];

	if (para > 0) {
		if (blockedBySfx())
			return;
		_regB4 |= val;
	}

	// Stereo disabled
	if (_state.flags & SndDriverState::kSndDisableStereo)
		_regB4 |= 0xC0;

	writeReg(0xB4, _regB4);
}

void KonamiMCDSndChannel::setFrequency(uint16 frq) {
	if (blockedBySfx())
		return;

	static const uint8 freqTableA[128] = {
		0x26, 0x28, 0x2c, 0x2d, 0x30, 0x34, 0x36, 0x39, 0x3d, 0x40, 0x44, 0x49, 0x26, 0x28, 0x2c, 0x2d,
		0x30, 0x34, 0x36, 0x39, 0x3d, 0x40, 0x44, 0x49, 0x26, 0x28, 0x2c, 0x2d, 0x30, 0x34, 0x36, 0x39,
		0x3d, 0x40, 0x44, 0x49, 0x26, 0x28, 0x2c, 0x2d, 0x30, 0x34, 0x36, 0x39, 0x3d, 0x40, 0x44, 0x49,
		0x26, 0x28, 0x2c, 0x2d, 0x30, 0x34, 0x36, 0x39, 0x3d, 0x40, 0x44, 0x49, 0x26, 0x28, 0x2c, 0x2d,
		0x30, 0x34, 0x36, 0x39, 0x3d, 0x40, 0x44, 0x49, 0x26, 0x28, 0x2c, 0x2d, 0x30, 0x34, 0x36, 0x39,
		0x3d, 0x40, 0x44, 0x49, 0x26, 0x28, 0x2c, 0x2d, 0x30, 0x34, 0x36, 0x39, 0x3d, 0x40, 0x44, 0x49,
		0x06, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x0a, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0e, 0x0e,
		0x0f, 0x10, 0x11, 0x24, 0x13, 0x14, 0x16, 0x16, 0x18, 0x1a, 0x1b, 0x1d, 0x1e, 0x20, 0x22, 0x25
	};

	static const uint16 freqTableB[128] = {
		0x0284, 0x02aa, 0x02d2, 0x02fe, 0x032b, 0x035b, 0x038f, 0x03c5,	0x03fe, 0x043b, 0x047b, 0x04bf, 0x0a84, 0x0aaa, 0x0ad2, 0x0afe,
		0x0b2b, 0x0b5b, 0x0b8f, 0x0bc5, 0x0bfe, 0x0c3b, 0x0c7b, 0x0cbf,	0x1284, 0x12aa, 0x12d2, 0x12fe, 0x132b, 0x135b, 0x138f, 0x13c5,
		0x13fe, 0x143b, 0x147b, 0x14bf, 0x1a84, 0x1aaa, 0x1ad2, 0x1afe,	0x1b2b, 0x1b5b, 0x1b8f, 0x1bc5, 0x1bfe, 0x1c3b, 0x1c7b, 0x1cbf,
		0x2284, 0x22aa, 0x22d2, 0x22fe, 0x232b, 0x235b, 0x238f, 0x23c5,	0x23fe, 0x243b, 0x247b, 0x24bf, 0x2a84, 0x2aaa, 0x2ad2, 0x2afe,
		0x2b2b, 0x2b5b, 0x2b8f, 0x2bc5, 0x2bfe, 0x2c3b, 0x2c7b, 0x2cbf,	0x3284, 0x32aa, 0x32d2, 0x32fe, 0x332b, 0x335b, 0x338f, 0x33c5,
		0x33fe, 0x343b, 0x347b, 0x34bf, 0x3a84, 0x3aaa, 0x3ad2, 0x3afe, 0x3b2b, 0x3b5b, 0x3b8f, 0x3bc5, 0x3bfe, 0x3c3b, 0x3c7b, 0x3cbf,
		0x3d08, 0x006b, 0x0071, 0x0078, 0x007f, 0x0087, 0x008f, 0x0097, 0x00a1, 0x00aa, 0x00b4, 0x00bf, 0x00ca, 0x00d6, 0x00e3, 0x00f1,
		0x00ff, 0x010e, 0x011e, 0x012f, 0x0142, 0x0155, 0x0169, 0x017f, 0x0195, 0x01ad, 0x01c7, 0x01e2, 0x01ff, 0x021d, 0x023d, 0x025f
	};

	frq &= 0x7fff;
	_freqLast = frq;

	frq = (((frq & 0xFF) * freqTableA[frq >> 8]) >> 8) + freqTableB[frq >> 8];

	writeReg(0xA4, frq >> 8);
	writeReg(0xA0, frq & 0xFF);
}

void KonamiMCDSndChannel::setProgram(uint8 prg) {
	if (blockedBySfx())
		return;
#ifdef SNDDRV_DEBUG
	debug("Channel %d: Setting program %d", _num, prg);
#endif
	const uint8 *in = _instrumentData;
	assert(in);
	assert((uint32)(prg << 2) < _instrumentDataSize);
	in = in + READ_BE_UINT32(in + (prg << 2));
	assert(in < _instrumentData + _instrumentDataSize);

	writeReg(0xB0, *in);
	_algorithm = (*in++) & 7;
	uint8 cr = _carrier[_algorithm] >> 4;

	static const uint8 regSteps[] = { 0x00, 0x08, 0x04, 0x0C };

	for (int i = 0x00; i < 4; i += 1) {
		writeReg(0x30 + regSteps[i], *in++);
		writeReg(0x40 + regSteps[i], (cr & 1) ? 0xFF : *in);
		_tl[i] = *in++;
		cr >>= 1;
		for (uint8 reg = 0x50; reg < 0x90; reg += 0x10)
			writeReg(reg + regSteps[i], *in++);
	}

	if (!(_state.flags & SndDriverState::kSndKeepPanPos)) {
		_panPosition = 0xC0;
		_panEffectRate = 0;
	}

	setParameter(3, _panPosition);
}

void KonamiMCDSndChannel::makeFunctions() {
#ifdef SNDDRV_DEBUG
	#define OP(x) { &KonamiMCDSndChannel::op_##x, #x }
	#define FnObj ChanOpcode(new ChanOpcodeFunc(this, opcTable[i].func), opcTable[i].name, _num, dbytes[i])
	typedef void(KonamiMCDSndChannel::*OpFunc)(const uint8*&);
	static const uint8 dbytes[] = {
		1, 2, 1, 1, 3, 3, 0, 3, 0, 3, 1, 2, 1, 1, 1, 2,
		1, 3, 1, 0, 2, 2, 0, 0, 0, 1, 0, 1, 2, 4, 4, 0
	};
	struct TableEntry {
		OpFunc func;
		const char *name;
	};
#else
	#define OP(x) &KonamiMCDSndChannel::op_##x
	#define FnObj ChanOpcode(this, opcTable[i])
	typedef void(KonamiMCDSndChannel::*TableEntry)(const uint8*&);
#endif

	static const TableEntry opcTable[] = {
		OP(break_toneOff),
		OP(break_keepTone),
		OP(programChange),
		OP(panPos),
		OP(vibrato),
		OP(pitchBendSpecial),
		OP(setRepeatSectionMarker1),
		OP(repeatSection1),
		OP(setRepeatSectionMarker2),
		OP(repeatSection2),
		OP(setSpeed),
		OP(panEffect),
		OP(transposeCr),
		OP(toggleLFO),
		OP(setVolume),
		OP(volumeSlide),
		OP(portamento),
		OP(pitchWheel),
		OP(transposeFn),
		OP(nop),
		OP(lfoSensivity),
		OP(ampModSensitivity),
		OP(setThemeMarker),
		OP(repeatTheme),
		OP(nop),
		OP(vibratoDepthModulation),
		OP(nop),
		OP(envRate),
		OP(changePitchBendMod),
		OP(jump),
		OP(jumpToSubroutine),
		OP(return)
	};

	for (int i = 0; i < ARRAYSIZE(opcTable); ++i)
		_opcodes.push_back(Common::SharedPtr<ChanOpcode>(new FnObj));

#undef OP
#undef FnObj
}

void KonamiMCDSndChannel::op_nop(const uint8*&) {
	// No operation
}

void KonamiMCDSndChannel::op_break_toneOff(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_eventDelayTimer = _eventDelay = *data++;
	_noteDuration = _lastNoteDuration = _noteTimer = _velocity = 0;
	keyOff();
	_state.flags |= SndDriverState::kSndParseBreak;
}

void KonamiMCDSndChannel::op_break_keepTone(const uint8 *&data) {
	checkPtr(data, 2, __FUNCTION__);
	_eventDelayTimer = _eventDelay = *data++;
	_noteDuration = _lastNoteDuration = *data++;
	_noteTimer = MAX<uint8>(1, (_noteDuration * 2 * _eventDelay) >> 8);
	_state.flags |= SndDriverState::kSndParseBreak;
}

void KonamiMCDSndChannel::op_programChange(const uint8 *&data) {
	programChange(data);
}

void KonamiMCDSndChannel::op_panPos(const uint8 *&data) {
	static const uint8 flags[] = { 0x80, 0xC0, 0x40, 0x00 };
	assert(*data < ARRAYSIZE(flags));
	checkPtr(data, 1, __FUNCTION__);
	_panPosition = flags[*data++];
	_panEffectRate = 0;
	setParameter(3, _panPosition);
}

void KonamiMCDSndChannel::op_vibrato(const uint8 *&data) {
	checkPtr(data, 3, __FUNCTION__);
	_vbrDelay = *data++;
	_vbrRate = *data++;
	_vbrDepth = *data++;
	_vbrDepthCur = _vbrDepth << 8;
	_vbrDepthModulationRate = 0;
}

void KonamiMCDSndChannel::op_pitchBendSpecial(const uint8 *&data) {
	checkPtr(data, 3, __FUNCTION__);
	_pbExtSpeed = *data++;
	_pbExtMask = READ_BE_UINT16(data);
	_pbExtPos = 0;
	_pbExtTimer = 0;
}

void KonamiMCDSndChannel::op_setRepeatSectionMarker1(const uint8 *&data) {
	_sectionMarker1 = data;
}

void KonamiMCDSndChannel::op_repeatSection1(const uint8 *&data) {
	checkPtr(data, 3, __FUNCTION__);
	uint8 val = *data++;
	if (val == 0 || val != ++_section1RepeatCounter) {
		int8 in = *data++;
		if (in) {
			_section1VolAdjust += in;
			_state.volUpdate |= (1 << _id);
		}
		in = *data++;
		_section1Transpose += (in << 3);
		data = _sectionMarker1;

	} else {
		_section1RepeatCounter = 0;
		_section1VolAdjust = 0;
		_section1Transpose = 0;
		data += 2;
	}
}

void KonamiMCDSndChannel::op_setRepeatSectionMarker2(const uint8 *&data) {
	_sectionMarker2 = data;
	_section2RepeatCounter = 0;
	_section2VolAdjust = 0;
	_section2Transpose = 0;
}

void KonamiMCDSndChannel::op_repeatSection2(const uint8 *&data) {
	checkPtr(data, 3, __FUNCTION__);
	uint8 val = *data++;
	if (val == 0 || val != ++_section2RepeatCounter) {
		int8 in = *data++;
		if (in) {
			_section2VolAdjust += in;
			_state.volUpdate |= (1 << _id);
		}
		in = *data++;
		_section2Transpose += (in << 3);
		data = _sectionMarker2;
	} else {
		data += 2;
	}
}

void KonamiMCDSndChannel::op_setSpeed(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	if (/*_palMode*/0)
		_speed = MIN<uint>(((*data++ * 6) / 5) + 2, 0xFF);
	else
		_speed = *data++;
}

void KonamiMCDSndChannel::op_panEffect(const uint8 *&data) {
	checkPtr(data, 2, __FUNCTION__);
	_panEffectRate = _panEffectRateCounter = *data++;
	_panEffectType = *data++;
	setParameter(3, _panEffectType == 3 ? 0x40 : 0x80);
	_panEffectState = 0;
}

void KonamiMCDSndChannel::op_transposeCr(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_transposeMSB = (int8)*data++;
}

void KonamiMCDSndChannel::op_toggleLFO(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	uint8 val = *data++;
	if (blockedBySfx())
		return;
	_sai->writeReg(0, 0x22, 0x08 | val);
}

void KonamiMCDSndChannel::op_setVolume(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_volume = (*data++) << 8;
	_volSlideDuration = 0;
}

void KonamiMCDSndChannel::op_volumeSlide(const uint8 *&data) {
	checkPtr(data, 2, __FUNCTION__);
	_volSlideDuration = MAX<uint>(1, *data++);
	_volSlideDest = *data++;
	int16 diff = (_volSlideDest << 8) - _volume;
	_volSlideRate = diff / _volSlideDuration;
}

void KonamiMCDSndChannel::op_portamento(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_pitchBendDuration = *data++;
	_pitchBendDelay = 0;
	_flags |= KChPortamento;
}

void KonamiMCDSndChannel::op_pitchWheel(const uint8 *&data) {
	checkPtr(data, 3, __FUNCTION__);
	_pitchBendDelay = *data++;
	_pitchBendDuration = *data++;
	_pitchBendDist = (int8)(*data++) << 8;
	_flags &= ~KChPortamento;
}

void KonamiMCDSndChannel::op_transposeFn(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_transposeLSB = (int8)(*data++) * 4;
}

void KonamiMCDSndChannel::op_lfoSensivity(const uint8 *&data) {
	checkPtr(data, 2, __FUNCTION__);
	_lfoSensitivityDelay = *data++;
	_lfoSensitivity = (*data++) << 4;
	if (_lfoSensitivityDelay == 0)
		setParameter(1, _lfoSensitivity);
}

void KonamiMCDSndChannel::op_ampModSensitivity(const uint8 *&data) {
	checkPtr(data, 2, __FUNCTION__);
	_ampModSensitivityDelay = *data++;
	_ampModSensitivity = (*data++) << 4;
	if (_ampModSensitivityDelay == 0)
		setParameter(2, _ampModSensitivity);
}

void KonamiMCDSndChannel::op_setThemeMarker(const uint8 *&data) {
	_themeStart = data;
	_flags &= ~(kChFinishedTheme | kChRepeatingTheme);
}

void KonamiMCDSndChannel::op_repeatTheme(const uint8 *&data) {
	if (_flags & kChFinishedTheme) {
		_flags = (_flags & ~kChFinishedTheme) | kChRepeatingTheme;
		_resumeAfterThemePos = data;
		data = _themeStart;
	} else if (_flags & kChRepeatingTheme) {
		_flags = (_flags & ~kChRepeatingTheme) | kChFinishedTheme;
		data = _resumeAfterThemePos;
	} else {
		_flags |= kChFinishedTheme;
	}
}

void KonamiMCDSndChannel::op_vibratoDepthModulation(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_vbrDepthModulationRate = *data++;
	if (_vbrDepthModulationRate)
		_vbrDepthRate = (int16)((_vbrDepth << 8) / _vbrDepthModulationRate);
}

void KonamiMCDSndChannel::op_envRate(const uint8 *&data) {
	checkPtr(data, 1, __FUNCTION__);
	_decayRate = (*data++) << 5;
}

void KonamiMCDSndChannel::op_changePitchBendMod(const uint8 *&data) {
	checkPtr(data, 2, __FUNCTION__);
	_pitchBendRate = READ_LE_INT16(data); // really LE
	data += 2;
}

void KonamiMCDSndChannel::op_jump(const uint8 *&data) {
	jump(data);
}

void KonamiMCDSndChannel::op_jumpToSubroutine(const uint8 *&data) {
	_returnPos = data + 4;
	_flags |= KChSubroutine;
	jump(data);
}

void KonamiMCDSndChannel::op_return(const uint8 *&data) {
	if (_flags & KChSubroutine) {
		_flags &= ~KChSubroutine;
		data = _returnPos;
	} else {
		_sndId = 0;
		if (_id < 6) {
			_flags |= kChBlockVolume;
			chanOff();
			keyOff();
			finish();
		}

		_state.flags |= SndDriverState::kSndParseBreak;
		if (!(_state.flags & SndDriverState::kSndSfxMode)) {
			_state.chanUse &= ~(1 << _id);
			if (_state.chanUse == 0)
				_state.progress |= 0x80;
		}

		_state.result = _state.progress & 0xC0;
		if (_state.result)
			_state.progress = ((_state.progress & 0x0F) + 1) % 16;
		_state.result |= _state.progress;
	}
}

const uint8 KonamiMCDSndChannel::_carrier[8] = {
	0x80, 0x80, 0x80, 0x80, 0xA0, 0xE0, 0xE0, 0xF0
};

KonamiMCDAudioDriver::KonamiMCDAudioDriver(Audio::Mixer *mixer, MessageHandler *msg) : SegaAudioPluginDriver(), _msg(msg), _sai(nullptr), _channels(nullptr), _state(),
	_instrumentData(nullptr), _trackData(nullptr), _curTrackPrio(0), _comListWritePos(0), _comListWritePosPrev(0), _comListReadPos(0), _curSfxPrio(0),
		_instrumentDataSize(7680), _trackDataSize(2630), _trackDataSize2(0), _trackDataEnd(nullptr) {

	_sai = new SegaAudioInterface(mixer, this);
	assert(_sai);

	_channels = new KonamiMCDSndChannel*[10]();
	assert(_channels);
	for (int i = 0; i < 6; ++i)
		_channels[i] = new KonamiMCDSndChannelDef(i, _sai, _state);
	for (int i = 6; i < 8; ++i)
		_channels[i] = new KonamiMCDSndChannelRtm(i, _sai, _state);
	for (int i = 8; i < 10; ++i)
		_channels[i] = new KonamiMCDSndChannelDef(i, _sai, _state);
	memset(_comList, 0, sizeof(_comList));
}

KonamiMCDAudioDriver::~KonamiMCDAudioDriver() {
	delete _sai;
	if (_channels) {
		for (int i = 0; i < 10; ++i)
			delete _channels[i];
		delete[] _channels;
	}
	delete[] _instrumentData;
	delete[] _trackData;
}

bool KonamiMCDAudioDriver::init() {
	if (!_sai->init())
		return false;
	_sai->writeReg(0, 0x26, 0xE9);
	_sai->writeReg(0, 0x27, 0x3F);

	return true;
}

bool KonamiMCDAudioDriver::loadData(const uint8 *data, uint32 dataSize) {
	if (!data)
		return false;

	if (dataSize < _instrumentDataSize)
		return false;

	delete[] _instrumentData;
	_instrumentData = new uint8[_instrumentDataSize]();
	assert(_instrumentData);
	memcpy(_instrumentData, data, _instrumentDataSize);
	dataSize -= _instrumentDataSize;
	data += _instrumentDataSize;

	if (dataSize < _trackDataSize)
		return false;

	delete[] _trackData;
	_trackData = new uint8[dataSize]();
	assert(_trackData);
	memcpy(_trackData, data, dataSize);

	_trackDataSize2 = dataSize - _trackDataSize;
	_trackDataEnd = _trackData + dataSize;

	for (int i = 0; i < 10; ++i)
		_channels[i]->loadInstrumentData(_instrumentData, _instrumentDataSize);

	return true;
}

void KonamiMCDAudioDriver::doCommand(int cmd, int arg) {
	SegaAudioInterface::MutexLock lock(_sai->stackLockMutex());

	if (cmd >= 241) {
		switch (cmd) {
		case 241:
			reset();
			break;
		case 242:
			stopChannels(0);
			break;
		case 243:
			disableStereoMode(true);
			break;
		case 244:
			disableStereoMode(false);
			break;
		default:
			break;
		}
	} else if (cmd < 123) {
		if (cmd > 49 && cmd < 63) {
			const StaticRes_SCD::FMSoundTrackDef &d = StaticRes_SCD::_fmSndTrackDefs[cmd - 1];
			if (_curTrackPrio <= d.prio) {
				_curTrackPrio = d.prio;
				_comListWritePosPrev = _comListWritePos;
			}
		}

		_comList[_comListWritePos++] = cmd;
		if (_comListWritePos == sizeof(_comList))
			_comListWritePos = 0;
	}
}

void KonamiMCDAudioDriver::setVolume(uint8 vol1, uint8 vol2) {
	SegaAudioInterface::MutexLock lock(_sai->stackLockMutex());
	_state.volume1 = vol1;
	_state.volume2 = vol2;
}

uint8 KonamiMCDAudioDriver::getStatus() const {
	SegaAudioInterface::MutexLock lock(_sai->stackLockMutex());
	return _state.result;
}

void KonamiMCDAudioDriver::timerCallbackA() {
	handleComList();
	nextTick();
}

void KonamiMCDAudioDriver::reset() {
	if (_msg != nullptr)
		_msg->clear();

	_state.clear();
	_comListReadPos = _comListWritePos = _comListWritePosPrev = 0;
	_curTrackPrio = _curSfxPrio = 0;

	for (int i = 0; i < 10; ++i)
		_channels[i]->clear();

	for (int i = 0; i < 6; ++i)
		_channels[i]->off();
}

void KonamiMCDAudioDriver::stopChannels(int scope) {
	for (int i = 0; i < 6; ++i) {
		if (_channels[i]->busy())
			_channels[i]->setVolume(0);
	}
	scope = scope ? 8 : 10;
	for (int i = 0; i < scope; ++i)
		_channels[i]->stop();
}

void KonamiMCDAudioDriver::disableStereoMode(bool disable) {
	if (disable)
		_state.flags |= SndDriverState::kSndDisableStereo;
	else
		_state.flags &= ~SndDriverState::kSndDisableStereo;
}

void KonamiMCDAudioDriver::handleComList() {
	if (_comListWritePos == _comListReadPos)
		return;
	if (_comListWritePosPrev == _comListReadPos)
		_curTrackPrio = 0;

	uint8 cmd = _comList[_comListReadPos++];
		if (_comListReadPos == sizeof(_comList))
			_comListReadPos = 0;

	switch (cmd) {
	case 0:
		break;
	case 241:
		reset();
		break;
	case 242:
		stopChannels(0);
		break;
	case 243:
		disableStereoMode(true);
		break;
	case 244:
		disableStereoMode(false);
		break;
	default:
		startSound(cmd);
		break;
	}
}

void KonamiMCDAudioDriver::nextTick() {
	if (_state.volume1 < 21 || _state.volume2 < 21) {
		if (!(_state.flags & SndDriverState::kSndFadeoutDone)) {
			_state.flags |= SndDriverState::kSndFadeoutDone;
			stopChannels(1);
		}
	} else {
		_state.flags &= ~SndDriverState::kSndFadeoutDone;
		if (!(_state.flags & SndDriverState::kSndBlockChannels)) {
			for (int i = 0; i < 6; ++i)
				_channels[i]->update();
		}

		updateRtmChannels();

		if (_state.flags & SndDriverState::kSndBlockChannels) {
			_state.flags &= ~SndDriverState::kSndBlockChannels;
			return;
		}
	}

	updateSfxChannels();
}

void KonamiMCDAudioDriver::startSound(uint8 sndId) {
	if ((sndId - 1) >= ARRAYSIZE(StaticRes_SCD::_fmSndTrackDefs))
		error("%s(): Sound ID %d out of range", __FUNCTION__, sndId);

	const StaticRes_SCD::FMSoundTrackDef &d = StaticRes_SCD::_fmSndTrackDefs[sndId - 1];

	_state.flags &= ~SndDriverState::kSndFadeoutDone;

	if (d.a & 0x0F) {
		_state.chanUse = 0;
		_state.flags &= ~SndDriverState::kSndSfxStarting;
		stopChannels(0);
	} else {
		_state.flags |= SndDriverState::kSndSfxStarting;
	}

	if ((_state.flags & SndDriverState::kSndSfxStarting) || (d.poly < 6))
		_state.flags &= ~SndDriverState::kSndBlockChannels;
	else
		_state.flags |= SndDriverState::kSndBlockChannels;

	KonamiMCDSndChannel **c = &_channels[d.chan];
	uint8 prio = (d.chan & 1) << 7;

	if (sndId < 50) {
		if (_state.sfxId[1] != sndId) {
			if (_state.sfxId[0] != sndId) {
				if ((*c)->getPrio() < d.prio) {
					_state.sfxId[1] = sndId;
				} else {
					--c;
					if ((*c)->getPrio() > d.prio)
						return;
					_state.sfxId[0] = sndId;
				}
			} else {
				--c;
			}
		}
	}

	assert(c >= _channels);

	if (_state.flags & SndDriverState::kSndSfxStarting) {
		_state.flags &= ~(SndDriverState::kSndKeepPanPos | SndDriverState::kSndSfxMode | SndDriverState::kSndSfxIsPlaying);
		_curSfxPrio = prio;
	} else {
		_state.flags &= ~(SndDriverState::kSndSfxIsPlaying - 1);
	}

	const uint8 *ptr = getTrack(d.track);
	const uint32 *offs = reinterpret_cast<const uint32*>(ptr);

	for (int i = 0; i <= d.poly; ++i) {
		if (_state.flags & SndDriverState::kSndSfxStarting) {
			if ((*c)->getNumber() == 9)
				_state.flags |= SndDriverState::kSndLockSfxChan2;
			else if ((*c)->getNumber() == 8)
				_state.flags |= SndDriverState::kSndLockSfxChan1;
			_sai->setSoundEffectChanMask(0x1E00 | ((_state.flags & (SndDriverState::kSndLockSfxChan1 | SndDriverState::kSndLockSfxChan2)) << 4));
		}

		(*c)->initSound(ptr + READ_BE_UINT32(offs++), _trackDataEnd, sndId, (_state.flags & SndDriverState::kSndSfxStarting) ? _curSfxPrio : -1);
#ifdef SNDDRV_DEBUG
		debug("Starting sound %d on channel %d", sndId, (*c)->getNumber());
#endif
		if (!(_state.flags & SndDriverState::kSndSfxStarting))
			_state.chanUse |= (1 << (*c)->getId());
		++c;
	}
}

void KonamiMCDAudioDriver::updateRtmChannels() {
	_state.msg[0] = _state.msg[1] = _state.msg[2] = 0;

	for (int i = 6; i < 8; ++i) {
		KonamiMCDSndChannel *c = _channels[i];
		if (!c->busy())
			continue;

		if (c->checkTicker())
			c->update();
	}

	if (_msg != nullptr && (_state.msg[0] || _state.msg[1] || _state.msg[2]))
		_msg->write(_state.msg[0], _state.msg[1], _state.msg[2]);
}

void KonamiMCDAudioDriver::updateSfxChannels() {
	if (!(_state.flags & SndDriverState::kSndBlockChannels)) {
		_state.flags &= ~(SndDriverState::kSndKeepPanPos | SndDriverState::kSndSfxMode | SndDriverState::kSndSfxIsPlaying);
		if (_state.flags & (SndDriverState::kSndLockSfxChan1 | SndDriverState::kSndLockSfxChan2)) {
			_state.flags |= (SndDriverState::kSndSfxIsPlaying | SndDriverState::kSndSfxMode);
			for (int i = 8; i < 10; ++i)
				_channels[i]->update();
		}
	}
	_state.flags &= ~SndDriverState::kSndSfxMode;
}

const uint8 *KonamiMCDAudioDriver::getTrack(uint32 index) const {
	uint32 offset = (_state.flags & SndDriverState::kSndSfxStarting) ? 0: _trackDataSize;
	uint32 totalSize = (_state.flags & SndDriverState::kSndSfxStarting) ? _trackDataSize : _trackDataSize2;
	assert((index << 2) < totalSize);
	const uint8 *res = _trackData + offset;
	res = res + READ_BE_UINT32(&res[index << 2]);
	assert(res < _trackData + _trackDataSize + _trackDataSize2);
	return res;
}

SegaSoundDevice::SegaSoundDevice(FIO *fio) : _fio(fio), SoundDevice(), _sai(nullptr), _lastTrack(-1), _pauseStartTime(0), _pcmResourceInfo(nullptr), _pcmResidentDataInfo(nullptr), _pcmInstrumentInfo(nullptr),
	_pcmSounds(nullptr), _pcmDisableStereo(false), _temp(nullptr), _pcmResourceNumber(-1), _pcmState(0), _pcmMode(0), _faderFlags(0), _faderTimer1(0), _faderTimer2(0), _reduceVolume2(false),
		_fmVolume1(0xFF), _fmLastVolume1(0), _fmVolume2(0xFF), _fmLastVolume2(0), _fmCmdQueue(nullptr), _fmCmdQueueWPos(0), _fmCmdQueueRPos(0), _fmDriver(nullptr), _driverMsg()	{
	_temp = new uint8[0x8000]();
	assert(_temp);
	_pcmSounds = new PCMSound[4]();
	assert(_pcmSounds);
	_fmCmdQueue = new uint8[128]();
	assert(_fmCmdQueue);
}

SegaSoundDevice::~SegaSoundDevice() {
	delete _sai;
	delete _fmDriver;
	delete[] _pcmResourceInfo;
	delete[] _pcmResidentDataInfo;
	delete[] _pcmInstrumentInfo;
	delete[] _temp;
	delete[] _pcmSounds;
	delete[] _fmCmdQueue;
}

bool SegaSoundDevice::init(Audio::Mixer *mixer) {
	_fmDriver = new KonamiMCDAudioDriver(mixer, &_driverMsg);
	if (!_fmDriver)
		return false;

	_sai = new SegaAudioInterface(mixer, nullptr);
	if (!_sai || !_sai->init() || !_fmDriver->init())
		return false;
	// Setup first 4 pcm channels as sound effect channels.
	_sai->setSoundEffectChanMask(0x1E00);

	if (!fmLoadData() || !pcmLoadData()) {
		delete _sai;
		delete _fmDriver;
		return false;
	}

	return true;
}

void SegaSoundDevice::cdaPlay(int track) {
	assert(track > 0);
	g_system->getAudioCDManager()->play(track - 1, 1, 0, 0);
	_lastTrack = track;
	_musicStartTime = g_system->getMillis();
}

void SegaSoundDevice::cdaStop() {
	g_system->getAudioCDManager()->stop();
	_lastTrack = -1;
}

bool SegaSoundDevice::cdaIsPlaying() const {
	return g_system->getAudioCDManager()->isPlaying();
}

uint32 SegaSoundDevice::cdaGetTime() const {
	uint32 relTime = cdaIsPlaying() ? g_system->getMillis() - _musicStartTime : 0;
	return Util::makeBCDTimeStamp(relTime);
}

void SegaSoundDevice::fmSendCommand(int cmd, int restoreVolume) {
	if (cmd == 0xFF) {
		_faderFlags |= 4;
		return;
	}
	if (cmd < 241 && restoreVolume) {
		_faderFlags &= ~4;
		_fmVolume1 = 0xFF;
	}

	_fmCmdQueue[_fmCmdQueueWPos++] = cmd;
	if (_fmCmdQueueWPos == 128)
		_fmCmdQueueWPos = 0;
}

uint8 SegaSoundDevice::fmGetStatus() const {
	return _fmDriver->getStatus();
}

void SegaSoundDevice::pcmSendCommand(int cmd, int arg) {
	if (cmd >= 248) {
		switch (cmd) {
		case 252:
			pcmDisableStereoMode(true);
			break;
		case 253:
			pcmDisableStereoMode(false);
			break;
		case 255:
			_pcmState &= ~8;
			pcmStop();
			break;
		default:
			break;
		}
	} else {
		_pcmState |= 8;
		pcmStartSound(cmd, arg);
	}
}

void SegaSoundDevice::pcmInitSound(int sndId) {
	_pcmResourceNumber = sndId;
	if (sndId < 0)
		return;

	_pcmMode = _pcmResourceInfo[_pcmResourceNumber].mode;
	if (_pcmMode == 1) {
		_pcmState |= 4;
	} else if (_pcmMode == 2 || _pcmMode == 0x82) {
		if (_pcmMode == 2) {
			_faderTimer2 = 0;
			_faderFlags = (_faderFlags & ~2) | 1;
		}
		_pcmState = (_pcmState & ~8) | 1;
	}
}

uint8 SegaSoundDevice::pcmGetStatus() const {
	return _pcmState;
}

int16 SegaSoundDevice::pcmGetResourceId() const {
	return _pcmResourceNumber;
}

void SegaSoundDevice::pause(bool toggle) {
	if (toggle) {
		if (_pauseStartTime == 0) {
			if (cdaIsPlaying()) {
				_pauseStartTime = g_system->getMillis();
				g_system->getAudioCDManager()->stop();
			} else {
				_lastTrack = -1;
				_pauseStartTime = 0;
			}
		}
	} else {
		if (_pauseStartTime != 0) {
			if (_lastTrack != -1)
				g_system->getAudioCDManager()->play(_lastTrack - 1, 1, (_pauseStartTime - _musicStartTime) / (1000 / 75), 0);
			int pauseDuration = g_system->getMillis() - _pauseStartTime;
			_musicStartTime += (pauseDuration - (pauseDuration % (1000 / 75)));
			_pauseStartTime = 0;
		}
	}
	g_system->getMixer()->pauseAll(toggle);
}

void SegaSoundDevice::update() {
	pcmCheckPlayStatus();
	pcmHandleDriverMessage();
	fmUpdateVolumeFader();
	pcmDelayedStart();
	fmHandleCmdQueue();
}

void SegaSoundDevice::reduceVolume2(bool enable) {
	_reduceVolume2 = enable;
}

void SegaSoundDevice::setMusicVolume(int vol) {
	_sai->setMusicVolume(vol);
}

void SegaSoundDevice::setSoundEffectVolume(int vol) {
	_sai->setSoundEffectVolume(vol);
}

bool SegaSoundDevice::fmLoadData() {
	uint32 dataSize = 0;
	const uint8 *data = _fio->fileData(53, &dataSize);
	if (!data || !dataSize)
		return false;

	bool res = _fmDriver->loadData(data, dataSize);

	delete[] data;

	return res;
}

static const uint8 volTable[] = {
	0x00, 0x00, 0x00, 0x00, 0x1e, 0x23, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3b, 0x3e, 0x42, 0x45, 0x47,
	0x4a, 0x4d, 0x50, 0x52, 0x55, 0x57, 0x59, 0x5c, 0x5e, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c,
	0x6e, 0x70, 0x72, 0x73, 0x75, 0x77, 0x79, 0x7a, 0x7c, 0x7e, 0x7f, 0x81, 0x82, 0x84, 0x85, 0x87,
	0x88, 0x8a, 0x8b, 0x8d, 0x8e, 0x8f, 0x91, 0x92, 0x93, 0x95, 0x96, 0x97, 0x98, 0x9a, 0x9b, 0x9c,
	0x9d, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae,
	0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
	0xbe, 0xbf, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xcb, 0xcc,
	0xcd, 0xce, 0xce, 0xcf, 0xce, 0xcf, 0xcf, 0xcf, 0xd0, 0xd1, 0xd1, 0xd2, 0xd3, 0xd3, 0xd4, 0xd5,
	0xd5, 0xd6, 0xd7, 0xd7, 0xd8, 0xd8, 0xd9, 0xda, 0xda, 0xdb, 0xdb, 0xdc, 0xdc, 0xdd, 0xde, 0xde,
	0xdf, 0xdf, 0xe0, 0xe0, 0xe1, 0xe1, 0xe2, 0xe2, 0xe3, 0xe3, 0xe4, 0xe4, 0xe5, 0xe5, 0xe6, 0xe6,
	0xe7, 0xe7, 0xe8, 0xe8, 0xe9, 0xe9, 0xe9, 0xea, 0xea, 0xeb, 0xeb, 0xec, 0xec, 0xec, 0xed, 0xed,
	0xee, 0xee, 0xee, 0xef, 0xef, 0xef, 0xf0, 0xf0, 0xf0, 0xf1, 0xf1, 0xf2, 0xf2, 0xf2, 0xf3, 0xf3,
	0xf3, 0xf3, 0xf4, 0xf4, 0xf4, 0xf5, 0xf5, 0xf5, 0xf6, 0xf6, 0xf6, 0xf6, 0xf7, 0xf7, 0xf7, 0xf7,
	0xf8, 0xf8, 0xf8, 0xf8, 0xf9, 0xf9, 0xf9, 0xf9, 0xfa, 0xfa, 0xfa, 0xfa, 0xfa, 0xfb, 0xfb, 0xfb,
	0xfb, 0xfb, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfd, 0xfe, 0xfe,
	0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void SegaSoundDevice::fmUpdateVolumeFader() {
	_fmVolume2 = _reduceVolume2 ? 112 : 255;
	if (_faderFlags & 4) {
		_faderFlags &= ~3;
		if (++_faderTimer1 != 1)
			return;
		_faderTimer1 = 0;
		if (_fmVolume1 >= 3)
			_fmVolume1 -= 3;

	} else if (_faderFlags & 1) {
		if (++_faderTimer2 != 2)
			return;
		_faderTimer2 = 0;
		if (_fmVolume1 >= 154) {
			_fmVolume1 -= 2;
		} else {
			_faderFlags &= ~1;
			_faderTimer2 = 0;
		}

	} else if (_faderFlags & 2) {
		if (++_faderTimer2 != 3)
			return;
		_faderTimer2 = 0;
		if (_fmVolume1 < 255) {
			_fmVolume1 += 2;
		} else {
			_faderFlags &= ~2;
			_faderTimer2 = 0;
		}
	}
}

void SegaSoundDevice::fmHandleCmdQueue() {
	if (_fmCmdQueueRPos != _fmCmdQueueWPos) {
		_fmDriver->doCommand(_fmCmdQueue[_fmCmdQueueRPos++], 0);
		if (_fmCmdQueueRPos == 128)
			_fmCmdQueueRPos = 0;
	}
	if ((_fmVolume1 != _fmLastVolume1) || (_fmVolume2 != _fmLastVolume2)) {
		_fmLastVolume1 = _fmVolume1;
		_fmLastVolume2 = _fmVolume2;
		_fmDriver->setVolume(volTable[_fmVolume1], volTable[_fmVolume2]);
	}
}

bool SegaSoundDevice::pcmLoadData() {
	uint32 dataSize = 0;
	const uint8 *data = _fio->fileData(56, &dataSize);
	if (!data || dataSize < 0x44A0)
		return false;

	_pcmResourceInfo = new PCMResourceInfo[0x800]();
	assert(_pcmResourceInfo);
	PCMResourceInfo *p1 = _pcmResourceInfo;
	_pcmResidentDataInfo = new PCMResidentDataInfo[248]();
	assert(_pcmResidentDataInfo);
	PCMResidentDataInfo *p2 = _pcmResidentDataInfo;

	const uint8 *in = data;
	for (int i = 0; i < 0x800; ++i) {
		p1->prio = *in++;
		p1->mode = *in++;
		p1->vol = *in++;
		p1->pan = *in++;
		p1->sector = READ_BE_UINT16(in);
		in += 2;
		p1->numSectors = READ_BE_UINT16(in);
		in += 2;
		p1++;
	}

	for (int i = 0; i < 0x4A; ++i) {
		p2->prio = *in++;
		p2->unit = *in++;
		p2->vol = *in++;
		p2->pan = *in++;
		p2->rate = READ_BE_UINT16(in);
		in += 2;
		p2->addr = READ_BE_UINT32(in);
		in += 10;
		p2++;
	}

	delete[] data;

	// Extra setup for the pcm data that is used by sound driver instruments.
	_pcmInstrumentInfo = new PCMInstrumentInfo[13]();
	assert(_pcmInstrumentInfo);
	uint16 dstAddr = 0x8000;

	for (int i = 1; i < 13; ++i) {
		if (i == 6 || i == 8 || i == 9 || i == 10)
			continue;

		SmpStrPtr str = createSamplesStream(_pcmResidentDataInfo[i].addr + 0x10);
		int64 sz = str->size() - 1;
		uint8 *smp = new uint8[sz + 2]();
		str->read(smp, sz);
		smp[sz] = 0x80;
		smp[sz + 1] = 0xFF;

		_pcmInstrumentInfo[i].addr = dstAddr;
		_pcmInstrumentInfo[i].loopst = dstAddr + sz;

		// Load and permanently keep pcm data for the sound driver instruments in pcm memory.
		_sai->loadPCMData(dstAddr, smp, sz + 2);
		dstAddr += (sz + 2);

		if (dstAddr & 0xFF)
			dstAddr = (dstAddr + 0xFF) & ~0xFF;

		delete[] smp;
	}

	return true;
}

void SegaSoundDevice::pcmDisableStereoMode(bool disable) {
	_pcmDisableStereo = disable ? true : false;
}

void SegaSoundDevice::pcmStop() {
	for (int i = 0; i < 8; ++i)
		_sai->stopPCMChannel(i);
}

void SegaSoundDevice::pcmStartSound(uint8 sndId, int8 unit, uint8 vol) {
	if (unit < -1 || unit > 1)
		return;

	PCMResidentDataInfo &p = _pcmResidentDataInfo[sndId];
	if (unit == -1)
		unit = p.unit;

	if (unit > 3)
		error("%s(): PCM unit %d out of bounds", __FUNCTION__, unit);

	PCMSound &s = _pcmSounds[unit];
	if (s.prio > p.prio)
		return;
	pcmResetUnit(unit, s);
	s.unit = p.unit;
	s.volBase = p.vol;
	s.volEff = vol ? (p.vol * vol) >> 8 : p.vol;
	s.pan = p.pan;
	s.rate = p.rate;
	s.addr = p.addr + 0x10;

	pcmStartSoundInternal(unit, unit << 1, s);
	pcmStartSoundInternal(unit, (unit << 1) | 1, s);

	s.prio = p.prio;
}

void SegaSoundDevice::pcmCheckPlayStatus() {
	int nu = (_pcmMode == 2 || _pcmMode == 0x82) ? 1 : 2;
	int nc = (_pcmMode == 2 || _pcmMode == 0x82) ? 4 : 2;
	uint8 flg = (_pcmMode == 2 || _pcmMode == 0x82) ? 2 : 8;

	if (_pcmState & 1)
		return;

	for (int i = 0; i < nu; ++i) {
		PCMSound &s = _pcmSounds[i];
		if (s.prio == 0)
			continue;
		s.flags &= ~2;

		bool finished = true;
		for (int ii = 0; ii < nc; ++ii) {
			if (_sai->isPCMChannelPlaying((i << 1) + ii))
				finished = false;
		}
		if (finished) {
			pcmResetUnit(i, s);
			if (_pcmMode == 2) {
				_faderTimer2 = 0;
				_faderFlags = (_faderFlags & ~1) | 2;
			}
			_pcmState &= ~flg;
			_pcmMode = 0;
		}
	}
}

void SegaSoundDevice::pcmHandleDriverMessage() {
	if (!_driverMsg.hasNewMsg())
		return;

	uint8 a = 0;
	uint8 b = 0;
	uint8 c = 0;

	_driverMsg.read(a, b, c);

	if ((a & 0x1F) && a < 13)
		pcmStartInstrument(2, a, ((b & 7) | ((c & 0x60) >> 2)) << 3);
	if ((b & 0xF8) && (b >> 3) < 13)
		pcmStartInstrument(3, b >> 3, (c & 0x1F) << 3);
}

void SegaSoundDevice::pcmDelayedStart() {
	if (_pcmState & 1) {
		PCMSound &s = _pcmSounds[0];
		assert(_pcmResourceNumber >= 0);
		PCMResourceInfo &p = _pcmResourceInfo[_pcmResourceNumber];
		s.addr = 0x65000;
		s.prio = p.prio;
		s.volEff = p.vol;
		s.pan = p.pan;
		s.rate = 0x400;

		pcmStartSoundInternal(0, 0, s);
		pcmStartSoundInternal(0, 1, s);
		pcmStartSoundInternal(0, 2, s);
		pcmStartSoundInternal(0, 3, s);

		_pcmState = (_pcmState & ~1) | 2;
		_pcmResourceNumber = -1;
	}
	_pcmState &= ~4;
}

SegaSoundDevice::SmpStrPtr SegaSoundDevice::createSamplesStream(uint32 addr) {
	uint32 numSamples = 0;

	for (Common::Array<PCMCacheEntry>::const_iterator i = _pcmCache.begin(); i != _pcmCache.end(); ++i) {
		if (i->addr == addr && i->resNo == _pcmResourceNumber)
			return SmpStrPtr(new Common::MemoryReadStream(i->data.get(), i->dataSize, DisposeAfterUse::NO));
	}

	Common::SeekableReadStream *str = _fio->readStream(addr < 0x65000 ? 54 : 55);
	if (!str)
		error("%s(): Error reading file", __FUNCTION__);

	assert(addr < 0x65000 || _pcmResourceNumber >= 0);
	uint32 start = addr < 0x65000 ? addr - 0x55000 : _pcmResourceInfo[_pcmResourceNumber].sector * 0x800 + addr - 0x65000;
	uint32 max = addr < 0x65000 ? 0x10000 : _pcmResourceInfo[_pcmResourceNumber].numSectors * 0x800;

	str->seek(start, SEEK_SET);
	numSamples = 0;

	for (bool loop = true; loop; ) {
		uint32 readSize = MIN<uint32>(0x8000, str->size() - str->pos());
		readSize = str->read(_temp, readSize);
		for (uint32 i = 0; i < readSize; ++i) {
			if (_temp[i] == 0xFF) {
				loop = false;
				readSize = i;
			}
		}
		if (readSize == 0)
			error("%s(): Error reading file", __FUNCTION__);

		numSamples += readSize;
		// The samples are supposed to have a 'FF' end marker, but we should still provide
		// an abort condition for corrupted files or other unexpected cases.
		if (numSamples > max)
			error("%s(): Samples stream overflow", __FUNCTION__);
	}

	str->seek(start, SEEK_SET);
	uint8 *buff = new uint8[numSamples]();
	str->read(buff, numSamples);

	_pcmCache.push_back(PCMCacheEntry(addr, _pcmResourceNumber, Common::SharedPtr<uint8>(buff, Common::ArrayDeleter<const uint8>()), numSamples));
	if (_pcmCache.size() > 10)
		_pcmCache.erase(_pcmCache.begin());

	return SmpStrPtr(new Common::MemoryReadStream(buff, numSamples, DisposeAfterUse::NO));
}

void SegaSoundDevice::pcmResetUnit(uint8 unit, PCMSound &s) {
	s.prio = 0;
	s.flags &= ~1;
	_sai->stopPCMChannel(unit << 1);
	_sai->stopPCMChannel((unit << 1) | 1);
}

void SegaSoundDevice::pcmStartSoundInternal(uint8 unit, uint8 chan, PCMSound &s) {
	SmpStrPtr smp = createSamplesStream(s.addr);
	_sai->playPCMStream(smp, chan << 5, 0x2000, chan, s.rate, calcPan(s), calcVolume(unit, s));
}

void SegaSoundDevice::pcmStartInstrument(uint8 unit, uint8 sndId, uint8 vol) {
	if (sndId == 6 || sndId == 8 || sndId == 9 || sndId == 10)
		return;

	PCMResidentDataInfo &p = _pcmResidentDataInfo[sndId];
	PCMInstrumentInfo &ins = _pcmInstrumentInfo[sndId];
	PCMSound &s = _pcmSounds[unit];

	s.id = sndId;
	if (s.prio > p.prio)
		return;
	pcmResetUnit(unit, s);

	s.unit = p.unit;
	s.volBase = p.vol;
	s.volEff = vol ? (p.vol * vol) >> 8 : p.vol;
	s.pan = p.pan;
	s.rate = p.rate;

	_sai->playPCMChannel(unit << 1, ins.addr >> 8, ins.loopst, s.rate, calcPan(s), calcVolume(unit, s));
	_sai->playPCMChannel((unit << 1) | 1, ins.addr >> 8, ins.loopst, s.rate, calcPan(s), calcVolume(unit, s));

	s.prio = p.prio;
}

uint8 SegaSoundDevice::calcVolume(uint8 unit, PCMSound &s) const {
	uint8 vol = s.volEff;
	if (_reduceVolume2)
		vol = (vol * 112) >> 8;
	if (unit >= 2 && _fmVolume1 != 0xFF)
		vol = (vol * _fmVolume1) >> 8;
	return vol;
}

uint8 SegaSoundDevice::calcPan(PCMSound &s) const {
	uint8 pan = (s.pan << 4) | (s.pan >> 4);
	if (_pcmDisableStereo) {
		pan = ((pan & 0x0F) + (pan >> 4)) >> 1;
		pan = pan | (pan << 4);
	}
	return pan;
}

SoundDevice *SoundDevice::createSegaSoundDevice(FIO *fio) {
	return new SegaSoundDevice(fio);
}

} // End of namespace Snatcher

