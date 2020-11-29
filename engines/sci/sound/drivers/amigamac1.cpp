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
#include "sci/sound/drivers/macmixer.h"
#include "sci/resource/resource.h"

#include "audio/mixer.h"
#include "audio/mods/paula.h"
#include "common/array.h"
#include "common/debug-channels.h"
#include "common/hashmap.h"
#include "common/memstream.h"
#include "common/mutex.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Sci {

class MidiPlayer_AmigaMac1 : public MidiPlayer {
public:
	enum {
		kVoices = 4,
		kFreqTableSize = 56,
		kBaseFreq = 60
	};

	enum kEnvState {
		kEnvStateAttack,
		kEnvStateDecay,
		kEnvStateSustain,
		kEnvStateRelease
	};

	MidiPlayer_AmigaMac1(SciVersion version, Audio::Mixer *mixer, uint extraSamples, bool wantSignedSamples, Common::Mutex &mutex);
	virtual ~MidiPlayer_AmigaMac1();

	// MidiPlayer
	void close() override;
	void send(uint32 b) override;
	void setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) override;
	uint32 getBaseTempo() override { return (1000000 + kBaseFreq / 2) / kBaseFreq; }
	byte getPlayId() const override { return 0x06; }
	int getPolyphony() const override { return kVoices; }
	bool hasRhythmChannel() const override { return false; }
	void setVolume(byte volume) override;
	int getVolume() override;
	void playSwitch(bool play) override;

protected:
	struct Wave {
		Wave() : name(), phase1Start(0), phase1End(0), phase2Start(0), phase2End(0),
				 nativeNote(0), freqTable(nullptr), samples(nullptr), size(0) {}

		char name[9];
		uint16 phase1Start, phase1End;
		uint16 phase2Start, phase2End;
		uint16 nativeNote;
	
		// This table contains frequency data for about one octave with 3 pitch bend positions between semitones
		// On Mac, this table contains a fixed-point source-samples-per-output-sample value
		// On Amiga, this table contains how many clock cycles each source sample should be output
		const uint32 *freqTable;
		const byte *samples;
		uint32 size;
	};

	struct NoteRange {
		NoteRange() : startNote(0), endNote(0), wave(nullptr), transpose(0), attackSpeed(0),
					  attackTarget(0), decaySpeed(0), decayTarget(0), releaseSpeed(0), fixedNote(0),
					  loop(false) {}

		int16 startNote;
		int16 endNote;

		const Wave *wave;

		int16 transpose;
	
		byte attackSpeed;
		byte attackTarget;
		byte decaySpeed;
		byte decayTarget;
		byte releaseSpeed;

		int16 fixedNote;
		bool loop;
	};

	struct Instrument {
		Instrument() : name() {}

		char name[9];
		Common::Array<NoteRange> noteRange;
	};

	Common::Array<const Instrument *> _instruments;
	typedef Common::HashMap<uint32, const Wave *> WaveMap;
	WaveMap _waves;
	typedef Common::HashMap<uint32, const uint32 *> FreqTableMap;
	FreqTableMap _freqTables;

	bool _playSwitch;
	uint _masterVolume;

	Audio::Mixer *_mixer;
	Audio::SoundHandle _mixerSoundHandle;
	Common::TimerManager::TimerProc _timerProc;
	void *_timerParam;
	bool _isOpen;

	uint32 *loadFreqTable(Common::SeekableReadStream &stream);
	const Wave *loadWave(Common::SeekableReadStream &stream, bool isEarlyPatch);
	bool loadInstruments(Common::SeekableReadStream &patch, bool isEarlyPatch);
	void freeInstruments();
	void distributeVoices();
	void onTimer();

	class Channel;
	class Voice {
	public:
		Voice(MidiPlayer_AmigaMac1 &driver, byte id) :
			_channel(nullptr),
			_note(-1),
			_velocity(0),
			_isReleased(false),
			_isSustained(false),
			_ticks(0),
			_releaseTicks(0),
			_envState(kEnvStateAttack),
			_envCurVel(0),
			_envCntDown(0),
			_noteRange(nullptr),
			_wave(nullptr),
			_freqTable(nullptr),
			_id(id),
			_driver(driver) {}

		virtual ~Voice() {}

		void noteOn(int8 note, int8 velocity);
		void noteOff();

		virtual void play(int8 note, int8 velocity) = 0;
		virtual void stop() = 0;
		virtual void setVolume(byte volume) = 0;
		virtual bool calcVoiceStep() = 0;

		void calcMixVelocity();
		void processEnvelope();

		Channel *_channel;
		int8 _note;
		byte _velocity;
		bool _isReleased;
		bool _isSustained;
		uint16 _ticks;
		uint16 _releaseTicks;

		kEnvState _envState;
		int8 _envCurVel;
		byte _envCntDown;

		const NoteRange *_noteRange;
		const Wave *_wave;
		const uint32 *_freqTable;
		const byte _id;

	private:
		MidiPlayer_AmigaMac1 &_driver;
	};

	Common::Array<Voice *> _voices;
	typedef Common::Array<Voice *>::const_iterator VoiceIt;

	class Channel {
	public:
		Channel(MidiPlayer_AmigaMac1 &driver) :
			_patch(0),
			_pitch(0x2000),
			_hold(false),
			_pan(64),
			_volume(63),
			_lastVoiceIt(driver._voices.begin()),
			_extraVoices(0),
			_driver(driver) {}

		void noteOn(int8 note, int8 velocity);
		void noteOff(int8 note);

		Voice *findVoice();
		void voiceMapping(byte voices);
		void assignVoices(byte voices);
		void releaseVoices(byte voices);
		void changePatch(int8 patch);
		void holdPedal(int8 pedal);
		void setPitchWheel(uint16 pitch);

		int8 _patch;
		uint16 _pitch;
		bool _hold;
		int8 _pan;
		int8 _volume;
		VoiceIt _lastVoiceIt;
		byte _extraVoices;

	private:
		MidiPlayer_AmigaMac1 &_driver;
	};

	Common::Array<Channel *> _channels;
	typedef Common::Array<Channel *>::const_iterator ChanIt;

	static const byte _envSpeedToStep[32];
	static const byte _envSpeedToSkip[32];
	static const byte _velocityMap[64];

	const uint _extraSamples;
	const bool _wantSignedSamples;

	Common::Mutex &_mixMutex;
	Common::Mutex _timerMutex;
};

