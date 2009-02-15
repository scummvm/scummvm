/***************************************************************************
 softsequencers.c  Copyright (C) 2004 Christoph Reichenbach


 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public Licence as
 published by the Free Software Foundaton; either version 2 of the
 Licence, or (at your option) any later version.

 It is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 merchantibility or fitness for a particular purpose. See the
 GNU General Public Licence for more details.

 You should have received a copy of the GNU General Public Licence
 along with this program; see the file COPYING. If not, write to
 the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.


 Please contact the maintainer for any program-related bug reports or
 inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

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
sfx_find_softseq(char *name)
{
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
