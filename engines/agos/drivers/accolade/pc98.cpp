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

#include "audio/softsynth/fmtowns_pc98/pc98_audio.h"
#include "audio/mididrv.h"
#include "audio/mixer.h"
#include "engines/engine.h"
#include "common/func.h"

namespace AGOS {

class PC98CommonDriver : public MidiDriver {
public:
	enum PC98DriverProperties {
		kPropMusicVolume = 0x10,
		kPropSfxVolume = 0x20,
		kPropPause = 0x30
	};
public:
	PC98CommonDriver();
	~PC98CommonDriver() override {};

	bool isOpen() const override { return _isOpen; }
	void send(uint32 b) override;
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;
	uint32 property(int prop, uint32 param) override;
	uint32 getBaseTempo() override { return _baseTempo; }
	MidiChannel *allocateChannel() override { return nullptr; }
	MidiChannel *getPercussionChannel() override { return nullptr; }

protected:
	void updateSounds();
	void updateParser();
	void reset();

	uint32 _baseTempo;
	bool _isOpen;

	Audio::Mixer *_mixer;

	const uint8 *_instrumentsRemap;
	const int8 *_instrumentLevelAdjust;
	const uint8 *_partsRemap;

	uint8 _chanUse[16];
	uint8 _ngDelay;
	bool _allNotes;
	bool _programLock;
	bool _noFadeRemap;
	bool _delayedProgramChange;

private:
	virtual void noteOn(uint8 part, uint8 note, uint8 velo) = 0;
	virtual void noteOff(uint8 part, uint8 note) = 0;
	virtual void programChange(uint8 part, uint8 prog) = 0;
	virtual void processSounds() = 0;
	virtual void setVolume(int musicVolume, int sfxVolume) = 0;
	virtual void pause(bool paused) = 0;

	class TimerCb {
	public:
		typedef void(*FuncType)(void*);
		TimerCb(const FuncType func, void *arg) : _func(func), _arg(arg) {}
		bool isValid() const { return _func && _arg; }
		void operator()() const { (*_func)(_arg); }
	private:
		const FuncType _func;
		void *_arg;
	} *_timerCb;

	uint32 _internalUpdateTimer;

	uint16 _musicVolume;
	uint16 _sfxVolume;
	int8 _fadeVolumeAdjust;
	uint8 _partPrograms[16];
};

class PC98FMDriver : public PC98CommonDriver, private PC98AudioPluginDriver {
public:
	PC98FMDriver();
	~PC98FMDriver() override;

	int open() override;
	void close() override;

private:
	void noteOn(uint8 part, uint8 note, uint8 velo) override;
	void noteOff(uint8 part, uint8 note) override;
	void programChange(uint8 part, uint8 prog) override;
	void processSounds() override;
	void setVolume(int musicVolume, int sfxVolume) override;
	void pause(bool paused) override {}

	void loadInstrument(uint8 chan, uint8 prg);
	void startNote(uint8 chan, uint8 note, uint8 velo);
	void stopNote(uint8 chan, uint8 note);

	void timerCallbackA() override {}
	void timerCallbackB() override;

	PC98AudioCore *_pc98a;

	uint8 _chanAssign[3];
	uint8 _chanNotes[3];
	uint8 _partProgramsInternal[16];
	uint8 _partNotes[16];

	static const uint8 _instrumentsRemapFM[128];
	static const uint8 _instrumentLevelAdjustFM[128];
	static const uint8 _partsRemapFM[16];
	static const uint8 _instrumentPatches[16][25];
	static const uint8 _ngMapping[76];
	static const uint8 _carrier[8];
	static const uint16 _frequency[12];
};

class PC98MidiDriver : public PC98CommonDriver {
public:
	PC98MidiDriver(DeviceHandle dev);
	~PC98MidiDriver() override;

	int open() override;
	void close() override;

