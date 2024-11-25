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

#include "engines/scumm/imuse/drivers/amiga.h"
#include "audio/mixer.h"
#include "common/file.h"
#include "common/translation.h"
#include "gui/error.h"

namespace Scumm {

struct Instrument_Amiga {
	struct Samples {
		uint16 rate;
		uint16 baseNote;
		int16 noteRangeMin;
		int16 noteRangeMax;
		int16 sustainLevel;
		uint16 type;
		uint32 numSamples;
		uint32 dr_offset;
		uint32 dr_numSamples;
		int16 levelFadeDelayAT;
		int16 levelFadeDelayRL;
		int16 levelFadeTriggerRL;
		int16 levelFadeDelayDC;
		const int8 *data;
	};
	Samples samples[8];
	int numBlocks;
};

class SoundChannel_Amiga {
public:
	SoundChannel_Amiga(IMuseDriver_Amiga *driver, int id, Instrument_Amiga *instruments);
	~SoundChannel_Amiga();

	static SoundChannel_Amiga *allocate(int prio);
	void connect(IMusePart_Amiga *part);
	void disconnect();

	void noteOn(byte note, byte velocity, byte program, int8 transpose, int16 pitchBend);
	void ctrl_volume(uint8 volume);
	void ctrl_sustain(bool sustainToggle);
	void transposePitchBend(int8 transpose, int16 pitchBend);

	void updateLevel();
	void updateEnvelope();

	uint8 getNote() const { return _note; }
	SoundChannel_Amiga *next() const { return _next; }

private:
	void keyOn(const int8 *data1, uint16 data1Size, const int8 *data2, uint16 data2Size, uint16 period);
	void keyOff();
	void setRepeatData(const int8 *data, uint16 size);
	void setVelocity(uint8 velo, int delay);
	void setVolume(uint8 volume);

	uint16 calculatePeriod(int16 tone, uint8 baseNote, uint16 rate);

	void createVolumeTable();

	SoundChannel_Amiga *_prev, *_next;
	IMusePart_Amiga *_assign;

	uint8 _id;
	uint8 _note;
	bool _sustain;

	IMuseDriver_Amiga *_driver;

	static uint8 _allocCurPos;
	static SoundChannel_Amiga *_channels[4];

	enum EnvelopeState {
		kReady = 0,
		kRelease = 1,
		kDecay = 2,
		kAttack = 3,
		kRestart = 4
	};

	struct IOUnit {
		IOUnit() : program(0), block(0), volume(63), currentLevel(0), fadeTargetLevel(0), fadeLevelDelta(0), fadeLevelMod(0), levelFadeTriggerDC(0), fadeLevelTicks(0),
			fadeLevelTicker(0), fadeLevelDuration(0), releaseData(nullptr), releaseDataSize(0), repeatData(nullptr), repeatDataSize(0), envelopeState(kReady) {}
		uint8 program;
		uint8 block;
		uint8 volume;
		uint8 currentLevel;
		uint8 fadeTargetLevel;
		uint8 fadeLevelDelta;
		uint16 fadeLevelTicks;
		int8 fadeLevelMod;
		bool levelFadeTriggerDC;
		uint32 fadeLevelTicker;
		uint32 fadeLevelDuration;
		const int8 *releaseData;
		uint16 releaseDataSize;
		const int8 *repeatData;
		uint16 repeatDataSize;
		uint8 envelopeState;
	};

	IOUnit _ioUnit;

	const Instrument_Amiga *_instruments;

	static const int8 _muteData[16];
	static const uint8 *_volTable;
};

class IMusePart_Amiga : public MidiChannel {
public:
	IMusePart_Amiga(IMuseDriver_Amiga *driver, int id);
	~IMusePart_Amiga() override {}

	MidiDriver *device() override { return _driver; }
	byte getNumber() override { return _id; }
	bool allocate();
	void release() override;

	void send(uint32 b) override;

	void noteOff(byte note) override;
	void noteOn(byte note, byte velocity) override;
	void controlChange(byte control, byte value) override;
	void programChange(byte program) override;
	void pitchBend(int16 bend) override;
	void pitchBendFactor(byte value) override;
	void transpose(int8 value) override;
	void detune(int16 value) override;

