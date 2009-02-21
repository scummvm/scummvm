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

/* Tandy/PCJr sequencer for FreeSCI */

#include "sci/sfx/softseq.h"
#include "sci/include/sci_midi.h"

namespace Sci {

#define FREQUENCY 44100
#define CHANNELS_NR 3
#define VOLUME_SHIFT 3

static int global_volume = 100; /* Base volume */
static int volumes[CHANNELS_NR] = { 100, 100, 100 };
static int notes[CHANNELS_NR] = {0, 0, 0}; /* Current halftone, or 0 if off */
static int freq_count[CHANNELS_NR] = {0, 0, 0};
static int channel_assigner = 0;
static int channels_assigned = 0;
static int chan_nrs[CHANNELS_NR] = { -1, -1, -1};

extern sfx_softseq_t sfx_softseq_pcspeaker;
/* Forward-declare the sequencer we are defining here */


static int
SN76496_set_option(sfx_softseq_t *self, const char *name, const char *value) {
	return SFX_ERROR;
}

static int
SN76496_init(sfx_softseq_t *self, byte *patch, int patch_len, byte *patch2,
             int patch2_len) {
	return SFX_OK;
}

static void
SN76496_exit(sfx_softseq_t *self) {
}

static void
SN76496_event(sfx_softseq_t *self, byte command, int argc, byte *argv) {
	int i;
	int chan = -1;
#if 0
	error("Note [%02x : %02x %02x]\n", command,  argc ? argv[0] : 0, (argc > 1) ? argv[1] : 0);
#endif
	if ((command & 0xe0) == 0x80) {
		int chan_nr = command & 0xf;

		/* First, test for channel having been assigned already */
		if (channels_assigned & (1 << chan_nr)) {
			/* Already assigned this channel number: */
			for (i = 0; i < CHANNELS_NR; i++)
				if (chan_nrs[i] == chan_nr) {
					chan = i;
					break;
				}
		} else {
			/* Assign new channel round-robin */

			/* Mark channel as unused: */
			if (chan_nrs[channel_assigner] >= 0)
				channels_assigned &= ~(1 << chan_nrs[channel_assigner]);

			/* Remember channel: */
			chan_nrs[channel_assigner] = chan_nr;
			/* Mark channel as used */
			channels_assigned |= (1 << chan_nrs[channel_assigner]);

			/* Save channel for use later in this call: */
			chan = channel_assigner;
			/* Round-ropin iterate channel assigner: */
			channel_assigner = (channel_assigner + 1) % CHANNELS_NR;
		}
	}
#if 0
	error(" --> %d [%04x], {%d,%d,%d}@%d\n", chan,
	        channels_assigned, chan_nrs[0], chan_nrs[1], chan_nrs[2], channel_assigner);
#endif

	switch (command & 0xf0) {

	case 0x80:
		if (argv[0] == notes[chan])
			notes[chan] = 0;
		break;

	case 0x90:
		if (!argv[1]) {
			if (argv[chan] == notes[chan])
				notes[chan] = 0;
		} else {
			notes[chan] = argv[0];
			volumes[chan] = argv[1];
		}
		break;

	case 0xb0:
		if (argv[1] == SCI_MIDI_CHANNEL_NOTES_OFF)
			notes[chan] = 0;
		break;


	default:
#if DEBUG
		error("[SFX:PCM-PC] Unused MIDI command %02x %02x %02x\n", command, argc ? argv[0] : 0, (argc > 1) ? argv[1] : 0);
#endif
		break; /* ignore */
	}
}

#define BASE_NOTE 129	/* A10 */
#define BASE_OCTAVE 10	/* A10, as I said */

static int
freq_table[12] = { /* A4 is 440Hz, halftone map is x |-> ** 2^(x/12) */
	28160, /* A10 */
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

static inline int
get_freq(int note) {
	int halftone_delta = note - BASE_NOTE;
	int oct_diff = ((halftone_delta + BASE_OCTAVE * 12) / 12) - BASE_OCTAVE;
	int halftone_index = (halftone_delta + (12 * 100)) % 12 ;
	int freq = (!note) ? 0 : freq_table[halftone_index] / (1 << (-oct_diff));

	return freq;
}


void
SN76496_poll(sfx_softseq_t *self, byte *dest, int len) {
	gint16 *buf = (gint16 *) dest;
	int i;
	int chan;
	int freq[CHANNELS_NR];

	for (chan = 0; chan < CHANNELS_NR; chan++)
		freq[chan] = get_freq(notes[chan]);

	for (i = 0; i < len; i++) {
		int result = 0;

		for (chan = 0; chan < CHANNELS_NR; chan++)
			if (notes[chan]) {
				int volume = (global_volume * volumes[chan])
				             >> VOLUME_SHIFT;

				freq_count[chan] += freq[chan];
				while (freq_count[chan] >= (FREQUENCY << 1))
					freq_count[chan] -= (FREQUENCY << 1);

				if (freq_count[chan] - freq[chan] < 0) {
					/* Unclean rising edge */
					int l = volume << 1;
					result += -volume + (l * freq_count[chan]) / freq[chan];
				} else if (freq_count[chan] >= FREQUENCY
				           && freq_count[chan] - freq[chan] < FREQUENCY) {
					/* Unclean falling edge */
					int l = volume << 1;
					result += volume - (l * (freq_count[chan] - FREQUENCY)) / freq[chan];
				} else {
					if (freq_count[chan] < FREQUENCY)
						result += volume;
					else
						result += -volume;
				}
			}
		buf[i] = result;
	}

}

void
SN76496_allstop(sfx_softseq_t *self) {
	int i;
	for (i = 0; i < CHANNELS_NR; i++)
		notes[i] = 0;
}

void
SN76496_volume(sfx_softseq_t *self, int new_volume) {
	global_volume = new_volume;
}


sfx_softseq_t sfx_softseq_SN76496 = {
	"SN76496",
	"0.1",
	SN76496_set_option,
	SN76496_init,
	SN76496_exit,
	SN76496_volume,
	SN76496_event,
	SN76496_poll,
	SN76496_allstop,
	NULL,
	SFX_SEQ_PATCHFILE_NONE,
	SFX_SEQ_PATCHFILE_NONE,
	0x10,  /* Tandy/PCJr channels */
	0, /* No rhythm channel */
	3, /* # of voices */
	{FREQUENCY, SFX_PCM_MONO, SFX_PCM_FORMAT_S16_NATIVE}
};

} // End of namespace Sci
