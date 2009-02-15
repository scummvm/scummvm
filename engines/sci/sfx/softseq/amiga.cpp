/***************************************************************************
 amiga.c  Copyright (C) 2007 Walter van Niftrik


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl>

***************************************************************************/

#include "sci/include/resource.h"
#include "sci/include/sci_memory.h"
#include "sci/sfx/softseq.h"

#include "common/frac.h"

#define FREQUENCY 44100
#define CHANNELS_NR 10
#define HW_CHANNELS_NR 16

/* Samplerate of the instrument bank */
#define BASE_FREQ 20000

/* Instrument looping flag */
#define MODE_LOOP 1 << 0
/* Instrument pitch changes flag */
#define MODE_PITCH 1 << 1

#define PAN_LEFT 91
#define PAN_RIGHT 164

/* #define DEBUG */

typedef struct envelope {
	/* Phase period length in samples */
	int length;
	/* Velocity delta per period */
	int delta;
	/* Target velocity */
	int target;
} envelope_t;

/* Fast decay envelope */
static envelope_t env_decay = {FREQUENCY / (32 * 64), 1, 0};

typedef struct instrument {
	char name[30];
	int mode;
	/* Size of non-looping part in bytes */
	int size;
	/* Starting offset and size of loop in bytes */
	int loop_size;
	/* Transpose value in semitones */
	int transpose;
	/* Envelope */
	envelope_t envelope[4];
	sbyte *samples;
	sbyte *loop;
} instrument_t;

typedef struct bank {
	char name[30];
	int size;
	instrument_t *instruments[256];
} bank_t;

typedef struct channel {
	int instrument;
	int note;
	int note_velocity;
	int velocity;
	int envelope;
	/* Number of samples till next envelope event */
	int envelope_samples;
	int decay;
	int looping;
	int hw_channel;
	frac_t offset;
	frac_t rate;
} channel_t;

typedef struct hw_channel {
	int instrument;
	int volume;
	int pan;
} hw_channel_t;

/* Instrument bank */
static bank_t bank;
/* Internal channels */
static channel_t channels[CHANNELS_NR];
/* External channels */
static hw_channel_t hw_channels[HW_CHANNELS_NR];
/* Overall volume */
static int volume = 127;

