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

#include "common/file.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "audio/fmopl.h"
#include "audio/mididrv.h"

#include "dgds/sound/resource/sci_resource.h"
#include "dgds/sound/drivers/mididriver.h"
#include "dgds/sound/scispan.h"
#include "dgds/dgds.h"

namespace Dgds {

#ifdef __DC__
#define STEREO false
#else
#define STEREO true
#endif

class MidiDriver_AdLib : public MidiDriver {
public:
	enum {
		kVoices = 9,
		kRhythmKeys = 62
	};

	MidiDriver_AdLib() : _isSCI0(false), _playSwitch(true), _masterVolume(15),
		_numVoiceMax(kVoices), _rhythmKeyMap(), _opl(nullptr), _adlibTimerParam(nullptr), _adlibTimerProc(nullptr), _stereo(false), _isOpen(false) { }
	~MidiDriver_AdLib() override { }

	// MidiDriver
	int open() override { return -1; } // Dummy implementation (use openAdLib)
	int openAdLib();
	void close() override;
	void send(uint32 b) override;

	MidiChannel *allocateChannel() override { return nullptr; }
	MidiChannel *getPercussionChannel() override { return nullptr; }
	bool isOpen() const override { return _isOpen; }
	uint32 getBaseTempo() override { return 1000000 / OPL::OPL::kDefaultCallbackFrequency; }

	// MidiDriver
	void setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) override;

	void onTimer();

	void setVolume(byte volume);
	void playSwitch(bool play);
	bool loadResource(const SciSpan<const byte> &data);
	uint32 property(int prop, uint32 param) override;

	bool useRhythmChannel() const { return _rhythmKeyMap; }

private:
	enum ChannelID {
		kLeftChannel = 1,
		kRightChannel = 2
	};

	struct AdLibOperator {
		bool amplitudeMod;
		bool vibrato;
		bool envelopeType;
		bool kbScaleRate;
		byte frequencyMult;		// (0-15)
		byte kbScaleLevel;		// (0-3)
		byte totalLevel;		// (0-63, 0=max, 63=min)
		byte attackRate;		// (0-15)
		byte decayRate;			// (0-15)
		byte sustainLevel;		// (0-15)
		byte releaseRate;		// (0-15)
		byte waveForm;			// (0-3)
	};

	struct AdLibModulator {
		byte feedback;			// (0-7)
		bool algorithm;
	};

	struct AdLibPatch {
		AdLibOperator op[2];
		AdLibModulator mod;
	};

	struct Channel {
		uint8 patch;			// Patch setting
		uint8 volume;			// Channel volume (0-63)
		uint8 pan;				// Pan setting (0-127, 64 is center)
		uint8 holdPedal;		// Hold pedal setting (0 to 63 is off, 127 to 64 is on)
		uint8 extraVoices;		// The number of additional voices this channel optimally needs
		uint16 pitchWheel;		// Pitch wheel setting (0-16383, 8192 is center)
		uint8 lastVoice;		// Last voice used for this MIDI channel
		bool enableVelocity;	// Enable velocity control (SCI0)
		uint8 voices;			// Number of voices currently used by this channel
		uint8 mappedVoices;		// Number of voices currently mapped to this channel

		Channel() : patch(0), volume(63), pan(64), holdPedal(0), extraVoices(0),
					pitchWheel(8192), lastVoice(0), enableVelocity(false), voices(0),
					mappedVoices(0) { }
	};

	struct AdLibVoice {
		int8 channel;			// MIDI channel that is currently using this voice, or -1
		int8 mappedChannel;		// MIDI channel that this voice is mapped to, or -1
		int8 note;				// Currently playing MIDI note or -1
		int patch;				// Currently playing patch or -1
		uint8 velocity;			// Note velocity
		bool isSustained;		// Flag indicating a note that is being sustained by the hold pedal
		uint16 age;				// Age of the current note

		AdLibVoice() : channel(-1), mappedChannel(-1), note(-1), patch(-1), velocity(0), isSustained(false), age(0) { }
	};

	bool _stereo;
	bool _isSCI0;
	OPL::OPL *_opl;
	bool _isOpen;
	bool _playSwitch;
	int _masterVolume;
	const uint8 _numVoiceMax;
	Channel _channels[MIDI_CHANNELS];
	AdLibVoice _voices[kVoices];
	Common::SpanOwner<SciSpan<const byte> > _rhythmKeyMap;
	Common::Array<AdLibPatch> _patches;
	Common::List<int> _voiceQueue;

