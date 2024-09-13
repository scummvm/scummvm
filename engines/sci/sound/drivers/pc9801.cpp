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

#include "sci/sci.h"

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/softsynth/fmtowns_pc98/pc98_audio.h"

#include "sci/resource/resource.h"
#include "sci/sound/drivers/mididriver.h"

/*	PC-98 sound driver for:
	Quest for Glory 1		SCI_VERSION_0_LATE		first driver type
	Police Quest 2			SCI_VERSION_0_LATE		second driver type
	Castle of Dr. Brain		SCI_VERSION_1_LATE		third driver type
	Space Quest IV			SCI_VERSION_1_LATE		third driver type
	King's Quest V			SCI_VERSION_1_LATE		latest driver type
*/

namespace Sci {

class MidiPart_PC9801;
class SoundChannel_PC9801 {
public:
	SoundChannel_PC9801(PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, int type, SciSpan<const uint8> instrumentData, bool &soundOn);
	virtual ~SoundChannel_PC9801();

	void noteOff();
	void noteOn(uint8 note, uint8 velo);

	virtual void processNoteEvent(uint8 note, bool soundOn);
	void update();
	void setVolume(uint8 volume);

	virtual void reset();
	virtual void toggleNoiseGenerator(bool) {}

	int getType() const { return _type; }

	uint8 _assign;
	uint8 _note;
	uint8 _sustain;
	uint16 _duration;

protected:
	enum ChannelStatusFlags {
		kChanVbrEnable = 0x01,
		kChanVbrRestartEnv = 0x02,
		kChanKeyOn = 0x04,
		kChanNgEnable = 0x08,
		kChanNgRestartEnv = 0x10,
		kChanNgDecrease = 0x20,
		kChanVbrMode = 0x40,
		kChanVbrDecrease = 0x80
	};

	int recalculateFrequency(uint16 note, uint16 modifier, uint8 *destOctaveBlock, uint16 *destFrequency, uint8 *destVbrFrequencyModifier);
	uint8 getVolume();
	virtual void processSounds();
	void programChangeInit(SciSpan<const uint8> &data);
	void writeReg(uint8 part, uint8 reg, uint8 val);

	int8 _transpose;
	uint8 _vbrInitialDelay;
	uint8 _vbrEnvelopeTimer;
	uint8 _vbrEnvelopeSpeed;

	uint16 _frequencyCourse;
	uint8 _frequencyBlock;
	uint8 _frequencyNoteModifier;

	uint8 _vbrDepthIncr;
	uint8 _vbrDecrTime;
	uint8 _vbrDepthDecr;
	uint8 _vbrIncrTime;
	uint8 _vbrSensitivity;
	uint16 _vbrIncrStep;
	uint16 _vbrDecrStep;
	uint8 _vbrModulationTimer;
	uint16 _vbrFrequencyModifier;
	uint8 _vbrCur;
	uint8 _flags;

	const uint16 *_noteFrequency;
	const uint16 *_noteFrequencyModifier;

	SciSpan<const uint8> _instrumentData;
	const SciVersion _version;

private:
	virtual void programChange(uint8 program) = 0;
	virtual bool prepareFrequencyAndVolume(bool updateVolume);
	virtual void sendSoundOnOff(bool soundOn) = 0;
	virtual void sendFrequency() = 0;
	virtual void sendVolume() = 0;

	uint8 _velo;
	uint8 _volume;
	uint8 _program;
	bool &_soundOn;

	MidiPart_PC9801 **_parts;
	PC98AudioCore *_pc98a;
	const int _type;
};

class SoundChannel_PC9801_FM4OP : public SoundChannel_PC9801 {
public:
	SoundChannel_PC9801_FM4OP(uint8 id, PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, SciSpan<const uint8> instrumentData, uint8 patchSize, bool &soundOn);
	~SoundChannel_PC9801_FM4OP() override {}

private:
	void programChange(uint8 program) override;
	void sendSoundOnOff(bool soundOn) override;
	void sendVolume() override;
	void sendFrequency() override;

	uint8 _operatorLevel[4];
	uint8 _carrier;
	uint8 _operatorFlags;

	const uint8 _regPrt;
	const uint8 _regOffs;
	const uint8 _patchSize;
};

class SoundChannel_PC9801_FM2OP : public SoundChannel_PC9801 {
public:
	SoundChannel_PC9801_FM2OP(uint8 id, PC98AudioCore *pc98a, MidiPart_PC9801 **parts,  SciVersion version, SciSpan<const uint8> instrumentData, uint8 patchSize, bool &soundOn);
	~SoundChannel_PC9801_FM2OP() override {}

	void processNoteEvent(uint8 note, bool soundOn) override;
	void reset() override;

private:
	void programChange(uint8 program) override;
	bool prepareFrequencyAndVolume(bool updateVolume) override;
	void processSounds() override;
	void sendSoundOnOff(bool soundOn) override;
	void sendVolume() override;
	void sendFrequency() override;

	uint8 _operatorLevel[2];
	uint8 _operatorFrqIndex[2];
	uint16 _frequencyCourse2;
	uint8 _frequencyNoteModifier2;
	int16 _vbrFrequencyModifier2;
	uint16 _vbrIncrStep2;
	uint16 _vbrDecrStep2;
	uint8 _vbrCur2;
	static uint8 _activeOperators;

	const uint16 *_opFreqOffset;
	const uint8 _patchOffset;
	const uint8 _regPrt;
	const uint8 _regOffs;
	const uint8 _patchSize;
};

class SoundChannel_PC9801_SSG : public SoundChannel_PC9801 {
public:
	SoundChannel_PC9801_SSG(uint8 id, PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, SciSpan<const uint8> instrumentData, uint8 patchOffset, uint8 patchSize, bool &soundOn);
	~SoundChannel_PC9801_SSG() override {}
	void reset() override;
	void toggleNoiseGenerator(bool enable) override;

private:
	enum {
		kEnvSSG_silent = 0x00,
		kEnvSSG_decay = 0x01,
		kEnvSSG_sustain = 0x02,
		kEnvSSG_attack = 0x03,
		kEnvSSG_keyOn = 0x10
	};

	void programChange(uint8 program) override;
	void processSounds() override;
	void sendSoundOnOff(bool soundOn) override;
	void sendVolume() override;
	void sendFrequency() override;
	void updateNg();
	void sendActiveChannelsStatus();

	uint8 _instrumentChanMask;
	uint8 _chanDisableMask;
	uint8 _chanEnableMask1;
	uint8 _chanEnableMask2;
	uint8 _currentLevel;
	uint8 _ngPhaseStep;
	uint8 _ngPhase;
	uint8 _ngEnvelopeTimer;
	uint8 _ngSpeed;
	uint8 _ssgEnvelopeTimer;
	uint8 _ssgLevel;
	uint8 _ssgSpeed;
	uint8 _ssgEnvelopeState;
	bool _ccngEnabled;

	static uint8 _activeChannnelsStatus;
	SciSpan<const uint8> _selectedInstrument;

	const uint8 *_ngFreq;
	const uint8 _regOffs;
	const uint8 _patchOffset;
	const uint8 _patchSize;
};

class MidiPart_PC9801 {
public:
	MidiPart_PC9801(uint8 id, SoundChannel_PC9801 **channels, uint8 numChan, SciVersion version);
	~MidiPart_PC9801() {}

	void noteOff(uint8 note);
	void noteOn(uint8 note, uint8 velo);
	void controlChangeVolume(uint8 vol);
	void controlChangeSustain(uint8 sus);
	void controlChangePolyphony(uint8 numChan);
	void controlChangeNoiseGenerator(uint8 status);
	void controlChangeAllNotesOff();
	void programChange(uint8 prg);
	void pitchBend(int16 val);

	void addChannels(int num, int resetMissingChannels = -1, int channelType = -1);
	void dropChannels(int num);

	uint8 getCurrentProgram() const { return _program; }
	uint8 getCurrentVolume() const { return _volume; }
	uint16 getCurrentPitchBend() const { return _pitchBend; }
	uint8 getMissingChannels() const { return _chanMissing; }

private:
	int allocateChannel();
	void assignFreeChannels();

