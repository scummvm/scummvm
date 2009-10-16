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

#include "../sequencer.h"

namespace Sci {

sfx_sequencer_t *sfx_sequencers[] = {
	NULL
};


sfx_sequencer_t *sfx_find_sequencer(char *name) {
	if (!name) {
		/* Implement default policy for your platform (if any) here, or in a function
		** called from here (if it's non-trivial). Try to use midi_devices[0], if
		** feasible. */

		return sfx_sequencers[0]; /* default */
	} else {
		int n = 0;
		while (sfx_sequencers[n]
		        && scumm_stricmp(sfx_sequencers[n]->name, name))
			++n;

		return sfx_sequencers[n];
	}
}

} // End of namespace Sci
