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
#include "kyra/sound/drivers/mlalf98.h"

#include "audio/softsynth/fmtowns_pc98/pc98_audio.h"

namespace Kyra {

class SoundChannel {
public:
	SoundChannel(PC98AudioCore *pc98a, int part, int regOffset, int type);
	virtual ~SoundChannel();

	virtual void setData(uint8 *dataStart, uint8 *loopStart, const uint8 *dataEnd, uint8 *instrBuffer);
	void globalBlock();
	void globalUnblock();

	void toggleMute(bool mute);
	virtual void restore() {};
	virtual bool checkFinished() { return false; }

	void update();

	void startFadeOut();
	void updateFadeOut();
	void abortFadeOut();

	virtual void keyOff() = 0;

	int idFlag() const;

protected:
	enum Flags {
		kInternalMask = 0x0F,
		kF1_10 = 1 << 4,
		kFade = 1 << 5,
		kTremolo = 1 << 6,
		kEnvelope = 1 << 7
	};

	enum Flags2 {
		kEoT = 1 << 0,
		kF2_02 = 1 << 1,
		kF2_04 = 1 << 2,
		kMute = 1 << 3,
		kF2_10 = 1 << 4,
		kVbrDelay = 1 << 5,
		kNoSustain = 1 << 6,
		kVbrEnable = 1 << 7
	};

	template <class SoundChannelImpl> class SoundOpcode : public Common::Functor1Mem<uint8*&, void, SoundChannelImpl> {
	public:
		typedef Common::Functor1Mem<uint8*&, void, SoundChannelImpl> SCFunc;
		SoundOpcode(SoundChannelImpl *sc, const typename SCFunc::FuncType &func, const char *dsc, int chanId, int chanType, int cDataBytes) : SCFunc(sc, func), _dataLen(cDataBytes) {
			static const char tpstr[4][4] = { "FM ", "SSG", "RHY", "EXT" };
			Common::String args;
			while (cDataBytes--)
				args += "0x%02x ";
			_msg = Common::String::format("%s Channel %d: %s() [ %s]", tpstr[chanType], chanId, dsc, args.c_str());
			memset(_dt, 0, 7);
		}
		~SoundOpcode() override {}
		void run(uint8 *&arg) {
			assert(arg);
			memcpy(_dt, arg, _dataLen);
			debugC(3, kDebugLevelSound, _msg.c_str(), _dt[0], _dt[1], _dt[2], _dt[3], _dt[4], _dt[5], _dt[6]);
			if (SCFunc::isValid())
				SCFunc::operator()(arg);
		}
	private:
		int _dataLen;
		uint8 _dt[7];
		Common::String _msg;
	};

	void vbrResetDelay();
	void vbrReset();

	virtual void clear();
	virtual void writeDevice(uint8 reg, uint8 val);

	uint8 _ticksLeft;
	uint8 _program;
	uint8 *_dataPtr;
	const uint8 *_dataEnd;
	uint8 *_loopStartPtr;
	uint8 _volume;
	uint8 _algorithm;
	const uint8 _regOffset;
	const uint8 _part;
	int16 _transpose;
	uint8 _envCurLvl;
	uint8 _envRR;
	uint8 _vbrDelay;
	uint8 _vbrRem;
	uint8 _vbrRate;
	uint8 _vbrTicker;
	int16 _vbrStepSize;
	int16 _vbrModifier;
	uint8 _vbrDepth;
	uint8 _vbrState;
	uint8 _duration;
	uint16 _frequency;
	uint8 _flags2;
	uint8 _note;
	uint8 _flags;
	uint8 *_backupData;

	static bool _globalBlock;

	int8 _fadeVolModifier;
	uint8 _fadeProgress;
	uint8 _fadeTicker;
	uint8 _trmCarrier;
	uint8 *_instrBuffer;

protected:
	virtual void op_setTranspose(uint8 *&data);
	void op_setDuration(uint8 *&data);
	void op_setVibrato(uint8 *&data);
	void op_repeatSectionBegin(uint8 *&data);
	void op_repeatSectionJumpIf(uint8 *&data);
	void op_jumpToSubroutine(uint8 *&data);
	void op_sustain(uint8 *&data);
	void op_repeatSectionAbort(uint8 *&data);
	void op_runOpcode2(uint8 *&data);

private:
	virtual void op2_setExtPara(uint8 *&data);
	void op2_setEnvGenFlags(uint8 *&data);
	void op2_setEnvGenTimer(uint8 *&data);
	void op2_beginFadeout(uint8 *&data);
	void op2_4(uint8 *&data);
	void op2_toggleFadeout(uint8 *&data);
	void op2_returnFromSubroutine(uint8 *&data);
	void op2_7(uint8 *&data);

	void op3_vbrInit(uint8 *&data);
	void op3_vbrDisable(uint8 *&data);
	void op3_vbrEnable(uint8 *&data);
	void op3_vbrSetDelay(uint8 *&data);
	void op3_vbrSetRate(uint8 *&data);
	void op3_vbrSetStepSize(uint8 *&data);
	void op3_vbrSetDepth(uint8 *&data);
	void op3_vbrSetTremolo(uint8 *&data);

	typedef SoundOpcode<SoundChannel> Opcode;
	Common::Array<Opcode*> _subOpcodes[2];

private:
	virtual void parse() = 0;
	virtual void updateVolume() = 0;
	void updateSounds();
	virtual void updateVibrato() = 0;

	const int _type;
	PC98AudioCore *_pc98a;
	bool _mute;
};

class SoundChannelNonSSG : public SoundChannel {
public:
	SoundChannelNonSSG(PC98AudioCore *pc98a, int part, int regOffset, int type);
	~SoundChannelNonSSG() override;

protected:
	void parse() override;
	uint8 _statusB4;

private:
	virtual void finish();
	void soundOff();

	virtual void noteOn(uint8 note) = 0;
	virtual void reduceVolume() {}
	void updateVolume() override {}
	void updateVibrato() override {}

	virtual void op_programChange(uint8 *&data) = 0;
	virtual void op_setVolume(uint8 *&data) = 0;
	virtual void op_setSpecialMode(uint8 *&data);
	virtual void op_setPanPos(uint8 *&data) = 0;
	virtual void op_writeDevice(uint8 *&data);
	virtual void op_modifyVolume(uint8 *&data);
	void op_enableLFO(uint8 *&data);

	typedef SoundOpcode<SoundChannelNonSSG> Opcode;
	Common::Array<Opcode*> _opcodes;
};

class MusicChannelFM : public SoundChannelNonSSG {
public:
	MusicChannelFM(PC98AudioCore *pc98a, int part, int regOffset);
	~MusicChannelFM() override;

	void restore() override;
	void keyOff() override;

private:
	void clear() override;
	void parse() override;
	void noteOn(uint8 note) override;
	void updateVolume() override;
	void reduceVolume() override;
	void updateVibrato() override;

	virtual bool usingSpecialMode() const;
	virtual uint8 getSpecialFrequencyModifier(uint8 index);
	virtual void setSpecialFrequencyModifier(uint8 index, uint8 val);
	virtual void toggleSpecialMode(bool on);

	void sendVolume(uint8 volume);
	void sendTrmVolume(uint8 volume);
	void keyOn();

	void writeDevice(uint8 reg, uint8 val) override;

	void op_programChange(uint8 *&data) override;
	void op_setVolume(uint8 *&data) override;
	void op_setSpecialMode(uint8 *&data) override;
	void op_setPanPos(uint8 *&data) override;
	void op_modifyVolume(uint8 *&data) override;

	static uint16 _frequency2;
	static uint8 _specialModeModifier[4];
	static bool _specialMode;
	static uint8 *_registers;
};

class MusicChannelSSG : public SoundChannel {
public:
	MusicChannelSSG(PC98AudioCore *pc98a, int part, int regOffset);
	~MusicChannelSSG() override;

	void keyOff() override;

private:
	enum EnvState {
		kAttack = 0x10,
		kDecay = 0x20,
		kSustain = 0x40,
		kUpdate = 0x80
	};

	void parse() override;
	void noteOff();
	void noteOn(uint8 note);
	uint8 processEnvelope();
	uint8 envGetAttLevel();
	void envSendAttLevel(uint8 val);

	void updateVolume() override;
	void updateVibrato() override;

	void clear() override;

