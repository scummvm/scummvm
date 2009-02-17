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

extern "C" {
#include "../softseq.h"
#include "../sequencer.h"
#include "../device.h"
#include "resource.h"
#include "sci_memory.h"
}

#include "mt32/mt32emu.h"

/* FIXME */
#define FREESCI_GAMEDIR ".freesci"

#define SAMPLE_RATE 32000
#define CHANNELS SFX_PCM_STEREO_LR
#define STEREO 1

static MT32Emu::Synth *synth;
static sfx_sequencer_t *mt32seq;
static int initializing;

/* MIDI writer */

static int
mt32_midi_init(struct _midi_writer *self) {
	return SFX_OK;
}

static int
mt32_midi_set_option(struct _midi_writer *self, char *name, char *value) {
	return SFX_ERROR;
}

static int
mt32_midi_write(struct _midi_writer *self, unsigned char *buf, int len) {
	if (buf[0] == 0xf0)
		synth->playSysex(buf, len);
	else if (len < 4) {
		MT32Emu::Bit32u msg;
		msg = buf[0];
		if (len > 0)
			msg |= buf[1] << 8;
		if (len > 1)
			msg |= buf[2] << 16;
		synth->playMsg(msg);
	} else
		sciprintf("MT32EMU: Skipping non-sysex message of more than 3 bytes.\n");

	return SFX_OK;
}

static void
mt32_midi_delay(struct _midi_writer *self, int ticks) {
}

static void
mt32_midi_reset_timer(struct _midi_writer *self) {
}

static void
mt32_midi_close(struct _midi_writer *self) {
}

static midi_writer_t midi_writer_mt32 = {
	"mt32",
	mt32_midi_init,
	mt32_midi_set_option,
	mt32_midi_write,
	mt32_midi_delay,
	NULL,
	mt32_midi_reset_timer,
	mt32_midi_close
};

/* Software sequencer */

static void printDebug(void *userData, const char *fmt, va_list list) {
	if (initializing) {
		vprintf(fmt, list);
		printf("\n");
	}
}

static void
mt32_poll(sfx_softseq_t *self, byte *dest, int count) {
	synth->render((MT32Emu::Bit16s *) dest, count);
}

static int
mt32_init(sfx_softseq_t *self, byte *data_ptr, int data_length, byte *data2_ptr,
          int data2_length) {
	MT32Emu::SynthProperties prop;
	char *home = sci_get_homedir();
	char *romdir;

	if (!home) {
		sciprintf("MT32EMU: Couldn't determine home directory.\n");
		return SFX_ERROR;
	}

	romdir = (char *) sci_malloc(sizeof(home) + 2 * sizeof(G_DIR_SEPARATOR_S)
	                             + sizeof(FREESCI_GAMEDIR) + 1);

	strcpy(romdir, home);
	strcat(romdir, G_DIR_SEPARATOR_S);
	strcat(romdir, FREESCI_GAMEDIR);
	strcat(romdir, G_DIR_SEPARATOR_S);

	mt32seq = sfx_find_sequencer("MT32");
	if (!mt32seq) {
		sciprintf("MT32EMU: Unable to find MT32 sequencer.\n");
		return SFX_ERROR;
	}

	prop.sampleRate = SAMPLE_RATE;
	prop.useReverb = true;
	prop.useDefaultReverb = true;
	prop.baseDir = romdir;
	prop.userData = NULL;
	prop.report = NULL;
	prop.printDebug = printDebug;
	prop.openFile = NULL;
	prop.closeFile = NULL;

	initializing = 1;
	synth = new MT32Emu::Synth();
	if (!synth->open(prop))
		return SFX_ERROR;
	initializing = 0;

	mt32seq->open(data_length, data_ptr, data2_length, data2_ptr,
	              &midi_writer_mt32);

	free(romdir);

	return SFX_OK;
}

static void
mt32_exit(sfx_softseq_t *self) {
	synth->close();
	delete synth;

	mt32seq->close();
}

static void
mt32_allstop(sfx_softseq_t *self) {
	if (mt32seq->allstop)
		mt32seq->allstop();
}

static void
mt32_volume(sfx_softseq_t *self, int volume) {
	if (mt32seq->volume)
		mt32seq->volume(volume / 2); /* High volume causes clipping. */
}

static int
mt32_set_option(sfx_softseq_t *self, const char *name, const char *value) {
	return SFX_ERROR;
}

static void
mt32_event(sfx_softseq_t *self, byte cmd, int argc, byte *argv) {
	mt32seq->event(cmd, argc, argv);
}

sfx_softseq_t sfx_softseq_mt32 = {
	"mt32emu",
	"0.1",
	mt32_set_option,
	mt32_init,
	mt32_exit,
	mt32_volume,
	mt32_event,
	mt32_poll,
	mt32_allstop,
	NULL,
	001,	/* patch.001 */
	SFX_SEQ_PATCHFILE_NONE,
	0x01,	/* playflag */
	1,	/* do play channel 9 */
	32, /* Max polypgony */
	{SAMPLE_RATE, CHANNELS, SFX_PCM_FORMAT_S16_NATIVE}
};
