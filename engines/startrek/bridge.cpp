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

#include "startrek/iwfile.h"
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initBridge(bool b) {
	_gfx->loadPalette("bridge");
	_sound->loadMusicFile("bridge");

	initStarfieldPosition();
	// TODO: starfield

	loadBridge();
}

void StarTrekEngine::loadBridge() {
	initStarfield(72, 30, 247, 102, 0);
	// TODO
	//initStarfieldSprite();

	_gfx->setBackgroundImage("bridge");
	_gfx->loadPri("bridge");
	_gfx->copyBackgroundScreen();
	_system->updateScreen();

	loadBridgeActors();
	//sub_1312C();	// TODO

	// TODO
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
	//initStarfieldSprite();
}

void StarTrekEngine::loadBridgeActors() {
	loadActorAnim(0, "bstndki", 0, 4, 256);
	loadActorAnim(1, "bstndkp", 0, 0, 256);
	loadActorAnim(6, "bstnduh", 0, 0, 256);
	loadActorAnim(5, "bstndch", 0, 0, 256);
	loadActorAnim(4, "bstndsu", 0, 0, 256);
	loadActorAnim(7, "bstndsc", 0, 0, 256);
	loadActorAnim(2, "xstndmc", 0, 0, 256);
}

void StarTrekEngine::cleanupBridge() {
	// TODO
	// if (!v_starfieldInitialized)
	// sub_12691()
	// else
	// clearScreenAndDelSeveralSprites()
	// v_mouseControllingShip = 0
	// v_keyboardControlsMouse = 1
}

void StarTrekEngine::runBridge() {
	//while (_gameMode == GAMEMODE_BRIDGE && !_resetGameMode) {

	//}
}

} // End of namespace StarTrek