	Common::TimerManager::TimerProc _adlibTimerProc;
	void *_adlibTimerParam;

	void loadInstrument(const SciSpan<const byte> &ins);
	void voiceOn(int voice, int note, int velocity);
	void voiceOff(int voice);
	void setPatch(int voice, int patch);
	void setNote(int voice, int note, bool key);
	void setVelocity(int voice);
	void setOperator(int oper, AdLibOperator &op);
	void setRegister(int reg, int value, int channels = kLeftChannel | kRightChannel);
	void renewNotes(int channel, bool key);
	void noteOn(int channel, int note, int velocity);
	void noteOff(int channel, int note);
	int findVoice(int channel);
	int findVoiceLateSci11(int channel);
	void voiceMapping(int channel, int voices);
	void assignVoices(int channel, int voices);
	void releaseVoices(int channel, int voices);
	void donateVoices();
	void queueMoveToBack(int voice);
	void setVelocityReg(int regOffset, int velocity, int kbScaleLevel, int pan);
	int calcVelocity(int voice, int op);
};

class MidiPlayer_AdLib : public MidiPlayer {
public:
	MidiPlayer_AdLib() : MidiPlayer() { _driver = new MidiDriver_AdLib(); }
	~MidiPlayer_AdLib() override {
		delete _driver;
		_driver = nullptr;
	}

	int open() override;
	void close() override;

	byte getPlayId() const override;
	int getPolyphony() const override { return MidiDriver_AdLib::kVoices; }
	bool hasRhythmChannel() const override { return false; }
	void setVolume(byte volume) override { static_cast<MidiDriver_AdLib *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) override { static_cast<MidiDriver_AdLib *>(_driver)->playSwitch(play); }
	int getLastChannel() const override { return (static_cast<const MidiDriver_AdLib *>(_driver)->useRhythmChannel() ? 8 : 15); }
};

static const byte registerOffset[MidiDriver_AdLib::kVoices] = {
	0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
};

static const byte velocityMap1[64] = {
	0x00, 0x0c, 0x0d, 0x0e, 0x0f, 0x11, 0x12, 0x13,
	0x14, 0x16, 0x17, 0x18, 0x1a, 0x1b, 0x1c, 0x1d,
	0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2d, 0x2d, 0x2e,
	0x2f, 0x30, 0x31, 0x32, 0x32, 0x33, 0x34, 0x34,
	0x35, 0x36, 0x36, 0x37, 0x38, 0x38, 0x39, 0x3a,
	0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d,
	0x3d, 0x3e, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f
};

static const byte velocityMap2[64] = {
	0x00, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
	0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x21,
	0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
	0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x2f, 0x30,
	0x31, 0x32, 0x32, 0x33, 0x34, 0x34, 0x35, 0x36,
	0x36, 0x37, 0x38, 0x38, 0x39, 0x39, 0x3a, 0x3a,
	0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d,
	0x3d, 0x3e, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f
};

// One octave with three pitch wheel positions after each note
static const int adlibFreq[48] = {
	0x157, 0x15c, 0x161, 0x166, 0x16b, 0x171, 0x176, 0x17b,
	0x181, 0x186, 0x18c, 0x192, 0x198, 0x19e, 0x1a4, 0x1aa,
	0x1b0, 0x1b6, 0x1bd, 0x1c3, 0x1ca, 0x1d0, 0x1d7, 0x1de,
	0x1e5, 0x1ec, 0x1f3, 0x1fa, 0x202, 0x209, 0x211, 0x218,
	0x220, 0x228, 0x230, 0x238, 0x241, 0x249, 0x252, 0x25a,
	0x263, 0x26c, 0x275, 0x27e, 0x287, 0x290, 0x29a, 0x2a4
};