	uint8 _id;
	uint8 _program;
	uint8 _volume;
	uint8 _sustain;
	uint16 _pitchBend;
	uint8 _outChan;
	uint8 _chanMissing;

	SoundChannel_PC9801 **_chan;
	const SciVersion _version;
	const int _numChan;
	const uint8 _noteRangeLow;
	const uint8 _noteRangeHigh;
};

class MidiDriver_PC9801 : public MidiDriver, public PC98AudioPluginDriver {
public:
	enum {
		MIDI_PROP_PLAYSWITCH = 1,
		MIDI_PROP_POLYPHONY = 2,
		MIDI_PROP_CHANNEL_ID = 3
	};

	MidiDriver_PC9801(Audio::Mixer *mixer, SciVersion version);
	~MidiDriver_PC9801() override;

	int open() override;
	bool isOpen() const override { return _isOpen; }
	void close() override;

	void send(uint32 b) override;
	static void assignFreeChannels(int num);
	uint32 property(int prop, uint32 param) override;
	void initTrack(SciSpan<const byte> &header);

	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;
	uint32 getBaseTempo() override;
	MidiChannel *allocateChannel() override { return nullptr; }
	MidiChannel *getPercussionChannel() override { return nullptr; }

	void timerCallbackB() override;

private:
	bool loadInstruments(const SciSpan<const uint8> &data);
	void updateParser();
	void updateChannels();
	void reset();

	Common::TimerManager::TimerProc _timerProc;
	void *_timerProcPara;

	static MidiPart_PC9801 **_parts;
	SoundChannel_PC9801 **_chan;
	Common::SpanOwner<SciSpan<uint8> > _instrumentData;

	uint8 _masterVolume;
	bool _soundOn;

	uint8 _numChan;
	uint8 _ssgPatchOffset;
	uint8 _patchSize;

	uint8 _internalVersion;
	const uint8 _playID;
	const uint8 _channelMask1;
	uint8 _channelMask2;
	uint8 _polyphony;

	bool _isOpen;
	bool _ready;

	const uint16 _baseTempo;

	PC98AudioCore *_pc98a;
	Audio::Mixer *_mixer;
	const SciVersion _version;
};

class MidiPlayer_PC9801 : public MidiPlayer {
public:
	enum {
		MIDI_PROP_PLAYSWITCH = 1,
		MIDI_PROP_POLYPHONY = 2,
		MIDI_PROP_CHANNEL_ID = 3
	};

	MidiPlayer_PC9801(SciVersion version);
	~MidiPlayer_PC9801() override;

	bool hasRhythmChannel() const override;
	byte getPlayId() const override;
	int getPolyphony() const override;
	void playSwitch(bool play) override;