	void op_programChange(uint8 *&data);
	void op_setVolume(uint8 *&data);
	void op_chanEnable(uint8 *&data);
	void op_setNoiseGenerator(uint8 *&data);
	void op_setInstrument(uint8 *&data);
	void op_modifyVolume(uint8 *&data);
	void op_loadInstrument(uint8 *&data);

	uint8 *getProgramData(uint8 program) const;
	bool _externalPrograms;

	typedef SoundOpcode<MusicChannelSSG> Opcode;
	Common::Array<Opcode*> _opcodes;

	uint8 _envStartLvl;
	uint8 _envAR;
	uint8 _envDR;
	uint8 _envSL;
	uint8 _envSR;
	static uint8 _ngState;
	static uint8 _enState;

	const uint8 _regOffsetAttn;

	uint8 *_envDataTemp;
	static const uint8 _envDataPreset[96];
};

class MusicChannelRHY : public SoundChannelNonSSG {
public:
	MusicChannelRHY(PC98AudioCore *pc98a, int part, int regOffset);
	~MusicChannelRHY() override;

	void keyOff() override;

private:
	void noteOn(uint8 note) override;
	void updateVolume() override;

	void op_programChange(uint8 *&data) override;
	void op_setVolume(uint8 *&data) override;
	void op_setPanPos(uint8 *&data) override;
	void op_modifyVolume(uint8 *&data) override;

	uint8 _instrLevel[6];

	uint8 _activeInstruments;
};

class MusicChannelEXT : public SoundChannelNonSSG {
public:
	MusicChannelEXT(PC98AudioCore *pc98a, int part, int regOffset, MLALF98::ADPCMData *const &data);
	~MusicChannelEXT() override;

	void keyOff() override;

private:
	void noteOn(uint8 note) override;
	void updateVibrato() override;
	void clear() override;
	void writeDevice(uint8 reg, uint8 val) override;

	void op_programChange(uint8 *&data) override;
	void op_setVolume(uint8 *&data) override;
	void op_setPanPos(uint8 *&data) override;
	void op_setTranspose(uint8 *&data) override;

	void op2_setExtPara(uint8 *&data) override;

	uint8 _panPos;
	uint8 _useVolPreset;
	uint8 _volume2;
	uint8 _instrument;
	MLALF98::ADPCMData *const &_extBuffer;
	uint16 _smpStart;
	uint16 _smpEnd;

	PC98AudioCore *_pc98a;
};

class SoundEffectChannel : public MusicChannelFM {
public:
	SoundEffectChannel(PC98AudioCore *pc98a, int part, int regOffset, SoundChannel *replaceChannel);
	~SoundEffectChannel() override;

	void setData(uint8 *dataStart, uint8 *loopStart, const uint8 *dataEnd, uint8 *instrBuffer) override;
	bool checkFinished() override;

private:
	void finish() override;
	bool usingSpecialMode() const override;
	uint8 getSpecialFrequencyModifier(uint8 index) override;
	void setSpecialFrequencyModifier(uint8 index, uint8 val) override;
	void toggleSpecialMode(bool on) override;
	void clear() override;
	void writeDevice(uint8 reg, uint8 val) override;

	void op_writeDevice(uint8 *&data) override;

	uint8 _specialModeModifier[4];
	bool _specialMode;
	bool _isFinished;

	SoundChannel *_replaceChannel;
};

class MLALF98Internal : public PC98AudioPluginDriver {
public:
	MLALF98Internal(Audio::Mixer *mixer, EmuType emuType);
	~MLALF98Internal() override;

	static MLALF98Internal *open(Audio::Mixer *mixer, EmuType emuType);
	static void close();

	void loadMusicData(Common::SeekableReadStream *data);
	void loadSoundEffectData(Common::SeekableReadStream *data);
	void loadExtData(MLALF98::ADPCMDataArray &data);

	void startMusic(int track);
	void fadeOutMusic();
	void startSoundEffect(int track);

	void allChannelsOff();
	void resetExtUnit();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	// Plugin driver interface
	void timerCallbackA() override;
	void timerCallbackB() override;

private:
	uint8 *_musicBuffer;
	int _musicBufferSize;
	uint8 *_sfxBuffer;
	int _sfxBufferSize;
	MLALF98::ADPCMData *_extBuffer;
	int _extBufferSize;

	Common::Array<SoundChannel*> _musicChannels;
	Common::Array<SoundChannel*> _sfxChannels;

	const bool _type86;
	PC98AudioCore *_pc98a;

	static MLALF98Internal *_refInstance;
	static int _refCount;