int MidiDriver_AdLib::openAdLib() {
	_stereo = STEREO;

	debug(3, "ADLIB: Starting driver in %s mode", (_isSCI0 ? "SCI0" : "SCI1"));

	// Fill in the voice queue
	for (int i = 0; i < kVoices; ++i)
		_voiceQueue.push_back(i);

	_opl = OPL::Config::create(_stereo ? OPL::Config::kDualOpl2 : OPL::Config::kOpl2);

	// Try falling back to mono, thus plain OPL2 emulator, when no Dual OPL2 is available.
	if (!_opl && _stereo) {
		_stereo = false;
		_opl = OPL::Config::create(OPL::Config::kOpl2);
	}

	if (!_opl)
		return -1;

	if (!_opl->init()) {
		delete _opl;
		_opl = nullptr;
		return -1;
	}

	setRegister(0xBD, 0);
	setRegister(0x08, 0);
	setRegister(0x01, 0x20);

	_isOpen = true;

	_opl->start(new Common::Functor0Mem<void, MidiDriver_AdLib>(this, &MidiDriver_AdLib::onTimer));

	return 0;
}

void MidiDriver_AdLib::close() {
	delete _opl;
	_rhythmKeyMap.clear();
}

void MidiDriver_AdLib::setVolume(byte volume) {
	_masterVolume = volume;
	renewNotes(-1, true);
}

// MIDI messages can be found at https://web.archive.org/web/20120128110425/http://www.midi.org/techspecs/midimessages.php
void MidiDriver_AdLib::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	//debug("midi send %02x %x %02x %02x", command, channel, op1, op2);

	switch (command) {
	case 0x80:
		noteOff(channel, op1);
		break;
	case 0x90:
		noteOn(channel, op1, op2);
		break;
	case 0xb0:
		switch (op1) {
		case 0x07:
			_channels[channel].volume = op2 >> 1;
			renewNotes(channel, true);
			break;
		case 0x0a:
			_channels[channel].pan = op2;
			renewNotes(channel, true);
			break;
		case 0x40:
			_channels[channel].holdPedal = op2;
			if (op2 == 0) {
				for (int i = 0; i < kVoices; i++) {
					if ((_voices[i].channel == channel) && _voices[i].isSustained)
						voiceOff(i);
				}
			}
			break;
		case 0x4b:
#ifndef ADLIB_DISABLE_VOICE_MAPPING
			voiceMapping(channel, op2);
#endif
			break;
		case 0x4e:
			_channels[channel].enableVelocity = op2;
			break;
		case SCI_MIDI_CHANNEL_NOTES_OFF:
			for (int i = 0; i < kVoices; i++)
				if ((_voices[i].channel == channel) && (_voices[i].note != -1))
					voiceOff(i);
			break;
		default:
			//warning("ADLIB: ignoring MIDI command %02x %02x %02x", command | channel, op1, op2);
			break;
		}
		break;
	case 0xc0:
		_channels[channel].patch = op1;
		break;
	// The original AdLib driver from sierra ignores aftertouch completely, so should we
	case 0xa0: // Polyphonic key pressure (aftertouch)
	case 0xd0: // Channel pressure (aftertouch)
		break;
	case 0xe0:
		_channels[channel].pitchWheel = (op1 & 0x7f) | ((op2 & 0x7f) << 7);
		renewNotes(channel, true);
		break;
	default:
		warning("ADLIB: Unknown event %02x", command);
	}
}

void MidiDriver_AdLib::setTimerCallback(void *timerParam, Common::TimerManager::TimerProc timerProc) {
	_adlibTimerProc = timerProc;
	_adlibTimerParam = timerParam;
}

void MidiDriver_AdLib::onTimer() {
	if (_adlibTimerProc)
		(*_adlibTimerProc)(_adlibTimerParam);

	// Increase the age of the notes
	for (int i = 0; i < kVoices; i++) {
		if (_voices[i].note != -1)
			_voices[i].age++;
	}
}

void MidiDriver_AdLib::loadInstrument(const SciSpan<const byte> &ins) {
	AdLibPatch patch;

	// Set data for the operators
	for (int i = 0; i < 2; i++) {
		const byte *op = ins.getUnsafeDataAt(i * 13, 13);
		patch.op[i].kbScaleLevel = op[0] & 0x3;
		patch.op[i].frequencyMult = op[1] & 0xf;
		patch.op[i].attackRate = op[3] & 0xf;
		patch.op[i].sustainLevel = op[4] & 0xf;
		patch.op[i].envelopeType = op[5];
		patch.op[i].decayRate = op[6] & 0xf;
		patch.op[i].releaseRate = op[7] & 0xf;
		patch.op[i].totalLevel = op[8] & 0x3f;
		patch.op[i].amplitudeMod = op[9];
		patch.op[i].vibrato = op[10];
		patch.op[i].kbScaleRate = op[11];
	}
	patch.op[0].waveForm = ins[26] & 0x3;
	patch.op[1].waveForm = ins[27] & 0x3;

	// Set data for the modulator
	patch.mod.feedback = ins[2] & 0x7;
	patch.mod.algorithm = !ins[12]; // Flag is inverted

	_patches.push_back(patch);
}

