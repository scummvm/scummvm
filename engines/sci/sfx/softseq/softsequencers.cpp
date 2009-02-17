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

#include "../softseq.h"

extern sfx_softseq_t sfx_softseq_opl2;
extern sfx_softseq_t sfx_softseq_SN76496;
extern sfx_softseq_t sfx_softseq_pcspeaker;
extern sfx_softseq_t sfx_softseq_amiga;
extern sfx_softseq_t sfx_softseq_mt32;
extern sfx_softseq_t sfx_softseq_fluidsynth;

static sfx_softseq_t *sw_sequencers[] = {
	&sfx_softseq_opl2,
	/*	&sfx_softseq_mt32, */
	&sfx_softseq_SN76496,
	&sfx_softseq_pcspeaker,
	&sfx_softseq_amiga,
#ifdef HAVE_FLUIDSYNTH_H
	&sfx_softseq_fluidsynth,
#endif
	NULL
};


sfx_softseq_t *
sfx_find_softseq(const char *name) {
	if (!name)
		return sw_sequencers[0];
	else {
		int i = 0;
		while (sw_sequencers[i])
			if (!strcmp(sw_sequencers[i]->name, name))
				return sw_sequencers[i];
			else
				++i;

		return NULL; /* Not found */
	}
}
