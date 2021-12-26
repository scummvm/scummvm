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

#include "ags/engine/ac/cd_audio.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/globals.h"

namespace AGS3 {

int init_cd_player() {
	_G(use_cdplayer) = 0;
	return _G(platform)->InitializeCDPlayer();
}

int cd_manager(int cmdd, int datt) {
	if (!_G(triedToUseCdAudioCommand)) {
		_G(triedToUseCdAudioCommand) = true;
		init_cd_player();
	}
	if (cmdd == 0) return _G(use_cdplayer);
	if (_G(use_cdplayer) == 0) return 0;  // ignore other commands

	return _G(platform)->CDPlayerCommand(cmdd, datt);
}

} // namespace AGS3