void MidiDriver_AdLib::voiceMapping(int channel, int voices) {
	int curVoices = 0;

	for (int i = 0; i < _numVoiceMax; i++)
		if (_voices[i].mappedChannel == channel)
			curVoices++;

	curVoices += _channels[channel].extraVoices;

	if (curVoices < voices) {
		debug(3, "ADLIB: assigning %i additional voices to channel %i", voices - curVoices, channel);
		assignVoices(channel, voices - curVoices);
	} else if (curVoices > voices) {
		debug(3, "ADLIB: releasing %i voices from channel %i", curVoices - voices, channel);
		releaseVoices(channel, curVoices - voices);
		donateVoices();
	}
}

void MidiDriver_AdLib::assignVoices(int channel, int voices) {
	assert(voices > 0);

	for (int i = 0; i < _numVoiceMax; i++)
		if (_voices[i].mappedChannel == -1) {
			if (_voices[i].note != -1) // Late SCI1.1, stop note on unmapped channel
				voiceOff(i);
			_voices[i].mappedChannel = channel;
			++_channels[channel].mappedVoices;
			if (--voices == 0)
				return;
		}

	// This is already too advanced for SCI0...
	if (!_isSCI0)
		_channels[channel].extraVoices += voices;
}

void MidiDriver_AdLib::releaseVoices(int channel, int voices) {
	if (_channels[channel].extraVoices >= voices) {
		_channels[channel].extraVoices -= voices;
		return;
	}

	voices -= _channels[channel].extraVoices;
	_channels[channel].extraVoices = 0;

	for (int i = 0; i < _numVoiceMax; i++) {
		if ((_voices[i].mappedChannel == channel) && (_voices[i].note == -1)) {
			_voices[i].mappedChannel = -1;
			--_channels[channel].mappedVoices;
			if (--voices == 0)
				return;
		}
	}

	for (int i = 0; i < _numVoiceMax; i++) {
		if (_voices[i].mappedChannel == channel) {
			voiceOff(i);
			_voices[i].mappedChannel = -1;
			--_channels[channel].mappedVoices;
			if (--voices == 0)
				return;
		}
	}
}

void MidiDriver_AdLib::donateVoices() {
	if (_isSCI0)
		return;

	int freeVoices = 0;

	for (int i = 0; i < kVoices; i++)
		if (_voices[i].mappedChannel == -1)
			freeVoices++;

	if (freeVoices == 0)
		return;

	for (int i = 0; i < MIDI_CHANNELS; i++) {
		if (_channels[i].extraVoices >= freeVoices) {
			assignVoices(i, freeVoices);
			_channels[i].extraVoices -= freeVoices;
			return;
		} else if (_channels[i].extraVoices > 0) {
			assignVoices(i, _channels[i].extraVoices);
			freeVoices -= _channels[i].extraVoices;
			_channels[i].extraVoices = 0;
		}
	}
}

void MidiDriver_AdLib::renewNotes(int channel, bool key) {
	for (int i = 0; i < kVoices; i++) {
		// Update all notes playing this channel
		if ((channel == -1) || (_voices[i].channel == channel)) {
			if (_voices[i].note != -1)
				setNote(i, _voices[i].note, key);
		}
	}
}

void MidiDriver_AdLib::noteOn(int channel, int note, int velocity) {
	if (velocity == 0)
		return noteOff(channel, note);

	velocity >>= 1;

	// Check for playable notes
	if ((note < 12) || (note > 107))
		return;

	for (int i = 0; i < kVoices; i++) {
		if ((_voices[i].channel == channel) && (_voices[i].note == note)) {
			voiceOff(i);
			voiceOn(i, note, velocity);
			return;
		}
	}

	int voice = _rhythmKeyMap ? findVoiceLateSci11(channel) : findVoice(channel);

	if (voice == -1) {
		debug(3, "ADLIB: failed to find free voice assigned to channel %i", channel);
		return;
	}

	voiceOn(voice, note, velocity);
}