/* Frequencies for every note */
static int freq_table[] = {
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

static void
set_envelope(channel_t *channel, envelope_t *envelope, int phase) {
	channel->envelope = phase;
	channel->envelope_samples = envelope[phase].length;

	if (phase == 0)
		channel->velocity = channel->note_velocity / 2;
	else
		channel->velocity = envelope[phase - 1].target;
}

static inline int
interpolate(sbyte *samples, frac_t offset) {
	int x = fracToInt(offset);
	int diff = (samples[x + 1] - samples[x]) << 8;

	return (samples[x] << 8) + fracToInt(diff * (offset & FRAC_LO_MASK));
}

static void
play_instrument(gint16 *dest, channel_t *channel, int count) {
	int index = 0;
	int vol = hw_channels[channel->hw_channel].volume;
	instrument_t *instrument = bank.instruments[channel->instrument];

	while (1) {
		/* Available source samples until end of segment */
		frac_t lin_avail;
		int seg_end, rem, i, amount;
		sbyte *samples;

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
			envelope_t *envelope;
			int delta, target, velocity;

			if (channel->decay)
				envelope = &env_decay;
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
						set_envelope(channel, instrument->envelope, channel->envelope + 1);
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
			if (instrument->mode & MODE_LOOP) {
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

static void
change_instrument(int channel, int instrument) {
#ifdef DEBUG
	if (bank.instruments[instrument])
		sciprintf("[sfx:seq:amiga] Setting channel %i to \"%s\" (%i)\n", channel, bank.instruments[instrument]->name, instrument);
	else
		sciprintf("[sfx:seq:amiga] Warning: instrument %i does not exist (channel %i)\n", instrument, channel);
#endif
	hw_channels[channel].instrument = instrument;
}

static void
stop_channel(int ch) {
	int i;

	/* Start decay phase for note on this hw channel, if any */
	for (i = 0; i < CHANNELS_NR; i++)
		if (channels[i].note != -1 && channels[i].hw_channel == ch && !channels[i].decay) {
			/* Trigger fast decay envelope */
			channels[i].decay = 1;
			channels[i].envelope_samples = env_decay.length;
			break;
		}
}

static void
stop_note(int ch, int note) {
	int channel;
	instrument_t *instrument;

	for (channel = 0; channel < CHANNELS_NR; channel++)
		if (channels[channel].note == note && channels[channel].hw_channel == ch && !channels[channel].decay)
			break;

	if (channel == CHANNELS_NR) {
#ifdef DEBUG
		sciprintf("[sfx:seq:amiga] Warning: cannot stop note %i on channel %i\n", note, ch);
#endif
		return;
	}

	instrument = bank.instruments[channels[channel].instrument];

	/* Start the envelope phases for note-off if looping is on and envelope is enabled */
	if ((instrument->mode & MODE_LOOP) && (instrument->envelope[0].length != 0))
		set_envelope(&channels[channel], instrument->envelope, 2);
}

static void
start_note(int ch, int note, int velocity) {
	instrument_t *instrument;
	int channel;

	if (hw_channels[ch].instrument < 0 || hw_channels[ch].instrument > 255) {
		sciprintf("[sfx:seq:amiga] Error: invalid instrument %i on channel %i\n", hw_channels[ch].instrument, ch);
		return;
	}

	instrument = bank.instruments[hw_channels[ch].instrument];

	if (!instrument) {
		sciprintf("[sfx:seq:amiga] Error: instrument %i does not exist\n", hw_channels[ch].instrument);
		return;
	}

	for (channel = 0; channel < CHANNELS_NR; channel++)
		if (channels[channel].note == -1)
			break;

	if (channel == CHANNELS_NR) {
		sciprintf("[sfx:seq:amiga] Warning: could not find a free channel\n");
		return;
	}

	stop_channel(ch);

	if (instrument->mode & MODE_PITCH) {
		int fnote = note + instrument->transpose;

		if (fnote < 0 || fnote > 127) {
			sciprintf("[sfx:seq:amiga] Error: illegal note %i\n", fnote);
			return;
		}

		/* Compute rate for note */
		channels[channel].rate = doubleToFrac(freq_table[fnote] / (double) FREQUENCY);
	} else
		channels[channel].rate = doubleToFrac(BASE_FREQ / (double) FREQUENCY);

	channels[channel].instrument = hw_channels[ch].instrument;
	channels[channel].note = note;
	channels[channel].note_velocity = velocity;

	if ((instrument->mode & MODE_LOOP) && (instrument->envelope[0].length != 0))
		set_envelope(&channels[channel], instrument->envelope, 0);
	else {
		channels[channel].velocity = 64;
		channels[channel].envelope_samples = -1;
	}

	channels[channel].offset = 0;
	channels[channel].hw_channel = ch;
	channels[channel].decay = 0;
	channels[channel].looping = 0;
}

static gint16 read_int16(byte *data) {
	return (data[0] << 8) | data[1];
}

static gint32 read_int32(byte *data) {
	return (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
}

static instrument_t *read_instrument(FILE *file, int *id) {
	instrument_t *instrument;
	byte header[61];
	int size;
	int seg_size[3];
	int loop_offset;
	int i;

	if (fread(header, 1, 61, file) < 61) {
		sciprintf("[sfx:seq:amiga] Error: failed to read instrument header\n");
		return NULL;
	}

	instrument = (instrument_t *) sci_malloc(sizeof(instrument_t));

	seg_size[0] = read_int16(header + 35) * 2;
	seg_size[1] = read_int16(header + 41) * 2;
	seg_size[2] = read_int16(header + 47) * 2;

	instrument->mode = header[33];
	instrument->transpose = (gint8) header[34];
	for (i = 0; i < 4; i++) {
		int length = (gint8) header[49 + i];

		if (length == 0 && i > 0)
			length = 256;

		instrument->envelope[i].length = length * FREQUENCY / 60;
		instrument->envelope[i].delta = (gint8) header[53 + i];
		instrument->envelope[i].target = header[57 + i];
	}
	/* Final target must be 0 */
	instrument->envelope[3].target = 0;

	loop_offset = read_int32(header + 37) & ~1;
	size = seg_size[0] + seg_size[1] + seg_size[2];

	*id = read_int16(header);

	strncpy(instrument->name, (char *) header + 2, 29);
	instrument->name[29] = 0;
#ifdef DEBUG
	sciprintf("[sfx:seq:amiga] Reading instrument %i: \"%s\" (%i bytes)\n",
	          *id, instrument->name, size);
	sciprintf("                Mode: %02x\n", instrument->mode);
	sciprintf("                Looping: %s\n", instrument->mode & MODE_LOOP ? "on" : "off");
	sciprintf("                Pitch changes: %s\n", instrument->mode & MODE_PITCH ? "on" : "off");
	sciprintf("                Segment sizes: %i %i %i\n", seg_size[0], seg_size[1], seg_size[2]);
	sciprintf("                Segment offsets: 0 %i %i\n", loop_offset, read_int32(header + 43));
#endif
	instrument->samples = (sbyte *) sci_malloc(size + 1);
	if (fread(instrument->samples, 1, size, file) < (unsigned int)size) {
		sciprintf("[sfx:seq:amiga] Error: failed to read instrument samples\n");
		return NULL;
	}

	if (instrument->mode & MODE_LOOP) {
		if (loop_offset + seg_size[1] > size) {
#ifdef DEBUG
			sciprintf("[sfx:seq:amiga] Warning: looping samples extend %i bytes past end of sample block\n",
			          loop_offset + seg_size[1] - size);
#endif
			seg_size[1] = size - loop_offset;
		}

		if (seg_size[1] < 0) {
			sciprintf("[sfx:seq:amiga] Error: invalid looping point\n");
			return NULL;
		}

		instrument->size = seg_size[0];
		instrument->loop_size = seg_size[1];

		instrument->loop = (sbyte*)sci_malloc(instrument->loop_size + 1);
		memcpy(instrument->loop, instrument->samples + loop_offset, instrument->loop_size);

		instrument->samples[instrument->size] = instrument->loop[0];
		instrument->loop[instrument->loop_size] = instrument->loop[0];
	} else {
		instrument->size = size;
		instrument->samples[instrument->size] = 0;
	}

	return instrument;
}

static int
ami_set_option(sfx_softseq_t *self, const char *name, const char *value) {
	return SFX_ERROR;
}

static int
ami_init(sfx_softseq_t *self, byte *patch, int patch_len, byte *patch2, int patch2_len) {
	FILE *file;
	byte header[40];
	int i;

	file = sci_fopen("bank.001", "rb");

	if (!file) {
		sciprintf("[sfx:seq:amiga] Error: file bank.001 not found\n");
		return SFX_ERROR;
	}

	if (fread(header, 1, 40, file) < 40) {
		sciprintf("[sfx:seq:amiga] Error: failed to read header of file bank.001\n");
		fclose(file);
		return SFX_ERROR;
	}

	for (i = 0; i < 256; i++)
		bank.instruments[i] = NULL;

	for (i = 0; i < CHANNELS_NR; i++) {
		channels[i].note = -1;
	}

	for (i = 0; i < HW_CHANNELS_NR; i++) {
		hw_channels[i].instrument = -1;
		hw_channels[i].volume = 127;
		hw_channels[i].pan = (i % 4 == 0 || i % 4 == 3 ? PAN_LEFT : PAN_RIGHT);
	}

	bank.size = read_int16(header + 38);
	strncpy(bank.name, (char *) header + 8, 29);
	bank.name[29] = 0;
#ifdef DEBUG
	sciprintf("[sfx:seq:amiga] Reading %i instruments from bank \"%s\"\n", bank.size, bank.name);
#endif

	for (i = 0; i < bank.size; i++) {
		int id;
		instrument_t *instrument = read_instrument(file, &id);

		if (!instrument) {
			sciprintf("[sfx:seq:amiga] Error: failed to read bank.001\n");
			fclose(file);
			return SFX_ERROR;
		}

		if (id < 0 || id > 255) {
			sciprintf("[sfx:seq:amiga] Error: instrument ID out of bounds\n");
			return SFX_ERROR;
		}

		bank.instruments[id] = instrument;
	}

	fclose(file);

	return SFX_OK;
}

static void
ami_exit(sfx_softseq_t *self) {
	int i;

	for (i = 0; i < bank.size; i++) {
		if (bank.instruments[i]) {
			sci_free(bank.instruments[i]->samples);
			sci_free(bank.instruments[i]);
		}
	}
}

static void
ami_event(sfx_softseq_t *self, byte command, int argc, byte *argv) {
	int channel, oper;

	channel = command & 0x0f;
	oper = command & 0xf0;

	if (channel >= HW_CHANNELS_NR) {
#ifdef DEBUG
		sciprintf("[sfx:seq:amiga] Warning: received event for non-existing channel %i\n", channel);
#endif
		return;
	}

	switch (oper) {
	case 0x90:
		if (argv[1] > 0)
			start_note(channel, argv[0], argv[1]);
		else
			stop_note(channel, argv[0]);
		break;
	case 0xb0:
		switch (argv[0]) {
		case 0x07:
			hw_channels[channel].volume = argv[1];
			break;
		case 0x0a:
#ifdef DEBUG
			sciprintf("[sfx:seq:amiga] Warning: ignoring pan 0x%02x event for channel %i\n", argv[1], channel);
#endif
			break;
		case 0x7b:
			stop_channel(channel);
			break;
		default:
			sciprintf("[sfx:seq:amiga] Warning: unknown control event 0x%02x\n", argv[0]);
		}
		break;
	case 0xc0:
		change_instrument(channel, argv[0]);
		break;
	default:
		sciprintf("[sfx:seq:amiga] Warning: unknown event %02x\n", command);
	}
}

void
ami_poll(sfx_softseq_t *self, byte *dest, int len) {
	int i, j;
	gint16 *buf = (gint16 *) dest;
	gint16 *buffers = (gint16*)malloc(len * 2 * CHANNELS_NR);

	memset(buffers, 0, len * 2 * CHANNELS_NR);
	memset(dest, 0, len * 4);

	/* Generate samples for all notes */
	for (i = 0; i < CHANNELS_NR; i++)
		if (channels[i].note >= 0)
			play_instrument(buffers + i * len, &channels[i], len);

	for (j = 0; j < len; j++) {
		int mixedl = 0, mixedr = 0;

		/* Mix and pan */
		for (i = 0; i < CHANNELS_NR; i++) {
			mixedl += buffers[i * len + j] * (256 - hw_channels[channels[i].hw_channel].pan);
			mixedr += buffers[i * len + j] * hw_channels[channels[i].hw_channel].pan;
		}

		/* Adjust volume */
		buf[2 * j] = mixedl * volume >> 16;
		buf[2 * j + 1] = mixedr * volume >> 16;
	}
}

void
ami_volume(sfx_softseq_t *self, int new_volume) {
	volume = new_volume;
}

void
ami_allstop(sfx_softseq_t *self) {
	int i;
	for (i = 0; i < HW_CHANNELS_NR; i++)
		stop_channel(i);
}

sfx_softseq_t sfx_softseq_amiga = {
	"amiga",
	"0.1",
	ami_set_option,
	ami_init,
	ami_exit,
	ami_volume,
	ami_event,
	ami_poll,
	ami_allstop,
	NULL,
	SFX_SEQ_PATCHFILE_NONE,
	SFX_SEQ_PATCHFILE_NONE,
	0x40,
	0, /* No rhythm channel (9) */
	HW_CHANNELS_NR, /* # of voices */
	{FREQUENCY, SFX_PCM_STEREO_LR, SFX_PCM_FORMAT_S16_NATIVE}
};
