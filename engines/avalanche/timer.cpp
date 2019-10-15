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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* Original name: TIMEOUT	The scheduling unit. */

#include "avalanche/avalanche.h"
#include "avalanche/timer.h"

namespace Avalanche {

Timer::Timer(AvalancheEngine *vm) {
	_vm = vm;

	resetVariables();
}

/**
 * Add a nex timer
 * @remarks	Originally called 'set_up_timer'
 */
void Timer::addTimer(int32 duration, byte action, byte reason) {
	byte i = 0;
	while ((i < 7) && (_times[i]._timeLeft != 0)) {
		if (_times[i]._reason == reason) // We only add a timer if it's not already in the array.
			return;
		i++;
	}

	if (i == 7)
		return; // Oh dear... No timer left

	// Everything's OK here!
	_times[i]._timeLeft = duration;
	_times[i]._action = action;
	_times[i]._reason = reason;
}

/**
 * Update the timers
 * @remarks	Originally called 'one_tick'
 */
void Timer::updateTimer() {
	if (_vm->_dropdown->isActive())
		return;

	for (int i = 0; i < 7; i++) {
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
				_vm->fadeIn();
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
			default:
				break;
			}
		}
	}

	_vm->_roomCycles++; // Cycles since you've been in this room.
}

void Timer::loseTimer(byte which) {
	for (int i = 0; i < 7; i++) {
		if (_times[i]._reason == which)
			_times[i]._timeLeft = 0; // Cancel this one!
	}

}

void Timer::openDrawbridge() {
	_vm->_drawbridgeOpen++;
	_vm->_background->draw(-1, -1, _vm->_drawbridgeOpen - 2);

	if (_vm->_drawbridgeOpen == 4)
		_vm->_magics[1]._operation = kMagicNothing; // You may enter the drawbridge.
	else
		addTimer(7, kProcOpenDrawbridge, kReasonDrawbridgeFalls);
}

void Timer::avariciusTalks() {
	_vm->_dialogs->displayScrollChain('Q', _vm->_avariciusTalk);
	_vm->_avariciusTalk++;

	if (_vm->_avariciusTalk < 17)
		addTimer(177, kProcAvariciusTalks, kReasonAvariciusTalks);
	else
		_vm->incScore(3);
}

void Timer::urinate() {
	_vm->_animation->_sprites[0]->turn(kDirUp);
	_vm->_animation->stopWalking();
	_vm->drawDirection();
	addTimer(14, kProcToilet, kReasonGoToToilet);
}

void Timer::toilet() {
	_vm->_dialogs->displayText("That's better!");
}

void Timer::bang() {
	Common::String tmpStr = Common::String::format("%c< BANG! >", kControlItalic);
	_vm->_dialogs->displayText(tmpStr);
	addTimer(30, kProcBang2, kReasonExplosion);
}

void Timer::bang2() {
	_vm->_dialogs->displayText("Hmm... sounds like Spludwick's up to something...");
}

void Timer::stairs() {
	_vm->_sound->blip();
	_vm->_animation->_sprites[0]->walkTo(3);
	_vm->_background->draw(-1, -1, 1);
	_vm->_brummieStairs = 2;
	_vm->_magics[10]._operation = kMagicSpecial;
	_vm->_magics[10]._data = 2; // Reached the bottom of the stairs.
	_vm->_magics[3]._operation = kMagicNothing; // Stop them hitting the sides (or the game will hang.)
}

void Timer::cardiffSurvey() {
	if (_vm->_cardiffQuestionNum == 0) {
		_vm->_cardiffQuestionNum++;
		_vm->_dialogs->displayScrollChain('Q', 27);
	}

	_vm->_dialogs->displayScrollChain('Z', _vm->_cardiffQuestionNum);
	_vm->_interrogation = _vm->_cardiffQuestionNum;
	addTimer(182, kProcCardiffSurvey, kReasonCardiffsurvey);
}

void Timer::cardiffReturn() {
	_vm->_dialogs->displayScrollChain('Q', 28);
	cardiffSurvey(); // Add end of question.
}

