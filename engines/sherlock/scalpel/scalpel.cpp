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

ScalpelEngine::ScalpelEngine(OSystem *syst, const SherlockGameDescription *gameDesc) :
		SherlockEngine(syst, gameDesc) {
	_tempFadeStyle = 0;
	_chessResult = 0;
}

/**
 * Game initialization
 */
void ScalpelEngine::initialize() {
	SherlockEngine::initialize();

	_flags.resize(100 * 8);
	_flags[3] = true;		// Turn on Alley
	_flags[39] = true;		// Turn on Baker Street

	// Starting room
	_rooms->_goToRoom = 4;
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
		Sprite titleSprites("title2.vgs", true);
		_screen->_backBuffer.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);

		// London, England
		_screen->_backBuffer.transBlitFrom(titleSprites[0], Common::Point(10, 11));
		_screen->randomTransition();
		finished = _events->delay(1000, true);

		// November, 1888
		if (finished) {
			_screen->_backBuffer.transBlitFrom(titleSprites[1], Common::Point(101, 102));
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
		Sprite titleSprites("title.vgs", true);
		_screen->_backBuffer.blitFrom(*_screen);
		_screen->_backBuffer2.blitFrom(*_screen);
		
		// The Lost Files of
		_screen->_backBuffer.transBlitFrom(titleSprites[0], Common::Point(75, 6));
		// Sherlock Holmes
		_screen->_backBuffer.transBlitFrom(titleSprites[1], Common::Point(34, 21));
		// copyright
		_screen->_backBuffer.transBlitFrom(titleSprites[2], Common::Point(4, 190));

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
			_screen->transBlitFrom(titleSprites[3], Common::Point(72, 51));
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

int ScalpelEngine::doChessBoard() {
	// TODO
	return 0;
}

void ScalpelEngine::playDarts() {
	// TODO
}

/**
 * Starting a scene within the game
 */
void ScalpelEngine::startScene() {
	if (_rooms->_goToRoom == 100 || _rooms->_goToRoom == 98) {
		// Chessboard selection
		if (_sound->_musicEnabled) {
			if (_sound->loadSong(100)) {
				if (_sound->_music)
					_sound->startSong();
			}
		}

		_rooms->_goToRoom = doChessBoard();

		_sound->freeSong();
		_hsavedPos = Common::Point(-1, -1);
		_hsavedFs = -1;
	}

	// Some rooms are prologue cutscenes, rather than normal game scenes. These are:
	//  2: Blackwood's capture
	// 52: Rescuing Anna
	// 53: Moorehead's death / subway train
	// 55: Fade out and exit 
	// 70: Brumwell suicide
	switch (_rooms->_goToRoom) {
	case 2:
	case 52:
	case 53:
	case 70:
		if (_sound->_musicEnabled && _sound->loadSong(_rooms->_goToRoom)) {
			if (_sound->_music)
				_sound->startSong();
		}

		switch (_rooms->_goToRoom) {
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
		if (_rooms->_goToRoom != 53) {
			_events->wait(40);
			_screen->fadeToBlack(3);
		}

		switch (_rooms->_goToRoom) {
		case 52:
			_rooms->_goToRoom = 27;			// Go to the Lawyer's Office
			_rooms->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_rooms->_overPos = Common::Point(22900 - 600, 9400 + 900);	// Overland position
			_rooms->_oldCharPoint = 27;
			break;

		case 53:
			_rooms->_goToRoom = 17;			// Go to St. Pancras Station
			_rooms->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_rooms->_overPos = Common::Point(32500 - 600, 3000 + 900);	// Overland position
			_rooms->_oldCharPoint = 17;
			break;

		default:
			_rooms->_goToRoom = 4;			// Back to Baker st.
			_rooms->_bigPos = Common::Point(0, 0);	// Overland scroll position
			_rooms->_overPos = Common::Point(14500 - 600, 8400 + 900);	// Overland position
			_rooms->_oldCharPoint = 4;
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

	if (_rooms->_goToRoom == 99) {
		// Chess Board
		playDarts();
		_chessResult = _rooms->_goToRoom = 19;	// Go back to the bar
	}

	_chessResult = _rooms->_goToRoom;
}

} // End of namespace Scalpel

} // End of namespace Scalpel
