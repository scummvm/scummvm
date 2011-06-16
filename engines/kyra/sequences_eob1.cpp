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

#ifdef ENABLE_EOB

#include "kyra/eob1.h"
#include "kyra/screen_eob.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

#include "base/version.h"

namespace Kyra {

int EobEngine::mainMenu() {
	int menuChoice = 4;

	Screen::FontId of = _screen->_currentFont;
	
	while (menuChoice >= 0 && !shouldQuit()) {
		switch (menuChoice) {
			case 0:
				_screen->loadPalette("EOBPAL.COL", _screen->getPalette(0));
				_screen->loadEobCpsFileToPage("INTRO", 0, 5, 3, 2);
				_screen->setScreenPalette(_screen->getPalette(0));
				_screen->_curPage = 2;
				of = _screen->setFont(Screen::FID_6_FNT);
				_screen->printText(gScummVMVersion, 280 - strlen(gScummVMVersion) * 6, 153, _screen->getPagePixel(2, 0, 0), 0);
				_screen->setFont(of);
				_screen->fillRect(0, 159, 319, 199, _screen->getPagePixel(2, 0, 0));
				gui_drawBox(77, 165, 173, 29, 13, 14, 12);
				gui_drawBox(76, 164, 175, 31, 13, 14, -1);
				_screen->_curPage = 0;
				_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();
				menuChoice = mainMenuLoop();
				break;

			case 1:
				// load game in progress
				//
				menuChoice = -1;
				break;

			case 2:
				// create new party
				menuChoice = -2;
				break;

			case 3:
				// quit
				menuChoice = -5;
				break;

			case 4:
				// intro
				_sound->loadSoundFile("SOUND");
				_screen->hideMouse();
				seq_playOpeningCredits();
				seq_playIntro();
				_screen->showMouse();
				_sound->loadSoundFile("ADLIB");
				menuChoice = 0;
				break;
		}
	}

	return shouldQuit() ? -5 : menuChoice;
}

int EobEngine::mainMenuLoop() {
	int sel = -1;
	do {
		_screen->setScreenDim(28);
		_gui->setupMenu(8, 0, _mainMenuStrings, -1, 0, 0);

		while (sel == -1 && !shouldQuit())
			sel = _gui->handleMenu(8, _mainMenuStrings, 0, -1, 0);
	} while ((sel < 0 || sel > 5) && !shouldQuit());

	return sel + 1;
}

void EobEngine::seq_playOpeningCredits() {
	static const char *cmpList[] = { "WESTWOOD.CMP", "AND.CMP", "SSI.CMP", "PRESENT.CMP", "DAND.CMP" };
	static const uint8 frameDelay[] = { 140, 50, 100, 50, 140 };

	_screen->loadPalette("WESTWOOD.COL", _screen->getPalette(0));
	_screen->setScreenPalette(_screen->getPalette(0));

	_screen->loadBitmap(cmpList[0], 5, 3, 0);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	_sound->playTrack(1);
	delay(frameDelay[0] * _tickLength);

	for (int i = 1; i < 5 && !shouldQuit() && !skipFlag(); i++) {
		_screen->loadBitmap(cmpList[i], 5, 3, 0);
		uint32 nextFrameTimer = _system->getMillis() + frameDelay[i] * _tickLength;
		_screen->crossFadeRegion(0, 50, 0, 50, 320, 102, 2, 0);
		delayUntil(nextFrameTimer);
	}

	delay(50 * _tickLength);
}

void EobEngine::seq_playIntro() {
	//_sound->playTrack(2);
}

void EobEngine::seq_playFinale() {

}

}	// End of namespace Kyra

#endif // ENABLE_EOB