void Timer::cwytalotInHerts() {
	_vm->_dialogs->displayScrollChain('Q', 29);
}

void Timer::getTiedUp() {
	_vm->_dialogs->displayScrollChain('Q', 34); // ...Trouble!
	_vm->_userMovesAvvy = false;
	_vm->_beenTiedUp = true;
	_vm->_animation->stopWalking();

	AnimationType *spr = _vm->_animation->_sprites[1];
	spr->stopWalk();
	spr->stopHoming();
	spr->_callEachStepFl = true;
	spr->_eachStepProc = Animation::kProcGrabAvvy;
	addTimer(70, kProcGetTiedUp2, kReasonGettingTiedUp);
}

void Timer::getTiedUp2() {
	_vm->_animation->_sprites[0]->walkTo(3);
	_vm->_animation->_sprites[1]->walkTo(4);
	_vm->_magics[3]._operation = kMagicNothing; // No effect when you touch the boundaries.
	_vm->_friarWillTieYouUp = true;
}

void Timer::hangAround() {
	_vm->_animation->_sprites[1]->_doCheck = false;

	AnimationType *avvy = _vm->_animation->_sprites[0];
	avvy->init(7, true); // Robin Hood
	_vm->setRoom(kPeopleRobinHood, kRoomRobins);
	_vm->_animation->appearPed(0, 1);
	_vm->_dialogs->displayScrollChain('Q', 39);
	avvy->walkTo(6);
	addTimer(55, kProcHangAround2, kReasonHangingAround);
}

void Timer::hangAround2() {
	_vm->_dialogs->displayScrollChain('Q', 40);
	AnimationType *spr = _vm->_animation->_sprites[1];
	spr->_vanishIfStill = false;
	spr->walkTo(3);
	_vm->setRoom(kPeopleFriarTuck, kRoomRobins);
	_vm->_dialogs->displayScrollChain('Q', 41);
	_vm->_animation->_sprites[0]->remove();
	spr->remove(); // Get rid of Robin Hood and Friar Tuck.

	addTimer(1, kProcAfterTheShootemup, kReasonHangingAround); // Immediately call the following proc (when you have a chance).

	_vm->_tiedUp = false;

	// We don't need the ShootEmUp during the whole game, it's only playable once.
	ShootEmUp *shootemup = new ShootEmUp(_vm);
	_shootEmUpScore = shootemup->run();
	delete shootemup;
}

void Timer::afterTheShootemup() {
	_vm->flipRoom(_vm->_room, 1);

	_vm->_animation->_sprites[0]->init(0, true); // Avalot.
	_vm->_animation->appearPed(0, 1);
	_vm->_userMovesAvvy = true;
	_vm->_objects[kObjectCrossbow - 1] = true;
	_vm->refreshObjectList();

	byte gain = (_shootEmUpScore + 5) / 10; // Rounding up.
	_vm->_dialogs->displayText(Common::String::format("%cYour score was %d.%c%cYou gain (%d \xf6 10) = %d points.", kControlItalic, _shootEmUpScore, kControlNewLine, kControlNewLine, _shootEmUpScore, gain));

	if (gain > 20) {
		_vm->_dialogs->displayText("But we won't let you have more than 20 points!");
		_vm->incScore(20);
	} else
		_vm->incScore(gain);


	_vm->_dialogs->displayScrollChain('Q', 70);
}