	void priority(byte value) override;
	void sysEx_customInstrument(uint32 type, const byte *instr, uint32 dataSize) override {}

	int getPriority() const { return _priority; }
	SoundChannel_Amiga *getChannel() const { return _out; }
	void setChannel(SoundChannel_Amiga *chan) { _out = chan; }

private:
	void controlModulationWheel(byte value);
	void controlVolume(byte value);
	void controlSustain(byte value);

	uint8 _priority;
	uint8 _program;
	int8 _modulation;
	int8 _transpose;
	int8 _detune;
	int16 _pitchBend;
	uint8 _pitchBendSensitivity;
	uint16 _volume;
	bool _sustain;
	bool _allocated;
	const uint8 _id;
	SoundChannel_Amiga *_out;
	IMuseDriver_Amiga *_driver;
};

SoundChannel_Amiga::SoundChannel_Amiga(IMuseDriver_Amiga *driver, int id, Instrument_Amiga *instruments) : _driver(driver), _id(id), _instruments(instruments),
	_assign(nullptr), _next(nullptr), _prev(nullptr), _sustain(false), _note(0) {
	assert(id > -1 && id < 4);
	_channels[id] = this;
	createVolumeTable();
}

SoundChannel_Amiga::~SoundChannel_Amiga() {
	_channels[_id] = nullptr;

	// delete volume table only if this is the last remaining SoundChannel_Amiga object
	for (int i = 0; i < 4; ++i) {
		if (_channels[i])
			return;
	}

	delete[] _volTable;
	_volTable = nullptr;
}

SoundChannel_Amiga *SoundChannel_Amiga::allocate(int prio) {
	SoundChannel_Amiga *res = nullptr;

	for (int i = 0; i < 4; i++) {
		if (++_allocCurPos == 4)
			_allocCurPos = 0;

		SoundChannel_Amiga *temp = _channels[_allocCurPos];
		if (!temp->_assign)
			return temp;

		if (temp->_next)
			continue;

		if (prio >= temp->_assign->getPriority()) {
			res = temp;
			prio = temp->_assign->getPriority();
		}
	}

	if (res)
		res->disconnect();

	return res;
}

void SoundChannel_Amiga::connect(IMusePart_Amiga *part) {
	if (!part)
		return;

	_assign = part;
	_next = part->getChannel();
	_prev = nullptr;
	part->setChannel(this);
	if (_next)
		_next->_prev = this;
}

void SoundChannel_Amiga::disconnect() {
	keyOff();

	SoundChannel_Amiga *p = _prev;
	SoundChannel_Amiga *n = _next;

	if (n)
		n->_prev = p;
	if (p)
		p->_next = n;
	else
		_assign->setChannel(n);
	_assign = nullptr;
}

void SoundChannel_Amiga::noteOn(byte note, byte volume, byte program, int8 transpose, int16 pitchBend) {
	if (program > 128)
		program = 128;

	if (program != 128 && !_instruments[program].samples[0].data)
		program = 128;

	_note = note;
	_sustain = false;

	_ioUnit.block = 0;
	_ioUnit.program = program;
	const Instrument_Amiga::Samples *s = &_instruments[program].samples[_ioUnit.block];
	int16 pnote = note + transpose + (pitchBend >> 7);

	if (_instruments[program].numBlocks > 1) {
		for (int i = 0; i < _instruments[program].numBlocks; ++i) {
			if (pnote >= _instruments[program].samples[i].noteRangeMin && pnote <= _instruments[program].samples[i].noteRangeMax) {
				_ioUnit.block = i;
				s = &_instruments[program].samples[_ioUnit.block];
				break;
			}
		}
	}

	_driver->disableChannel(_id);
	setVelocity(0, 0);
	setVolume(volume);

	if (s->type > 1)
		return;

	uint16 period = calculatePeriod(pitchBend + ((_note + transpose) << 7), s->baseNote, s->rate);

	if (s->type == 1) {
		keyOn(s->data, s->numSamples, nullptr, 0, period);
		setRepeatData(nullptr, 0);
	} else {
		if (s->dr_numSamples) {
			keyOn(s->data, s->dr_numSamples, s->data + s->dr_offset, s->dr_numSamples - s->dr_offset, period);
			setRepeatData(s->data + s->dr_numSamples, s->numSamples - s->dr_numSamples);
		} else {
			keyOn(s->data, s->numSamples, s->data + s->dr_offset, s->numSamples - s->dr_offset, period);
			setRepeatData(nullptr, 0);
		}
	}
}

void SoundChannel_Amiga::ctrl_volume(uint8 volume) {
	setVolume(volume);
}

void SoundChannel_Amiga::ctrl_sustain(bool sustainToggle) {
	if (_sustain && !sustainToggle)
		disconnect();
	else if (sustainToggle)
		_sustain = true;
}

void SoundChannel_Amiga::transposePitchBend(int8 transpose, int16 pitchBend) {
	const Instrument_Amiga::Samples *s = &_instruments[_ioUnit.program].samples[_ioUnit.block];
	_driver->setChannelPeriod(_id, calculatePeriod(((_note + transpose) << 7) + pitchBend, s->baseNote, s->rate));
}

void SoundChannel_Amiga::updateLevel() {
	if (!_ioUnit.fadeLevelMod)
		return;

	_ioUnit.fadeLevelDuration += _ioUnit.fadeLevelDelta;
	if (_ioUnit.fadeLevelDuration <= _ioUnit.fadeLevelTicker)
		return;

	while (_ioUnit.fadeLevelDuration > _ioUnit.fadeLevelTicker && _ioUnit.currentLevel != _ioUnit.fadeTargetLevel) {
		_ioUnit.fadeLevelTicker += _ioUnit.fadeLevelTicks;
		_ioUnit.currentLevel += _ioUnit.fadeLevelMod;
	}

	_driver->setChannelVolume(_id, _volTable[(_ioUnit.volume << 5) + _ioUnit.currentLevel]);

	if (_ioUnit.currentLevel != _ioUnit.fadeTargetLevel)
		return;

	_ioUnit.fadeLevelMod = 0;
	if (!_ioUnit.levelFadeTriggerDC)
		return;

	const Instrument_Amiga::Samples *s = &_instruments[_ioUnit.program].samples[_ioUnit.block];
	setVelocity(s->sustainLevel >> 1, s->levelFadeDelayDC);
}

void SoundChannel_Amiga::updateEnvelope() {
	if (_ioUnit.envelopeState == kReady)
		return;

	uint8 envCur = _ioUnit.envelopeState--;
	if (envCur == kAttack) {
		const Instrument_Amiga::Samples *s = &_instruments[_ioUnit.program].samples[_ioUnit.block];
		_driver->enableChannel(_id);
		if (s->levelFadeDelayDC) {
			setVelocity(31, s->levelFadeDelayAT);
			if (s->levelFadeDelayAT)
				_ioUnit.levelFadeTriggerDC = true;
			else
				setVelocity(s->sustainLevel >> 1, s->levelFadeDelayDC);
		} else {
			setVelocity(s->sustainLevel >> 1, s->levelFadeDelayAT);
		}
	}

	if (envCur == kRelease) {
		_driver->setChannelSampleStart(_id, _ioUnit.releaseData);
		_driver->setChannelSampleLen(_id, _ioUnit.releaseDataSize);
	}
}

void SoundChannel_Amiga::keyOn(const int8 *attackData, uint16 attackDataSize, const int8 *releaseData, uint16 releaseDataSize, uint16 period) {
	_driver->setChannelSampleStart(_id, attackData);
	_driver->setChannelSampleLen(_id, attackDataSize >> 1);
	_driver->setChannelPeriod(_id, period);

	if (releaseData) {
		_ioUnit.releaseData = releaseData;
		_ioUnit.releaseDataSize = releaseDataSize >> 1;
	} else {
		_ioUnit.releaseData = _muteData;
		_ioUnit.releaseDataSize = ARRAYSIZE(_muteData) >> 1;
	}

	_ioUnit.envelopeState = kRestart;
}

void SoundChannel_Amiga::keyOff() {
	_ioUnit.levelFadeTriggerDC = 0;
	if (_ioUnit.repeatData) {
		_driver->setChannelSampleStart(_id, _ioUnit.repeatData);
		_driver->setChannelSampleLen(_id, _ioUnit.repeatDataSize);
		_ioUnit.releaseData = _muteData;
		_ioUnit.releaseDataSize = ARRAYSIZE(_muteData) >> 1;
		_ioUnit.envelopeState = kDecay;
	} else {
		_ioUnit.envelopeState = kReady;
	}

	if (_instruments[_ioUnit.program].samples[_ioUnit.block].levelFadeTriggerRL)
		setVelocity(0, _instruments[_ioUnit.program].samples[_ioUnit.block].levelFadeDelayRL);
}

void SoundChannel_Amiga::setRepeatData(const int8 *data, uint16 size) {
	_ioUnit.repeatData = data;
	_ioUnit.repeatDataSize = size >> 1;
}

void SoundChannel_Amiga::setVelocity(uint8 velo, int delay) {
	_ioUnit.levelFadeTriggerDC = 0;

	if (delay) {
		_ioUnit.fadeTargetLevel = velo;
		_ioUnit.fadeLevelDelta = ABS(_ioUnit.currentLevel - velo);
		_ioUnit.fadeLevelTicks = (delay << 10) / 5500;
		_ioUnit.fadeLevelMod = (_ioUnit.currentLevel >= velo) ? -1 : 1;
		_ioUnit.fadeLevelTicker = _ioUnit.fadeLevelDuration = 0;
	} else {
		_driver->setChannelVolume(_id, _volTable[(_ioUnit.volume << 5) + velo]);
		_ioUnit.currentLevel = _ioUnit.fadeTargetLevel = velo;
		_ioUnit.fadeLevelMod = 0;
	}
}

void SoundChannel_Amiga::setVolume(uint8 volume) {
	volume >>= 1;
	_ioUnit.volume = volume;
	_driver->setChannelVolume(_id, _volTable[(volume << 5) + _ioUnit.currentLevel]);
}

uint16 SoundChannel_Amiga::calculatePeriod(int16 tone, uint8 baseNote, uint16 rate) {
	static const uint32 octavePeriods[13] = { 0x4000, 0x43CE, 0x47D7, 0x4C1B, 0x50A2, 0x556D, 0x5A82, 0x5FE4, 0x6598, 0x6BA2, 0x7209, 0x78D0, 0x8000 };

	int16 frq_coarse = tone >> 7;
	uint8 frq_fine = tone & 0x7F;
	int16 octTrans = baseNote;
	rate <<= 3;

	for (int16 octTransHi = baseNote + 12; octTransHi <= frq_coarse; octTransHi += 12) {
		rate >>= 1;
		octTrans = octTransHi;
	}

	while (octTrans > frq_coarse) {
		rate += rate;
		octTrans -= 12;
	}

	uint32 res = (((octavePeriods[11 - (frq_coarse - octTrans)] * rate) >> 18) * frq_fine + ((octavePeriods[12 - (frq_coarse - octTrans)] * rate) >> 18) * (0x80 - frq_fine)) >> 7;

	if (!res)
		return 124;

	while (res < 124)
		res += res;

	if (res > 65535)
		res = 65535;

	return res & 0xFFFF;
}

void SoundChannel_Amiga::createVolumeTable() {
	if (_volTable)
		return;

	uint8 *volTbl = new uint8[2048];
	for (int a = 0; a < 64; ++a) {
		volTbl[a << 5] = 0;
		for (int b = 1; b < 32; ++b)
			volTbl[(a << 5) + b] = (a * (b + 1)) >> 5;
	}
	_volTable = volTbl;
}

uint8 SoundChannel_Amiga::_allocCurPos = 0;

const uint8 *SoundChannel_Amiga::_volTable = nullptr;

SoundChannel_Amiga *SoundChannel_Amiga::_channels[4] = { nullptr, nullptr, nullptr, nullptr };

const int8 SoundChannel_Amiga::_muteData[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

IMusePart_Amiga::IMusePart_Amiga(IMuseDriver_Amiga *driver, int id) : _driver(driver), _id(id), _allocated(false), _out(nullptr), _priority(0), _program(0),
	_pitchBend(0), _pitchBendSensitivity(2), _volume(0), _modulation(0), _transpose(0), _detune(0), _sustain(false) {
}

bool IMusePart_Amiga::allocate() {
	if (_allocated)
		return false;

	_allocated = true;

	while (_out)
		_out->disconnect();

	return true;
}

void IMusePart_Amiga::release() {
	_allocated = false;

	while (_out)
		_out->disconnect();
}

void IMusePart_Amiga::send(uint32 b) {
	_driver->send(b | _id);
}

void IMusePart_Amiga::noteOff(byte note) {
	for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next()) {
		if (note == cur->getNote()) {
			if (_sustain)
				cur->ctrl_sustain(true);
			else
				cur->disconnect();
		}
	}
}

void IMusePart_Amiga::noteOn(byte note, byte velocity) {
	if (!velocity) {
		noteOff(note);
		return;
	}

	SoundChannel_Amiga *chan = SoundChannel_Amiga::allocate(_priority);
	if (!chan)
		return;

	chan->connect(this);
	// The velocity parameter is ignored here.
	chan->noteOn(note, _volume, _program, _transpose, ((_pitchBend * _pitchBendSensitivity) >> 6) + _detune);
}

void IMusePart_Amiga::controlChange(byte control, byte value) {
	switch (control) {
	case 1:
		controlModulationWheel(value);
		break;
	case 7:
		controlVolume(value);
		break;
	case 10:
		// The original driver has no support for this.
		break;
	case 64:
		controlSustain(value);
		break;
	case 123:
		while (_out)
			_out->disconnect();
		break;
	default:
		break;
	}
}

void IMusePart_Amiga::programChange(byte program) {
	_program = program;
}

void IMusePart_Amiga::pitchBend(int16 bend) {
	_pitchBend = bend;
	for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next())
		cur->transposePitchBend(_transpose, ((_pitchBend * _pitchBendSensitivity) >> 6) + _detune);
}

