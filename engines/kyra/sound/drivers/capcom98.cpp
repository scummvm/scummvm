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

#ifdef ENABLE_EOB

#include "kyra/sound/drivers/capcom98.h"
#include "audio/softsynth/fmtowns_pc98/pc98_audio.h"
#include "common/func.h"
#include "common/system.h"

namespace Kyra {

class CapcomPC98Player {
public:
	enum Flags {
		kStdPlay	= 1 << 0,
		kPrioPlay	= 1 << 1,
		kPrioClaim	= 1 << 2,
		kFadeOut	= 1 << 3,
		kStdStop	= 1 << 8,
		kPrioStop	= 1 << 9,
		kSavedState	= 1 << 10
	};

	CapcomPC98Player(bool playerPrio, uint16 playFlags, uint16 chanReserveFlags, uint16 chanDisableFlags);
	virtual ~CapcomPC98Player() {}

	virtual bool init() = 0;
	virtual void deinit() = 0;
	virtual void reset() = 0;
	virtual void loadInstruments(const uint8 *data, uint16 number) {}

	void startSound(const uint8 *data, uint8 volume, bool loop);
	void stopSound();
	uint8 getMarker(uint8 id) const { return _soundMarkers[id & 0x0F]; }

	static uint16 getStatus() { return _flags; }

	void fadeOut(uint16 speed);
	void allNotesOff(uint16 chanFlags = 0xFFFF);
	virtual void setMasterVolume (int vol) = 0;

	void nextTick();
	virtual void processSounds() = 0;

protected:
	uint16 _soundMarkers[16];
	uint8 _fadeState;
	const uint16 _chanReserveFlags;
	const uint16 _chanDisableFlags;

private:
	virtual void send(uint32 evt) = 0;
	virtual PC98AudioCore::MutexLock lockMutex() = 0;
	void storeEvent(uint32 evt);
	void restorePlayer();
	virtual void restoreStateIntern() {}
	uint16 playFlag() const { return _playFlags & (kStdPlay | kPrioPlay); }
	uint16 extraFlag() const { return _playFlags & kPrioClaim; }
	uint16 stopFlag() const { return (_playFlags & (kStdPlay | kPrioPlay)) << 8; }

	const uint8 *_data;
	const uint8 *_curPos;
	uint16 _numEventsTotal;
	uint16 _numEventsLeft;
	uint8 _volume;
	uint16 _midiTicker;

	static uint16 _flags;
	bool _loop;

	uint16 _fadeSpeed;
	uint16 _fadeTicker;

	Common::Array<uint32> _storedEvents;

	const bool _playerPrio;
	const uint16 _playFlags;
};

class CapcomPC98_MIDI final : public CapcomPC98Player {
public:
	typedef Common::Functor0Mem<PC98AudioCore::MutexLock, CapcomPC98AudioDriverInternal> MutexProc;

	CapcomPC98_MIDI(MidiDriver::DeviceHandle dev, bool isMT32, MutexProc &mutexProc);
	~CapcomPC98_MIDI();

	bool init() override;
	void deinit() override {}
	void reset() override;

	void setMasterVolume (int vol) override;

	void processSounds() override;

private:
	void send(uint32 evt) override;
	PC98AudioCore::MutexLock lockMutex() override;

	MidiDriver *_midi;
	const bool _isMT32;
	const uint8 *_programMapping;

	uint8 _chanVolume[16];

	static const uint8 _programMapping_mt32ToGM[128];

	MutexProc &_mproc;
};

class CapcomPC98_FM_Channel {
public:
	CapcomPC98_FM_Channel(uint8 id, PC98AudioCore *&ac, const Common::Array<const uint8*>&instruments, const uint8 &fadeState);
	~CapcomPC98_FM_Channel();

	void reset();

	void keyOff();
	void noteOff(uint8 note);
	void noteOn(uint8 note, uint8 velo);
	void programChange(uint8 prg);
	void pitchBend(uint16 pb);
	void restore();

	void modWheel(uint8 mw);
	void breathControl(uint8 bc);
	void pitchBendSensitivity(uint8 pbs);
	void portamentoTime(uint8 pt);
	void volume(uint8 vol);
	void togglePortamento(uint8 enable);
	void allNotesOff();

	void processSounds();

private:
	typedef Common::Functor0Mem<void, CapcomPC98_FM_Channel> VbrHandlerProc;

	const uint8 _regOffset;
	uint8 _program;
	bool _isPlaying;
	uint8 _note;
	uint8 _carrier;
	uint8 _volume;
	uint8 _velocity;
	uint8 _noteEffCur;
	uint8 _noteEffPrev;
	uint8 _breathControl;
	uint16 _frequency;
	uint8 _modWheel;
	uint16 _pitchBendSensitivity;
	int16 _pitchBendPara;
	int16 _pitchBendEff;
	int32 _vbrState;
	uint32 _vbrStep;
	uint16 _vbrCycleTicker;
	uint16 _vbrDelayTicker;
	VbrHandlerProc *_vbrHandler;
	bool _prtEnable;
	uint8 _prtTime;
	int32 _prtState;
	int32 _prtStep;
	uint16 _prtCycleLength;

	struct Instrument {
		char name[9];
		uint8 fbl_alg;
		uint8 vbrType;
		uint8 vbrCycleLength;
		uint8 vbrSensitivity;
		uint8 resetEffect;
		uint8 vbrDelay;
		/*uint8 ff;
		uint8 f10;
		uint8 f11;
		uint8 f12;
		uint8 f13;
		uint8 f14;*/
		uint8 *regData;
	} _instrument;

private:
	void loadInstrument(const uint8 *in);
	void updatePitchBend();
	void updateVolume();
	void updatePortamento();
	void updateFrequency();
	void setupPortamento();
	void setupVibrato();

	void dummyProc() {}
	void vbrHandler0();
	void vbrHandler1();
	void vbrHandler2();
	void vbrHandler3();

	VbrHandlerProc *_vbrHandlers[5];
	PC98AudioCore *&_ac;
	const Common::Array<const uint8*>&_instruments;

	const uint8 &_fadeState;

	static const uint16 _freqMSBTable[12];
	static const uint8 _freqLSBTables[12][64];
	static const uint8 _volTablesInst[4][128];
	static const uint8 _volTableCarrier[128];
	static const uint8 _volTablePara[128];
};

class CapcomPC98_FM final : public CapcomPC98Player, PC98AudioPluginDriver {
public:
	typedef Common::Functor0Mem<void, CapcomPC98AudioDriverInternal> CBProc;

