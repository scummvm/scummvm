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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sound/softsynth/emumidi.h"
#include "sci/sfx/sci_midi.h"

#include "common/file.h"
#include "common/frac.h"
#include "common/util.h"

namespace Sci {

/* #define DEBUG */

// Frequencies for every note
// FIXME Store only one octave
static const int freq_table[] = {
	58, 62, 65, 69, 73, 78, 82, 87,
	92, 98, 104, 110, 117, 124, 131, 139,
	147, 156, 165, 175, 185, 196, 208, 220,
	234, 248, 262, 278, 294, 312, 331, 350,
	371, 393, 417, 441, 468, 496, 525, 556,
	589, 625, 662, 701, 743, 787, 834, 883,
	936, 992, 1051, 1113, 1179, 1250, 1324, 1403,
	1486, 1574, 1668, 1767, 1872, 1984, 2102, 2227,
	2359, 2500, 2648, 2806, 2973, 3149, 3337, 3535,
	3745, 3968, 4204, 4454, 4719, 5000, 5297, 5612,
	5946, 6299, 6674, 7071, 7491, 7937, 8408, 8908,
	9438, 10000, 10594, 11224, 11892, 12599, 13348, 14142,
	14983, 15874, 16817, 17817, 18877, 20000, 21189, 22449,
	23784, 25198, 26696, 28284, 29966, 31748, 33635, 35635,
	37754, 40000, 42378, 44898, 47568, 50396, 53393, 56568,
	59932, 63496, 67271, 71271, 75509, 80000, 84757, 89796
};

class MidiDriver_Amiga : public MidiDriver_Emulated {
public:
	enum {
		kVoices = 4
	};

	MidiDriver_Amiga(Audio::Mixer *mixer) : MidiDriver_Emulated(mixer), _playSwitch(true), _masterVolume(15) { }
	virtual ~MidiDriver_Amiga() { }

	// MidiDriver
	int open();
	void close();
	void send(uint32 b);
	MidiChannel *allocateChannel() { return NULL; }
	MidiChannel *getPercussionChannel() { return NULL; }

	// AudioStream
	bool isStereo() const { return true; }
	int getRate() const { return _mixer->getOutputRate(); }

	// MidiDriver_Emulated
	void generateSamples(int16 *buf, int len);

	void setVolume(byte volume);
	void playSwitch(bool play);
	virtual uint32 property(int prop, uint32 param);

private:
	enum {
		kModeLoop = 1 << 0, // Instrument looping flag
		kModePitch = 1 << 1 // Instrument pitch changes flag
	};

	enum {
		kChannels = 10,
		kBaseFreq = 20000, // Samplerate of the instrument bank
		kPanLeft = 91,
		kPanRight = 164
	};

	struct Channel {
		int instrument;
		int volume;
		int pan;
	};

	struct Envelope {
		int length; // Phase period length in samples
		int delta; // Velocity delta per period
		int target; // Target velocity
	};

	struct Voice {
		int instrument;
		int note;
		int note_velocity;
		int velocity;
		int envelope;
		int envelope_samples; // Number of samples till next envelope event
		int decay;
		int looping;
		int hw_channel;
		frac_t offset;
		frac_t rate;
	};

	struct Instrument {
		char name[30];
		int mode;
		int size; // Size of non-looping part in bytes
		int loop_size; // Starting offset and size of loop in bytes
		int transpose; // Transpose value in semitones
		Envelope envelope[4]; // Envelope
		int8 *samples;
		int8 *loop;
	};

	struct Bank {
		char name[30];
		uint size;
		Instrument *instruments[256];
	};

	bool _playSwitch;
	int _masterVolume;
	int _frequency;
	Envelope _envDecay;
	Bank _bank; // Instrument bank

	Channel _channels[MIDI_CHANNELS];
	/* Internal channels */
	Voice _voices[kChannels];

