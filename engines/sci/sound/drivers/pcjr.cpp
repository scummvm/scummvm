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

#include "sci/sound/drivers/mididriver.h"
#include "sci/resource/resource.h"

#include "audio/softsynth/emumidi.h"

#include "common/debug.h"
#include "common/system.h"

namespace Sci {

class SoundHWChannel {
public:
	SoundHWChannel(const Common::SharedPtr<const uint16> &freqTable, const Common::SharedPtr<const uint16> &smpVolTable, const byte &masterVol, int outputRate);
	virtual ~SoundHWChannel() {}

	virtual void reset();
	virtual void updateChannelVolume() = 0;
	virtual void processEnvelope() = 0;
	void recalcSample();

	void setPart(byte number) { _partMapping = number; }

	virtual void noteOn(byte note, byte velocity) = 0;
	virtual void noteOff(byte sustain) = 0;
	virtual void chanOff();
	virtual void sustainOff() {}
	void pitchBend(uint16 pb);
	void setVolume(byte volume) { _ctrlVolume = volume; }

	int16 currentSample() const { return _curSample; }
	bool isMappedToPart(byte part) const { return _partMapping == part; }
	bool hasNote(byte note) const { return _note == note; }
	bool isPlaying() const { return _note != 0xFF; }
	virtual bool isSustained() const { return false; }
	virtual uint16 getDuration() const { return 0; }

protected:
	byte _partMapping;
	byte _ctrlVolume;
	byte _velocity;
	int8 _envVolume;
	byte _note;
	byte _pbDiv;
	byte _envState;
	byte _envCount;
	int8 _envAttn;
	uint16 _smpAmplitude;

	const int _outputRate;
	const byte &_masterVolume;
	const Common::SharedPtr<const uint16> _freqTable;
	const Common::SharedPtr<const uint16> _smpVolTable;

private:
	int getFrequency() const;

	int16 _pitchBend;
	int16 _curSample;
	int _freqCount;
};

class SoundChannel_PCSpeaker : public SoundHWChannel {
public:
	SoundChannel_PCSpeaker(const Common::SharedPtr<const uint16> &freqTable, const Common::SharedPtr<const uint16> &smpVolTable, const byte &masterVol, int outputRate) : SoundHWChannel(freqTable, smpVolTable, masterVol, outputRate) {}
	~SoundChannel_PCSpeaker() override {}

	void reset() override;
	void updateChannelVolume() override;
	void processEnvelope() override {}

	void noteOn(byte note, byte velocity) override;
	void noteOff(byte) override;
};

class SoundChannel_PCJr_SCI0 : public SoundHWChannel {
public:
	SoundChannel_PCJr_SCI0(const Common::SharedPtr<const uint16> &freqTable, const Common::SharedPtr<const uint16> &smpVolTable, const byte &masterVol, int outputRate) : SoundHWChannel(freqTable, smpVolTable, masterVol, outputRate), _envCount2(0) {}
	~SoundChannel_PCJr_SCI0() override {}

	void reset() override;
	void updateChannelVolume() override;
	void processEnvelope() override;

	void noteOn(byte note, byte velocity) override;
	void noteOff(byte) override;

private:
	void envAT();
	void envST();
	void envRL();

	byte _envCount2;
};

class SoundChannel_PCJr_SCI1 : public SoundHWChannel {
public:
	SoundChannel_PCJr_SCI1(const Common::SharedPtr<const uint16> &freqTable, const Common::SharedPtr<const uint16> &smpVolTable, const byte &masterVol, int outputRate, const uint16 *&instrumentOffsets, const byte *&instrumentData, byte *&program) :
		SoundHWChannel(freqTable, smpVolTable, masterVol, outputRate), _instrumentOffsets(instrumentOffsets), _instrumentData(instrumentData), _program(program), _duration(0), _releaseDuration(0), _sustain(0),
			_release(0), _envData(nullptr) {}
	~SoundChannel_PCJr_SCI1() override {}