void Timer::jacquesWakesUp() {
	_vm->_jacquesState++;

	switch (_vm->_jacquesState) { // Additional pictures.
	case 1:
		_vm->_background->draw(-1, -1, 0); // Eyes open.
		_vm->_dialogs->displayScrollChain('Q', 45);
		break;
	case 2: // Going through the door.
		_vm->_background->draw(-1, -1, 1); // Not on the floor.
		_vm->_background->draw(-1, -1, 2); // But going through the door.
		_vm->_magics[5]._operation = kMagicNothing; // You can't wake him up now.
		break;
	case 3: // Gone through the door.
		_vm->_background->draw(-1, -1, 1); // Not on the floor, either.
		_vm->_background->draw(-1, -1, 3); // He's gone... so the door's open.
		_vm->setRoom(kPeopleJacques, kRoomNowhere); // Gone!
		break;
	default:
		break;
	}

	if (_vm->_jacquesState == 5) {
		_vm->_bellsAreRinging = true;
		_vm->_aylesIsAwake = true;
		_vm->incScore(2);
	}

	switch (_vm->_jacquesState) {
	case 1:
	case 2:
	case 3:
		addTimer(12, kProcJacquesWakesUp, kReasonJacquesWakingUp);
		break;
	case 4:
		addTimer(24, kProcJacquesWakesUp, kReasonJacquesWakingUp);
		break;
	default:
		break;
	}
}

void Timer::naughtyDuke() { // This is when the Duke comes in and takes your money.
	AnimationType *spr = _vm->_animation->_sprites[1];
	spr->init(9, false); // Here comes the Duke.
	_vm->_animation->appearPed(1, 0); // He starts at the door...
	spr->walkTo(2); // He walks over to you.

	// Let's get the door opening.
	_vm->_background->draw(-1, -1, 0);
	_vm->_sequence->startNaughtyDukeSeq();

	addTimer(50, kProcNaughtyDuke2, kReasonNaughtyDuke);
}

void Timer::naughtyDuke2() {
	AnimationType *spr = _vm->_animation->_sprites[1];
	_vm->_dialogs->displayScrollChain('Q', 48); // "Ha ha, it worked again!"
	spr->walkTo(0); // Walk to the door.
	spr->_vanishIfStill = true; // Then go away!

	addTimer(32, kProcNaughtyDuke3, kReasonNaughtyDuke);
}

void Timer::naughtyDuke3() {
	_vm->_background->draw(-1, -1, 0);
	_vm->_sequence->startNaughtyDukeSeq();
}

void Timer::jump() {
	AnimationType *avvy = _vm->_animation->_sprites[0];

	_vm->_jumpStatus++;
	switch (_vm->_jumpStatus) {
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
	default:
		break;
	}

	if (_vm->_jumpStatus == 20) { // End of jump.
		_vm->_userMovesAvvy = true;
		_vm->_jumpStatus = 0;
	} else // Still jumping.
		addTimer(1, kProcJump, kReasonJumping);

	if ((_vm->_jumpStatus == 10) // You're at the highest point of your jump.
			&& (_vm->_room == kRoomInsideCardiffCastle)
			&& (_vm->_arrowInTheDoor == true)
			&& (_vm->_animation->inField(2))) { // Beside the wall
		// Grab the arrow!
		if (_vm->_carryNum >= kCarryLimit)
			_vm->_dialogs->displayText("You fail to grab it, because your hands are full.");
		else {
			_vm->_background->draw(-1, -1, 1);
			_vm->_arrowInTheDoor = false; // You've got it.
			_vm->_objects[kObjectBolt - 1] = true;
			_vm->refreshObjectList();
			_vm->_dialogs->displayScrollChain('Q', 50);
			_vm->incScore(3);
		}
	}
}

void Timer::crapulusSaysSpludOut() {
	_vm->_dialogs->displayScrollChain('Q', 56);
	_vm->_crapulusWillTell = false;
}

void Timer::buyDrinks() {
	_vm->_background->draw(-1, -1, 10); // Malagauche gets up again.
	_vm->_malagauche = 0;

	_vm->_dialogs->displayScrollChain('D', _vm->_drinking); // Display message about it.
	_vm->_animation->wobble(); // Do the special effects.
	_vm->_dialogs->displayScrollChain('D', 1); // That'll be thruppence.
	if (_vm->decreaseMoney(3)) // Pay 3d.
		_vm->_dialogs->displayScrollChain('D', 3); // Tell 'em you paid up.
	_vm->_parser->drink();
}