	CapcomPC98_FM(Audio::Mixer *mixer, CBProc &cbproc, bool playerPrio, uint16 playFlags, uint8 chanReserveFlags, uint8 chanDisableFlags, bool needsTimer);
	~CapcomPC98_FM() override;

	bool init() override;
	void deinit() override;
	void reset() override;
	void loadInstruments(const uint8 *data, uint16 number) override;

	void setMasterVolume (int vol) override;

	PC98AudioCore::MutexLock lockMutex() override;

private:
	void send(uint32 evt) override;
	void timerCallbackA() override;
	void processSounds() override;

	void controlChange(uint8 ch, uint8 control, uint8 val);

	void restoreStateIntern() override;

	PC98AudioCore *_ac;
	CapcomPC98_FM_Channel **_chan;
	Common::Array<const uint8*>_instruments;

	CBProc &_cbProc;

	bool _ready;

	static const uint8 _initData[72];
};

class CapcomPC98AudioDriverInternal {
public:
	CapcomPC98AudioDriverInternal(Audio::Mixer *mixer, MidiDriver::DeviceHandle dev);
	~CapcomPC98AudioDriverInternal();

	static CapcomPC98AudioDriverInternal *open(Audio::Mixer *mixer, MidiDriver::DeviceHandle dev);
	static void close();

	bool isUsable() const { return _ready; }

	void reset();
	void loadFMInstruments(const uint8 *data);
	void startSong(const uint8 *data, uint8 volume, bool loop);
	void stopSong();
	void startSoundEffect(const uint8 *data, uint8 volume);
	void stopSoundEffect();

	int checkSoundMarker() const;
	bool songIsPlaying() const;
	bool soundEffectIsPlaying() const;

	void fadeOut();
	void allNotesOff();

	void setMusicVolume(int volume);
	void setSoundEffectVolume(int volume);

	void timerCallback();
	PC98AudioCore::MutexLock lockMutex();

private:
	void updateMasterVolume();

	CapcomPC98Player *_players[2];
	CapcomPC98_FM *_fmDevice;
	CapcomPC98_FM::CBProc *_timerProc;
	CapcomPC98_MIDI::MutexProc *_mutexProc;

	static CapcomPC98AudioDriverInternal *_refInstance;
	static int _refCount;

	int _musicVolume;
	int _sfxVolume;

