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

// TODO: Original Mac driver allows the interpreter to play notes. Channel
// is allocated via controller 0x4D. Find out if this is used. This would
// allow for music and sfx to be played simultaneously.

// FIXME: SQ3, LSL2 and HOYLE1 for Amiga don't seem to load any
// patches, even though patches are present. Later games do load
// patches, but include disabled patches with a 'd' appended to the
// filename, e.g. sound.010d. For SQ3, LSL2 and HOYLE1, we should
// probably disable patch loading. Maybe the original interpreter
// loads these disabled patches under some specific condition?

#include "sci/sound/drivers/mididriver.h"
#include "sci/sound/drivers/macmixer.h"
#include "sci/resource/resource.h"

#include "common/file.h"
#include "common/memstream.h"
#include "common/mutex.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "audio/mods/paula.h"

namespace Sci {

class MidiPlayer_AmigaMac0 : public MidiPlayer {
public:
	enum {
		kVoices = 4,
		kBaseFreq = 60
	};

	MidiPlayer_AmigaMac0(SciVersion version, Audio::Mixer *mixer, Common::Mutex &mutex);
	virtual ~MidiPlayer_AmigaMac0();

	// MidiPlayer
	void close() override;
	void send(uint32 b) override;
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;
	uint32 getBaseTempo() override { return (1000000 + kBaseFreq / 2) / kBaseFreq; }
	byte getPlayId() const override { return 0x40; }
	int getPolyphony() const override { return kVoices; }
	bool hasRhythmChannel() const override { return false; }
	void setVolume(byte volume) override;
	int getVolume() override;
	void playSwitch(bool play) override;
	void initTrack(SciSpan<const byte> &trackData) override;

protected:
	bool _playSwitch;
	uint _masterVolume;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _mixerSoundHandle;
	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	bool _isOpen;

	void freeInstruments();
	void onTimer();

	struct Instrument {
		Instrument() :
			name(),
			loop(false),
			fixedNote(false),
			seg2Offset(0),
			seg3Offset(0),
			samples(nullptr),
			transpose(0),
			envelope() {}

		~Instrument() { delete[] samples; }

		char name[31];
		bool loop;
		bool fixedNote;
		uint32 seg2Offset;
		uint32 seg3Offset;
		const byte *samples;
		int16 transpose;

		struct Envelope {
			byte skip;
			int8 step;
			byte target;
		} envelope[4];
	};

	Common::Array<const Instrument *> _instruments;

	class Voice {
	public:
		Voice(MidiPlayer_AmigaMac0 &driver, byte id) :
			_patch(0),
			_note(-1),
			_velocity(0),
			_pitch(0),
			_instrument(nullptr),
			_loop(false),
			_envState(0),
			_envCntDown(0),
			_envCurVel(0),
			_volume(0),
			_id(id),
			_driver(driver) {}

		virtual ~Voice() {}

		virtual void noteOn(int8 note, int8 velocity) = 0;
		virtual void noteOff(int8 note) = 0;
		virtual void setPitchWheel(int16 pitch) {}

		virtual void stop() = 0;
		virtual void setEnvelopeVolume(byte volume) = 0;

		void processEnvelope();

		byte _patch;
		int8 _note;
		byte _velocity;
		uint16 _pitch;

		const Instrument *_instrument;
		bool _loop;

		byte _envState;
		byte _envCntDown;
		int8 _envCurVel;

		byte _volume;
		byte _id;

	private:
		MidiPlayer_AmigaMac0 &_driver;
	};

	Common::Array<Voice *> _voices;
	typedef Common::Array<Voice *>::const_iterator VoiceIt;

	Voice *_channels[MIDI_CHANNELS];