	void setEnvelope(Voice *channel, Envelope *envelope, int phase);
	int interpolate(int8 *samples, frac_t offset);
	void playInstrument(int16 *dest, Voice *channel, int count);
	void changeInstrument(int channel, int instrument);
	void stopChannel(int ch);
	void stopNote(int ch, int note);
	void startNote(int ch, int note, int velocity);
	Instrument *readInstrument(Common::File &file, int *id);
};

void MidiDriver_Amiga::setEnvelope(Voice *channel, Envelope *envelope, int phase) {
	channel->envelope = phase;
	channel->envelope_samples = envelope[phase].length;

	if (phase == 0)
		channel->velocity = channel->note_velocity / 2;
	else
		channel->velocity = envelope[phase - 1].target;
}

int MidiDriver_Amiga::interpolate(int8 *samples, frac_t offset) {
	int x = fracToInt(offset);
	int diff = (samples[x + 1] - samples[x]) << 8;

	return (samples[x] << 8) + fracToInt(diff * (offset & FRAC_LO_MASK));
}

void MidiDriver_Amiga::playInstrument(int16 *dest, Voice *channel, int count) {
	int index = 0;
	int vol = _channels[channel->hw_channel].volume;
	Instrument *instrument = _bank.instruments[channel->instrument];

	while (1) {
		/* Available source samples until end of segment */
		frac_t lin_avail;
		int seg_end, rem, i, amount;
		int8 *samples;

		if (channel->looping) {
			samples = instrument->loop;
			seg_end = instrument->loop_size;
		} else {
			samples = instrument->samples;
			seg_end = instrument->size;
		}

		lin_avail = intToFrac(seg_end) - channel->offset;

		rem = count - index;

		/* Amount of destination samples that we will compute this iteration */
		amount = lin_avail / channel->rate;

		if (lin_avail % channel->rate)
			amount++;

		if (amount > rem)
			amount = rem;

		/* Stop at next envelope event */
		if ((channel->envelope_samples != -1) && (amount > channel->envelope_samples))
			amount = channel->envelope_samples;

		for (i = 0; i < amount; i++) {
			dest[index++] = interpolate(samples, channel->offset) * channel->velocity / 64 * channel->note_velocity * vol / (127 * 127);
			channel->offset += channel->rate;
		}

		if (channel->envelope_samples != -1)
			channel->envelope_samples -= amount;

		if (channel->envelope_samples == 0) {
			Envelope *envelope;
			int delta, target, velocity;

			if (channel->decay)
				envelope = &_envDecay;
			else
				envelope = &instrument->envelope[channel->envelope];

			delta = envelope->delta;
			target = envelope->target;
			velocity = channel->velocity - envelope->delta;

			/* Check whether we have reached the velocity target for the current phase */
			if ((delta >= 0 && velocity <= target) || (delta < 0 && velocity >= target)) {
				channel->velocity = target;

				/* Stop note after velocity has dropped to 0 */
				if (target == 0) {
					channel->note = -1;
					break;
				} else
					switch (channel->envelope) {
					case 0:
					case 2:
						/* Go to next phase */
						setEnvelope(channel, instrument->envelope, channel->envelope + 1);
						break;
					case 1:
					case 3:
						/* Stop envelope */
						channel->envelope_samples = -1;
						break;
					}
			} else {
				/* We haven't reached the target yet */
				channel->envelope_samples = envelope->length;
				channel->velocity = velocity;
			}
		}

		if (index == count)
			break;

		if (fracToInt(channel->offset) >= seg_end) {
			if (instrument->mode & kModeLoop) {
				/* Loop the samples */
				channel->offset -= intToFrac(seg_end);
				channel->looping = 1;
			} else {
				/* All samples have been played */
				channel->note = -1;
				break;
			}
		}
	}
}

void MidiDriver_Amiga::changeInstrument(int channel, int instrument) {
#ifdef DEBUG
	if (_bank.instruments[instrument])
		printf("[sfx:seq:amiga] Setting channel %i to \"%s\" (%i)\n", channel, _bank.instruments[instrument]->name, instrument);
	else
		warning("[sfx:seq:amiga] instrument %i does not exist (channel %i)", instrument, channel);
#endif
	_channels[channel].instrument = instrument;
}

void MidiDriver_Amiga::stopChannel(int ch) {
	int i;

	/* Start decay phase for note on this hw channel, if any */
	for (i = 0; i < kChannels; i++)
		if (_voices[i].note != -1 && _voices[i].hw_channel == ch && !_voices[i].decay) {
			/* Trigger fast decay envelope */
			_voices[i].decay = 1;
			_voices[i].envelope_samples = _envDecay.length;
			break;
		}
}

void MidiDriver_Amiga::stopNote(int ch, int note) {
	int channel;
	Instrument *instrument;

	for (channel = 0; channel < kChannels; channel++)
		if (_voices[channel].note == note && _voices[channel].hw_channel == ch && !_voices[channel].decay)
			break;

	if (channel == kChannels) {
#ifdef DEBUG
		warning("[sfx:seq:amiga] cannot stop note %i on channel %i", note, ch);
#endif
		return;
	}

	instrument = _bank.instruments[_voices[channel].instrument];

	/* Start the envelope phases for note-off if looping is on and envelope is enabled */
	if ((instrument->mode & kModeLoop) && (instrument->envelope[0].length != 0))
		setEnvelope(&_voices[channel], instrument->envelope, 2);
}

void MidiDriver_Amiga::startNote(int ch, int note, int velocity) {
	Instrument *instrument;
	int channel;

	if (_channels[ch].instrument < 0 || _channels[ch].instrument > 255) {
		warning("[sfx:seq:amiga] invalid instrument %i on channel %i", _channels[ch].instrument, ch);
		return;
	}

	instrument = _bank.instruments[_channels[ch].instrument];

	if (!instrument) {
		warning("[sfx:seq:amiga] instrument %i does not exist", _channels[ch].instrument);
		return;
	}

	for (channel = 0; channel < kChannels; channel++)
		if (_voices[channel].note == -1)
			break;

	if (channel == kChannels) {
		warning("[sfx:seq:amiga] could not find a free channel");
		return;
	}

	stopChannel(ch);

	if (instrument->mode & kModePitch) {
		int fnote = note + instrument->transpose;

		if (fnote < 0 || fnote > 127) {
			warning("[sfx:seq:amiga] illegal note %i\n", fnote);
			return;
		}

		/* Compute rate for note */
		_voices[channel].rate = doubleToFrac(freq_table[fnote] / (double) _frequency);
	} else
		_voices[channel].rate = doubleToFrac(kBaseFreq / (double) _frequency);

	_voices[channel].instrument = _channels[ch].instrument;
	_voices[channel].note = note;
	_voices[channel].note_velocity = velocity;

	if ((instrument->mode & kModeLoop) && (instrument->envelope[0].length != 0))
		setEnvelope(&_voices[channel], instrument->envelope, 0);
	else {
		_voices[channel].velocity = 64;
		_voices[channel].envelope_samples = -1;
	}

	_voices[channel].offset = 0;
	_voices[channel].hw_channel = ch;
	_voices[channel].decay = 0;
	_voices[channel].looping = 0;
}

MidiDriver_Amiga::Instrument *MidiDriver_Amiga::readInstrument(Common::File &file, int *id) {
	Instrument *instrument;
	byte header[61];
	int size;
	int seg_size[3];
	int loop_offset;
	int i;

	if (file.read(header, 61) < 61) {
		warning("[sfx:seq:amiga] failed to read instrument header");
		return NULL;
	}

	instrument = new Instrument;

	seg_size[0] = READ_BE_UINT16(header + 35) * 2;
	seg_size[1] = READ_BE_UINT16(header + 41) * 2;
	seg_size[2] = READ_BE_UINT16(header + 47) * 2;

	instrument->mode = header[33];
	instrument->transpose = (int8) header[34];
	for (i = 0; i < 4; i++) {
		int length = (int8) header[49 + i];

		if (length == 0 && i > 0)
			length = 256;

		instrument->envelope[i].length = length * _frequency / 60;
		instrument->envelope[i].delta = (int8)header[53 + i];
		instrument->envelope[i].target = header[57 + i];
	}
	/* Final target must be 0 */
	instrument->envelope[3].target = 0;

	loop_offset = READ_BE_UINT32(header + 37) & ~1;
	size = seg_size[0] + seg_size[1] + seg_size[2];

	*id = READ_BE_UINT16(header);

	strncpy(instrument->name, (char *) header + 2, 29);
	instrument->name[29] = 0;
#ifdef DEBUG
	printf("[sfx:seq:amiga] Reading instrument %i: \"%s\" (%i bytes)\n",
	          *id, instrument->name, size);
	printf("                Mode: %02x\n", instrument->mode);
	printf("                Looping: %s\n", instrument->mode & kModeLoop ? "on" : "off");
	printf("                Pitch changes: %s\n", instrument->mode & kModePitch ? "on" : "off");
	printf("                Segment sizes: %i %i %i\n", seg_size[0], seg_size[1], seg_size[2]);
	printf("                Segment offsets: 0 %i %i\n", loop_offset, read_int32(header + 43));
#endif
	instrument->samples = (int8 *) malloc(size + 1);
	if (file.read(instrument->samples, size) < (unsigned int)size) {
		warning("[sfx:seq:amiga] failed to read instrument samples");
		free(instrument->samples);
		free(instrument);
		return NULL;
	}

	if (instrument->mode & kModeLoop) {
		if (loop_offset + seg_size[1] > size) {
#ifdef DEBUG
			warning("[sfx:seq:amiga] looping samples extend %i bytes past end of sample block",
			          loop_offset + seg_size[1] - size);
#endif
			seg_size[1] = size - loop_offset;
		}

		if (seg_size[1] < 0) {
			warning("[sfx:seq:amiga] invalid looping point");
			free(instrument->samples);
			free(instrument);
			return NULL;
		}

		instrument->size = seg_size[0];
		instrument->loop_size = seg_size[1];

		instrument->loop = (int8*)malloc(instrument->loop_size + 1);
		memcpy(instrument->loop, instrument->samples + loop_offset, instrument->loop_size);

		instrument->samples[instrument->size] = instrument->loop[0];
		instrument->loop[instrument->loop_size] = instrument->loop[0];
	} else {
		instrument->loop = NULL;
		instrument->size = size;
		instrument->samples[instrument->size] = 0;
	}

	return instrument;
}

uint32 MidiDriver_Amiga::property(int prop, uint32 param) {
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

int MidiDriver_Amiga::open() {
	_frequency = _mixer->getOutputRate();
	_envDecay.length = _frequency / (32 * 64);
	_envDecay.delta = 1;
	_envDecay.target = 0;

	Common::File file;
	byte header[40];

	if (!file.open("bank.001")) {
		warning("[sfx:seq:amiga] file bank.001 not found");
		return Common::kUnknownError;
	}

	if (file.read(header, 40) < 40) {
		warning("[sfx:seq:amiga] failed to read header of file bank.001");
		return Common::kUnknownError;
	}

	for (uint i = 0; i < 256; i++)
		_bank.instruments[i] = NULL;

	for (uint i = 0; i < kChannels; i++) {
		_voices[i].note = -1;
		_voices[i].hw_channel = 0;
	}

	for (uint i = 0; i < MIDI_CHANNELS; i++) {
		_channels[i].instrument = -1;
		_channels[i].volume = 127;
		_channels[i].pan = (i % 4 == 0 || i % 4 == 3 ? kPanLeft : kPanRight);
	}

	_bank.size = READ_BE_UINT16(header + 38);
	strncpy(_bank.name, (char *) header + 8, 29);
	_bank.name[29] = 0;
#ifdef DEBUG
	printf("[sfx:seq:amiga] Reading %i instruments from bank \"%s\"\n", _bank.size, _bank.name);
#endif

	for (uint i = 0; i < _bank.size; i++) {
		int id;
		Instrument *instrument = readInstrument(file, &id);

		if (!instrument) {
			warning("[sfx:seq:amiga] failed to read bank.001");
			return Common::kUnknownError;
		}

		if (id < 0 || id > 255) {
			warning("[sfx:seq:amiga] Error: instrument ID out of bounds");
			return Common::kUnknownError;
		}

		_bank.instruments[id] = instrument;
	}

	MidiDriver_Emulated::open();

	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_mixerSoundHandle, this, -1, _mixer->kMaxChannelVolume, 0, false);

	return Common::kNoError;
}

void MidiDriver_Amiga::close() {
	_mixer->stopHandle(_mixerSoundHandle);

	for (uint i = 0; i < _bank.size; i++) {
		if (_bank.instruments[i]) {
			if (_bank.instruments[i]->loop)
				free(_bank.instruments[i]->loop);
			free(_bank.instruments[i]->samples);
			delete _bank.instruments[i];
		}
	}
}

void MidiDriver_Amiga::playSwitch(bool play) {
	_playSwitch = play;
}

void MidiDriver_Amiga::setVolume(byte volume_) {
	_masterVolume = volume_;
}

void MidiDriver_Amiga::send(uint32 b) {
	byte command = b & 0xf0;
	byte channel = b & 0xf;
	byte op1 = (b >> 8) & 0xff;
	byte op2 = (b >> 16) & 0xff;

	switch (command) {
	case 0x90:
		if (op2 > 0)
			startNote(channel, op1, op2);
		else
			stopNote(channel, op1);
		break;
	case 0xb0:
		switch (op1) {
		case 0x07:
			_channels[channel].volume = op2;
			break;
		case 0x0a:
#ifdef DEBUG
			warning("[sfx:seq:amiga] ignoring pan 0x%02x event for channel %i", op2, channel);
#endif
			break;
		case 0x7b:
			stopChannel(channel);
			break;
		default:
			warning("[sfx:seq:amiga] unknown control event 0x%02x", op1);
		}
		break;
	case 0xc0:
		changeInstrument(channel, op1);
		break;
	default:
		warning("[sfx:seq:amiga] unknown event %02x", command);
	}
}

void MidiDriver_Amiga::generateSamples(int16 *data, int len) {
	if (len == 0)
		return;

	int16 *buffers = (int16*)malloc(len * 2 * kChannels);

	memset(buffers, 0, len * 2 * kChannels);

	/* Generate samples for all notes */
	for (int i = 0; i < kChannels; i++)
		if (_voices[i].note >= 0)
			playInstrument(buffers + i * len, &_voices[i], len);

	if (isStereo()) {
		for (int j = 0; j < len; j++) {
			int mixedl = 0, mixedr = 0;

			/* Mix and pan */
			for (int i = 0; i < kChannels; i++) {
				mixedl += buffers[i * len + j] * (256 - _channels[_voices[i].hw_channel].pan);
				mixedr += buffers[i * len + j] * _channels[_voices[i].hw_channel].pan;
			}

			/* Adjust volume */
			data[2 * j] = mixedl * _masterVolume >> 13;
			data[2 * j + 1] = mixedr * _masterVolume >> 13;
		}
	} else {
		for (int j = 0; j < len; j++) {
			int mixed = 0;

			/* Mix */
			for (int i = 0; i < kChannels; i++)
				mixed += buffers[i * len + j];

			/* Adjust volume */
			data[j] = mixed * _masterVolume >> 6;
		}
	}

	free(buffers);
}

class MidiPlayer_Amiga : public MidiPlayer {
public:
	MidiPlayer_Amiga() { _driver = new MidiDriver_Amiga(g_system->getMixer()); }
	int getPlayMask() const { return 0x40; }
	int getPolyphony() const { return MidiDriver_Amiga::kVoices; }
	bool hasRhythmChannel() const { return false; }
	void setVolume(byte volume) { static_cast<MidiDriver_Amiga *>(_driver)->setVolume(volume); }
	void playSwitch(bool play) { static_cast<MidiDriver_Amiga *>(_driver)->playSwitch(play); }
	void loadInstrument(int idx, byte *data);
};

MidiPlayer *MidiPlayer_Amiga_create() {
	return new MidiPlayer_Amiga();
}

} // End of namespace Sci