	void initTrack(SciSpan<const byte> &trackData) override;
};

SoundChannel_PC9801::SoundChannel_PC9801(PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, int type, SciSpan<const uint8> instrumentData, bool &soundOn)
	: _pc98a(pc98a), _parts(parts), _version(version), _type(type), _instrumentData(instrumentData), _soundOn(soundOn), _assign(0xff), _note(0xff), _velo(0),
	_volume(0), _transpose(0), _sustain(0), _duration(0), _program(0xff), _vbrInitialDelay(0), _vbrEnvelopeTimer(0),
	_vbrEnvelopeSpeed(0), _vbrDepthIncr(0), _vbrDecrTime(0), _vbrDepthDecr(0), _vbrIncrTime(0), _vbrSensitivity(0),
	_vbrFrequencyModifier(0), _vbrIncrStep(0), _vbrDecrStep(0), _vbrModulationTimer(0), _flags(0), _vbrCur(0x80),
	_frequencyBlock(0), _frequencyCourse(0), _frequencyNoteModifier(0) {

	static const uint16 courseV0[] = { 0x269, 0x28D, 0x2B5, 0x2DE, 0x30A, 0x339, 0x368, 0x39D, 0x3D4, 0x40E, 0x44A, 0x48C };
	static const uint16 courseV1[] = { 0x26A, 0x28E, 0x2B5, 0x2DF, 0x30A, 0x339, 0x36A, 0x39E, 0x3D5, 0x40F, 0x44D, 0x48F };
	static const uint16 fine[] = { 0x24, 0x27, 0x2A, 0x2B, 0x2F, 0x31, 0x34, 0x37, 0x3A, 0x3E, 0x42, 0x45 };

	_noteFrequency = (version > SCI_VERSION_0_LATE) ? courseV1 : courseV0;
	_noteFrequencyModifier = fine;
}

SoundChannel_PC9801::~SoundChannel_PC9801() {
	_instrumentData.clear();
}

void SoundChannel_PC9801::noteOff() {
	if (_sustain)
		return;

	processNoteEvent(_note, false);

	_note = 0xFF;
	_duration = 0;
}

void SoundChannel_PC9801::noteOn(uint8 note, uint8 velo) {
	_duration = 0;

	if (_program != _parts[_assign]->getCurrentProgram() && _soundOn) {
		_program = _parts[_assign]->getCurrentProgram();
		programChange(_program);
	}

	if (_version > SCI_VERSION_0_LATE) {
		velo >>= 1;
		if (velo > 63)
			velo = 63;
		_velo = velo;
	}

	processNoteEvent(note, true);
}

void SoundChannel_PC9801::processNoteEvent(uint8 note, bool soundOn) {
	if (_note != note) {
		_note = note;
		_vbrEnvelopeTimer = _vbrInitialDelay;
		_vbrFrequencyModifier = 0;
		_vbrCur = 0x80;
		_flags |= kChanVbrRestartEnv;
	}

	if (!prepareFrequencyAndVolume(soundOn))
		soundOn = false;

	sendSoundOnOff(soundOn);
}

void SoundChannel_PC9801::update() {
	processSounds();
	if (_note != 0xFF)
		_duration++;
}

void SoundChannel_PC9801::setVolume(uint8 volume) {
	static const uint8 volumeTable[] = { 0x7F, 0x7F, 0x30, 0x30, 0x28, 0x28, 0x20, 0x20, 0x18, 0x18, 0x10, 0x10, 0x08, 0x08, 0x00, 0x00 };
	assert(volume < 16);
	_volume = volumeTable[volume];
}

void SoundChannel_PC9801::reset() {
	_assign = 0xFF;
	_note = 0xFF;
	_volume = 0;
}

int SoundChannel_PC9801::recalculateFrequency(uint16 note, uint16 modifier, uint8 *destOctaveBlock, uint16 *destFrequency, uint8 *destVbrFrequencyModifier) {
	note += _transpose;
	uint16 pb = _parts[_assign]->getCurrentPitchBend();

	if (pb == 0x2000) {
		pb = 0;
	} else {
		int dir = 1;
		if (pb > 0x2000) {
			pb -= 0x2000;
		} else if (pb < 0x2000) {
			pb = 0x2000 - pb;
			dir = -1;
		}

		uint8 noteDiff = pb / 684;
		note += (noteDiff * dir);

		pb %= 684;
		if (pb > 682)
			pb = 682;
		pb = ((pb * 6) >> 4);
		pb *= dir;
	}

	note += (modifier >> 8);
	pb += (modifier & 0xff);
	if ((pb >> 8) == 1) {
		pb &= 0xFF;
		note++;
	}

	if (_version == SCI_VERSION_0_LATE)
		note -= 12;

	if (note < 12 || note > 107)
		return -1;

	if (_version == SCI_VERSION_0_LATE && _type == 2) {
		uint16 rs = _noteFrequency[note - 12];
		if (destFrequency)
			*destFrequency = rs;
		return rs;
	}

	uint8 block = (note / 12) - 1;
	note %= 12;
	uint16 res =_noteFrequency[note];
	uint16 pitchVbrMultiplier = _noteFrequencyModifier[note];

	if (_type != 2)
		res |= (block << 11);
	else if (block == 0)
		return -1;

	if (pb)
		res += (((pb * pitchVbrMultiplier) & 0x0FF0) >> 8);

	if (res < 0x4000) {
		if (destFrequency)
			*destFrequency = res;
		if (destOctaveBlock)
			*destOctaveBlock = block;
		if (destVbrFrequencyModifier)
			*destVbrFrequencyModifier = pitchVbrMultiplier;
		return res;
	}

	return -1;
}

uint8 SoundChannel_PC9801::getVolume() {
	static const uint8 volumeTable1[] = {
		0x00, 0x0c, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x13, 0x14, 0x16, 0x17, 0x18, 0x1a, 0x1b, 0x1c, 0x1d,
		0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2d, 0x2d, 0x2e,
		0x2f, 0x30, 0x31, 0x32, 0x32, 0x33, 0x34, 0x34, 0x35, 0x36,	0x36, 0x37, 0x38, 0x38, 0x39, 0x3a,
		0x3b, 0x3b,	0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d, 0x3d, 0x3e, 0x3e, 0x3e,	0x3e, 0x3f, 0x3f, 0x3f
	};

	static const uint8 volumeTable2[] = {
		0x00, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61, 0x61,
		0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x6f, 0x70,
		0x71, 0x72, 0x72, 0x73, 0x74, 0x74, 0x75, 0x76, 0x76, 0x77, 0x78, 0x78, 0x79, 0x79, 0x7a, 0x7a,
		0x7b, 0x7b, 0x7b, 0x7c, 0x7c, 0x7c, 0x7d, 0x7d, 0x7d, 0x7e, 0x7e, 0x7e, 0x7e, 0x7f, 0x7f, 0x7f
	};

	uint16 partVolume = ((_assign != 0xFF) ? _parts[_assign]->getCurrentVolume() : 0);
	if (_version == SCI_VERSION_0_LATE)
		return partVolume;

	partVolume += 1;
	uint16 velocity = volumeTable1[_velo] + 1;
	uint16 volume = _soundOn ? (partVolume * velocity) >> 6 : 0;
	volume = volumeTable2[volume] - _volume;

	return (volume < 0x7F ? volume : 0);
}

void SoundChannel_PC9801::processSounds() {
	if (!(_flags & kChanVbrEnable))
		return;

	if (_flags & kChanVbrRestartEnv) {
		if (--_vbrEnvelopeTimer)
			return;

		_vbrIncrStep = _vbrDepthIncr * _vbrSensitivity * _frequencyNoteModifier;
		_vbrDecrStep = _vbrDepthDecr * _vbrSensitivity * _frequencyNoteModifier;
		_vbrModulationTimer = ((_flags & kChanVbrMode) ? _vbrIncrTime : _vbrDecrTime) >> 1;
		_vbrCur = 0x80;

		_flags &= ~(kChanVbrDecrease | kChanVbrRestartEnv);
		if (_flags & kChanVbrMode)
			_flags |= kChanVbrDecrease;
	}

	uint16 t = _vbrEnvelopeTimer + _vbrEnvelopeSpeed;
	_vbrEnvelopeTimer = t & 0xFF;
	if (t & 0x100)
		return;

	if (!--_vbrModulationTimer) {
		_vbrModulationTimer = (_flags & kChanVbrDecrease) ? _vbrDecrTime : _vbrIncrTime;
		_flags ^= kChanVbrDecrease;
	}

	if (_flags & kChanVbrDecrease) {
		uint8 sL = _vbrDecrStep & 0xFF;
		uint8 sH = _vbrDecrStep >> 8;
		bool ovrflow = (sL > _vbrCur);
		_vbrCur -= sL;
		if (!ovrflow)
			return;
		_vbrFrequencyModifier -= (sH + 1);
	} else {
		uint8 sL = _vbrDecrStep & 0xFF;
		uint8 sH = _vbrDecrStep >> 8;
		bool ovrflow = (sL + _vbrCur > 255);
		_vbrCur += sL;
		if (!ovrflow)
			return;
		_vbrFrequencyModifier += (sH + 1);
	}

	sendFrequency();
}

void SoundChannel_PC9801::programChangeInit(SciSpan<const uint8> &data) {
	_transpose = (int8)(data[0] & 0xC0 && data[0] < 0xC0 ? data[0] ^ 0x80 : data[0]);
	_vbrInitialDelay = data[1];
	_vbrDepthIncr = data[2];
	_vbrDecrTime = data[3];
	_vbrDepthDecr = data[4];
	_vbrIncrTime = data[5];
	_vbrSensitivity = (data[6] & 3) + 1;
	_vbrEnvelopeSpeed = data[6] >> 3;
	_flags = (_flags & ~(kChanVbrEnable | kChanVbrMode)) | ((data[6] << 4) & 0x40);
	if (_vbrInitialDelay)
		_flags |= (kChanVbrEnable | kChanVbrRestartEnv);
}

void SoundChannel_PC9801::writeReg(uint8 part, uint8 reg, uint8 val) {
	_pc98a->writeReg(part, reg, val);
}

bool SoundChannel_PC9801::prepareFrequencyAndVolume(bool updateVolume) {
	if (recalculateFrequency(_note, 0, &_frequencyBlock, &_frequencyCourse, &_frequencyNoteModifier) == -1)
		return false;

	sendFrequency();
	if (updateVolume)
		sendVolume();

	return true;
}

SoundChannel_PC9801_FM4OP::SoundChannel_PC9801_FM4OP(uint8 id, PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, SciSpan<const uint8> instrumentData, uint8 patchSize, bool &soundOn)
	: SoundChannel_PC9801(pc98a, parts, version, 0, instrumentData, soundOn), _carrier(0), _regPrt(id > 2 ? 1 : 0), _regOffs(id % 3),
	_operatorFlags((id > 2 ? 4 : 0) | (id % 3) | 0xF0), _patchSize(patchSize) {
	_operatorLevel[0] = _operatorLevel[1] = _operatorLevel[2] = _operatorLevel[3] = 0x7F;
}

void SoundChannel_PC9801_FM4OP::programChange(uint8 program) {
	static const uint8 carrier[] = { 0x10, 0x10, 0x10, 0x10, 0x30, 0x70, 0x70, 0xF0 };
	static const uint8 steps[] = { 0, 16, 8, 24 };
	SciSpan<const uint8> data = _instrumentData.subspan(program * _patchSize);

	if (_version == SCI_VERSION_1_LATE) {
		programChangeInit(data);
		uint8 pos = 8;

		for (uint8 i = 0x40 + _regOffs; i < 0x50 + _regOffs; i += 4)
			writeReg(_regPrt, i, 0xFF);
		for (uint8 i = 0x30 + _regOffs; i < 0x40 + _regOffs; i += 4)
			writeReg(_regPrt, i, data[pos++]);
		for (int i = 0; i < 4; ++i)
			_operatorLevel[i] = data[pos++];
		for (uint8 i = 0x50 + _regOffs; i < 0xA0 + _regOffs; i += 4)
			writeReg(_regPrt, i, data[pos++]);
		writeReg(_regPrt, 0xB0 + _regOffs, data[pos]);
		_carrier = carrier[data[pos] & 7];

	} else {
		uint8 pos = 11;
		uint8 opFlags = data[pos];
		uint8 fba = data[pos + 1] & 0x3F;
		_carrier = carrier[fba & 7];

		pos += 8;
		uint8 reg = 0x30 + _regOffs;
		for (int i = 0; i < 4; ++i)
			writeReg(_regPrt, reg + (i << 2), data[pos + steps[i & 3]] & 0x7F);

		pos -= 3;
		_operatorLevel[0] = (opFlags & 0x08) ? data[pos + steps[0]] : 0x7F;
		_operatorLevel[1] = (opFlags & 0x20) ? data[pos + steps[1]] : 0x7F;
		_operatorLevel[2] = (opFlags & 0x10) ? data[pos + steps[2]] : 0x7F;
		_operatorLevel[3] = (opFlags & 0x40) ? data[pos + steps[3]] : 0x7F;

		pos += 4;
		reg = 0x50 + _regOffs;
		for (int i = 0; i < 16; ++i)
			writeReg(_regPrt, reg + (i << 2), data[pos + steps[i & 3] + (i >> 2)]);

		if (fba >= 24)
			fba -= 24;
		else
			fba &= 7;

		writeReg(_regPrt, 0xB0 + _regOffs, fba);
		_operatorFlags = (_operatorFlags & 7) | (opFlags << 1);
	}
}

void SoundChannel_PC9801_FM4OP::sendSoundOnOff(bool soundOn) {
	_flags = soundOn ? (_flags | kChanKeyOn) : (_flags & ~kChanKeyOn);
	if (soundOn && _version == SCI_VERSION_0_LATE)
		writeReg(0, 0x28, _operatorFlags & 7);
	writeReg(0, 0x28, soundOn ? _operatorFlags : _operatorFlags & 7);
}

void SoundChannel_PC9801_FM4OP::sendVolume() {
	uint8 vol = getVolume();
	uint16 c = _carrier;

	for (uint8 i = 0; i < 4; ++i) {
		uint8 r = _operatorLevel[i];
		c += c;
		if (c & 0x100) {
			c &= 0xFF;
			if (_version == SCI_VERSION_1_LATE) {
				r = (((r ^ 0x7F) * vol) / 0x7F) * 2;
				r = ((r < 0x7F) ? 0x7F - r : 0) + 20;
				if (r > 0x7F)
					r = 0x7F;
			} else {
				r = 127 - ((127 - r) * vol / 128);
			}
		}
		writeReg(_regPrt, 0x40 + (i << 2) + _regOffs, r);
	}
}

void SoundChannel_PC9801_FM4OP::sendFrequency() {
	uint16 freq = _frequencyCourse + _vbrFrequencyModifier;
	writeReg(_regPrt, 0xA4 + _regOffs, freq >> 8);
	writeReg(_regPrt, 0xA0 + _regOffs, freq & 0xFF);
}

uint8 SoundChannel_PC9801_FM2OP::_activeOperators = 0;

SoundChannel_PC9801_FM2OP::SoundChannel_PC9801_FM2OP(uint8 id, PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, SciSpan<const uint8> instrumentData, uint8 patchSize, bool &soundOn)
	: SoundChannel_PC9801(pc98a, parts, version, 1, instrumentData, soundOn), _patchOffset(37), _patchSize(patchSize), _frequencyCourse2(0), _frequencyNoteModifier2(0),
	_vbrFrequencyModifier2(0), _vbrCur2(0x80), _vbrIncrStep2(0), _vbrDecrStep2(0), _regPrt(id > 3 ? 1 : 0), _regOffs(id & 1) {

	static const uint16 opFreqOffset[] = { 0x0000, 0x0600, 0x07CF, 0x0980 };

	_operatorLevel[0] = _operatorLevel[1] = 0x7F;
	_operatorFrqIndex[0] = _operatorFrqIndex[1] = 0;
	_opFreqOffset = opFreqOffset;
}

void SoundChannel_PC9801_FM2OP::processNoteEvent(uint8 note, bool soundOn) {
	if (_note != note) {
		_vbrCur2 = 0x80;
		_vbrFrequencyModifier2 = 0;
	}
	SoundChannel_PC9801::processNoteEvent(note, soundOn);
}

void SoundChannel_PC9801_FM2OP::reset() {
	SoundChannel_PC9801::reset();
	_activeOperators = 0;
}

void SoundChannel_PC9801_FM2OP::programChange(uint8 program) {
	SciSpan<const uint8> data = _instrumentData.subspan(program * _patchSize + _patchOffset);

	programChangeInit(data);
	uint8 pos = 7;

	for (uint8 i = 0x42 + (_regOffs << 2); i < 0x52 + (_regOffs << 2); i += 8)
		writeReg(_regPrt, i, 0xFF);
	for (uint8 i = 0x32 + (_regOffs << 2); i < 0x42 + (_regOffs << 2); i += 8)
		writeReg(_regPrt, i, data[pos++]);
	for (int i = 0; i < 2; ++i)
		_operatorLevel[i] = data[pos++];
	for (uint8 i = 0x52 + (_regOffs << 2); i < 0x72 + (_regOffs << 2); i += 8)
		writeReg(_regPrt, i, data[pos++]);
	_operatorFrqIndex[0] = data[pos] >> 6;
	_operatorFrqIndex[1] = data[pos + 1] >> 6;
	for (uint8 i = 0x72 + (_regOffs << 2); i < 0xA2 + (_regOffs << 2); i += 8)
		writeReg(_regPrt, i, data[pos++]);
}

bool SoundChannel_PC9801_FM2OP::prepareFrequencyAndVolume(bool updateVolume) {
	if (recalculateFrequency(_note, _opFreqOffset[_operatorFrqIndex[0]], nullptr, &_frequencyCourse, &_frequencyNoteModifier) == -1)
		return false;
	if (recalculateFrequency(_note, _opFreqOffset[_operatorFrqIndex[1]], nullptr, &_frequencyCourse2, &_frequencyNoteModifier2) == -1)
		return false;

	sendFrequency();
	if (updateVolume)
		sendVolume();

	return true;
}

void SoundChannel_PC9801_FM2OP::sendSoundOnOff(bool soundOn) {
	uint8 op = 0x30 << (_regOffs << 1);

	if (soundOn) {
		_flags |= kChanKeyOn;
		_activeOperators |= op;
	} else {
		_flags &= ~kChanKeyOn;
		_activeOperators &= ~op;
	}

	writeReg(0, 0x28, _activeOperators | (_regPrt << 2) | 2);
}

void SoundChannel_PC9801_FM2OP::sendVolume() {
	writeReg(_regPrt, 0x42 + (_regOffs << 2), _operatorLevel[0]);
	uint8 r = (((_operatorLevel[1] ^ 0x7F) * getVolume()) / 0x7F) * 2;
	r = ((r < 0x7F) ? 0x7F - r : 0) + 16;
	writeReg(_regPrt, 0x4A + (_regOffs << 2), r);
}

void SoundChannel_PC9801_FM2OP::processSounds() {
	if (!(_flags & kChanVbrEnable))
		return;

	if (_flags & kChanVbrRestartEnv) {
		if (--_vbrEnvelopeTimer)
			return;

		_vbrIncrStep = _vbrDepthIncr * _vbrSensitivity * _frequencyNoteModifier;
		_vbrIncrStep2 = _vbrDepthIncr * _vbrSensitivity * _frequencyNoteModifier2;
		_vbrDecrStep = _vbrDepthDecr * _vbrSensitivity * _frequencyNoteModifier;
		_vbrDecrStep2 = _vbrDepthDecr * _vbrSensitivity * _frequencyNoteModifier2;
		_vbrModulationTimer = ((_flags & kChanVbrMode) ? _vbrIncrTime : _vbrDecrTime) >> 1;
		_vbrCur = 0x80;

		_flags &= ~(kChanVbrDecrease | kChanVbrRestartEnv);
		if (_flags & kChanVbrMode)
			_flags |= kChanVbrDecrease;
	}

	uint16 t = _vbrEnvelopeTimer + _vbrEnvelopeSpeed;
	_vbrEnvelopeTimer = t & 0xFF;
	if (t & 0x100)
		return;

	if (!--_vbrModulationTimer) {
		_vbrModulationTimer = (_flags & kChanVbrDecrease) ? _vbrDecrTime : _vbrIncrTime;
		_flags ^= kChanVbrDecrease;
	}

	if (_flags & kChanVbrDecrease) {
		uint8 sL = _vbrDecrStep & 0xFF;
		uint8 sH = _vbrDecrStep >> 8;
		bool ovrflow = (sL > _vbrCur);
		_vbrCur -= sL;
		if (ovrflow)
			_vbrFrequencyModifier -= (sH + 1);
		sL = _vbrDecrStep2 & 0xFF;
		sH = _vbrDecrStep2 >> 8;
		ovrflow = (sL > _vbrCur2);
		_vbrCur2 -= sL;
		if (ovrflow)
			_vbrFrequencyModifier2 -= (sH + 1);
	} else {
		uint8 sL = _vbrDecrStep & 0xFF;
		uint8 sH = _vbrDecrStep >> 8;
		bool ovrflow = (sL + _vbrCur > 255);
		_vbrCur += sL;
		if (ovrflow)
			_vbrFrequencyModifier += (sH + 1);
		sL = _vbrDecrStep2 & 0xFF;
		sH = _vbrDecrStep2 >> 8;
		ovrflow = (sL + _vbrCur2 > 255);
		_vbrCur2 += sL;
		if (ovrflow)
			_vbrFrequencyModifier2 += (sH + 1);
	}

	sendFrequency();
}

void SoundChannel_PC9801_FM2OP::sendFrequency() {
	uint16 freq = _frequencyCourse + _vbrFrequencyModifier;
	writeReg(_regPrt, 0xAD - _regOffs, freq >> 8);
	writeReg(_regPrt, 0xA9 - _regOffs, freq & 0xFF);
	freq = _frequencyCourse2 + _vbrFrequencyModifier2;
	writeReg(_regPrt, 0xAE - (_regOffs << 3), freq >> 8);
	writeReg(_regPrt, 0xAA - (_regOffs << 3), freq & 0xFF);
}

uint8 SoundChannel_PC9801_SSG::_activeChannnelsStatus = 0x3F;

SoundChannel_PC9801_SSG::SoundChannel_PC9801_SSG(uint8 id, PC98AudioCore *pc98a, MidiPart_PC9801 **parts, SciVersion version, SciSpan<const uint8> instrumentData, uint8 patchOffset, uint8 patchSize, bool &soundOn)
	: SoundChannel_PC9801(pc98a, parts, version, 2, instrumentData, soundOn), _patchOffset(patchOffset), _patchSize(patchSize), _regOffs(id & 3),
	_instrumentChanMask(0x3F), _ngPhaseStep(0),	_currentLevel(0), _ssgEnvelopeState(kEnvSSG_silent), _ngEnvelopeTimer(0),
	_ngSpeed(0), _ngPhase(0), _ssgEnvelopeTimer(0), _ssgLevel(0), _ssgSpeed(0), _ccngEnabled(false) {

	static const uint16 courseV0[] = {
		0xfcf, 0xef1, 0xe16, 0xd59, 0xc87, 0xbcd, 0xb27, 0xa93, 0x9ee, 0x96a, 0x8da, 0x865, 0x778, 0x70d, 0x6a6, 0x643,
		0x5ec, 0x599, 0x545, 0x4f7, 0x4b5, 0x46d, 0x432, 0x3f4, 0x3c5, 0x386, 0x353, 0x325, 0x2f6, 0x2cc, 0x2a3, 0x27d,
		0x25a, 0x238, 0x218, 0x1fa, 0x1dd, 0x1c3, 0x1a9, 0x192, 0x17b, 0x166, 0x152, 0x13f, 0x12d, 0x11c, 0x10c, 0x0fd,
		0x0ef, 0x0e1, 0x0d5, 0x0c9, 0x0bd, 0x0b3, 0x0a9, 0x09f, 0x096, 0x08e, 0x086, 0x07e, 0x077, 0x071, 0x06a, 0x064,
		0x05f, 0x059, 0x054, 0x050, 0x04b, 0x047, 0x043, 0x03f, 0x03c, 0x038, 0x035, 0x032, 0x02f, 0x02d, 0x02a, 0x028,
		0x026, 0x023, 0x021, 0x020, 0x01e, 0x01c, 0x01b, 0x019, 0x018, 0x016, 0x015, 0x014, 0x013, 0x012, 0x011, 0x010
	};

	static const uint16 courseV1[] = { 0x82D, 0x8A9, 0x92D, 0x9B6, 0xA4D, 0xAEA, 0xB90, 0xC40, 0xCFA, 0xDC0, 0xE90, 0xF6F };
	static const uint16 fine[] = { 0x7C, 0x84, 0x8B, 0x94, 0x9D, 0xA6, 0xB0, 0xBA, 0xC5, 0xD0, 0xDE, 0xEB };
	static const uint8 noiseFrq[] = { 0x00, 0x01, 0x04, 0x07, 0x0A, 0x0D, 0x10, 0x13, 0x16, 0x19, 0x1C, 0x1F };
	static const uint8 disableMask[] = { 0xF6, 0xED, 0xDB };
	static const uint8 enableMask1[] = { 0xBE, 0xBD, 0xBB };
	static const uint8 enableMask2[] = { 0xB7, 0xAF, 0x9F };

	_noteFrequency = (version > SCI_VERSION_0_LATE) ? courseV1 : courseV0;
	_noteFrequencyModifier = fine;
	_ngFreq = noiseFrq;
	_chanDisableMask = disableMask[id & 3];
	_chanEnableMask1 = enableMask1[id & 3];
	_chanEnableMask2 = enableMask2[id & 3];
}

void SoundChannel_PC9801_SSG::reset() {
	SoundChannel_PC9801::reset();
	_activeChannnelsStatus = (_version == SCI_VERSION_1_LATE) ? 0x3F : 0xFF;
	sendActiveChannelsStatus();
}

void SoundChannel_PC9801_SSG::toggleNoiseGenerator(bool enable) {
	_ccngEnabled = enable;
}

void SoundChannel_PC9801_SSG::programChange(uint8 program) {
	_selectedInstrument = _instrumentData.subspan(program * _patchSize + _patchOffset);

	if (_version == SCI_VERSION_1_LATE) {
		programChangeInit(_selectedInstrument);

		_flags &= ~kChanNgEnable;
		if (_selectedInstrument[7])
			_flags |= (kChanNgEnable | kChanNgRestartEnv);
		_instrumentChanMask = _selectedInstrument[22];
		_activeChannnelsStatus = (_activeChannnelsStatus & _chanDisableMask) | (~_chanDisableMask & (_instrumentChanMask & 0x3F));
		if (!(_instrumentChanMask & 8)) {
			_ngPhaseStep = _selectedInstrument[21] & 0x1F;
			updateNg();
		}

		sendActiveChannelsStatus();

	} else {
		writeReg(0, 13, _selectedInstrument[0]);
		writeReg(0,  6, _selectedInstrument[1]);
		writeReg(0, 11, _selectedInstrument[2]);
		writeReg(0, 12, _selectedInstrument[3]);
	}

	_currentLevel = 0;
	_ssgEnvelopeState = kEnvSSG_silent;
}

void SoundChannel_PC9801_SSG::processSounds() {
	if (_ssgEnvelopeState == kEnvSSG_silent)
		return;

	SoundChannel_PC9801::processSounds();

	if (!(_instrumentChanMask & 0x38) && (_flags & kChanNgEnable)) {
		bool cont = false;
		if (_flags & kChanNgRestartEnv) {
			if (--_ngEnvelopeTimer) {
				cont = true;
			} else {
				uint8 flg = _selectedInstrument[10] & 4;
				_ngSpeed = ((flg ? _selectedInstrument[9] : _selectedInstrument[8]) & 0x1F) >> 1;
				_flags = (_flags & ~(kChanNgDecrease | kChanNgRestartEnv)) | (flg << 3);
			}
		}
		if (!cont) {
			uint16 s = _ngEnvelopeTimer + (_selectedInstrument[10] >> 3);
			_ngEnvelopeTimer = s & 0xFF;
			if (s & 0x100)
				cont = true;
			if (!cont) {
				if (--_ngSpeed) {
					if (_flags & kChanNgDecrease) {
						_ngSpeed = _selectedInstrument[8] & 0x1F;
						_flags &= ~kChanNgDecrease;
					} else {
						_ngSpeed = _selectedInstrument[9] & 0x1F;
						_flags |= kChanNgDecrease;
					}
				}

				if (_flags & kChanNgDecrease) {
					_ngPhase -= (_selectedInstrument[9] >> 5) * ((_selectedInstrument[10] & 3) + 1);
				} else {
					_ngPhase += (_selectedInstrument[8] >> 5) * ((_selectedInstrument[10] & 3) + 1);
				}
				updateNg();
			}
		}
	}

	uint16 s = _ssgEnvelopeTimer + (_selectedInstrument[21] & 0x60);
	_ssgEnvelopeTimer = s & 0xFF;
	if ((s & 0x100) || !(_ssgEnvelopeState & 0x0F))
		return;

	uint8 vol = _currentLevel;
	if (_ssgLevel & 1)
		vol = (vol > (_ssgLevel & ~1)) ? vol - (_ssgLevel & ~1) : 0;
	else
		vol = (vol + _ssgLevel > 255) ? 255 : vol + _ssgLevel;

	if (!--_ssgSpeed) {
		if (--_ssgEnvelopeState == (kEnvSSG_keyOn | kEnvSSG_sustain)) {
			_ssgLevel = _selectedInstrument[13];
			_ssgSpeed = _selectedInstrument[14];
			vol = (uint8)CLIP<int16>(vol + (int8)(_selectedInstrument[19] & 0xF0), 0, 255);
		}
		if (_ssgEnvelopeState == (kEnvSSG_keyOn | kEnvSSG_decay)) {
			_ssgLevel = _selectedInstrument[15];
			_ssgSpeed = _selectedInstrument[16];
			vol = (uint8)CLIP<int16>(vol + (int8)((_selectedInstrument[20] & 0x0F) << 4), 0, 255);
		}
	}

	if (_ssgEnvelopeState == kEnvSSG_silent)
		vol = 0;

	if (_currentLevel != vol) {
		_currentLevel = vol;
		sendVolume();
	}
}

void SoundChannel_PC9801_SSG::sendSoundOnOff(bool soundOn) {
	if (_version == SCI_VERSION_1_LATE && soundOn && !(_ssgEnvelopeState & kEnvSSG_keyOn)) {
		_currentLevel = _selectedInstrument[19] << 4;
		_ssgEnvelopeState = (kEnvSSG_keyOn | kEnvSSG_attack);
		_ssgLevel = _selectedInstrument[11];
		_ssgSpeed = _selectedInstrument[12];
		_ssgEnvelopeTimer = 1;
		if (!(_selectedInstrument[22] & 0x38)) {
			if (_selectedInstrument[21] & 0x80)
				_ngPhaseStep = _ngFreq[_note % 12];
			_ngEnvelopeTimer = _selectedInstrument[7];
			_ngPhase = 0;
			updateNg();
			_flags |= kChanNgRestartEnv;
		}
	} else if (_version == SCI_VERSION_1_LATE && !soundOn) {
		int16 l = _currentLevel + (int8)(_selectedInstrument[20] & 0xF0);
		_currentLevel = (uint8)CLIP<int16>(l, 0, 255);
		_ssgEnvelopeState = kEnvSSG_decay;
		_ssgLevel = _selectedInstrument[17];
		_ssgSpeed = _selectedInstrument[18];
		_note = 0xFF;
	} else if (_version == SCI_VERSION_0_LATE && soundOn) {
		_activeChannnelsStatus |= ~_chanEnableMask1;
		if (_ccngEnabled)
			_activeChannnelsStatus |= ~_chanEnableMask2;
		sendActiveChannelsStatus();
		_activeChannnelsStatus &= _chanEnableMask1;
		if (_ccngEnabled)
			_activeChannnelsStatus &= _chanEnableMask2;
		_currentLevel = 1;
		sendActiveChannelsStatus();
		writeReg(0, 13, _selectedInstrument[0]);
	} else if (_version == SCI_VERSION_0_LATE) {
		_activeChannnelsStatus |= ~_chanEnableMask1;
		if (_ccngEnabled)
			_activeChannnelsStatus |= ~_chanEnableMask2;
		_currentLevel = 0;
		_note = 0xFF;
		sendActiveChannelsStatus();
	}
	sendVolume();
}

void SoundChannel_PC9801_SSG::sendVolume() {
	uint8 v1 = getVolume();
	uint16 r = 0;

	if (_version == SCI_VERSION_1_LATE) {
		uint8 vol = v1 + (((v1 >> 1) + v1) >> 2);

		if (vol > 0x7F)
			vol = 0x7F;
		vol = (vol >> 3) & 0x0F;

		r = _currentLevel & 0xF0;
		for (uint8 i = 0; i < 4; ++i) {
			r += r;
			if (r & 0x100)
				r = (r + vol) & 0xFF;
		}

		r = (r + 15) >> 4;

	} else {
		r = _currentLevel * (0x10 | v1 >> 3);
	}

	writeReg(0, 8 + _regOffs, r);
}

void SoundChannel_PC9801_SSG::sendFrequency() {
	uint16 freq = _frequencyCourse;

	if (_version > SCI_VERSION_0_LATE) {
		freq = (uint16)(freq + _vbrFrequencyModifier) >> (8 - _frequencyBlock);
		if (!freq)
			return;
		freq = 62400 / freq;
	}

	writeReg(0, _regOffs << 1, freq & 0xFF);
	writeReg(0, (_regOffs << 1) + 1, freq >> 8);
}

void SoundChannel_PC9801_SSG::updateNg() {
	int p = _ngPhase + _ngPhaseStep;
	if (p > 255)
		p = 0;
	if (p > 31)
		p = 31;
	_ngPhase = p & 0xFF;
	writeReg(0, 6, _ngPhase);
}

void SoundChannel_PC9801_SSG::sendActiveChannelsStatus() {
	writeReg(0, 7, (_activeChannnelsStatus & 0x3F) | 0x80);
}

MidiPart_PC9801::MidiPart_PC9801(uint8 id, SoundChannel_PC9801 **channels, uint8 numChan, SciVersion version) : _chan(channels), _id(id), _version(version), _numChan(numChan),
	_program(0), _volume(0x3f), _sustain(0), _chanMissing(0), _pitchBend(0x2000), _outChan(0), _noteRangeLow(version > SCI_VERSION_0_LATE ? 12 : 24), _noteRangeHigh(version > SCI_VERSION_0_LATE ? 107 : 119) {
}

void MidiPart_PC9801::noteOff(uint8 note) {
	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign != _id || _chan[i]->_note != note)
			continue;
		if (_sustain && _version > SCI_VERSION_0_LATE)
			_chan[i]->_sustain = 1;
		else
			_chan[i]->noteOff();
		return;
	}
}

