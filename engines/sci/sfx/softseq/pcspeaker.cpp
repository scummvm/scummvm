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

/* PC speaker software sequencer for SCI */

#include "sci/sfx/softseq.h"
#include "sci/sfx/sci_midi.h"

namespace Sci {

#define FREQUENCY 94020

static int volume = 0x0600;
static int note = 0; /* Current halftone, or 0 if off */
static int freq_count = 0;

extern sfx_softseq_t sfx_softseq_pcspeaker;
/* Forward-declare the sequencer we are defining here */


static int sps_set_option(sfx_softseq_t *self, const char *name, const char *value) {
	return SFX_ERROR;
}

static int sps_init(sfx_softseq_t *self, byte *patch, int patch_len, byte *patch2,
	int patch2_len) {
	return SFX_OK;
}

static void sps_exit(sfx_softseq_t *self) {
}

static void sps_event(sfx_softseq_t *self, byte command, int argc, byte *argv) {
#if 0
	fprintf(stderr, "Note [%02x : %02x %02x]\n", command,  argc ? argv[0] : 0, (argc > 1) ? argv[1] : 0);
#endif

	switch (command & 0xf0) {

	case 0x80:
		if (argv[0] == note)
			note = 0;
		break;

	case 0x90:
		if (!argv[1]) {
			if (argv[0] == note)
				note = 0;
		} else
			note = argv[0];
		/* Ignore velocity otherwise; we just use the global one */
		break;

	case 0xb0:
		if (argv[1] == SCI_MIDI_CHANNEL_NOTES_OFF)
			note = 0;
		break;

	default:
#if DEBUG
		fprintf(stderr, "[SFX:PCM-PC] Unused MIDI command %02x %02x %02x\n", command, argc ? argv[0] : 0, (argc > 1) ? argv[1] : 0);
#endif
		break; /* ignore */
	}
}

#define BASE_NOTE 129	/* A10 */
#define BASE_OCTAVE 10	/* A10, as I said */

static int freq_table[12] = { /* A4 is 440Hz, halftone map is x |-> ** 2^(x/12) */
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


void sps_poll(sfx_softseq_t *self, byte *dest, int len) {
	int halftone_delta = note - BASE_NOTE;
	int oct_diff = ((halftone_delta + BASE_OCTAVE * 12) / 12) - BASE_OCTAVE;
	int halftone_index = (halftone_delta + (12 * 100)) % 12 ;
	int freq = (!note) ? 0 : freq_table[halftone_index] / (1 << (-oct_diff));
	int16 *buf = (int16 *) dest;

	int i;
	for (i = 0; i < len; i++) {
		if (note) {
			freq_count += freq;
			while (freq_count >= (FREQUENCY << 1))
				freq_count -= (FREQUENCY << 1);

			if (freq_count - freq < 0) {
				/* Unclean rising edge */
				int l = volume << 1;
				buf[i] = -volume + (l * freq_count) / freq;
			} else if (freq_count >= FREQUENCY
			           && freq_count - freq < FREQUENCY) {
				/* Unclean falling edge */
				int l = volume << 1;
				buf[i] = volume - (l * (freq_count - FREQUENCY)) / freq;
			} else {
				if (freq_count < FREQUENCY)
					buf[i] = volume;
				else
					buf[i] = -volume;
			}
		} else
			buf[i] = 0;
	}

}

void sps_volume(sfx_softseq_t *self, int new_volume) {
	volume = new_volume << 4;
}

void sps_allstop(sfx_softseq_t *self) {
	note = 0;
}

sfx_softseq_t sfx_softseq_pcspeaker = {
	"pc-speaker",
	"0.3",
	sps_set_option,
	sps_init,
	sps_exit,
	sps_volume,
	sps_event,
	sps_poll,
	sps_allstop,
	NULL,
	SFX_SEQ_PATCHFILE_NONE,
	SFX_SEQ_PATCHFILE_NONE,
	0x20,  /* PC speaker channel only */
	0, /* No rhythm channel */
	1, /* # of voices */
	{FREQUENCY, SFX_PCM_MONO, SFX_PCM_FORMAT_S16_NATIVE}
};

} // End of namespace Sci