MidiPlayer_AmigaMac1::MidiPlayer_AmigaMac1(SciVersion version, Audio::Mixer *mixer, uint extraSamples, bool wantSignedSamples, Common::Mutex &mutex) :
	MidiPlayer(version),
	_playSwitch(true),
	_masterVolume(15),
	_mixer(mixer),
	_mixerSoundHandle(),
	_timerProc(),
	_timerParam(nullptr),
	_isOpen(false),
	_extraSamples(extraSamples),
	_wantSignedSamples(wantSignedSamples),
	_mixMutex(mutex) {

	assert(_extraSamples > 0);
}

MidiPlayer_AmigaMac1::~MidiPlayer_AmigaMac1() {
	close();
}

void MidiPlayer_AmigaMac1::close() {
	if (!_isOpen)
		return;

	_mixer->stopHandle(_mixerSoundHandle);

	for (ChanIt c = _channels.begin(); c != _channels.end(); ++c)
		delete *c;
	_channels.clear();

	for (VoiceIt v = _voices.begin(); v != _voices.end(); ++v)
		delete *v;
	_voices.clear();

	freeInstruments();

	_isOpen = false;
}

void MidiPlayer_AmigaMac1::setTimerCallback(void *timer_param, Common::TimerManager::TimerProc timer_proc) {
	Common::StackLock lock(_timerMutex);
	_timerProc = timer_proc;
	_timerParam = timer_param;
}

void MidiPlayer_AmigaMac1::setVolume(byte volume) {
	Common::StackLock lock(_mixMutex);
	_masterVolume = volume;
}

int MidiPlayer_AmigaMac1::getVolume() {
	Common::StackLock lock(_mixMutex);
	return _masterVolume;
}

void MidiPlayer_AmigaMac1::playSwitch(bool play) {
	Common::StackLock lock(_mixMutex);
	_playSwitch = play;
}

uint32 *MidiPlayer_AmigaMac1::loadFreqTable(Common::SeekableReadStream &stream) {
	uint32 *freqTable = new ufrac_t[kFreqTableSize];

	for (uint i = 0; i < kFreqTableSize; ++i)
		freqTable[i] = stream.readUint32BE();

	return freqTable;
}

const MidiPlayer_AmigaMac1::Wave *MidiPlayer_AmigaMac1::loadWave(Common::SeekableReadStream &stream, bool isEarlyPatch) {
	Wave *wave = new Wave();

	stream.read(wave->name, 8);
	wave->name[8] = 0;

	bool isSigned = true;
	if (!isEarlyPatch)
		isSigned = stream.readUint16BE();

	wave->phase1Start = stream.readUint16BE();
	wave->phase1End = stream.readUint16BE();
	wave->phase2Start = stream.readUint16BE();
	wave->phase2End = stream.readUint16BE();
	wave->nativeNote = stream.readUint16BE();
	const uint32 freqTableOffset = stream.readUint32BE();

	// Sanity checks of segment offsets
	if ((wave->phase2End & ~1) > wave->phase1End || wave->phase1Start > wave->phase1End || wave->phase2Start > wave->phase2End)
		error("MidiPlayer_AmigaMac1: Invalid segment offsets found for wave '%s'", wave->name);

	// On Mac, 1480 additional samples are present, rounded up to the next word boundary
	// This allows for a maximum step of 8 during sample generation without bounds checking
	// On Amiga, 224 additional samples are present
	wave->size = ((wave->phase1End + 1) + _extraSamples + 1) & ~1;
	byte *samples = new byte[wave->size];
	stream.read(samples, wave->size);
	wave->samples = samples;

	if (_wantSignedSamples && !isSigned) {
		// The original code uses a signed 16-bit type here, while some samples
		// exceed INT_MAX in size. In this case, it will change one "random" byte
		// in memory and then stop converting. We simulate this behaviour here, minus
		// the memory corruption.
		// The "maincrnh" instrument in Castle of Dr. Brain has an incorrect signedness
		// flag, but is not actually converted because of its size.

		if (wave->phase1End + _extraSamples <= 0x8000) {
			for (uint32 i = 0; i < wave->size; ++i)
				samples[i] -= 0x80;
		} else {
			debugC(kDebugLevelSound, "MidiPlayer_AmigaMac1: Skipping sign conversion for wave '%s' of size %d bytes", wave->name, wave->size);
		}
	}

	if (!_freqTables.contains(freqTableOffset)) {
		stream.seek(freqTableOffset);
		_freqTables[freqTableOffset] = loadFreqTable(stream);
	}

	wave->freqTable = _freqTables[freqTableOffset];
	return wave;
}