void MidiPart_PC9801::noteOn(uint8 note, uint8 velo) {
	if (note < _noteRangeLow || note > _noteRangeHigh)
		return;

	if (velo == 0) {
		noteOff(note);
		return;
	}

	velo >>= 1;

	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign != _id || _chan[i]->_note != note)
			continue;
		_chan[i]->_sustain = 0;
		_chan[i]->noteOff();
		_chan[i]->noteOn(note, velo);
		return;
	}

	int chan = allocateChannel();
	if (chan != -1)
		_chan[chan]->noteOn(note, velo);
}

void MidiPart_PC9801::controlChangeVolume(uint8 vol) {
	_volume = (_version < SCI_VERSION_1_LATE) ? vol : CLIP(vol >> 1, 0, 0x3f);
	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign == _id && _chan[i]->_note != 0xff)
			_chan[i]->processNoteEvent(_chan[i]->_note, true);
	}
}

void MidiPart_PC9801::controlChangeSustain(uint8 sus) {
	if (_version < SCI_VERSION_1_LATE)
		return;

	_sustain = sus;
	if (_sustain)
		return;

	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign == _id && _chan[i]->_sustain) {
			_chan[i]->_sustain = 0;
			_chan[i]->noteOff();
		}
	}
}

void MidiPart_PC9801::controlChangePolyphony(uint8 numChan) {
	if (_version < SCI_VERSION_1_LATE)
		return;

	uint8 numAssigned = 0;
	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign == _id)
			numAssigned++;
	}

	numAssigned += _chanMissing;
	if (numAssigned < numChan) {
		addChannels(numChan - numAssigned);
	} else if (numAssigned > numChan) {
		dropChannels(numAssigned - numChan);
		assignFreeChannels();
	}
}

