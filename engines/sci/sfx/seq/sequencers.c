/***************************************************************************
 sequencers.c Copyright (C) 2004 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#include "../sequencer.h"
#include "sci/include/resource.h"

#ifndef SCUMMVM
extern sfx_sequencer_t sfx_sequencer_gm;
extern sfx_sequencer_t sfx_sequencer_mt32;
#ifdef HAVE_SYS_SOUNDCARD_H
extern sfx_sequencer_t sfx_sequencer_oss_adlib;
#endif
#endif // SCUMMVM

sfx_sequencer_t *sfx_sequencers[] = {
#ifndef SCUMMVM
	&sfx_sequencer_gm,
	&sfx_sequencer_mt32,
#ifdef HAVE_SYS_SOUNDCARD_H
	&sfx_sequencer_oss_adlib,
#endif
#endif // SCUMMVM
	NULL
};


sfx_sequencer_t *
sfx_find_sequencer(char *name)
{
	if (!name) {
		/* Implement default policy for your platform (if any) here, or in a function
		** called from here (if it's non-trivial). Try to use midi_devices[0], if
		** feasible. */

		return sfx_sequencers[0]; /* default */
	} else {
		int n = 0;
		while (sfx_sequencers[n]
		       && strcasecmp(sfx_sequencers[n]->name, name))
		       ++n;

		return sfx_sequencers[n];
	}
}