int MidiDriver_AdLib::findVoice(int channel) {
	int voice = -1;
	int oldestVoice = -1;
	uint32 oldestAge = 0;

	// Try to find a voice assigned to this channel that is free (round-robin)
	for (int i = 0; i < kVoices; i++) {
		int v = (_channels[channel].lastVoice + i + 1) % kVoices;

		if (_voices[v].mappedChannel == channel) {
			if (_voices[v].note == -1) {
				voice = v;
				_voices[voice].channel = channel;
				break;
			}

			// We also keep track of the oldest note in case the search fails
			// Notes started in the current time slice will not be selected
			if (_voices[v].age >= oldestAge) {
				oldestAge = _voices[v].age;
				oldestVoice = v;
			}
		}
	}

	if (voice == -1) {
		if (!oldestAge)
			return -1;
		voiceOff(oldestVoice);
		voice = oldestVoice;
		_voices[voice].channel = channel;
	}

	_channels[channel].lastVoice = voice;

	return voice;
}

int MidiDriver_AdLib::findVoiceLateSci11(int channel) {
	Common::List<int>::const_iterator it;

	// Search for unused voice
	for (it = _voiceQueue.begin(); it != _voiceQueue.end(); ++it) {
		int voice = *it;
		if (_voices[voice].note == -1 && _voices[voice].patch == _channels[channel].patch) {
			_voices[voice].channel = channel;
			return voice;
		}
	}

	// Same as before, minus the program check
	for (it = _voiceQueue.begin(); it != _voiceQueue.end(); ++it) {
		int voice = *it;
		if (_voices[voice].note == -1) {
			_voices[voice].channel = channel;
			return voice;
		}
	}

	// Search for channel with highest excess of voices
	int maxExceed = 0;
	int maxExceedChan = 0;
	for (uint i = 0; i < MIDI_CHANNELS; ++i) {
		if (_channels[i].voices > _channels[i].mappedVoices) {
			int exceed = _channels[i].voices - _channels[i].mappedVoices;
			if (exceed > maxExceed) {
				maxExceed = exceed;
				maxExceedChan = i;
			}
		}
	}

	// Stop voice on channel with highest excess if possible, otherwise stop
	// note on this channel.
	int stopChan = (maxExceed > 0) ? maxExceedChan : channel;

	for (it = _voiceQueue.begin(); it != _voiceQueue.end(); ++it) {
		int voice = *it;
		if (_voices[voice].channel == stopChan) {
			voiceOff(voice);
			_voices[voice].channel = channel;
			return voice;
		}
	}

	return -1;
}

void MidiDriver_AdLib::queueMoveToBack(int voice) {
	_voiceQueue.remove(voice);
	_voiceQueue.push_back(voice);
}

void MidiDriver_AdLib::noteOff(int channel, int note) {
	for (int i = 0; i < kVoices; i++) {
		if ((_voices[i].channel == channel) && (_voices[i].note == note)) {
			if (_channels[channel].holdPedal)
				_voices[i].isSustained = true;
			else
				voiceOff(i);
			return;
		}
	}
}

void MidiDriver_AdLib::voiceOn(int voice, int note, int velocity) {
	int channel = _voices[voice].channel;
	int patch = _channels[channel].patch;

	_voices[voice].age = 0;
	++_channels[channel].voices;
	queueMoveToBack(voice);

	if ((channel == 9) && _rhythmKeyMap) {
		patch = CLIP(note, 27, 88) + 101;
	}

	// Set patch if different from current patch
	if (patch != _voices[voice].patch && _playSwitch)
		setPatch(voice, patch);

	_voices[voice].velocity = velocity;
	setNote(voice, note, true);
}

void MidiDriver_AdLib::voiceOff(int voice) {
	int channel = _voices[voice].channel;

	_voices[voice].isSustained = false;
	setNote(voice, _voices[voice].note, 0);
	_voices[voice].note = -1;
	_voices[voice].age = 0;
	queueMoveToBack(voice);
	--_channels[channel].voices;
}