	static void timerCallback(void *obj);

private:
	void noteOn(uint8 part, uint8 note, uint8 velo) override;
	void noteOff(uint8 part, uint8 note) override;
	void programChange(uint8 part, uint8 prog) override;
	void processSounds() override {}
	void setVolume(int musicVolume, int sfxVolume) override;
	void pause(bool paused) override;
	void sendSysexWithCheckSum(uint8 *data);

	MidiDriver *_drv;
	DeviceHandle _dev;

	uint8 _volSysex[9];
	uint8 _partAssignSysexGS[9];
	uint8 _partAssignSysexMT32[9];

	static const uint8 _instrumentsRemapMT32[128];
	static const uint8 _instrumentsRemapGM[128];
	static const uint8 _partsRemapMidi[16];
	static const uint8 _sysexMsg[3][9];
};

PC98CommonDriver::PC98CommonDriver() : _mixer(g_engine->_mixer), _baseTempo(0), _fadeVolumeAdjust(0), _allNotes(false), _programLock(false), _isOpen(false), _noFadeRemap(false), _delayedProgramChange(false), _ngDelay(0), _timerCb(nullptr), _musicVolume(0xff), _sfxVolume(0xff), _internalUpdateTimer(0) {
	memset(_partPrograms, 0, sizeof(_partPrograms));
	memset(_chanUse, 0, sizeof(_chanUse));
}

void PC98CommonDriver::send(uint32 b) {
	if (!_isOpen)
		return;

	byte para2 = (b >> 16) & 0xFF;
	byte para1 = (b >> 8) & 0xFF;
	byte ch = b & 0x0F;

	switch (b & 0xF0) {
	case 0x80:
		noteOff(ch, para1);
		break;
	case 0x90:
		if (para2) {
			int16 velo = para2;
			if (ch != 9)
				velo = CLIP<int16>(velo + _instrumentLevelAdjust[_partPrograms[ch]], 0, 127);
			velo = CLIP<int16>(velo + _fadeVolumeAdjust, 0, 127);
			noteOn(ch, para1, velo);
		} else {
			noteOff(ch, para1);
		}
		break;
	case 0xC0:
		_partPrograms[ch] = para1;
		programChange(ch, ch == 9 ? 0 : _instrumentsRemap[para1 & 0x07F]);
		break;
	default:
		// 0xA0 and 0xB0 are parsing related and will be filtered and handled in MidiParser_S1D.
		if (!((b & 0xF0) == 0xB0 && (para1 == 0x7b || para1 == 0x07)))
			warning("PC98CommonDriver::send(): Unsupported Midi Message: 0x%02x 0x%02x 0x%02x", b & 0xFF, para1, para2);
		break;
	}
}

void PC98CommonDriver::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	delete _timerCb;
	_timerCb = (_isOpen && timerParam && timerProc) ? new TimerCb(timerProc, timerParam) : nullptr;
}

uint32 PC98CommonDriver::property(int prop, uint32 param) {
	uint32 res = 0;
	switch (prop) {
	case kPropMusicVolume:
	case kPropSfxVolume: {
		uint16 &v = (prop == kPropMusicVolume) ? _musicVolume : _sfxVolume;
		res = v;
		if ((int32)param != -1)
			v = param & 0x1ff;
		if (_isOpen)
			setVolume(_musicVolume, _sfxVolume);
		break;
	}
	case kPropPause: {
		if (_isOpen)
			pause(param);
		break;
	}
	default:
		break;
	}
	return res;
}

void PC98CommonDriver::updateSounds() {
	if (!_isOpen)
		return;

	_internalUpdateTimer += _baseTempo;
	if (_internalUpdateTimer >= 16667) {
		_internalUpdateTimer -= 16667;

		// I haven't implemented music fading in and out, since Elvira 1 (the
		// only game for this sound driver) does not use the feature at all.
		// The fade volume would have to be updated here...

		for (int i = 0; i < 16; ++i)
			_chanUse[i] = 0;

		processSounds();
	}
}

void PC98CommonDriver::updateParser() {
	if (_isOpen && _timerCb && _timerCb->isValid())
		(*_timerCb)();
}