	int _marker;
	bool _ready;
};

uint16 CapcomPC98Player::_flags = 0;

CapcomPC98Player::CapcomPC98Player(bool playerPrio, uint16 playFlags, uint16 chanReserveFlags, uint16 chanDisableFlags) : _playerPrio(playerPrio), _playFlags(playFlags), _chanReserveFlags(chanReserveFlags), _chanDisableFlags(chanDisableFlags),
	_data(nullptr), _curPos(nullptr), _numEventsTotal(0), _numEventsLeft(0), _volume(0), _midiTicker(0), _loop(false), _fadeState(0), _fadeSpeed(1), _fadeTicker(0) {
	memset(_soundMarkers, 0, sizeof(_soundMarkers));
	_flags = 0;
}

void CapcomPC98Player::startSound(const uint8 *data, uint8 volume, bool loop) {
	stopSound();

	PC98AudioCore::MutexLock lock = lockMutex();
	_numEventsTotal = _numEventsLeft = READ_LE_UINT16(data);
	_data = _curPos = data + 2;
	_volume = volume & 0x7F;
	_loop = loop;
	_midiTicker = 0;

	for (int i = 0; i < 16; ++i) {
		if ((1 << i) & _chanReserveFlags)
			send(0x0007B0 | i | (_volume << 16));
	}

	_flags &= ~(stopFlag() | kFadeOut);
	_flags |= (playFlag() | extraFlag());
}

void CapcomPC98Player::stopSound() {
	while (_flags & playFlag()) {
		g_system->delayMillis(4);
		PC98AudioCore::MutexLock lock = lockMutex();
		_flags |= stopFlag();
	}
	g_system->delayMillis(8);
}

void CapcomPC98Player::fadeOut(uint16 speed) {
	if (speed) {
		_fadeTicker =_fadeSpeed = speed;
		_fadeState = 0;
		_flags |= kFadeOut;
	} else {
		stopSound();
	}
}

void CapcomPC98Player::allNotesOff(uint16 chanFlags) {
	for (int i = 0; i < 16; ++i) {
		if ((1 << i) & chanFlags)
			send(0x007BB0 | i);
	}
}

void CapcomPC98Player::nextTick() {
	if (_flags & playFlag()) {
		if (_flags & kFadeOut) {
			if (_fadeTicker) {
				--_fadeTicker;
			} else {
				_fadeTicker = _fadeSpeed;
				if (++_fadeState == 100)
					_flags |= stopFlag();
			}
		} else {
			_fadeState = 0;
			_fadeTicker = _fadeSpeed;
		}

		if (!_playerPrio) {
			if (_flags & kPrioClaim) {
				_flags &= ~kPrioClaim;
				_flags |= kSavedState;
				allNotesOff(~_chanReserveFlags);
			} else if (((_flags & kPrioStop) || !(_flags & kPrioPlay)) && (_flags & kSavedState)) {
				_flags &= ~kSavedState;
				restorePlayer();
			}
		}

		if (_flags & stopFlag()) {
			allNotesOff((_playerPrio || (_flags & kPrioPlay)) ? _chanReserveFlags : 0xFFFF);
			_flags &= ~playFlag();
			if (!(_flags & (kStdPlay | kPrioPlay)))
				_flags &= ~kFadeOut;

		} else if (_numEventsLeft) {
			bool eot = false;

			while (_numEventsLeft && !eot) {
				eot = false;

				uint32 in = READ_LE_UINT32(_curPos);
				if ((in & 0xFF) > _midiTicker)
					break;

				_midiTicker -= (in & 0xFF);
				uint8 chanFlag = 1 << ((in >> 8) & 0x0F);

				if (!(chanFlag & _chanDisableFlags)) {
					if (_playerPrio || !(_flags & kPrioPlay) || ((_flags & kPrioPlay) && (chanFlag & _chanReserveFlags))) {
						if (_volume == 0x7F || ((in >> 12) & 0xFFF) != 0x07B)
							send(in >> 8);
					} else {
						storeEvent(in >> 8);
					}
				}

				_curPos += 4;
				eot = (--_numEventsLeft == 0);
			}

			if (eot) {
				if (_loop) {
					_numEventsLeft = _numEventsTotal;
					_curPos = _data;
				} else {
					allNotesOff((_playerPrio || (_flags & kPrioPlay)) ? _chanReserveFlags : 0xFFFF);
					_flags &= ~playFlag();
				}
			}
		}
	}

	processSounds();

	_midiTicker++;
}

void CapcomPC98Player::storeEvent(uint32 evt) {
	if ((1 << (evt & 0x0F)) & _chanReserveFlags)
		return;

	uint8 st = evt & 0xFF;

	for (Common::Array<uint32>::iterator i = _storedEvents.begin(); i != _storedEvents.end(); ++i) {
		if ((*i & 0xFF) == st) {
			*i = evt;
			return;
		}
	}

	st >>= 4;

	if (st == 0x0B || st == 0x0C || st == 0x0E)
		_storedEvents.push_back(evt);
}

void CapcomPC98Player::restorePlayer() {
	restoreStateIntern();
	for (Common::Array<uint32>::iterator i = _storedEvents.begin(); i != _storedEvents.end(); ++i)
		send(*i);
	_storedEvents.clear();
}

CapcomPC98_MIDI::CapcomPC98_MIDI(MidiDriver::DeviceHandle dev, bool isMT32, MutexProc &mutexProc) : CapcomPC98Player(true, kStdPlay, 0xFFFF, 0), _isMT32(isMT32), _mproc(mutexProc), _midi(nullptr), _programMapping(nullptr) {
	_midi = MidiDriver::createMidi(dev);
	uint8 *map = new uint8[128];
	assert(map);

	if (isMT32) {		
		memcpy(map, _programMapping_mt32ToGM, 128);
	} else {
		for (uint8 i = 0; i < 128; ++i)
			map[i] = i;
	}

	_programMapping = map;
	memset(_chanVolume, 0, sizeof(_chanVolume));
}

CapcomPC98_MIDI::~CapcomPC98_MIDI() {
	_midi->close();
	delete _midi;
	delete[] _programMapping;
}

bool CapcomPC98_MIDI::init() {
	if (!_midi || !_programMapping)
		return false;

	if (_midi->open())
		return false;

	if (_isMT32) {
		_midi->sendMT32Reset();
	} else {
		static const byte gmResetSysEx[] = { 0x7E, 0x7F, 0x09, 0x01 };
		_midi->sysEx(gmResetSysEx, sizeof(gmResetSysEx));
		g_system->delayMillis(100);
	}

	reset();

	return true;
}

void CapcomPC98_MIDI::reset() {
	memset(_chanVolume, 0x7F, sizeof(_chanVolume));
}

void CapcomPC98_MIDI::send(uint32 evt) {
	uint8 cmd = evt & 0xF0;
	uint8 ch = evt & 0x0F;
	uint8 p1 = (evt >> 8) & 0xFF;
	uint8 p2 = (evt >> 16) & 0xFF;

	if (cmd == 0xC0) {
		evt = (evt & 0xFF) | (_programMapping[p1] << 8);
	} else if (cmd == 0xB0) {
		if (p1 == 3) {
			_soundMarkers[ch] = p2;
			return;
		} else if (((evt >> 8) & 0xFF) == 7) {
			_chanVolume[ch] = p2;
		}
	}
	_midi->send(evt);
}

void CapcomPC98_MIDI::setMasterVolume (int vol) {

}

void CapcomPC98_MIDI::processSounds() {
	if (_fadeState) {
		for (int i = 0; i < 16; ++i)
			_midi->send(0x0007B0 | i | (CLIP<int>(_chanVolume[i] - _fadeState, 0, 127) << 16));
	}
}

PC98AudioCore::MutexLock CapcomPC98_MIDI::lockMutex() {
	if (!_mproc.isValid())
		error("CapcomPC98_MIDI::lockMutex(): Invalid call");
	return _mproc();
}

// This is not identical to the one we have in our common code (not even similar).
const uint8 CapcomPC98_MIDI::_programMapping_mt32ToGM[128] = {
	0x00, 0x02, 0x01, 0x03, 0x04, 0x05, 0x10, 0x13, 0x16, 0x65, 0x0a, 0x00, 0x68, 0x67, 0x2e, 0x25,
	0x08, 0x09, 0x0a, 0x0c, 0x0d, 0x0e, 0x57, 0x38, 0x3b, 0x3c, 0x3d, 0x3e, 0x3b, 0x3b, 0x3b, 0x1f,
	0x3d, 0x1c, 0x1c, 0x1c, 0x1c, 0x1e, 0x1f, 0x1f, 0x35, 0x38, 0x37, 0x38, 0x36, 0x33, 0x39, 0x70,
	0x30, 0x30, 0x31, 0x22, 0x22, 0x22, 0x22, 0x7a, 0x58, 0x5a, 0x5e, 0x59, 0x5b, 0x60, 0x60, 0x1a,
	0x51, 0x4f, 0x4e, 0x50, 0x54, 0x55, 0x56, 0x52, 0x4a, 0x49, 0x4c, 0x4d, 0x6e, 0x6b, 0x6d, 0x6c,
	0x2f, 0x2f, 0x5e, 0x52, 0x57, 0x22, 0x56, 0x38, 0x20, 0x24, 0x5d, 0x22, 0x21, 0x5d, 0x4d, 0x5d,
	0x29, 0x24, 0x66, 0x39, 0x22, 0x65, 0x22, 0x5c, 0x57, 0x69, 0x6a, 0x69, 0x6c, 0x6d, 0x0f, 0x35,
	0x70, 0x71, 0x72, 0x76, 0x75, 0x74, 0x73, 0x77, 0x78, 0x79, 0x7a, 0x7c, 0x7b, 0x7d, 0x7e, 0x7f
};

CapcomPC98_FM_Channel::CapcomPC98_FM_Channel(uint8 id, PC98AudioCore *&ac, const Common::Array<const uint8*>&instruments, const uint8 &fadeState) : _regOffset(id), _ac(ac), _instruments(instruments),
	_isPlaying(false), _note(0), _carrier(0), _volume(0), _velocity(0), _noteEffCur(0), _program(0), _noteEffPrev(0), _breathControl(0), _frequency(0), _modWheel(0), _pitchBendSensitivity(0),
	_pitchBendPara(0), _pitchBendEff(0), _vbrState(0), _vbrStep(0), _vbrCycleTicker(0), _vbrDelayTicker(0), _vbrHandler(nullptr), _prtEnable(false),
	_prtTime(0), _prtState(0), _prtStep(0), _prtCycleLength(0), _fadeState(fadeState) {
	typedef void (CapcomPC98_FM_Channel::*Proc)();
	static const Proc procs[] = {
		&CapcomPC98_FM_Channel::vbrHandler0,
		&CapcomPC98_FM_Channel::vbrHandler1,
		&CapcomPC98_FM_Channel::vbrHandler2,
		&CapcomPC98_FM_Channel::vbrHandler3,
		&CapcomPC98_FM_Channel::dummyProc
	};

	assert(ARRAYSIZE(_vbrHandlers) == ARRAYSIZE(procs));
	for (int i = 0; i < ARRAYSIZE(_vbrHandlers); ++i) {
		_vbrHandlers[i] = new VbrHandlerProc(this, procs[i]);
		assert(_vbrHandlers[i]);
	}

	memset(&_instrument, 0, sizeof(_instrument));
	_instrument.regData = new uint8[52];
	memset(_instrument.regData, 0, 52);

	reset();
}

CapcomPC98_FM_Channel::~CapcomPC98_FM_Channel() {
	for (int i = 0; i < ARRAYSIZE(_vbrHandlers); ++i)
		delete _vbrHandlers[i];

	delete[] _instrument.regData;
}

void CapcomPC98_FM_Channel::reset() {
	_vbrHandler = _vbrHandlers[4];
	_frequency = 0xFFFF;
	_vbrState = 0;
	_prtState = 0;
	_prtCycleLength = 0;
	_prtEnable = false;
	_pitchBendSensitivity = 3072;
	_pitchBendEff = 0;
	_pitchBendPara = 0;
	_isPlaying = false;
	_note = 0xFF;
	_volume = 0x7F;
	_velocity = _breathControl = 0x40;
	_noteEffCur = 60;
	_modWheel = 0;
}

void CapcomPC98_FM_Channel::keyOff() {
	_ac->writeReg(0, 0x28, _regOffset);
}

void CapcomPC98_FM_Channel::noteOff(uint8 note) {
	if (!_isPlaying || _note != note)
		return;
	keyOff();
	_isPlaying = false;
}

void CapcomPC98_FM_Channel::noteOn(uint8 note, uint8 velo) {
	_noteEffPrev = _noteEffCur;
	_note = note;
	_velocity = velo;

	if (note > 11)
		note -= 12;
	if (note > 127)
		note += 12;

	_noteEffCur = note;

	if (!_isPlaying && _instrument.resetEffect)
		setupVibrato();

	setupPortamento();
	updateFrequency();

	if (!_isPlaying) {
		updateVolume();
		_ac->writeReg(0, 0x28, _regOffset | 0xF0);
	}

	_isPlaying = true;
}

void CapcomPC98_FM_Channel::programChange(uint8 prg) {
	if (prg >= _instruments.size())
		return;

	_program = prg;
	loadInstrument(_instruments[prg]);

	_ac->writeReg(0, 0xB0 + _regOffset, _instrument.fbl_alg);

	static const uint8 carriers[] = { 0x08, 0x08, 0x08, 0x08, 0x0C, 0x0E, 0x0E, 0x0F };
	_carrier = carriers[_instrument.fbl_alg & 7];

	const uint8 *s = _instrument.regData;
	for (int i = _regOffset; i < 16 + _regOffset; i += 4) {
		_ac->writeReg(0, 0x30 + i, s[0]);
		_ac->writeReg(0, 0x50 + i, s[2]);
		_ac->writeReg(0, 0x60 + i, s[3]);
		_ac->writeReg(0, 0x70 + i, s[4]);
		_ac->writeReg(0, 0x80 + i, s[5]);
		_ac->writeReg(0, 0x90 + i, s[6]);
		s += 13;
	}

	setupVibrato();
}

void CapcomPC98_FM_Channel::pitchBend(uint16 pb) {
	_pitchBendPara = (pb - 0x2000) << 2;
	updatePitchBend();
}

void CapcomPC98_FM_Channel::restore() {
	programChange(_program);
	_frequency = 0xFFFF;
}

void CapcomPC98_FM_Channel::modWheel(uint8 mw) {
	_modWheel = mw;
}

void CapcomPC98_FM_Channel::breathControl(uint8 bc) {
	_breathControl = bc;
	updateVolume();
}

void CapcomPC98_FM_Channel::pitchBendSensitivity(uint8 pbs) {
	_pitchBendSensitivity = pbs;
	pitchBend(_pitchBendPara);
}

void CapcomPC98_FM_Channel::portamentoTime(uint8 pt) {
	_prtTime = pt;
}

void CapcomPC98_FM_Channel::volume(uint8 vol) {
	_volume = vol;
	if (_isPlaying)
		updateVolume();
}

void CapcomPC98_FM_Channel::togglePortamento(uint8 enable) {
	_prtEnable = (enable >= 0x40);
}

void CapcomPC98_FM_Channel::allNotesOff() {
	_isPlaying = false;
	for (int i = _regOffset; i < 16 + _regOffset; i += 4) {
		_ac->writeReg(0, 0x40 + i, 0x7F);
		_ac->writeReg(0, 0x80 + i, 0xFF);
	}
	_ac->writeReg(0, 0x28, _regOffset);
}

void CapcomPC98_FM_Channel::processSounds() {
	if (!_isPlaying)
		return;

	if (_vbrHandler->isValid())
		(*_vbrHandler)();
	updatePortamento();
	updateFrequency();

	if (_fadeState)
		updateVolume();
}

void CapcomPC98_FM_Channel::loadInstrument(const uint8 *in) {
	memcpy(_instrument.name, in, 8);
	in += 8;
	_instrument.fbl_alg = *in++;
	_instrument.vbrType = *in++;
	_instrument.vbrCycleLength = *in++;
	_instrument.vbrSensitivity = *in++;
	_instrument.resetEffect = *in++;
	_instrument.vbrDelay = *in++;
	/*_instrument.ff = **/in++;
	/*_instrument.f10 = **/in++;
	/*_instrument.f11 = **/in++;
	/*_instrument.f12 = **/in++;
	/*_instrument.f13 = **/in++;
	/*_instrument.f14 = **/in++;
	assert(_instrument.regData);
	memcpy(_instrument.regData, in, 52);
}

void CapcomPC98_FM_Channel::updatePitchBend() {
	_pitchBendEff = (_pitchBendPara * (_pitchBendSensitivity << 1)) >> 16;
	updateFrequency();
}

void CapcomPC98_FM_Channel::updateVolume() {
	uint8 cr = _carrier;
	const uint8 *s = _instrument.regData;
	for (int i = _regOffset; i < 16 + _regOffset; i += 4) {
		uint16 vol = 0;
		if (cr & 1) {
			vol += _volTableCarrier[_volume];
			vol += _fadeState;
		}

		uint8 a = _breathControl;
		uint8 b = s[10];
		if (b & 0x80) {
			a = ~a & 0x7F;
			b &= 0x7F;
		}
		vol += (((_volTablePara[a] * b) & 0x7FFF) >> 7);

		a = _velocity;
		b = s[7];
		if (b & 0x80) {
			a = ~a & 0x7F;
			b &= 0x7F;
		}
		vol += (((_volTablePara[a] * b) & 0x7FFF) >> 7);

		a = _volTablesInst[s[8] & 3][_noteEffCur];
		b = s[9];
		if (b & 0x80) {
			a = ~a & 0x7F;
			b &= 0x7F;
		}
		vol += (((a * b) & 0x7FFF) >> 7);
		vol += s[1];

		if (vol > 0x7f)
			vol=vol;

		vol = MIN<uint16>(vol, 0x7F);

		_ac->writeReg(0, 0x40 + i, vol & 0xFF);
		s += 13;
		cr >>= 1;
	}
}

void CapcomPC98_FM_Channel::updatePortamento() {
	if (_prtCycleLength) {
		_prtCycleLength--;
		_prtState += _prtStep;
	} else {
		_prtState = 0;
	}
}

void CapcomPC98_FM_Channel::updateFrequency() {
	int16 tone = (MIN<int16>(_modWheel + _instrument.vbrSensitivity, 255) * (_vbrState >> 16)) >> 8;
	tone = CLIP<int16>(tone + (_noteEffCur << 8), 0, 0x5FFF);
	tone = CLIP<int16>(tone + _pitchBendEff, 0, 0x5FFF);
	tone = CLIP<int16>(tone + (_prtState >> 16), 0, 0x5FFF);

	uint8 block = ((tone >> 8) / 12) & 7;
	uint8 msb = (tone >> 8) % 12;
	uint8 lsb = (tone & 0xFF) >> 2;

	uint16 freq = (block << 11) + _freqMSBTable[msb] + _freqLSBTables[msb][lsb];
	if (_frequency == freq)
		return;

	_frequency = freq;
	_ac->writeReg(0, 0xA4 + _regOffset, freq >> 8);
	_ac->writeReg(0, 0xA0 + _regOffset, freq & 0xFF);
}

void CapcomPC98_FM_Channel::setupPortamento() {
	if (!_prtTime || !_prtEnable) {
		_prtCycleLength = 0;
		_prtState = 0;
		return;
	}

	int16 diff = (_noteEffCur << 8) - CLIP<int16>((_prtState >> 16) | (_noteEffPrev << 8), 0, 0x5FFF);
	_prtCycleLength = _prtTime;
	_prtStep = (diff << 16) / _prtTime;
	_prtState = (-diff << 16);
}

void CapcomPC98_FM_Channel::setupVibrato() {
	_vbrHandler = _vbrHandlers[4];

	if (_instrument.vbrCycleLength == 0 || _instrument.vbrType > 4)
		return;

	_vbrDelayTicker = _instrument.vbrDelay;

	switch (_instrument.vbrType) {
	case 0:
	case 4:
		_vbrStep = ((_instrument.vbrType == 4 ? 512 : 3072) << 16) / _instrument.vbrCycleLength;
		_vbrCycleTicker = 0;
		_vbrState = 0;
		_vbrHandler = _vbrHandlers[0];
		break;

	case 1:
		_vbrState = _vbrDelayTicker ? 0 : 3072 << 16;
		_vbrCycleTicker = 0;
		_vbrHandler = _vbrHandlers[1];
		break;

	case 2:
	case 3:
		_vbrStep = (6144 << 16) / _instrument.vbrCycleLength;
		_vbrState = _vbrDelayTicker ? 0 : (_instrument.vbrType == 2 ? -3072 : 3072) << 16;
		_vbrCycleTicker = _instrument.vbrCycleLength - 1;
		_vbrHandler = _vbrHandlers[_instrument.vbrType];
		
		break;

	default:
		break;
	}
}

void CapcomPC98_FM_Channel::vbrHandler0() {
	if (_vbrDelayTicker) {
		_vbrDelayTicker--;
		return;
	}

	if ((_vbrCycleTicker < _instrument.vbrCycleLength) || (_vbrCycleTicker >= _instrument.vbrCycleLength * 3))
		_vbrState += _vbrStep;
	else
		_vbrState -= _vbrStep;

	if (++_vbrCycleTicker >= _instrument.vbrCycleLength * 4) {
		_vbrCycleTicker = 0;
		_vbrState = 0;
	}
}

void CapcomPC98_FM_Channel::vbrHandler1() {
	if (_vbrDelayTicker) {
		_vbrDelayTicker--;
		return;
	}

	_vbrState = ((_vbrCycleTicker >= _instrument.vbrCycleLength) ? -3072 : 3072) << 16;

	if (++_vbrCycleTicker >= (_instrument.vbrCycleLength << 1))
		_vbrCycleTicker = 0;
}

void CapcomPC98_FM_Channel::vbrHandler2() {
	if (_vbrDelayTicker) {
		_vbrDelayTicker--;
		return;
	}

	_vbrState += _vbrStep;

	if (++_vbrCycleTicker >= _instrument.vbrCycleLength) {
		_vbrCycleTicker = 0;
		_vbrState = -(3072 << 16);
	}
}

void CapcomPC98_FM_Channel::vbrHandler3() {
	if (_vbrDelayTicker) {
		_vbrDelayTicker--;
		return;
	}

	_vbrState -= _vbrStep;

	if (++_vbrCycleTicker >= _instrument.vbrCycleLength) {
		_vbrCycleTicker = 0;
		_vbrState = 3072 << 16;
	}
}

const uint16 CapcomPC98_FM_Channel::_freqMSBTable[] = {
	0x026a, 0x028f, 0x02b6, 0x02df, 0x030b, 0x0339, 0x036a, 0x039e, 0x03d5, 0x0410, 0x044e, 0x048f
};

const uint8 CapcomPC98_FM_Channel::_freqLSBTables[12][64] = {
	{
		0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09,
		0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x11, 0x12,
		0x12, 0x13, 0x14, 0x14, 0x15, 0x15, 0x16, 0x16, 0x17, 0x18, 0x18, 0x19, 0x19, 0x1a, 0x1a, 0x1b,
		0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 0x1f, 0x1f, 0x20, 0x20, 0x21, 0x21, 0x22, 0x23, 0x23, 0x24, 0x24
	},
	{
		0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x08, 0x09,
		0x0a, 0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x11, 0x12, 0x13,
		0x13, 0x14, 0x14, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x19, 0x19, 0x1a, 0x1b, 0x1b, 0x1c, 0x1c,
		0x1d, 0x1e, 0x1e, 0x1f, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x26, 0x26
	},
	{
		0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x09,
		0x0a, 0x0b, 0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14,
		0x14, 0x15, 0x16, 0x16, 0x17, 0x18, 0x18, 0x19, 0x19, 0x1a, 0x1b, 0x1b, 0x1c, 0x1d, 0x1d, 0x1e,
		0x1f, 0x1f, 0x20, 0x21, 0x21, 0x22, 0x23, 0x23, 0x24, 0x25, 0x25, 0x26, 0x27, 0x27, 0x28, 0x29
	},
	{
		0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0a,
		0x0b, 0x0c, 0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x14, 0x15,
		0x16, 0x16, 0x17, 0x18, 0x18, 0x19, 0x1a, 0x1b, 0x1b, 0x1c, 0x1d, 0x1d, 0x1e, 0x1f, 0x1f, 0x20,
		0x21, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x26, 0x27, 0x28, 0x28, 0x29, 0x2a, 0x2b, 0x2b
	},
	{
		0x00, 0x01, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b,
		0x0b, 0x0c, 0x0d, 0x0d, 0x0e, 0x0f, 0x10, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16,
		0x17, 0x17, 0x18, 0x19, 0x1a, 0x1a, 0x1b, 0x1c, 0x1d, 0x1d, 0x1e, 0x1f, 0x1f, 0x20, 0x21, 0x22,
		0x22, 0x23, 0x24, 0x25, 0x25, 0x26, 0x27, 0x28, 0x28, 0x29, 0x2a, 0x2b, 0x2b, 0x2c, 0x2d, 0x2e
	},
	{
		0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
		0x0c, 0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x12, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17, 0x18,
		0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1f, 0x1f, 0x20, 0x21, 0x22, 0x22, 0x23, 0x24,
		0x25, 0x26, 0x26, 0x27, 0x28, 0x29, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2d, 0x2e, 0x2f, 0x30, 0x31
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x06, 0x07, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0b, 0x0c,
		0x0d, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x18, 0x19,
		0x1a, 0x1b, 0x1c, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x25, 0x26,
		0x27, 0x28, 0x29, 0x2a, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x33
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0a, 0x0b, 0x0c, 0x0d,
		0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
		0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x28,
		0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
		0x0f, 0x10, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1a, 0x1b, 0x1c,
		0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
		0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
		0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
		0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
		0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40
	},
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
		0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
		0x22, 0x23, 0x24, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x32, 0x33,
		0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44
	}
};