void IMusePart_Amiga::pitchBendFactor(byte value) {
	_pitchBendSensitivity = value;
	for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next())
		cur->transposePitchBend(_transpose, ((_pitchBend * _pitchBendSensitivity) >> 6) + _detune);
}

void IMusePart_Amiga::transpose(int8 value) {
	_transpose = value << 1;
	for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next())
		cur->transposePitchBend(_transpose, ((_pitchBend * _pitchBendSensitivity) >> 6) + _detune);
}

void IMusePart_Amiga::detune(int16 value) {
	_detune = (int8)value;
	for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next())
		cur->transposePitchBend(_transpose, ((_pitchBend * _pitchBendSensitivity) >> 6) + _detune);
}

void IMusePart_Amiga::priority(byte value) {
	_priority = value;
}

void IMusePart_Amiga::controlModulationWheel(byte value) {
	_modulation = (int8)value;
}

void IMusePart_Amiga::controlVolume(byte value) {
	_volume = value;
	for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next())
		cur->ctrl_volume(_volume);
}

void IMusePart_Amiga::controlSustain(byte value) {
	_sustain = value;
	if (!value) {
		for (SoundChannel_Amiga *cur = _out; cur; cur = cur->next())
			cur->ctrl_sustain(false);
	}
}

IMuseDriver_Amiga::IMuseDriver_Amiga(Audio::Mixer *mixer) : Paula(true, mixer->getOutputRate(), (mixer->getOutputRate() * 1000) / 181818), _mixer(mixer), _isOpen(false), _soundHandle(),
	_numParts(24), _baseTempo(5500), _internalTempo(5500), _timerProc(nullptr), _timerProcPara(nullptr), _parts(nullptr), _chan(nullptr), _instruments(nullptr), _missingFiles(0), _ticker(0) {
	setAudioFilter(true);

	_instruments = new Instrument_Amiga[129]();
	loadInstrument(128);

	_parts = new IMusePart_Amiga*[_numParts];
	for (int i = 0; i < _numParts; i++)
		_parts[i] = new IMusePart_Amiga(this, i);

	_chan = new SoundChannel_Amiga*[4];
	for (int i = 0; i < 4; i++)
		_chan[i] = new SoundChannel_Amiga(this, i, _instruments);
}