bool MidiPlayer_AmigaMac1::loadInstruments(Common::SeekableReadStream &patch, bool isEarlyPatch) {
	_instruments.resize(128);

	for (uint patchIdx = 0; patchIdx < 128; ++patchIdx) {
		patch.seek(patchIdx * 4);
		uint32 offset = patch.readUint32BE();

		if (offset == 0)
			continue;

		Instrument *instrument = new Instrument();

		patch.seek(offset);
		patch.read(instrument->name, 8);
		instrument->name[8] = 0;
		patch.skip(2); // Unknown

		debugC(kDebugLevelSound, "Instrument[%d]: '%s'", patchIdx, instrument->name);

		while (1) {
			NoteRange noteRange;

			noteRange.startNote = patch.readUint16BE();

			if (patch.err() || patch.eos())
				return false;

			if (noteRange.startNote == -1)
				break;

			noteRange.endNote = patch.readUint16BE();

			const uint32 waveOffset = patch.readUint32BE();

			noteRange.transpose = patch.readSint16BE();

			noteRange.attackSpeed = patch.readByte();
			noteRange.attackTarget = patch.readByte();
			noteRange.decaySpeed = patch.readByte();
			noteRange.decayTarget = patch.readByte();
			noteRange.releaseSpeed = patch.readByte();

			patch.skip(1); // Probably releaseTarget, unused
			noteRange.fixedNote = patch.readSint16BE();
			noteRange.loop = !patch.readUint16BE();

			int32 nextNoteRangePos = patch.pos();

			if (!_waves.contains(waveOffset)) {
				patch.seek(waveOffset);
				_waves[waveOffset] = loadWave(patch, isEarlyPatch);
			}

			noteRange.wave = _waves[waveOffset];

			debugC(kDebugLevelSound, "\tNotes %d-%d", noteRange.startNote, noteRange.endNote);
			debugC(kDebugLevelSound, "\t\tWave: '%s'", noteRange.wave->name);
			debugC(kDebugLevelSound, "\t\t\tSegment 1: %d-%d", noteRange.wave->phase1Start, noteRange.wave->phase1End);
			debugC(kDebugLevelSound, "\t\t\tSegment 2: %d-%d", noteRange.wave->phase2Start, noteRange.wave->phase2End);
			debugC(kDebugLevelSound, "\t\tTranspose = %d, Fixed note = %d, Loop = %d", noteRange.transpose, noteRange.fixedNote, noteRange.loop);
			debugC(kDebugLevelSound, "\t\tAttack: %d delta, %d target", noteRange.attackSpeed, noteRange.attackTarget);
			debugC(kDebugLevelSound, "\t\tDecay: %d delta, %d target", noteRange.decaySpeed, noteRange.decayTarget);
			debugC(kDebugLevelSound, "\t\tRelease: %d delta, %d target", noteRange.releaseSpeed, 0);
			debugC(kDebugLevelSound, "\t\tRelease: %d delta, %d target", noteRange.releaseSpeed, 0);

			instrument->noteRange.push_back(noteRange);

			_instruments[patchIdx] = instrument;
			patch.seek(nextNoteRangePos);
		}
	}

	return true;
}

void MidiPlayer_AmigaMac1::freeInstruments() {
	for (WaveMap::iterator it = _waves.begin(); it != _waves.end(); ++it)
		delete it->_value;
	_waves.clear();

	for (FreqTableMap::iterator it = _freqTables.begin(); it != _freqTables.end(); ++it)
		delete[] it->_value;
	_freqTables.clear();

	for (Common::Array<const Instrument *>::iterator it = _instruments.begin(); it != _instruments.end(); ++it)
		delete *it;
	_instruments.clear();
}

void MidiPlayer_AmigaMac1::onTimer() {
	_mixMutex.unlock();
	_timerMutex.lock();

	if (_timerProc)
		(*_timerProc)(_timerParam);

	_timerMutex.unlock();
	_mixMutex.lock();

	for (VoiceIt it = _voices.begin(); it != _voices.end(); ++it) {
		Voice *v = *it;
		if (v->_note != -1) {
			++v->_ticks;
			if (v->_isReleased)
				++v->_releaseTicks;
			v->processEnvelope();
			v->calcMixVelocity();
		}
	}
}

