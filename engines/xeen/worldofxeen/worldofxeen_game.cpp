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

#include "xeen/worldofxeen/worldofxeen_game.h"

namespace Xeen {

WorldOfXeenEngine::WorldOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: XeenEngine(syst, gameDesc) {
}

void WorldOfXeenEngine::playGame () {
	XeenEngine::playGame();
}

void WorldOfXeenEngine::showDarkSideTitle() {
	showDarkSideTitle1();
	if (shouldQuit())
	return;

	showDarkSideTitle2();
	if (shouldQuit())
	return;
	
	// TODO: Only show intro sequence if not seen before
	showDarkSideIntroSequence();
}

void WorldOfXeenEngine::showDarkSideTitle1() {
	// TODO: Starting method, and sound
	//sub_28F40
	_screen->loadPalette("dark.pal");
	SpriteResource nwc[4] = {
		SpriteResource("nwc1.int"), SpriteResource("nwc2.int"),
		SpriteResource("nwc3.int"), SpriteResource("nwc4.int")
	};
	Common::File voc[3];
	voc[0].open("dragon1.voc");
	voc[1].open("dragon2.voc");
	voc[2].open("dragon3.voc");

	// Load backgrounds
	_screen->loadBackground("nwc1.raw");
	_screen->loadPage(0);
	_screen->loadBackground("nwc2.raw");
	_screen->loadPage(1);

	// Draw the screen and fade it in
	_screen->horizMerge(0);
	_screen->draw();
	_screen->fadeIn(4);

	// Initial loop for dragon roaring
	int nwcIndex = 0, nwcFrame = 0;
	for (int idx = 0; idx < 55 && !shouldQuit(); ++idx) {
		// Render the next frame
		_events->updateGameCounter();
		_screen->vertMerge(0);
		nwc[nwcIndex].draw(*_screen, nwcFrame);
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

		if (_events->wait(2, true))
			return;
	}

	// Loop for dragon using flyspray
	for (int idx = 0; idx < 42 && !shouldQuit(); ++idx) {
		_events->updateGameCounter();
		_screen->vertMerge(SCREEN_HEIGHT);
		nwc[3].draw(*_screen, idx);
		_screen->draw();

		switch (idx) {
		case 3:
			_sound->startMusic(40);
			break;
		case 11:
			_sound->startMusic(0);
		case 27:
		case 30:
			_sound->startMusic(3);
			break;
		case 31:
			_sound->proc2(voc[2]);
			break;
		case 33:
			_sound->startMusic(2);
			break;
		default:
			break;
		}

		if (_events->wait(2, true))
			return;
	}

	// Pause for a bit
	_events->wait(10, true);
}

void WorldOfXeenEngine::showDarkSideTitle2() {
	_screen->fadeOut(8);
	//TODO: Stuff

	_screen->loadBackground("jvc.raw");
	_screen->draw();
	_screen->fadeIn(4);

	_events->updateGameCounter();
	_events->wait(60, true);
}

void WorldOfXeenEngine::showDarkSideIntroSequence() {
	_screen->fadeOut(8);
	_screen->loadBackground("pyramid2.raw");
	_screen->loadPage(0);
	_screen->loadPage(1);
	_screen->loadBackground("pyramid3.raw");
	_screen->saveBackground(1);

	SpriteResource sprites[3] = {
		SpriteResource("title.int"), SpriteResource("pyratop.int"), SpriteResource("pyramid.int")
	};
	Common::File voc[2];
	voc[0].open("pharoh1a.voc");
	voc[1].open("pharoh1b.voc");

	_screen->vertMerge(SCREEN_HEIGHT);
	_screen->loadPage(0);
	_screen->loadPage(1);

	int yp = 0;
	int frame = 0;
	int idx1 = 0;
	bool skipElapsed = false;
	uint32 timeExpired = 0;
	//	bool fadeFlag = true;

	for (int idx = 200; idx > 0; ) {
		_events->updateGameCounter();
		_screen->vertMerge(yp);

		sprites[0].draw(*_screen, 0);
		if (frame)
			sprites[0].draw(*_screen, frame);

		idx1 = (idx1 + 1) % 4;
		if (!idx1)
			frame = (frame + 1) % 10;

		_screen->draw();
		if (!skipElapsed) {
			timeExpired = MAX(_events->timeElapsed(), (uint32)1);
			skipElapsed = true;
		}

		idx -= timeExpired;
		frame = MIN(frame + timeExpired, (uint)200);

		while (_events->timeElapsed() < 1) {
			_events->pollEventsAndWait();
			if (_events->isKeyMousePressed())
				return;
		}
	}

	_screen->vertMerge(SCREEN_HEIGHT);
	_screen->saveBackground(1);
	_screen->draw();
	_screen->freePages();

	_events->updateGameCounter();
	_events->wait(30, true);

	// TODO: More
	_sound->playSong(voc[0]);
	_sound->playSong(voc[1]);
}

} // End of namespace Xeen