const uint8 CapcomPC98_FM_Channel::_volTablesInst[4][128] = {
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x05, 0x0a, 0x0f, 0x14, 0x19, 0x1e, 0x23, 0x28, 0x2d, 0x32, 0x37, 0x3c,
		0x41, 0x46, 0x4b, 0x50, 0x55, 0x5a, 0x5f, 0x64, 0x69, 0x6e, 0x73, 0x78, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
		0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f
	},
	{
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f,
		0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f, 0x00, 0x7f
	}
};

const uint8 CapcomPC98_FM_Channel::_volTableCarrier[] = {
	0x2a, 0x2a, 0x29, 0x29, 0x29, 0x28, 0x28, 0x28, 0x27, 0x27, 0x27, 0x26, 0x26, 0x26, 0x25, 0x25,
	0x25, 0x24, 0x24, 0x24, 0x23, 0x23, 0x23, 0x22, 0x22, 0x22, 0x21, 0x21, 0x21, 0x20, 0x20, 0x20,
	0x1f, 0x1f, 0x1f, 0x1e, 0x1e, 0x1e, 0x1d, 0x1d, 0x1d, 0x1c, 0x1c, 0x1c, 0x1b, 0x1b, 0x1b, 0x1a,
	0x1a, 0x1a, 0x19, 0x19, 0x19, 0x18, 0x18, 0x18, 0x17, 0x17, 0x17, 0x16, 0x16, 0x16, 0x15, 0x15,
	0x15, 0x14, 0x14, 0x14, 0x13, 0x13, 0x13, 0x12, 0x12, 0x12, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10,
	0x0f, 0x0f, 0x0f, 0x0e, 0x0e, 0x0e, 0x0d, 0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0b, 0x0b, 0x0b, 0x0a,
	0x0a, 0x0a, 0x09, 0x09, 0x09, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07, 0x06, 0x06, 0x06, 0x05, 0x05,
	0x05, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00
};