MidiPlayer_AmigaMac1::Voice *MidiPlayer_AmigaMac1::Channel::findVoice() {
	assert(_lastVoiceIt != _driver._voices.end());

	VoiceIt voiceIt = _lastVoiceIt;
	uint16 maxTicks = 0;
	VoiceIt maxTicksVoiceIt = _driver._voices.end();

	do {
		++voiceIt;

		if (voiceIt == _driver._voices.end())
			voiceIt = _driver._voices.begin();

		Voice *v = *voiceIt;

		if (v->_channel == this) {
			if (v->_note == -1) {
				_lastVoiceIt = voiceIt;
				return v;
			}

			uint16 ticks;

			if (v->_releaseTicks != 0)
				ticks = v->_releaseTicks + 0x8000;
			else
				ticks = v->_ticks;

			if (ticks >= maxTicks) {
				maxTicks = ticks;
				maxTicksVoiceIt = voiceIt;
			}
		}
	} while (voiceIt != _lastVoiceIt);

	if (maxTicksVoiceIt != _driver._voices.end()) {
		(*maxTicksVoiceIt)->noteOff();
		_lastVoiceIt = maxTicksVoiceIt;
		return *maxTicksVoiceIt;
	}

	return nullptr;
}

void MidiPlayer_AmigaMac1::Channel::voiceMapping(byte voices) {
	int curVoices = 0;

	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it)
		if ((*it)->_channel == this)
			curVoices++;

	curVoices += _extraVoices;

	if (curVoices < voices)
		assignVoices(voices - curVoices);
	else if (curVoices > voices) {
		releaseVoices(curVoices - voices);
		_driver.distributeVoices();
	}
}

void MidiPlayer_AmigaMac1::Channel::assignVoices(byte voices) {
	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
		Voice *v = *it;

		if (!v->_channel) {
			v->_channel = this;

			if (v->_note != -1)
				v->noteOff();

			if (--voices == 0)
				break;
		}
	}

	_extraVoices += voices;
}

void MidiPlayer_AmigaMac1::Channel::releaseVoices(byte voices) {
	if (_extraVoices >= voices) {
		_extraVoices -= voices;
		return;
	}

	voices -= _extraVoices;
	_extraVoices = 0;

	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
		Voice *v = *it;

		if ((v->_channel == this) && (v->_note == -1)) {
			v->_channel = nullptr;
			if (--voices == 0)
				return;
		}
	}

	do {
		uint16 maxTicks = 0;
		Voice *maxTicksVoice = _driver._voices[0];

		for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
			Voice *v = *it;

			if (v->_channel == this) {
				// The original code seems to be broken here. It reads a word value from
				// byte array _voiceSustained.
				uint16 ticks = v->_releaseTicks;
				if (ticks > 0)
					ticks += 0x8000;
				else
					ticks = v->_ticks;

				if (ticks >= maxTicks) {
					maxTicks = ticks;
					maxTicksVoice = v;
				}
			}
		}
		maxTicksVoice->_isSustained = false;
		maxTicksVoice->noteOff();
		maxTicksVoice->_channel = nullptr;
	} while (--voices > 0);
}

void MidiPlayer_AmigaMac1::distributeVoices() {
	int freeVoices = 0;

	for (VoiceIt it = _voices.begin(); it != _voices.end(); ++it)
		if (!(*it)->_channel)
			freeVoices++;

	if (freeVoices == 0)
		return;

	for (ChanIt it = _channels.begin(); it != _channels.end(); ++it) {
		Channel *channel = *it;

		if (channel->_extraVoices != 0) {
			if (channel->_extraVoices >= freeVoices) {
				channel->_extraVoices -= freeVoices;
				channel->assignVoices(freeVoices);
				return;
			} else {
				freeVoices -= channel->_extraVoices;
				const byte extraVoices = channel->_extraVoices;
				channel->_extraVoices = 0;
				channel->assignVoices(extraVoices);
			}
		}
	}
}

void MidiPlayer_AmigaMac1::Voice::noteOn(int8 note, int8 velocity) {
	_isReleased = false;
	_envCurVel = 0;
	_envState = kEnvStateAttack;
	_envCntDown = 0;
	_ticks = 0;
	_releaseTicks = 0;

	const int8 patchId = _channel->_patch;

	// Check for valid patch
	if (patchId < 0 || (uint)patchId >= _driver._instruments.size() || !_driver._instruments[patchId])
		return;

	const Instrument *ins = _driver._instruments[patchId];

	// Each patch links to one or more waves, where each wave is assigned to a range of notes.
	Common::Array<NoteRange>::const_iterator noteRange;
	for (noteRange = ins->noteRange.begin(); noteRange != ins->noteRange.end(); ++noteRange) {
		if (noteRange->startNote <= note && note <= noteRange->endNote)
			break;
	}

	// Abort if this note has no wave assigned to it
	if (noteRange == ins->noteRange.end())
		return;

	const Wave *wave = noteRange->wave;
	const uint32 *freqTable = wave->freqTable;

	_noteRange = noteRange;
	_wave = wave;
	_freqTable = freqTable;

	play(note, velocity);
}

