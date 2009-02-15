/***************************************************************************
 timers.c Copyright (C) 2002 Christoph Reichenbach


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

#include "sci/include/sfx_timer.h"
#include "sci/include/resource.h"

#ifdef SCUMMVM
extern sfx_timer_t sfx_timer_scummvm;
#else // SCUMMVM

#ifdef HAVE_SETITIMER
extern sfx_timer_t sfx_timer_sigalrm;
#endif

#ifdef _DREAMCAST
extern sfx_timer_t sfx_timer_pthread;
#endif
#endif // SCUMMVM

sfx_timer_t *sfx_timers[] = {
#ifdef SCUMMVM
	&sfx_timer_scummvm,
#else // SCUMMVM
#ifdef HAVE_SETITIMER
	&sfx_timer_sigalrm,
#endif
#ifdef _DREAMCAST
	&sfx_timer_pthread,
#endif
#endif // SCUMMVM
	NULL
};


sfx_timer_t *
sfx_find_timer(char *name)
{
	if (!name) {
		/* Policies go here */
		return sfx_timers[0];
	} else {
		int n = 0;
		while (sfx_timers[n]
		       && strcasecmp(sfx_timers[n]->name, name))
			++n;

		return sfx_timers[n];
	}
}