void Timer::buyWine() {
	_vm->_background->draw(-1, -1, 10); // Malagauche gets up again.
	_vm->_malagauche = 0;

	_vm->_dialogs->displayScrollChain('D', 50); // You buy the wine.
	_vm->_dialogs->displayScrollChain('D', 1); // It'll be thruppence.
	if (_vm->decreaseMoney(3)) {
		_vm->_dialogs->displayScrollChain('D', 4); // You paid up.
		_vm->_objects[kObjectWine - 1] = true;
		_vm->refreshObjectList();
		_vm->_wineState = 1; // OK Wine.
	}
}

void Timer::callsGuards() {
	_vm->_dialogs->displayScrollChain('Q', 58); // "GUARDS!!!"
	_vm->gameOver();
}

void Timer::greetsMonk() {
	_vm->_dialogs->displayScrollChain('Q', 59);
	_vm->_enteredLustiesRoomAsMonk = true;
}

void Timer::fallDownOubliette() {
	_vm->_magics[8]._operation = kMagicNothing;

	AnimationType *avvy = _vm->_animation->_sprites[0];
	avvy->_moveY++; // Increments dx/dy!
	avvy->_y += avvy->_moveY;   // Dowwwn we go...
	addTimer(3, kProcFallDownOubliette, kReasonFallingDownOubliette);
}

void Timer::meetAvaroid() {
	if (_vm->_metAvaroid) {
		Common::String tmpStr = Common::String::format("You can't expect to be %cthat%c lucky twice in a row!",
			kControlItalic, kControlRoman);
		_vm->_dialogs->displayText(tmpStr);
		_vm->gameOver();
	} else {
		_vm->_dialogs->displayScrollChain('Q', 60);
		_vm->_metAvaroid = true;
		addTimer(1, kProcRiseUpOubliette, kReasonRisingUpOubliette);

		AnimationType *avvy = _vm->_animation->_sprites[0];
		avvy->_facingDir = kDirLeft;
		avvy->_x = 151;
		avvy->_moveX = -3;
		avvy->_moveY = -5;

		_vm->_graphics->setBackgroundColor(kColorGreen);
	}
}

void Timer::riseUpOubliette() {
	AnimationType *avvy = _vm->_animation->_sprites[0];
	avvy->_visible = true;
	avvy->_moveY++; // Decrements dx/dy!
	avvy->_y -= avvy->_moveY; // Uuuupppp we go...
	if (avvy->_moveY > 0)
		addTimer(3, kProcRiseUpOubliette, kReasonRisingUpOubliette);
	else
		_vm->_userMovesAvvy = true;
}

void Timer::robinHoodAndGeida() {
	AnimationType *avvy = _vm->_animation->_sprites[0];
	avvy->init(7, true);
	_vm->_animation->appearPed(0, 6);
	avvy->walkTo(5);

	AnimationType *spr = _vm->_animation->_sprites[1];
	spr->stopWalk();
	spr->_facingDir = kDirLeft;
	addTimer(20, kProcRobinHoodAndGeidaTalk, kReasonRobinHoodAndGeida);
	_vm->_geidaFollows = false;
}

void Timer::robinHoodAndGeidaTalk() {
	_vm->_dialogs->displayScrollChain('Q', 66);

	AnimationType *avvy = _vm->_animation->_sprites[0];
	AnimationType *spr = _vm->_animation->_sprites[1];
	avvy->walkTo(1);
	spr->walkTo(1);
	avvy->_vanishIfStill = true;
	spr->_vanishIfStill = true;

	addTimer(162, kProcAvalotReturns, kReasonRobinHoodAndGeida);
}

void Timer::avalotReturns() {
	AnimationType *avvy = _vm->_animation->_sprites[0];
	AnimationType *spr = _vm->_animation->_sprites[1];
	avvy->remove();
	spr->remove();
	avvy->init(0, true);
	_vm->_animation->appearPed(0, 0);
	_vm->_dialogs->displayScrollChain('Q', 67);
	_vm->_userMovesAvvy = true;
}

/**
 * This is used when you sit down in the pub in Notts. It loops around
 * so that it will happen when Avvy stops walking.
 * @remarks	Originally called 'avvy_sit_down'
 */