	void reset() override;
	void updateChannelVolume() override;
	void processEnvelope() override;

	void noteOn(byte note, byte velocity) override;
	void noteOff(byte sustain) override;
	void chanOff() override;
	void sustainOff() override;

	bool isSustained() const override { return _sustain != 0; }
	uint16 getDuration() const override { return _releaseDuration ? _releaseDuration + 0x8000 : _duration; }

private:
	void processInstrument();

	uint16 _duration;
	uint16 _releaseDuration;
	byte _sustain;
	byte _release;
	byte *&_program;
	const byte *_envData;
	const uint16 *&_instrumentOffsets;
	const byte *&_instrumentData;
};

SoundHWChannel::SoundHWChannel(const Common::SharedPtr<const uint16> &freqTable, const Common::SharedPtr<const uint16> &smpVolTable, const byte &masterVol, int outputRate) : _freqTable(freqTable), _smpVolTable(smpVolTable), _masterVolume(masterVol), _outputRate(outputRate) {
	reset();
}

void SoundHWChannel::reset() {
	_partMapping = _note = 0xFF;
	_ctrlVolume = _velocity = 0;
	_envVolume = 0;
	_envAttn = 0;
	_envState = _envCount = 0;
	_freqCount = 0;
	_smpAmplitude = 0;
	_curSample = 0;
	_pitchBend = 0;
	_pbDiv = 0;
}

void SoundHWChannel::recalcSample() {
	int freq = getFrequency();

	if (freq == 0) {
		_curSample = 0;
		return;
	}

	_freqCount += freq;

	while (_freqCount >= (_outputRate << 1))
		_freqCount -= (_outputRate << 1);

	if (_freqCount - freq < 0) {
		// Unclean rising edge
		int l = _smpAmplitude << 1;
		_curSample = (int16)~_smpAmplitude + (l * _freqCount) / freq;
	} else if (_freqCount >= _outputRate && _freqCount - freq < _outputRate) {
		// Unclean falling edge
		int l = _smpAmplitude << 1;
		_curSample = _smpAmplitude - (l * (_freqCount - _outputRate)) / freq;
	} else {
		if (_freqCount < _outputRate)
			_curSample = _smpAmplitude;
		else
			_curSample = (int16)~_smpAmplitude;
	}
}

void SoundHWChannel::chanOff() {
	_note = 0xFF;
	_envState = 0;
	_envAttn = 0;
	_envVolume = 0;
	_envCount = 0;
}

void SoundHWChannel::pitchBend(uint16 pb) {
	_pitchBend = ((pb < 0x2000) ? -(0x2000 - pb) : pb - 0x2000) / _pbDiv;
}

#define SCI_PCJR_BASE_NOTE 129	// A10
#define SCI_PCJR_BASE_OCTAVE 10	// A10, as I said

int SoundHWChannel::getFrequency() const {
	if (_note == 0xFF)
		return 0;

	int halftoneDelta = (_note - SCI_PCJR_BASE_NOTE) * 4 + _pitchBend;
	int octDiff = ((halftoneDelta + SCI_PCJR_BASE_OCTAVE * 48) / 48) - SCI_PCJR_BASE_OCTAVE;
	int halftoneIndex = (halftoneDelta + (48 * 100)) % 48;
	int freq = _freqTable.get()[halftoneIndex] / (1 << (-octDiff));

	return freq;
}

#undef SCI_PCJR_BASE_NOTE
#undef SCI_PCJR_BASE_OCTAVE

void SoundChannel_PCSpeaker::reset() {
	SoundHWChannel::reset();
	_pbDiv = 171;
	updateChannelVolume();
}

void SoundChannel_PCSpeaker::updateChannelVolume() {
	// The PC speaker has a fixed volume level. The original will turn it off if the volume is zero
	// or otherwise turn it on. We just use the PCJr volume table for the master volume, so that our
	// volume controls still work.
	_smpAmplitude = _smpVolTable.get()[15 - _masterVolume];
}

void SoundChannel_PCSpeaker::noteOn(byte note, byte velocity) {
	_note = note;
}

void SoundChannel_PCSpeaker::noteOff(byte) {
	chanOff();
}

void SoundChannel_PCJr_SCI0::reset() {
	SoundHWChannel::reset();
	_ctrlVolume = 96;
	_pbDiv = 171;
	updateChannelVolume();
}

void SoundChannel_PCJr_SCI0::updateChannelVolume() {
	int veloAttn = 3 - (_velocity >> 5);
	int volAttn = 15 - (_ctrlVolume >> 3);
	int attn = volAttn + veloAttn + _envVolume;
	if (attn >= 15) {
		attn = 15;
		if (_envState >= 2)
			chanOff();
	} else if (attn < 0) {
		attn = 0;
	}
	attn = CLIP<int>(attn + (15 - _masterVolume), volAttn, 15);
	_smpAmplitude = _smpVolTable.get()[attn];
}

void SoundChannel_PCJr_SCI0::processEnvelope() {
	switch (_envState) {
	case 1:
		envAT();
		break;
	case 2:
		envST();
		break;
	case 3:
		envRL();
		break;
	default:
		break;
	}
}

void SoundChannel_PCJr_SCI0::noteOn(byte note, byte velocity) {
	_note = note;
	_velocity = velocity;
	_envState = 1;
	_envCount = 0;
	envAT();
}

void SoundChannel_PCJr_SCI0::noteOff(byte) {
	_envState = 3;
	_envCount = 0;
	envRL();
}

void SoundChannel_PCJr_SCI0::envAT() {
	static const int8 envTable[] = { 0, -2, -3, -2, -1, 0, 127 };
	byte c = _envCount++;
	if (envTable[c] == 127) {
		_envState++;
		_envCount = 0;
		_envCount2 = 20;
		envST();
	} else {
		// The original makes a signed check whether the table value is negative. If negative, it will put
		// zero, if zero or positive it will put the table value. Since the table is all zero or negative,
		// it will always be zero. This is weird, but just just be the remainder of a sloppy bug fix. If
		// we put the table value instead of zero, e. g. the LSL3 opening fanfare will sound wrong.
		_envVolume = _envAttn = 0;
		updateChannelVolume();
	}
}

void SoundChannel_PCJr_SCI0::envST() {
	if (!_envCount2 || --_envCount2)
		return;
	_envVolume = ++_envAttn;
	_envCount2 = 20;
	updateChannelVolume();
}

void SoundChannel_PCJr_SCI0::envRL() {
	static const byte envTable[] = { 1, 1, 1, 2, 2, 3, 3, 4, 4, 6, 6, 7, 8, 9, 10, 11, 12, 127 };
	byte c = _envCount++;
	_envVolume = (envTable[c] == 127) ? 15 : _envAttn + envTable[c];
	updateChannelVolume();
}

void SoundChannel_PCJr_SCI1::reset() {
	SoundHWChannel::reset();
	_duration = _releaseDuration = 0;
	_sustain = _release = 0;
	_ctrlVolume = _envVolume = 15;
	_envData = nullptr;
	_pbDiv = 170;
	updateChannelVolume();	
}

void SoundChannel_PCJr_SCI1::updateChannelVolume() {
	static const byte veloTable[16] = { 0x01, 0x03, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F };
	int velo = _velocity ? veloTable[_velocity >> 3] : 0;
	int vol = _ctrlVolume ? MAX<int>(1, _ctrlVolume >> 3) : 0;
	int tl = ((vol * velo / 15) * _envVolume / 15) * _masterVolume;
	if (tl > 0 && tl < 15)
		tl = 15;
	int attn = 15 - tl / 15;
	_smpAmplitude = _smpVolTable.get()[attn];
}

void SoundChannel_PCJr_SCI1::processEnvelope() {
	++_duration;
	if (_release)
		++_releaseDuration;
	processInstrument();
	updateChannelVolume();
}

void SoundChannel_PCJr_SCI1::noteOn(byte note, byte velocity) {
	_note = note;
	_velocity = velocity;
	_envState = 0;
	_envCount = 0;
	_duration = 0;
	_releaseDuration = 0;
	_release = 0;
	_sustain = 0;

	assert(_instrumentOffsets);
	assert(_instrumentData);
	_envData = &_instrumentData[_instrumentOffsets[_program[_partMapping]]];
}

void SoundChannel_PCJr_SCI1::noteOff(byte sustain) {
	byte &dest = sustain ? _sustain : _release;
	dest = 1;
}

void SoundChannel_PCJr_SCI1::chanOff() {
	SoundHWChannel::chanOff();
	_duration = 0;
	_releaseDuration = 0;
	_sustain = 0;
	_release = 0;
}

void SoundChannel_PCJr_SCI1::sustainOff() {
	_sustain = 0;
	_release = 1;
}

void SoundChannel_PCJr_SCI1::processInstrument() {
	if (_envCount == 0xFE) {
		if (_release)
			_envCount = 0;
		else
			return;
	}

	if (_envCount == 0) {
		byte a = _envData[_envState << 1];
		if (a == 0xFF) {
			chanOff();
			_envCount = 0;
		} else {
			_envCount = _envData[(_envState << 1) + 1];
			_envVolume = a;
			++_envState;
		}
	} else {
		--_envCount;
	}
}

class MidiDriver_PCJr : public MidiDriver_Emulated {
public:
	friend class MidiPlayer_PCJr;
	enum Properties {
		kPropNone = 0,
		kPropVolume
	};

