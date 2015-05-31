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

#include "sherlock/user_interface.h"
#include "sherlock/sherlock.h"
#include "sherlock/scalpel/scalpel_user_interface.h"
#include "sherlock/tattoo/tattoo_user_interface.h"

namespace Sherlock {

UserInterface *UserInterface::init(SherlockEngine *vm) {
	if (vm->getGameID() == GType_SerratedScalpel)
		return new Scalpel::ScalpelUserInterface(vm);
	else
		return new Tattoo::TattooUserInterface(vm);
}

UserInterface::UserInterface(SherlockEngine *vm) : _vm(vm) {
	_menuMode = STD_MODE;
	_menuCounter = 0;
	_infoFlag = false;
	_windowOpen = false;
	_endKeyActive = true;
	_invLookFlag = 0;
	_slideWindows = true;
	_helpStyle = false;
	_windowBounds = Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH - 1, SHERLOCK_SCREEN_HEIGHT - 1);
	_lookScriptFlag = false;

	_key = _oldKey = '\0';
	_selector = _oldSelector = -1;
	_temp = _oldTemp = 0;
	_temp1 = 0;
	_lookHelp = 0;
}

} // End of namespace Sherlock