void MidiPlayer_AmigaMac1::Voice::noteOff() {
	stop();
	_velocity = 0;
	_note = -1;
	_isSustained = false;
	_isReleased = false;
	_envState = kEnvStateAttack;
	_envCntDown = 0;
	_ticks = 0;
	_releaseTicks = 0;
}

void MidiPlayer_AmigaMac1::Voice::processEnvelope() {
	if (!_noteRange->loop) {
		_envCurVel = _noteRange->attackTarget;
		return;
	}

	if (_isReleased)
		_envState = kEnvStateRelease;

	switch(_envState) {
	case kEnvStateAttack: {
		if (_envCntDown != 0) {
			--_envCntDown;
			return;
		}
		_envCntDown = _envSpeedToSkip[_noteRange->attackSpeed];
		_envCurVel += _envSpeedToStep[_noteRange->attackSpeed];
		if (_envCurVel >= _noteRange->attackTarget) {
			_envCurVel = _noteRange->attackTarget;
			_envState = kEnvStateDecay;
		}
		break;
	}
	case kEnvStateDecay: {
		if (_envCntDown != 0) {
			--_envCntDown;
			return;
		}
		_envCntDown = _envSpeedToSkip[_noteRange->decaySpeed];
		_envCurVel -= _envSpeedToStep[_noteRange->decaySpeed];
		if (_envCurVel <= _noteRange->decayTarget) {
			_envCurVel = _noteRange->decayTarget;
			_envState = kEnvStateSustain;
		}
		break;
	}
	case kEnvStateSustain:
		_envCurVel = _noteRange->decayTarget;
		break;
	case kEnvStateRelease: {
		if (_envCntDown != 0) {
			--_envCntDown;
			return;
		}
		_envCntDown = _envSpeedToSkip[_noteRange->releaseSpeed];
		_envCurVel -= _envSpeedToStep[_noteRange->releaseSpeed];
		if (_envCurVel <= 0)
			noteOff();
	}
	}
}

void MidiPlayer_AmigaMac1::Voice::calcMixVelocity() {
	byte voiceVelocity = _velocity;

	if (_channel->_volume != 0) {
		if (voiceVelocity != 0) {
			voiceVelocity = voiceVelocity * _channel->_volume / 63;
			if (_envCurVel != 0) {
				voiceVelocity = voiceVelocity * _envCurVel / 63;
				if (_driver._masterVolume != 0) {
					voiceVelocity = voiceVelocity * (_driver._masterVolume << 2) / 63;
					if (voiceVelocity == 0)
						++voiceVelocity;
				} else {
					voiceVelocity = 0;
				}
			} else {
				voiceVelocity = 0;
			}
		}
	} else {
		voiceVelocity = 0;
	}

	if (!_driver._playSwitch)
		voiceVelocity = 0;

	setVolume(voiceVelocity);
}

void MidiPlayer_AmigaMac1::Channel::noteOn(int8 note, int8 velocity) {
	if (velocity == 0) {
		noteOff(note);
		return;
	}

	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
		Voice *v = *it;

		if (v->_channel == this && v->_note == note) {
			v->_isSustained = false;
			v->noteOff();
			v->noteOn(note, velocity);
			return;
		}
	}

	Voice *v = findVoice();
	if (v)
		v->noteOn(note, velocity);
}

void MidiPlayer_AmigaMac1::Channel::noteOff(int8 note) {
	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
		Voice *v = *it;

		if (v->_channel == this && v->_note == note) {
			if (_hold)
				v->_isSustained = true;
			else {
				v->_isReleased = true;
				v->_envCntDown = 0;
			}
			return;
		}
	}
}

void MidiPlayer_AmigaMac1::Channel::changePatch(int8 patch) {
	_patch = patch;
}

void MidiPlayer_AmigaMac1::Channel::holdPedal(int8 pedal) {
	_hold = pedal;

	if (pedal != 0)
		return;

	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
		Voice *v = *it;

		if (v->_channel == this && v->_isSustained) {
			v->_isSustained = false;
			v->_isReleased = true;
		}
	}
}

void MidiPlayer_AmigaMac1::Channel::setPitchWheel(uint16 pitch) {
	_pitch = pitch;

	for (VoiceIt it = _driver._voices.begin(); it != _driver._voices.end(); ++it) {
		Voice *v = *it;

		if (v->_note != -1 && v->_channel == this)
			v->calcVoiceStep();
	}
}

void MidiPlayer_AmigaMac1::send(uint32 b) {
	Common::StackLock lock(_mixMutex);

	const byte command = b & 0xf0;
	Channel *channel = _channels[b & 0xf];
	const byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch(command) {
	case 0x80:
		channel->noteOff(op1);
		break;
	case 0x90:
		channel->noteOn(op1, op2);
		break;
	case 0xb0:
		switch (op1) {
		case 0x07:
			if (op2 != 0) {
				op2 >>= 1;
				if (op2 == 0)
					++op2;
			}
			channel->_volume = op2;
			break;
		case 0x0a:
			channel->_pan = op2;
			break;
		case 0x40:
			channel->holdPedal(op2);
			break;
		case 0x4b:
			channel->voiceMapping(op2);
			break;
		case 0x7b:
			for (VoiceIt it = _voices.begin(); it != _voices.end(); ++it) {
				Voice *v = *it;

				if (v->_channel == channel && v->_note != -1)
					v->noteOff();
			}
		}
		break;
	case 0xc0:
		channel->changePatch(op1);
		break;
	case 0xe0:
		channel->setPitchWheel((op2 << 7) | op1);
		break;
	}
}

