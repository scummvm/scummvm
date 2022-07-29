/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "immortal/immortal.h"

namespace Immortal {

void ImmortalEngine::logicInit() {
	_titlesShown = 0;
	_time = 0;
	_promoting = 0;
	_restart = true;
	//level_initAtStartOfGameOnly
	_lastCertLen = 0;
}

void ImmortalEngine::restartLogic() {
	_singleStep = false;
	_levelOver = false;
	_gameFlags = kSavedNone;

	// Here's where the majority of the game actually gets initialized
	//miscInit();
	//qarrayInit();
	//cycInit();		<-- room.initCycles()
	//fsetInit();		<-- room.initTorches()
	//levelInit();		<-- presumably creates room
	//roomInit();		<-- will be run in constructor of room
	//monstInit();		<-- room.initMonsters()		\
	//objectInit();		<-- room.initObjects()
	//doorInit();		<-- room.initDoors()		 |- probably all get run from room constructor
	//sparkInit();		<-- room.initSparks()
	//bulletInit();		<-- room.initProjectiles()	/
	//objectInit(); 	<-- again? Odd...
	//genericSpriteInit();	<-- room.initGenSprites()

	// Probably will be written as:
	// qarrayInit();  <-- will probably just be like, _qarray = new Common::Array<int? *>();
	// levelInit();

	if (fromOldGame() == false) {
		_level = 0;
		//levelNew();
	}

	if (_level != 7) {
		_themePaused = true;	// and #-1-2 = set both flags for themePaused
	}
}

void ImmortalEngine::logic() {
	trapKeys();							// First thing in any gameloop is to check if we should restart/toggle sound
	_time += 1;

	/* This is actually the main game state loop. I think the best way to translate it
	 * is as a do-while loop. As in, check if the gamestate says we need to restart,
	 * and if so, restart the logic and check again
	 * Personally, I think this should have been a jump table for the different
	 * game state routines, indexed by a single game state variable.
	 * Ie. LDX _gameState : JMP (gameStates),X
	 * Much cleaner I think. Regardless, this will probably be a switch statement eventually.
	 */
	do {

		if (_restart == true) {
			restartLogic();
			_restart = false;
		}

		// This is the original logic, but I think it makes more sense if this were an else if statement
		if (_gameOverFlag == true) {
			gameOver();
			_gameOverFlag = false;
			_restart = true;

		} else if (_levelOver == true) {
			_themePaused = true;
			_levelOver = false;

			if (_level == (_maxLevels-1)) {
				//textPrint(kPrintYouWin);
				debug("YOU WIN");

			} else {
				//makeCertificate();
				//printCertificate();
				if (_level == 0) {
					//manual2();		// <- debug?
				}
				_promoting = 1;
			}
			_restart = true;

		} else {

			// Here's where the gameplay sequence actually happens!
			doSingleStep();				// Debug step function
			//monstRunAll();
			//objectRunAll();
			//doInfiniteHallways();
			//levelDrawAll();
			updateHitGauge();

			// What the heck? Check if we are in level 0: room 0, with no lit torches, and no projectiles
			// If so, dim the screen
			_dim = 0;
			if ((_level == 0) && (/*_currentLevel.getShowRoom()*/0 == 0) && (/*roomLighted()*/false == false) && (/*getNumBullets()*/ 0 == 0)) {
				_dim += 1;
			}

			if (_level == 7) {
				doGroan();
			}

			if (/*monstIsCombat(kPlayerID)*/true == true) {
				if (getPlaying() != kSongCombat) {
					playCombatSong();
				}
			} else {
				if (getPlaying() != kSongMaze) {
					playMazeSong();
				}
			}
		}

	} while (_restart == true);
}

void ImmortalEngine::trapKeys() {
	/* Weird name for a normal routine. It simply checks for the
	 * restart key (or button on the nes), or the sound toggle,
	 * (if debug mode is active it also checks for the
	 * _singleStep key), and then performs a high level action
	 * (on the NES it only checks restart, and it opens a dialog to do it)
	 */
	getInput();
	switch (_pressedAction) {
		case kActionDBGStep:
			_singleStep = true;
			break;
		case kActionRestart:
			gameOver();
			break;
		case kActionSound:
			toggleSound();
		default:
			break;
	}
}

void ImmortalEngine::doSingleStep() {
	/* This is a very cool debug routine. If you press the _singleStep key,
	 * the engine enters this debug mode where it waits for another key press.
	 * If the key is anything other than the _singleStep key, it will advance
	 * the engine one frame (or rather, until we hit this routine again, which
	 * should be one frame). If you hit the _singleStep key, it will exit the mode
	 * and advance normally again.
	 */
	if (_singleStep == true) {
		// If singleStep mode is active, stop the engine until we get input
		waitKey();
		// If the input is anything other than DGBStep, advance one frame
		if (_pressedAction == kActionDBGStep) {
			// Otherwise, we want to exit the mode
			_singleStep = false;
		}
	}
}

void ImmortalEngine::updateHitGauge() {
	/* This HUD (essentially) drawing routine is a bit weird because
	 * the game was originally meant to have multiple player characters
	 * in the room at once. So the engine sees the player as a 'monster'
	 * in the same way it sees enemies (and presumably would have seen other players).
	 * As such, this routine asks the room to ask the monster called player,
	 * what their health is. If the game considered the player unique, this would
	 * probably just check a global 'health' variable instead.
	 */
	//int h = _rooms[_currentRoom]._monsters[kPlayerID]._getHits();
	int hits = 0;
	if (hits != _lastGauge) {
		// Update the mirror value if the health has changed since last frame
		_lastGauge = hits;
		drawGauge(hits);
	}
}

void ImmortalEngine::drawGauge(int h) {
	/* Draw the health bar:
	 * We have two variables, the current health (number of hits remaining),
	 * and the difference betweeen the current health and max health (16).
	 * We then do some silly branching logic that is functionally the same
	 * as a for loop for the available health, and then another for unavailable health.
	 * But we could also write it much more efficiently like this:
	 * sta tmp : lda #$16 : tay : dey : sub tmp : tax
	 * -
	 * txa : beq +
	 * lda #$1 : dex
	 * +
	 * jsr draw
	 * dey : bpl -
	 * Ie. Loop over the entire bar, and once you run out of one icon to draw, that 0 becomes
	 * the index of the chr for the other icons.
	 */
	int r = 16 - h;
	_penX = kGaugeX;
	_penY = kGaugeY;
	h--;
	if (h >= 0) {
		// This could be written as a regular for loop, but the game thinks of start/stop as different from on
		printChr(kGaugeStart);
		h--;
		for (; h >= 0; h--) {
			if (h == 0) {
				// Redundant code is redundant
				printChr(kGaugeStop);

			} else {
				printChr(kGaugeOn);
			}
		}

	} else {
		// Oh hey, this one is indeed a normal for loop
		for (; r >= 0; r--) {
			printChr(kGaugeOff);
		}
	}
}

void ImmortalEngine::doGroan() {
	//getRandom();
}

bool ImmortalEngine::printAnd(const Common::String s) {
	// Just prints what it's given and then checks for input
	textPrint(s);
	getInput();
	if (_heldAction != kActionNothing) {
		return true;
	}
	return false;
}

bool ImmortalEngine::fromOldGame() {
	/*
	if (_titlesShown == 0) {
		_titlesShown++;
		_dontResetColors = 1;
		printAnd(kTitle0);
		printAnd(kTitle4);
		getInput();
		return false;
	}

	_dontResetColors = 0;
	if (_promoting == 1) {
		_promoting = 0;
	
	} else {

		do {
			if (!textPrint(kOldGameString)) {
				// They choose not to load an old game
				return false;			
			}
		} while (getCertificate());

		if (_lastCertLen == 0) {
			return false;
		}
	}

	_level = _cert + kCertLevel;
	setGameFlags(((_cert + kCertHiGameFlags) << 4) | (_cert + kCertLoGameFlags));
	uint16 hits = _cert + kCertHits;
	uint16 quick = _cert + kCertQuickness;
	uint16 gold = ((_cert + kCertGoldHi) << 4) | (_cert + kCertGoldLo);
	// monstMakePlayer(hits, quick, gold);	<- will become room.makePlayer();

	uint8 tmp = 3;
	uint8 flags = kObjIsRunning;
	uint8 frame;
	uint8 type;

	// room.makeObject(tmp, flags, gold, )*/
	return true;
}

void ImmortalEngine::setGameFlags(uint16 f) {

}

// There's no way this routine needs to still be here. In fact I'm not sure it needed to be in the game anyway?
int ImmortalEngine::getLevel() {
	return _level;
}

int ImmortalEngine::logicFreeze() {
	// Very silly way of checking if the level is over and/or the game is over
	int g = _gameOverFlag | _levelOver;
	return (g ^ 1) >> 1;
}

void ImmortalEngine::gameOverDisplay() {
	_themePaused = true;
	//text_print(kGameOverString)
	debug("GAME OVER");
}

void ImmortalEngine::gameOver() {
	_gameOverFlag = 1;
}

void ImmortalEngine::levelOver() {
	_levelOver = 1;
}

void ImmortalEngine::setSavedKing() {
	_gameFlags |= kSavedKing;
}

bool ImmortalEngine::isSavedKing() {
	if ((_gameFlags & kSavedKing) == 1) {
		return true;
	} else {
		return false;
	}
}

void ImmortalEngine::setSavedAna() {
	_gameFlags |= kSavedAna;
}

bool ImmortalEngine::isSavedAna() {
	if ((_gameFlags & kSavedAna) == 1) {
		return true;
	} else {
		return false;
	}
}

bool ImmortalEngine::getCertificate() {
	return true;
}

} // namespace Immortal



















