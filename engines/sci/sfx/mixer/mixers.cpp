/***************************************************************************
 mixers.c Copyright (C) 2003 Christoph Reichenbach


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

#include "../mixer.h"
#include "sci/include/resource.h"

extern sfx_pcm_mixer_t sfx_pcm_mixer_soft_linear;

#ifdef __DC__
extern sfx_pcm_mixer_t sfx_pcm_mixer_dc;
#endif

static sfx_pcm_mixer_t *mixers[] = {
#ifdef __DC__
	&sfx_pcm_mixer_dc,
#endif
	&sfx_pcm_mixer_soft_linear,
	NULL
};

sfx_pcm_mixer_t *
sfx_pcm_find_mixer(char *name) {
	int i = 0;

	if (name)
		while (mixers[i] && strcmp(name, mixers[i]->name))
			++i;

	return mixers[i];
}