void PC98CommonDriver::reset() {
	memset(_partPrograms, 0, sizeof(_partPrograms));
	memset(_chanUse, 0, sizeof(_chanUse));
	_allNotes = false;
	_programLock = false;
	_noFadeRemap = false;
	_delayedProgramChange = false;
	_ngDelay = 0;
}

PC98FMDriver::PC98FMDriver() : PC98CommonDriver(), _pc98a(nullptr) {
	_baseTempo = 10080;
	_instrumentsRemap = _instrumentsRemapFM;
	_instrumentLevelAdjust = (const int8*)_instrumentLevelAdjustFM;
	_partsRemap = _partsRemapFM;
	memset(_partProgramsInternal, 0, sizeof(_partProgramsInternal));
	memset(_partNotes, 0, sizeof(_partNotes));
	memset(_chanAssign, 0, sizeof(_chanAssign));
	memset(_chanNotes, 0, sizeof(_chanNotes));
}

PC98FMDriver::~PC98FMDriver() {
	_mixer->stopAll();
	close();
}

int PC98FMDriver::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	delete _pc98a;

	_pc98a = new PC98AudioCore(g_engine->_mixer, this, kType26);
	if (_pc98a && _pc98a->init()) {
		_pc98a->writeReg(0, 0x06, 0x0a);
		_pc98a->writeReg(0, 0x07, 0x9c);
		for (int i = 8; i < 11; ++i)
			_pc98a->writeReg(0, i, 0);
		_pc98a->writeReg(0, 0x27, 0x3a);
	} else {
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	memset(_partProgramsInternal, 0, sizeof(_partProgramsInternal));
	memset(_partNotes, 0, sizeof(_partNotes));
	memset(_chanAssign, 0, sizeof(_chanAssign));
	memset(_chanNotes, 0, sizeof(_chanNotes));

	reset();

	_isOpen = true;

	return 0;
}

void PC98FMDriver::close() {
	_isOpen = false;
	delete _pc98a;
	_pc98a = nullptr;
	setTimerCallback(nullptr, nullptr);
}

void PC98FMDriver::noteOn(uint8 part, uint8 note, uint8 velo) {
	if (_delayedProgramChange && part != 9) {
		int ch = 0x80;
		uint8 high = 0x80;
		for (int i = 2; i >= 0; --i) {
			if (_chanAssign[i] == 0x80) {
				ch = i;
				break;
			}
			if (part < _chanAssign[i] && high > _chanAssign[i]) {
				ch = i;
				high = _chanAssign[i];
			}
		}
		if (ch == 0x80)
			return;

		loadInstrument(ch, _partProgramsInternal[part]);

		_partNotes[ch] = note;
		_chanAssign[ch] = part;
		part = ch;
	}
	startNote(part, note, velo);
}

void PC98FMDriver::noteOff(uint8 part, uint8 note) {
	if (_delayedProgramChange) {
		if (part == 9) {
			_pc98a->writeReg(0, 6, 0);
			stopNote(part, note);
		} else {
			for (int i = 2; i >= 0; --i) {
				if (_chanAssign[i] != part || (note != _partNotes[i] && !_allNotes))
					continue;
				_chanAssign[i] = 0x80;
				_partNotes[i] = 0;
				stopNote(i, note);
			}
		}
	} else {
		stopNote(part, note);
	}
}

void PC98FMDriver::programChange(uint8 part, uint8 prog) {
	if (!_delayedProgramChange)
		loadInstrument(part, prog);
	_partProgramsInternal[part] = prog;
}

void PC98FMDriver::processSounds() {
	if (_ngDelay)
		--_ngDelay;
	if (!_ngDelay)
		_pc98a->writeReg(0, 0x0a, 0);
}

void PC98FMDriver::setVolume(int musicVolume, int sfxVolume) {
	_pc98a->setMusicVolume(musicVolume);
	_pc98a->setSoundEffectVolume(sfxVolume);
}

