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

#include "common/util.h"

#include "sci/sfx/device.h"
#include "sci/sfx/seq/instrument-map.h"

namespace Sci {

#if 0
static midi_writer_t *writer = NULL;

static Common::Error midi_gm_open(int patch_len, byte *data, int patch2_len, byte *data2, void *device) {
	sfx_instrument_map_t *instrument_map = sfx_instrument_map_load_sci(data, patch_len);

	if (!instrument_map) {
		warning("[GM]  No GM instrument map found, trying MT-32 instrument map..");
		instrument_map = sfx_instrument_map_mt32_to_gm(data2, patch2_len);
	}

	writer = sfx_mapped_writer((midi_writer_t *) device, instrument_map);

	if (!writer)
		return Common::kUnknownError;

	if (writer->reset_timer)
		writer->reset_timer(writer);

	return Common::kNoError;
}

static Common::Error midi_gm_close() {
	return Common::kNoError;
}

static Common::Error midi_gm_event(byte command, int argc, byte *argv) {
	byte data[4];

	assert(argc < 4);
	data[0] = command;
	memcpy(data + 1, argv, argc);

	writer->write(writer, data, argc + 1);

	return Common::kNoError;
}

static Common::Error midi_gm_delay(int ticks) {
	writer->delay(writer, ticks);

	return Common::kNoError;
}

static Common::Error midi_gm_reset_timer(uint32 ts) {
	writer->reset_timer(writer);

	return Common::kNoError;
}

#define MIDI_MASTER_VOLUME_LEN 8

static Common::Error midi_gm_volume(uint8 volume) {
	byte data[MIDI_MASTER_VOLUME_LEN] = {
		0xf0,
		0x7f,
		0x7f,
		0x04,
		0x01,
		volume,
		volume,
		0xf7
	};

	writer->write(writer, data, MIDI_MASTER_VOLUME_LEN);
	if (writer->flush)
		writer->flush(writer);

	return Common::kNoError;
}

static Common::Error midi_gm_allstop() {
	byte data[3] = { 0xb0,
	                 0x78, /* all sound off */
	                 0
	               };
	int i;

	/* All sound off on all channels */
	for (i = 0; i < 16; i++) {
		data[0] = 0xb0 | i;
		writer->write(writer, data, 3);
	}
	if (writer->flush)
		writer->flush(writer);

	return Common::kNoError;
}

static Common::Error midi_gm_reverb(int reverb) {
	byte data[3] = { 0xb0,
	                 91, /* set reverb */
	                 reverb
	               };
	int i;

	/* Set reverb on all channels */
	for (i = 0; i < 16; i++)
		if (i != 9) {
			data[0] = 0xb0 | i;
			writer->write(writer, data, 3);
		}
	if (writer->flush)
		writer->flush(writer);

	return Common::kNoError;
}

static Common::Error midi_gm_set_option(char *x, char *y) {
	return Common::kUnknownError;
}

sfx_sequencer_t sfx_sequencer_gm = {
	"General MIDI",
	"0.1",
	SFX_DEVICE_MIDI,
	&midi_gm_set_option,
	&midi_gm_open,
	&midi_gm_close,
	&midi_gm_event,
	&midi_gm_delay,
	&midi_gm_reset_timer,
	&midi_gm_allstop,
	&midi_gm_volume,
	&midi_gm_reverb,
	004,	/* patch.004 */
	001,	/* patch.001 */
	0x01,	/* playflag */
	1,	/* do play rhythm */
	64,	/* max polyphony */
	0	/* no write-ahead needed inherently */
};
#endif

} // End of namespace Sci