void Timer::avvySitDown() {
	AnimationType *avvy = _vm->_animation->_sprites[0];
	if (avvy->_homing)    // Still walking.
		addTimer(1, kProcAvvySitDown, kReasonSittingDown);
	else {
		_vm->_background->draw(-1, -1, 2);
		_vm->_sittingInPub = true;
		_vm->_userMovesAvvy = false;
		avvy->_visible = false;
	}
}

void Timer::ghostRoomPhew() {
	Common::String tmpStr = Common::String::format("%cPHEW!%c You're glad to get out of %cthere!",
		kControlItalic, kControlRoman, kControlItalic);
	_vm->_dialogs->displayText(tmpStr);
}

void Timer::arkataShouts() {
	if (_vm->_teetotal)
		return;

	_vm->_dialogs->displayScrollChain('Q', 76);
	addTimer(160, kProcArkataShouts, kReasonArkataShouts);
}

/**
 * @remarks Contains the content of the function 'winning_pic', originally located in PINGO.
 */
void Timer::winning() {
	_vm->_dialogs->displayScrollChain('Q', 79);

	// This was originally located in winning_pic:
	CursorMan.showMouse(false);
	_vm->_graphics->saveScreen();
	_vm->fadeOut();
	_vm->_graphics->drawWinningPic();
	_vm->_graphics->refreshScreen();
	_vm->fadeIn();

	// Waiting for a keypress or a left mouseclick:
	Common::Event event;
	bool escape = false;
	while (!_vm->shouldQuit() && !escape) {
		_vm->_graphics->refreshScreen();
		while (_vm->getEvent(event)) {
			if ((event.type == Common::EVENT_LBUTTONUP) || (event.type == Common::EVENT_KEYDOWN)) {
				escape = true;
				break;
			}
		}
	}

	_vm->fadeOut();
	_vm->_graphics->restoreScreen();
	_vm->_graphics->removeBackup();
	_vm->fadeIn();
	CursorMan.showMouse(true);
	// winning_pic's end.

	_vm->callVerb(kVerbCodeScore);
	_vm->_dialogs->displayText(" T H E    E N D ");
	_vm->_letMeOut = true;
}

void Timer::avalotFalls() {
	AnimationType *avvy = _vm->_animation->_sprites[0];
	if (avvy->_stepNum < 5) {
		avvy->_stepNum++;
		addTimer(3, kProcAvalotFalls, kReasonFallingOver);
	} else {
		Common::String toDisplay = Common::String::format("%c%c%c%c%c%c%c%c%c%c%c%c%cZ%c",
			kControlNewLine, kControlNewLine, kControlNewLine, kControlNewLine,
			kControlNewLine, kControlNewLine, kControlInsertSpaces, kControlInsertSpaces,
			kControlInsertSpaces, kControlInsertSpaces, kControlInsertSpaces,
			kControlInsertSpaces, kControlRegister, kControlIcon);
		_vm->_dialogs->displayText(toDisplay);
	}
}

void Timer::spludwickGoesToCauldron() {
	if (_vm->_animation->_sprites[1]->_homing)
		addTimer(1, kProcSpludwickGoesToCauldron, kReasonSpludwickWalk);
	else
		addTimer(17, kProcSpludwickLeavesCauldron, kReasonSpludwickWalk);
}

void Timer::spludwickLeavesCauldron() {
	_vm->_animation->_sprites[1]->_callEachStepFl = true; // So that normal procs will continue.
}

void Timer::giveLuteToGeida() { // Moved here from Acci.
	_vm->_dialogs->displayScrollChain('Q', 86);
	_vm->incScore(4);
	_vm->_lustieIsAsleep = true;
	_vm->_sequence->startGeidaLuteSeq();
}

void Timer::resetVariables() {
	for (int i = 0; i < 7; i++) {
		_times[i]._timeLeft = 0;
		_times[i]._action = 0;
		_times[i]._reason = 0;
	}

	_shootEmUpScore = 0;
}

} // End of namespace Avalanche.
