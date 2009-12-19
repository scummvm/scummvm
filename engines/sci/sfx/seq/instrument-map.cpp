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

#include "common/scummsys.h"
#include "sci/sfx/softseq/mididriver.h"
#include "sci/sfx/seq/instrument-map.h"

namespace Sci {

sfx_instrument_map_t *sfx_instrument_map_new(int velocity_maps_nr) {
	sfx_instrument_map_t *map = (sfx_instrument_map_t *)malloc(sizeof(sfx_instrument_map_t));
	int i;

	map->initialisation_block_size = 0;
	map->initialisation_block = NULL;

	map->velocity_maps_nr = velocity_maps_nr;
	map->percussion_velocity_map_index = SFX_NO_VELOCITY_MAP;

	if (velocity_maps_nr == 0)
		map->velocity_map = NULL; /* Yes, this complicates control flow needlessly, but it avoids some of the pointless
					  ** warnings that certain memory tools seem to find appropriate. */
	else {
		map->velocity_map = (byte **)malloc(sizeof(byte *) * velocity_maps_nr);
		for (i = 0; i < velocity_maps_nr; ++i)
			map->velocity_map[i] = (byte *)malloc(SFX_VELOCITIES_NR);
	}
	for (i = 0; i < SFX_INSTRUMENTS_NR; ++i)
		map->velocity_map_index[i] = SFX_NO_VELOCITY_MAP;

	map->percussion_volume_adjust = 0;
	for (i = 0; i < SFX_RHYTHM_NR; ++i)
		map->percussion_map[i] = i;


	for (i = 0; i < SFX_INSTRUMENTS_NR; ++i) {
		map->patch_map[i].patch = i;
		map->patch_key_shift[i] = 0;
		map->patch_volume_adjust[i] = 0;
	}

	return map;
}

void sfx_instrument_map_free(sfx_instrument_map_t *map) {
	if (!map)
		return;

	if (map->velocity_map) {
		int i;
		for (i = 0; i < map->velocity_maps_nr; i++)
			free(map->velocity_map[i]);
		free(map->velocity_map);
		map->velocity_map = NULL;
	}

	if (map->initialisation_block) {
		free(map->initialisation_block);
		map->initialisation_block = NULL;
	}

	free(map);
}

#define PATCH_MAP_OFFSET		0x0000
#define PATCH_KEY_SHIFT_OFFSET		0x0080
#define PATCH_VOLUME_ADJUST_OFFSET	0x0100
#define PATCH_PERCUSSION_MAP_OFFSET	0x0180
#define PATCH_PERCUSSION_VOLUME_ADJUST	0x0200
#define PATCH_VELOCITY_MAP_INDEX	0x0201
#define PATCH_VELOCITY_MAP(i)		(0x0281 + (0x80 * i))
#define PATCH_INIT_DATA_SIZE_LE		0x0481
#define PATCH_INIT_DATA			0x0483

#define PATCH_INSTRUMENT_MAPS_NR 4

#define PATCH_MIN_SIZE PATCH_INIT_DATA


static int patch001_type0_length(byte *data, size_t length) {
	unsigned int pos = 492 + 246 * data[491];

	/*  printf("timbres %d (post = %04x)\n",data[491], pos);*/

	if ((length >= (pos + 386)) && (data[pos] == 0xAB) && (data[pos + 1] == 0xCD))
		pos += 386;

	/*  printf("pos = %04x (%02x %02x)\n", pos, data[pos], data[pos + 1]); */

	if ((length >= (pos + 267)) && (data[pos] == 0xDC) && (data[pos + 1] == 0xBA))
		pos += 267;

	/*  printf("pos = %04x %04x (%d)\n", pos, length, pos-length); */


	if (pos == length)
		return 1;
	return 0;
}

static int patch001_type1_length(byte *data, size_t length) {
	if ((length >= 1155) && (((data[1154] << 8) + data[1153] + 1155) == (int)length))
		return 1;
	return 0;
}

int sfx_instrument_map_detect(byte *data, size_t length) {
	/* length test */
	if (length < 1155)
		return SFX_MAP_MT32;
	if (length > 16889)
		return SFX_MAP_MT32_GM;
	if (patch001_type0_length(data, length) &&
	        !patch001_type1_length(data, length))
		return SFX_MAP_MT32;
	if (patch001_type1_length(data, length) &&
	        !patch001_type0_length(data, length))
		return SFX_MAP_MT32_GM;
	return SFX_MAP_UNKNOWN;
}


sfx_instrument_map_t *sfx_instrument_map_load_sci(byte *data, size_t size) {
	sfx_instrument_map_t * map;
	int i, m;

	if (data == NULL)
		return NULL;

	if (size < PATCH_MIN_SIZE) {
		warning("[instrument-map] Instrument map too small:  %d of %d", (int) size, PATCH_MIN_SIZE);
		return NULL;
	}

	map = sfx_instrument_map_new(PATCH_INSTRUMENT_MAPS_NR);

	/* Set up MIDI intialisation data */
	map->initialisation_block_size = (int16)READ_LE_UINT16(data + PATCH_INIT_DATA_SIZE_LE);
	if (map->initialisation_block_size) {
		if (size < PATCH_MIN_SIZE + map->initialisation_block_size) {
			warning("[instrument-map] Instrument map too small for initialisation block:  %d of %d", (int) size, PATCH_MIN_SIZE);
			return NULL;
		}

		if (size > PATCH_MIN_SIZE + map->initialisation_block_size)
			warning("[instrument-map] Instrument larger than required by initialisation block:  %d of %d", (int) size, PATCH_MIN_SIZE);

		if (map->initialisation_block_size != 0) {
			map->initialisation_block = (byte *)malloc(map->initialisation_block_size);
			memcpy(map->initialisation_block, data + PATCH_INIT_DATA, map->initialisation_block_size);
		}
	}

	/* Set up basic instrument info */
	for (i = 0; i < SFX_INSTRUMENTS_NR; i++) {
		map->patch_map[i].patch = (char)data[PATCH_MAP_OFFSET + i];
		map->patch_key_shift[i] = (char)data[PATCH_KEY_SHIFT_OFFSET + i];
		map->patch_volume_adjust[i] = (char)data[PATCH_VOLUME_ADJUST_OFFSET + i];
		map->patch_bend_range[i] = SFX_UNMAPPED;
		map->velocity_map_index[i] = data[PATCH_VELOCITY_MAP_INDEX + i];
	}

	/* Set up percussion maps */
	map->percussion_volume_adjust = data[PATCH_PERCUSSION_VOLUME_ADJUST];
	for (i = 0; i < SFX_RHYTHM_NR; i++) {
		map->percussion_map[i] = data[PATCH_PERCUSSION_MAP_OFFSET + i];
		map->percussion_velocity_scale[i] = SFX_MAX_VELOCITY;
	}

	/* Set up velocity maps */
	for (m = 0; m < PATCH_INSTRUMENT_MAPS_NR; m++) {
		byte *velocity_map = map->velocity_map[m];
		for (i = 0; i < SFX_VELOCITIES_NR; i++)
			velocity_map[i] = data[PATCH_VELOCITY_MAP(m) + i];
	}

	map->percussion_velocity_map_index = 0;

	return map;
}


/* Output with the instrument map */
#define MIDI_CHANNELS_NR 0x10

struct decorated_midi_writer_t : public midi_writer_t {
	midi_writer_t *writer;
	sfx_patch_map_t patches[MIDI_CHANNELS_NR];
	sfx_instrument_map_t *map;
};


static void init_decorated(struct _midi_writer *self_) {
	decorated_midi_writer_t *self = (decorated_midi_writer_t *) self_;
	self->writer->init(self->writer);
}

static void set_option_decorated(struct _midi_writer *self_, char *name, char *value) {
	decorated_midi_writer_t *self = (decorated_midi_writer_t *) self_;
	self->writer->set_option(self->writer, name, value);
}

static void delay_decorated(struct _midi_writer *self_, int ticks) {
	decorated_midi_writer_t *self = (decorated_midi_writer_t *) self_;
	self->writer->delay(self->writer, ticks);
}

static void flush_decorated(struct _midi_writer *self_) {
	decorated_midi_writer_t *self = (decorated_midi_writer_t *) self_;
	if (self->writer->flush)
		self->writer->flush(self->writer);
}

static void reset_timer_decorated(struct _midi_writer *self_) {
	decorated_midi_writer_t *self = (decorated_midi_writer_t *) self_;
	self->writer->reset_timer(self->writer);
}


static void close_decorated(decorated_midi_writer_t *self) {
	sfx_instrument_map_free(self->map);
	self->map = NULL;
	self->writer->close(self->writer);
	free((void *)self->name);
	self->name = NULL;
	free(self);
}

#define BOUND_127(x) (((x) < 0)? 0 : (((x) > 0x7f)? 0x7f : (x)))

static int bound_hard_127(int i, const char *descr) {
	int r = BOUND_127(i);
	if (r != i)
		warning("[instrument-map] Hard-clipping %02x to %02x in %s", i, r, descr);
	return r;
}

static Common::Error set_bend_range(midi_writer_t *writer, int channel, int range) {
	byte buf[3] = {0xb0, 0x65, 0x00};

	buf[0] |= channel & 0xf;
	if (writer->write(writer, buf, 3) != Common::kNoError)
		return Common::kUnknownError;

	buf[1] = 0x64;
	if (writer->write(writer, buf, 3) != Common::kNoError)
		return Common::kUnknownError;

	buf[1] = 0x06;
	buf[2] = BOUND_127(range);
	if (writer->write(writer, buf, 3) != Common::kNoError)
		return Common::kUnknownError;

	buf[1] = 0x26;
	buf[2] = 0;
	if (writer->write(writer, buf, 3) != Common::kNoError)
		return Common::kUnknownError;

	return Common::kNoError;
}

static Common::Error write_decorated(decorated_midi_writer_t *self, byte *buf, int len) {
	sfx_instrument_map_t *map = self->map;
	int op = *buf & 0xf0;
	int chan = *buf & 0x0f;
	int patch = self->patches[chan].patch;
	int rhythm = self->patches[chan].rhythm;

	assert(len >= 1);

	if (op == 0xC0 && chan != MIDI_RHYTHM_CHANNEL) { /* Program change */
		/*int*/
		patch = bound_hard_127(buf[1], "program change");
		int instrument = map->patch_map[patch].patch;
		int bend_range = map->patch_bend_range[patch];

		self->patches[chan] = map->patch_map[patch];

		if (instrument == SFX_UNMAPPED || instrument == SFX_MAPPED_TO_RHYTHM)
			return Common::kNoError;

		assert(len >= 2);
		buf[1] = bound_hard_127(instrument, "patch lookup");

		if (self->writer->write(self->writer, buf, len) != Common::kNoError)
			return Common::kUnknownError;

		if (bend_range != SFX_UNMAPPED)
			return set_bend_range(self->writer, chan, bend_range);

		return Common::kNoError;
	}

	if (chan == MIDI_RHYTHM_CHANNEL || patch == SFX_MAPPED_TO_RHYTHM) {
		/* Rhythm channel handling */
		switch (op) {
		case 0x80:
		case 0x90: { /* Note off / note on */
			int velocity, instrument, velocity_map_index, velocity_scale;

			if (patch == SFX_MAPPED_TO_RHYTHM) {
				buf[0] = (buf[0] & ~0x0f) | MIDI_RHYTHM_CHANNEL;
				instrument = rhythm;
				velocity_scale = SFX_MAX_VELOCITY;
			} else {
				int instrument_index = bound_hard_127(buf[1], "rhythm instrument index");
				instrument = map->percussion_map[instrument_index];
				velocity_scale = map->percussion_velocity_scale[instrument_index];
			}

			if (instrument == SFX_UNMAPPED)
				return Common::kNoError;

			assert(len >= 3);

			velocity = bound_hard_127(buf[2], "rhythm velocity");
			velocity_map_index = map->percussion_velocity_map_index;

			if (velocity_map_index != SFX_NO_VELOCITY_MAP)
				velocity = BOUND_127(velocity + map->velocity_map[velocity_map_index][velocity]);

			velocity = BOUND_127(velocity * velocity_scale / SFX_MAX_VELOCITY);

			buf[1] = bound_hard_127(instrument, "rhythm instrument");
			buf[2] = velocity;

			break;
		}

		case 0xB0: { /* Controller change */
			assert(len >= 3);
			if (buf[1] == 0x7) /* Volume change */
				buf[2] = BOUND_127(buf[2] + map->percussion_volume_adjust);
			break;
		}

		default:
			break;
		}

	} else {
		/* Instrument channel handling */

		if (patch == SFX_UNMAPPED)
			return Common::kNoError;

		switch (op) {
		case 0x80:
		case 0x90: { /* Note off / note on */
			int note = bound_hard_127(buf[1], "note");
			int velocity = bound_hard_127(buf[2], "velocity");
			int velocity_map_index = map->velocity_map_index[patch];
			assert(len >= 3);

			note += map->patch_key_shift[patch];
			/* Not the most efficient solutions, but the least error-prone */
			while (note < 0)
				note += 12;
			while (note > 0x7f)
				note -= 12;

			if (velocity_map_index != SFX_NO_VELOCITY_MAP)
				velocity = BOUND_127(velocity + map->velocity_map[velocity_map_index][velocity]);

			buf[1] = note;
			buf[2] = velocity;
			break;
		}

		case 0xB0: /* Controller change */
			assert(len >= 3);
			if (buf[1] == 0x7) /* Volume change */
				buf[2] = BOUND_127(buf[2] + map->patch_volume_adjust[patch]);
			break;

		default:
			break;
		}
	}

	return self->writer->write(self->writer, buf, len);
}

#define MIDI_BYTES_PER_SECOND 3250 /* This seems to be the minimum guarantee by the standard */
#define MAX_PER_TICK (MIDI_BYTES_PER_SECOND / 60) /* After this, we ought to issue one tick of pause */

static void init(midi_writer_t *writer, byte *data, size_t len) {
	size_t offset = 0;
	byte status = 0;

	/* Send init data as separate MIDI commands */
	while (offset < len) {
		int args;
		byte op = data[offset];
		byte msg[3];
		int i;

		if (op == 0xf0) {
			int msg_len;
			byte *find = (byte *) memchr(data + offset, 0xf7, len - offset);

			if (!find) {
				warning("[instrument-map] Failed to find end of sysex message");
				return;
			}

			msg_len = find - data - offset + 1;
			writer->write(writer, data + offset, msg_len);

			/* Wait at least 40ms after sysex */
			writer->delay(writer, 3);
			offset += msg_len;
			continue;
		}

		if (op < 0x80)
			op = status;
		else {
			status = op;
			offset++;
		}

		msg[0] = op;

		switch (op & 0xf0) {
		case 0xc0:
		case 0xd0:
			args = 1;
			break;
		default:
			args = 2;
		}

		if (args + offset > len) {
			warning("[instrument-map] Insufficient bytes remaining for MIDI command %02x", op);
			return;
		}

		for (i = 0; i < args; i++)
			msg[i + 1] = data[offset + i];

		writer->write(writer, msg, args + 1);
		offset += args;

		if (writer->flush)
			writer->flush(writer);
	}
}

#define NAME_SUFFIX "+instruments"

midi_writer_t *sfx_mapped_writer(midi_writer_t *writer, sfx_instrument_map_t *map) {
	int i;
	decorated_midi_writer_t *retval;

	if (map == NULL)
		return writer;

	retval = (decorated_midi_writer_t *)malloc(sizeof(decorated_midi_writer_t));
	retval->writer = writer;
	retval->name = (char *)malloc(strlen(writer->name) + strlen(NAME_SUFFIX) + 1);
	strcpy(retval->name, writer->name);
	strcat(retval->name, NAME_SUFFIX);

	retval->init = (Common::Error (*)(midi_writer_t *)) init_decorated;
	retval->set_option = (Common::Error (*)(midi_writer_t *, char *, char *)) set_option_decorated;
	retval->write = (Common::Error (*)(midi_writer_t *, byte *, int)) write_decorated;
	retval->delay = (void (*)(midi_writer_t *, int)) delay_decorated;
	retval->flush = (void (*)(midi_writer_t *)) flush_decorated;
	retval->reset_timer = (void (*)(midi_writer_t *)) reset_timer_decorated;
	retval->close = (void (*)(midi_writer_t *)) close_decorated;

	retval->map = map;

	init(writer, map->initialisation_block, map->initialisation_block_size);

	for (i = 0; i < MIDI_CHANNELS_NR; i++)
		retval->patches[i].patch = SFX_UNMAPPED;

	return (midi_writer_t *) retval;
}

} // End of namespace Sci