	Common::Mutex &_mixMutex;
	Common::Mutex _timerMutex;
};

MidiPlayer_AmigaMac0::MidiPlayer_AmigaMac0(SciVersion version, Audio::Mixer *mixer, Common::Mutex &mutex) :
	MidiPlayer(version),
	_playSwitch(true),
	_masterVolume(15),
	_mixer(mixer),
	_mixerSoundHandle(),
	_timerProc(),
	_timerParam(nullptr),
	_isOpen(false),
	_channels(),
	_mixMutex(mutex) {}

MidiPlayer_AmigaMac0::~MidiPlayer_AmigaMac0() {
	close();
}

void MidiPlayer_AmigaMac0::close() {
	if (!_isOpen)
		return;

	_mixer->stopHandle(_mixerSoundHandle);

	for (uint ci = 0; ci < ARRAYSIZE(_channels); ++ci)
		_channels[ci] = nullptr;

	for (VoiceIt v = _voices.begin(); v != _voices.end(); ++v)
		delete *v;
	_voices.clear();

	freeInstruments();

	_isOpen = false;
}

void MidiPlayer_AmigaMac0::setVolume(byte volume) {
	Common::StackLock lock(_mixMutex);
	_masterVolume = CLIP<byte>(volume, 0, 15);
}

int MidiPlayer_AmigaMac0::getVolume() {
	Common::StackLock lock(_mixMutex);
	return _masterVolume;
}

void MidiPlayer_AmigaMac0::playSwitch(bool play) {
	Common::StackLock lock(_mixMutex);
	_playSwitch = play;
}

void MidiPlayer_AmigaMac0::initTrack(SciSpan<const byte>& header) {
	if (!_isOpen)
		return;

	uint8 readPos = 0;
	const uint8 caps = header.getInt8At(readPos++);

	// We only implement the MIDI functionality here, samples are
	// handled by the generic sample code
	if (caps != 0)
		return;

	Common::StackLock lock(_mixMutex);

	uint vi = 0;

	for (uint i = 0; i < 15; ++i) {
		readPos++;
		const uint8 flags = header.getInt8At(readPos++);

		if ((flags & getPlayId()) && (vi < kVoices))
			_channels[i] = _voices[vi++];
		else
			_channels[i] = nullptr;
	}

	_channels[15] = nullptr;

	for (VoiceIt it = _voices.begin(); it != _voices.end(); ++it) {
		Voice *voice = *it;
		voice->stop();
		voice->_note = -1;
		voice->_envState = 0;
		voice->_pitch = 0x2000;
	}
}

void MidiPlayer_AmigaMac0::freeInstruments() {
	for (Common::Array<const Instrument *>::iterator it = _instruments.begin(); it != _instruments.end(); ++it)
		delete *it;

	_instruments.clear();
}

void MidiPlayer_AmigaMac0::onTimer() {
	_mixMutex.unlock();
	_timerMutex.lock();

	if (_timerProc)
		(*_timerProc)(_timerParam);

	_timerMutex.unlock();
	_mixMutex.lock();

	for (VoiceIt it = _voices.begin(); it != _voices.end(); ++it)
		(*it)->processEnvelope();
}

void MidiPlayer_AmigaMac0::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	Common::StackLock lock(_timerMutex);
	_timerProc = timerProc;
	_timerParam = timerParam;
}

void MidiPlayer_AmigaMac0::send(uint32 b) {
	Common::StackLock lock(_mixMutex);

	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	Voice *voice = _channels[channel];

	if (!voice)
		return;

	switch(command) {
	case 0x80:
		voice->noteOff(op1);
		break;
	case 0x90:
		voice->noteOn(op1, op2);
		break;
	case 0xb0:
		// Not in original driver
		if (op1 == 0x7b && voice->_note != -1 && voice->_envState < 4)
			voice->noteOff(voice->_note);
		break;
	case 0xc0:
		voice->_patch = op1;
		break;
	case 0xe0:
		voice->setPitchWheel((op2 << 7) | op1);
		break;
	}
}