void PC98FMDriver::loadInstrument(uint8 chan, uint8 prg) {
	if (chan > 2)
		return;

	assert(prg < ARRAYSIZE(_instrumentPatches));
	const uint8 *src = _instrumentPatches[prg];
	_pc98a->writeReg(0, 0xB0 | chan, *src++);

	for (uint8 reg = 0x30 | chan; reg < 0x40; reg += 4) {
		_pc98a->writeReg(0, reg, *src++);
		_pc98a->writeReg(0, reg + 0x10, *src++);
		_pc98a->writeReg(0, reg + 0x20, *src++);
		_pc98a->writeReg(0, reg + 0x30, (*src++) & 0x1F);
		_pc98a->writeReg(0, reg + 0x40, (*src++) & 0x1F);
		_pc98a->writeReg(0, reg + 0x50, *src++);
	}
}

void PC98FMDriver::startNote(uint8 chan, uint8 note, uint8 velo) {
	if (chan == 9) {
		if (note >= sizeof(_ngMapping) || _ngMapping[note] == 0xff)
			return;
		_pc98a->writeReg(0, 0x06, _ngMapping[note]);
		_pc98a->writeReg(0, 0x0a, 0x0a);
		_ngDelay = 3;
	}

	if (chan > 2)
		return;

	if (_chanUse[chan] && note < _chanNotes[chan])
		return;

	_allNotes = true;
	stopNote(chan, 0);
	_allNotes = false;
	_chanNotes[chan] = note;
	_chanUse[chan]++;

	const uint8 *instr = _instrumentPatches[_partProgramsInternal[chan]];
	uint8 c = _carrier[*instr & 7];

	instr += 2;
	const uint8 *pos = instr;
	uint8 instvl = 0x7F;
	for (int i = 0; i < 4; ++i) {
		if (((c >> i) & 1) && *pos < instvl)
			instvl = *pos;
		pos += 6;
	}

	pos = instr;
	velo = 0x7f - (0x57 + (velo >> 2)) - instvl;
	for (uint8 i = 0x40 | chan; i < 0x50; i += 4) {
		if (c & 1)
			_pc98a->writeReg(0, i, MIN<uint8>(*pos + velo, 0x7f));
		pos += 6;
		c >>= 1;
	}

	if (note > 18)
		note -= 12;
	uint16 frq = _frequency[note % 12];
	uint8 bl = (note / 12) << 3;
	_pc98a->writeReg(0, 0xa4 | chan, (frq >> 8) | bl);
	_pc98a->writeReg(0, 0xa0 | chan, frq & 0xff);

	_pc98a->writeReg(0, 0x28, 0xF0 | chan);
}

void PC98FMDriver::stopNote(uint8 chan, uint8 note) {
	if (chan > 2)
		return;

	if (_allNotes || note == _chanNotes[chan])
		_pc98a->writeReg(0, 0x28, chan);
}

void PC98FMDriver::timerCallbackB() {
	updateSounds();
	PC98AudioCore::MutexLock tempUnlock = _pc98a->stackUnlockMutex();
	updateParser();
}

const uint8 PC98FMDriver::_instrumentsRemapFM[128] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0x00, 0x09, 0x0a, 0x0b, 0x0c, 0x01, 0x02, 0x0f,
	0x0f, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,	0x18, 0x19, 0x19, 0x1b, 0x1c, 0x1d, 0x1e, 0x03,
	0x04, 0x21, 0x22, 0x23, 0x05, 0x25, 0x06, 0x27,	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2e,
	0x30, 0x31, 0x35, 0x07, 0x35, 0x35, 0x36, 0x37,	0x38, 0x08, 0x3a, 0x3b, 0x3c, 0x3e, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x44, 0x44, 0x45, 0x09, 0x47,	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x0a, 0x51, 0x51,
	0x51, 0x54, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	0x58, 0x59, 0x5a, 0x5b, 0x5a, 0x5a, 0x5e, 0x5f,
	0x60, 0x61, 0x67, 0x63, 0x0c, 0x65, 0x66, 0x67,	0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c,
	0x0c, 0x0c, 0x0c, 0x0d, 0x0e, 0x0f, 0x76, 0x77,	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

