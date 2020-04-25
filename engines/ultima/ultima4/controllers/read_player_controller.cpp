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

#include "ultima/ultima4/controllers/read_player_controller.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {

ReadPlayerController::ReadPlayerController() : ReadChoiceController("12345678 \033\n") {
#ifdef IOS_ULTIMA4
	U4IOS::beginCharacterChoiceDialog();
#endif
}

ReadPlayerController::~ReadPlayerController() {
#ifdef IOS_ULTIMA4
	U4IOS::endCharacterChoiceDialog();
#endif
}

bool ReadPlayerController::keyPressed(int key) {
	bool valid = ReadChoiceController::keyPressed(key);
	if (valid) {
		if (_value < '1' ||
		        _value > ('0' + g_ultima->_saveGame->_members))
			_value = '0';
	} else {
		_value = '0';
	}
	return valid;
}

int ReadPlayerController::getPlayer() {
	return _value - '1';
}

int ReadPlayerController::waitFor() {
	ReadChoiceController::waitFor();
	return getPlayer();
}

} // End of namespace Ultima4
} // End of namespace Ultima