void MidiPlayer_AmigaMac0::Voice::processEnvelope() {
	if (_envState == 0 || _envState == 3)
		return;

	if (_envState == 6) {
		stop();
		_envState = 0;
		return;
	}

	if (_envCntDown == 0) {
		const uint envIdx = (_envState > 3 ? _envState - 2 : _envState - 1);

		_envCntDown = _instrument->envelope[envIdx].skip;
		int8 velocity = _envCurVel;

		if (velocity <= 0) {
			stop();
			_envState = 0;
			return;
		}

		if (velocity > 63)
			velocity = 63;

		if (!_driver._playSwitch)
			velocity = 0;

		setEnvelopeVolume(velocity);

		const int8 step = _instrument->envelope[envIdx].step;
		if (step < 0) {
			_envCurVel -= step;
			if (_envCurVel > _instrument->envelope[envIdx].target) {
				_envCurVel = _instrument->envelope[envIdx].target;
				++_envState;
			}
		} else {
			_envCurVel -= step;
			if (_envCurVel < _instrument->envelope[envIdx].target) {
				_envCurVel = _instrument->envelope[envIdx].target;
				++_envState;
			}
		}
	}

	--_envCntDown;
}

class MidiPlayer_Mac0 : public Mixer_Mac<MidiPlayer_Mac0>, public MidiPlayer_AmigaMac0 {
public:
	MidiPlayer_Mac0(SciVersion version, Audio::Mixer *mixer, Mode mode);

	// MidiPlayer
	int open(ResourceManager *resMan) override;
	void setVolume(byte volume) override;

	// MidiDriver
	void close() override;

	// Mixer_Mac
	static int8 applyChannelVolume(byte volume, byte sample);
	void interrupt() { onTimer(); }
	void onChannelFinished(uint channel);

private:
	enum {
		kStepTableSize = 84
	};

	template <Mode mode>
	void generateSamples(int16 *buf, int len);

	struct MacInstrument : public Instrument {
		MacInstrument() :
			Instrument(),
			endOffset(0) {}

		uint32 endOffset;
	};

	class MacVoice : public Voice {
	public:
		MacVoice(MidiPlayer_Mac0 &driver, byte id) :
			Voice(driver, id),
			_macDriver(driver) {}

	private:
		void noteOn(int8 note, int8 velocity) override;
		void noteOff(int8 note) override;

		void stop() override;
		void setEnvelopeVolume(byte volume) override;

		void calcVoiceStep();

		MidiPlayer_Mac0 &_macDriver;
	};

	bool loadInstruments(Common::SeekableReadStream &patch);

	ufrac_t _stepTable[kStepTableSize];
};

MidiPlayer_Mac0::MidiPlayer_Mac0(SciVersion version, Audio::Mixer *mixer, Mixer_Mac<MidiPlayer_Mac0>::Mode mode) :
	Mixer_Mac<MidiPlayer_Mac0>(mode),
	MidiPlayer_AmigaMac0(version, mixer, _mutex) {

	for (uint i = 0; i < kStepTableSize; ++i)
		_stepTable[i] = round(0x2000 * pow(2.0, i / 12.0));
}

int MidiPlayer_Mac0::open(ResourceManager *resMan) {
	if (_isOpen)
		return MidiDriver::MERR_ALREADY_OPEN;

	Resource *patch = g_sci->getResMan()->findResource(ResourceId(kResourceTypePatch, 200), false);
	if (!patch) {
		warning("MidiPlayer_Mac0: Failed to open patch 200");
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	}

	Common::MemoryReadStream stream(patch->toStream());
	if (!loadInstruments(stream)) {
		freeInstruments();
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	}

	for (byte vi = 0; vi < kVoices; ++vi)
		_voices.push_back(new MacVoice(*this, vi));

	startMixer();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	_isOpen = true;

	return 0;
}

void MidiPlayer_Mac0::setVolume(byte volume) {
	MidiPlayer_AmigaMac0::setVolume(volume);
	setMixerVolume(volume / 2 + 1);
}