void MidiPart_PC9801::controlChangeNoiseGenerator(uint8 enable) {
	if (_version > SCI_VERSION_0_LATE)
		return;

	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign == _id)
			_chan[i]->toggleNoiseGenerator(enable);
	}
}

void MidiPart_PC9801::controlChangeAllNotesOff() {
	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign == _id && _chan[i]->_note != 0xff)
			_chan[i]->noteOff();
	}
}

void MidiPart_PC9801::programChange(uint8 prg) {
	_program = prg;
}

void MidiPart_PC9801::pitchBend(int16 val) {
	if (_version < SCI_VERSION_1_LATE)
		return;

	_pitchBend = val;
	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign == _id && _chan[i]->_note != 0xff)
			_chan[i]->processNoteEvent(_chan[i]->_note, true);
	}
}


void MidiPart_PC9801::addChannels(int num, int resetMissingChannels, int channelType) {
	if (resetMissingChannels != -1)
		_chanMissing = resetMissingChannels;

	for (int i = 0; i < _numChan; ++i) {
		if (_chan[i]->_assign != 0xFF || (channelType != -1 && _chan[i]->getType() != channelType))
			continue;

		_chan[i]->_assign = _id;

#ifdef DEBUG_REMAP
		debug("===== MidiDriver_PC9801: hardware channel %d is assigned to device channel %d =====", i, _id);
#endif
		if (_chan[i]->_note != 0xff)
			_chan[i]->noteOff();

		if (!--num)
			break;
	}
	_chanMissing += num;
}