	MidiDriver_PCJr(Audio::Mixer *mixer, SciVersion version, bool pcsMode);
	~MidiDriver_PCJr() override;

	// MidiDriver
	int open() override;
	void close() override;
	void send(uint32 b) override;
	MidiChannel *allocateChannel() override { return nullptr; }
	MidiChannel *getPercussionChannel() override { return nullptr; }
	uint32 property(int prop, uint32 value) override;
	void initTrack(SciSpan<const byte> &header);
	
	// AudioStream
	bool isStereo() const override { return false; }
	int getRate() const override { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len) override;

private:
	bool loadInstruments(Resource &resource);
	void processEnvelopes();

	void noteOn(byte part, byte note, byte velocity);
	void noteOff(byte part, byte note);
	void controlChange(byte part, byte controller, byte value);
	void programChange(byte part, byte program);
	void pitchBend(byte part, uint16 value);

	void controlChangeSustain(byte part, byte sus);
	void controlChangePolyphony(byte part, byte numChan);
	void addChannels(byte part, byte num);
	void dropChannels(byte part, byte num);
	void assignFreeChannels(byte part);
	byte allocateChannel(byte part);

	byte _masterVolume;
	byte *_chanMapping;
	byte *_chanMissing;
	byte *_program;
	byte *_sustain;

