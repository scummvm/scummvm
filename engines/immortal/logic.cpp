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

#include "immortal/room.h"
#include "immortal/immortal.h"

namespace Immortal {

int ImmortalEngine::logicFreeze() {
	// Very silly way of checking if the level is over and/or the game is over
	int g = _gameOverFlag | _levelOver;
	return (g ^ 1) >> 1;
}

void ImmortalEngine::logicInit() {
	_titlesShown = 0;
	_time = 0;
	_promoting = 0;
	_restart = true;
	levelInitAtStartOfGameOnly();
	_lastCertLen = 0;
}

void ImmortalEngine::restartLogic() {
	_singleStep = false;
	_levelOver = false;
	_gameFlags = kSavedNone;

	// Here's where the majority of the game actually gets initialized
	miscInit();
	cycleFreeAll();
	levelInit();
	//roomInit();		<-- will be run in constructor of room
	//monstInit();		<-- room.initMonsters()		\
	//objectInit();		<-- room.initObjects()
	//doorInit();		<-- room.initDoors()		 |- probably all get run from room constructor
	//sparkInit();		<-- room.initSparks()
	//bulletInit();		<-- room.initProjectiles()	/
	//objectInit(); 	<-- again? Odd...
	//genericSpriteInit();	<-- room.initGenSprites()

	if (fromOldGame() == false) {
		_level = 0;
		levelNew(_level);
	}

	_rooms[_currentRoom]->flameInit();

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
				textPrint(kStrYouWin, 0);

			} else {
				makeCertificate();
				printCertificate();
				_promoting = 1;
			}
			_restart = true;

		} else {

			// Here's where the gameplay sequence actually happens!
			doSingleStep();				// Debug step function
			//monstRunAll();
			//objectRunAll();
			//doInfiniteHallways();
			levelDrawAll();
			updateHitGauge();

			_dim = 0;
			if ((_level == 0) && (/*_currentLevel.getShowRoom()*/0 == 0) && (_rooms[_currentRoom]->roomLighted() == false) && (/*getNumBullets()*/ 0 == 0)) {
				_dim += 1;
			}

			if (_level == 7) {
				doGroan();
			}

			if (/*monstIsCombat(kPlayerID)*/false == true) {
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

int ImmortalEngine::keyOrButton() {
	// Returns a key if a key was pressed, or 13 if a button was pressed

	int button = 0;
	while (button == 0) {
		getInput();
		switch (_pressedAction) {
			case kActionKey:
				button = _pressedAction;
			case kActionFire:
			case kActionButton:
				button = 13;
			default:
				break;
		}
	}
	return button;
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
	//int hits = _rooms[_currentRoom]._monsters[kPlayerID]._getHits();
	int hits = 15;
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
	setPen(kGaugeX, kGaugeY);
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

bool ImmortalEngine::printAnd(Str s) {
	// Only ever used by fromOldGame()
	// Just prints what it's given and then checks for input
	textPrint(s, 0);
	getInput();
	if (_heldAction != kActionNothing) {
		return true;
	}
	return false;
}

bool ImmortalEngine::fromOldGame() {
	/* This is the basic load game routine (and also title display).
	 * It lets the user enter a password, or start a new game.
	 * Either way it sets up the inventory for the level, and also
	 * various object related things for the specific level.
	 */
	if (_titlesShown == 0) {
		_titlesShown++;
		_dontResetColors = 1;
		printAnd(kStrTitle0);
		printAnd(kStrTitle4);
		getInput();
		return false;
	}

	_dontResetColors = 0;
	if (_promoting == 1) {
		_promoting = 0;
	
	} else {

		do {
			if (!textPrint(kStrOldGame, 0)) {
				// They choose not to load an old game
				return false;			
			}
		} while (getCertificate() == true);

		if (_lastCertLen == 0) {
			return false;
		}
	}

	// Set game flags
	_level = _certificate[kCertLevel];

	setGameFlags((_certificate[kCertHiGameFlags] << 4) | _certificate[kCertLoGameFlags]);

	// Create the player

	//uint8 hits  = _certificate[kCertHits];
	//uint8 quick = _certificate[kCertQuickness];
	//uint8 gold  = (_certificate[kCertGoldHi] << 4) | _certificate[kCertGoldLo];
	// monstMakePlayer(hits, quick, gold);	<- will become room.makePlayer();

	// Create the inventory
	// room.makeObject(3, kObjIsRunning, 0, goldType);

	// Hi bits of inventory
	int certInv = _certificate[kCertInvHi];

	if ((certInv & 1) != 0 ) {
		if (_level < 2) {
			//room.makeObject(3, 0, 0, waterType);
		}
	}

	if ((certInv & 2) != 0) {
		//room.makeObject(3, 0, kRingFrame, dunRingType);
	}

	if ((certInv & 4) != 0) {
		if (_level < 6) {
			//room.makeObject(3, 0, kSporesFrame, wormFoodType);
		}
	}

	if ((certInv & 8) != 0) {
		//room.makeObject(3, 0, 0 (?), coinType);
	}


	// Low bits of inventory
	certInv = _certificate[kCertInvLo];

	// This would have been much more clean as a set of tables instead of a long branching tree
	switch (_certificate[kCertLevel]) {
		case 1:
			if ((certInv & 2) != 0) {
				//room.makeObject(3, 0, kSporesFrame, sporesType);
			}

			if ((certInv & 4) != 0) {
				//room.makeObject(3, 0, kSporesFrame, wowCharmType);
			}

			break;
		case 4:
			if ((certInv & 2) != 0) {
				//room.makeObject(3, kIsInvisible, kSporesFrame, coffeeType);
			}

			break;
		case 3:
			if ((certInv & 1) != 0) {
				//room.makeObject(3, kIsRunning, kRingFrame, faceRingType);
			}

			break;
		case 7:
			if ((certInv & 1) != 0) {
				//room.makeObject(6, kUsesFireButton, kSporesFrame, bronzeType);
			}

			if ((certInv & 2) != 0) {
				//room.makeObject(3, 0, kSporesFrame, tractorType);
			}

			if ((certInv & 4) != 0) {
				//room.makeObject(3, 0, kSporesFrame, antiType);
			}

		default:
			break;
	}
	levelNew(_level);
	return true;
}

void ImmortalEngine::makeCertificate() {
	/* The code for this bit doesn't really make sense,
	 * so I will write it as it is, but I am noting here
	 * that it should be:
	 * jsr monst_getGold : ... sta certificate+certgoldhi
	 * jsr monst_getQuickness : sta certificate+certquickness
	 * instead of getquickness : get gold : sta gold : sta quickness
	 * also no need to ldx 0 since this is player only ram right?
	 */

	//uint8 q = room._playerQuickness
	//uint16 g = room._playerGold
	uint16 g = 0;

	_certificate[kCertGoldLo] = g & 0xf;
	_certificate[kCertGoldHi] = g >> 4;
	_certificate[kCertQuickness] = g >> 4; // Should actually be = q, but this is what the game does

	_certificate[kCertHits] = 0; //room._playerHits
	_certificate[kCertLoGameFlags] = getGameFlags() & 0xf;
	_certificate[kCertLoGameFlags] = getGameFlags() >> 4;

	_certificate[kCertLevel] = _level + 1;
	_certificate[kCertInvLo] = 0;
	_certificate[kCertInvHi] = 0;

	if (true/*room.monster[kPlayerID].hasObject(waterType)*/) {
		_certificate[kCertInvHi] |= 1;
	}

	if (true/*room.monster[kPlayerID].hasObject(dunRingType)*/) {
		_certificate[kCertInvHi] |= 2;
	}

	if (true/*room.monster[kPlayerID].hasObject(wormFoodType)*/) {
		_certificate[kCertInvHi] |= 4;
	}

	if (true/*room.monster[kPlayerID].hasObject(coinType)*/) {
		_certificate[kCertInvHi] |= 8;
	}

	// The lo byte of the inventory is used for items that only exist on a specific level, and are removed after
	switch (_certificate[kCertLevel]) {
		case 1:
			if (true/*room.monster[kPlayerID].hasObject(sporesType)*/) {
				_certificate[kCertInvLo] |= 2;
			}

			if (true/*room.monster[kPlayerID].hasObject(wowCharmType)*/) {
				_certificate[kCertInvLo] |= 4;
			}

		case 3:
			if (true/*room.monster[kPlayerID].hasObject(faceRingType)*/) {
				_certificate[kCertInvLo] |= 1;
			}

		case 4:
			if (true/*room.monster[kPlayerID].hasObject(coffeeType)*/) {
				_certificate[kCertInvLo] |= 2;
			}

		case 7:
			if (true/*room.monster[kPlayerID].hasObject(bronzeType)*/) {
				_certificate[kCertInvLo] |= 1;
			}

			if (true/*room.monster[kPlayerID].hasObject(tractorType)*/) {
				_certificate[kCertInvLo] |= 2;
			}

			if (true/*room.monster[kPlayerID].hasObject(antiType)*/) {
				_certificate[kCertInvLo] |= 4;
			}

		default:
			_lastCertLen = 13;
			uint8 checksum[4];
			calcCheckSum(_lastCertLen, checksum);
			_certificate[0] = checksum[0];
			_certificate[1] = checksum[1];
			_certificate[2] = checksum[2];
			_certificate[3] = checksum[3];
	}
}

void ImmortalEngine::calcCheckSum(int l, uint8 checksum[]) {
	checksum[0] = 4;
	checksum[1] = 0xa5;

	/* The game logic seems to allow a len of 4 (cmp 4 : bcc),
	 * but the checksum iny before it checks if the sizes are the same,
	 * so shouldn't a cert of len 4 cause it to loop 0xfffc times?
	 */
	for (int i = 4; i <= l; i++) {
		checksum[0] = (_certificate[i] + checksum[0]) ^ checksum[1];
		checksum[1] = (_certificate[i] << 1) + checksum[1];
	}

	checksum[3] = checksum[1] >> 4;
	checksum[2] = checksum[1] & 0xf;
	checksum[1] = checksum[0] >> 4;
	checksum[0] = checksum[0] & 0xf;
}

bool ImmortalEngine::getCertificate() {
	textPrint(kStrCert, 0);
	int certLen = 0;
	bool entered = false;
	int k = 0;

	// My goodness the logic for this is a mess.
	while (entered == false) {
		k = keyOrButton();
		if (k == 13) {
			entered = true;

		} else if (k == 0x7f) {
			// The input was a backspace
			if (certLen != 0) {
				certLen--;				// Length is one smaller now
				backspace();			// move the drawing position back and reprint the '-' char
				backspace();
				printChr('-');
			}			

		} else {
			// The input was a key
			if (certLen != kMaxCertificate) {
				if ((k >= 'a') && (k < '{')) {
					k -= 0x20;
				}

				if (k >= '0') {
					if (k < ('9' + 1)) {
						k -= '0';
					}

					else {
						if (k < 'A') {
							continue;
						}

						if (k < ('F' + 1)) {
							k -= ('A' - 10);
						}
					}

					int certK = k;
					if ((k < ('Z' + 1)) && (k >= 'A')) {
						k += ('a' - 'A');
					}
					backspace();
					printChr(k);
					printChr('-');
					_certificate[certLen] = certK;
					certLen++;
				}
			}
		}
	}

	// Input of certificate is finished
	if (certLen == 0) {
		certLen = _lastCertLen;
	}
	if (certLen != 0) {
		if (certLen < 4) {
			textPrint(kStrBadCertificate, 0);
			return false;
		}
		uint8 checksum[4];
		calcCheckSum(certLen, checksum);
		for (int i = 0; i < 4; i++) {
			if (checksum[i] != _certificate[i]) {
				textPrint(kStrBadCertificate, 0);
				return false;
			}
		}
	}

	// Cert is good
	_lastCertLen = certLen;
	return true;
}

void ImmortalEngine::printCertificate() {
	/* In contrast to the other certificate routines,
	 * this one is nice and simple. You could also
	 * just add the appropriate offset for the letters,
	 * but grabbing it from a table is faster and doesn't
	 * use a lot of space (especially if it's used anywhere else)
	 */
	char toHex[] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	textBeginning(kStrCert, 0);
	for (int i = 0; i < _lastCertLen; i++) {
		printChr(toHex[_certificate[i]]);
	}
	textEnd(kStrCert2, 0);
}

bool ImmortalEngine::isSavedKing() {
	if ((_gameFlags & kSavedKing) == 1) {
		return true;
	} else {
		return false;
	}
}

bool ImmortalEngine::isSavedAna() {
	if ((_gameFlags & kSavedAna) == 1) {
		return true;
	} else {
		return false;
	}
}


/*
 * These functions don't really need to be functions
 */

void ImmortalEngine::setGameFlags(uint16 f) {
	_gameFlags = f;
}

uint16 ImmortalEngine::getGameFlags() {
	return _gameFlags;
}

int ImmortalEngine::getLevel() {
	return _level;
}

void ImmortalEngine::gameOverDisplay() {
	_themePaused = true;
	textPrint(kStrGameOver, 0);
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

void ImmortalEngine::setSavedAna() {
	_gameFlags |= kSavedAna;
}


/*
 * Not relevant yet (music)
 */

void ImmortalEngine::doGroan() {
	//getRandom();
}


} // namespace Immortal
