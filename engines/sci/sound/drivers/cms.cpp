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

#include "sci/sound/drivers/mididriver.h"

#include "audio/softsynth/emumidi.h"
#include "audio/softsynth/cms.h"
#include "audio/mixer.h"

#include "common/system.h"

#include "sci/resource/resource.h"
#include "sci/util.h"

#define SCI0_CMS_ORIGINAL_BUG		1

namespace Sci {

class MidiDriver_CMS;

class CMSVoice {
public:
	CMSVoice(uint8 id, MidiDriver_CMS *driver, CMSEmulator *cms, SciSpan<const uint8>& patchData);
	virtual ~CMSVoice() {}

	virtual void noteOn(int note, int velocity) = 0;
	virtual void noteOff() = 0;
	virtual void stop() = 0;
	virtual void programChange(int program) = 0;
	virtual void pitchWheel() {}

	virtual void update() = 0;

	virtual void reset() {}
	virtual void setPanMask(uint8) {}

	uint8 _assign;
	uint8 _note;
	bool _sustained;
	uint16 _duration;
	uint16 _releaseDuration;
	CMSVoice *_secondaryVoice;

protected:
	void sendFrequency();
	void cmsWrite(uint8 reg, uint8 val);

	CMSEmulator *_cms;
	MidiDriver_CMS *_driver;
	SciSpan<const uint8> _patchData;

	const uint8 _id;
	const uint8 _regOffset;
	const uint8 _portOffset;

	static uint8 _octaveRegs[6];
	static const int _frequencyTable[48];

private:
	virtual void recalculateFrequency(uint8 &freq, uint8 &octave) = 0;
};

class CMSVoice_V0 : public CMSVoice {
public:
	CMSVoice_V0(uint8 id, MidiDriver_CMS *driver, CMSEmulator *cms, SciSpan<const uint8>& patchData);
	~CMSVoice_V0() override {}

	void noteOn(int note, int) override;
	void noteOff() override;
	void stop() override;
	void programChange(int program) override;

	void update() override;

	void reset() override;
	void setPanMask(uint8 mask) override;

private:
	void recalculateFrequency(uint8 &frequency, uint8 &octave) override;
	void recalculateEnvelopeLevels();
	void selectEnvelope(int id);

	enum EnvelopeState {
		kReady		= 0,
		kRestart	= 1,
		kAttack		= 2,
		kDecay		= 3,
		kSustain	= 4,
		kRelease	= 5		
	};
	
	EnvelopeState _envState;
	uint8 _envAR;
	uint8 _envTL;
	uint8 _envDR;
	uint8 _envSL;
	uint8 _envRR;	
	uint8 _envSLI;
	uint8 _envPAC;
	uint8 _envPA;
#ifdef SCI0_CMS_ORIGINAL_BUG
	static uint8  _envAR1;
#endif

	uint8 _envNote;
	uint8 _envSSL;
	uint8 _panMask;
	uint8 _strMask;

	int8 _transFreq;
	int8 _transOct;

	bool _vbrOn;
	uint8 _vbrSteps;
	uint8 _vbrState;
	int8 _vbrMod;
	int8 _vbrCur;
	int16 _vbrPhase;

	int _currentLevel;
	bool _updateCMS;

	const bool _isSecondary;

	static const uint8 _volumeTable[176];
	static const uint8 _pitchWheelTable[65];
};

class CMSVoice_V1 : public CMSVoice {
public:
	CMSVoice_V1(uint8 id, MidiDriver_CMS *driver, CMSEmulator *cms, SciSpan<const uint8>& patchData);
	~CMSVoice_V1() override {}

	void noteOn(int note, int velocity) override;
	void noteOff() override;
	void stop() override;
	void programChange(int program) override;
	void pitchWheel() override;

	void update() override;

private:
	void recalculateFrequency(uint8 &frequency, uint8 &octave) override;

	void updateVoiceAmplitude();
	void setupVoiceAmplitude();

	SciSpan<const uint8> _patchDataCur;
	uint8 _velocity;
	uint8 _patchDataIndex;
	uint8 _amplitudeTimer;
	uint8 _amplitudeModifier;
	bool _release;

	static const int _velocityTable[32];
};

class MidiDriver_CMS : public MidiDriver_Emulated {
public:
	enum {
		MIDI_PROP_CHANNEL_VOLUME = 1,
		MIDI_PROP_CHANNEL_PITCHWHEEL = 2,
		MIDI_PROP_CHANNEL_PANPOS = 3,
		MIDI_PROP_PLAYSWITCH = 4
	};

public:
	MidiDriver_CMS(Audio::Mixer *mixer, ResourceManager *resMan, SciVersion version);
	~MidiDriver_CMS() override;

	int open() override;
	void close() override;

	void send(uint32 b) override;
	uint32 property(int prop, uint32 param) override;

	void initTrack(SciSpan<const byte>& header);

	void onTimer() override;

	MidiChannel *allocateChannel() override { return 0; }
	MidiChannel *getPercussionChannel() override { return 0; }

	bool isStereo() const override { return true; }
	int getRate() const override { return _rate; }

private:
	void noteOn(int channelNr, int note, int velocity);
	void noteOff(int channelNr, int note);
	void controlChange(int channelNr, int control, int value);
	void programChange(int channelNr, int value);
	void pitchWheel(int channelNr, int value);

	void voiceMapping(int channelNr, int value);
	void bindVoices(int channelNr, int voices, bool bindSecondary, bool doProgramChange);
	void unbindVoices(int channelNr, int voices, bool bindSecondary);
	void donateVoices(bool bindSecondary);
	int findVoice(int channelNr, int note);
	int findVoiceBasic(int channelNr);

	void writeToChip(int chip, int address, int data);
	void generateSamples(int16 *buffer, int len) override;

