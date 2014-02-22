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
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/graphics.h"
#include "mads/msurface.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/dialogs_nebular.h"

namespace MADS {

namespace Nebular {

GameNebular::GameNebular(MADSEngine *vm): Game(vm) {
	_surface =MSurface::init(MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT - MADS_INTERFACE_HEIGHT);
}

bool GameNebular::checkCopyProtection() {
	if (!ConfMan.getBool("copy_protection"))
		return true;

	CopyProtectionDialog::show(_vm);
	return false;
}

} // End of namespace Nebular

} // End of namespace MADS