const byte MidiPlayer_AmigaMac1::_envSpeedToStep[32] = {
	0x40, 0x32, 0x24, 0x18, 0x14, 0x0f, 0x0d, 0x0b, 0x09, 0x08, 0x07, 0x06, 0x05, 0x0a, 0x04, 0x03,
	0x05, 0x02, 0x03, 0x0b, 0x05, 0x09, 0x09, 0x01, 0x02, 0x03, 0x07, 0x05, 0x04, 0x03, 0x03, 0x02
};

const byte MidiPlayer_AmigaMac1::_envSpeedToSkip[32] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x01, 0x00, 0x01, 0x07, 0x02, 0x05, 0x07, 0x00, 0x01, 0x02, 0x08, 0x08, 0x08, 0x09, 0x0e, 0x0b
};

const byte MidiPlayer_AmigaMac1::_velocityMap[64] = {
	0x01, 0x02, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x07, 0x08, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
	0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
	0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2a,
	0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x34, 0x35, 0x37, 0x39, 0x3a, 0x3c, 0x3e, 0x40
};

class MidiPlayer_Mac1 : public Mixer_Mac<MidiPlayer_Mac1>, public MidiPlayer_AmigaMac1 {
public:
	MidiPlayer_Mac1(SciVersion version, Audio::Mixer *mixer, Mixer_Mac<MidiPlayer_Mac1>::Mode mode);

	// MidiPlayer
	int open(ResourceManager *resMan) override;

	// MidiDriver
	void close() override;

	// Mixer_Mac
	static int8 applyChannelVolume(byte velocity, byte sample);
	void interrupt() { onTimer(); }
	void onChannelFinished(uint channel);

private:
	static int euclDivide(int x, int y);

	class MacVoice : public MidiPlayer_AmigaMac1::Voice {
	public:
		MacVoice(MidiPlayer_Mac1 &driver, byte id) :
			MidiPlayer_AmigaMac1::Voice(driver, id),
			_macDriver(driver) {}

	private:
		void play(int8 note, int8 velocity) override;
		void stop() override;
		void setVolume(byte volume) override;
		bool calcVoiceStep() override;

		ufrac_t calcStep(int8 note);

		MidiPlayer_Mac1 &_macDriver;
	};
};

MidiPlayer_Mac1::MidiPlayer_Mac1(SciVersion version, Audio::Mixer *mixer, Mixer_Mac<MidiPlayer_Mac1>::Mode mode) :
	Mixer_Mac<MidiPlayer_Mac1>(mode),
	MidiPlayer_AmigaMac1(version, mixer, 1480, false, _mutex) {}

int MidiPlayer_Mac1::open(ResourceManager *resMan) {
	if (_isOpen)
		return MidiDriver::MERR_ALREADY_OPEN;

	const Resource *patch = resMan->findResource(ResourceId(kResourceTypePatch, 7), false);
	if (!patch) {
		warning("MidiPlayer_Mac1: Failed to open patch 7");
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	}

	Common::MemoryReadStream stream(patch->toStream());
	if (!loadInstruments(stream, false)) {
		freeInstruments();
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	}

	for (byte vi = 0; vi < kVoices; ++vi)
		_voices.push_back(new MacVoice(*this, vi));

	for (byte ci = 0; ci < MIDI_CHANNELS; ++ci)
		_channels.push_back(new MidiPlayer_AmigaMac1::Channel(*this));

	startMixer();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO);

	_isOpen = true;

	return 0;
}

void MidiPlayer_Mac1::close() {
	MidiPlayer_AmigaMac1::close();
	stopMixer();
}

void MidiPlayer_Mac1::onChannelFinished(uint channel) {
	_voices[channel]->noteOff();
}

void MidiPlayer_Mac1::MacVoice::play(int8 note, int8 velocity) {
	if (velocity != 0)
		velocity = _velocityMap[velocity >> 1];

	_velocity = velocity;
	_note = note;

	if (!calcVoiceStep()) {
		_note = -1;
		return;
	}

	_macDriver.setChannelVolume(_id, 0);

	uint16 endOffset = _wave->phase2End;

	if (endOffset == 0)
		endOffset = _wave->phase1End;

	uint16 loopLength = 0;

	if (_wave->phase2End != 0 && _noteRange->loop)
		loopLength = endOffset - _wave->phase2Start + 1;

	_macDriver.setChannelData(_id, _wave->samples, _wave->phase1Start, endOffset, loopLength);
}

void MidiPlayer_Mac1::MacVoice::stop() {
	_macDriver.resetChannel(_id);
}

void MidiPlayer_Mac1::MacVoice::setVolume(byte volume) {
	_macDriver.setChannelVolume(_id, volume);
	_macDriver.setChannelPan(_id, _channel->_pan);
}