	struct Channel {
		Channel() : program(0), volume(0), pan(0x40), hold(0), missingVoices(0), lastVoiceUsed(0), pitchWheel(0x2000), isValid(true) {}
		uint8 program;
		uint8 volume;
		uint8 pan;
		uint8 hold;
		uint8 missingVoices;
		uint8 lastVoiceUsed;
		uint16 pitchWheel;
		bool isValid;
	};

	Channel _channel[16];
	CMSVoice *_voice[12];

	const int _numVoicesPrimary;
	const int _numVoicesSecondary;

	CMSEmulator *_cms;
	ResourceManager *_resMan;
	Common::SpanOwner<SciSpan<const uint8> > _patchData;

	bool _playSwitch;
	uint16 _masterVolume;

	const int _actualTimerInterval;
	const int _reqTimerInterval;
	int _updateTimer;
	int _rate;

	SciVersion _version;
};

CMSVoice::CMSVoice(uint8 id, MidiDriver_CMS* driver, CMSEmulator *cms, SciSpan<const uint8>& patchData) : _id(id), _regOffset(id > 5 ? id - 6 : id), _portOffset(id > 5 ? 2 : 0),
	_driver(driver), _cms(cms), _assign(0xFF), _note(0xFF), _sustained(false), _duration(0), _releaseDuration(0), _secondaryVoice(0), _patchData(patchData) {
	assert(_id < 12);
	_octaveRegs[_id >> 1] = 0;
}

void CMSVoice::sendFrequency() {
	uint8 frequency = 0;
	uint8 octave = 0;

	recalculateFrequency(frequency, octave);	

	uint8 octaveData = _octaveRegs[_id >> 1];
	octaveData = (_id & 1) ? (octaveData & 0x0F) | (octave << 4) : (octaveData & 0xF0) | octave;

	cmsWrite(8 + _regOffset, frequency);
	cmsWrite(0x10 + (_regOffset >> 1), octaveData);
}

void CMSVoice::cmsWrite(uint8 reg, uint8 val) {
	_cms->portWrite(0x221 + _portOffset, reg);
	_cms->portWrite(0x220 + _portOffset, val);

	if (reg >= 16 && reg <= 18)
		_octaveRegs[_id >> 1] = val;
}

uint8 CMSVoice::_octaveRegs[6] = {
	0, 0, 0, 0, 0, 0
};

const int CMSVoice::_frequencyTable[48] = {
	  3,  10,  17,  24,
	 31,  38,  46,  51,
	 58,  64,  71,  77,
	 83,  89,  95, 101,
	107, 113, 119, 124,
	130, 135, 141, 146,
	151, 156, 162, 167,
	172, 177, 182, 186,
	191, 196, 200, 205,
	209, 213, 217, 222,
	226, 230, 234, 238,
	242, 246, 250, 253
};

#ifdef SCI0_CMS_ORIGINAL_BUG
uint8 CMSVoice_V0::_envAR1 = 0;
#endif

CMSVoice_V0::CMSVoice_V0(uint8 id, MidiDriver_CMS* driver, CMSEmulator *cms, SciSpan<const uint8>& patchData) : CMSVoice(id, driver, cms, patchData), _envState(kReady), _currentLevel(0), _strMask(0),
	_envAR(0), _envTL(0), _envDR(0), _envSL(0), _envRR(0), _envSLI(0), _vbrOn(false), _vbrSteps(0), _vbrState(0), _vbrMod(0), _vbrCur(0), _isSecondary(id > 7),
	_vbrPhase(0), _transOct(0), _transFreq(0), _envPAC(0), _envPA(0), _panMask(_id & 1 ? 0xF0 : 0x0F), _envSSL(0), _envNote(0xFF), _updateCMS(false) {
#ifdef SCI0_CMS_ORIGINAL_BUG
	_envAR1 = 0;
#endif
}

void CMSVoice_V0::noteOn(int note, int) {
	if (!_driver->property(MidiDriver_CMS::MIDI_PROP_PLAYSWITCH, 0xFFFF) || !_envTL)
		return;

	_note = note;
	_envNote = note + 3;
	_envState = kRestart;
	_vbrPhase = 0;
	_vbrCur = _vbrMod;
	_vbrState = _vbrSteps & 0x0F;
	_envPAC = _envPA;

	if (_secondaryVoice)
		_secondaryVoice->noteOn(note, 127);
}

void CMSVoice_V0::noteOff() {
	if (!_driver->property(MidiDriver_CMS::MIDI_PROP_PLAYSWITCH, 0xFFFF) || !_envTL)
		return;

	_note = 0xFF;
	_envState = kRelease;
	if (_secondaryVoice)
		_secondaryVoice->noteOff();
}

void CMSVoice_V0::stop() {
	_note = 0xFF;
	if (_envState != kReady)
		_envState = kRelease;
	if (_secondaryVoice)
		_secondaryVoice->stop();
}

void CMSVoice_V0::programChange(int program) {
	if (program > 127) {
		// I encountered this with PQ2 at the airport (program 204 sent on part 13). The original driver does not really handle that.
		// In fact, it even interprets this value as signed so it will not point into the instrument data buffer, but into a random
		// invalid memory location (in the case of 204 it will read a value of 8 from the device init data array). Since there seems
		// to be no effect on the sound I don't emulate this (mis)behaviour. 
		warning("CMSVoice_V0::programChange:: Invalid program '%d' requested on midi channel '%d'", program, _assign);
		program = 0;
	} else if (program == 127) {
		// This seems to replace the start of track offset with the current position so that 0xFC (kEndOfTrack)
		// midi events would not reset the track to the start, but to the current position instead. This cannot
		// be handled here. All versions of the SCI0 driver that I have seen so far do this. Still, I somehow
		// doubt that it will ever come up, but let's see...
		warning("CMSVoice_V0::programChange(): Unhandled program change 127");
		return;
	}

	SciSpan<const uint8> data = _patchData.subspan(128 + (_patchData.getUint8At(program) << 3));
	uint8 pos = _isSecondary ? 3 : 0;

	selectEnvelope(data.getUint8At(pos++));

	if (_isSecondary) {
		_envSSL = data.getUint8At(pos++);
		// This decides whether the secondary voice has the same or the opposite pan position as the primary voice.
		_panMask = _strMask ^ -(_envSSL & 1);
	}

	_transOct = data.getInt8At(pos++);
	_transFreq = data.getInt8At(pos++);

	if (_isSecondary)
		_envPA = data.getUint8At(pos++);

	if (_secondaryVoice) {
		assert(!_isSecondary);
		if (data.getUint8At(pos) == 0xFF) {
			_secondaryVoice->stop();
			_secondaryVoice->_assign = 0xFF;
			_secondaryVoice = 0;
		} else {
			_secondaryVoice->setPanMask(_panMask);
			_secondaryVoice->programChange(program);
		}
	}
}

void CMSVoice_V0::update() {
	if (_updateCMS) {
		sendFrequency();
		cmsWrite(_regOffset, ((_currentLevel & 0xF0) | (_currentLevel >> 4)) & _panMask);
		_updateCMS = false;
	}

	recalculateEnvelopeLevels();

	switch (_envState) {
	case kReady:
		_envNote = 0xFF;
		return;

	case kRestart:
		if (_envPAC) {
			--_envPAC;
			break;
		} else {
#ifdef SCI0_CMS_ORIGINAL_BUG
			// This is bugged in two ways. The attack rate value is misinterpreted as a int8 for the comparison (the only place in the code
			// where it does that). And it always uses the attack rate of voice no. 1 as a modifier instead of the voice's own attack rate.
			// Keeping these bugs will result in faithful audio. It probably even sounds as intended, since the sequencer will have likely
			// suffered from the same bug and calculated the deltas based on that...
			_currentLevel = ((_currentLevel >> 1) > (int8)_envAR)?((_currentLevel >> 1) - _envAR1) & 0xFF : (_envAR - _envAR1) & 0xFF;
#else
			_currentLevel = ((_currentLevel >> 1) > _envAR) ? ((_currentLevel >> 1) - _envAR) : 0;
#endif
			_envState = kAttack;
		}
		// fall through

	case kAttack:
		_currentLevel = _currentLevel + _envAR;
		if (_currentLevel > _envTL || _currentLevel > 0xFF) {
			_currentLevel = _envTL;
			_envState = kDecay;
		}
		break;

	case kDecay:
		_currentLevel -= _envDR;
		if (_currentLevel <= _envSL) {
			if (_currentLevel < 0)
				_currentLevel = 0;
			_envState = kSustain;
		}
		break;

	case kSustain:
		_currentLevel = _envSL;
		break;

	case kRelease:
		_currentLevel -= _envRR;
		if (_currentLevel < 0) {
			_currentLevel = 0;
			_envState = kReady;
		}
		break;

	default:
		break;
	}

	if (_vbrOn && _envState != kRestart) {
		_vbrPhase += _vbrCur;
		if (!--_vbrState) {
			_vbrCur = -_vbrCur;
			_vbrState = (_vbrSteps & 0x0F) << 1;
		}
	}

	_updateCMS = true;
	++_duration;
}

void CMSVoice_V0::reset() {
	_envState = kReady;
	_secondaryVoice = 0;
	_assign = _note = _envNote = 0xFF;
	_panMask = _id & 1 ? 0xF0 : 0x0F;
	_envTL = 0;
	_currentLevel = 0;
	_duration = 0;
	_envPA = 0;
	_transFreq = _transOct = 0;
	selectEnvelope(3);
}

void CMSVoice_V0::setPanMask(uint8 mask) {
	_strMask = mask;
}

void CMSVoice_V0::recalculateFrequency(uint8 &freq, uint8 &octave) {
	if (_assign == 0xFF || _envNote == 0xFF)
		return;

	uint8 note = _envNote % 12;
	octave = CLIP<int>(_envNote / 12 - 2, 0, 7);

	int16 pbVal = (_driver->property(MidiDriver_CMS::MIDI_PROP_CHANNEL_PITCHWHEEL, _assign) & 0x7FFF) - 0x2000;
	int16 pbEff = _pitchWheelTable[ABS(pbVal) >> 7] * ((pbVal < 0) ? -1 : 1);
	int frequency = note * 4 + pbEff;

	if (frequency < 0) {
		if (octave) {
			frequency += 48;
			--octave;
		} else {
			frequency = 0;
		}
	} else if (frequency >= 48) {
		if (octave < 7) {
			frequency -= 48;
			++octave;
		} else {
			frequency = 47;
		}
	}
	
	octave = CLIP<int8>(octave + _transOct, 0, 7);
	frequency = _frequencyTable[frequency & 0xFF] + _transFreq + _vbrPhase;

	if (frequency > 255) {
		frequency &= 0xFF;
		octave++;
	} else if (frequency < 0) {
		frequency &= 0xFF;
		octave--;
	}

	octave = CLIP<int8>(octave, 0, 7);
	freq = frequency;
}

void CMSVoice_V0::recalculateEnvelopeLevels() {
	uint8 chanVol = _driver->property(MidiDriver_CMS::MIDI_PROP_CHANNEL_VOLUME, _assign);

	if (_envTL && _isSecondary) {
		int volIndexTLS = (chanVol >> 4) | (_envSSL & 0xF0);
		assert(volIndexTLS < ARRAYSIZE(_volumeTable));
		_envTL = _volumeTable[volIndexTLS];
	} else if (_envTL) {
		_envTL = chanVol;
	}
	
	int volIndexSL = (_envSLI << 4) + (_envTL >> 4);
	assert(volIndexSL < ARRAYSIZE(_volumeTable));
	_envSL = _volumeTable[volIndexSL];
}

void CMSVoice_V0::selectEnvelope(int id) {
	SciSpan<const uint8> in = _patchData.subspan(512 + ((id & 0x1F) << 3));
	_envAR = *in++;
	_envTL = *in++;
	_envDR = *in++;
	_envSLI = *in++;
	_envRR = *in++;
	/*unused*/in++;
	_vbrMod = *in++;
	_vbrSteps = *in++;
	_vbrOn = _vbrMod;
	_vbrCur = _vbrMod;
	_vbrState = _vbrSteps & 0x0F;
	_vbrPhase = 0;
#ifdef SCI0_CMS_ORIGINAL_BUG
	if (_id == 1)
		_envAR1 = _envAR;
#endif
}

const uint8 CMSVoice_V0::_volumeTable[176] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10,	0x10, 0x10, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x20,	0x20, 0x20, 0x30, 0x30, 0x30, 0x30, 0x40, 0x40,
	0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x20,	0x30, 0x30, 0x40, 0x40, 0x40, 0x50, 0x50, 0x60,
	0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x30, 0x30,	0x40, 0x40, 0x50, 0x50, 0x60, 0x60, 0x70, 0x70,
	0x00, 0x00, 0x10, 0x10, 0x20, 0x30, 0x30, 0x40,	0x40, 0x50, 0x60, 0x60, 0x70, 0x70, 0x80, 0x90,
	0x00, 0x00, 0x10, 0x20, 0x20, 0x30, 0x40, 0x40,	0x50, 0x60, 0x70, 0x70, 0x80, 0x90, 0x90, 0xa0,
	0x00, 0x00, 0x10, 0x20, 0x30, 0x40, 0x40, 0x50,	0x60, 0x70, 0x80, 0x80, 0x90, 0xa0, 0xb0, 0xc0,
	0x00, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60,	0x70, 0x80, 0x90, 0x90, 0xa0, 0xb0, 0xc0, 0xd0,
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,	0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0
};