IMuseDriver_Amiga::~IMuseDriver_Amiga() {
	close();

	Common::StackLock lock(_mutex);

	if (_chan) {
		for (int i = 0; i < 4; i++)
			delete _chan[i];
		delete[] _chan;
	}
	_chan = nullptr;

	if (_parts) {
		for (int i = 0; i < _numParts; i++)
			delete _parts[i];
		delete[] _parts;
	}
	_parts = nullptr;

	delete[] _instruments;
}

int IMuseDriver_Amiga::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	// Load all instruments at once. The original will load the programs that are necessary for the currently playing
	// sounds into a fixed 100000 bytes buffer. The approach here needs more memory (approx. 480 KB for MI2), but we
	// can easily afford this and it saves me the trouble of implementing a loader into the imuse code. The original
	// loader is quite unpleasant, since it scans the whole imuse midi track for program change events and collects
	// the program numbers for each such event in a buffer. Afterwards these instruments will get loaded.
	for (int i = 0; i < 128; ++i)
		loadInstrument(i);

	// Actually not all of the .IMS files are required to play. Many of these contain copies of the same instruments.
	// Each floppy disk contains one of the .IMS files. This would reduce the number of necessary floppy disk changes
	// when playing from the floppy disks. Obviously we don't need the redundancy files. The error dialog will display
	// only the required files. These are different for MI2 and INDY4.
	if (_missingFiles) {
		Common::U32String message = _("This AMIGA version is missing (at least) the following file(s):\n\n");
		for (int i = 0; i < 11; ++i) {
			if (_missingFiles & (1 << i))
				message += Common::String::format("AMIGA%d.IMS\n", i + 1);
		}
		message += _("\nPlease copy these file(s) into the game data directory.\n\n");
		::GUI::displayErrorDialog(message);
		return MERR_DEVICE_NOT_AVAILABLE;
	}

	startPaula();
	_mixer->playStream(Audio::Mixer::kPlainSoundType,
		&_soundHandle, this, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_isOpen = true;

	return 0;
}