ufrac_t MidiPlayer_Mac1::MacVoice::calcStep(int8 note) {
	uint16 noteAdj = note + 127 - _wave->nativeNote;
	uint16 pitch = _channel->_pitch;
	pitch /= 170;
	noteAdj += (pitch >> 2) - 12;
	uint octaveRsh = 0;

	if (noteAdj < 255)
		octaveRsh = 21 - (noteAdj + 9) / 12;

	noteAdj = (noteAdj + 9) % 12;

	const uint freqTableIndex = (noteAdj << 2) + (pitch & 3);
	assert(freqTableIndex + 8 < kFreqTableSize);
	ufrac_t step = (ufrac_t)_freqTable[freqTableIndex + 4];

	int16 transpose = _noteRange->transpose;
	if (transpose > 0) {
		ufrac_t delta = (ufrac_t)_freqTable[freqTableIndex + 8] - step;
		delta >>= 4;
		delta >>= octaveRsh;
		delta *= transpose;
		step >>= octaveRsh;
		step += delta;
	} else if (transpose < 0) {
		ufrac_t delta = step - (ufrac_t)_freqTable[freqTableIndex];
		delta >>= 4;
		delta >>= octaveRsh;
		delta *= -transpose;
		step >>= octaveRsh;
		step -= delta;
	} else {
		step >>= octaveRsh;
	}

	return step;
}

bool MidiPlayer_Mac1::MacVoice::calcVoiceStep() {
	int8 note = _note;

	int16 fixedNote = _noteRange->fixedNote;
	if (fixedNote != -1)
		note = fixedNote;

	ufrac_t step = calcStep(note);
	if (step == (ufrac_t)-1)
		return false;

	_macDriver.setChannelStep(_id, step);

	return true;
}

int MidiPlayer_Mac1::euclDivide(int x, int y) {
	// Assumes y > 0
	if (x % y < 0)
		return x / y - 1;
	else
		return x / y;
}

int8 MidiPlayer_Mac1::applyChannelVolume(byte volume, byte sample) {
	return euclDivide((sample - 0x80) * volume, 63);
}

class MidiPlayer_Amiga1 : public Audio::Paula, public MidiPlayer_AmigaMac1 {
public:
	MidiPlayer_Amiga1(SciVersion version, Audio::Mixer *mixer);

	// MidiPlayer
	int open(ResourceManager *resMan) override;

	// MidiDriver
	void close() override;

	// Paula
	void interrupt() override;

private:
	class AmigaVoice : public MidiPlayer_AmigaMac1::Voice {
	public:
		AmigaVoice(MidiPlayer_Amiga1 &driver, uint id) :
			MidiPlayer_AmigaMac1::Voice(driver, id),
			_amigaDriver(driver) {}

		void play(int8 note, int8 velocity) override;
		void stop() override;
		void setVolume(byte volume) override;
		bool calcVoiceStep() override;

	private:
		uint16 calcPeriod(int8 note);

		MidiPlayer_Amiga1 &_amigaDriver;
	};

	bool _isSci1Ega;

	static const byte _velocityMapSci1Ega[64];
};

MidiPlayer_Amiga1::MidiPlayer_Amiga1(SciVersion version, Audio::Mixer *mixer) :
	Paula(true, mixer->getOutputRate(), (mixer->getOutputRate() + kBaseFreq / 2) / kBaseFreq, kFilterModeA500),
	MidiPlayer_AmigaMac1(version, mixer, 224, true, _mutex),
	_isSci1Ega(false) {}

int MidiPlayer_Amiga1::open(ResourceManager *resMan) {
	if (_isOpen)
		return MidiDriver::MERR_ALREADY_OPEN;

	const Resource *patch = resMan->findResource(ResourceId(kResourceTypePatch, 9), false);

	if (!patch) {
		patch = resMan->findResource(ResourceId(kResourceTypePatch, 5), false);

		if (!patch) {
			warning("MidiPlayer_Amiga1: Failed to open patch");
			return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
		}

		_isSci1Ega = true;
	}

	// SCI1 EGA banks start with a uint32 patch size, skip it
	Common::MemoryReadStream stream(patch->toStream(_isSci1Ega ? 4 : 0));
	if (!loadInstruments(stream, _isSci1Ega)) {
		freeInstruments();
		return MidiDriver::MERR_DEVICE_NOT_AVAILABLE;
	}

	for (byte vi = 0; vi < kVoices; ++vi)
		_voices.push_back(new AmigaVoice(*this, vi));

	for (byte ci = 0; ci < MIDI_CHANNELS; ++ci)
		_channels.push_back(new MidiPlayer_AmigaMac1::Channel(*this));

	startPaula();
	// Enable reverse stereo to counteract Audio::Paula's reverse stereo
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, DisposeAfterUse::NO, false, true);

	_isOpen = true;

	return 0;
}

void MidiPlayer_Amiga1::close() {
	MidiPlayer_AmigaMac1::close();
	stopPaula();
}

void MidiPlayer_Amiga1::interrupt() {
	// In the original driver, the interrupt handlers for each voice
	// call voiceOff when non-looping samples are finished.
	for (uint vi = 0; vi < kVoices; ++vi) {
		if (_voices[vi]->_note != -1 && !_voices[vi]->_noteRange->loop && getChannelDmaCount(vi) > 0)
			_voices[vi]->noteOff();
	}

	onTimer();
}