	uint32 _sndUpdateSmpQty;
	uint32 _sndUpdateSmpQtyRem;
	uint32 _sndUpdateCountDown;
	uint32 _sndUpdateCountDownRem;

	SoundHWChannel **_channels;

	const uint16 *_instrumentOffsets;
	const byte *_instrumentData;

	const SciVersion _version;
	const byte _numChannels;
	const bool _pcsMode;
};

void MidiDriver_PCJr::send(uint32 b) {
	byte command = b & 0xff;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;
	byte part = command & 0x0f;

	switch (command & 0xf0) {
	case 0x80:
		noteOff(part, op1);
		break;
	case 0x90:
		if (!op2)
			noteOff(part, op1);
		else 
			noteOn(part, op1, op2);	
		break;
	case 0xb0:
		controlChange(part, op1, op2);
		break;
	case 0xc0:
		programChange(part, op1);
		break;
	case 0xe0:
		pitchBend(part, (op1 & 0x7f) | ((op2 & 0x7f) << 7));
		break;
	default:
		debug(2, "Unused MIDI command %02x %02x %02x", command, op1, op2);
		break;
	}

	if (!_pcsMode) {
		_sndUpdateCountDown = 1;
		_sndUpdateCountDownRem = 0;
	}
}

uint32 MidiDriver_PCJr::property(int prop, uint32 value) {
	uint32 res = 0;
	value &= 0xffff;

	switch (prop) {
	case kPropVolume:
		res = _masterVolume;
		if (value != 0xffff) {
			_masterVolume = value;
			for (int i = 0; i < _numChannels; ++i)
				_channels[i]->updateChannelVolume();
		}
		break;
	default:
		break;
	}

	return res;
}

void MidiDriver_PCJr::initTrack(SciSpan<const byte> &header) {
	if (!_isOpen || _version > SCI_VERSION_0_LATE)
		return;

	uint8 readPos = 0;
	uint8 caps = header.getInt8At(readPos++);
	if (caps != 0 && caps != 2)
		return;

	for (int i = 0; i < _numChannels; ++i)
		_channels[i]->reset();

	if (_version == SCI_VERSION_0_EARLY) {
		byte chanFlag = _pcsMode ? 0x04 : 0x02;
		for (int i = 0, numAssigned = 0; i < 16 && numAssigned < _numChannels; ++i) {
			uint8 f = header.getInt8At(++readPos);
			if ((!(f & 8) || (f & 1)) && (f & chanFlag))
				_channels[numAssigned++]->setPart(i);
		}
	} else {
		byte chanFlag = _pcsMode ? 0x20 : 0x10;
		for (int i = 0, numAssigned = 0; i < 16 && numAssigned < _numChannels; ++i) {
			uint8 f = header.getInt8At(++readPos);
			readPos++;
			if (f & chanFlag)
				_channels[numAssigned++]->setPart(i);
		}
	}
}

void MidiDriver_PCJr::generateSamples(int16 *data, int len) {
	for (int i = 0; i < len; i++) {
		if (!--_sndUpdateCountDown) {
			_sndUpdateCountDown = _sndUpdateSmpQty;
			_sndUpdateCountDownRem += _sndUpdateSmpQtyRem;
			while (_sndUpdateCountDownRem >= (_sndUpdateSmpQty << 16)) {
				_sndUpdateCountDownRem -= (_sndUpdateSmpQty << 16);
				++_sndUpdateCountDown;
			}
			processEnvelopes();
		}

		int16 result = 0;

		for (int chan = 0; chan < _numChannels; chan++) {
			if (!_channels[chan]->isPlaying())
				continue;
			_channels[chan]->recalcSample();
			result += _channels[chan]->currentSample();
		}
		data[i] = result;
	}
}

MidiDriver_PCJr::MidiDriver_PCJr(Audio::Mixer *mixer, SciVersion version, bool pcsMode) : MidiDriver_Emulated(mixer), _version(version), _pcsMode(pcsMode), _numChannels(pcsMode ? 1 : 3),
	_masterVolume(0), _channels(nullptr), _instrumentOffsets(nullptr), _instrumentData(nullptr) {

	uint16 *smpVolTable = new uint16[16]();
	for (int i = 0; i < 15; ++i) // The last entry is left at zero.
		smpVolTable[i] = (double)((32767 & ~_numChannels) / _numChannels) / pow(10.0, (double)i / 10.0);
	Common::SharedPtr<const uint16> smpVolTablePtr(smpVolTable, Common::ArrayDeleter<const uint16>());

	uint16 *freqTable = new uint16[48]();
	assert(freqTable);
	for (int i = 0; i < 48; ++i)
		freqTable[i] = pow(2, (double)(288 + i) / 48.0) * 440.0;
	Common::SharedPtr<const uint16> freqTablePtr(freqTable, Common::ArrayDeleter<const uint16>());

	_chanMapping = new byte[16]();
	_chanMissing = new byte[16]();
	_program = new byte[16]();
	_sustain = new byte[16]();

	assert(_chanMapping);
	assert(_chanMissing);
	assert(_program);
	assert(_sustain);

	_channels = new SoundHWChannel*[_numChannels]();
	assert(_channels);
	for (int i = 0; i < _numChannels; ++i) {
		if (pcsMode)
			_channels[i] = new SoundChannel_PCSpeaker(freqTablePtr, smpVolTablePtr, _masterVolume, getRate());
		else if (version <= SCI_VERSION_0_LATE)
			_channels[i] = new SoundChannel_PCJr_SCI0(freqTablePtr, smpVolTablePtr, _masterVolume, getRate());
		else
			_channels[i] = new SoundChannel_PCJr_SCI1(freqTablePtr, smpVolTablePtr, _masterVolume, getRate(), _instrumentOffsets, _instrumentData, _program);
	}

	_sndUpdateSmpQty = (mixer->getOutputRate() << 16) / 0x3C0000;
	_sndUpdateSmpQtyRem = (mixer->getOutputRate() << 16) % 0x3C0000;
	_sndUpdateCountDown = _sndUpdateSmpQty;
	_sndUpdateCountDownRem = 0;
}

MidiDriver_PCJr::~MidiDriver_PCJr() {
	close();
	if (_channels) {
		for (int i = 0; i < _numChannels; ++i)
			delete _channels[i];
		delete[] _channels;
	}

	delete[] _instrumentOffsets;
	delete[] _instrumentData;
	delete[] _chanMapping;
	delete[] _chanMissing;
	delete[] _program;
	delete[] _sustain;
}

int MidiDriver_PCJr::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_masterVolume = 15;
	for (int i = 0; i < _numChannels; i++)
		_channels[i]->reset();

