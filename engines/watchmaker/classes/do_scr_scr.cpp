/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "watchmaker/classes/do_scr_scr.h"
#include "watchmaker/define.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/types.h"
#include "watchmaker/ll/ll_anim.h"
#include "watchmaker/classes/do_string.h"

namespace Watchmaker {

/* -----------------19/05/98 16.40-------------------
 *                  doScrScrUseWith
 * --------------------------------------------------*/
void doScrScrUseWith(WGame &game) {
	uint8 sent = TRUE;

	switch (UseWith[USED]) {
	case o2DQUADRODIVANO:
		if (UseWith[WITH] == o2DCASSAFORTECH) {
			StartAnim(game, a2D6);
			sent = false;
		}
		break;

	default:
		sent = TRUE;
		break;
	}

	if (sent)
		if (!(!(bUseWith & UW_WITHI) && (UseWith[USED] == UseWith[WITH])))
			PlayerSpeak(game, game.init.Obj[UseWith[USED]].action[CurPlayer]);
}

} // End of namespace Watchmaker
