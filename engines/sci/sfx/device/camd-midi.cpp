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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifdef HAVE_PROTO_CAMD_H

#include "sfx_engine.h"
#include "../device.h"

#include <proto/camd.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <stdio.h>


#define SWAP_BYTES
#define FILL_BYTES

#define SCI_CAMD_MIDI_VERSION "0.1"
#define SYSEX_PREFIX 0xf0

static const char *devicename = "via686.out.0";

struct Library *CamdBase = NULL;
struct CamdIFace *ICamd = NULL;
static struct MidiLink *midi_link = NULL;
static struct MidiNode *midi_node = NULL;

#define ABORT(m) {						\
			if (CamdBase)				\
                   		IExec->CloseLibrary(CamdBase);	\
			sciprintf("[SFX] CAMD driver: ");	\
			sciprintf(m);				\
			sciprintf("\n");			\
			return SFX_ERROR;			\
	}

static int
camd_init(midi_writer_t *self) {
	sciprintf("[SFX] Initialising CAMD raw MIDI backend, v%s\n", SCI_CAMD_MIDI_VERSION);

	CamdBase = IExec->OpenLibrary("camd.library", 36L);
	if (!CamdBase)
		ABORT("Could not open 'camd.library'");

	ICamd = (struct CamdIFace *) IExec->GetInterface(CamdBase, "main", 1, NULL);
	if (!ICamd)
		ABORT("Error while retrieving CAMD interface\n");

	midi_node = ICamd->CreateMidi(MIDI_MsgQueue, 0L, MIDI_SysExSize, 4096L, MIDI_Name, "freesci", TAG_END);
	if (!midi_node)
		ABORT("Could not create CAMD MIDI node");

	midi_link = ICamd->AddMidiLink(midi_node, MLTYPE_Sender, MLINK_Location, devicename, TAG_END);
	if (!midi_link)
		ABORT(("Could not create CAMD MIDI link to '%s'", devicename));

	sciprintf("[SFX] CAMD initialisation completed\n");

	return SFX_OK;
}

static int
camd_set_option(midi_writer_t *self, char *name, char *value) {
	return SFX_ERROR;
}

#define MAX_MIDI_LEN 3

static int
camd_write(midi_writer_t *self, unsigned char *buffer, int len) {
	if (len == 0)
		return SFX_OK;

	if (buffer[0] == SYSEX_PREFIX) {
		/* Must send this as a SysEx */
		ICamd->PutSysEx(midi_link, buffer);
	} else {
		ULONG data = 0l;
		int i;
		int readlen = (len > MAX_MIDI_LEN) ? MAX_MIDI_LEN : len;

		for (i = 0; i < readlen; i++)
			if (len >= i) {
				data <<= 8;
				data |= buffer[i];
			}
		data <<= (8 * (sizeof(ULONG) - readlen));

		if (len > MAX_MIDI_LEN)
			sciprintf("[SFX] Warning: Truncated MIDI message to fit CAMD format (sent %d: %02x %02x %02x, real length %d)\n",
			          MAX_MIDI_LEN, buffer[0], buffer[1], buffer[2], len);

		ICamd->PutMidi(midi_link, data);
	}

	return SFX_OK;
}

static void
camd_delay(midi_writer_t *self, int ticks) {
}

static void
camd_reset_timer(midi_writer_t *self) {
}

static void
camd_close(midi_writer_t *self) {
#ifdef NO_OP
	return;
#endif
	if (CamdBase)
		IExec->CloseLibrary(CamdBase);
}

midi_writer_t sfx_device_midi_camd = {
	"camd-midi",
	&camd_init,
	&camd_set_option,
	&camd_write,
	&camd_delay,
	NULL,
	&camd_reset_timer,
	&camd_close
};

#endif /* HAVE_PROTO_CAMD_H */
