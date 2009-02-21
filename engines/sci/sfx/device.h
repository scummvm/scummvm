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

/* song player structure */

#ifndef SCI_SFX_DEVICE_H
#define SCI_SFX_DEVICE_H

#include "common/scummsys.h"

namespace Sci {

/* Various types of resources */
#define SFX_DEVICE_NONE 0
#define SFX_DEVICE_MIDI 1 /* midi writer */
#define SFX_DEVICE_OPL2 2 /* OPL/2 sequencer */

struct _midi_device {
	const char *name;

	int (*init)(struct _midi_device *self);
	/* Initializes the device
	** Parameters: (midi_device_t *) self: Self reference
	** Returns   : (int) SFX_OK on success, SFX_ERROR if the device could not be
	**                   opened
	*/

	int (*set_option)(struct _midi_device *self, char *name, char *value);
	/* Sets an option for the device
	** Parameters: (char *) name: Name of the option to set
	**             (char *) value: Value of the option to set
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise (unsupported option)
	*/
};

struct midi_writer_t {
	char *name; /* Name description of the device */

	int (*init)(midi_writer_t *self);
	/* Initializes the writer
	** Parameters: (midi_writer_t *) self: Self reference
	** Returns   : (int) SFX_OK on success, SFX_ERROR if the device could not be
	**                   opened
	*/

	int (*set_option)(midi_writer_t *self, char *name, char *value);
	/* Sets an option for the writer
	** Parameters: (char *) name: Name of the option to set
	**             (char *) value: Value of the option to set
	** Returns   : (int) SFX_OK on success, SFX_ERROR otherwise (unsupported option)
	*/

	int (*write)(midi_writer_t *self, unsigned char *buf, int len);
	/* Writes some bytes to the MIDI stream
	** Parameters: (char *) buf: The buffer to write
	**             (int) len: Number of bytes to write
	** Returns   : (int) SFX_OK on success, SFX_ERROR on failure
	** No delta time is expected here.
	*/

	void (*delay)(midi_writer_t *self, int ticks);
	/* Introduces an explicit delay
	** Parameters: (int) ticks: Number of 60 Hz ticks to sleep
	*/

	void (*flush)(midi_writer_t *self); /* May be NULL */
	/* Flushes the MIDI file descriptor
	** Parameters: (midi_writer_t *) self: Self reference
	*/

	void (*reset_timer)(midi_writer_t *self);
	/* Resets the timer associated with this device
	** Parameters: (midi_writer_t *) self: Self reference
	** This function makes sure that a subsequent write would have effect
	** immediately, and any delay() would be relative to the point in time
	** this function was invoked at.
	*/

	void (*close)(midi_writer_t *self);
	/* Closes the associated MIDI device
	** Parameters: (midi_writer_t *) self: Self reference
	*/
};


void *sfx_find_device(int type, char *name);
/* Looks up a device by name
** Parameters: (int) type: Device type to look up
**             (char *) name: Comma-separated list of devices to choose from
**                            (in the order specified), or NULL for default
** Returns   : (void *) The device requested, or NULL if no match was found
*/

} // End of namespace Sci

#endif // SCI_SFX_DEVICE_H
