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

#define VOLUME_SHIFT 3

#define BASE_NOTE 129	// A10
#define BASE_OCTAVE 10	// A10, as I said

static const int freq_table[12] = { // A4 is 440Hz, halftone map is x |-> ** 2^(x/12)
	28160, // A10
	29834,
	31608,
	33488,
	35479,
	37589,
	39824,
	42192,
	44701,
	47359,
	50175,
	53159
};

static inline int get_freq(int note) {
	int halftone_delta = note - BASE_NOTE;
	int oct_diff = ((halftone_delta + BASE_OCTAVE * 12) / 12) - BASE_OCTAVE;
	int halftone_index = (halftone_delta + (12 * 100)) % 12;
	int freq = (!note) ? 0 : freq_table[halftone_index] / (1 << (-oct_diff));

	return freq;
}

class MidiDriver_PCJr : public MidiDriver_Emulated {
public:
	friend class MidiPlayer_PCJr;

	enum {
		kMaxChannels = 3
	};

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
	void updateChannelVolume(int chan);
	void updateSounds();

	void envAT(byte chan);
	void envST(byte chan);
	void envRL(byte chan);
	void envINST(byte chan);
	void chanNoteOn(byte chan, byte note, byte velocity);
	void chanOff(byte chan);

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

	byte _globalVolume; // Base volume
	byte _chanVolume[kMaxChannels];
	byte _chanVelocity[kMaxChannels];
	int8 _chanEnvVolume[kMaxChannels];
	uint16 _smpVolume[kMaxChannels];
	byte _notes[kMaxChannels]; // Current halftone, or 0 if off
	byte _envState[kMaxChannels];
	byte _envCount[kMaxChannels];
	byte _envCount2[kMaxChannels];
	const byte *_envData[kMaxChannels];
	int8 _envAttn[kMaxChannels];
	int _freq_count[kMaxChannels];
	byte _partMapping[kMaxChannels];
	uint16 _duration[kMaxChannels];
	uint16 _releaseDuration[kMaxChannels];
	byte _chanSustain[kMaxChannels];
	byte _chanRelease[kMaxChannels];
	byte _chanMapping[16];
	byte _chanMissing[16];
	byte _program[16];
	byte _partSustain[16];
	uint32 _sndUpdateSmpQty;
	uint32 _sndUpdateSmpQtyRem;
	uint32 _sndUpdateCountDown;
	uint32 _sndUpdateCountDownRem;
	const uint16 *_smpVolTable;
	const uint16 *_instrumentOffsets;
	const uint8 *_instrumentData;
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
		pitchBend(part, op1 | (op2 << 7));
		break;
	default:
		debug(2, "Unused MIDI command %02x %02x %02x", command, op1, op2);
		break;
	}
}