void MidiDriver_AdLib::setNote(int voice, int note, bool key) {
	int channel = _voices[voice].channel;

	if ((channel == 9) && _rhythmKeyMap)
		note = _rhythmKeyMap[CLIP(note, 27, 88) - 27];

	_voices[voice].note = note;

	int index = note << 2;
	uint16 pitchWheel = _channels[channel].pitchWheel;
	int sign;

	if (pitchWheel == 0x2000) {
		pitchWheel = 0;
		sign = 0;
	} else if (pitchWheel > 0x2000) {
		pitchWheel -= 0x2000;
		sign = 1;
	} else {
		pitchWheel = 0x2000 - pitchWheel;
		sign = -1;
	}

	pitchWheel /= 171;

	if (sign == 1)
		index += pitchWheel;
	else
		index -= pitchWheel;

	if (index > 0x1fc) // Limit to max MIDI note (<< 2)
		index = 0x1fc;

	if (index < 0) // Not in SSCI
		index = 0;

	int freq = adlibFreq[index % 48];

	setRegister(0xA0 + voice, freq & 0xff);

	int oct = index / 48;
	if (oct > 0)
		--oct;

	if (oct > 7) // Not in SSCI
		oct = 7;

	setRegister(0xB0 + voice, (key << 5) | (oct << 2) | (freq >> 8));
	setVelocity(voice);
}

void MidiDriver_AdLib::setVelocity(int voice) {
	AdLibPatch &patch = _patches[_voices[voice].patch];
	int pan = _channels[_voices[voice].channel].pan;
	setVelocityReg(registerOffset[voice] + 3, calcVelocity(voice, 1), patch.op[1].kbScaleLevel, pan);

	// In AM mode we need to set the level for both operators
	if (_patches[_voices[voice].patch].mod.algorithm == 1)
		setVelocityReg(registerOffset[voice], calcVelocity(voice, 0), patch.op[0].kbScaleLevel, pan);
}

int MidiDriver_AdLib::calcVelocity(int voice, int op) {
	if (_isSCI0) {
		int velocity = _masterVolume;

		if (velocity > 0)
			velocity += 3;

		if (velocity > 15)
			velocity = 15;

		int insVelocity;
		if (_channels[_voices[voice].channel].enableVelocity)
			insVelocity = _voices[voice].velocity;
		else
			insVelocity = 63 - _patches[_voices[voice].patch].op[op].totalLevel;

		// Note: Later SCI0 has a static table that is close to this formula, but not exactly the same.
		// Early SCI0 does (velocity * (insVelocity / 15))
		return velocity * insVelocity / 15;
	} else {
		AdLibOperator &oper = _patches[_voices[voice].patch].op[op];
		int velocity = _channels[_voices[voice].channel].volume + 1;
		velocity = velocity * (velocityMap1[_voices[voice].velocity] + 1) / 64;
		velocity = velocity * (_masterVolume + 1) / 16;

		if (--velocity < 0)
			velocity = 0;

		return velocityMap2[velocity] * (63 - oper.totalLevel) / 63;
	}
}

void MidiDriver_AdLib::setVelocityReg(int regOffset, int velocity, int kbScaleLevel, int pan) {
	if (!_playSwitch)
		velocity = 0;

	if (_stereo) {
		int velLeft = velocity;
		int velRight = velocity;

		if (pan > 0x40)
			velLeft = velLeft * (0x7f - pan) / 0x3f;
		else if (pan < 0x40)
			velRight = velRight * pan / 0x40;

		setRegister(0x40 + regOffset, (kbScaleLevel << 6) | (63 - velLeft), kLeftChannel);
		setRegister(0x40 + regOffset, (kbScaleLevel << 6) | (63 - velRight), kRightChannel);
	} else {
		setRegister(0x40 + regOffset, (kbScaleLevel << 6) | (63 - velocity));
	}
}

void MidiDriver_AdLib::setPatch(int voice, int patch) {
	if ((patch < 0) || ((uint)patch >= _patches.size())) {
		warning("ADLIB: Invalid patch %i requested", patch);
		// Substitute instrument 0
		patch = 0;
	}

	_voices[voice].patch = patch;
	AdLibModulator &mod = _patches[patch].mod;

	// Set the common settings for both operators
	setOperator(registerOffset[voice], _patches[patch].op[0]);
	setOperator(registerOffset[voice] + 3, _patches[patch].op[1]);

	// Set the additional settings for the modulator
	byte algorithm = mod.algorithm ? 1 : 0;
	setRegister(0xC0 + voice, (mod.feedback << 1) | algorithm);
}