	if (_version > SCI_VERSION_0_LATE && !_pcsMode) {
		ResourceManager *resMan = g_sci->getResMan();
		Resource *resource = resMan->findResource(ResourceId(kResourceTypePatch, 101), false);
		if (resource == nullptr)
			return MERR_CANNOT_CONNECT;
		if (!loadInstruments(*resource))
			return MERR_CANNOT_CONNECT;
	}

	int res = MidiDriver_Emulated::open();

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	return res;
}

void MidiDriver_PCJr::close() {
	_mixer->stopHandle(_mixerSoundHandle);
}

bool MidiDriver_PCJr::loadInstruments(Resource &resource) {
	uint16 *instrumentOffsets = new uint16[128]();
	if (!instrumentOffsets)
		return false;

	uint32 readPos = 0;
	for (int i = 0; i < 128; ++i) {
		int in = resource.getUint16LEAt(readPos) - 0x100;
		if (in < 0) {
			warning("%s(): Error reading instrument patch resource", __FUNCTION__);
			delete[] instrumentOffsets;
			return false;
		}
		instrumentOffsets[i] = in;
		readPos += 2;
	}

	uint8 *instrumentData = new uint8[770]();
	if (!instrumentData)
		return false;

	uint32 writePos = 0;
	for (int i = 0; i < 40; ++i) {
		for (uint8 in = 0; writePos < 770 && in != 0xFF; ) {
			in = resource.getUint8At(readPos++);
			instrumentData[writePos++] = in;			
		}
	}

	_instrumentOffsets = instrumentOffsets;
	_instrumentData = instrumentData;

	return true;
}