void MidiPlayer_Amiga1::AmigaVoice::play(int8 note, int8 velocity) {
	if (velocity != 0) {
		if (_amigaDriver._isSci1Ega)
			velocity = _velocityMapSci1Ega[velocity >> 1];
		else
			velocity = _velocityMap[velocity >> 1];
	}

	_velocity = velocity;
	_note = note;

	if (!calcVoiceStep()) {
		_note = -1;
		return;
	}

	_amigaDriver.setChannelVolume(_id, 0);

	// The original driver uses double buffering to play the samples. We will instead
	// play the data directly. The original driver might be OB1 in end offsets and
	// loop sizes on occasion. Currently, this behavior isn't taken into account, and
	// the samples are played according to the meta data in the sound bank.
	const int8 *samples = (const int8 *)_wave->samples;
	uint16 phase1Start = _wave->phase1Start;
	uint16 phase1End = _wave->phase1End;
	uint16 phase2Start = _wave->phase2Start;
	uint16 phase2End = _wave->phase2End;
	bool loop = _noteRange->loop;

	uint16 endOffset = phase2End;

	if (endOffset == 0)
		endOffset = phase1End;

	// Paula consumes one word at a time
	phase1Start &= 0xfffe;
	phase2Start &= 0xfffe;

	// If endOffset is odd, the sample byte at endOffset is played, otherwise it isn't
	endOffset = (endOffset + 1) & 0xfffe;

	int phase1Len = endOffset - phase1Start;
	int phase2Len = endOffset - phase2Start;

	// The original driver delays the voice start for two MIDI ticks, possibly
	// due to DMA requirements
	if (phase2End == 0 || !loop) {
		// Non-looping
		_amigaDriver.setChannelData(_id, samples + phase1Start, nullptr, phase1Len, 0);
	} else {
		// Looping
		_amigaDriver.setChannelData(_id, samples + phase1Start, samples + phase2Start, phase1Len, phase2Len);
	}
}

void MidiPlayer_Amiga1::AmigaVoice::stop() {
	_amigaDriver.clearVoice(_id);
}

void MidiPlayer_Amiga1::AmigaVoice::setVolume(byte volume) {
	_amigaDriver.setChannelVolume(_id, volume);
}

uint16 MidiPlayer_Amiga1::AmigaVoice::calcPeriod(int8 note) {
	uint16 noteAdj = note + 127 - _wave->nativeNote;
	uint16 pitch = _channel->_pitch;
	pitch /= 170;
	noteAdj += (pitch >> 2) - 12;

	// SCI1 EGA is off by one note
	if (_amigaDriver._isSci1Ega)
		++noteAdj;

	const uint octaveRsh = noteAdj / 12;
	noteAdj %= 12;

	const uint freqTableIndex = (noteAdj << 2) + (pitch & 3);
	assert(freqTableIndex + 8 < kFreqTableSize);
	uint32 period = _freqTable[freqTableIndex + 4];

	int16 transpose = _noteRange->transpose;
	if (transpose > 0) {
		uint32 delta = period - _freqTable[freqTableIndex + 8];
		delta >>= 4;
		delta *= transpose;
		period -= delta;
	} else if (transpose < 0) {
		uint32 delta = _freqTable[freqTableIndex] - period;
		delta >>= 4;
		delta *= -transpose;
		period += delta;
	}

	period >>= octaveRsh;

	if (period < 0x7c || period > 0xffff)
		return (uint16)-1;

	return period;
}

bool MidiPlayer_Amiga1::AmigaVoice::calcVoiceStep() {
	int8 note = _note;

	int16 fixedNote = _noteRange->fixedNote;
	if (fixedNote != -1)
		note = fixedNote;

	uint16 period = calcPeriod(note);
	if (period == (uint16)-1)
		return false;

	// Audio::Paula uses int16 instead of uint16?
	_amigaDriver.setChannelPeriod(_id, period);

	return true;
}

const byte MidiPlayer_Amiga1::_velocityMapSci1Ega[64] = {
	0x01, 0x04, 0x07, 0x0a, 0x0c, 0x0f, 0x11, 0x15, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x21, 0x22, 0x23,
	0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x30, 0x31, 0x31,
	0x32, 0x32, 0x33, 0x33, 0x34, 0x34, 0x35, 0x35, 0x36, 0x36, 0x37, 0x37, 0x38, 0x38, 0x38, 0x39,
	0x39, 0x39, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3c, 0x3d, 0x3e, 0x3e, 0x3f, 0x3f, 0x40, 0x40
};

MidiPlayer *MidiPlayer_AmigaMac1_create(SciVersion version, Common::Platform platform) {
	if (platform == Common::kPlatformMacintosh)
		return new MidiPlayer_Mac1(version, g_system->getMixer(), Mixer_Mac<MidiPlayer_Mac1>::kModeHqStereo);
	else
		return new MidiPlayer_Amiga1(version, g_system->getMixer());
}

} // End of namespace Sci
