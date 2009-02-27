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

#include <sfx_engine.h>
#include "../device.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace Sci {

#define SCI_UNIXRAW_MIDI_VERSION "0.1"

#ifndef O_SYNC
#  define O_SYNC 0
#endif

static int fd;
static const char *devicename = "/dev/midi";

static int unixraw_init(midi_writer_t *self) {
	sciprintf("[SFX] Initialising UNIX raw MIDI backend, v%s\n", SCI_UNIXRAW_MIDI_VERSION);

	fd = open(devicename, O_WRONLY | O_SYNC);

	if (!IS_VALID_FD(fd)) {
		sciprintf("[SFX] Failed to open %s\n", devicename);
		return SFX_ERROR;
	}

	return SFX_OK;
}

static int unixraw_set_option(midi_writer_t *self, char *name, char *value) {
	return SFX_ERROR;
}

static int unixraw_write(midi_writer_t *self, unsigned char *buffer, int len) {
	if (write(fd, buffer, len) != len) {
		sciprintf("[SFX] MIDI write error\n");
		return SFX_ERROR;
	}
	return SFX_OK;
}

static void unixraw_delay(midi_writer_t *self, int ticks) {
}

static void unixraw_reset_timer(midi_writer_t *self) {
}

static void unixraw_close(midi_writer_t *self) {
	close(fd);
}

midi_writer_t sfx_device_midi_unixraw = {
	"unixraw-midi",
	&unixraw_init,
	&unixraw_set_option,
	&unixraw_write,
	&unixraw_delay,
	NULL,
	&unixraw_reset_timer,
	&unixraw_close
};

} // End of namespace Sci