void MidiDriver_PCJr::processEnvelopes() {
	for (int i = 0; i < _numChannels; i++) {
		if (_channels[i]->isPlaying())
			_channels[i]->processEnvelope();
	}
}

void MidiDriver_PCJr::noteOn(byte part, byte note, byte velocity) {
	if (_pcsMode) {
		if (!_channels[0]->isMappedToPart(part))
			return;
		_channels[0]->chanOff();
		if (note >= 24 && note <= 119)
			_channels[0]->noteOn(note, velocity);
		return;
	} else if (_version > SCI_VERSION_0_LATE) {
		if (note < 21 || note > 116)
			return;

		for (int i = 0; i < _numChannels; ++i) {
			if (!_channels[i]->isMappedToPart(part) || !_channels[i]->hasNote(note))
				continue;
			_channels[i]->chanOff();
			_channels[i]->noteOn(note, velocity);
			return;
		}
	} else if (note == 0 || note == 1) {
		return;
	}

	byte c = allocateChannel(part);
	if (c != 0xFF)
		_channels[c]->noteOn(note, velocity);
}

void MidiDriver_PCJr::noteOff(byte part, byte note) {
	for (int i = 0; i < _numChannels; ++i) {
		if (!_channels[i]->isMappedToPart(part) || !_channels[i]->hasNote(note))
			continue;
		_channels[i]->noteOff(_sustain[part]);
	}	
}