const uint8 CapcomPC98_FM_Channel::_volTablePara[] = {
	0x7f, 0x7e, 0x7d, 0x7c, 0x7b, 0x7a, 0x79, 0x78, 0x77, 0x76, 0x75, 0x74, 0x73, 0x72, 0x71, 0x70,
	0x6f, 0x6e, 0x6d, 0x6c, 0x6b, 0x6a, 0x69, 0x68, 0x67, 0x66, 0x65, 0x64, 0x63, 0x62, 0x61, 0x60,
	0x5f, 0x5e, 0x5d, 0x5c, 0x5b, 0x5a, 0x59, 0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51, 0x50,
	0x4f, 0x4e, 0x4d, 0x4c, 0x4b, 0x4a, 0x49, 0x48, 0x47, 0x46, 0x45, 0x44, 0x43, 0x42, 0x41, 0x40,
	0x3f, 0x3e, 0x3d, 0x3c, 0x3b, 0x3a, 0x39, 0x38, 0x37, 0x36, 0x35, 0x34, 0x33, 0x32, 0x31, 0x30,
	0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20,
	0x1f, 0x1e, 0x1d, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11, 0x10,
	0x0f, 0x0e, 0x0d, 0x0c, 0x0b, 0x0a, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00
};

CapcomPC98_FM::CapcomPC98_FM(Audio::Mixer *mixer, CBProc &proc, bool playerPrio, uint16 playFlags, uint8 chanReserveFlags, uint8 chanDisableFlags, bool needsTimer) : CapcomPC98Player(playerPrio, playFlags, chanReserveFlags, chanDisableFlags), PC98AudioPluginDriver(), _cbProc(proc), _ac(nullptr), _chan(nullptr), _ready(false) {
	_ac = new PC98AudioCore(mixer, needsTimer ? this : nullptr, kType26);
	assert(_ac);
	_chan = new CapcomPC98_FM_Channel*[3];
	assert(_chan);
	for (int i = 0; i < 3; ++i)
		_chan[i] = new CapcomPC98_FM_Channel(i, _ac, _instruments, _fadeState);
}