void MidiPart_PC9801::dropChannels(int num) {
	if (_chanMissing == num) {
		_chanMissing = 0;
		return;
	} else if (_chanMissing > num) {
		_chanMissing -= num;
		return;
	}

	num -= _chanMissing;
	_chanMissing = 0;

	for (int i = 0; i < _numChan; i++) {
		if (_chan[i]->_assign != _id || _chan[i]->_note != 0xff)
			continue;
		_chan[i]->_assign = 0xff;

#ifdef DEBUG_REMAP
		debug("===== MidiDriver_PC9801: hardware channel %d is dropped from device channel %d =====", i, _id);
#endif

		if (!--num)
			return;
	}

	for (int i = 0; i < _numChan; i++) {
		if (_chan[i]->_assign != _id)
			continue;
		_chan[i]->_sustain = 0;
		_chan[i]->noteOff();
		_chan[i]->_assign = 0xff;
		if (!--num)
			return;
	}
}

int MidiPart_PC9801::allocateChannel() {
	int chan = _outChan;
	int ovrChan = 0;
	int ld = 0;
	bool found = false;

	for (bool loop = true; loop; ) {
		if (++chan == _numChan)
			chan = 0;

		if (chan == _outChan)
			loop = false;

		if (_id == _chan[chan]->_assign) {
			if (_chan[chan]->_note == 0xff || _version < SCI_VERSION_1_LATE) {
				found = true;
				break;
			}

			if (_chan[chan]->_duration >= ld) {
				ld = _chan[chan]->_duration;
				ovrChan = chan;
			}
		}
	}

	if (!found) {
		if (!ld)
			return -1;
		chan = ovrChan;
		_chan[chan]->_sustain = 0;
		_chan[chan]->noteOff();
	}

	_outChan = chan;
	return chan;
}

