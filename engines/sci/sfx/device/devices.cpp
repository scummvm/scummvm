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
#include "sci/sfx/device.h"

#include "sci/tools.h"

namespace Sci {

static struct _midi_device *devices_midi[] = {
	NULL
};

static struct _midi_device *devices_opl2[] = {
	NULL
};


/** -- **/

struct _midi_device **devices[] = {
	NULL, /* No device */
	devices_midi,
	devices_opl2,
};

void *sfx_find_device(int type, char *name) {
	struct _midi_device *dev = NULL;
	int i = 0;

	if (!type)
		return NULL;

	if (!name) {
		dev = devices[type][0];
	} else {
		while (devices[type][i] && !strcmp(name, devices[type][i]->name))
			++i;

		dev = devices[type][i];
	}

	if (dev) {
		if (dev->init(dev)) {
			fprintf(stderr, "[SFX] Opening device '%s' failed\n", dev->name);
			return NULL;
		}

		return dev;
	};

	return NULL;
}

} // End of namespace Sci