void MidiDriver_PCJr::controlChange(byte part, byte controller, byte value) {
	switch (controller) {
	case 7:
		for (int i = 0; i < _numChannels; ++i) {
			if (_channels[i]->isMappedToPart(part))
				_channels[i]->setVolume(value);
		}
		break;
	case 64:
		controlChangeSustain(part, value);
		break;
	case SCI_MIDI_SET_POLYPHONY:
		controlChangePolyphony(part, value);
		break;
	case SCI_MIDI_CHANNEL_NOTES_OFF:
	case SCI_MIDI_CHANNEL_SOUND_OFF:
		for (int i = 0; i < _numChannels; ++i) {
			if (_channels[i]->isMappedToPart(part))
				_channels[i]->chanOff();
		}
		break;
	default:
		break;
	}
}

void MidiDriver_PCJr::programChange(byte part, byte program) {
	if (_version > SCI_VERSION_0_LATE && !_pcsMode)
		_program[part] = program;
}

void MidiDriver_PCJr::pitchBend(byte part, uint16 value) {
	for (int i = 0; i < _numChannels; ++i) {
		if (_channels[i]->isMappedToPart(part))
			_channels[i]->pitchBend(value);
	}
}

void MidiDriver_PCJr::controlChangeSustain(byte part, byte sus) {
	if (_version <= SCI_VERSION_0_LATE || _pcsMode)
		return;

	_sustain[part] = sus;
	if (sus)
		return;

	for (int i = 0; i < _numChannels; ++i) {
		if (_channels[i]->isMappedToPart(part) && _channels[i]->isSustained())
			_channels[i]->sustainOff();
	}
}

void MidiDriver_PCJr::controlChangePolyphony(byte part, byte numChan) {
	if (_version <= SCI_VERSION_0_LATE)
		return;

	if (_pcsMode) {
		if (numChan == 0 || !_channels[0]->isMappedToPart(part))
			_channels[0]->chanOff();
		_channels[0]->setPart(numChan ? part : 0xFF);
		return;
	}

	uint8 numAssigned = 0;
	for (int i = 0; i < _numChannels; ++i) {
		if (_channels[i]->isMappedToPart(part))
			numAssigned++;
	}

	numAssigned += _chanMissing[part];
	if (numAssigned < numChan) {
		addChannels(part, numChan - numAssigned);
	} else if (numAssigned > numChan) {
		dropChannels(part, numAssigned - numChan);
		assignFreeChannels(part);
	}
}

void MidiDriver_PCJr::addChannels(byte part, byte num) {
	for (int i = 0; i < _numChannels; ++i) {
		if (!_channels[i]->isMappedToPart(0xFF))
			continue;
		_channels[i]->setPart(part);

		if (_channels[i]->isPlaying())
			_channels[i]->chanOff();

		if (!--num)
			break;
	}
	_chanMissing[part] += num;
}

void MidiDriver_PCJr::dropChannels(byte part, byte num) {
	if (_chanMissing[part] == num) {
		_chanMissing[part] = 0;
		return;
	} else if (_chanMissing[part] > num) {
		_chanMissing[part] -= num;
		return;
	}

	num -= _chanMissing[part];
	_chanMissing[part] = 0;

	for (int i = 0; i < _numChannels; i++) {
		if (!_channels[i]->isMappedToPart(part) || _channels[i]->isPlaying())
			continue;
		_channels[i]->setPart(0xFF);
		if (!--num)
			return;
	}

	do {
		uint16 oldest = 0;
		byte dropCh = 0;
		for (int i = 0; i < _numChannels; i++) {
			if (!_channels[i]->isMappedToPart(part))
				continue;

			uint16 ct = _channels[i]->getDuration();

			if (ct >= oldest) {
				dropCh = i;
				oldest = ct;
			}
		}

		_channels[dropCh]->chanOff();
		_channels[dropCh]->setPart(0xFF);
	} while (--num);
}