void MidiPlayer_Mac0::close() {
	MidiPlayer_AmigaMac0::close();
	stopMixer();
}

int8 MidiPlayer_Mac0::applyChannelVolume(byte volume, byte sample) {
	int8 signedSample = sample - 0x80;

	if (volume == 0)
		return 0;

	if (volume == 63)
		return signedSample;

	if (signedSample >= 0)
		return (signedSample * volume + 32) / 64;
	else
		return ~((~signedSample * volume + 32) / 64);
}

void MidiPlayer_Mac0::onChannelFinished(uint channel) {
	_voices[channel]->_envState = 0;
}

void MidiPlayer_Mac0::MacVoice::stop() {
	_macDriver.resetChannel(_id);
}

void MidiPlayer_Mac0::MacVoice::calcVoiceStep() {
	int8 note = _note;

	if (_instrument->fixedNote)
		note = 72;

	int16 index = note + _instrument->transpose;

	index -= 24;

	while (index < 0)
		index += 12;

	while (index >= kStepTableSize)
		index -= 12;

	_macDriver.setChannelStep(_id, _macDriver._stepTable[index]);
}

void MidiPlayer_Mac0::MacVoice::setEnvelopeVolume(byte volume) {
	if (_macDriver._masterVolume == 0 || !_macDriver._playSwitch)
		volume = 0;
	_macDriver.setChannelVolume(_id, volume * _volume >> 6);
}

void MidiPlayer_Mac0::MacVoice::noteOn(int8 note, int8 velocity) {
	if (velocity == 0) {
		noteOff(note);
		return;
	}

	stop();
	_envState = 0;

	if (!_macDriver._instruments[_patch]) // Not in original driver
		return;

	_instrument = _macDriver._instruments[_patch];

	_velocity = velocity;
	_volume = velocity >> 1;
	_envCurVel = 64;
	_envCntDown = 0;
	_loop = _instrument->loop;
	_note = note;

	calcVoiceStep();

	const MacInstrument *ins = static_cast<const MacInstrument *>(_instrument);

	if (_loop) {
		_envState = 1;
		_macDriver.setChannelData(_id, ins->samples, 0, ins->seg3Offset, ins->seg3Offset - ins->seg2Offset);
	} else {
		_macDriver.setChannelData(_id, ins->samples, 0, ins->endOffset);
	}

	setEnvelopeVolume(63);
}

void MidiPlayer_Mac0::MacVoice::noteOff(int8 note) {
	if (_note == note) {
		if (_envState != 0) {
			_envState = 4;
			_envCntDown = 0;
		}
		// Original driver doesn't reset note anywhere that I could find,
		// but this seems like a good place to do that
		_note = -1;
	}
}

