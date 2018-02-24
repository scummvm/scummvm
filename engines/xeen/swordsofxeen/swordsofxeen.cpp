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

#include "xeen/swordsofxeen/swordsofxeen.h"

namespace Xeen {
namespace SwordsOfXeen {

SwordsOfXeenEngine::SwordsOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: XeenEngine(syst, gameDesc) {
}

void SwordsOfXeenEngine::outerGameLoop() {
	// TODO: Implement Swords of Xeen main menu

	playGame();
}

void SwordsOfXeenEngine::death() {
	Windows &windows = *g_vm->_windows;

	_screen->loadBackground("blank.raw");
	windows[28].setBounds(Common::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_screen->fadeIn(0x81);
	_screen->loadPalette("scr.pal");
	_screen->fadeIn(0x81);

	SpriteResource lose("lose.int");
	lose.draw(0, 0, Common::Point(0, 0));
	lose.draw(0, 1, Common::Point(160, 0));
	_sound->playSound("laff1.voc");

	bool breakFlag = false;
	for (int idx = 0, idx2 = 0; idx < (_files->_isDarkCc ? 10 : 23); ++idx) {
		_events->updateGameCounter();

		if (_files->_isDarkCc) {
			breakFlag = _events->wait(2);
		} else {
			if (idx == 1 || idx == 11)
				_sound->playFX(33);
			breakFlag = _events->wait(2);

			if (idx == 15)
				_sound->playFX(34);
		}

		if ((_files->_isDarkCc ? 9 : 10) == idx) {
			if ((_files->_isDarkCc ? 2 : 1) > idx2) {
				// Restart loop
				idx = -1;
				++idx2;
			}
		}
	}

	// Wait for press
	if (!breakFlag)
		_events->waitForPress();

	_screen->fadeOut(4);
	_screen->loadBackground("blank.raw");
	_screen->loadPalette("dark.pal");
	_sound->stopAllAudio();
}

void SwordsOfXeenEngine::dream() {
	error("TODO: Swords of Xeen dream sequence, if any");
}

void SwordsOfXeenEngine::showCutscene(const Common::String &name, int status, uint score) {
	_quitMode = QMODE_MENU;
}

} // End of namespace SwordsOfXeen
} // End of namespace Xeen