void MidiDriver_PCJr::assignFreeChannels(byte part) {
	uint8 freeChan = 0;
	for (int i = 0; i < _numChannels; i++) {
		if (_channels[i]->isMappedToPart(0xFF))
			freeChan++;
	}

	if (!freeChan)
		return;

	for (int i = 0; i < 16; i++) {
		if (!_chanMissing[i])
			continue;
		if (_chanMissing[i] < freeChan) {
			freeChan -= _chanMissing[i];
			addChannels(part, _chanMissing[i]);
			_chanMissing[i] = 0;
		} else {
			_chanMissing[i] -= freeChan;
			addChannels(part, _chanMissing[i]);
			return;
		}
	}
}

byte MidiDriver_PCJr::allocateChannel(byte part) {
	byte res = 0xFF;
	if (_version <= SCI_VERSION_0_LATE) {
		for (int i = 0; i < _numChannels; ++i) {
			if (_channels[i]->isMappedToPart(part))
				res = i;
		}
		return res;
	}

	uint16 oldest = 0;
	byte c = _chanMapping[part];

	for (bool loop = true; loop;) {
		c = (c + 1) % _numChannels;
		if (c == _chanMapping[part])
			loop = false;

		if (!_channels[c]->isMappedToPart(part))
			continue;

		if (!_channels[c]->isPlaying()) {
			_chanMapping[part] = c;
			return c;
		}

		uint16 ct = _channels[c]->getDuration();
		if (ct < oldest)
			continue;

		res = c;
		oldest = ct;		
	}

	if (oldest != 0) {
		_chanMapping[part] = res;
		_channels[res]->chanOff();
	}

	return res;
}

class MidiPlayer_PCJr : public MidiPlayer {
public:
	MidiPlayer_PCJr(SciVersion version, bool pcsMode) : MidiPlayer(version) { _driver = new MidiDriver_PCJr(g_system->getMixer(), version, pcsMode); }
	byte getPlayId() const override;
	int getPolyphony() const override { return 3; }
	bool hasRhythmChannel() const override { return false; }
	void setVolume(byte volume) override { _driver->property(MidiDriver_PCJr::kPropVolume, volume); }
	void initTrack(SciSpan<const byte> &trackData) override;
};

byte MidiPlayer_PCJr::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
		return 0x02;
	case SCI_VERSION_0_LATE:
		return 0x10;
	default:
		return 0x13;
	}
}

void MidiPlayer_PCJr::initTrack(SciSpan<const byte> &trackData) {
	if (_driver)
		static_cast<MidiDriver_PCJr*>(_driver)->initTrack(trackData);
}

MidiPlayer *MidiPlayer_PCJr_create(SciVersion version) {
	return new MidiPlayer_PCJr(version, false);
}

class MidiPlayer_PCSpeaker : public MidiPlayer_PCJr {
public:
	MidiPlayer_PCSpeaker(SciVersion version) : MidiPlayer_PCJr(version, true) { }
	byte getPlayId() const override;
	int getPolyphony() const override { return 1; }
};

byte MidiPlayer_PCSpeaker::getPlayId() const {
	switch (_version) {
	case SCI_VERSION_0_EARLY:
		return 0x04;
	case SCI_VERSION_0_LATE:
		return 0x20;
	default:
		return 0x12;
	}
}

MidiPlayer *MidiPlayer_PCSpeaker_create(SciVersion version) {
	return new MidiPlayer_PCSpeaker(version);
}

} // End of namespace Sci