const uint8 PC98FMDriver::_instrumentLevelAdjustFM[128] = {
	0x28, 0x0f, 0x28, 0x1d, 0x14, 0x14, 0x23, 0x19,	0x28, 0x13, 0x23, 0x23, 0x30, 0x23, 0x17, 0x16,
	0x23, 0x14, 0x15, 0x13, 0x23, 0x23, 0x19, 0x19,	0x32, 0x19, 0x16, 0x23, 0x05, 0x0a, 0x05, 0x0a,
	0x35, 0x28, 0x2d, 0x23, 0x19, 0x1c, 0x22, 0x23,	0x23, 0x1a, 0x2d, 0x23, 0x23, 0x23, 0x23, 0x1e,
	0x32, 0x1e, 0x37, 0x23, 0x18, 0x2e, 0x2b, 0x32,	0x11, 0x14, 0x0f, 0x0f, 0x14, 0x14, 0x14, 0x14,
	0x28, 0x28, 0x28, 0x28, 0x28, 0x28, 0x0a, 0x28,	0x0d, 0x14, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23,
	0x23, 0x0a, 0x19, 0x19, 0x14, 0x14, 0x12, 0x14,	0x2b, 0x2c, 0x34, 0x2e, 0x30, 0x31, 0x15, 0x29,
	0x32, 0x23, 0x23, 0x23, 0x23, 0x0b, 0x23, 0x23,	0x23, 0x23, 0x23, 0x23, 0x14, 0x14, 0x1e, 0x23,
	0x23, 0x23, 0x23, 0x23, 0x2c, 0x23, 0x23, 0x23,	0x23, 0x23, 0x23, 0x23, 0x00, 0x23, 0x23, 0x23
};