const uint8 CMSVoice_V0::_pitchWheelTable[65] = {
	0x00, 0x01, 0x02, 0x02, 0x03, 0x04, 0x05, 0x05,
	0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0b,
	0x0c, 0x0d, 0x0e, 0x0e, 0x0f, 0x10, 0x11, 0x11,
	0x12, 0x13, 0x14, 0x14, 0x15, 0x16, 0x17, 0x17,
	0x18, 0x19, 0x1a, 0x1a, 0x1b, 0x1c, 0x1d, 0x1d,
	0x1e, 0x1f, 0x20, 0x20, 0x21, 0x22, 0x23, 0x23,
	0x24, 0x25, 0x26, 0x26, 0x27, 0x28, 0x29, 0x29,
	0x2a, 0x2b, 0x2c, 0x2c, 0x2d, 0x2e, 0x2f, 0x2f,
	0x30
};

CMSVoice_V1::CMSVoice_V1(uint8 id, MidiDriver_CMS* driver, CMSEmulator *cms, SciSpan<const uint8>& patchData) : CMSVoice(id, driver, cms, patchData), _velocity(0), _patchDataIndex(0),
	_amplitudeTimer(0), _amplitudeModifier(0), _release(false) {
}

void CMSVoice_V1::noteOn(int note, int velocity) {
	_note = note;
	_release = false;
	_patchDataIndex = 0;
	_amplitudeTimer = 0;
	_duration = 0;
	_releaseDuration = 0;
	_velocity = velocity ? _velocityTable[velocity >> 3] : 0;
	sendFrequency();
}

