/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra.h"
#include "kyra/kyra2.h"
#include "kyra/screen.h"
#include "kyra/wsamovie.h"
#include "kyra/sound.h"

#include "common/system.h"

namespace Kyra {

void KyraEngine_v2::seq_menu() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::seq_menu()");

	_sound->loadMusicFile("K2INTRO");
	_screen->loadBitmap("VIRGIN.CPS", 7, 7, _screen->_currentPalette);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0);
	_screen->updateScreen();
	_screen->fadeFromBlack();
	delay(60 * _tickLength);
	_screen->fadeToBlack();
	_screen->clearCurPage();

	if (_quitFlag)
		return;

	uint8 pal[768];
	int i;

	WSAMovieV2 *title = new WSAMovieV2(this);
	title->setDrawPage(0);

	title->open("WESTWOOD.WSA", 0, pal);
	assert(title->opened());

	title->setX(0); title->setY(0);
	title->displayFrame(0);
	_screen->updateScreen();
	_screen->fadePalette(pal, 0x54);

	_sound->playTrack(2);

	for (i = 1; i < 18 && !_quitFlag; ++i) {
		uint32 nextRun = _system->getMillis() + 6 * _tickLength;
		title->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}

	title->close();

	_screen->fadeToBlack();
	_screen->clearCurPage();

	if (_quitFlag) {
		delete title;
		return;
	}

	title->open("TITLE.WSA", 0, pal);
	assert(title->opened());

	title->setX(0); title->setY(0);
	title->displayFrame(0);
	_screen->updateScreen();
	_screen->fadePalette(pal, 0x54);

	_sound->playTrack(3);

	for (i = 1; i < 26 && !_quitFlag; ++i) {
		uint32 nextRun = _system->getMillis() + 6 * _tickLength;
		title->displayFrame(i);
		_screen->updateScreen();
		delayUntil(nextRun);
	}

	title->close();

	delete title;
}

} // end of namespace Kyra