const uint8 PC98FMDriver::_partsRemapFM[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

const uint8 PC98FMDriver::_instrumentPatches[16][25] = {
	{ 0x3f, 0x06, 0x6b, 0x1f, 0x0e, 0x00, 0xff, 0x03, 0x0a, 0x1f, 0x02, 0x01, 0x0f, 0x03, 0x0a, 0x1f, 0x02, 0x01, 0x0f, 0x02, 0x00, 0x1f, 0x02, 0x01, 0x0f },
	{ 0x3f, 0x38, 0x75, 0x1f, 0x81, 0x01, 0x0a, 0x01, 0x00, 0x14, 0x82, 0x01, 0x0a, 0x73, 0x00, 0x14, 0x82, 0x01, 0x0a, 0x62, 0x00, 0x14, 0x82, 0x01, 0x0a },
	{ 0x07, 0x08, 0x3e, 0xdf, 0x15, 0x00, 0x0f, 0x03, 0x11, 0x5f, 0x1f, 0x00, 0x0a, 0x02, 0x25, 0x5d, 0x1f, 0x00, 0x0a, 0x02, 0x00, 0x92, 0x1f, 0x00, 0x0a },
	{ 0x3d, 0x4a, 0x23, 0x1b, 0x11, 0x00, 0xfa, 0x41, 0x00, 0x14, 0x00, 0x00, 0x0a, 0x42, 0x00, 0x14, 0x00, 0x00, 0x0a, 0x40, 0x00, 0x14, 0x00, 0x00, 0x0a },
	{ 0x3c, 0x0b, 0x2b, 0x5a, 0x02, 0x01, 0x35, 0x63, 0x2a, 0x55, 0x01, 0x00, 0x24, 0x03, 0x19, 0x5c, 0x09, 0x05, 0x44, 0x21, 0x00, 0x4d, 0x06, 0x00, 0x44 },
	{ 0x3c, 0x01, 0x20, 0x52, 0x0c, 0x01, 0x5a, 0x22, 0x17, 0x4f, 0x0a, 0x01, 0x2a, 0x26, 0x05, 0x45, 0x0a, 0x00, 0x0f, 0x31, 0x00, 0x47, 0x02, 0x00, 0x0f },
	{ 0x2c, 0x3a, 0x2d, 0x58, 0x0e, 0x00, 0xf7, 0x05, 0x39, 0x5a, 0x0e, 0x00, 0xf4, 0x02, 0x00, 0x58, 0x08, 0x00, 0xf4, 0x01, 0x05, 0x9a, 0x08, 0x00, 0xf4 },
	{ 0x3c, 0x01, 0x20, 0x52, 0x0c, 0x01, 0x2a, 0x21, 0x17, 0x4f, 0x0a, 0x01, 0x5a, 0x11, 0x00, 0x12, 0x8a, 0x01, 0x3a, 0x61, 0x07, 0x14, 0x82, 0x01, 0x3a },
	{ 0x00, 0x01, 0x7f, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x23, 0x5f, 0x05, 0x00, 0xf3, 0x71, 0x1b, 0x5f, 0x0c, 0x01, 0xf5, 0x01, 0x07, 0x5f, 0x0c, 0x00, 0xf5 },
	{ 0x18, 0x37, 0x2c, 0x9e, 0x0d, 0x08, 0xb6, 0x31, 0x18, 0x1c, 0x04, 0x03, 0x36, 0x30, 0x22, 0xdc, 0x06, 0x0a, 0xb6, 0x32, 0x00, 0x9c, 0x01, 0x05, 0x26 },
	{ 0x3b, 0x0a, 0x00, 0x1f, 0x00, 0x00, 0x0a, 0x02, 0x22, 0x18, 0x0e, 0x00, 0x0a, 0x60, 0x3a, 0x18, 0x0c, 0x00, 0xfa, 0x31, 0x00, 0x51, 0x0b, 0x00, 0x38 },
	{ 0x2c, 0x34, 0x21, 0x58, 0x0e, 0x00, 0xf7, 0x76, 0x2f, 0x58, 0x14, 0x00, 0xfa, 0x30, 0x00, 0xd8, 0x84, 0x00, 0xf2, 0x72, 0x0b, 0x98, 0x8c, 0x00, 0xf6 },
	{ 0x3b, 0x08, 0x06, 0x5f, 0x00, 0x00, 0x1a, 0x01, 0x19, 0x4e, 0x0e, 0x00, 0x0a, 0x61, 0x30, 0x58, 0x18, 0x00, 0x5a, 0x30, 0x00, 0x50, 0x0b, 0x00, 0x38 },
	{ 0x3b, 0x0a, 0x0d, 0x16, 0x00, 0x00, 0x0a, 0x00, 0x0b, 0x1a, 0x16, 0x40, 0xfb, 0x0d, 0x13, 0x1a, 0x1a, 0xc0, 0xfa, 0x01, 0x00, 0x5e, 0x8e, 0x00, 0xf7 },
	{ 0x3b, 0x0e, 0x0c, 0x1f, 0x00, 0x00, 0x05, 0x0a, 0x25, 0x1b, 0x1b, 0x80, 0xfa, 0x00, 0x31, 0x1f, 0x0a, 0xc0, 0xf5, 0x00, 0x00, 0x5c, 0x8e, 0x40, 0xf7 },
	{ 0x32, 0x02, 0x15, 0x58, 0x14, 0x00, 0xfa, 0x31, 0x25, 0x5f, 0x0a, 0x40, 0xf4, 0x01, 0x17, 0x5a, 0x0c, 0x80, 0xf6, 0x01, 0x00, 0x9a, 0x8b, 0x00, 0xf5 }
};

const uint8 PC98FMDriver::_ngMapping[76] = {
	0x18, 0x1c, 0x3c, 0x20, 0x3e, 0x24, 0x40, 0x28,	0x2c, 0x38, 0x30, 0x3c, 0x00, 0x00, 0x36, 0x00,	0x38, 0x00, 0x00,
	0x00, 0x16, 0x11, 0x16, 0x16, 0x11, 0x16, 0x11, 0x16, 0x11, 0x0f, 0x32, 0x00, 0x00, 0x0d, 0x00, 0x10, 0x1f, 0x1f,
	0x0a, 0x08, 0x0a, 0x19, 0x04, 0x19, 0x04, 0x14, 0x04, 0x14, 0x0f, 0x0c, 0x0f, 0x0c, 0xff, 0xff, 0x0d, 0xff,	0x12,
	0xff, 0xff, 0xff, 0x0f, 0x19, 0x0f, 0x0f, 0x19, 0x0f, 0x19, 0x0f, 0x19, 0x14, 0x06, 0xff, 0xff, 0x0f, 0xff, 0x00
};

const uint8 PC98FMDriver::_carrier[8] = {
	0x08, 0x08, 0x08, 0x08, 0x0C, 0x0E, 0x0E, 0x0F
};

const uint16 PC98FMDriver::_frequency[12] = {
	0x0267, 0x028d, 0x02b3, 0x02dd, 0x0308, 0x0337, 0x0368, 0x039c, 0x03d3, 0x040e, 0x044b, 0x048b
};

#define MIDIMSG32(s, p1, p2) (p2 << 16 | p1 << 8 | s)

PC98MidiDriver::PC98MidiDriver(MidiDriver::DeviceHandle dev) : _dev(dev), _drv(nullptr) {
	_instrumentsRemap = (getMusicType(dev) == MT_MT32) ? _instrumentsRemapMT32 : (getMusicType(dev) == MT_GM ? _instrumentsRemapGM : nullptr);
	int8 *tbl2 = new int8[128]();
	_instrumentLevelAdjust = tbl2;
	_partsRemap = _partsRemapMidi;
	memcpy(_volSysex, _sysexMsg[0], 9);
	memcpy(_partAssignSysexGS, _sysexMsg[1], 9);
	memcpy(_partAssignSysexMT32, _sysexMsg[2], 9);
}

PC98MidiDriver::~PC98MidiDriver() {
	close();
	delete[] _instrumentLevelAdjust;
}

int PC98MidiDriver::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	delete _drv;

	_drv = MidiDriver::createMidi(_dev);
	if (!_drv || !_instrumentsRemap)
		return MERR_DEVICE_NOT_AVAILABLE;

	_baseTempo = _drv->getBaseTempo();
	int res = _drv->open();

	if (!res) {
		_drv->setTimerCallback(this, &timerCallback);

		for (byte i = 0xB1; i < 0xBA; ++i)
			_drv->send(MIDIMSG32(i, 0x79, 0));

		property(kPropMusicVolume, Audio::Mixer::kMaxChannelVolume);

		if (getMusicType(_dev) == MT_MT32) {
			_partAssignSysexGS[7] = 0x10;
			for (uint8 i = 0x10; i < 0x20; ++i) {
				_partAssignSysexGS[5] = i;
				sendSysexWithCheckSum(_partAssignSysexGS);
			}

			for (uint8 i = 0x01; i < 0x0A; ++i) {
				_partAssignSysexMT32[6] = 0x0C + i;
				_partAssignSysexMT32[7] = i;
				sendSysexWithCheckSum(_partAssignSysexMT32);
			}

		} else if (getMusicType(_dev) == MT_GM) {
			_partAssignSysexGS[5] = 0x10;
			_partAssignSysexGS[7] = 9;
			sendSysexWithCheckSum(_partAssignSysexGS);
			uint8 p = 0;
			for (uint8 i = 0x11; i < 0x20; ++i) {
				_partAssignSysexGS[5] = i;
				_partAssignSysexGS[7] = p++;
				if (p == 9)
					p++;
				sendSysexWithCheckSum(_partAssignSysexGS);
			}

			_partAssignSysexMT32[7] = 0x10;
			for (uint8 i = 0x0D; i < 0x16; ++i) {
				_partAssignSysexMT32[6] = i;
				sendSysexWithCheckSum(_partAssignSysexMT32);
			}

			_drv->send(MIDIMSG32(0xB9, 0x07, 0x46));
		}

		reset();
		_isOpen = true;
	}

	return res;
}

