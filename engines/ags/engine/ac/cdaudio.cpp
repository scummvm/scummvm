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

#include "ac/cdaudio.h"
#include "platform/base/agsplatformdriver.h"

namespace AGS3 {

int use_cdplayer = 0;
bool triedToUseCdAudioCommand = false;
int need_to_stop_cd = 0;

int init_cd_player() {
	use_cdplayer = 0;
	return platform->InitializeCDPlayer();
}

int cd_manager(int cmdd, int datt) {
	if (!triedToUseCdAudioCommand) {
		triedToUseCdAudioCommand = true;
		init_cd_player();
	}
	if (cmdd == 0) return use_cdplayer;
	if (use_cdplayer == 0) return 0; // ignore other commands

	return platform->CDPlayerCommand(cmdd, datt);
}

} // namespace AGS3
