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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* TIMEOUT	The scheduling unit. */

#include "avalanche/avalanche.h"

#include "avalanche/timeout2.h"
#include "avalanche/visa2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/animation.h"
#include "avalanche/scrolls2.h"
#include "avalanche/acci2.h"
#include "avalanche/sequence2.h"
#include "avalanche/enid2.h"
#include "avalanche/pingo2.h"

#include "common/textconsole.h"

namespace Avalanche {

Timeout::Timeout(AvalancheEngine *vm) {
	_vm = vm;

	for (byte i = 0; i < 7; i++) {
		_times[i]._timeLeft = 0;
		_times[i]._action = 0;
		_times[i]._reason = 0;
	}
	_timerLost = false;
}

/**
 * Add a nex timer
 * @remarks	Originally called 'set_up_timer'
 */
void Timeout::addTimer(int32 duration, byte action, byte reason) {
	if ((_vm->_gyro->isLoaded == false) || (_timerLost == true)) {
		byte i = 0;
		while ((i < 7) && (_times[i]._timeLeft != 0))
			i++;

		if (i == 7)
			return; // Oh dear... No timer left

		// Everything's OK here!
		_times[i]._timeLeft = duration;
		_times[i]._action = action;
		_times[i]._reason = reason;
	} else {
		_vm->_gyro->isLoaded = false;
		return;
	}
}

/**
 * Update the timers
 * @remarks	Originally called 'one_tick'
 */
void Timeout::updateTimer() {
	if (_vm->_gyro->_dropdownActive)
		return;

	for (byte i = 0; i < 7; i++) {
		if (_times[i]._timeLeft <= 0)
			continue;

		_times[i]._timeLeft--;

		if (_times[i]._timeLeft == 0) {
			switch (_times[i]._action) {
			case kProcOpenDrawbridge :
				openDrawbridge();
				break;
			case kProcAvariciusTalks :
				avariciusTalks();
				break;
			case kProcUrinate :
				urinate();
				break;
			case kProcToilet :
				toilet();
				break;
			case kProcBang:
				bang();
				break;
			case kProcBang2:
				bang2();
				break;
			case kProcStairs:
				stairs();
				break;
			case kProcCardiffSurvey:
				cardiffSurvey();
				break;
			case kProcCardiffReturn:
				cardiffReturn();
				break;
			case kProcCwytalotInHerts:
				cwytalotInHerts();
				break;
			case kProcGetTiedUp:
				getTiedUp();
				break;
			case kProcGetTiedUp2:
				getTiedUp2();
				break;
			case kProcHangAround:
				hangAround();
				break;
			case kProcHangAround2:
				hangAround2();
				break;
			case kProcAfterTheShootemup:
				afterTheShootemup();
				break;
			case kProcJacquesWakesUp:
				jacquesWakesUp();
				break;
			case kProcNaughtyDuke:
				naughtyDuke();
				break;
			case kProcNaughtyDuke2:
				naughtyDuke2();
				break;
			case kProcNaughtyDuke3:
				naughtyDuke3();
				break;
			case kProcJump:
				jump();
				break;
			case kProcSequence:
				_vm->_sequence->callSequencer();
				break;
			case kProcCrapulusSpludOut:
				crapulusSaysSpludOut();
				break;
			case kProcDawnDelay:
				_vm->_lucerna->dawn();
				break;
			case kProcBuyDrinks:
				buyDrinks();
				break;
			case kProcBuyWine:
				buyWine();
				break;
			case kProcCallsGuards:
				callsGuards();
				break;
			case kProcGreetsMonk:
				greetsMonk();
				break;
			case kProcFallDownOubliette:
				fallDownOubliette();
				break;
			case kProcMeetAvaroid:
				meetAvaroid();
				break;
			case kProcRiseUpOubliette:
				riseUpOubliette();
				break;
			case kProcRobinHoodAndGeida:
				robinHoodAndGeida();
				break;
			case kProcRobinHoodAndGeidaTalk:
				robinHoodAndGeidaTalk();
				break;
			case kProcAvalotReturns:
				avalotReturns();
				break;
			case kProcAvvySitDown:
				avvySitDown();
				break;
			case kProcGhostRoomPhew:
				ghostRoomPhew();
				break;
			case kProcArkataShouts:
				arkataShouts();
				break;
			case kProcWinning:
				winning();
				break;
			case kProcAvalotFalls:
				avalotFalls();
				break;
			case kProcSpludwickGoesToCauldron:
				spludwickGoesToCauldron();
				break;
			case kProcSpludwickLeavesCauldron:
				spludwickLeavesCauldron();
				break;
			case kProcGiveLuteToGeida:
				giveLuteToGeida();
				break;
			}
		}
	}
	_vm->_gyro->_roomTime++; // Cycles since you've been in this room.
	_vm->_gyro->_dna._totalTime++; // Total amount of time for this game.
}

void Timeout::loseTimer(byte which) {
	for (byte i = 0; i < 7; i++) {
		if (_times[i]._reason == which)
			_times[i]._timeLeft = 0; // Cancel this one!
	}

	_timerLost = true;
}

void Timeout::openDrawbridge() {
	_vm->_gyro->_dna._drawbridgeOpen++;
	_vm->_celer->drawBackgroundSprite(-1, -1, _vm->_gyro->_dna._drawbridgeOpen - 1);

	if (_vm->_gyro->_dna._drawbridgeOpen == 4)
		_vm->_gyro->_magics[1]._operation = _vm->_gyro->kMagicNothing; // You may enter the drawbridge.
	else
		addTimer(7, kProcOpenDrawbridge, kReasonDrawbridgeFalls);
}

void Timeout::avariciusTalks() {
	_vm->_visa->dixi('q', _vm->_gyro->_dna._avariciusTalk);
	_vm->_gyro->_dna._avariciusTalk++;

	if (_vm->_gyro->_dna._avariciusTalk < 17)
		addTimer(177, kProcAvariciusTalks, kReasonAvariciusTalks);
	else
		_vm->_lucerna->incScore(3);
}

void Timeout::urinate() {
	_vm->_animation->tr[0].turn(Animation::kDirUp);
	_vm->_animation->stopWalking();
	_vm->_lucerna->drawDirection();
	addTimer(14, kProcToilet, kReasonGoToToilet);
}

void Timeout::toilet() {
	_vm->_scrolls->displayText("That's better!");
}

void Timeout::bang() {
	_vm->_scrolls->displayText(Common::String(_vm->_scrolls->kControlItalic) + "< BANG! >");
	addTimer(30, kProcBang2, kReasonExplosion);
}

void Timeout::bang2() {
	_vm->_scrolls->displayText("Hmm... sounds like Spludwick's up to something...");
}

void Timeout::stairs() {
	_vm->_gyro->blip();
	_vm->_animation->tr[0].walkto(4);
	_vm->_celer->drawBackgroundSprite(-1, -1, 2);
	_vm->_gyro->_dna._brummieStairs = 2;
	_vm->_gyro->_magics[10]._operation = _vm->_gyro->kMagicSpecial;
	_vm->_gyro->_magics[10]._data = 2; // Reached the bottom of the stairs.
	_vm->_gyro->_magics[3]._operation = _vm->_gyro->kMagicNothing; // Stop them hitting the sides (or the game will hang.)
}

void Timeout::cardiffSurvey() {
	if (_vm->_gyro->_dna._cardiffQuestionNum == 0) {
		_vm->_gyro->_dna._cardiffQuestionNum++;
		_vm->_visa->dixi('q', 27);
	}

	_vm->_visa->dixi('z', _vm->_gyro->_dna._cardiffQuestionNum);
	_vm->_gyro->_interrogation = _vm->_gyro->_dna._cardiffQuestionNum;
	addTimer(182, kProcCardiffSurvey, kReasonCardiffsurvey);
}

void Timeout::cardiffReturn() {
	_vm->_visa->dixi('q', 28);
	cardiffSurvey(); // Add end of question.
}

void Timeout::cwytalotInHerts() {
	_vm->_visa->dixi('q', 29);
}

void Timeout::getTiedUp() {
	_vm->_visa->dixi('q', 34); // ...Trouble!
	_vm->_gyro->_dna._userMovesAvvy = false;
	_vm->_gyro->_dna._beenTiedUp = true;
	_vm->_animation->stopWalking();
	_vm->_animation->tr[1].stopWalk();
	_vm->_animation->tr[1].stophoming();
	_vm->_animation->tr[1]._callEachStepFl = true;
	_vm->_animation->tr[1]._eachStepProc = _vm->_animation->kProcGrabAvvy;
	addTimer(70, kProcGetTiedUp2, kReasonGettingTiedUp);
}

void Timeout::getTiedUp2() {
	_vm->_animation->tr[0].walkto(4);
	_vm->_animation->tr[1].walkto(5);
	_vm->_gyro->_magics[3]._operation = _vm->_gyro->kMagicNothing; // No effect when you touch the boundaries.
	_vm->_gyro->_dna._friarWillTieYouUp = true;
}

void Timeout::hangAround() {
	_vm->_animation->tr[1]._doCheck = false;
	_vm->_animation->tr[0].init(7, true, _vm->_animation); // Robin Hood
	_vm->_gyro->_whereIs[_vm->_gyro->kPeopleRobinHood - 150] = r__robins;
	_vm->_animation->apped(1, 2);
	_vm->_visa->dixi('q', 39);
	_vm->_animation->tr[0].walkto(7);
	addTimer(55, kProcHangAround2, kReasonHangingAround);
}

void Timeout::hangAround2() {
	_vm->_visa->dixi('q', 40);
	_vm->_animation->tr[1]._vanishIfStill = false;
	_vm->_animation->tr[1].walkto(4);
	_vm->_gyro->_whereIs[_vm->_gyro->kPeopleFriarTuck - 150] = r__robins;
	_vm->_visa->dixi('q', 41);
	_vm->_animation->tr[0].done();
	_vm->_animation->tr[1].done(); // Get rid of Robin Hood and Friar Tuck.

	addTimer(1, kProcAfterTheShootemup, kReasonHangingAround);
	// Immediately call the following proc (when you have a chance).

	_vm->_gyro->_dna._tiedUp = false;

	_vm->_enid->backToBootstrap(1); // Call the shoot-'em-up.
}

void Timeout::afterTheShootemup() {

	_vm->_animation->fliproom(_vm->_gyro->_dna._room, 0);
	// Only placed this here to replace the minigame. TODO: Remove it when the shoot em' up is implemented!

	_vm->_animation->tr[0].init(0, true, _vm->_animation); // Avalot.
	_vm->_animation->apped(1, 2);
	_vm->_gyro->_dna._userMovesAvvy = true;
	_vm->_gyro->_dna._objects[_vm->_gyro->kObjectCrossbow - 1] = true;
	_vm->_lucerna->refreshObjectList();

	// Same as the added line above: TODO: Remove it later!!!
	_vm->_scrolls->displayText(Common::String("P.S.: There should have been the mini-game called \"shoot em' up\", but I haven't implemented it yet: you get the crossbow automatically.")
		+ _vm->_scrolls->kControlNewLine + _vm->_scrolls->kControlNewLine + "Peter (uruk)");

#if 0
	byte shootscore, gain;

	shootscore = mem[storage_seg * storage_ofs];
	gain = (shootscore + 5) / 10; // Rounding up.

	display(string("\6Your score was ") + strf(shootscore) + '.' + "\r\rYou gain (" +
		strf(shootscore) + " 0xF6 10) = " + strf(gain) + " points.");

	if (gain > 20) {
		display("But we won't let you have more than 20 points!");
		points(20);
	} else
		points(gain);
#endif

	warning("STUB: Timeout::after_the_shootemup()");

	_vm->_visa->dixi('q', 70);
}

void Timeout::jacquesWakesUp() {
	_vm->_gyro->_dna._jacquesState++;

	switch (_vm->_gyro->_dna._jacquesState) { // Additional pictures.
	case 1 :
		_vm->_celer->drawBackgroundSprite(-1, -1, 1); // Eyes open.
		_vm->_visa->dixi('Q', 45);
		break;
	case 2 : // Going through the door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 2); // Not on the floor.
		_vm->_celer->drawBackgroundSprite(-1, -1, 3); // But going through the door.
		_vm->_gyro->_magics[5]._operation = _vm->_gyro->kMagicNothing; // You can't wake him up now.
		break;
	case 3 :  // Gone through the door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 2); // Not on the floor, either.
		_vm->_celer->drawBackgroundSprite(-1, -1, 4); // He's gone... so the door's open.
		_vm->_gyro->_whereIs[_vm->_gyro->kPeopleJacques - 150] = 0; // Gone!
		break;
	}

	if (_vm->_gyro->_dna._jacquesState == 5) {
		_vm->_gyro->_dna._bellsAreRinging = true;
		_vm->_gyro->_dna._aylesIsAwake = true;
		_vm->_lucerna->incScore(2);
	}

	switch (_vm->_gyro->_dna._jacquesState) {
	case 1:
	case 2:
	case 3:
		addTimer(12, kProcJacquesWakesUp, kReasonJacquesWakingUp);
		break;
	case 4:
		addTimer(24, kProcJacquesWakesUp, kReasonJacquesWakingUp);
		break;
	}
}

void Timeout::naughtyDuke() { // This is when the Duke comes in and takes your money.
	_vm->_animation->tr[1].init(9, false, _vm->_animation); // Here comes the Duke.
	_vm->_animation->apped(2, 1); // He starts at the door...
	_vm->_animation->tr[1].walkto(3); // He walks over to you.

	// Let's get the door opening.
	_vm->_celer->drawBackgroundSprite(-1, -1, 1);
	_vm->_sequence->firstShow(2);
	_vm->_sequence->startToClose();

	addTimer(50, kProcNaughtyDuke2, kReasonNaughtyDuke);
}

void Timeout::naughtyDuke2() {
	_vm->_visa->dixi('q', 48); // "Ha ha, it worked again!"
	_vm->_animation->tr[1].walkto(1); // Walk to the door.
	_vm->_animation->tr[1]._vanishIfStill = true; // Then go away!
	addTimer(32, kProcNaughtyDuke3, kReasonNaughtyDuke);
}

void Timeout::naughtyDuke3() {
	_vm->_celer->drawBackgroundSprite(-1, -1, 1);
	_vm->_sequence->firstShow(2);
	_vm->_sequence->startToClose();
}

void Timeout::jump() {
	_vm->_gyro->_dna._jumpStatus++;

	switch (_vm->_gyro->_dna._jumpStatus) {
	case 1:
	case 2:
	case 3:
	case 5:
	case 7:
	case 9:
		_vm->_animation->tr[0]._y--;
		break;
	case 12:
	case 13:
	case 14:
	case 16:
	case 18:
	case 19:
		_vm->_animation->tr[0]._y++;
		break;
	}

	if (_vm->_gyro->_dna._jumpStatus == 20) { // End of jump.
		_vm->_gyro->_dna._userMovesAvvy = true;
		_vm->_gyro->_dna._jumpStatus = 0;
	} else { // Still jumping.
		addTimer(1, kProcJump, kReasonJumping);
	}

	if ((_vm->_gyro->_dna._jumpStatus == 10) // You're at the highest point of your jump.
			&& (_vm->_gyro->_dna._room == r__insidecardiffcastle)
			&& (_vm->_gyro->_dna._arrowInTheDoor == true)
			&& (_vm->_animation->infield(3))) { // Beside the wall
		// Grab the arrow!
		if (_vm->_gyro->_dna._carryNum >= kCarryLimit)
			_vm->_scrolls->displayText("You fail to grab it, because your hands are full.");
		else {
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_gyro->_dna._arrowInTheDoor = false; // You've got it.
			_vm->_gyro->_dna._objects[_vm->_gyro->kObjectBolt - 1] = true;
			_vm->_lucerna->refreshObjectList();
			_vm->_visa->dixi('q', 50);
			_vm->_lucerna->incScore(3);
		}
	}
}

void Timeout::crapulusSaysSpludOut() {
	_vm->_visa->dixi('q', 56);
	_vm->_gyro->_dna._crapulusWillTell = false;
}

void Timeout::buyDrinks() {
	_vm->_celer->drawBackgroundSprite(-1, -1, 11); // Malagauche gets up again.
	_vm->_gyro->_dna._malagauche = 0;

	_vm->_visa->dixi('D', _vm->_gyro->_dna._drinking); // Display message about it.
	_vm->_pingo->wobble(); // Do the special effects.
	_vm->_visa->dixi('D', 1); // That'll be thruppence.
	if (_vm->_gyro->decreaseMoney(3)) // Pay 3d.
		_vm->_visa->dixi('D', 3); // Tell 'em you paid up.
	_vm->_acci->drink();
}

void Timeout::buyWine() {
	_vm->_celer->drawBackgroundSprite(-1, -1, 11); // Malagauche gets up again.
	_vm->_gyro->_dna._malagauche = 0;

	_vm->_visa->dixi('D', 50); // You buy the wine.
	_vm->_visa->dixi('D', 1); // It'll be thruppence.
	if (_vm->_gyro->decreaseMoney(3)) {
		_vm->_visa->dixi('D', 4); // You paid up.
		_vm->_gyro->_dna._objects[_vm->_gyro->kObjectWine - 1] = true;
		_vm->_lucerna->refreshObjectList();
		_vm->_gyro->_dna._wineState = 1; // OK Wine.
	}
}

void Timeout::callsGuards() {
	_vm->_visa->dixi('Q', 58); // "GUARDS!!!"
	_vm->_lucerna->gameOver();
}

void Timeout::greetsMonk() {
	_vm->_visa->dixi('Q', 59);
	_vm->_gyro->_dna._enteredLustiesRoomAsMonk = true;
}

void Timeout::fallDownOubliette() {
	_vm->_gyro->_magics[8]._operation = _vm->_gyro->kMagicNothing;
	_vm->_animation->tr[0]._moveY++; // Increments dx/dy!
	_vm->_animation->tr[0]._y += _vm->_animation->tr[0]._moveY;   // Dowwwn we go...
	addTimer(3, kProcFallDownOubliette, kReasonFallingDownOubliette);
}

void Timeout::meetAvaroid() {
	if (_vm->_gyro->_dna._metAvaroid) {
		_vm->_scrolls->displayText(Common::String("You can't expect to be ") + _vm->_scrolls->kControlItalic + "that"
			+ _vm->_scrolls->kControlRoman + " lucky twice in a row!");
		_vm->_lucerna->gameOver();
	} else {
		_vm->_visa->dixi('Q', 60);
		_vm->_gyro->_dna._metAvaroid = true;
		addTimer(1, kProcRiseUpOubliette, kReasonRisingUpOubliette);

		_vm->_animation->tr[0]._facingDir = Animation::kDirLeft;
		_vm->_animation->tr[0]._x = 151;
		_vm->_animation->tr[0]._moveX = -3;
		_vm->_animation->tr[0]._moveY = -5;

		_vm->_gyro->setBackgroundColor(2);
	}
}

void Timeout::riseUpOubliette() {
	_vm->_animation->tr[0]._visible = true;
	_vm->_animation->tr[0]._moveY++; // Decrements dx/dy!
	_vm->_animation->tr[0]._y -= _vm->_animation->tr[0]._moveY; // Uuuupppp we go...
	if (_vm->_animation->tr[0]._moveY > 0)
		addTimer(3, kProcRiseUpOubliette, kReasonRisingUpOubliette);
	else
		_vm->_gyro->_dna._userMovesAvvy = true;
}

void Timeout::robinHoodAndGeida() {
	_vm->_animation->tr[0].init(7, true, _vm->_animation);
	_vm->_animation->apped(1, 7);
	_vm->_animation->tr[0].walkto(6);
	_vm->_animation->tr[1].stopWalk();
	_vm->_animation->tr[1]._facingDir = Animation::kDirLeft;
	addTimer(20, kProcRobinHoodAndGeidaTalk, kReasonRobinHoodAndGeida);
	_vm->_gyro->_dna._geidaFollows = false;
}

void Timeout::robinHoodAndGeidaTalk() {
	_vm->_visa->dixi('q', 66);
	_vm->_animation->tr[0].walkto(2);
	_vm->_animation->tr[1].walkto(2);
	_vm->_animation->tr[0]._vanishIfStill = true;
	_vm->_animation->tr[1]._vanishIfStill = true;
	addTimer(162, kProcAvalotReturns, kReasonRobinHoodAndGeida);
}

void Timeout::avalotReturns() {
	_vm->_animation->tr[0].done();
	_vm->_animation->tr[1].done();
	_vm->_animation->tr[0].init(0, true, _vm->_animation);
	_vm->_animation->apped(1, 1);
	_vm->_visa->dixi('q', 67);
	_vm->_gyro->_dna._userMovesAvvy = true;
}

/**
 * This is used when you sit down in the pub in Notts. It loops around
 * so that it will happen when Avvy stops walking.
 * @remarks	Originally called 'avvy_sit_down'
 */
void Timeout::avvySitDown() {
	if (_vm->_animation->tr[0]._homing)    // Still walking.
		addTimer(1, kProcAvvySitDown, kReasonSittingDown);
	else {
		_vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_gyro->_dna._sittingInPub = true;
		_vm->_gyro->_dna._userMovesAvvy = false;
		_vm->_animation->tr[0]._visible = false;
	}
}

void Timeout::ghostRoomPhew() {
	_vm->_scrolls->displayText(Common::String(_vm->_scrolls->kControlItalic) + "PHEW!" + _vm->_scrolls->kControlRoman
		+ " You're glad to get out of " + _vm->_scrolls->kControlItalic + "there!");
}

void Timeout::arkataShouts() {
	if (_vm->_gyro->_dna._teetotal)
		return;

	_vm->_visa->dixi('q', 76);
	addTimer(160, kProcArkataShouts, kReasonArkataShouts);
}

void Timeout::winning() {
	_vm->_visa->dixi('q', 79);
	_vm->_pingo->winningPic();

	warning("STUB: Timeout::winning()");
#if 0
	do {
		_vm->_lucerna->checkclick();
	} while (!(_vm->_gyro->mrelease == 0));
#endif
	// TODO: To be implemented with Pingo::winningPic().

	_vm->_lucerna->callVerb(_vm->_acci->kVerbCodeScore);
	_vm->_scrolls->displayText(" T H E    E N D ");
	_vm->_gyro->_letMeOut = true;
}

void Timeout::avalotFalls() {
	if (_vm->_animation->tr[0]._stepNum < 5) {
		_vm->_animation->tr[0]._stepNum++;
		addTimer(3, kProcAvalotFalls, kReasonFallingOver);
	} else {
		Common::String toDisplay;
		for (byte i = 0; i < 6; i++)
			toDisplay += _vm->_scrolls->kControlNewLine;
		for (byte i = 0; i < 6; i++)
			toDisplay += _vm->_scrolls->kControlInsertSpaces;
		toDisplay = toDisplay + _vm->_scrolls->kControlRegister + 'Z' + _vm->_scrolls->kControlIcon;
		_vm->_scrolls->displayText(toDisplay);
	}
}

void Timeout::spludwickGoesToCauldron() {
	if (_vm->_animation->tr[1]._homing)
		addTimer(1, kProcSpludwickGoesToCauldron, kReasonSpludWalk);
	else
		addTimer(17, kProcSpludwickLeavesCauldron, kReasonSpludWalk);
}

void Timeout::spludwickLeavesCauldron() {
	_vm->_animation->tr[1]._callEachStepFl = true; // So that normal procs will continue.
}

void Timeout::giveLuteToGeida() { // Moved here from Acci.
	_vm->_visa->dixi('Q', 86);
	_vm->_lucerna->incScore(4);
	_vm->_gyro->_dna._lustieIsAsleep = true;
	_vm->_sequence->firstShow(5);
	_vm->_sequence->thenShow(6); // He falls asleep...
	_vm->_sequence->startToClose(); // Not really closing, but we're using the same procedure.
}

} // End of namespace Avalanche.