void MidiPart_PC9801::assignFreeChannels() {
	uint8 freeChan = 0;
	for (int i = 0; i < _numChan; i++) {
		if (_chan[i]->_assign == 0xff)
			freeChan++;
	}

	if (!freeChan)
		return;

	MidiDriver_PC9801::assignFreeChannels(freeChan);
}

MidiPart_PC9801 **MidiDriver_PC9801::_parts = nullptr;

MidiDriver_PC9801::MidiDriver_PC9801(Audio::Mixer *mixer, SciVersion version)
	: _mixer(mixer), _version(version), _pc98a(nullptr), _chan(nullptr), _numChan(6), _internalVersion(0xFF), _ssgPatchOffset(0xFF), _patchSize(0),
	_timerProc(nullptr), _timerProcPara(nullptr), _baseTempo(10080), _ready(false), _isOpen(false), _masterVolume(0x0f) ,_soundOn(true), _playID(0),
	_polyphony(9), _channelMask1(0x10), _channelMask2(0x02) {
}

MidiDriver_PC9801::~MidiDriver_PC9801() {
	close();
}

int MidiDriver_PC9801::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	if (!_pc98a) {
		_pc98a =
#ifdef SCI_PC98_AUDIO_EXTENDED
		new PC98AudioCore(_mixer, this, kType86);
#else
		new PC98AudioCore(_mixer, this, kType26);
#endif
	}

	if (!_ready) {
		if (!_pc98a->init())
			return MERR_CANNOT_CONNECT;
		_pc98a->setSoundEffectChanMask(0);
		_pc98a->ssgSetVolume(205);
		_ready = true;
	}

	ResourceManager *resMan = g_sci->getResMan();
	Resource *resource = resMan->findResource(ResourceId(kResourceTypePatch, _version < SCI_VERSION_1_LATE ? 2 : 8), false);
	if (resource == nullptr)
		return MERR_CANNOT_CONNECT;
	if (!loadInstruments(*resource))
		return MERR_CANNOT_CONNECT;

	if (_version == SCI_VERSION_0_LATE && _channelMask2 == 0x00) {
		_internalVersion = 0;
		_polyphony = 3;
	} else if (_version == SCI_VERSION_0_LATE && _channelMask2 == 0x02) {
		_internalVersion = 1;
		_polyphony = 6;
	} else if (_patchSize == 60 && _ssgPatchOffset == 37)
		_internalVersion = 2;
	else if (_patchSize == 81 && _ssgPatchOffset == 58)
		_internalVersion = 3;
	else
		return MERR_CANNOT_CONNECT;

	if (_internalVersion == 3)
		_numChan++;

	int config = _internalVersion;

#ifdef	SCI_PC98_AUDIO_EXTENDED
	_numChan = 9;
	config = 4;
#endif

	static const int channelConfig[6][11] = {
		{  0,  0,  0,  2,  2,  2, -1, -1, -1, -1, -1 },
		{  0,  0,  0,  2,  2,  2, -1, -1, -1, -1, -1 },
		{  0,  0,  0,  2,  2,  2, -1, -1, -1, -1, -1 },
		{  0,  0,  1,  1,  2,  2,  2, -1, -1, -1, -1 },
		{  0,  0,  0,  0,  0,  0,  2,  2,  2, -1, -1 }
	};

	_parts = new MidiPart_PC9801*[16];
	_chan = new SoundChannel_PC9801*[_numChan];

	int numSSG = 0;
	for (int i = 0; i < _numChan; ++i) {
		if (channelConfig[config][i] == 0)
			_chan[i] = new SoundChannel_PC9801_FM4OP(i, _pc98a, _parts, _version, *_instrumentData, _patchSize, _soundOn);
		else if (channelConfig[config][i] == 1)
			_chan[i] = new SoundChannel_PC9801_FM2OP(i, _pc98a, _parts, _version, *_instrumentData, _patchSize, _soundOn);
		else if (channelConfig[config][i] == 2)
			_chan[i] = new SoundChannel_PC9801_SSG(numSSG++, _pc98a, _parts, _version, *_instrumentData, _ssgPatchOffset, _patchSize, _soundOn);
		else
			_chan[i] = nullptr;
	}

	for (int i = 0; i < 16; ++i)
		_parts[i] = new MidiPart_PC9801(i, _chan, _numChan, _version);

	_isOpen = true;

	reset();

	return 0;
}

void MidiDriver_PC9801::close() {
	if (!_isOpen)
		return;

	bool ready = _ready;
	_isOpen = _ready = false;

	delete _pc98a;
	_pc98a = nullptr;

	if (_parts) {
		for (int i = 0; i < 16; ++i) {
			delete _parts[i];
			_parts[i] = nullptr;
		}
		delete[] _parts;
		_parts = nullptr;
	}

	if (_chan) {
		for (int i = 0; i < _numChan; ++i) {
			delete _chan[i];
			_chan[i] = nullptr;
		}
		delete[] _chan;
		_chan = nullptr;
	}

	_instrumentData.clear();

	_ready = ready;
}