bool MidiPlayer_Mac0::loadInstruments(Common::SeekableReadStream &patch) {
	char name[33];

	if (patch.read(name, 8) < 8 || strncmp(name, "X1iUo123", 8) != 0) {
		warning("MidiPlayer_Mac0: Incorrect ID string in patch bank");
		return false;
	}

	if (patch.read(name, 32) < 32) {
		warning("MidiPlayer_Mac0: Error reading patch bank");
		return false;
	}
	name[32] = 0;

	debugC(kDebugLevelSound, "Bank: '%s'", name);

	_instruments.resize(128);

	for (byte i = 0; i < 128; i++) {
		patch.seek(40 + i * 4);
		uint32 offset = patch.readUint32BE();

		if (offset == 0) {
			_instruments[i] = 0;
			continue;
		}

		patch.seek(offset);

		MacInstrument *instrument = new MacInstrument();
		_instruments[i] = instrument;

		patch.readUint16BE(); // index

		const uint16 flags = patch.readUint16BE();
		instrument->loop = flags & 1;
		instrument->fixedNote = !(flags & 2);

		instrument->seg2Offset = patch.readUint32BE();
		instrument->seg3Offset = patch.readUint32BE();
		instrument->endOffset = patch.readUint32BE();

		instrument->transpose = patch.readUint16BE();

		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			instrument->envelope[stage].skip = patch.readByte();

		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			instrument->envelope[stage].step = patch.readByte();

		// In the original, it uses the stage 0 step as the stage 3 target,
		// but we (most likely) don't have to replicate this bug.
		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			instrument->envelope[stage].target = patch.readByte();

		patch.read(instrument->name, 30);
		instrument->name[30] = 0;

		debugC(kDebugLevelSound, "\tInstrument[%d]: '%s'", i, instrument->name);
		debugC(kDebugLevelSound, "\t\tSegment offsets: %d, %d, %d", instrument->seg2Offset, instrument->seg3Offset, instrument->endOffset);
		debugC(kDebugLevelSound, "\t\tTranspose = %d, Fixed note = %d, Loop = %d", instrument->transpose, instrument->fixedNote, instrument->loop);
		debugC(kDebugLevelSound, "\t\tEnvelope:");
		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			debugC(kDebugLevelSound, "\t\t\tStage %d: skip %d, step %d, target %d", stage, instrument->envelope[stage].skip, instrument->envelope[stage].step, instrument->envelope[stage].target);

		uint32 sampleSize = (instrument->loop ? instrument->seg3Offset : instrument->endOffset) + 1111;
		byte *samples = new byte[sampleSize];
		patch.read(samples, sampleSize);
		instrument->samples = samples;
	}

	return true;
}

class MidiPlayer_Amiga0 : public Audio::Paula, public MidiPlayer_AmigaMac0 {
public:
	MidiPlayer_Amiga0(SciVersion version, Audio::Mixer *mixer);

	// MidiPlayer
	int open(ResourceManager *resMan) override;

	// MidiDriver
	void close() override;

	// Audio::Paula
	void interrupt() override { onTimer(); }

private:
	struct AmigaInstrument : public Instrument {
		AmigaInstrument() :
			Instrument(),
			seg1Size(0),
			seg2Size(0),
			seg3Size(0) {}

		int16 seg1Size;
		int16 seg2Size;
		int16 seg3Size;
	};

	class AmigaVoice : public Voice {
	public:
		AmigaVoice(MidiPlayer_Amiga0 &driver, uint id) :
			Voice(driver, id),
			_amigaDriver(driver) {}

	private:
		void noteOn(int8 note, int8 velocity) override;
		void noteOff(int8 note) override;
		void setPitchWheel(int16 pitch) override;

		void stop() override;
		void setEnvelopeVolume(byte volume) override;

		void calcVoiceStep();

		MidiPlayer_Amiga0 &_amigaDriver;
	};

	uint _defaultInstrument;
	bool _isEarlyDriver;

	bool loadInstruments(Common::SeekableReadStream &patch);

	uint16 _periodTable[333];
};

MidiPlayer_Amiga0::MidiPlayer_Amiga0(SciVersion version, Audio::Mixer *mixer) :
	Audio::Paula(true, mixer->getOutputRate(), mixer->getOutputRate() / kBaseFreq),
	MidiPlayer_AmigaMac0(version, mixer, _mutex),
	_defaultInstrument(0),
	_isEarlyDriver(false) {

	// These values are close, but not identical to the original
	for (int i = 0; i < ARRAYSIZE(_periodTable); ++i)
		_periodTable[i] = 3579545 / 20000.0 / pow(2.0, (i - 308) / 48.0);
}

void MidiPlayer_Amiga0::AmigaVoice::setEnvelopeVolume(byte volume) {
	// Early games ignore note velocity for envelope-enabled notes
	if (_amigaDriver._isEarlyDriver)
		_amigaDriver.setChannelVolume(_id, volume * _amigaDriver._masterVolume >> 4);
	else
		_amigaDriver.setChannelVolume(_id, (volume * _amigaDriver._masterVolume >> 4) * _volume >> 6);
}