void CMSVoice_V1::noteOff() {
	_release = true;
}

void CMSVoice_V1::stop() {
	_velocity = 0;
	_note = 0xFF;
	_sustained = false;
	_release = false;
	_patchDataIndex = 0;
	_amplitudeTimer = 0;
	_amplitudeModifier = 0;
	_duration = 0;
	_releaseDuration = 0;

	setupVoiceAmplitude();
}

void CMSVoice_V1::programChange(int program) {
	_patchDataCur = _patchData.subspan(_patchData.getUint16LEAt(program << 1));
}

void CMSVoice_V1::pitchWheel() {
	sendFrequency();
}

void CMSVoice_V1::update() {
	if (_note == 0xFF)
		return;

	if (_release)
		++_releaseDuration;
	++_duration;

	updateVoiceAmplitude();
	setupVoiceAmplitude();
}

void CMSVoice_V1::recalculateFrequency(uint8 &freq, uint8 &octave) {
	assert(_assign != 0xFF);

	int frequency = (CLIP<int>(_note, 21, 116) - 21) * 4;
	int16 pw = _driver->property(MidiDriver_CMS::MIDI_PROP_CHANNEL_PITCHWHEEL, _assign);
	int modifier = (pw < 0x2000) ? (0x2000 - pw) / 170 : ((pw > 0x2000) ? (pw - 0x2000) / 170 : 0);

	if (modifier) {
		if (pw < 0x2000) {
			if (frequency > modifier)
				frequency -= modifier;
			else
				frequency = 0;
		} else {
			int tempFrequency = 384 - frequency;
			if (modifier < tempFrequency)
				frequency += modifier;
			else
				frequency = 383;
		}
	}

	octave = 0;
	while (frequency >= 48) {
		frequency -= 48;
		++octave;
	}

	freq = _frequencyTable[frequency & 0xFF];
}