CapcomPC98_FM::~CapcomPC98_FM() {
	delete _ac;
	if (_chan) {
		for (int i = 0; i < 3; ++i)
			delete _chan[i];
		delete[] _chan;
	}
}

bool CapcomPC98_FM::init() {
	if (!(_chan && _ac && _ac->init()))
		return false;

	_ac->writeReg(0, 7, 0xBF);
	for (int i = 0; i < 14; ++i) {
		if (i != 7)
			_ac->writeReg(0, i, 0);
	}

	static const uint8 iniData[] = { 0x00, 0x7F, 0x1F, 0x1F, 0x1F, 0xFF, 0x00 };
	for (int i = 0; i < 7; ++i) {
		for (int ii = 0; ii < 16; ++ii) {
			if ((ii & 3) != 3)
				_ac->writeReg(0, 0x30 + (i << 4) + ii, iniData[i]);
		}
	}

	for (int i = 0; i < 3; ++i)
		_ac->writeReg(0, 0xB0 + i, 0xC0);

	_ac->writeReg(0, 0x24, 0x91);
	_ac->writeReg(0, 0x25, 0x00);
	_ac->writeReg(0, 0x27, 0x15);

	loadInstruments(_initData, 1);

	reset();

	_ready = true;

	return true;
}

void CapcomPC98_FM::deinit() {
	PC98AudioCore::MutexLock lock = _ac->stackLockMutex();
	_ready = false;
}