void PC98MidiDriver::close() {
	_isOpen = false;

	if (_drv) {
		_drv->setTimerCallback(nullptr, nullptr);
		_mixer->stopAll();
		_drv->close();
		delete _drv;
		_drv = nullptr;
	}

	setTimerCallback(nullptr, nullptr);
}

void PC98MidiDriver::timerCallback(void *obj) {
	PC98MidiDriver *drv = static_cast<PC98MidiDriver*>(obj);
	drv->updateSounds();
	drv->updateParser();
}

void PC98MidiDriver::noteOn(uint8 part, uint8 note, uint8 velo) {
	_drv->send(MIDIMSG32(0x90 | _partsRemap[part & 0x0F], note, velo));
}

void PC98MidiDriver::noteOff(uint8 part, uint8 note) {
	if (_allNotes)
		_drv->send(MIDIMSG32(0xB0 | _partsRemap[part & 0x0F], 0x7B, 0));
	else
		_drv->send(MIDIMSG32(0x80 | _partsRemap[part & 0x0F], note, 0));
}

void PC98MidiDriver::programChange(uint8 part, uint8 prog) {
	if (!_programLock)
		_drv->send(MIDIMSG32(0xC0 | _partsRemap[part & 0x0F], prog, 0));
}

void PC98MidiDriver::setVolume(int musicVolume, int sfxVolume) {
	if (!_isOpen)
		return;

	if (getMusicType(_dev) == MT_MT32) {
		_volSysex[7] = musicVolume * 100 / Audio::Mixer::kMaxChannelVolume;
		sendSysexWithCheckSum(_volSysex);
	} else {
		uint8 vol = musicVolume * 127 / Audio::Mixer::kMaxChannelVolume;
		for (int i = 0; i < 16; ++i)
			_drv->send(MIDIMSG32(0xB0 | _partsRemap[i], 0x07, vol));
	}
}

