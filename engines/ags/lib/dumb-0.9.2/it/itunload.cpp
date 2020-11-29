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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*  _______         ____    __         ___    ___
 * \    _  \       \    /  \  /       \   \  /   /       '   '  '
 *  |  | \  \       |  |    ||         |   \/   |         .      .
 *  |  |  |  |      |  |    ||         ||\  /|  |
 *  |  |  |  |      |  |    ||         || \/ |  |         '  '  '
 *  |  |  |  |      |  |    ||         ||    |  |         .      .
 *  |  |_/  /        \  \__//          ||    |  |
 * /_______/ynamic    \____/niversal  /__\  /____\usic   /|  .  . ibliotheque
 *                                                      /  \
 *                                                     / .  \
 * itunload.c - Code to free an Impulse Tracker       / / \  \
 *              module from memory.                  | <  /   \_
 *                                                   |  \/ /\   /
 * By entheh.                                         \_  /  > /
 *                                                      | \ / /
 *                                                      |  ' /
 *                                                       \__/
 */

#include "ags/lib/dumb-0.9.2/dumb.h"
#include "ags/lib/dumb-0.9.2/it/it.h"

namespace AGS3 {

void _dumb_it_unload_sigdata(sigdata_t *vsigdata) {
	if (vsigdata) {
		DUMB_IT_SIGDATA *sigdata = (DUMB_IT_SIGDATA *)vsigdata;
		int n;

		if (sigdata->order)
			free(sigdata->order);

		if (sigdata->instrument)
			free(sigdata->instrument);

		if (sigdata->sample) {
			for (n = 0; n < sigdata->n_samples; n++) {
				if (sigdata->sample[n].left)
					free(sigdata->sample[n].left);
				if (sigdata->sample[n].right)
					free(sigdata->sample[n].right);
			}
			free(sigdata->sample);
		}

		if (sigdata->pattern) {
			for (n = 0; n < sigdata->n_patterns; n++)
				if (sigdata->pattern[n].entry)
					free(sigdata->pattern[n].entry);
			free(sigdata->pattern);
		}

		if (sigdata->midi)
			free(sigdata->midi);

		{
			IT_CHECKPOINT *checkpoint = sigdata->checkpoint;
			while (checkpoint) {
				IT_CHECKPOINT *next = checkpoint->next;
				_dumb_it_end_sigrenderer(checkpoint->sigrenderer);
				free(checkpoint);
				checkpoint = next;
			}
		}

		free(vsigdata);
	}
}

} // namespace AGS3
