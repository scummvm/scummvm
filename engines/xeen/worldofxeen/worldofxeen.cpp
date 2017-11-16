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

#include "xeen/worldofxeen/worldofxeen.h"
#include "xeen/worldofxeen/darkside_cutscenes.h"
#include "xeen/worldofxeen/clouds_cutscenes.h"
#include "xeen/worldofxeen/worldofxeen_menu.h"
#include "xeen/sound.h"

namespace Xeen {
namespace WorldOfXeen {

WorldOfXeenEngine::WorldOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: XeenEngine(syst, gameDesc), CloudsCutscenes(this),
		DarkSideCutscenes(this) {
	_seenDarkSideIntro = false;
	_pendingAction = WOX_PLAY_GAME;
}

void WorldOfXeenEngine::outerGameLoop() {
	//_pendingAction = getGameID() == GType_DarkSide ? WOX_DARKSIDE_INTRO : WOX_CLOUDS_INTRO;
	_pendingAction = WOX_MENU;
	if (gDebugLevel >= 1)
		// Skip main menu when starting in debug mode
		_pendingAction = WOX_PLAY_GAME;

	while (!shouldQuit() && _pendingAction != WOX_QUIT) {
		switch (_pendingAction) {
		case WOX_CLOUDS_INTRO:
			if (showCloudsTitle())
				showCloudsIntro();
			_pendingAction = WOX_MENU;
			break;

		case WOX_CLOUDS_ENDING:
			showCloudsEnding();
			_pendingAction = WOX_MENU;
			break;

		case WOX_DARKSIDE_INTRO:
			if (showDarkSideTitle())
				showDarkSideIntro();
			_pendingAction = WOX_MENU;
			break;

		case WOX_DARKSIDE_ENDING:
			showDarkSideEnding();
			_pendingAction = WOX_MENU;
			break;

		case WOX_WORLD_ENDING:
			// TODO
			return;

		case WOX_MENU:
			WorldOfXeenMenu::show(this);
			break;

		case WOX_PLAY_GAME:
			playGame();
			break;

		default:
			break;
		}
	}
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