void CapcomPC98_FM::reset() {
	for (int i = 0; i < 3; ++i)
		_chan[i]->reset();

	for (int i = 0; i < 3; ++i)
		_chan[i]->keyOff();

	for (int i = 0; i < 3; ++i)
		_chan[i]->programChange(0);
}

void CapcomPC98_FM::loadInstruments(const uint8 *data, uint16 number) {
	_instruments.clear();
	while (number--) {
		_instruments.push_back(data);
		data += 72;
	}
}

void CapcomPC98_FM::setMasterVolume (int vol) {

}

PC98AudioCore::MutexLock CapcomPC98_FM::lockMutex() {
	if (!_ready)
		error("CapcomPC98_FM::lockMutex(): Invalid call");
	return _ac->stackLockMutex();
}

void CapcomPC98_FM::send(uint32 evt) {
	uint8 ch = evt & 0x0F;
	uint8 p1 = (evt >> 8) & 0xFF;
	uint8 p2 = (evt >> 16) & 0xFF;

	if (ch > 2)
		return;

	switch (evt & 0xF0) {
	case 0x80:
		_chan[ch]->noteOff(p1);
		break;
	case 0x90:
		if (p2)
			_chan[ch]->noteOn(p1, p2);
		else
			_chan[ch]->noteOff(p1);
		break;
	case 0xB0:
		controlChange(ch, p1, p2);
		break;
	case 0xC0:
		_chan[ch]->programChange(p1);
		break;
	case 0xE0:
		_chan[ch]->pitchBend(((p2 & 0x7F) << 7) | (p1 & 0x7F));
		break;
	default:
		break;
	}
}


void CapcomPC98_FM::timerCallbackA() {
	if (_ready && _cbProc.isValid()) {
		PC98AudioCore::MutexLock lock = _ac->stackLockMutex();
		_cbProc();
	}
}

void CapcomPC98_FM::processSounds() {
	for (int i = 0; i < 3; ++i)
		_chan[i]->processSounds();
}

void CapcomPC98_FM::controlChange(uint8 ch, uint8 control, uint8 val) {
	if (ch > 2)
		return;

	switch (control) {
	case 1:
		_chan[ch]->modWheel(val);
		break;
	case 2:
		_chan[ch]->breathControl(val);
		break;
	case 3:
		_soundMarkers[ch] = val;
		break;
	case 4:
		_chan[ch]->pitchBendSensitivity(val);
		break;
	case 5:
		_chan[ch]->portamentoTime(val);
		break;
	case 7:
		_chan[ch]->volume(val);
		break;
	case 65:
		_chan[ch]->togglePortamento(val);
		break;
	case 123:
		_chan[ch]->allNotesOff();
		break;
	default:
		break;
	}
}

void CapcomPC98_FM::restoreStateIntern() {
	for (int i = 0; i < 3; ++i) {
		if ((1 << i) & _chanReserveFlags)
			continue;
		_chan[i]->restore();
	}
}