void IMuseDriver_Amiga::close() {
	if (!_isOpen)
		return;

	_isOpen = false;

	stopPaula();
	setTimerCallback(nullptr, nullptr);
	_mixer->stopHandle(_soundHandle);

	Common::StackLock lock(_mutex);

	unloadInstruments();

	g_system->delayMillis(20);
}

void IMuseDriver_Amiga::send(uint32 b) {
	if (!_isOpen)
		return;

	byte param2 = (b >> 16) & 0xFF;
	byte param1 = (b >> 8) & 0xFF;
	byte cmd = b & 0xF0;

	IMusePart_Amiga *p = _parts[b & 0x0F];

	switch (cmd) {
	case 0x80:
		p->noteOff(param1);
		break;
	case 0x90:
		p->noteOn(param1, param2);
		break;
	case 0xB0:
		p->controlChange(param1, param2);
		break;
	case 0xC0:
		p->programChange(param1);
		break;
	case 0xE0:
		p->pitchBend((param1 | (param2 << 7)) - 0x2000);
		break;
	case 0xF0:
		warning("IMuseDriver_Amiga: Receiving SysEx command on a send() call");
		break;

	default:
		break;
	}
}

void IMuseDriver_Amiga::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerProcPara = timer_param;
}

uint32 IMuseDriver_Amiga::getBaseTempo() {
	return _baseTempo;
}