uint32 MidiDriver_PCJr::property(int prop, uint32 value) {
	uint32 res = 0;
	value &= 0xffff;

	switch (prop) {
	case kPropVolume:
		res = _globalVolume;
		if (value != 0xffff) {
			_globalVolume = value;
			for (int i = 0; i < _numChannels; ++i)
				updateChannelVolume(i);
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

	for (int i = 0; i < _numChannels; ++i) {
		_smpVolume[i] = 0;
		_chanVolume[i] = 96;
		_chanVelocity[i] = 0;
		_notes[i] = 0xFF;
		_partMapping[i] = 0x10;
		_envState[i] = 0;
		_envCount[i] = 0;
		updateChannelVolume(i);
	}

	if (_version == SCI_VERSION_0_EARLY) {
		byte chanFlag = _pcsMode ? 0x04 : 0x02;
		for (int i = 0, numAssigned = 0; i < 16 && numAssigned < _numChannels; ++i) {
			uint8 f = header.getInt8At(++readPos);
			if ((!(f & 8) || (f & 1)) && (f & chanFlag))
				_partMapping[numAssigned++] = i;
		}
	} else {
		byte chanFlag = _pcsMode ? 0x20 : 0x10;
		for (int i = 0, numAssigned = 0; i < 16 && numAssigned < _numChannels; ++i) {
			uint8 f = header.getInt8At(++readPos);
			readPos++;
			if (f & chanFlag)
				_partMapping[numAssigned++] = i;
		}
	}
}

void MidiDriver_PCJr::generateSamples(int16 *data, int len) {
	int freq[kMaxChannels];
	int frequency = getRate();

	for (int chan = 0; chan < _numChannels; chan++)
		freq[chan] = get_freq(_notes[chan]);

	for (int i = 0; i < len; i++) {
		if (!--_sndUpdateCountDown) {
			_sndUpdateCountDown = _sndUpdateSmpQty;
			_sndUpdateCountDownRem += _sndUpdateSmpQtyRem;
			while (_sndUpdateCountDownRem >= (_sndUpdateSmpQty << 16)) {
				_sndUpdateCountDownRem -= (_sndUpdateSmpQty << 16);
				++_sndUpdateCountDown;
			}
			updateSounds();
		}

		int16 result = 0;

		for (int chan = 0; chan < _numChannels; chan++)
			if (_notes[chan] != 0xFF) {
				uint16 volume = _smpVolume[chan];
				_freq_count[chan] += freq[chan];
				while (_freq_count[chan] >= (frequency << 1))
					_freq_count[chan] -= (frequency << 1);

				if (_freq_count[chan] - freq[chan] < 0) {
					// Unclean rising edge
					int l = volume << 1;
					result += (int16)~volume + (l * _freq_count[chan]) / freq[chan];
				} else if (_freq_count[chan] >= frequency && _freq_count[chan] - freq[chan] < frequency) {
					// Unclean falling edge
					int l = volume << 1;
					result += volume - (l * (_freq_count[chan] - frequency)) / freq[chan];
				} else {
					if (_freq_count[chan] < frequency)
						result += volume;
					else
						result += (int16)~volume;
				}
			}
		data[i] = result;
	}
}

MidiDriver_PCJr::MidiDriver_PCJr(Audio::Mixer *mixer, SciVersion version, bool pcsMode) : MidiDriver_Emulated(mixer), _version(version), _pcsMode(pcsMode),
	_numChannels(pcsMode ? 1 : 3), _globalVolume(0), _instrumentOffsets(nullptr), _instrumentData(nullptr), _smpVolTable(nullptr) {
	for (int i = 0; i < kMaxChannels; ++i) {
		_chanVolume[i] = 0;
		_chanVelocity[i] = 0;
		_chanEnvVolume[i] = 0;
		_smpVolume[i] = 0;
		_notes[i] = 0;
		_envState[i] = 0;
		_envCount[i] = 0;
		_envCount2[i] = 0;
		_envAttn[i] = 0;
		_freq_count[i] = 0;
		_partMapping[i] = 0;
		_duration[i] = 0;
		_releaseDuration[i] = 0;
		_chanSustain[i] = 0;
		_chanRelease[i] = 0;
		_envData[i] = nullptr;
	}

	for (int i = 0; i < 16; ++i) {
		_chanMapping[i] = 0;
		_chanMissing[i] = 0;
		_program[i] = 0;
		_partSustain[i] = 0;
	}

	uint16 *smpVolTable = new uint16[16]();
	for (int i = 0; i < 15; ++i) // The last entry is left at zero.
		smpVolTable[i] = (double)((32767 & ~_numChannels) / _numChannels) / pow(10.0, (double)i / 10.0);
	_smpVolTable = smpVolTable;

	_sndUpdateSmpQty = (mixer->getOutputRate() << 16) / 0x3C0000;
	_sndUpdateSmpQtyRem = (mixer->getOutputRate() << 16) % 0x3C0000;
	_sndUpdateCountDown = _sndUpdateSmpQty;
	_sndUpdateCountDownRem = 0;
};

MidiDriver_PCJr::~MidiDriver_PCJr() {
	close();
	delete[] _smpVolTable;
	delete[] _instrumentOffsets;
	delete[] _instrumentData;
}

int MidiDriver_PCJr::open() {
	if (_isOpen)
		return MERR_ALREADY_OPEN;

	_globalVolume = 15;
	for (int i = 0; i < _numChannels; i++) {
		_smpVolume[i] = 0;
		_chanVolume[i] = 15;
		_chanVelocity[i] = 0;
		_chanEnvVolume[i] = 15;
		_partMapping[i] = 0xFF;
		_notes[i] = 0xFF;
		_freq_count[i] = 0;
		_envState[i] = 0;
		_envCount[i] = 0;
		updateChannelVolume(i);
	}

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

void MidiDriver_PCJr::updateChannelVolume(int chan) {
	assert(chan >= 0 && chan < kMaxChannels);
	int attn = 0;

	if (_pcsMode) {
		// The PC speaker has a fixed volume level. The original will turn it off if the volume is zero
		// or otherwise turn it on. We just use the PCJr volume table for the master volume, so that our
		// volume controls still work.
		attn = 15 - _globalVolume;
	} else if (_version <= SCI_VERSION_0_LATE) {
		int veloAttn = 3 - (_chanVelocity[chan] >> 5);
		int volAttn = 15 - (_chanVolume[chan] >> 3);
		attn = volAttn + veloAttn + _chanEnvVolume[chan];
		if (attn >= 15) {
			attn = 15;
			if (_envState[chan] >= 2)
				chanOff(chan);
		} else if (attn < 0) {
			attn = 0;
		}
		attn = CLIP<int>(attn + (15 - _globalVolume), volAttn, 15);
	} else {
		static const byte veloTable[16] = { 0x01, 0x03, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F };
		int velo = _chanVelocity[chan] ? veloTable[_chanVelocity[chan] >> 3] : 0;
		int vol = _chanVolume[chan] ? MAX<int>(1, _chanVolume[chan] >> 3) : 0;
		int tl = ((vol * velo / 15) * _chanEnvVolume[chan] / 15) * _globalVolume;
		if (tl > 0 && tl < 15)
			tl = 15;
		attn = 15 - tl / 15;
	}

	_smpVolume[chan] = _smpVolTable[attn];
}

void MidiDriver_PCJr::updateSounds() {
	if (_pcsMode)
		return;

	for (int i = 0; i < _numChannels; i++) {
		if (_notes[i] == 0xFF)
			continue;

		if (_version > SCI_VERSION_0_LATE) {
			++_duration[i];
			if (_chanRelease[i])
				++_releaseDuration[i];
			envINST(i);
			updateChannelVolume(i);
		} else {
			switch (_envState[i]) {
			case 1:
				envAT(i);
				break;
			case 2:
				envST(i);
				break;
			case 3:
				envRL(i);
				break;
			default:
				break;
			}
		}
	}
}

void MidiDriver_PCJr::envAT(byte chan) {
	static const int8 envTable[] = { 0, -2, -3, -2, -1, 0, 127 };
	byte c = _envCount[chan]++;
	if (envTable[c] == 127) {
		_envState[chan]++;
		_envCount[chan] = 0;
		_envCount2[chan] = 20;
		envST(chan);
	} else {
		_chanEnvVolume[chan] = _envAttn[chan] = envTable[c];
		updateChannelVolume(chan);
	}
}

void MidiDriver_PCJr::envST(byte chan) {
	if (!_envCount2[chan] || --_envCount2[chan])
		return;
	_chanEnvVolume[chan] = ++_envAttn[chan];
	_envCount2[chan] = 20;
	updateChannelVolume(chan);
}

void MidiDriver_PCJr::envRL(byte chan) {
	static const byte envTable[] = { 1, 1, 1, 2, 2, 3, 3, 4, 4, 6, 6, 7, 8, 9, 10, 11, 12, 127 };
	byte c = _envCount[chan]++;
	_chanEnvVolume[chan] = (envTable[c] == 127) ? 15 : _envAttn[chan] + envTable[c];
	updateChannelVolume(chan);
}

void MidiDriver_PCJr::envINST(byte chan) {
	if (_envCount[chan] == 0xFE) {
		if (_chanRelease[chan])
			_envCount[chan] = 0;
		else
			return;
	}

	if (_envCount[chan] == 0) {
		byte a = _envData[chan][_envState[chan] << 1];
		if (a == 0xFF) {
			chanOff(chan);
			_envCount[chan] = 0;
		} else {
			_envCount[chan] = _envData[chan][(_envState[chan] << 1) + 1];
			_chanEnvVolume[chan] = a;
			++_envState[chan];
		}
	} else {
		--_envCount[chan];
	}
}

void MidiDriver_PCJr::chanNoteOn(byte chan, byte note, byte velocity) {
	_notes[chan] = note;
	_chanVelocity[chan] = velocity;
	_envState[chan] = _version > SCI_VERSION_0_LATE ? 0 : 1;
	_envCount[chan] = 0;
	_duration[chan] = 0;
	_releaseDuration[chan] = 0;
	_chanRelease[chan] = 0;
	_chanSustain[chan] = 0;

	if (_pcsMode)
		return;

	if (_version <= SCI_VERSION_0_LATE) {
		envAT(chan);
	} else {
		assert(_instrumentOffsets);
		assert(_instrumentData);
		_envData[chan] = &_instrumentData[_instrumentOffsets[_program[_partMapping[chan]]]];
	}
}

void MidiDriver_PCJr::chanOff(byte chan) {
	_notes[chan] = 0xFF;
	_duration[chan] = 0;
	_releaseDuration[chan] = 0;
	_chanSustain[chan] = 0;
	_chanRelease[chan] = 0;
	_envState[chan] = 0;
	_envAttn[chan] = 0;
	_chanEnvVolume[chan] = 0;
	_envCount[chan] = 0;
}

void MidiDriver_PCJr::noteOn(byte part, byte note, byte velocity) {
	if (_pcsMode) {
		if (_partMapping[0] != part)
			return;
		chanOff(0);
		if (note < 24 || note > 119)
			return;
		chanNoteOn(0, note, velocity);
		return;
	} else if (_version > SCI_VERSION_0_LATE) {
		if (note < 21 || note > 116)
			return;

		for (int i = 0; i < _numChannels; ++i) {
			if (_partMapping[i] != part || _notes[i] != note)
				continue;
			chanOff(i);
			chanNoteOn(i, note, velocity);
			return;
		}
	}

	byte c = allocateChannel(part);
	if (c != 0xFF)
		chanNoteOn(c, note, velocity);
}

void MidiDriver_PCJr::noteOff(byte part, byte note) {
	for (int i = 0; i < _numChannels; ++i) {
		if (_partMapping[i] != part || _notes[i] != note)
			continue;

		if (_pcsMode) {
			chanOff(i);
		} else if (_version > SCI_VERSION_0_LATE) {
			if (_partSustain[part])
				_chanSustain[i] = 1;
			else
				_chanRelease[i] = 1;
		} else {
			_envState[i] = 3;
			_envCount[i] = 0;
			envRL(i);
		}
	}	
}

void MidiDriver_PCJr::controlChange(byte part, byte controller, byte value) {
	switch (controller) {
	case 7:
		for (int i = 0; i < _numChannels; ++i) {
			if (_partMapping[i] == part)
				_chanVolume[i] = value;
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
			if (_partMapping[i] == part)
				chanOff(i);
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
}

void MidiDriver_PCJr::controlChangeSustain(byte part, byte sus) {
	if (_version <= SCI_VERSION_0_LATE || _pcsMode)
		return;

	_partSustain[part] = sus;
	if (sus)
		return;

	for (int i = 0; i < _numChannels; ++i) {
		if (_partMapping[i] != part || !_chanSustain[i])
			continue;
		_chanSustain[i] = 0;
		_chanRelease[i] = 1;
	}
}

void MidiDriver_PCJr::controlChangePolyphony(byte part, byte numChan) {
	if (_version <= SCI_VERSION_0_LATE)
		return;

	if (_pcsMode) {
		if (numChan == 0 || _partMapping[0] != part)
			chanOff(0);
		_partMapping[0] = numChan ? part : 0xFF;
		return;
	}

	uint8 numAssigned = 0;
	for (int i = 0; i < _numChannels; ++i) {
		if (_partMapping[i] == part)
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
		if (_partMapping[i] != 0xFF)
			continue;
		_partMapping[i] = part;

		if (_notes[i] != 0xFF)
			chanOff(i);

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
		if (_partMapping[i] != part || _notes[i] != 0xFF)
			continue;
		_partMapping[i] = 0xFF;
		if (!--num)
			return;
	}

	do {
		uint16 oldest = 0;
		byte dropCh = 0;
		for (int i = 0; i < _numChannels; i++) {
			if (_partMapping[i] != part)
				continue;

			uint16 ct = _releaseDuration[i] ? _releaseDuration[i] + 0x8000 : _duration[i];

			if (ct >= oldest) {
				dropCh = i;
				oldest = ct;
			}
		}

		chanOff(dropCh);
		_partMapping[dropCh] = 0xFF;
	} while (--num);
}

void MidiDriver_PCJr::assignFreeChannels(byte part) {
	uint8 freeChan = 0;
	for (int i = 0; i < _numChannels; i++) {
		if (_partMapping[i] == 0xff)
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
			if (_partMapping[i] == part)
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

		if (_partMapping[c] != part)
			continue;

		if (_notes[c] == 0xFF) {
			_chanMapping[part] = c;
			return c;
		}

		uint16 ct = _releaseDuration[c] ? _releaseDuration[c] + 0x8000 : _duration[c];
		if (ct < oldest)
			continue;

		res = c;
		oldest = ct;		
	}

	if (oldest != 0) {
		_chanMapping[part] = res;
		chanOff(res);
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
};

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