	int _sfxPlaying;
	bool _ready;
};

bool SoundChannel::_globalBlock = false;

SoundChannel::SoundChannel(PC98AudioCore *pc98a, int part, int regOffset, int type) : _pc98a(pc98a), _regOffset(regOffset), _part(part),
_ticksLeft(0), _program(0), _volume(0), _algorithm(0), _envRR(0), _vbrDelay(0), _vbrRem(0), _vbrRate(0), _vbrTicker(0), _vbrStepSize(0), _vbrModifier(0),
_vbrDepth(0), _vbrState(0), _duration(0), _frequency(0), _flags2(0), _note(0), _flags(0),
_transpose(0), _envCurLvl(0), _fadeVolModifier(0), _fadeProgress(0), _fadeTicker(16), _trmCarrier(1),
_dataPtr(0), _dataEnd(0), _loopStartPtr(0), _instrBuffer(0), _backupData(0), _mute(false), _type(type) {
	_subOpcodes[0].reserve(8);
	_subOpcodes[1].reserve(8);
#define OPCODE(x, y, z)	_subOpcodes[x].push_back(new Opcode(this, &SoundChannel::y, #y, type == 1 ? (_regOffset >> 1) : (type == 0 ? (_regOffset + _part * 3) : 0), type, z))
	OPCODE(0, op2_setExtPara, 1);
	OPCODE(0, op2_setEnvGenFlags, 1);
	OPCODE(0, op2_setEnvGenTimer, 2);
	OPCODE(0, op2_beginFadeout, 1);
	OPCODE(0, op2_4, 1);
	OPCODE(0, op2_toggleFadeout, 1);
	OPCODE(0, op2_returnFromSubroutine, 0);
	OPCODE(0, op2_7, 1);
	OPCODE(1, op3_vbrInit, 5);
	OPCODE(1, op3_vbrDisable, 0);
	OPCODE(1, op3_vbrEnable, 0);
	OPCODE(1, op3_vbrSetDelay, 1);
	OPCODE(1, op3_vbrSetRate, 1);
	OPCODE(1, op3_vbrSetStepSize, 2);
	OPCODE(1, op3_vbrSetDepth, 1);
	OPCODE(1, op3_vbrSetTremolo, 2);
#undef OPCODE
}

SoundChannel::~SoundChannel() {
	for (int c = 0; c < 2; ++c) {
		for (Common::Array<Opcode*>::iterator i = _subOpcodes[c].begin(); i != _subOpcodes[c].end(); ++i)
			delete (*i);
	}
}

void SoundChannel::setData(uint8 *dataStart, uint8 *loopStart, const uint8 *dataEnd, uint8 *instrBuffer) {
	clear();
	_dataPtr = dataStart;
	_dataEnd = dataEnd;
	_loopStartPtr = loopStart;
	_instrBuffer = instrBuffer;
	_ticksLeft = 1;
}

void SoundChannel::globalBlock() {
	_globalBlock = true;
}

void SoundChannel::globalUnblock() {
	_globalBlock = false;
}

void SoundChannel::toggleMute(bool mute) {
	_mute = mute;
}

void SoundChannel::update() {
	if (!_dataPtr)
		return;

	if (_flags2 & kMute)
		globalBlock();

	parse();
	updateSounds();

	if (_flags2 & kMute)
		globalUnblock();
}

void SoundChannel::startFadeOut() {
	_fadeProgress = 16;
}

void SoundChannel::updateFadeOut() {
	if (--_fadeTicker)
		return;

	_fadeTicker = 16;

	if (!_fadeProgress)
		return;

	_fadeVolModifier = (--_fadeProgress) - 16;

	updateVolume();

	if (_fadeProgress)
		return;

	_fadeVolModifier = 0;
	keyOff();
}

void SoundChannel::abortFadeOut() {
	_fadeProgress = 0;
	_fadeVolModifier = 0;
}

int SoundChannel::idFlag() const {
	switch (_type) {
	case 0:
		return 1 << (_regOffset + _part * 3);
	case 1:
		return 1 << ((_regOffset >> 1) + 6);
	case 2:
		return 1 << 9;
	case 3:
		return 1 << 10;
	default:
		break;
	}
	return 0;
}

void SoundChannel::vbrResetDelay() {
	_vbrRem = _vbrDelay;
	_flags2 &= ~kVbrDelay;
}

void SoundChannel::vbrReset() {
	_vbrState = _vbrDepth >> 1;
	_vbrModifier = _vbrStepSize;
	if (_flags & kTremolo)
		_frequency = _envCurLvl;
}

void SoundChannel::clear() {
	_ticksLeft = _program = _volume	= _algorithm = _duration = _envRR = _vbrDelay = _vbrRem = _vbrRate = _vbrTicker = _vbrDepth = _vbrState = _flags2 = _note = _flags = 0;
	_frequency = _transpose = _vbrStepSize = _vbrModifier = 0;
	_dataPtr = _loopStartPtr = 0;
}

void SoundChannel::writeDevice(uint8 reg, uint8 val) {
	if (!_mute)
		_pc98a->writeReg(reg > 0x2F ? _part : 0, reg, val);
}

void SoundChannel::op_setTranspose(uint8 *&data) {
	_note = 0;
	int16 trp = READ_LE_INT16(data);
	data += 2;
	_transpose = (*data++) ? _transpose + trp : trp;
}

void SoundChannel::op_setDuration(uint8 *&data) {
	_duration = *data++;
}

void SoundChannel::op_setVibrato(uint8 *&data) {
	uint8 cmd = (*data++) & 0x0F;
	assert(cmd < _subOpcodes[1].size());
	_subOpcodes[1][cmd & 0x0F]->run(data);
}

void SoundChannel::op_repeatSectionBegin(uint8 *&data) {
	int16 offset = READ_LE_INT16(data);
	assert(offset > 0);
	// reset repeat counter
	data[offset - 1] = data[offset];
	data += 2;
}

void SoundChannel::op_repeatSectionJumpIf(uint8 *&data) {
	// reduce the repeat counter
	if (--data[0]) {
		// If the counter has not yet reached zero we go back to the beginning of the repeat section.
		data += 2;
		int16 offset = READ_LE_INT16(data);
		assert(offset > 0);
		data -= offset;
	} else {
		// If the counter has reached zero we reset it to the original value and advance to the next section.
		data[0] = data[1];
		data += 4;
	}
}

void SoundChannel::op_jumpToSubroutine(uint8 *&data) {
#if 0
	uint16 offset = READ_BE_UINT16(data);
	_backupData = data + 2;
	if (offset > 0x253D)
		offset -= 0x253D;
	else if (offset > 0x188F)
		offset -= 0x188F;
	else
		warning("SoundChannel::op_jumpToSubroutine(): invalid offset");
	data = _buffer + offset;
#else
	// This is a very ugly opcode which reads and sets an absolute 16 bit offset
	// inside the music driver segment. Thus, the ip can jump anywhere, not only
	// from one track or channel to another or from the music buffer to the sfx
	// buffer or vice versa, but also to any other code or data location within
	// the segment. Different driver versions are more or less doomed to mutual
	// incompatibility.
	// The music data buffer has offset 0x188F and the sfx data buffer 0x253D. So
	// I could implement this if I have to. I'd prefer if this never comes up,
	// though...
	data += 2;
	warning("SoundChannel::op_jumpToSubroutine(): not implemented");
#endif
}

void SoundChannel::op_sustain(uint8 *&data) {
	_flags2 &= ~kNoSustain;
}

void SoundChannel::op_repeatSectionAbort(uint8 *&data) {
	int16 offset = READ_LE_INT16(data);
	assert(offset > 0);
	data = (data[offset] == 1) ? data + offset + 4 : data + 2;
}

void SoundChannel::op_runOpcode2(uint8 *&data) {
	uint8 cmd = (*data++) & 0x0F;
	assert(cmd < _subOpcodes[0].size());
	_subOpcodes[0][cmd & 0x0F]->run(data);
}

void SoundChannel::op2_setExtPara(uint8 *&data) {
	data++;
}

void SoundChannel::op2_setEnvGenFlags(uint8 *&data) {
	_flags = *data++;
	writeDevice(0x0D, _flags);
	_flags |= kEnvelope;
}

void SoundChannel::op2_setEnvGenTimer(uint8 *&data) {
	writeDevice(0x0B, *data++);
	writeDevice(0x0C, *data++);
}

void SoundChannel::op2_beginFadeout(uint8 *&data) {
	_envRR = *data++;
	_flags |= kFade;
}

void SoundChannel::op2_4(uint8 *&data) {
	if (*data++)
		_flags |= kF1_10;
	else
		_flags &= ~kF1_10;
}

void SoundChannel::op2_toggleFadeout(uint8 *&data) {
	if (*data++) {
		_flags |= kFade;
	} else {
		_flags &= ~kFade;
		updateVolume();
	}
}

void SoundChannel::op2_returnFromSubroutine(uint8 *&data) {
	assert(_backupData);
	data = _backupData;
}

void SoundChannel::op2_7(uint8 *&data) {
	/*_unkbyte27 = */data++;
}

void SoundChannel::op3_vbrInit(uint8 *&data) {
	op3_vbrSetDelay(data);
	op3_vbrSetRate(data);
	_vbrStepSize = _vbrModifier = READ_LE_INT16(data);
	data += 2;
	op3_vbrSetDepth(data);
	_flags2 |= kVbrEnable;
}

void SoundChannel::op3_vbrDisable(uint8 *&data) {
	_flags2 &= ~kVbrEnable;
}

void SoundChannel::op3_vbrEnable(uint8 *&data) {
	_flags2 |= kVbrEnable;
}

void SoundChannel::op3_vbrSetDelay(uint8 *&data) {
	_vbrDelay = _vbrRem = *data++;
}

void SoundChannel::op3_vbrSetRate(uint8 *&data) {
	_vbrRate = _vbrTicker = *data++;
}

void SoundChannel::op3_vbrSetStepSize(uint8 *&data) {
	_vbrStepSize = _vbrModifier = READ_LE_INT16(data);
	data += 2;
	vbrResetDelay();
}

void SoundChannel::op3_vbrSetDepth(uint8 *&data) {
	_vbrDepth = *data++;
	_vbrState = _vbrDepth >> 1;
}

void SoundChannel::op3_vbrSetTremolo(uint8 *&data) {
	uint8 flags = *data++;
	if (!flags) {
		_flags &= ~kTremolo;
		return;
	}
	_flags |= kTremolo;
	_trmCarrier = flags;
	_frequency = _envCurLvl = *data++;
}

void SoundChannel::updateSounds() {
	if (!(_flags2 & kVbrEnable))
		return;

	if (!_dataPtr)
		return;

	if (*(_dataPtr - 1) == 0xF0)
		return;

	if (!(_flags2 & kVbrDelay)) {
		vbrResetDelay();
		vbrReset();
		_vbrTicker = _vbrRate;
		_flags2 |= kVbrDelay;
	}

	if (_vbrRem) {
		_vbrRem--;
		return;
	}

	if (--_vbrTicker)
		return;

	_vbrTicker = _vbrRate;
	if (!_vbrState) {
		_vbrModifier *= -1;
		_vbrState = _vbrDepth;
	}

	_vbrState--;
	updateVibrato();
}

SoundChannelNonSSG::SoundChannelNonSSG(PC98AudioCore *pc98a, int part, int regOffset, int type) : SoundChannel(pc98a, part, regOffset, type), _statusB4(0xC0) {
	_opcodes.reserve(16);
#define OPCODE(y, z)	_opcodes.push_back(new Opcode(this, &SoundChannelNonSSG::y, #y, _regOffset + _part * 3, type, z))
	OPCODE(op_programChange, 1);
	OPCODE(op_setVolume, type == 2 ? 7 : 1);
	OPCODE(op_setTranspose, 3);
	OPCODE(op_setDuration, 1);
	OPCODE(op_setVibrato, 1);
	OPCODE(op_repeatSectionBegin, 2);
	OPCODE(op_repeatSectionJumpIf, 4);
	OPCODE(op_setSpecialMode, 4);
	OPCODE(op_setPanPos, 1);
	OPCODE(op_jumpToSubroutine, 2);
	OPCODE(op_writeDevice, 2);
	OPCODE(op_modifyVolume, 1);
	OPCODE(op_enableLFO, 3);
	OPCODE(op_sustain, 0);
	OPCODE(op_repeatSectionAbort, 2);
	OPCODE(op_runOpcode2, 1);
#undef OPCODE
}

SoundChannelNonSSG::~SoundChannelNonSSG() {
	for (Common::Array<Opcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
		delete (*i);
}

void SoundChannelNonSSG::parse() {
	if (!_ticksLeft && (!_dataPtr || _dataPtr >= _dataEnd))
		return;

	if (--_ticksLeft) {
		if (_ticksLeft <= _duration)
			soundOff();
		return;
	}

	uint8 *pos = _dataPtr;
	_flags2 |= kNoSustain;

	uint8 cmd = 0;
	for (bool loop = true; loop && pos && pos < _dataEnd; ) {
		if (*pos == 0) {
			_flags2 |= kEoT;
			if (!_loopStartPtr) {
				_dataPtr = 0;
				finish();
				return;
			}
			pos = _loopStartPtr;
		}

		cmd = *pos++;
		if (cmd < 0xF0)
			break;

		_opcodes[cmd & 0x0F]->run(pos);
	}

	_ticksLeft = cmd & 0x7F;

	if (cmd & 0x80) {
		if ((_flags & kFade) && !(_flags & kF1_10))
			reduceVolume();
		keyOff();
	} else if (pos && pos < _dataEnd) {
		if (_flags2 & kNoSustain)
			keyOff();
		noteOn(*pos++);
	}
	_dataPtr = pos;
}

void SoundChannelNonSSG::finish() {
	keyOff();
}

void SoundChannelNonSSG::soundOff() {
	if (*_dataPtr == 0xFD)
		return;
	if (_flags & kFade)
		reduceVolume();
	else
		keyOff();
}

void SoundChannelNonSSG::op_setSpecialMode(uint8 *&data) {
	data += 4;
}

void SoundChannelNonSSG::op_writeDevice(uint8 *&data) {
	uint8 reg = *data++;
	uint8 val = *data++;
	writeDevice(reg, val);
}

void SoundChannelNonSSG::op_modifyVolume(uint8 *&data) {
	data++;
}

void SoundChannelNonSSG::op_enableLFO(uint8 *&data) {
	// This concerns only the fm channels, but the opcode may be called by other channels nonetheless
	writeDevice(0x22, (*data++) | 8);
	_statusB4 = (_statusB4 & 0xC0) | data[0] | (data[1] << 4 | data[1] >> 4);
	data += 2;
	writeDevice(0xB4, _statusB4);
}

uint8 *MusicChannelFM::_registers = 0;
bool MusicChannelFM::_specialMode = false;
uint8 MusicChannelFM::_specialModeModifier[4] = { 0, 0, 0, 0 };
uint16 MusicChannelFM::_frequency2 = 0;

MusicChannelFM::MusicChannelFM(PC98AudioCore *pc98a, int part, int regOffset) : SoundChannelNonSSG(pc98a, part, regOffset, 0) {
	if (!_registers) {
		_registers = new uint8[512];
		memset(_registers, 0, 512);
	}
}

MusicChannelFM::~MusicChannelFM() {
	delete[] _registers;
	_registers = 0;
}

void MusicChannelFM::restore() {
	for (int i = 0x30 + _regOffset; i < 0xA0; i += 4)
		writeDevice(i, _registers[(_part << 8) + i]);
	writeDevice(0xB0 + _regOffset, _registers[(_part << 8) + 0xB0 + _regOffset]);
	writeDevice(0xB4 + _regOffset, _registers[(_part << 8) + 0xB4 + _regOffset]);
	_note = 0;
}

void MusicChannelFM::keyOff() {
	debugC(7, kDebugLevelSound, "FM  Channel %d: keyOff() [Ticks: 0x%02x]", _part * 3 + _regOffset, _ticksLeft);
	writeDevice(0x28, (_part << 2) + _regOffset);
}

void MusicChannelFM::clear() {
	SoundChannel::clear();
	_specialMode = false;
	_statusB4 = 0xC0;
}

void MusicChannelFM::parse() {
	toggleSpecialMode(usingSpecialMode());
	SoundChannelNonSSG::parse();
}
void MusicChannelFM::noteOn(uint8 note) {
	static uint16 freqTableFM[12] = {
		0x026a, 0x028f, 0x02b6, 0x02df, 0x030b, 0x0339, 0x036a, 0x039e, 0x03d5, 0x0410, 0x044e, 0x048f
	};

	if (_note == note && !(_flags2 & kNoSustain))
		return;

	_note = note;

	if ((note & 0x0F) >= 12)
		return;

	debugC(5, kDebugLevelSound, "FM  Channel %d: noteOn() [Note: 0x%02x Ticks: 0x%02x]", _part * 3 + _regOffset, _note, _ticksLeft);

	uint16 frq = (((note & 0x70) << 7) | freqTableFM[note & 0x0F]) + _transpose;
	if (!(_flags & kTremolo))
		_frequency = _frequency2 = frq;

	if (_flags2 & kNoSustain)
		vbrResetDelay();

	vbrReset();

	if (usingSpecialMode())
		frq += getSpecialFrequencyModifier(0);

	writeDevice(0xA4 + _regOffset, frq >> 8);
	writeDevice(0xA0 + _regOffset, frq & 0xFF);
	keyOn();

	if (usingSpecialMode()) {
		for (int i = 1; i < 4; ++i) {
			uint16 frqFin = _frequency2 + getSpecialFrequencyModifier(i);
			writeDevice(0xA9 + i + _regOffset, frqFin >> 8);
			writeDevice(0xA5 + i + _regOffset, frqFin & 0xFF);
			keyOn();
		}
	}
}

void MusicChannelFM::updateVolume() {
	uint8 val = _fadeVolModifier + _volume;
	sendVolume(val < 20 ? val : 0);
}

void MusicChannelFM::reduceVolume() {
	sendVolume((uint8)(_volume + _envRR) >> 1);
	_flags2 |= kNoSustain;
}

void MusicChannelFM::updateVibrato() {
	_frequency += _vbrModifier;

	if (_flags & kTremolo) {
		sendTrmVolume(_frequency & 0xFF);
		return;
	}

	if (!usingSpecialMode()) {
		writeDevice(0xA4 + _regOffset, _frequency >> 8);
		writeDevice(0xA0 + _regOffset, _frequency & 0xFF);
		return;
	}

	for (int i = 0; i < 4; ++i) {
		uint16 frqFin = _frequency + getSpecialFrequencyModifier(i);
		writeDevice(0xA9 + i + _regOffset, frqFin >> 8);
		writeDevice(0xA5 + i + _regOffset, frqFin & 0xFF);
	}
}

bool MusicChannelFM::usingSpecialMode() const {
	return _specialMode == true && _regOffset == 2 && _part == 0;
}

uint8 MusicChannelFM::getSpecialFrequencyModifier(uint8 index) {
	assert(index < 4);
	return _specialModeModifier[index];
}

void MusicChannelFM::setSpecialFrequencyModifier(uint8 index, uint8 val) {
	assert(index < 4);
	_specialModeModifier[index] = val;
}

void MusicChannelFM::toggleSpecialMode(bool on) {
	_specialMode = on;
	uint8 flag = on ? 0x40 : 0;
	writeDevice(0x27, 0x3D + flag);
	writeDevice(0x27, 0x3F + flag);
}

void MusicChannelFM::sendVolume(uint8 volume) {
	static const uint8 volTable[20] = {
		0x36, 0x33, 0x30, 0x2d, 0x2a, 0x28, 0x25, 0x22, 0x20, 0x1d,
		0x1a, 0x18, 0x15, 0x12, 0x10, 0x0d, 0x0a, 0x08, 0x05, 0x02
	};

	static const uint8 carrier[8] = {
		0x08, 0x08, 0x08, 0x08, 0x0c, 0x0e, 0x0e, 0x0f
	};

	assert(volume < 20);
	assert(_algorithm < 8);

	uint8 reg = 0x40 + _regOffset;
	for (uint8 c = carrier[_algorithm]; c; c >>= 1) {
		if (c & 1)
			writeDevice(reg, volTable[volume]);
		reg += 4;
	}
}

void MusicChannelFM::sendTrmVolume(uint8 volume) {
	static uint8 cflg[4] = { 1, 4, 2, 8 };
	uint8 reg = 0x40 + _regOffset;
	for (int i = 0; i < 4; ++i) {
		if (_trmCarrier & cflg[i]) {
			writeDevice(reg, volume);
			reg += 4;
		}
	}
}

void MusicChannelFM::keyOn() {
	writeDevice(0x28, 0xF0 | ((_part << 2) + _regOffset));
	if (_flags & kFade)
		updateVolume();
}

void MusicChannelFM::writeDevice(uint8 reg, uint8 val) {
	_registers[(_part << 8) + reg] = val;
	SoundChannel::writeDevice(reg, val);
}

void MusicChannelFM::op_programChange(uint8 *&data) {
	_program = *data++;
	keyOff();

	for (int i = 0x80 + _regOffset; i < 0x90; i += 4)
		writeDevice(i, 0x0F);

	const uint8 *src = _instrBuffer + READ_LE_UINT16(_instrBuffer) + _program * 25;

	for (int i = 0x30 + _regOffset; i < 0x90; i += 4)
		writeDevice(i, *src++);

	_algorithm = *src & 7;
	writeDevice(0xB0 + _regOffset, *src);

	updateVolume();
}

void MusicChannelFM::op_setVolume(uint8 *&data) {
	_volume = *data++;
	updateVolume();
}

void MusicChannelFM::op_setSpecialMode(uint8 *&data) {
	toggleSpecialMode(true);
	for (int i = 0; i < 4; ++i)
		setSpecialFrequencyModifier(i, *data++);
}

void MusicChannelFM::op_setPanPos(uint8 *&data) {
	uint8 val = *data++;
	_statusB4 = (_statusB4 & 0x3f) | (val >> 2) | (val << 6);
	writeDevice(0xB4 + _regOffset, _statusB4);
}

void MusicChannelFM::op_modifyVolume(uint8 *&data) {
	_volume += *data++;
	updateVolume();
}

uint8 MusicChannelSSG::_enState = 0x38;
uint8 MusicChannelSSG::_ngState = 0;

MusicChannelSSG::MusicChannelSSG(PC98AudioCore *pc98a, int part, int regOffset) : SoundChannel(pc98a, part, regOffset, 1), _regOffsetAttn(8 + (regOffset >> 1)),
	_externalPrograms(0), _envDataTemp(0), _envStartLvl(0), _envAR(0), _envDR(0), _envSL(0), _envSR(0) {
	_opcodes.reserve(16);
#define OPCODE(y, z)	_opcodes.push_back(new Opcode(this, &MusicChannelSSG::y, #y, _regOffset >> 1, 1, z))
	OPCODE(op_programChange, 1);
	OPCODE(op_setVolume, 1);
	OPCODE(op_setTranspose, 3);
	OPCODE(op_setDuration, 1);
	OPCODE(op_setVibrato, 1);
	OPCODE(op_repeatSectionBegin, 2);
	OPCODE(op_repeatSectionJumpIf, 4);
	OPCODE(op_chanEnable, 1);
	OPCODE(op_setNoiseGenerator, 1);
	OPCODE(op_jumpToSubroutine, 2);
	OPCODE(op_setInstrument, 6);
	OPCODE(op_modifyVolume, 1);
	OPCODE(op_loadInstrument, 7);
	OPCODE(op_sustain, 0);
	OPCODE(op_repeatSectionAbort, 2);
	OPCODE(op_runOpcode2, 1);
#undef OPCODE
	_envDataTemp = new uint8[96];
	memcpy(_envDataTemp, _envDataPreset, 96);
}

MusicChannelSSG::~MusicChannelSSG() {
	for (Common::Array<Opcode*>::iterator i = _opcodes.begin(); i != _opcodes.end(); ++i)
		delete (*i);
	delete[] _envDataTemp;
}

void MusicChannelSSG::keyOff() {
	debugC(7, kDebugLevelSound, "SSG Channel %d: keyOff() [Ticks: 0x%02x]", _regOffset >> 1, _ticksLeft);
	_volume = 0;
	writeDevice(_regOffsetAttn, 0);
}

void MusicChannelSSG::parse() {
	if (!_ticksLeft && (!_dataPtr || _dataPtr >= _dataEnd))
		return;

	if (--_ticksLeft) {
		if (_ticksLeft <= _duration) {
			if (_dataPtr && _dataPtr < _dataEnd) {
				if (*_dataPtr == 0xFD)
					_flags2 &= ~kNoSustain;
				else
					noteOff();
			}
		}

		if (_volume & kUpdate) {
			uint8 val = processEnvelope();
			writeDevice(_regOffsetAttn, _globalBlock ? 0 : val);
		}
		return;
	}

	uint8 *pos = _dataPtr;
	if (pos && pos < _dataEnd) {
		if (*pos == 0xFD) {
			_flags2 &= ~kNoSustain;
			pos++;
		} else {
			_flags2 |= kNoSustain;
		}
	}

	uint8 cmd = 0;
	for (bool loop = true && pos && pos < _dataEnd; loop; ) {
		if (*pos == 0) {
			_flags2 |= kEoT;
			if (!_loopStartPtr) {
				_dataPtr = 0;
				keyOff();
				_flags2 &= ~kVbrEnable;
				return;
			}
			pos = _loopStartPtr;
		}

		cmd = *pos++;
		if (cmd < 0xF0)
			break;

		_opcodes[cmd & 0x0F]->run(pos);
	}

	_ticksLeft = cmd & 0x7F;

	if (cmd & 0x80)
		noteOff();
	else if (pos && pos < _dataEnd)
		noteOn(*pos++);
	_dataPtr = pos;
}

void MusicChannelSSG::noteOff() {
	debugC(7, kDebugLevelSound, "SSG Channel %d: noteOff() [Ticks: 0x%02x]", _regOffset >> 1, _ticksLeft);
	if (_flags & kEnvelope)
		writeDevice(_regOffsetAttn, 0);

	if (_flags & kFade) {
		_flags2 &= ~kNoSustain;
		if (_volume & kUpdate) {
			uint8 val = processEnvelope();
			writeDevice(_regOffsetAttn, _globalBlock ? 0 : val);
		}
		return;
	}

	if (_volume & kUpdate) {
		_volume &= ~(kAttack | kDecay | kSustain);
		_envCurLvl = MAX<int>(_envCurLvl - _envRR, 0);
		envSendAttLevel(envGetAttLevel());
	} else {
		envSendAttLevel(0);
	}
}

void MusicChannelSSG::noteOn(uint8 note) {
	static uint16 freqTableSSG[12] = {
		0x0EE8, 0x0E12, 0x0D48, 0x0C89, 0x0BD5, 0x0B2B, 0x0A8A, 0x09F3, 0x0964, 0x08DD, 0x085E, 0x07E6
	};

	if (_note == note && !(_flags2 & kNoSustain))
		return;

	_note = note;

	debugC(5, kDebugLevelSound, "SSG Channel %d: noteOn() [Note: 0x%02x Ticks: 0x%02x]", _regOffset >> 1, _note, _ticksLeft);

	assert((note & 0x0F) < 12);
	_frequency = freqTableSSG[note & 0x0F] + _transpose;
	uint16 frq = _frequency >> (_note >> 4);
	writeDevice(_regOffset, frq & 0xFF);
	writeDevice(_regOffset + 1, frq >> 8);

	uint8 lv = 0;
	if (_flags2 & kNoSustain) {
		if (_flags & kEnvelope) {
			writeDevice(_regOffsetAttn, 0x10);
			writeDevice(0x0D, _flags & kInternalMask);
		} else {
			_volume = (kUpdate | kAttack) | (_volume & 0x0F);
			_envCurLvl = _envStartLvl;
			_flags2 &= ~kVbrDelay;
		}
		_vbrState = _vbrDepth >> 1;
		_vbrRem = _vbrDelay;
		lv = envGetAttLevel();
	} else {
		lv = processEnvelope();
	}
	envSendAttLevel(lv);
}

uint8 MusicChannelSSG::processEnvelope() {
	if (_volume & kAttack) {
		_envCurLvl = (uint8)MIN<int>(_envCurLvl + _envAR, 0xFF);
		if (_envCurLvl == 0xFF)
			_volume ^= (kAttack | kDecay);
	} else if (_volume & kDecay) {
		_envCurLvl = (uint8)MAX<int>(_envCurLvl - _envDR, 0);
		_envCurLvl = MAX<uint8>(_envCurLvl, _envSL);
		if (_envCurLvl == _envSL)
			_volume ^= (kDecay | kSustain);
	} else if (_volume & kSustain) {
		_envCurLvl = (uint8)MAX<int>(_envCurLvl - _envSR, 0);
		if (_envCurLvl == 0)
			_volume &= ~(kAttack | kDecay | kSustain);
	} else {
		_envCurLvl = (uint8)MAX<int>(_envCurLvl - _envRR, 0);
	}
	return envGetAttLevel();
}

uint8 MusicChannelSSG::envGetAttLevel() {
	uint8 val = ((uint16)_envCurLvl * ((_volume & 0x0F) + 2)) >> 8;
	return !(_flags2 & kNoSustain) && (_flags & kFade) ? (uint8)(val + _envRR) >> 1 : val;
}

void MusicChannelSSG::envSendAttLevel(uint8 val) {
	if (!(_flags & kEnvelope))
		writeDevice(_regOffsetAttn, _globalBlock ? 0 : val);
}

void MusicChannelSSG::updateVolume() {
	uint8 volNew = (_volume & 0x0F) + _fadeVolModifier;
	_volume &= 0xF0;
	if (volNew < 16)
		_volume |= volNew;
}

void MusicChannelSSG::updateVibrato() {
	_frequency += _vbrModifier;
	uint16 frq = _frequency >> (_note >> 4);
	writeDevice(_regOffset, frq & 0xFF);
	writeDevice(_regOffset + 1, frq >> 8);
}

void MusicChannelSSG::clear() {
	SoundChannel::clear();
	memcpy(_envDataTemp, _envDataPreset, 96);
	_ngState = 0;
	_enState = 0x38;
	_envStartLvl = _envAR = _envDR = _envSL = _envSR = 0;
}

void MusicChannelSSG::op_programChange(uint8 *&data) {
	uint8 *src = getProgramData(*data++);
	_envStartLvl = *src++;
	_envAR = *src++;
	_envDR = *src++;
	_envSL = *src++;
	_envSR = *src++;
	_envRR = *src++;
	_volume |= (kUpdate | kAttack);
}

void MusicChannelSSG::op_setVolume(uint8 *&data) {
	_flags &= ~kEnvelope;
	_volume = (_volume & 0xF0) | (*data++);
	updateVolume();
}

void MusicChannelSSG::op_chanEnable(uint8 *&data) {
	uint8 c = (_regOffset >> 1) + 1;
	uint8 val = *data++;
	val = (val >> 1) | (val << 7);
	val = (val << c) | (val >> (8 - c));
	_enState = (((0x7B << c) | (0x7B >> (8 - c))) & _enState) | val;
	writeDevice(0x07, _enState);
}

void MusicChannelSSG::op_setNoiseGenerator(uint8 *&data) {
	_ngState = *data++;
	writeDevice(0x06, _ngState);
}

void MusicChannelSSG::op_setInstrument(uint8 *&data) {
	_envStartLvl = *data++;
	_envAR = *data++;
	_envDR = *data++;
	_envSL = *data++;
	_envSR = *data++;
	_envRR = *data++;
	_volume |= (kUpdate | kAttack);
}

void MusicChannelSSG::op_modifyVolume(uint8 *&data) {
	if (_flags & kEnvelope)
		return;
	uint8 newVol = (_volume & 0x0F) + (*data++);
	if (newVol > 15)
		return;
	_volume = (_volume & 0xF0) | newVol;
}

void MusicChannelSSG::op_loadInstrument(uint8 *&data) {
	uint8 *dst = getProgramData(*data++);
	memcpy(dst, data, 6);
	data += 6;
}

uint8 *MusicChannelSSG::getProgramData(uint8 program) const {
	return _externalPrograms ? _instrBuffer + READ_LE_UINT16(_instrBuffer + 2) + (program << 4) + 1 : _envDataTemp + program * 6;
}

const uint8 MusicChannelSSG::_envDataPreset[96] = {
	0xff, 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff,
	0xff, 0xc8, 0x00, 0x0a, 0xff, 0xff, 0xff, 0xc8,
	0x01, 0x0a, 0xff, 0xff, 0xff, 0xbe, 0x00, 0x0a,
	0xff, 0xff, 0xff, 0xbe, 0x01, 0x0a, 0xff, 0xff,
	0xff, 0xaa, 0x00, 0x0a, 0x28, 0x46, 0x0e, 0xbe,
	0x00, 0x0f, 0x78, 0x1e, 0xff, 0xff, 0x00, 0x0a,
	0xff, 0xff, 0xff, 0xe1, 0x08, 0x0f, 0xff, 0xff,
	0xff, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc8,
	0x08, 0xff, 0xff, 0xff, 0xff, 0xdc, 0x14, 0x08,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x0a, 0xff, 0xff,
	0xff, 0xff, 0x00, 0x0a, 0x78, 0x50, 0xff, 0xff,
	0x00, 0xff, 0xff, 0xff, 0xff, 0xdc, 0x00, 0xff
};

MusicChannelRHY::MusicChannelRHY(PC98AudioCore *pc98a, int part, int regOffset) : SoundChannelNonSSG(pc98a, part, regOffset, 2), _activeInstruments(0) {
	_instrLevel[0] = _instrLevel[1] = _instrLevel[2] = _instrLevel[3] = _instrLevel[4] = _instrLevel[5] = 0xC0;
}

MusicChannelRHY::~MusicChannelRHY() {
}

void MusicChannelRHY::keyOff() {
	debugC(7, kDebugLevelSound, "RHY Channel 0: keyOff() [Ticks: 0x%02x]", _ticksLeft);
	writeDevice(0x10, (_activeInstruments & 0x3F) | 0x80);
}

void MusicChannelRHY::noteOn(uint8 note) {
	if ((_flags2 & kNoSustain) && !_globalBlock)
		writeDevice(0x10, _activeInstruments & 0x3F);
	_note = note;
	debugC(5, kDebugLevelSound, "RHY Channel 0: noteOn() [Note: 0x%02x Ticks: 0x%02x]", _note, _ticksLeft);
}

void MusicChannelRHY::updateVolume() {
	uint8 val = ((MAX<uint8>((uint8)_fadeVolModifier - 1, 239) + 1) << 2) + (_volume & 0x3F);
	writeDevice(0x11, val < 64 ? val : 0);
}

void MusicChannelRHY::op_programChange(uint8 *&data) {
	_activeInstruments = _program = *data++;
}

void MusicChannelRHY::op_setVolume(uint8 *&data) {
	_volume = *data++;
	updateVolume();
	uint8 *p = _instrLevel;
	for (int i = 6; i; --i) {
		*p = (*p & 0xC0) | *data++;
		writeDevice(0x18 - (i - 6), *p++);
	}
}

void MusicChannelRHY::op_setPanPos(uint8 *&data) {
	uint8 val = *data++;
	uint8 offs = val & 0x0F;
	_instrLevel[offs] = (((val << 2) | (val >> 6)) & 0xC0) | (_instrLevel[offs] & 0x1F);
	writeDevice(0x18 + offs, _instrLevel[offs]);
}

void MusicChannelRHY::op_modifyVolume(uint8 *&data) {
	_volume += *data++;
	updateVolume();
}

MusicChannelEXT::MusicChannelEXT(PC98AudioCore *pc98a, int part, int regOffset, MLALF98::ADPCMData *const &data) : SoundChannelNonSSG(pc98a, part, regOffset, 3),
	_pc98a(pc98a), _extBuffer(data), _useVolPreset(0), _volume2(0), _instrument(0), _panPos(3), _smpStart(0), _smpEnd(0) {
}

MusicChannelEXT::~MusicChannelEXT() {
}

void MusicChannelEXT::keyOff() {
	debugC(7, kDebugLevelSound, "EXT Channel 0: keyOff() [Ticks: 0x%02x]", _ticksLeft);
	writeDevice(0x0B, 0x00);
	writeDevice(0x01, 0x00);
	writeDevice(0x00, 0x21);
}

void MusicChannelEXT::noteOn(uint8 note) {
	static uint16 freqTableEXT[12] = {
		0x4A82, 0x4EE4, 0x5389, 0x5875, 0x5DAC, 0x6332, 0x690C, 0x6F3F, 0x75D1, 0x7C76, 0x8426, 0x8BF5
	};

	if (!(_flags2 & kNoSustain) && _note == note)
		return;

	_note = note;

	debugC(5, kDebugLevelSound, "EXT Channel 0: noteOn() [Note: 0x%02x Ticks: 0x%02x]", _note, _ticksLeft);

	assert((note & 0x0F) < 12);
	_frequency = (freqTableEXT[note & 0x0F] + _transpose) >> (note >> 4);

	if (!(_flags2 & kNoSustain))
		vbrResetDelay();
	vbrReset();

	if (_globalBlock)
		return;

	writeDevice(0x0B, 0x00); // vol zero
	writeDevice(0x01, 0x00); // ram type default
	writeDevice(0x00, 0x21); // 0x20: external memory, 0x01: clear busy flag
	writeDevice(0x10, 0x08); // set irq flag mask
	writeDevice(0x10, 0x80); // reset irq flag
	writeDevice(0x02, _smpStart & 0xFF);
	writeDevice(0x03, _smpStart >> 8);
	writeDevice(0x04, _smpEnd & 0xFF);
	writeDevice(0x05, _smpEnd >> 8);
	writeDevice(0x09, _frequency & 0xFF);
	writeDevice(0x0A, _frequency >> 8);
	writeDevice(0x00, 0xA0); // 0x20: external memory, 0x80: start

	uint8 vol = (uint8)(MAX<int8>(_fadeVolModifier, -16) << 2) + _volume;
	if (_volume < vol)
		vol = 0;

	if (_useVolPreset) {
		vol += _volume2;
		if (_volume2 < vol)
			vol = 0;
	}

	writeDevice(0x0B, vol);
	writeDevice(0x01, ((_panPos << 6) | (_panPos >> 2)) & 0xC0);
}

void MusicChannelEXT::updateVibrato() {
	_frequency += _vbrModifier;
	writeDevice(0x09, _frequency & 0xFF);
	writeDevice(0x0A, _frequency >> 8);
}

void MusicChannelEXT::clear() {
	SoundChannel::clear();
	_panPos = 3;
	_useVolPreset = 0;
	_volume2 = 0;
}

void MusicChannelEXT::writeDevice(uint8 reg, uint8 val) {
	_pc98a->writeReg(1, reg, val);
}

void MusicChannelEXT::op_programChange(uint8 *&data) {
	_instrument = *data++;
	_program = _instrument - 1;
	_smpStart = _extBuffer[_program].smpStart;
	_smpEnd = _extBuffer[_program].smpEnd;
	if (_useVolPreset)
		_volume = _extBuffer[_program].volume;
}

void MusicChannelEXT::op_setVolume(uint8 *&data) {
	if (_useVolPreset)
		_volume2 = *data;
	else
		_volume = *data;
	data++;
}

void MusicChannelEXT::op_setPanPos(uint8 *&data) {
	_panPos = *data++;
}

void MusicChannelEXT::op_setTranspose(uint8 *&data) {
	_note = 0;
	int16 trp = READ_LE_INT16(data);
	data += 2;
	_transpose = (*data++) ? _transpose + trp : trp;
	uint16 val = _frequency + _transpose;
	writeDevice(0x09, val & 0xFF);
	writeDevice(0x0A, val >> 8);
}

void MusicChannelEXT::op2_setExtPara(uint8 *&data) {
	_useVolPreset = *data++;
}

SoundEffectChannel::SoundEffectChannel(PC98AudioCore *pc98a, int part, int regOffset, SoundChannel *replaceChannel) : MusicChannelFM(pc98a, part, regOffset),
	_replaceChannel(replaceChannel), _specialMode(false), _isFinished(false) {
	_specialModeModifier[0] = _specialModeModifier[1] = _specialModeModifier[2] = _specialModeModifier[3] = 0;
}

SoundEffectChannel::~SoundEffectChannel() {

}

void SoundEffectChannel::setData(uint8 *dataStart, uint8 *loopStart, const uint8 *dataEnd, uint8 *instrBuffer) {
	_replaceChannel->toggleMute(dataStart);
	SoundChannel::setData(dataStart, loopStart, dataEnd, instrBuffer);
}

bool SoundEffectChannel::checkFinished() {
	if (!_isFinished)
		return false;

	_replaceChannel->toggleMute(false);
	_replaceChannel->restore();

	_isFinished = false;
	return true;
}

void SoundEffectChannel::finish() {
	keyOff();
	_isFinished = true;
}

bool SoundEffectChannel::usingSpecialMode() const {
	return _specialMode;
}

uint8 SoundEffectChannel::getSpecialFrequencyModifier(uint8 index) {
	assert(index < 4);
	return _specialModeModifier[index];
}

void SoundEffectChannel::setSpecialFrequencyModifier(uint8 index, uint8 val) {
	assert(index < 4);
	_specialModeModifier[index] = val;
}

void SoundEffectChannel::toggleSpecialMode(bool on) {
	_specialMode = on;
	uint8 flag = on ? 0x40 : 0;
	writeDevice(0x27, 0x3E + flag);
	writeDevice(0x27, 0x3F + flag);
}

void SoundEffectChannel::clear() {
	SoundChannel::clear();
	_specialMode = false;
	_statusB4 = 0xC0;
}

void SoundEffectChannel::writeDevice(uint8 reg, uint8 val) {
	SoundChannel::writeDevice(reg, val);
}

void SoundEffectChannel::op_writeDevice(uint8 *&data) {
	uint8 reg = *data++;
	uint8 val = *data++;
	if (reg != 0x26)
		writeDevice(reg, val);
	if (reg == 0x25 || reg == 0x26)
		toggleSpecialMode(_specialMode);
}

#define iterateChannels(arr)	for (Common::Array<SoundChannel*>::iterator i = arr.begin(); i != arr.end(); ++i)

MLALF98Internal *MLALF98Internal::_refInstance = 0;
int MLALF98Internal::_refCount = 0;

MLALF98Internal::MLALF98Internal(Audio::Mixer *mixer, EmuType emuType) : PC98AudioPluginDriver(), _type86(emuType == kType86),
	_musicBuffer(0), _sfxBuffer(0), _extBuffer(0), _musicBufferSize(0), _sfxBufferSize(0), _extBufferSize(0), _pc98a(0), _sfxPlaying(0), _ready(false) {
	_pc98a = new PC98AudioCore(mixer, this, emuType);
	assert(_pc98a);

	_extBuffer = new MLALF98::ADPCMData[8];

	for (int i = 0; i < 3; ++i)
		_musicChannels.push_back(new MusicChannelFM(_pc98a, 0, i));
	for (int i = 3; i < 6; ++i)
		_musicChannels.push_back(new MusicChannelSSG(_pc98a, 0, (i - 3) << 1));

	if (_type86) {
		_musicChannels.push_back(new MusicChannelRHY(_pc98a, 0, 0));
		for (int i = 7; i < 10; ++i)
			_musicChannels.push_back(new MusicChannelFM(_pc98a, 1, i - 7));
		_musicChannels.push_back(new MusicChannelEXT(_pc98a, 1, 0, _extBuffer));
	}

	int replChanIndex = 2;
	_sfxChannels.push_back(new SoundEffectChannel(_pc98a, 0, 2, _musicChannels[replChanIndex++]));

	_pc98a->init();

	_ready = true;
}

MLALF98Internal::~MLALF98Internal() {
	_ready = false;

	delete _pc98a;

	iterateChannels(_musicChannels)
		delete (*i);

	iterateChannels(_sfxChannels)
		delete (*i);

	delete[] _musicBuffer;
	delete[] _sfxBuffer;
	delete[] _extBuffer;
}

MLALF98Internal *MLALF98Internal::open(Audio::Mixer *mixer, EmuType emuType) {
	_refCount++;

	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new MLALF98Internal(mixer, emuType);
	else if (_refCount < 2 || _refInstance == 0)
		error("MLALF98Internal::open(): Internal instance management failure");

	return _refInstance;
}

void MLALF98Internal::close() {
	if (!_refCount)
		return;

	_refCount--;

	if (!_refCount) {
		delete _refInstance;
		_refInstance = 0;
	}
}

void MLALF98Internal::loadMusicData(Common::SeekableReadStream *data) {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();

	if (!data)
		error("MLALF98Internal::loadMusicData(): Invalid data.");
	if (data->size() == 0)
		error("MLALF98Internal::loadMusicData(): Invalid data size.");

	iterateChannels(_musicChannels)
		(*i)->setData(0, 0, 0, 0);

	delete[] _musicBuffer;
	_musicBufferSize = data->size();
	_musicBuffer = new uint8[_musicBufferSize];
	data->read(_musicBuffer, _musicBufferSize);
}

void MLALF98Internal::loadSoundEffectData(Common::SeekableReadStream *data) {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();

	if (!data)
		error("MLALF98Internal::loadSoundEffectData(): Invalid data.");
	if (data->size() == 0)
		error("MLALF98Internal::loadSoundEffectData(): Invalid data size.");

	iterateChannels(_sfxChannels)
		(*i)->setData(0, 0, 0, 0);

	delete[] _sfxBuffer;
	_sfxBufferSize = data->size();
	_sfxBuffer = new uint8[_sfxBufferSize];
	data->read(_sfxBuffer, _sfxBufferSize);
}

void MLALF98Internal::loadExtData(MLALF98::ADPCMDataArray &data) {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	if (data.empty())
		error("MLALF98Internal::loadExtData(): Invalid data.");

	delete[] _extBuffer;
	_extBufferSize = data.size();
	_extBuffer = new MLALF98::ADPCMData[_extBufferSize];
	MLALF98::ADPCMData *dst = _extBuffer;
	for (MLALF98::ADPCMDataArray::iterator i = data.begin(); i != data.end(); ++i)
		*dst++ = *i;
 }

void MLALF98Internal::startMusic(int track) {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	iterateChannels(_musicChannels) {
		(*i)->abortFadeOut();
		(*i)->toggleMute(false);
	}

	_sfxPlaying = 0;

	_pc98a->writeReg(0, 0x27, 0x3C);
	_pc98a->writeReg(0, 0x10, 0x80);
	_pc98a->writeReg(0, 0x10, 0x00);
	_pc98a->writeReg(0, 0x24, 0x18);
	_pc98a->writeReg(0, 0x25, 0x02);

	iterateChannels(_sfxChannels)
		(*i)->setData(0, 0, 0, 0);

	iterateChannels(_musicChannels)
		(*i)->keyOff();
	iterateChannels(_sfxChannels)
		(*i)->keyOff();

	assert(track * 45 + 5 < _musicBufferSize);
	const uint8 *header = _musicBuffer + (track * 45) + 5;

	uint8 tempo = *header++;
	iterateChannels(_musicChannels) {
		uint16 offset1 = READ_LE_UINT16(header);
		assert(offset1 + 5 < _musicBufferSize);
		header += 2;
		uint16 offset2 = READ_LE_UINT16(header);
		assert(offset2 + 5 <= _musicBufferSize);
		header += 2;
		(*i)->setData(_musicBuffer + 5 + offset1, offset2 ? _musicBuffer + 5 + offset2 : 0, _musicBuffer + _musicBufferSize, _musicBuffer + 1);
	}

	debugC(3, kDebugLevelSound, "\nStarting music. Track: %03d", track);

	_pc98a->writeReg(0, 0x29, 0x83);
	for (int i = 0; i < 6; ++i)
		_pc98a->writeReg(0, i, 0);
	_pc98a->writeReg(0, 0x07, 0x38);
	_pc98a->writeReg(0, 0x26, tempo);

	for (int i = 0; i < 2; ++i) {
		for (int ii = 0; ii < 3; ++ii)
			_pc98a->writeReg(i, 0xB4 + ii, 0xC0);
	}

	_pc98a->writeReg(0, 0x22, 0x00);
	_pc98a->writeReg(0, 0x27, 0x3F);
}

void MLALF98Internal::fadeOutMusic() {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	iterateChannels(_musicChannels)
		(*i)->startFadeOut();
}

void MLALF98Internal::startSoundEffect(int track) {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	const uint8 *header = _sfxBuffer + (track << 1) + 3;
	uint16 offset = READ_LE_UINT16(header);
	assert(offset < _sfxBufferSize);
	_sfxPlaying = 0;
	int flags = 0;

	iterateChannels(_sfxChannels) {
		(*i)->setData(_sfxBuffer + offset, 0, _sfxBuffer + _sfxBufferSize, _sfxBuffer + 1);
		flags |= (*i)->idFlag();
		_sfxPlaying++;
	}

	debugC(3, kDebugLevelSound, "\nStarting sound effect. Track: %03d", track);

	_pc98a->setSoundEffectChanMask(flags);

	_pc98a->writeReg(0, 0x28, 0x02);
	_pc98a->writeReg(0, 0x24, 0x18);
	_pc98a->writeReg(0, 0x25, 0x02);
	_pc98a->writeReg(0, 0x82, 0x0F);
	_pc98a->writeReg(0, 0x86, 0x0F);
	_pc98a->writeReg(0, 0x8A, 0x0F);
	_pc98a->writeReg(0, 0x8E, 0x0F);
	_pc98a->writeReg(0, 0xB6, 0xC0);
	_pc98a->writeReg(0, 0x27, 0x3F);
}

void MLALF98Internal::allChannelsOff() {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	iterateChannels(_musicChannels)
		(*i)->keyOff();
	iterateChannels(_sfxChannels)
		(*i)->keyOff();
}

void MLALF98Internal::resetExtUnit() {
	PC98AudioCore::MutexLock lock = _pc98a->stackLockMutex();
	_pc98a->writeReg(1, 0x0B, 0x00); // vol zero
	_pc98a->writeReg(1, 0x01, 0x00); // ram type default
	_pc98a->writeReg(1, 0x00, 0x21); // 0x20: external memory, 0x01: clear busy flag
	_pc98a->writeReg(1, 0x10, 0x08); // set irq flag mask
	_pc98a->writeReg(1, 0x10, 0x80); // reset irq flag
	_pc98a->writeReg(1, 0x02, _extBuffer[0].smpStart & 0xFF);
	_pc98a->writeReg(1, 0x03, _extBuffer[0].smpStart >> 8);
	_pc98a->writeReg(1, 0x04, _extBuffer[0].smpEnd & 0xFF);
	_pc98a->writeReg(1, 0x05, _extBuffer[0].smpEnd >> 8);
	_pc98a->writeReg(1, 0x09, 0xCE);
	_pc98a->writeReg(1, 0x0A, 0x49);
	_pc98a->writeReg(1, 0x00, 0xA0); // 0x20: external memory, 0x80: start
	_pc98a->writeReg(1, 0x0B, _extBuffer[0].volume);
	_pc98a->writeReg(1, 0x01, 0xC0);
}

void MLALF98Internal::setMusicVolume(int volume) {
	_pc98a->setMusicVolume(volume);
}

void MLALF98Internal::setSoundEffectVolume(int volume) {
	_pc98a->setSoundEffectVolume(volume);
}

void MLALF98Internal::timerCallbackA() {
	if (!_ready || !_sfxPlaying)
		return;

	iterateChannels(_sfxChannels) {
		(*i)->update();
		if ((*i)->checkFinished()) {
			if (!--_sfxPlaying) {
				_pc98a->setSoundEffectChanMask(0);
				debugC(3, kDebugLevelSound, "Finished sound effect.\n");
			}
		}
	}
	//_updateCounterSfx++;
}

void MLALF98Internal::timerCallbackB() {
	if (!_ready)
		return;
	iterateChannels(_musicChannels)
		(*i)->update();
	iterateChannels(_musicChannels)
		(*i)->updateFadeOut();
	//_updateCounterMusic++;
}

MLALF98::MLALF98(Audio::Mixer *mixer, EmuType emuType) {
	_drv = MLALF98Internal::open(mixer, (PC98AudioPluginDriver::EmuType)emuType);
}

MLALF98::~MLALF98() {
	MLALF98Internal::close();
	_drv = 0;
}

void MLALF98::loadMusicData(Common::SeekableReadStream *data) {
	_drv->loadMusicData(data);
}

void MLALF98::loadSoundEffectData(Common::SeekableReadStream *data) {
	_drv->loadSoundEffectData(data);
}

void MLALF98::loadExtData(ADPCMDataArray &data) {
	_drv->loadExtData(data);
}

void MLALF98::startMusic(int track) {
	_drv->startMusic(track);
}

void MLALF98::fadeOutMusic() {
	_drv->fadeOutMusic();
}

void MLALF98::startSoundEffect(int track) {
	_drv->startSoundEffect(track);
}

void MLALF98::allChannelsOff() {
	_drv->allChannelsOff();
}

void MLALF98::resetExtUnit() {
	_drv->resetExtUnit();
}

void MLALF98::setMusicVolume(int volume) {
	_drv->setMusicVolume(volume);
}

void MLALF98::setSoundEffectVolume(int volume) {
	_drv->setSoundEffectVolume(volume);
}

#undef iterateChannels

} // End of namespace Kyra

#endif