void MidiDriver_PC9801::send(uint32 b) {
	if (!_isOpen)
		return;

	byte para2 = (b >> 16) & 0xFF;
	byte para1 = (b >> 8) & 0xFF;
	byte cmd = b & 0xF0;

	MidiPart_PC9801 *part = _parts[b & 0x0F];

	switch (cmd) {
	case 0x80:
		part->noteOff(para1);
		break;
	case 0x90:
		part->noteOn(para1, para2);
		break;
	case 0xb0:
		switch (para1) {
		case 7:
			if (_internalVersion > 0) // QFG ignores channel volume
				part->controlChangeVolume(para2);
			break;
		case 64:
			part->controlChangeSustain(para2);
			break;
		case SCI_MIDI_SET_POLYPHONY:
			part->controlChangePolyphony(para2);
			break;
		case 76:
			// This event (from the SCI0 driver) is parsing related and can't be handled here. Lets's see if this ever comes up.
			warning("MidiDriver_PC9801: Midi Control Change '0x%2x' not implemented", para1);
			break;
		case 81:
			part->controlChangeNoiseGenerator(para2);
			break;
		case 96:
			// This event (from the SCI0 driver) is parsing related. It is handled in MidiParser_SCI::processEvent().
			break;
		case SCI_MIDI_CHANNEL_NOTES_OFF:
			part->controlChangeAllNotesOff();
			break;
		default:
			break;
		}
		break;
	case 0xc0:
		if (_internalVersion > 0) // QFG ignores program changes
			part->programChange(para1);
		break;
	case 0xe0:
		part->pitchBend(para1 | (para2 << 7));
		break;
	default:
		break;
	}
}

void MidiDriver_PC9801::assignFreeChannels(int num) {
	assert(_parts);
	for (int i = 0; i < 16; ++i) {
		uint8 missing = _parts[i]->getMissingChannels();
		if (!missing)
			continue;
		if (missing < num) {
			num -= missing;
			_parts[i]->addChannels(missing, 0);
		} else {
			_parts[i]->addChannels(num, missing - num);
			return;
		}
	}
}

uint32 MidiDriver_PC9801::property(int prop, uint32 param) {
	if (!_isOpen)
		return 0;

	switch(prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff && param != _masterVolume && param < 16) {
			_masterVolume = param;
			for (int i = 0; i < _numChan; ++i) {
				_chan[i]->setVolume(_masterVolume);
				if (_chan[i]->_note != 0xFF)
					_chan[i]->processNoteEvent(_chan[i]->_note, true);
			}
		}
		return _masterVolume;
	case MIDI_PROP_PLAYSWITCH:
		_soundOn = param;
		break;
	case MIDI_PROP_POLYPHONY:
		return _polyphony;
	case MIDI_PROP_CHANNEL_ID:
		return _version < SCI_VERSION_1_LATE ? (_channelMask1 | _channelMask2) : _playID;
	default:
		break;
	}
	return 0;
}

void MidiDriver_PC9801::initTrack(SciSpan<const byte> &header) {
	if (!_isOpen || _version > SCI_VERSION_0_LATE)
		return;

	for (int i = 0; i < _numChan; ++i)
		_chan[i]->reset();

	uint8 readPos = 0;
	uint8 caps = header.getInt8At(readPos++);
	int numChan = (caps == 2) ? 15 : 16;
	if (caps != 0 && caps != 2)
		return;

	for (int i = 0; i < numChan; ++i) {
		_parts[i]->controlChangeVolume(103);

		uint8 num = (_internalVersion == 1) ? (header.getInt8At(readPos) & 0x7F) : 1;
		readPos++;
		uint8 flags = header.getInt8At(readPos);
		readPos++;

		if (flags & _channelMask1 && num)
			_parts[i]->addChannels(num, -1, 0);

		if (flags & _channelMask2 && num)
			_parts[i]->addChannels(num, -1, 2);

		if (_internalVersion == 0)
			_parts[i]->programChange(10);
	}
}

void MidiDriver_PC9801::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	_timerProc = timer_proc;
	_timerProcPara = timer_param;
}

uint32 MidiDriver_PC9801::getBaseTempo() {
	return _baseTempo;
}

void MidiDriver_PC9801::timerCallbackB() {
	if (!_isOpen)
		return;
	updateParser();
	updateChannels();
}

bool MidiDriver_PC9801::loadInstruments(const SciSpan<const uint8> &data) {
	if (!data)
		return false;

	SciSpan<const uint8> src = data;
	_instrumentData.clear();

	if (_version == SCI_VERSION_0_LATE) {
		_ssgPatchOffset = 48;
		_patchSize = 52;

		_instrumentData->allocate(96 * _patchSize);
		SciSpan<uint8> dst = *_instrumentData;

		for (bool load = true; load; ) {
			for (int i = 0; i < 48; ++i) {
				src.subspan(0, _patchSize).copyDataTo(dst);
				src += 64;
				dst += _patchSize;
			}
			uint16 id = (src.byteSize() >= 2) ? src.getInt16BEAt(0) : 0;
			if (id == 0xABCD || id == 0xCDAB) {
				src += 2;
				_channelMask2 = 0x00;
			} else {
				load = false;
			}
		}
	} else if (_version == SCI_VERSION_1_LATE) {
		_instrumentData->allocateFromSpan(++src);
		_patchSize = (data.byteSize() - 1) / 96;
		_ssgPatchOffset = (_patchSize == 81) ? 58 : 37;
	}

	return (_instrumentData->byteSize() && _patchSize && _ssgPatchOffset != 0xFF);
}

void MidiDriver_PC9801::updateParser() {
	if (_timerProc) {
		// The mutex lock has to be lifted, before entering the SCI engine space. The engine has its owns mutex and the different threads
		// will often cause an immediate lockup (each thread caught in the mutex lock of the other). I consider this safe for all realistic
		// scenarios, since a reentry of the space guarded by the PC98 audio mutex is not possible without triggering another mutex lock.
		// I have also rearranged the driver deconstruction appropriately.
		PC98AudioCore::MutexLock tempUnlock = _pc98a->stackUnlockMutex();
		_timerProc(_timerProcPara);
	}
}

void MidiDriver_PC9801::updateChannels() {
	for (int i = 0; i < _numChan; ++i)
		_chan[i]->update();
}

void MidiDriver_PC9801::reset() {
	if (!_ready)
		return;

	for (int i = 0; i < 3; ++i) {
		_pc98a->writeReg(0, 0x28, i);
		_pc98a->writeReg(0, i, 0);
		_pc98a->writeReg(0, 8 + i, 0);
	}

	uint8 flag = 0;
#ifndef SCI_PC98_AUDIO_EXTENDED
	if (_internalVersion == 3) {
		_pc98a->writeReg(0, 0xB2, 0x04);
		flag = 0x40;
	}
#endif

	_pc98a->writeReg(0, 0x27, 0x38);
	_pc98a->writeReg(0, 0x27, 0x3a | flag);

	if (!_isOpen)
		return;

	for (int i = 0; i < _numChan; ++i)
		_chan[i]->reset();
}

MidiPlayer_PC9801::MidiPlayer_PC9801(SciVersion version) : MidiPlayer(version) {
	_driver = new MidiDriver_PC9801(g_system->getMixer(), version);
}

MidiPlayer_PC9801::~MidiPlayer_PC9801() {
	delete _driver;
}

bool MidiPlayer_PC9801::hasRhythmChannel() const {
	return false;
}

byte MidiPlayer_PC9801::getPlayId() const {
	return _driver ? _driver->property(MIDI_PROP_CHANNEL_ID, 0) : 0;
}

int MidiPlayer_PC9801::getPolyphony() const {
	return _driver ? _driver->property(MIDI_PROP_POLYPHONY, 0) : 0;
}

void MidiPlayer_PC9801::playSwitch(bool play) {
	if (_driver)
		_driver->property(MIDI_PROP_PLAYSWITCH, play ? 1 : 0);
}

void MidiPlayer_PC9801::initTrack(SciSpan<const byte> &trackData) {
	if (_driver)
		static_cast<MidiDriver_PC9801*>(_driver)->initTrack(trackData);
}

MidiPlayer *MidiPlayer_PC9801_create(SciVersion _soundVersion) {
	return new MidiPlayer_PC9801(_soundVersion);
}

} // End of namespace Sci

