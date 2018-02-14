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
		: XeenEngine(syst, gameDesc), WorldOfXeenCutscenes(this) {
	_seenDarkSideIntro = false;
	_pendingAction = WOX_PLAY_GAME;
}

void WorldOfXeenEngine::outerGameLoop() {
	//_pendingAction = getGameID() == GType_DarkSide ? WOX_DARKSIDE_INTRO : WOX_CLOUDS_INTRO;
	_pendingAction = WOX_MENU;

	if (_loadSaveSlot != -1)
		// Loading savegame from launcher, so Skip menu and go straight to game
		_pendingAction = WOX_PLAY_GAME;

	while (!shouldQuit() && _pendingAction != WOX_QUIT) {
		// TODO: Remove this once proper startup menus are added for Clouds & Dark Side
		if (g_vm->getGameID() != GType_WorldOfXeen)
			_pendingAction = WOX_PLAY_GAME;

		WOXGameAction action = _pendingAction;
		_pendingAction = WOX_MENU;
		_quitMode = QMODE_NONE;

		switch (action) {
		case WOX_CLOUDS_INTRO:
			if (showCloudsTitle())
				showCloudsIntro();
			break;

		case WOX_CLOUDS_ENDING:
			//showCloudsEnding();
			break;

		case WOX_DARKSIDE_INTRO:
			if (showDarkSideTitle())
				showDarkSideIntro();
			break;

		case WOX_DARKSIDE_ENDING:
			showDarkSideEnding();
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

void WorldOfXeenEngine::death() {
	Window &w = (*_windows)[0];
	_sound->stopAllAudio();
	SpriteResource fireSprites[4] = {
		SpriteResource("fire1.vga"),
		SpriteResource("fire2.vga"),
		SpriteResource("fire3.vga"),
		SpriteResource("fire4.vga")
	};
	SpriteResource deathSprites("death.vga"), death1Sprites("death1.vga");
	const int Y_LIST[] = {
		196, 187, 179, 169, 159, 147, 138, 127, 113, 101, 86,
		73, 60, 48, 36, 23, 10, 0, 0
	};

	Graphics::ManagedSurface savedBg;
	savedBg.copyFrom(*_screen);

	fireSprites[0].draw(0, 0, Common::Point(0, 0));
	fireSprites[0].draw(0, 1, Common::Point(160, 0));
	w.update();
	_sound->playSound("fire.voc");

	// Fire will vertically consume the screen
	for (int idx = 2; idx < 36; idx += 2) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);

		fireSprites[idx / 10].draw(0, idx % 10, Common::Point(0, 0));
		fireSprites[idx / 10].draw(0, (idx % 10) + 1, Common::Point(160, 0));

		for (int yCtr = 0, frame = 0; yCtr < (idx / 2); ++yCtr, frame += 2) {
			deathSprites.draw(0, frame, Common::Point(0, Y_LIST[yCtr]));
			deathSprites.draw(0, frame + 1, Common::Point(160, Y_LIST[yCtr]));
		}

		w.update();
		_events->wait(1);
	}

	deathSprites.draw(0, 34, Common::Point(0, 0));
	deathSprites.draw(0, 35, Common::Point(160, 0));
	w.update();
	savedBg.blitFrom(*_screen);

	_sound->playSong(_files->_isDarkCc ? "laff1.voc" : "xeenlaff.voc");

	// Animation of Xeen or Alamar laughing
	for (int idx = 0, idx2 = 0; idx < (_files->_isDarkCc ? 10 : 23); ++idx) {
		_events->updateGameCounter();
		_screen->blitFrom(savedBg);

		if (idx != 0)
			death1Sprites.draw(0, idx - 1);
		w.update();

		if (_files->_isDarkCc) {
			_events->wait(2);
		} else {
			if (idx == 1 || idx == 11)
				_sound->playFX(33);
			_events->wait(2);
			if (idx == 15)
				_sound->playFX(34);
		}

		if (idx == (_files->_isDarkCc ? 9 : 10)) {
			if (idx2 < (_files->_isDarkCc ? 2 : 1)) {
				idx = -1;
				++idx2;
			}
		}

		if (!_sound->isPlaying())
			idx = 23;
	}

	_screen->blitFrom(savedBg);
	w.update();
}

void WorldOfXeenEngine::dream() {
	Windows &windows = *_windows;
	Graphics::ManagedSurface savedBg;

	savedBg.copyFrom(*_screen);
	_screen->fadeOut();
	_events->hideCursor();

	_screen->loadBackground("scene1.raw");
	windows[0].update();
	_screen->fadeIn();

	_events->updateGameCounter();
	while (!shouldExit() && _events->timeElapsed() < 7)
		_events->pollEventsAndWait();

	_sound->playSound("dreams2.voc", 1);
	while (!shouldExit() && _sound->isPlaying())
		_events->pollEventsAndWait();

	_sound->playSound("laff1.voc", 1);
	while (!shouldExit() && _sound->isPlaying())
		_events->pollEventsAndWait();

	_events->updateGameCounter();
	while (!shouldExit() && _events->timeElapsed() < 7)
		_events->pollEventsAndWait();

	_screen->fadeOut();
	_events->setCursor(0);
	_screen->blitFrom(savedBg);
	windows[0].update();

	_screen->fadeIn();
}

void WorldOfXeenEngine::showCutscene(const Common::String &name, int status, uint score) {
	_sound->stopAllAudio();
	_events->clearEvents();

	if (name == "ENDGAME")
		showCloudsEnding(score);
	else if (name == "ENDGAME2")
		showDarkSideEnding();
	else if (name == "WORLDEND")
		showWorldOfXeenEnding((GooberState)status, score);

	_screen->freePages();
	_sound->stopAllAudio();
	_events->clearEvents();
	_quitMode = QMODE_MENU;
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
