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

#include "sci/include/sfx_player.h"

extern sfx_player_t sfx_player_realtime;
extern sfx_player_t sfx_player_polled;

sfx_player_t *sfx_players[] = {
	&sfx_player_polled,
	&sfx_player_realtime,
	NULL
};

sfx_player_t *
sfx_find_player(char *name) {
	if (!name) {
		/* Implement platform policy here */

		return sfx_players[0];
	} else {
		int n = 0;
		while (sfx_players[n] &&
		        scumm_stricmp(sfx_players[n]->name, name))
			++n;

		return sfx_players[n];
	}
}
