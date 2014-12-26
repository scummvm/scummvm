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

#include "xeen/darkside/darkside_game.h"
#include "xeen/resources.h"
#include "xeen/screen.h"

namespace Xeen {

DarkSideEngine::DarkSideEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: XeenEngine(syst, gameDesc) {
}

void DarkSideEngine::playGame() {
	playGame();
}

void DarkSideEngine::darkSideIntro() {
	//sub_28F40
	_screen->loadPalette("dark.pal");
	File special("special.bin");
	SpriteResource nwc[4] = {
		SpriteResource("nwc1.int"), SpriteResource("nwc2.int"), 
		SpriteResource("nwc3.int"), SpriteResource("nwc4.int")
	};
	File voc[3] = {
		File("dragon1.voc"), File("dragon2.voc"), File("dragon3.voc")
	};

	// Load backgrounds
	_screen->loadBackground("nwc1.raw");
	_screen->loadPage(0);
	_screen->loadBackground("nwc2.raw");
	_screen->loadPage(1);

	// Draw the screen and fade it in
	_screen->horizMerge(0);
	_screen->draw();
	_screen->fade(4);

	bool breakFlag = false;
	int nwcIndex = 0, nwcFrame = 0;
	for (int idx = 0; idx < 55 && !shouldQuit() && !breakFlag; ++idx) {
		// Render the next frame
		_events->updateGameCounter();
		_screen->vertMerge(0);
		nwc[nwcIndex].draw(*_screen, nwcFrame, Common::Point(0, 0));
		_screen->draw();

		switch (idx) {
		case 17:
			_sound->proc2(voc[0]);
			break;
		case 34:
		case 44:
			++nwcIndex;
			nwcFrame = 0;
			break;
		case 35:
			_sound->proc2(voc[1]);
			break;
		default:
			++nwcFrame;
		}

		while (!shouldQuit() && _events->timeElapsed() < 2) {
			_events->pollEventsAndWait();
			Common::KeyState keyState;
			if (_events->getKey(keyState)) {
				if (keyState.keycode == Common::KEYCODE_ESCAPE)
					breakFlag = true;
			}
		}
	}
}

} // End of namespace Xeen