MidiChannel *IMuseDriver_Amiga::allocateChannel() {
	if (!_isOpen)
		return nullptr;

	for (int i = 0; i < _numParts; ++i) {
		if (_parts[i]->allocate())
			return _parts[i];
	}

	return nullptr;
}

MidiChannel *IMuseDriver_Amiga::getPercussionChannel() {
	return nullptr;
}

void IMuseDriver_Amiga::interrupt() {
	if (!_isOpen)
		return;

	for (_ticker += _internalTempo; _ticker >= _baseTempo; _ticker -= _baseTempo) {
		updateParser();
		updateSounds();
	}
}

void IMuseDriver_Amiga::updateParser() {
	if (_timerProc)
		_timerProc(_timerProcPara);
}

void IMuseDriver_Amiga::updateSounds() {
	for (int i = 0; i < 4; i++)
		_chan[i]->updateLevel();
	for (int i = 0; i < 4; i++)
		_chan[i]->updateEnvelope();
}

void IMuseDriver_Amiga::loadInstrument(int program) {
	Common::StackLock lock(_mutex);

	if (program == 128) {
		// The hard-coded default instrument definitions and sample data are the same in MI2 and INDY4.
		static const int8 defaultData[16] = { 0, 49, 90, 117, 127, 117, 90, 49, 0, -49, -90, -117, -127, -117, -90, -49 };
		static const Instrument_Amiga::Samples defaultSamples = { 428, 60, 0, 127, 33, 0, /*0, 0,*/16, 0, 0, 5, 300, 5, 100, defaultData };
		_instruments[128].numBlocks = 1;
		memcpy(&_instruments[128].samples[0], &defaultSamples, sizeof(Instrument_Amiga::Samples));
	}

	if (program > 127)
		return;

	Common::File ims;
	int32 header[10];
	uint32 offset = 0;
	memset(header, 0, sizeof(header));

	for (int i = 0; i < 8; ++i) {
		if (_instruments[program].samples[i].data) {
			delete[] _instruments[program].samples[i].data;
			_instruments[program].samples[i].data = nullptr;
		}
	}

	for (int fileNo = 1; fileNo != -1 && !ims.isOpen(); ) {
		if (!ims.open(Common::Path(Common::String::format("amiga%d.ims", fileNo)))) {
			_missingFiles |= (1 << (fileNo - 1));
			return;
		}

		ims.seek(16 + (program << 2), SEEK_SET);
		offset = ims.readUint32BE();
		if (offset & 0x40000000) {
			offset &= ~0x40000000;
			ims.seek(16 + (offset << 2), SEEK_SET);
			offset = ims.readUint32BE();
		}

		if (offset & 0x80000000) {
			offset &= ~0x80000000;
			ims.close();
			fileNo = offset ? offset : -1;
		} else {
			ims.seek(552 + offset, SEEK_SET);
			for (int i = 0; i < 10; ++i)
				header[i] = ims.readSint32BE();
		}
	}

	if (!ims.isOpen())
		return;

	for (int block = 0; block < 8; ++block) {
		int size = 0;

		if (header[block] != -1)
			size = (block != 7 && header[block + 1] != -1 ? header[block + 1] : header[9]) - header[block];

		if (size <= 0)
			break;

		size -= 38;
		Instrument_Amiga::Samples *s = &_instruments[program].samples[block];
		ims.seek(594 + offset + header[block], SEEK_SET);
		int8 *buf = new int8[size];

		s->rate = ims.readUint16BE();
		s->baseNote = ims.readUint16BE();
		s->noteRangeMin = ims.readSint16BE();
		s->noteRangeMax = ims.readSint16BE();
		s->sustainLevel = ims.readSint16BE();
		s->type = ims.readUint16BE();
		ims.skip(8);
		s->numSamples = size;
		s->dr_offset = ims.readUint32BE();
		s->dr_numSamples = ims.readUint32BE();
		s->levelFadeDelayAT = ims.readSint16BE();
		s->levelFadeDelayRL = ims.readSint16BE();
		s->levelFadeTriggerRL = ims.readSint16BE();
		s->levelFadeDelayDC = ims.readSint16BE();
		ims.read(buf, size);
		s->data = buf;
		_instruments[program].numBlocks = block + 1;
	}

	ims.close();
}

void IMuseDriver_Amiga::unloadInstruments() {
	Common::StackLock lock(_mutex);
	for (int prg = 0; prg < 128; ++prg) {
		for (int block = 0; block < 8; ++block) {
			if (_instruments[prg].samples[block].data)
				delete[] _instruments[prg].samples[block].data;
		}
	}
	memset(_instruments, 0, sizeof(Instrument_Amiga) * 128);
}

}