void MidiDriver_AdLib::setOperator(int reg, AdLibOperator &op) {
	setRegister(0x40 + reg, (op.kbScaleLevel << 6) | op.totalLevel);
	setRegister(0x60 + reg, (op.attackRate << 4) | op.decayRate);
	setRegister(0x80 + reg, (op.sustainLevel << 4) | op.releaseRate);
	setRegister(0x20 + reg, (op.amplitudeMod << 7) | (op.vibrato << 6)
				| (op.envelopeType << 5) | (op.kbScaleRate << 4) | op.frequencyMult);
	setRegister(0xE0 + reg, op.waveForm);
}

void MidiDriver_AdLib::setRegister(int reg, int value, int channels) {
	if (channels & kLeftChannel) {
		_opl->write(0x220, reg);
		_opl->write(0x221, value);
	}

	if (_stereo) {
		if (channels & kRightChannel) {
			_opl->write(0x222, reg);
			_opl->write(0x223, value);
		}
	}
}

void MidiDriver_AdLib::playSwitch(bool play) {
	_playSwitch = play;
	renewNotes(-1, play);
}

bool MidiDriver_AdLib::loadResource(const SciSpan<const byte> &data) {
	const uint32 size = data.size();
	if (size != 1344 && size != 2690 && size != 5382) {
		error("ADLIB: Unsupported patch format (%u bytes)", size);
		return false;
	}

	for (int i = 0; i < 48; i++)
		loadInstrument(data.subspan(28 * i));

	if (size == 1344) {
		byte dummy[28] = {0};

		// Only 48 instruments, add dummies
		for (int i = 0; i < 48; i++)
			loadInstrument(SciSpan<const byte>(dummy, sizeof(dummy)));
	} else if (size == 2690) {
		for (int i = 48; i < 96; i++)
			loadInstrument(data.subspan(2 + (28 * i)));
	} else {
		// SCI1.1 and later
		for (int i = 48; i < 190; i++) {
			loadInstrument(data.subspan(28 * i));
		}

		_rhythmKeyMap->allocateFromSpan(data.subspan(5320, kRhythmKeys));
	}

	return true;
}

uint32 MidiDriver_AdLib::property(int prop, uint32 param) {
	switch(prop) {
	case MIDI_PROP_MASTER_VOLUME:
		if (param != 0xffff)
			_masterVolume = param;
		return _masterVolume;
	default:
		break;
	}
	return 0;
}


int MidiPlayer_AdLib::open() {
	// Load up the patch.003 file, parse out the instruments
	SciResource *res = getMidiPatchData(3);
	bool ok = false;

	if (res) {
		ok = static_cast<MidiDriver_AdLib *>(_driver)->loadResource(*res);
		delete res;
	} else {
		// Early SCI0 games have the sound bank embedded in the AdLib driver

		Common::File f;

		if (f.open("ADL.DRV")) {
			int size = f.size();
			const uint patchSize = 1344;

			// Note: Funseeker's Guide also has another version of adl.drv, 8803 bytes.
			// This isn't supported, but it's not really used anywhere, as that demo
			// doesn't have sound anyway.
			if (size == 5684 || size == 5720 || size == 5727) {
				ok = f.seek(0x45a);
				if (ok) {
					Common::SpanOwner<SciSpan<const byte> > patchData;
					patchData->allocateFromStream(f, patchSize);
					ok = static_cast<MidiDriver_AdLib *>(_driver)->loadResource(*patchData);
				}
			}
		}
	}

	if (!ok) {
		warning("ADLIB: Failed to load patch.003");
		return -1;
	}

	return static_cast<MidiDriver_AdLib *>(_driver)->openAdLib();
}

void MidiPlayer_AdLib::close() {
	if (_driver) {
		_driver->close();
	}
}

byte MidiPlayer_AdLib::getPlayId() const {
	return 0x00;
}

MidiPlayer *MidiPlayer_AdLib_create() {
	return new MidiPlayer_AdLib();
}

} // End of namespace Dgds
