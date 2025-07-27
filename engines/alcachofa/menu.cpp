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

#include "alcachofa.h"
#include "menu.h"
#include "player.h"
#include "script.h"

namespace Alcachofa {

void Menu::updateOpeningMenu() {
	if (!_openAtNextFrame) {
		_openAtNextFrame =
			g_engine->input().wasMenuKeyPressed() &&
			g_engine->player().isAllowedToOpenMenu();
		return;
	}
	_openAtNextFrame = false;

	g_engine->sounds().pauseAll(true);
	// TODO: Add game time behaviour on opening menu
	_previousRoom = g_engine->player().currentRoom();
	_isOpen = true;
	// TODO: Render thumbnail
	g_engine->player().changeRoom("MENUPRINCIPAL", true);
	// TODO: Check original read lastSaveFileFileId and read options.cfg, we do not need that right?

	g_engine->player().heldItem() = nullptr;
	g_engine->scheduler().backupContext();
	g_engine->camera().backup(1);
	g_engine->camera().setPosition(Math::Vector3d(
		g_system->getWidth() / 2.0f, g_system->getHeight() / 2.0f, 0.0f));

	// TODO: Load thumbnail into capture graphic object
}

void Menu::continueGame() {
	assert(_previousRoom != nullptr);
	_isOpen = false;
	g_engine->input().nextFrame(); // presumably to clear all was* flags
	g_engine->player().changeRoom(_previousRoom->name(), true);
	g_engine->sounds().pauseAll(false);
	g_engine->camera().restore(1);
	g_engine->scheduler().restoreContext();
	// TODO: Reset time on continueing game
}

void Menu::newGame() {
	// this action might be unused just like the only room it would appear: MENUPRINCIPALINICIO
	g_engine->player().isGameLoaded() = true;
	g_engine->script().createProcess(MainCharacterKind::None, g_engine->world().initScriptName());
}

}