void CMSVoice_V1::updateVoiceAmplitude() {
	if (_amplitudeTimer != 0 && _amplitudeTimer != 254) {
		--_amplitudeTimer;
		return;
	} else if (_amplitudeTimer == 254) {
		if (!_release)
			return;
		_amplitudeTimer = 0;
	}

	int nextDataIndex = _patchDataIndex;
	uint8 timerData = 0;
	uint8 amplitudeData = _patchDataCur[nextDataIndex];

	if (amplitudeData == 255) {
		timerData = amplitudeData = 0;
		stop();
	} else {
		timerData = _patchDataCur[nextDataIndex + 1];
		nextDataIndex += 2;
	}

	_patchDataIndex = nextDataIndex;
	_amplitudeTimer = timerData;
	_amplitudeModifier = amplitudeData;
}

void CMSVoice_V1::setupVoiceAmplitude() {
	assert(_assign != 0xFF);
	uint amplitude = 0;
	uint8 chanVolume = _driver->property(MidiDriver_CMS::MIDI_PROP_CHANNEL_VOLUME, _assign);
	uint8 masterVolume = _driver->property(MIDI_PROP_MASTER_VOLUME, 0xFFFF);

	if (chanVolume && _velocity && _amplitudeModifier && masterVolume) {
		amplitude = chanVolume * _velocity;
		amplitude /= 0x0F;
		amplitude *= _amplitudeModifier;
		amplitude /= 0x0F;
		amplitude *= masterVolume;
		amplitude /= 0x0F;

		if (!amplitude)
			++amplitude;
	}

	uint8 amplitudeData = 0;
	int pan = _driver->property(MidiDriver_CMS::MIDI_PROP_CHANNEL_PANPOS, _assign) >> 2;
	if (pan >= 16) {
		amplitudeData = (amplitude * (31 - pan) / 0x0F) & 0x0F;
		amplitudeData |= (amplitude << 4);
	} else {
		amplitudeData = (amplitude * pan / 0x0F) & 0x0F;
		amplitudeData <<= 4;
		amplitudeData |= amplitude;
	}

	if (!_driver->property(MidiDriver_CMS::MIDI_PROP_PLAYSWITCH, 0xFFFF))
		amplitudeData = 0;

	cmsWrite(_regOffset, amplitudeData);
}

const int CMSVoice_V1::_velocityTable[32] = {
	 1,  3,  6,  8,  9, 10, 11, 12,
	12, 13, 13, 14, 14, 14, 15, 15,
	 0,  1,  2,  2,  3,  4,  4,  5,
	 6,  6,  7,  8,  8,  9, 10, 10
};

MidiDriver_CMS::MidiDriver_CMS(Audio::Mixer* mixer, ResourceManager* resMan, SciVersion version) : MidiDriver_Emulated(mixer), _resMan(resMan),
	_version(version), _cms(0), _rate(0), _playSwitch(true), _masterVolume(0), _numVoicesPrimary(version > SCI_VERSION_0_LATE ? 12 : 8),
	_actualTimerInterval(1000000 / _baseFreq), _reqTimerInterval(1000000/60), _numVoicesSecondary(version > SCI_VERSION_0_LATE ? 0 : 4) {
	memset(_voice, 0, sizeof(_voice));
	_updateTimer = _reqTimerInterval;
}

MidiDriver_CMS::~MidiDriver_CMS() {
	for (int i = 0; i < 12; ++i)
		delete _voice[i];
}

int MidiDriver_CMS::open() {
	if (_cms)
		return MERR_ALREADY_OPEN;

	assert(_resMan);
	Resource *res = _resMan->findResource(ResourceId(kResourceTypePatch, 101), false);
	if (!res)
		return -1;

	_patchData->allocateFromSpan(_version < SCI_VERSION_1_EARLY ? res->subspan(30) : *res);

	_rate = _mixer->getOutputRate();
	_cms = new CMSEmulator(_rate);
	assert(_cms);	
	
	for (uint i = 0; i < ARRAYSIZE(_channel); ++i)
		_channel[i] = Channel();

	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_version < SCI_VERSION_1_EARLY)
			_voice[i] = new CMSVoice_V0(i, this, _cms, *_patchData);
		else
			_voice[i] = new CMSVoice_V1(i, this, _cms, *_patchData);
	}
	
	_playSwitch = true;
	_masterVolume = 0;

	for (int i = 0; i < 31; ++i) {
		writeToChip(0, i, 0);
		writeToChip(1, i, 0);
	}

	// Enable frequency for all channels
	writeToChip(0, 0x14, _version < SCI_VERSION_1_EARLY ? 0x3F : 0xFF);
	writeToChip(1, 0x14, _version < SCI_VERSION_1_EARLY ? 0x3F : 0xFF);

	// Sync and reset generators
	writeToChip(0, 0x1C, 2);
	writeToChip(1, 0x1C, 2);

	// Enable all channels
	writeToChip(0, 0x1C, 1);
	writeToChip(1, 0x1C, 1);

	int retVal = MidiDriver_Emulated::open();
	if (retVal != 0)
		return retVal;

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);
	return 0;
}

void MidiDriver_CMS::close() {
	_mixer->stopHandle(_mixerSoundHandle);

	_patchData.clear();
	delete _cms;
	_cms = nullptr;
}

void MidiDriver_CMS::send(uint32 b) {
	const uint8 command = b & 0xf0;
	const uint8 channel = b & 0xf;
	const uint8 op1 = (b >> 8) & 0xff;
	const uint8 op2 = (b >> 16) & 0xff;

	// This is a SCI0 only feature. For SCI1 we simply set all channels to valid by default so that this check will always pass.
	if (!_channel[channel].isValid)
		return;

	switch (command) {
	case 0x80:
		noteOff(channel, op1);
		break;

	case 0x90:
		noteOn(channel, op1, op2);
		break;

	case 0xB0:
		controlChange(channel, op1, op2);
		break;

	case 0xC0:
		programChange(channel, op1);
		break;

	case 0xE0:
		pitchWheel(channel, (op1 & 0x7f) | ((op2 & 0x7f) << 7));
		break;

	default:
		break;
	}
}