int MidiPlayer_Amiga0::open(ResourceManager *resMan) {
	if (_isOpen)
		return MidiDriver::MERR_ALREADY_OPEN;

	_isEarlyDriver = g_sci->getGameId() == GID_LSL2 || g_sci->getGameId() == GID_SQ3;

	Common::File file;

	if (!file.open("bank.001")) {
		warning("MidiPlayer_Amiga0: Failed to open bank.001");
		return false;
	}

	if (!loadInstruments(file)) {
		freeInstruments();
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	}

	for (byte vi = 0; vi < NUM_VOICES; ++vi)
		_voices.push_back(new AmigaVoice(*this, vi));

	startPaula();
	// Enable reverse stereo to counteract Audio::Paula's reverse stereo
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO, false, true);
	_isOpen = true;

	return 0;
}

void MidiPlayer_Amiga0::close() {
	MidiPlayer_AmigaMac0::close();
	clearVoices();
	stopPaula();
}

void MidiPlayer_Amiga0::AmigaVoice::stop() {
	_amigaDriver.clearVoice(_id);
}

void MidiPlayer_Amiga0::AmigaVoice::calcVoiceStep() {
	int8 note = _note;

	if (_instrument->fixedNote)
		note = 101;

	int16 index = (note + _instrument->transpose) * 4;

	if (_pitch >= 0x2000)
		index += (_pitch - 0x2000) / 171;
	else
		index -= (0x2000 - _pitch) / 171;

	// For very high notes, the original driver reads out of bounds
	// (see e.g. SQ3 intro sequence). We compute the period for
	// these notes. The original hardware would not be able to
	// handle these very low periods, but Audio::Paula doesn't
	// seem to mind.

	while (index < 96)
		index += 48;

	index -= 96;

	while (index >= ARRAYSIZE(_amigaDriver._periodTable))
		index -= 48;

	_amigaDriver.setChannelPeriod(_id, _amigaDriver._periodTable[index]);
}

void MidiPlayer_Amiga0::AmigaVoice::noteOn(int8 note, int8 velocity) {
	if (velocity == 0) {
		noteOff(note);
		return;
	}

	_instrument = _amigaDriver._instruments[_patch];

	// Default to the first instrument in the bank
	if (!_instrument)
		_instrument = _amigaDriver._instruments[_amigaDriver._defaultInstrument];

	_velocity = velocity;
	_volume = velocity >> 1;
	_loop = _instrument->loop;
	_note = note;

	stop();
	_envState = 0;

	calcVoiceStep();

	const AmigaInstrument *ins = static_cast<const AmigaInstrument *>(_instrument);

	const int8 *seg1 = (const int8 *)ins->samples;
	const int8 *seg2 = seg1;
	int16 seg1Size = ins->seg1Size; 
	seg2 += ins->seg2Offset & 0xfffe;
	int16 seg2Size = ins->seg2Size;

	if (!_loop) {
		seg1Size = seg1Size + seg2Size + ins->seg3Size;
		seg2 = nullptr;
		seg2Size = 0;
	}

	if (ins->envelope[0].skip != 0 && _loop) {
		_envCurVel = _volume;
		_envCntDown = 0;
		_envState = 1;
	}

	_amigaDriver.setChannelData(_id, seg1, seg2, seg1Size * 2, seg2Size * 2);
	if (_amigaDriver._playSwitch)
		_amigaDriver.setChannelVolume(_id, _amigaDriver._masterVolume * _volume >> 4);
}

void MidiPlayer_Amiga0::AmigaVoice::noteOff(int8 note) {
	if (_note == note) {
		if (_envState != 0) {
			_envCurVel = _instrument->envelope[1].target;
			_envState = 4;
		}
		// Original driver doesn't reset note anywhere that I could find,
		// but this seems like a good place to do that
		_note = -1;
	}
}