const uint8 CapcomPC98_FM::_initData[72] = {
	0x49, 0x4e, 0x49, 0x54, 0x5f, 0x56, 0x4f, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00,
	0x00, 0x00, 0x01, 0x7f, 0x1f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x1f,
	0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x7f, 0x1f, 0x00, 0x00, 0x0f, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

CapcomPC98AudioDriverInternal *CapcomPC98AudioDriverInternal::_refInstance = 0;
int CapcomPC98AudioDriverInternal::_refCount = 0;

CapcomPC98AudioDriverInternal::CapcomPC98AudioDriverInternal(Audio::Mixer *mixer, MidiDriver::DeviceHandle dev) : _ready(false), _fmDevice(nullptr), _timerProc(nullptr), _mutexProc(nullptr), _marker(0), _musicVolume(0), _sfxVolume(0) {
	MusicType type = MidiDriver::getMusicType(dev);
	_timerProc = new CapcomPC98_FM::CBProc(this, &CapcomPC98AudioDriverInternal::timerCallback);
	assert(_timerProc);
	_mutexProc = new CapcomPC98_MIDI::MutexProc(this, &CapcomPC98AudioDriverInternal::lockMutex);
	assert(_mutexProc);

	if (type == MT_MT32 || type == MT_GM) {
		_players[0] = new CapcomPC98_MIDI(dev, type == MT_MT32, *_mutexProc);
		_players[1] = _fmDevice = new CapcomPC98_FM(mixer, *_timerProc, true, CapcomPC98Player::kPrioPlay, 4, (uint8)~4, true);
		_marker = 1;
	} else {
		_players[0] = new CapcomPC98_FM(mixer, *_timerProc, false, CapcomPC98Player::kStdPlay, 3, 0, false);
		_players[1] = _fmDevice = new CapcomPC98_FM(mixer, *_timerProc, true, CapcomPC98Player::kPrioPlay | CapcomPC98Player::kPrioClaim, 4, (uint8)~4, true);
	}

	bool ready = true;
	for (int i = 0; i < 2; ++i) {
		if (!(_players[i] && _players[i]->init()))
			ready = false;
	}

	_ready = ready;
}

CapcomPC98AudioDriverInternal::~CapcomPC98AudioDriverInternal() {
	_ready = false;

	for (int i = 0; i < 2; ++i)
		_players[i]->deinit();

	for (int i = 0; i < 2; ++i)
		delete _players[i];

	delete _timerProc;
	delete _mutexProc;
}

CapcomPC98AudioDriverInternal *CapcomPC98AudioDriverInternal::open(Audio::Mixer *mixer, MidiDriver::DeviceHandle dev) {
	_refCount++;

	if (_refCount == 1 && _refInstance == 0)
		_refInstance = new CapcomPC98AudioDriverInternal(mixer, dev);
	else if (_refCount < 2 || _refInstance == 0)
		error("CapcomPC98AudioDriverInternal::open(): Internal instance management failure");

	return _refInstance;
}

void CapcomPC98AudioDriverInternal::close() {
	if (!_refCount)
		return;

	_refCount--;

	if (!_refCount) {
		delete _refInstance;
		_refInstance = 0;
	}
}

void CapcomPC98AudioDriverInternal::reset() {
	for (int i = 0; i < 2; ++i)
		_players[i]->reset();
}

void CapcomPC98AudioDriverInternal::loadFMInstruments(const uint8 *data) {
	for (int i = 0; i < 2; ++i)
		_players[i]->loadInstruments(data + 2, READ_LE_UINT16(data));
}

void CapcomPC98AudioDriverInternal::startSong(const uint8 *data, uint8 volume, bool loop) {
	stopSong();
	if (_ready)
		_players[0]->startSound(data, volume, loop);
}

void CapcomPC98AudioDriverInternal::stopSong() {
	if (_ready)
		_players[0]->stopSound();
}

void CapcomPC98AudioDriverInternal::startSoundEffect(const uint8 *data, uint8 volume) {
	stopSoundEffect();
	if (_ready)
		_players[1]->startSound(data, volume, false);
}

void CapcomPC98AudioDriverInternal::stopSoundEffect() {
	if (_ready)
		_players[1]->stopSound();
}

int CapcomPC98AudioDriverInternal::checkSoundMarker() const {
	return _players[0]->getMarker(_marker);
}

bool CapcomPC98AudioDriverInternal::songIsPlaying() const {
	return CapcomPC98Player::getStatus() & CapcomPC98Player::kStdPlay;
}

bool CapcomPC98AudioDriverInternal::soundEffectIsPlaying() const {
	return CapcomPC98Player::getStatus() & CapcomPC98Player::kPrioPlay;
}

void CapcomPC98AudioDriverInternal::fadeOut() {
	if (!_ready)
		return;
	for (int i = 0; i < 2; ++i)
		_players[i]->fadeOut(1);
}

void CapcomPC98AudioDriverInternal::allNotesOff() {
	if (!_ready)
		return;
	for (int i = 0; i < 2; ++i)
		_players[i]->allNotesOff();
}

void CapcomPC98AudioDriverInternal::setMusicVolume(int volume) {
	_musicVolume = volume;
	updateMasterVolume();
}

void CapcomPC98AudioDriverInternal::setSoundEffectVolume(int volume) {
	_sfxVolume = volume;
	updateMasterVolume();
}

void CapcomPC98AudioDriverInternal::timerCallback() {
	for (int i = 0; i < 2; ++i)
		_players[i]->nextTick();
}

PC98AudioCore::MutexLock CapcomPC98AudioDriverInternal::lockMutex() {
	if (!_ready)
		error("CapcomPC98AudioDriverInternal::lockMutex(): Invalid call");

	return _fmDevice->lockMutex();
}

void CapcomPC98AudioDriverInternal::updateMasterVolume() {
	if (!_ready)
		return;
	_players[0]->setMasterVolume(_musicVolume);
	_players[1]->setMasterVolume(_sfxVolume);
}

CapcomPC98AudioDriver::CapcomPC98AudioDriver(Audio::Mixer *mixer, MidiDriver::DeviceHandle dev) {
	_drv = CapcomPC98AudioDriverInternal::open(mixer, dev);
}

CapcomPC98AudioDriver::~CapcomPC98AudioDriver() {
	CapcomPC98AudioDriverInternal::close();
	_drv = 0;
}

bool CapcomPC98AudioDriver::isUsable() const {
	return (_drv && _drv->isUsable());
}

void CapcomPC98AudioDriver::reset() {
	if (_drv)
		_drv->reset();
}

void CapcomPC98AudioDriver::loadFMInstruments(const uint8 *data) {
	if (_drv)
		_drv->loadFMInstruments(data);
}

void CapcomPC98AudioDriver::startSong(const uint8 *data, uint8 volume, bool loop) {
	if (_drv)
		_drv->startSong(data, volume, loop);
}

void CapcomPC98AudioDriver::stopSong() {
	if (_drv)
		_drv->stopSong();
}

void CapcomPC98AudioDriver::startSoundEffect(const uint8 *data, uint8 volume) {
	if (_drv)
		_drv->startSoundEffect(data, volume);
}

void CapcomPC98AudioDriver::stopSoundEffect() {
	if (_drv)
		_drv->stopSoundEffect();
}

int CapcomPC98AudioDriver::checkSoundMarker() const {
	return _drv ? _drv->checkSoundMarker() : 99;
}

bool CapcomPC98AudioDriver::songIsPlaying() const {
	return _drv ? _drv->songIsPlaying() : false;
}

bool CapcomPC98AudioDriver::soundEffectIsPlaying() const {
	return _drv ? _drv->soundEffectIsPlaying() : false;
}

void CapcomPC98AudioDriver::fadeOut() {
	if (_drv)
		_drv->fadeOut();
}

void CapcomPC98AudioDriver::allNotesOff() {
	if (_drv)
		_drv->allNotesOff();
}

void CapcomPC98AudioDriver::setMusicVolume(int volume) {
	if (_drv)
		_drv->setMusicVolume(volume);
}

void CapcomPC98AudioDriver::setSoundEffectVolume(int volume) {
	if (_drv)
		_drv->setSoundEffectVolume(volume);
}

} // End of namespace Kyra

#endif
