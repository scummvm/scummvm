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

#include "sherlock/scalpel/scalpel.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Scalpel {

#define NUM_PLACES 100
const int MAP_X[NUM_PLACES] = { 
	0, 368, 0, 219, 0, 282, 0, 43, 0, 0, 396, 408, 0, 0, 0, 568, 37, 325, 
	28, 0, 263, 36, 148, 469, 342, 143, 443, 229, 298, 0, 157, 260, 432, 
	174, 0, 351, 0, 528, 0, 136, 0, 0, 0, 555, 165, 0, 506, 0, 0, 344, 0, 0 
};
const int MAP_Y[NUM_PLACES] = { 
	0, 147, 0, 166, 0, 109, 0, 61, 0, 0, 264, 70, 0, 0, 0, 266, 341, 30, 275,
	0, 294, 146, 311, 230, 184, 268, 133, 94, 207, 0, 142, 142, 330, 255, 0, 
	37, 0, 70, 0, 116, 0, 0, 0, 50, 21, 0, 303, 0, 0, 229, 0, 0 
};

ScalpelEngine::ScalpelEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_chess = nullptr;
	_darts = nullptr;
	_tempFadeStyle = 0;
	_chessResult = 0;
}

ScalpelEngine::~ScalpelEngine() {
	delete _chess;
	delete _darts;
}

/**
 * Game initialization
 */
void ScalpelEngine::initialize() {
	SherlockEngine::initialize();

	_chess = new Chess(this);
	_darts = new Darts(this);

	_flags.resize(100 * 8);
	_flags[3] = true;		// Turn on Alley
	_flags[39] = true;		// Turn on Baker Street

	// Load the map co-ordinates for each scene
	for (int idx = 0; idx < NUM_PLACES; ++idx)
		_map.push_back(Common::Point(MAP_X[idx], MAP_Y[idx]));

	// Starting scene
	_scene->_goToRoom = 4;
}

/**
 * Show the opening sequence
 */
void ScalpelEngine::showOpening() {
	if (!showCityCutscene())
		return;
	if (!showAlleyCutscene())
		return;
	if (!showStreetCutscene())
		return;
	if (!showOfficeCutscene())
		return;

	_events->clearEvents();
	_sound->stopMusic();
}

bool ScalpelEngine::showCityCutscene() {
	byte palette[PALETTE_SIZE];
	
	_sound->playMusic("prolog1.mus");
	_titleOverride = "title.lib";
	_soundOverride = "title.snd";
	bool finished = _animation->playPrologue("26open1", 1, 255, true, 2);

	if (finished) {
		ImageFile titleImages("title2.vgs", true);
		_screen->_backBuffer.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);

		// London, England
		_screen->_backBuffer.transBlitFrom(titleImages[0], Common::Point(10, 11));
		_screen->randomTransition();
		finished = _events->delay(1000, true);

		// November, 1888
		if (finished) {
			_screen->_backBuffer.transBlitFrom(titleImages[1], Common::Point(101, 102));
			_screen->randomTransition();
			finished = _events->delay(5000, true);
		}

		// Transition out the title
		_screen->_backBuffer.blitFrom(_screen->_backBuffer2);
		_screen->randomTransition();
	}

	if (finished)
		finished = _animation->playPrologue("26open2", 1, 0, false, 2);

	if (finished) {
		ImageFile titleImages("title.vgs", true);
		_screen->_backBuffer.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);
		
		// The Lost Files of
		_screen->_backBuffer.transBlitFrom(titleImages[0], Common::Point(75, 6));
		// Sherlock Holmes
		_screen->_backBuffer.transBlitFrom(titleImages[1], Common::Point(34, 21));
		// copyright
		_screen->_backBuffer.transBlitFrom(titleImages[2], Common::Point(4, 190));

		_screen->verticalTransition();
		finished = _events->delay(4000, true);

		if (finished) {
			_screen->_backBuffer.blitFrom(_screen->_backBuffer2);
			_screen->randomTransition();
			finished = _events->delay(2000);
		}

		if (finished) {
			_screen->getPalette(palette);
			_screen->fadeToBlack(2);
		}

		if (finished) {
			// In the alley...
			_screen->transBlitFrom(titleImages[3], Common::Point(72, 51));
			_screen->fadeIn(palette, 3);
			finished = _events->delay(3000, true);
		}
	}

	_titleOverride = "";
	_soundOverride = "";
	return finished;
}

bool ScalpelEngine::showAlleyCutscene() {
	// TODO
	return true;
}

bool ScalpelEngine::showStreetCutscene() {
	// TODO
	return true;
}

bool ScalpelEngine::showOfficeCutscene() {
	// TODO
	return true;
}

/**
 * Starting a scene within the game
 */
