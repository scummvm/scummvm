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

using namespace Common;

namespace Alcachofa {

// originally the inventory only reacts to exactly top-left/bottom-right which is fine in
// fullscreen when you just slam the mouse cursor into the corner.
// In any other scenario this is cumbersome so I expand this area.
// And it is still pretty bad, especially in windowed mode so I should add key-based controls for it
static constexpr int16 kInventoryTriggerSize = 10;

Rect openInventoryTriggerBounds() {
	int16 size = kInventoryTriggerSize * 1024 / g_system->getWidth();
	return Rect(0, 0, size, size);
}

Rect closeInventoryTriggerBounds() {
	int16 size = kInventoryTriggerSize * 1024 / g_system->getWidth();
	return Rect(g_system->getWidth() - size, g_system->getHeight() - size, g_system->getWidth(), g_system->getHeight());
}

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

void GlobalUI::startClosingInventory() {
	_isOpeningInventory = false;
	_isClosingInventory = true;
	_timeForInventory = g_system->getMillis();
}

void GlobalUI::updateClosingInventory() {
	static constexpr uint32 kDuration = 300;
	static constexpr float kSpeed = -10 / 3.0f / 1000.0f;

	uint32 deltaTime = g_system->getMillis() - _timeForInventory;
	if (!_isClosingInventory || deltaTime >= kDuration)
		_isClosingInventory = false;
	else
		g_engine->world().inventory().drawAsOverlay((int32)(g_system->getHeight() * (deltaTime * kSpeed)));
}

bool GlobalUI::updateOpeningInventory() {
	static constexpr float kSpeed = 10 / 3.0f / 1000.0f;
	if (g_engine->player().isOptionsMenuOpen() || !g_engine->player().isGameLoaded())
		return false;

	if (_isOpeningInventory) {
		uint32 deltaTime = g_system->getMillis() - _timeForInventory;
		if (deltaTime >= 1000) {
			_isOpeningInventory = false;
			g_engine->world().inventory().open();
		}
		else {
			deltaTime = MIN<uint32>(300, deltaTime);
			g_engine->world().inventory().drawAsOverlay((int32)(g_system->getHeight() * (deltaTime * kSpeed - 1)));
		}
		return true;
	}
	else if (openInventoryTriggerBounds().contains(g_engine->input().mousePos2D())) {
		_isClosingInventory = false;
		_isOpeningInventory = true;
		_timeForInventory = g_system->getMillis();
		g_engine->player().activeCharacter()->stopWalking();
		g_engine->world().inventory().updateItemsByActiveCharacter();
		return true;
	}
	return false;
}

}
