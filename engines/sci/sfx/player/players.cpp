/***************************************************************************
 players.c Copyright (C) 2002..04 Christoph Reichenbach


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

#include "sci/include/sfx_player.h"

extern sfx_player_t sfx_player_realtime;
extern sfx_player_t sfx_player_polled;

sfx_player_t *sfx_players[] = {
	&sfx_player_polled,
	&sfx_player_realtime,
	NULL
};

sfx_player_t *
sfx_find_player(char *name)
{
	if (!name) {
		/* Implement platform policy here */

		return sfx_players[0];
	} else {
		int n = 0;
		while (sfx_players[n] &&
		       strcasecmp(sfx_players[n]->name, name))
			++n;

		return sfx_players[n];
	}
}
