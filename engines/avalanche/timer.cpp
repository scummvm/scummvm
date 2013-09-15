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

/* Original name: TIMEOUT	The scheduling unit. */

#include "avalanche/avalanche.h"

#include "avalanche/timer.h"
#include "avalanche/lucerna.h"
#include "avalanche/animation.h"
#include "avalanche/scrolls.h"
#include "avalanche/acci.h"
#include "avalanche/sequence.h"
#include "avalanche/pingo.h"

#include "common/textconsole.h"

namespace Avalanche {

Timer::Timer(AvalancheEngine *vm) {
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
void Timer::addTimer(int32 duration, byte action, byte reason) {
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
void Timer::updateTimer() {
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
	_vm->_gyro->_totalTime++; // Total amount of time for this game.
}

void Timer::loseTimer(byte which) {
	for (byte i = 0; i < 7; i++) {
		if (_times[i]._reason == which)
			_times[i]._timeLeft = 0; // Cancel this one!
	}

	_timerLost = true;
}

void Timer::openDrawbridge() {
	_vm->_gyro->_drawbridgeOpen++;
	_vm->_celer->drawBackgroundSprite(-1, -1, _vm->_gyro->_drawbridgeOpen - 1);

	if (_vm->_gyro->_drawbridgeOpen == 4)
		_vm->_gyro->_magics[1]._operation = Gyro::kMagicNothing; // You may enter the drawbridge.
	else
		addTimer(7, kProcOpenDrawbridge, kReasonDrawbridgeFalls);
}

void Timer::avariciusTalks() {
	_vm->_scrolls->displayScrollChain('q', _vm->_gyro->_avariciusTalk);
	_vm->_gyro->_avariciusTalk++;

	if (_vm->_gyro->_avariciusTalk < 17)
		addTimer(177, kProcAvariciusTalks, kReasonAvariciusTalks);
	else
		_vm->_lucerna->incScore(3);
}

void Timer::urinate() {
	_vm->_animation->_sprites[0].turn(Animation::kDirUp);
	_vm->_animation->stopWalking();
	_vm->_lucerna->drawDirection();
	addTimer(14, kProcToilet, kReasonGoToToilet);
}

void Timer::toilet() {
	_vm->_scrolls->displayText("That's better!");
}

void Timer::bang() {
	Common::String tmpStr = Common::String::format("%c< BANG! >", Scrolls::kControlItalic);
	_vm->_scrolls->displayText(tmpStr);
	addTimer(30, kProcBang2, kReasonExplosion);
}

void Timer::bang2() {
	_vm->_scrolls->displayText("Hmm... sounds like Spludwick's up to something...");
}

void Timer::stairs() {
	_vm->_gyro->blip();
	_vm->_animation->_sprites[0].walkTo(3);
	_vm->_celer->drawBackgroundSprite(-1, -1, 2);
	_vm->_gyro->_brummieStairs = 2;
	_vm->_gyro->_magics[10]._operation = Gyro::kMagicSpecial;
	_vm->_gyro->_magics[10]._data = 2; // Reached the bottom of the stairs.
	_vm->_gyro->_magics[3]._operation = Gyro::kMagicNothing; // Stop them hitting the sides (or the game will hang.)
}

void Timer::cardiffSurvey() {
	if (_vm->_gyro->_cardiffQuestionNum == 0) {
		_vm->_gyro->_cardiffQuestionNum++;
		_vm->_scrolls->displayScrollChain('q', 27);
	}

	_vm->_scrolls->displayScrollChain('z', _vm->_gyro->_cardiffQuestionNum);
	_vm->_gyro->_interrogation = _vm->_gyro->_cardiffQuestionNum;
	addTimer(182, kProcCardiffSurvey, kReasonCardiffsurvey);
}

void Timer::cardiffReturn() {
	_vm->_scrolls->displayScrollChain('q', 28);
	cardiffSurvey(); // Add end of question.
}

void Timer::cwytalotInHerts() {
	_vm->_scrolls->displayScrollChain('q', 29);
}

void Timer::getTiedUp() {
	_vm->_scrolls->displayScrollChain('q', 34); // ...Trouble!
	_vm->_gyro->_userMovesAvvy = false;
	_vm->_gyro->_beenTiedUp = true;
	_vm->_animation->stopWalking();

	AnimationType *spr = &_vm->_animation->_sprites[1];
	spr->stopWalk();
	spr->stopHoming();
	spr->_callEachStepFl = true;
	spr->_eachStepProc = Animation::kProcGrabAvvy;
	addTimer(70, kProcGetTiedUp2, kReasonGettingTiedUp);
}

void Timer::getTiedUp2() {
	_vm->_animation->_sprites[0].walkTo(3);
	_vm->_animation->_sprites[1].walkTo(4);
	_vm->_gyro->_magics[3]._operation = Gyro::kMagicNothing; // No effect when you touch the boundaries.
	_vm->_gyro->_friarWillTieYouUp = true;
}

void Timer::hangAround() {
	_vm->_animation->_sprites[1]._doCheck = false;

	AnimationType *avvy = &_vm->_animation->_sprites[0];
	avvy->init(7, true, _vm->_animation); // Robin Hood
	_vm->_gyro->_whereIs[Gyro::kPeopleRobinHood - 150] = r__robins;
	_vm->_animation->appearPed(1, 1);
	_vm->_scrolls->displayScrollChain('q', 39);
	avvy->walkTo(6);
	addTimer(55, kProcHangAround2, kReasonHangingAround);
}

void Timer::hangAround2() {
	_vm->_scrolls->displayScrollChain('q', 40);
	AnimationType *spr = &_vm->_animation->_sprites[1];
	spr->_vanishIfStill = false;
	spr->walkTo(3);
	_vm->_gyro->_whereIs[Gyro::kPeopleFriarTuck - 150] = r__robins;
	_vm->_scrolls->displayScrollChain('q', 41);
	_vm->_animation->_sprites[0].remove();
	spr->remove(); // Get rid of Robin Hood and Friar Tuck.

	addTimer(1, kProcAfterTheShootemup, kReasonHangingAround);
	// Immediately call the following proc (when you have a chance).

	_vm->_gyro->_tiedUp = false;

	// _vm->_enid->backToBootstrap(1); Call the shoot-'em-up. TODO: Replace it with proper ScummVM-friendly function(s)! Do not remove until then!
}

void Timer::afterTheShootemup() {

	_vm->_animation->flipRoom(_vm->_gyro->_room, 0);
	// Only placed this here to replace the minigame. TODO: Remove it when the shoot em' up is implemented!

	_vm->_animation->_sprites[0].init(0, true, _vm->_animation); // Avalot.
	_vm->_animation->appearPed(1, 1);
	_vm->_gyro->_userMovesAvvy = true;
	_vm->_gyro->_objects[Gyro::kObjectCrossbow - 1] = true;
	_vm->_lucerna->refreshObjectList();

	// Same as the added line above: TODO: Remove it later!!!
	_vm->_scrolls->displayText(Common::String("P.S.: There should have been the mini-game called \"shoot em' up\", but I haven't implemented it yet: you get the crossbow automatically.")
		+ Scrolls::kControlNewLine + Scrolls::kControlNewLine + "Peter (uruk)");

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

	warning("STUB: Timer::after_the_shootemup()");

	_vm->_scrolls->displayScrollChain('q', 70);
}

void Timer::jacquesWakesUp() {
	_vm->_gyro->_jacquesState++;

	switch (_vm->_gyro->_jacquesState) { // Additional pictures.
	case 1 :
		_vm->_celer->drawBackgroundSprite(-1, -1, 1); // Eyes open.
		_vm->_scrolls->displayScrollChain('Q', 45);
		break;
	case 2 : // Going through the door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 2); // Not on the floor.
		_vm->_celer->drawBackgroundSprite(-1, -1, 3); // But going through the door.
		_vm->_gyro->_magics[5]._operation = Gyro::kMagicNothing; // You can't wake him up now.
		break;
	case 3 :  // Gone through the door.
		_vm->_celer->drawBackgroundSprite(-1, -1, 2); // Not on the floor, either.
		_vm->_celer->drawBackgroundSprite(-1, -1, 4); // He's gone... so the door's open.
		_vm->_gyro->_whereIs[Gyro::kPeopleJacques - 150] = 0; // Gone!
		break;
	}

	if (_vm->_gyro->_jacquesState == 5) {
		_vm->_gyro->_bellsAreRinging = true;
		_vm->_gyro->_aylesIsAwake = true;
		_vm->_lucerna->incScore(2);
	}

	switch (_vm->_gyro->_jacquesState) {
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

void Timer::naughtyDuke() { // This is when the Duke comes in and takes your money.
	AnimationType *spr = &_vm->_animation->_sprites[1];
	spr->init(9, false, _vm->_animation); // Here comes the Duke.
	_vm->_animation->appearPed(2, 0); // He starts at the door...
	spr->walkTo(2); // He walks over to you.

	// Let's get the door opening.
	_vm->_celer->drawBackgroundSprite(-1, -1, 1);
	_vm->_sequence->firstShow(2);
	_vm->_sequence->startToClose();

	addTimer(50, kProcNaughtyDuke2, kReasonNaughtyDuke);
}

void Timer::naughtyDuke2() {
	AnimationType *spr = &_vm->_animation->_sprites[1];
	_vm->_scrolls->displayScrollChain('q', 48); // "Ha ha, it worked again!"
	spr->walkTo(0); // Walk to the door.
	spr->_vanishIfStill = true; // Then go away!
	addTimer(32, kProcNaughtyDuke3, kReasonNaughtyDuke);
}

void Timer::naughtyDuke3() {
	_vm->_celer->drawBackgroundSprite(-1, -1, 1);
	_vm->_sequence->firstShow(2);
	_vm->_sequence->startToClose();
}

void Timer::jump() {
	AnimationType *avvy = &_vm->_animation->_sprites[0];

	_vm->_gyro->_jumpStatus++;
	switch (_vm->_gyro->_jumpStatus) {
	case 1:
	case 2:
	case 3:
	case 5:
	case 7:
	case 9:
		avvy->_y--;
		break;
	case 12:
	case 13:
	case 14:
	case 16:
	case 18:
	case 19:
		avvy->_y++;
		break;
	}

	if (_vm->_gyro->_jumpStatus == 20) { // End of jump.
		_vm->_gyro->_userMovesAvvy = true;
		_vm->_gyro->_jumpStatus = 0;
	} else { // Still jumping.
		addTimer(1, kProcJump, kReasonJumping);
	}

	if ((_vm->_gyro->_jumpStatus == 10) // You're at the highest point of your jump.
			&& (_vm->_gyro->_room == r__insidecardiffcastle)
			&& (_vm->_gyro->_arrowInTheDoor == true)
			&& (_vm->_animation->inField(3))) { // Beside the wall
		// Grab the arrow!
		if (_vm->_gyro->_carryNum >= kCarryLimit)
			_vm->_scrolls->displayText("You fail to grab it, because your hands are full.");
		else {
			_vm->_celer->drawBackgroundSprite(-1, -1, 2);
			_vm->_gyro->_arrowInTheDoor = false; // You've got it.
			_vm->_gyro->_objects[Gyro::kObjectBolt - 1] = true;
			_vm->_lucerna->refreshObjectList();
			_vm->_scrolls->displayScrollChain('q', 50);
			_vm->_lucerna->incScore(3);
		}
	}
}

void Timer::crapulusSaysSpludOut() {
	_vm->_scrolls->displayScrollChain('q', 56);
	_vm->_gyro->_crapulusWillTell = false;
}

void Timer::buyDrinks() {
	_vm->_celer->drawBackgroundSprite(-1, -1, 11); // Malagauche gets up again.
	_vm->_gyro->_malagauche = 0;

	_vm->_scrolls->displayScrollChain('D', _vm->_gyro->_drinking); // Display message about it.
	_vm->_pingo->wobble(); // Do the special effects.
	_vm->_scrolls->displayScrollChain('D', 1); // That'll be thruppence.
	if (_vm->_gyro->decreaseMoney(3)) // Pay 3d.
		_vm->_scrolls->displayScrollChain('D', 3); // Tell 'em you paid up.
	_vm->_acci->drink();
}

void Timer::buyWine() {
	_vm->_celer->drawBackgroundSprite(-1, -1, 11); // Malagauche gets up again.
	_vm->_gyro->_malagauche = 0;

	_vm->_scrolls->displayScrollChain('D', 50); // You buy the wine.
	_vm->_scrolls->displayScrollChain('D', 1); // It'll be thruppence.
	if (_vm->_gyro->decreaseMoney(3)) {
		_vm->_scrolls->displayScrollChain('D', 4); // You paid up.
		_vm->_gyro->_objects[Gyro::kObjectWine - 1] = true;
		_vm->_lucerna->refreshObjectList();
		_vm->_gyro->_wineState = 1; // OK Wine.
	}
}

void Timer::callsGuards() {
	_vm->_scrolls->displayScrollChain('Q', 58); // "GUARDS!!!"
	_vm->_lucerna->gameOver();
}

void Timer::greetsMonk() {
	_vm->_scrolls->displayScrollChain('Q', 59);
	_vm->_gyro->_enteredLustiesRoomAsMonk = true;
}

void Timer::fallDownOubliette() {
	_vm->_gyro->_magics[8]._operation = Gyro::kMagicNothing;

	AnimationType *avvy = &_vm->_animation->_sprites[0];
	avvy->_moveY++; // Increments dx/dy!
	avvy->_y += avvy->_moveY;   // Dowwwn we go...
	addTimer(3, kProcFallDownOubliette, kReasonFallingDownOubliette);
}

void Timer::meetAvaroid() {
	if (_vm->_gyro->_metAvaroid) {
		Common::String tmpStr = Common::String::format("You can't expect to be %cthat%c lucky twice in a row!", Scrolls::kControlItalic, Scrolls::kControlRoman);
		_vm->_scrolls->displayText(tmpStr);
		_vm->_lucerna->gameOver();
	} else {
		_vm->_scrolls->displayScrollChain('Q', 60);
		_vm->_gyro->_metAvaroid = true;
		addTimer(1, kProcRiseUpOubliette, kReasonRisingUpOubliette);

		AnimationType *avvy = &_vm->_animation->_sprites[0];
		avvy->_facingDir = Animation::kDirLeft;
		avvy->_x = 151;
		avvy->_moveX = -3;
		avvy->_moveY = -5;

		_vm->_gyro->setBackgroundColor(2);
	}
}

void Timer::riseUpOubliette() {
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	avvy->_visible = true;
	avvy->_moveY++; // Decrements dx/dy!
	avvy->_y -= avvy->_moveY; // Uuuupppp we go...
	if (avvy->_moveY > 0)
		addTimer(3, kProcRiseUpOubliette, kReasonRisingUpOubliette);
	else
		_vm->_gyro->_userMovesAvvy = true;
}

void Timer::robinHoodAndGeida() {
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	avvy->init(7, true, _vm->_animation);
	_vm->_animation->appearPed(1, 6);
	avvy->walkTo(5);

	AnimationType *spr = &_vm->_animation->_sprites[1];
	spr->stopWalk();
	spr->_facingDir = Animation::kDirLeft;
	addTimer(20, kProcRobinHoodAndGeidaTalk, kReasonRobinHoodAndGeida);
	_vm->_gyro->_geidaFollows = false;
}

void Timer::robinHoodAndGeidaTalk() {
	_vm->_scrolls->displayScrollChain('q', 66);

	AnimationType *avvy = &_vm->_animation->_sprites[0];
	AnimationType *spr = &_vm->_animation->_sprites[1];
	avvy->walkTo(1);
	spr->walkTo(1);
	avvy->_vanishIfStill = true;
	spr->_vanishIfStill = true;

	addTimer(162, kProcAvalotReturns, kReasonRobinHoodAndGeida);
}

void Timer::avalotReturns() {
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	AnimationType *spr = &_vm->_animation->_sprites[1];
	avvy->remove();
	spr->remove();
	avvy->init(0, true, _vm->_animation);
	_vm->_animation->appearPed(1, 0);
	_vm->_scrolls->displayScrollChain('q', 67);
	_vm->_gyro->_userMovesAvvy = true;
}

/**
 * This is used when you sit down in the pub in Notts. It loops around
 * so that it will happen when Avvy stops walking.
 * @remarks	Originally called 'avvy_sit_down'
 */
void Timer::avvySitDown() {
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	if (avvy->_homing)    // Still walking.
		addTimer(1, kProcAvvySitDown, kReasonSittingDown);
	else {
		_vm->_celer->drawBackgroundSprite(-1, -1, 3);
		_vm->_gyro->_sittingInPub = true;
		_vm->_gyro->_userMovesAvvy = false;
		avvy->_visible = false;
	}
}

void Timer::ghostRoomPhew() {
	Common::String tmpStr = Common::String::format("%cPHEW!%c You're glad to get out of %cthere!",
		Scrolls::kControlItalic, Scrolls::kControlRoman, Scrolls::kControlItalic);
	_vm->_scrolls->displayText(tmpStr);
}

void Timer::arkataShouts() {
	if (_vm->_gyro->_teetotal)
		return;

	_vm->_scrolls->displayScrollChain('q', 76);
	addTimer(160, kProcArkataShouts, kReasonArkataShouts);
}

void Timer::winning() {
	_vm->_scrolls->displayScrollChain('q', 79);
	_vm->_pingo->winningPic();

	warning("STUB: Timer::winning()");
#if 0
	do {
		_vm->_lucerna->checkclick();
	} while (!(_vm->_gyro->mrelease == 0));
#endif
	// TODO: To be implemented with Pingo::winningPic().

	_vm->_lucerna->callVerb(Acci::kVerbCodeScore);
	_vm->_scrolls->displayText(" T H E    E N D ");
	_vm->_gyro->_letMeOut = true;
}

void Timer::avalotFalls() {
	AnimationType *avvy = &_vm->_animation->_sprites[0];
	if (avvy->_stepNum < 5) {
		avvy->_stepNum++;
		addTimer(3, kProcAvalotFalls, kReasonFallingOver);
	} else {
		Common::String toDisplay = Common::String::format("%c%c%c%c%c%c%c%c%c%c%c%c%cZ%c",
			Scrolls::kControlNewLine, Scrolls::kControlNewLine, Scrolls::kControlNewLine,
			Scrolls::kControlNewLine, Scrolls::kControlNewLine, Scrolls::kControlNewLine,
			Scrolls::kControlInsertSpaces, Scrolls::kControlInsertSpaces, Scrolls::kControlInsertSpaces,
			Scrolls::kControlInsertSpaces, Scrolls::kControlInsertSpaces, Scrolls::kControlInsertSpaces,
			Scrolls::kControlRegister, Scrolls::kControlIcon);
		_vm->_scrolls->displayText(toDisplay);
	}
}

void Timer::spludwickGoesToCauldron() {
	if (_vm->_animation->_sprites[1]._homing)
		addTimer(1, kProcSpludwickGoesToCauldron, kReasonSpludwickWalk);
	else
		addTimer(17, kProcSpludwickLeavesCauldron, kReasonSpludwickWalk);
}

void Timer::spludwickLeavesCauldron() {
	_vm->_animation->_sprites[1]._callEachStepFl = true; // So that normal procs will continue.
}

void Timer::giveLuteToGeida() { // Moved here from Acci.
	_vm->_scrolls->displayScrollChain('Q', 86);
	_vm->_lucerna->incScore(4);
	_vm->_gyro->_lustieIsAsleep = true;
	_vm->_sequence->firstShow(5);
	_vm->_sequence->thenShow(6); // He falls asleep...
	_vm->_sequence->startToClose(); // Not really closing, but we're using the same procedure.
}

} // End of namespace Avalanche.