void ScalpelEngine::startScene() {
	if (_scene->_goToRoom == 100 || _scene->_goToRoom == 98) {
		// Chessboard selection
		if (_sound->_musicEnabled) {
			if (_sound->loadSong(100)) {
				if (_sound->_music)
					_sound->startSong();
			}
		}

		_scene->_goToRoom = _chess->doChessBoard();

		_sound->freeSong();
		_scene->_hsavedPos = Common::Point(-1, -1);
		_scene->_hsavedFs = -1;
	}

	// Some rooms are prologue cutscenes, rather than normal game scenes. These are:
	//  2: Blackwood's capture
	// 52: Rescuing Anna
	// 53: Moorehead's death / subway train
	// 55: Fade out and exit 
	// 70: Brumwell suicide
	switch (_scene->_goToRoom) {
	case 2:
	case 52:
	case 53:
	case 70:
		if (_sound->_musicEnabled && _sound->loadSong(_scene->_goToRoom)) {
			if (_sound->_music)
				_sound->startSong();
		}

		switch (_scene->_goToRoom) {
		case 2:
			// Blackwood's capture
			_res->addToCache("final2.vda", "epilogue.lib");
			_res->addToCache("final2.vdx", "epilogue.lib");
			_animation->playPrologue("final1", 1, 3, true, 4);
			_animation->playPrologue("final22", 1, 0, false, 4);
			break;

		case 52:
			// Rescuing Anna
			_res->addToCache("finalr2.vda", "epilogue.lib");
			_res->addToCache("finalr2.vdx", "epilogue.lib");
			_res->addToCache("finale1.vda", "epilogue.lib");
			_res->addToCache("finale1.vdx", "epilogue.lib");
			_res->addToCache("finale2.vda", "epilogue.lib");
			_res->addToCache("finale2.vdx", "epilogue.lib");
			_res->addToCache("finale3.vda", "epilogue.lib");
			_res->addToCache("finale3.vdx", "epilogue.lib");
			_res->addToCache("finale4.vda", "EPILOG2.lib");
			_res->addToCache("finale4.vdx", "EPILOG2.lib");

			_animation->playPrologue("finalr1", 1, 3, true, 4);
			_animation->playPrologue("finalr2", 1, 0, false, 4);

			if (!_res->isInCache("finale2.vda")) {
				// Finale file isn't cached
				_res->addToCache("finale2.vda", "epilogue.lib");
				_res->addToCache("finale2.vdx", "epilogue.lib");
				_res->addToCache("finale3.vda", "epilogue.lib");
				_res->addToCache("finale3.vdx", "epilogue.lib");
				_res->addToCache("finale4.vda", "EPILOG2.lib");
				_res->addToCache("finale4.vdx", "EPILOG2.lib");
			}

			_animation->playPrologue("finale1", 1, 0, false, 4);
			_animation->playPrologue("finale2", 1, 0, false, 4);
			_animation->playPrologue("finale3", 1, 0, false, 4);

			_useEpilogue2 = true;
			_animation->playPrologue("finale4", 1, 0, false, 4);
			_useEpilogue2 = false;
			break;

		case 53:
			// Moorehead's death / subway train
			_res->addToCache("SUBWAY2.vda", "epilogue.lib");
			_res->addToCache("SUBWAY2.vdx", "epilogue.lib");
			_res->addToCache("SUBWAY3.vda", "epilogue.lib");
			_res->addToCache("SUBWAY3.vdx", "epilogue.lib");

			_animation->playPrologue("SUBWAY1", 1, 3, true, 4);
			_animation->playPrologue("SUBWAY2", 1, 0, false, 4);
			_animation->playPrologue("SUBWAY3", 1, 0, false, 4);

			// Set fading to direct fade temporary so the transition goes quickly.
			_tempFadeStyle = _screen->_fadeStyle ? 257 : 256;
			_screen->_fadeStyle = false;
			break;

		case 70:
			// Brumwell suicide
			_animation->playPrologue("suicid", 1, 3, true, 4);
			break;
		default:
			break;
		}

		// Except for the Moorehead Murder scene, fade to black first
		if (_scene->_goToRoom != 53) {
			_events->wait(40);
			_screen->fadeToBlack(3);
		}

		switch (_scene->_goToRoom) {
		case 52:
			_scene->_goToRoom = 27;			// Go to the Lawyer's Office
			_scene->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_scene->_overPos = Common::Point(22900 - 600, 9400 + 900);	// Overland position
			_scene->_oldCharPoint = 27;
			break;

		case 53:
			_scene->_goToRoom = 17;			// Go to St. Pancras Station
			_scene->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_scene->_overPos = Common::Point(32500 - 600, 3000 + 900);	// Overland position
			_scene->_oldCharPoint = 17;
			break;

		default:
			_scene->_goToRoom = 4;			// Back to Baker st.
			_scene->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_scene->_overPos = Common::Point(14500 - 600, 8400 + 900);	// Overland position
			_scene->_oldCharPoint = 4;
			break;
		}

		// Free any song from the previous scene
		_sound->freeSong();
		break;

	case 55:
		// Exit game
		_screen->fadeToBlack(3);
		quitGame();
		return;

	default:
		break;
	}

	_events->loadCursors("rmouse.vgs");
	_events->changeCursor(0);

	if (_scene->_goToRoom == 99) {
		// Chess Board
		_darts->playDarts();
		_chessResult = _scene->_goToRoom = 19;	// Go back to the bar
	}

	_chessResult = _scene->_goToRoom;
}

} // End of namespace Scalpel

} // End of namespace Scalpel