void PC98MidiDriver::pause(bool paused) {
	if (paused) {
		_allNotes = true;
		for (int i = 0; i < 16; ++i)
			noteOff(i, 0);
		_allNotes = false;
	}
}

void PC98MidiDriver::sendSysexWithCheckSum(uint8 *data) {
	uint8 chk = 0;
	for (int i = 4; i < 8; ++i)
		chk += data[i];
	data[8] = 0x80 - (chk & 0x7f);
	_drv->sysEx(data, 9);
}

const uint8 PC98MidiDriver::_instrumentsRemapMT32[128] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x6e, 0x65, 0x66, 0x67,	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

const uint8 PC98MidiDriver::_instrumentsRemapGM[128] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,	0x10, 0x09, 0x0a, 0x0b, 0x0c, 0x10, 0x10, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x26,
	0x58, 0x21, 0x22, 0x23, 0x61, 0x25, 0x0b, 0x27,	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x2d, 0x34, 0x35, 0x36, 0x37,	0x38, 0x2e, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x23, 0x47,	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4c, 0x4e, 0x4f,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x4c, 0x65, 0x66, 0x67,	0x0c, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x75, 0x76, 0x74, 0x76, 0x77,	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

const uint8 PC98MidiDriver::_partsRemapMidi[16] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,	0x0f, 0x09, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f
};

const uint8 PC98MidiDriver::_sysexMsg[3][9] = {
	{ 0x41, 0x10, 0x16, 0x12, 0x10, 0x00, 0x16, 0x64, 0x00 },
	{ 0x41, 0x10, 0x42, 0x12, 0x40, 0x10, 0x02, 0x10, 0x00 },
	{ 0x41, 0x10, 0x16, 0x12, 0x10, 0x00, 0x00, 0x00, 0x00 }
};

MidiDriver *MidiDriverPC98_create(MidiDriver::DeviceHandle dev) {
	MusicType type = MidiDriver::getMusicType(dev);
	if (type == MT_PC98)
		return new PC98FMDriver();
	else if (type == MT_GM || type == MT_MT32)
		return new PC98MidiDriver(dev);
	return nullptr;
}

#undef MIDIMSG32

} // End of namespace AGOS