void MidiPlayer_Amiga0::AmigaVoice::setPitchWheel(int16 pitch) {
	if (_amigaDriver._isEarlyDriver)
		return;

	_pitch = pitch;

	if (_note != -1)
		calcVoiceStep();
}

bool MidiPlayer_Amiga0::loadInstruments(Common::SeekableReadStream &patch) {
	char name[31];

	if (patch.read(name, 8) < 8 || strncmp(name, "X0iUo123", 8) != 0) {
		warning("MidiPlayer_Amiga0: Incorrect ID string in patch bank");
		return false;
	}

	if (patch.read(name, 30) < 30) {
		warning("MidiPlayer_Amiga0: Error reading patch bank");
		return false;
	}
	name[30] = 0;

	debugC(kDebugLevelSound, "Bank: '%s'", name);

	_instruments.resize(128);

	const uint16 instrumentCount = patch.readUint16BE();

	if (instrumentCount == 0) {
		warning("MidiPlayer_Amiga0: No instruments found in patch bank");
		return false;
	}

	for (uint i = 0; i < instrumentCount; ++i) {
		AmigaInstrument *instrument = new AmigaInstrument();

		const uint16 patchIdx = patch.readUint16BE();
		_instruments[patchIdx] = instrument;

		if (i == 0)
			_defaultInstrument = patchIdx;

		patch.read(instrument->name, 30);
		instrument->name[30] = 0;
		const uint16 flags = patch.readUint16BE();
		instrument->loop = flags & 1;
		instrument->fixedNote = !(flags & 2);
		instrument->transpose = patch.readSByte();
		instrument->seg1Size = patch.readSint16BE();
		instrument->seg2Offset = patch.readUint32BE();
		instrument->seg2Size = patch.readSint16BE();
		instrument->seg3Offset = patch.readUint32BE();
		instrument->seg3Size = patch.readSint16BE();

		// There's some envelope-related bugs here in the original, these were not replicated
		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			instrument->envelope[stage].skip = patch.readByte();

		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			instrument->envelope[stage].step = patch.readByte();

		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			instrument->envelope[stage].target = patch.readByte();

		int32 sampleSize = instrument->seg1Size + instrument->seg2Size + instrument->seg3Size;
		sampleSize <<= 1;
		byte *samples = new byte[sampleSize];
		patch.read(samples, sampleSize);
		instrument->samples = samples;

		if (patch.eos() || patch.err()) {
			warning("MidiPlayer_Amiga0: Error reading patch bank");
			return false;
		}

		debugC(kDebugLevelSound, "\tInstrument[%d]: '%s'", patchIdx, instrument->name);
		debugC(kDebugLevelSound, "\t\tSegment 1: offset 0, size %d", instrument->seg1Size * 2);
		debugC(kDebugLevelSound, "\t\tSegment 2: offset %d, size %d", instrument->seg2Offset, instrument->seg2Size * 2);
		debugC(kDebugLevelSound, "\t\tSegment 3: offset %d, size %d", instrument->seg3Offset, instrument->seg3Size * 2);
		debugC(kDebugLevelSound, "\t\tTranspose = %d, Fixed note = %d, Loop = %d", instrument->transpose, instrument->fixedNote, instrument->loop);
		debugC(kDebugLevelSound, "\t\tEnvelope:");
		for (uint stage = 0; stage < ARRAYSIZE(instrument->envelope); ++stage)
			debugC(kDebugLevelSound, "\t\t\tStage %d: skip %d, step %d, target %d", stage, instrument->envelope[stage].skip, instrument->envelope[stage].step, instrument->envelope[stage].target);
	}

	return true;
}

MidiPlayer *MidiPlayer_AmigaMac0_create(SciVersion version, Common::Platform platform) {
	if (platform == Common::kPlatformMacintosh)
		return new MidiPlayer_Mac0(version, g_system->getMixer(), MidiPlayer_Mac0::kModeHq);
	else
		return new MidiPlayer_Amiga0(version, g_system->getMixer());
}

} // End of namespace Sci
