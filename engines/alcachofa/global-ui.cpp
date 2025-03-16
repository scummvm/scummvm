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

#include "global-ui.h"
#include "alcachofa.h"

namespace Alcachofa {

GlobalUI::GlobalUI() {
	auto &world = g_engine->world();
	_generalFont.reset(new Font(world.getGlobalAnimationName(GlobalAnimationKind::GeneralFont)));
	_dialogFont.reset(new Font(world.getGlobalAnimationName(GlobalAnimationKind::DialogFont)));
	_iconMortadelo.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::MortadeloIcon)));
	_iconFilemon.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::FilemonIcon)));
	_iconInventory.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::InventoryIcon)));
	_iconMortadeloDisabled.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::MortadeloDisabledIcon)));
	_iconFilemonDisabled.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::FilemonDisabledIcon)));
	_iconInventoryDisabled.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::InventoryDisabledIcon)));

	_generalFont->load();
	_dialogFont->load();
	_iconMortadelo->load();
	_iconFilemon->load();
	_iconInventory->load();
	_iconMortadeloDisabled->load();
	_iconFilemonDisabled->load();
	_iconInventoryDisabled->load();
}

}