uint32 MidiDriver_CMS::property(int prop, uint32 param) {
	switch (prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff)
			_masterVolume = param;
		return _masterVolume;
	case MIDI_PROP_PLAYSWITCH:
		if (param != 0xffff)
			_playSwitch = param ? true : false;
		return _playSwitch ? 1 : 0;
	case MIDI_PROP_CHANNEL_VOLUME:
		return (param < 16) ? _channel[param].volume : 0;
	case MIDI_PROP_CHANNEL_PITCHWHEEL:
		return (param < 16) ? _channel[param].pitchWheel : 0;
	case MIDI_PROP_CHANNEL_PANPOS:
		return (param < 16) ? _channel[param].pan : 0;
	default:
		return MidiDriver_Emulated::property(prop, param);
	}
}

void MidiDriver_CMS::initTrack(SciSpan<const byte>& header) {
	if (!_isOpen || _version > SCI_VERSION_0_LATE)
		return;

	uint8 readPos = 0;
	uint8 caps = header.getInt8At(readPos++);
	int numChan = (caps == 2) ? 15 : 16;
	if (caps != 0 && caps != 2)
		return;

	for (int i = 0; i < 12; ++i)
		_voice[i]->reset();

	for (int i = 0; i < 16; ++i) {
		_channel[i].isValid = false;
		_channel[i].volume = 180;
		_channel[i].pitchWheel = 0x2000;
		_channel[i].pan = 0;

		if (i >= numChan)
			continue;

		uint8 num = header.getInt8At(readPos++) & 0x0F;
		uint8 flags = header.getInt8At(readPos++);

		if (num == 15 || !(flags & 4))
			continue;

		// Another strange thing about this driver... All channels to be used have to be marked as valid here
		// or they will be blocked in send(). Even control change voice mapping won't be accessible. This means
		// that a num == 0 setting could even make sense here, since it will mark that channel as valid for
		// later use (e.g. assigning voices via control change voice mapping).
		_channel[i].isValid = true;
		if (num == 0)
			continue;

		// This weird driver will assign a second voice if the number of requested voices is exactly 1.
		// The secondary voice is configured differently (has its own instrument patch data) and it
		// is controlled through the primary voice. It will not receive its own separate commands. The
		// main purpose seems providing stereo channels with 2 discrete voices for the left and right
		// speaker output. However, the instrument patch can also turn this around so that both voices
		// use the same panning. What an awesome concept...
		bindVoices(i, num, num == 1, false);
	}
}

void MidiDriver_CMS::onTimer() {
	for (_updateTimer -= _actualTimerInterval; _updateTimer <= 0; _updateTimer += _reqTimerInterval) {
		for (uint i = 0; i < ARRAYSIZE(_voice); ++i)
			_voice[i]->update();
	}
}

void MidiDriver_CMS::noteOn(int channelNr, int note, int velocity) {
	if (note < 21 || note > 116)
		return;

	if (velocity == 0) {
		noteOff(channelNr, note);
		return;
	}

	for (int i = 0; i < _numVoicesPrimary; ++i) {
		if (_voice[i]->_assign == channelNr && _voice[i]->_note == note) {
			if (_version > SCI_VERSION_0_LATE) {
				_voice[i]->stop();
				_voice[i]->programChange(_channel[channelNr].program);
			}
			_voice[i]->noteOn(note, velocity);
			return;
		}
	}

#ifdef CMS_DISABLE_VOICE_MAPPING
	int id = findVoiceBasic(channelNr);
#else
	int id = findVoice(channelNr, note);
#endif
	if (id != -1) {
		if (_version > SCI_VERSION_0_LATE)
			_voice[id]->programChange(_channel[channelNr].program);
		_voice[id]->noteOn(note, velocity);
	}
}

void MidiDriver_CMS::noteOff(int channelNr, int note) {
	for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
		if (_voice[i]->_assign == channelNr && _voice[i]->_note == note) {
			if (_channel[channelNr].hold != 0)
				_voice[i]->_sustained = true;
			else
				_voice[i]->noteOff();
		}
	}
}

void MidiDriver_CMS::controlChange(int channelNr, int control, int value) {
	// The original SCI0 CMS drivers do not have Midi control 123. I support it nonetheless,
	// since our current music engine seems to want to have it and it does not cause problems either.
	if (_version < SCI_VERSION_1_EARLY && (control == 10 || control == 64))
		return;

	switch (control) {
	case 7:
		_channel[channelNr].volume = (_version < SCI_VERSION_1_EARLY) ? MAX<uint8>((value & 0x78) << 1, 0x40) : (value ? MAX<uint8>(value >> 3, 1) : 0);
		break;

	case 10:
		_channel[channelNr].pan = value;
		break;

	case 64:
		_channel[channelNr].hold = value;

		if (!value) {
			for (int i = 0; i < _numVoicesPrimary; ++i) {
				if (_voice[i]->_assign == channelNr && _voice[i]->_sustained) {
					_voice[i]->_sustained = false;
					_voice[i]->noteOff();
				}
			}
		}
		break;

	case 75:
#ifndef CMS_DISABLE_VOICE_MAPPING
		voiceMapping(channelNr, value);
#endif
		break;

	case 123:
		for (uint i = 0; i < ARRAYSIZE(_voice); ++i) {
			if (_voice[i]->_assign == channelNr && _voice[i]->_note != 0xFF)
				_voice[i]->stop();
		}
		break;

	default:
		return;
	}
}

void MidiDriver_CMS::programChange(int channelNr, int value) {
	_channel[channelNr].program = value;
	if (_version > SCI_VERSION_0_LATE)
		return;

	for (int i = 0; i < _numVoicesPrimary; ++i) {
		if (_voice[i]->_assign == channelNr)
			_voice[i]->programChange(value);
	}
}

void MidiDriver_CMS::pitchWheel(int channelNr, int value) {
	_channel[channelNr].pitchWheel = value;
	for (int i = 0; i < _numVoicesPrimary; ++i) {
		if (_voice[i]->_assign == channelNr && _voice[i]->_note != 0xFF)
			_voice[i]->pitchWheel();
	}
}

void MidiDriver_CMS::voiceMapping(int channelNr, int value) {
	int curVoices = 0;

	for (int i = 0; i < _numVoicesPrimary; ++i) {
		if (_voice[i]->_assign == channelNr)
			++curVoices;
	}

	curVoices += _channel[channelNr].missingVoices;

	if (curVoices < value) {
		bindVoices(channelNr, value - curVoices, curVoices == 0 && value == 1, true);
	} else if (curVoices > value) {
		unbindVoices(channelNr, curVoices - value, value == 1);
		donateVoices(value == 1);
	}/*else if (_version < SCI_VERSION_1_EARLY && value == 1) {
		// The purpose of these lines would be to fill up missing secondary voices.
		// I have commented them out, since the original driver doesn't do that either.
		unbindVoices(channelNr, 1, true);
		bindVoices(channelNr, 1, true, true);
	}*/
}

void MidiDriver_CMS::bindVoices(int channelNr, int voices, bool bindSecondary, bool doProgramChange) {
	int secondary = bindSecondary ? _numVoicesSecondary : 0;

	for (int i = 0; i < _numVoicesPrimary; ++i) {
		if (_voice[i]->_assign != 0xFF)
			continue;

		_voice[i]->_assign = channelNr;
		if (_voice[i]->_note != 0xFF)
			_voice[i]->stop();

		for (int ii = _numVoicesPrimary; ii < _numVoicesPrimary + secondary; ++ii) {
			if (_voice[ii]->_assign != 0xFF)
				continue;

			_voice[ii]->_assign = channelNr;
			_voice[i]->_secondaryVoice = _voice[ii];

			break;
		}

		// This will also release the secondary voice binding immediately if the current patch does
		// not require such an extra channel. This condition will not be checked when called from initTrack().
		if (doProgramChange)
			_voice[i]->programChange(_channel[channelNr].program);

		--voices;
		if (voices == 0)
			break;
	}

	_channel[channelNr].missingVoices += voices;
}

void MidiDriver_CMS::unbindVoices(int channelNr, int voices, bool bindSecondary) {
	int secondary = bindSecondary ? _numVoicesSecondary : 0;
	Channel &channel = _channel[channelNr];

	if (channel.missingVoices >= voices) {
		channel.missingVoices -= voices;
	} else {
		voices -= channel.missingVoices;
		channel.missingVoices = 0;

		for (int i = 0; i < _numVoicesPrimary; ++i) {
			if (_voice[i]->_assign == channelNr && _voice[i]->_note == 0xFF) {
				_voice[i]->_assign = 0xFF;
				
				CMSVoice *sec = _voice[i]->_secondaryVoice;
				if (sec) {
					sec->stop();
					sec->_assign = 0xFF;
					_voice[i]->_secondaryVoice = 0;
				}

				--voices;
				if (voices == 0)
					return;
			}
		}

		do {
			uint16 voiceTime = 0;
			uint voiceNr = 0;

			for (int i = 0; i < _numVoicesPrimary; ++i) {
				if (_voice[i]->_assign != channelNr)
					continue;

				uint16 curTime = _voice[i]->_releaseDuration;
				if (curTime)
					curTime += 0x8000;
				else
					curTime = _voice[i]->_duration;

				if (curTime >= voiceTime) {
					voiceNr = i;
					voiceTime = curTime;
				}
			}

			_voice[voiceNr]->_sustained = false;
			_voice[voiceNr]->stop();
			_voice[voiceNr]->_assign = 0xFF;

			CMSVoice *sec = _voice[voiceNr]->_secondaryVoice;
			if (sec) {
				sec->stop();
				sec->_assign = 0xFF;
				_voice[voiceNr]->_secondaryVoice = 0;
			}

			--voices;
		} while (voices != 0);
	}

	for (int i = _numVoicesPrimary; i < _numVoicesPrimary + secondary; ++i) {
		if (_voice[i]->_assign != 0xFF)
			continue;

		_voice[i]->_assign = channelNr;
		if (_voice[i]->_note != 0xFF)
			_voice[i]->stop();

		for (int ii = 0; ii < _numVoicesPrimary; ++ii) {
			if (_voice[ii]->_assign != channelNr)
				continue;
			_voice[ii]->_secondaryVoice = _voice[i];
			// This will release the secondary binding immediately if the current patch does not require such an extra channel.
			_voice[ii]->programChange(_channel[channelNr].program);
			break;
		}

		if (_voice[i]->_assign == channelNr && _voice[i]->_note != 0xFF)
			_voice[i]->stop();
		break;
	}
}

void MidiDriver_CMS::donateVoices(bool bindSecondary) {
	int freeVoices = 0;

	for (int i = 0; i < _numVoicesPrimary; ++i) {
		if (_voice[i]->_assign == 0xFF)
			++freeVoices;
	}

	if (!freeVoices)
		return;

	for (int i = 0; i < ARRAYSIZE(_channel); ++i) {
		Channel &channel = _channel[i];

		if (!channel.missingVoices) {
			continue;
		} else if (channel.missingVoices < freeVoices) {
			freeVoices -= channel.missingVoices;
			int missing = channel.missingVoices;
			channel.missingVoices = 0;
			bindVoices(i, missing, bindSecondary, true);
		} else {
			channel.missingVoices -= freeVoices;
			bindVoices(i, freeVoices, bindSecondary, true);
			return;
		}
	}
}

int MidiDriver_CMS::findVoice(int channelNr, int note) {
	Channel &channel = _channel[channelNr];
	int voiceNr = channel.lastVoiceUsed;
	int newVoice = 0;
	int newVoiceAltSCI0 = (_version > SCI_VERSION_0_LATE) ? -2 : -1;
	uint16 newVoiceTime = 0;

	bool loopDone = false;
	do {
		++voiceNr;

		if (voiceNr == _numVoicesPrimary)
			voiceNr = 0;

		if (voiceNr == channel.lastVoiceUsed)
			loopDone = true;

		if (_voice[voiceNr]->_assign == channelNr) {
			if (_voice[voiceNr]->_note == 0xFF) {
				channel.lastVoiceUsed = (_version > SCI_VERSION_0_LATE) ? voiceNr : _numVoicesPrimary - 1;
				return voiceNr;
			}

			int cnt = 1;
			for (int i = voiceNr + 1; i < _numVoicesPrimary; ++i) {
				if (_voice[i]->_assign == channelNr)
					++cnt;
			}

			// The SCI0 driver will (before resorting to the "note age test") simply return the first
			// assigned voice as long as there are no other (primary) voices assigned to the midi part.
			if (cnt == 1 && newVoiceAltSCI0 == -1)
				newVoiceAltSCI0 = voiceNr;

			uint16 curTime = _voice[voiceNr]->_releaseDuration;
			if (curTime)
				curTime += 0x8000;
			else
				curTime = _voice[voiceNr]->_duration;

			if (curTime >= newVoiceTime) {
				newVoice = voiceNr;
				newVoiceTime = curTime;
			}
		}
	} while (!loopDone);

	if (newVoiceAltSCI0 >= 0)
		return newVoiceAltSCI0;

	if (newVoiceTime > 0) {
		voiceNr = newVoice;
		channel.lastVoiceUsed = _numVoicesPrimary - 1;

		if (_version > SCI_VERSION_0_LATE) {
			_voice[voiceNr]->stop();
			channel.lastVoiceUsed = voiceNr;
		}

		return voiceNr;
	}
	
	return -1;
}

int MidiDriver_CMS::findVoiceBasic(int channelNr) {
	int voice = -1;
	int oldestVoice = -1;
	int oldestAge = -1;

	// Try to find a voice assigned to this channel that is free (round-robin)
	for (int i = 0; i < _numVoicesPrimary; i++) {
		int v = (_channel[channelNr].lastVoiceUsed + i + 1) % _numVoicesPrimary;

		if (_voice[v]->_note == 0xFF) {
			voice = v;
			break;
		}

		// We also keep track of the oldest note in case the search fails
		if (_voice[v]->_duration > oldestAge) {
			oldestAge = _voice[v]->_duration;
			oldestVoice = v;
		}
	}

	if (voice == -1) {
		if (oldestVoice >= 0) {
			_voice[oldestVoice]->stop();
			voice = oldestVoice;
		} else {
			return -1;
		}
	}

	_voice[voice]->_assign = channelNr;
	_channel[channelNr].lastVoiceUsed = (_version > SCI_VERSION_0_LATE) ? voice : 0;
	return voice;
}

void MidiDriver_CMS::writeToChip(int chip, int address, int data) {
	assert(chip == 0 || chip == 1);
	_cms->portWrite(0x221 + (chip << 1), address);
	_cms->portWrite(0x220 + (chip << 1), data);
}

void MidiDriver_CMS::generateSamples(int16 *buffer, int len) {
	_cms->readBuffer(buffer, len);
}

class MidiPlayer_CMS : public MidiPlayer {
public:
	MidiPlayer_CMS(SciVersion version) : MidiPlayer(version), _filesMissing(false) {}

	int open(ResourceManager *resMan) override;
	void close() override;

	void initTrack(SciSpan<const byte>& header) override;

	bool hasRhythmChannel() const override { return false; }
	byte getPlayId() const override { return _version > SCI_VERSION_0_LATE ? 9 : 4; }
	int getPolyphony() const override { return 12; }

	void playSwitch(bool play) override { _driver->property(MidiDriver_CMS::MIDI_PROP_PLAYSWITCH, play ? 1 : 0); }

	const char *reportMissingFiles() override { return _filesMissing ? _requiredFiles : 0; }

private:
	bool _filesMissing;
	static const char _requiredFiles[];
};

int MidiPlayer_CMS::open(ResourceManager *resMan) {
	if (_driver)
		return MidiDriver::MERR_ALREADY_OPEN;

	_driver = new MidiDriver_CMS(g_system->getMixer(), resMan, _version);
	int driverRetVal = _driver->open();

	if (driverRetVal == -1)
		_filesMissing = true;

	return driverRetVal;
}

void MidiPlayer_CMS::close() {
	_driver->setTimerCallback(0, 0);
	_driver->close();
	delete _driver;
	_driver = nullptr;
}

void MidiPlayer_CMS::initTrack(SciSpan<const byte>& header) {
	if (_driver)
		static_cast<MidiDriver_CMS*>(_driver)->initTrack(header);
}

const char MidiPlayer_CMS::_requiredFiles[] = "'PATCH.101'";

MidiPlayer *MidiPlayer_CMS_create(SciVersion version) {
	return new MidiPlayer_CMS(version);
}

} // End of namespace SCI
