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

#include "lastexpress/entities/pascale.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Pascale::Pascale(LastExpressEngine *engine) : Entity(engine, kCharacterHeadWait) {
	ADD_CALLBACK_FUNCTION_S(Pascale, draw);
	ADD_CALLBACK_FUNCTION(Pascale, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Pascale, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION_I(Pascale, updateFromTime);
	ADD_CALLBACK_FUNCTION_SII(Pascale, updatePosition);
	ADD_CALLBACK_FUNCTION_S(Pascale, playSound);
	ADD_CALLBACK_FUNCTION_SSI(Pascale, draw2);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeSophieAndRebecca);
	ADD_CALLBACK_FUNCTION(Pascale, sitSophieAndRebecca);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeCath);
	ADD_CALLBACK_FUNCTION(Pascale, seatCath);
	ADD_CALLBACK_FUNCTION(Pascale, chapter1);
	ADD_CALLBACK_FUNCTION(Pascale, greetAugust);
	ADD_CALLBACK_FUNCTION(Pascale, seatAnna);
	ADD_CALLBACK_FUNCTION(Pascale, greetAnna);
	ADD_CALLBACK_FUNCTION(Pascale, greetTatiana);
	ADD_CALLBACK_FUNCTION(Pascale, servingDinner);
	ADD_CALLBACK_FUNCTION(Pascale, function18);
	ADD_CALLBACK_FUNCTION(Pascale, function19);
	ADD_CALLBACK_FUNCTION(Pascale, chapter2);
	ADD_CALLBACK_FUNCTION(Pascale, chapter3);
	ADD_CALLBACK_FUNCTION(Pascale, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Pascale, abbotSeatMe3);
	ADD_CALLBACK_FUNCTION(Pascale, welcomeAbbot);
	ADD_CALLBACK_FUNCTION(Pascale, chapter4);
	ADD_CALLBACK_FUNCTION(Pascale, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Pascale, meetCoudert);
	ADD_CALLBACK_FUNCTION(Pascale, tellAugust);
	ADD_CALLBACK_FUNCTION(Pascale, walkDownTrain);
	ADD_CALLBACK_FUNCTION(Pascale, walkUpTrain);
	ADD_CALLBACK_FUNCTION(Pascale, chapter5);
	ADD_CALLBACK_FUNCTION(Pascale, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Pascale, hiding);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(1, Pascale, draw)
	Entity::draw(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(2, Pascale, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Pascale, callbackActionOnDirection)
	if (savepoint.action == kCharacterActionExcuseMeCath) {
		if (!params->param1) {
			getSound()->excuseMe(kCharacterHeadWait);
			params->param1 = 1;
		}

		return;
	}

	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Pascale, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Pascale, updatePosition)
	Entity::updatePosition(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Pascale, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(7, Pascale, draw2)
	Entity::draw2(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Pascale, welcomeSophieAndRebecca)
	// Welcomes Sophie And Rebecca
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_850;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("901");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			switch (getProgress().chapter) {
			default:
				break;

			case kChapter1:
				getSound()->playSound(kCharacterHeadWait, "REB1198", kSoundVolumeEntityDefault, 30);
				break;

			case kChapter3:
				getSound()->playSound(kCharacterHeadWait, "REB3001", kSoundVolumeEntityDefault, 30);
				break;

			case kChapter4:
				getSound()->playSound(kCharacterHeadWait, "REB4001", kSoundVolumeEntityDefault, 30);
				break;
			}

			setCallback(2);
			setup_sitSophieAndRebecca();
			break;

		case 2:
			getSavePoints()->push(kCharacterHeadWait, kCharacterRebecca, kCharacterAction157370960);

			setCallback(3);
			setup_draw("905");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterHeadWait);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 4) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Pascale, sitSophieAndRebecca)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionExitCompartment:
		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterHeadWait, "012C1");
		getEntities()->drawSequenceRight(kCharacterRebecca, "012C2");
		getEntities()->drawSequenceRight(kCharacterTableD, "012C3");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Pascale, welcomeCath)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1 && !getSoundQueue()->isBuffered(kCharacterHeadWait))
			getEntities()->updatePositionExit(kCharacterHeadWait, kCarRestaurant, 64);
		break;

	case kCharacterActionExitCompartment:
		if (!params->param2) {
			params->param2 = 1;

			getSound()->playSound(kCharacterHeadWait, "HED1001A");
			getSound()->playSound(kCharacterCath, "LIB004");

			getScenes()->loadSceneFromPosition(kCarRestaurant, 69);
		}

		callbackAction();
		break;

	case kCharacterAction4:
		if (!params->param1) {
			params->param1 = 1;
			getSound()->playSound(kCharacterHeadWait, "HED1001");
		}
		break;

	case kCharacterActionDefault:
		getEntities()->updatePositionEnter(kCharacterHeadWait, kCarRestaurant, 64);
		getEntities()->drawSequenceRight(kCharacterHeadWait, "035A");
		break;

	case kCharacterActionDrawScene:
		if (params->param1 && getEntities()->isPlayerPosition(kCarRestaurant, 64)) {
			getSound()->playSound(kCharacterHeadWait, "HED1001A");
			getSound()->playSound(kCharacterCath, "LIB004");

			getScenes()->loadSceneFromPosition(kCarRestaurant, 69);

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Pascale, seatCath)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		getSavePoints()->push(kCharacterHeadWait, kCharacterAugust, kCharacterAction168046720);
		getSavePoints()->push(kCharacterHeadWait, kCharacterAnna, kCharacterAction168046720);
		getSavePoints()->push(kCharacterHeadWait, kCharacterAlexei, kCharacterAction168046720);
		getEntities()->updatePositionEnter(kCharacterHeadWait, kCarRestaurant, 55);

		setCallback(1);
		setup_welcomeCath();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterHeadWait, kCharacterAugust, kCharacterAction168627977);
			getSavePoints()->push(kCharacterHeadWait, kCharacterAnna, kCharacterAction168627977);
			getSavePoints()->push(kCharacterHeadWait, kCharacterAlexei, kCharacterAction168627977);
			getEntities()->updatePositionExit(kCharacterHeadWait, kCarRestaurant, 55);

			setCallback(2);
			setup_draw("905");
			break;

		case 2:
			getEntities()->clearSequences(kCharacterHeadWait);
			getData()->entityPosition = kPosition_5900;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Pascale, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_servingDinner();
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction239072064, 0);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction257489762, 2);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction207769280, 6);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction101824388, 7);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction136059947, 8);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction223262556, 1);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction269479296, 3);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction352703104, 4);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction352768896, 5);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction191604416, 10);
		getSavePoints()->addData(kCharacterHeadWait, kCharacterAction190605184, 11);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Pascale, greetAugust)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("902");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (!ENTITY_PARAM(1, 3)) {
				getEntities()->drawSequenceLeft(kCharacterHeadWait, "010E");
				getEntities()->drawSequenceLeft(kCharacterAugust, "BLANK");

				setCallback(2);
				setup_playSound("AUG1001");
				break;
			}

			setCallback(3);
			setup_draw("905");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterAugust, "010B");

			setCallback(3);
			setup_draw("905");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterHeadWait);
			getSavePoints()->push(kCharacterHeadWait, kCharacterTrainM, kCharacterActionDeliverMessageToTyler);
			ENTITY_PARAM(0, 1) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Pascale, seatAnna)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionExitCompartment:
		getEntities()->updatePositionExit(kCharacterHeadWait, kCarRestaurant, 62);

		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterTableA, "001C3");
		getEntities()->drawSequenceRight(kCharacterAnna, "001C2");
		getEntities()->drawSequenceRight(kCharacterHeadWait, "001C1");

		getEntities()->updatePositionEnter(kCharacterHeadWait, kCarRestaurant, 62);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Pascale, greetAnna)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("901");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSound()->playSound(kCharacterHeadWait, "ANN1047");

			setCallback(2);
			setup_seatAnna();
			break;

		case 2:
			getSavePoints()->push(kCharacterHeadWait, kCharacterAnna, kCharacterAction157370960);

			setCallback(3);
			setup_draw("904");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterHeadWait);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Pascale, greetTatiana)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("903");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterHeadWait, kCharacterTatiana, kCharacterAction122358304);
			getEntities()->drawSequenceLeft(kCharacterHeadWait, "014B");
			getEntities()->updatePositionEnter(kCharacterHeadWait, kCarRestaurant, 67);

			if (getSoundQueue()->isBuffered("TAT1069A"))
				getSoundQueue()->fade("TAT1069A");
			else if (getSoundQueue()->isBuffered("TAT1069B"))
				getSoundQueue()->fade("TAT1069B");

			setCallback(2);
			setup_playSound("TAT1066");
			break;

		case 2:
			getEntities()->updatePositionExit(kCharacterHeadWait, kCarRestaurant, 67);
			getSavePoints()->push(kCharacterHeadWait, kCharacterTatiana, kCharacterAction122288808);

			setCallback(3);
			setup_draw("906");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterHeadWait);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 3) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Pascale, servingDinner)
switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param2) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 69)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 70)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 71))
				params->param2 = 1;

			if (!params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61))
				params->param1 = 1;
		}

		if (!getEntities()->isInKitchen(kCharacterHeadWait))
			break;

		if (ENTITY_PARAM(0, 5) && ENTITY_PARAM(0, 6)) {
			setup_function18();
			break;
		}

		if (!getEntities()->isSomebodyInsideRestaurantOrSalon())
			goto label_callback3;

		if (params->param1 && !params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61)) {
			setCallback(1);
			setup_seatCath();
			break;
		}

label_callback1:
		if (ENTITY_PARAM(0, 1) && !ENTITY_PARAM(1, 3)) {
			setCallback(2);
			setup_greetAugust();
			break;
		}

label_callback2:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_greetTatiana();
			break;
		}

label_callback3:
		if (ENTITY_PARAM(0, 2)) {
			setCallback(4);
			setup_greetAnna();
			break;
		}

label_callback4:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(5);
			setup_welcomeSophieAndRebecca();
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 0;
			params->param2 = 1;
			goto label_callback1;

		case 2:
			goto label_callback2;

		case 3:
			goto label_callback3;

		case 4:
			goto label_callback4;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Pascale, function18)
	if (savepoint.action != kCharacterActionNone)
		return;

	if (getState()->time > kTime1242000 && !params->param1) {
		params->param1 = 1;

		getSavePoints()->push(kCharacterHeadWait, kCharacterWaiter1, kCharacterAction101632192);
		getSavePoints()->push(kCharacterHeadWait, kCharacterWaiter2, kCharacterAction101632192);
		getSavePoints()->push(kCharacterHeadWait, kCharacterCook, kCharacterAction101632192);
		getSavePoints()->push(kCharacterHeadWait, kCharacterTrainM, kCharacterAction101632192);

		setup_function19();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Pascale, function19)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1 && getEntityData(kCharacterCath)->entityPosition < kPosition_3650) {
			getObjects()->update(kObject65, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getSavePoints()->push(kCharacterHeadWait, kCharacterTableA, kCharacterActionDrawTablesWithChairs, "001P");
			getSavePoints()->push(kCharacterHeadWait, kCharacterTableB, kCharacterActionDrawTablesWithChairs, "005J");
			getSavePoints()->push(kCharacterHeadWait, kCharacterTableC, kCharacterActionDrawTablesWithChairs, "009G");
			getSavePoints()->push(kCharacterHeadWait, kCharacterTableD, kCharacterActionDrawTablesWithChairs, "010M");
			getSavePoints()->push(kCharacterHeadWait, kCharacterTableE, kCharacterActionDrawTablesWithChairs, "014F");
			getSavePoints()->push(kCharacterHeadWait, kCharacterTableF, kCharacterActionDrawTablesWithChairs, "024D");

			params->param1 = 1;
		}
		break;

	case kCharacterActionDefault:
		getData()->car = kCarRestaurant;
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;

		getEntities()->clearSequences(kCharacterHeadWait);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Pascale, chapter2)
	if (savepoint.action == kCharacterActionDefault) {
		getEntities()->clearSequences(kCharacterHeadWait);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObject65, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorForward);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Pascale, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter3Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterHeadWait);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 7) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Pascale, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!getEntities()->isInKitchen(kCharacterHeadWait))
			break;

		if (ENTITY_PARAM(0, 7)) {
			setCallback(1);
			setup_abbotSeatMe3();
			break;
		}

label_callback:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(2);
			setup_welcomeSophieAndRebecca();
		}
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			goto label_callback;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Pascale, abbotSeatMe3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;
		getEntities()->updatePositionEnter(kCharacterHeadWait, kCarRestaurant, 67);

		setCallback(1);
		setup_welcomeAbbot();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->updatePositionExit(kCharacterHeadWait, kCarRestaurant, 67);
			getSavePoints()->push(kCharacterHeadWait, kCharacterAbbot, kCharacterAction122288808);

			setCallback(2);
			setup_draw("906");
			break;

		case 2:
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 7) = 0;
			getEntities()->clearSequences(kCharacterHeadWait);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Pascale, welcomeAbbot)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionEndSound:
		if (!params->param1) {
			getSound()->playSound(kCharacterHeadWait, "ABB3015A");
			params->param1 = 1;
		}
		break;

	case kCharacterActionExitCompartment:
		callbackAction();
		break;

	case kCharacterAction10:
		getSavePoints()->push(kCharacterHeadWait, kCharacterTableE, kCharacterAction136455232);
		break;

	case kCharacterActionDefault:
		getSound()->playSound(kCharacterHeadWait, "ABB3015", kSoundVolumeEntityDefault, 105);
		getEntities()->drawSequenceRight(kCharacterHeadWait, "029A1");
		getEntities()->drawSequenceRight(kCharacterAbbot, "029A2");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Pascale, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter4Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterHeadWait);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 8) = 0;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Pascale, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime2511000 && !params->param4) {
			params->param2 = 1;
			params->param4 = 1;
		}

		if (!getEntities()->isInKitchen(kCharacterHeadWait))
			break;

		if (getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			if (ENTITY_PARAM(0, 8)) {
				setCallback(1);
				setup_meetCoudert();
				break;
			}

label_callback1:
			if (ENTITY_PARAM(1, 2) && ENTITY_PARAM(1, 4)) {
				if (!params->param3)
					params->param3 = (uint)(getState()->time + 9000);

				if (params->param5 != kTimeInvalid) {

					if (params->param3 < getState()->time) {
						params->param5 = kTimeInvalid;
						setCallback(2);
						setup_tellAugust();
						break;
					}

					if (!getEntities()->isInRestaurant(kCharacterCath) || !params->param5)
						params->param5 = (uint)getState()->time;

					if (params->param5 < getState()->time) {
						params->param5 = kTimeInvalid;
						setCallback(2);
						setup_tellAugust();
						break;
					}
				}
			}

label_callback2:
			if (params->param1 && !params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61)) {
				setCallback(3);
				setup_seatCath();
				break;
			}
		}

label_callback3:
		if (ENTITY_PARAM(0, 4)) {
			setCallback(4);
			setup_welcomeSophieAndRebecca();
		}
		break;

	case kCharacterActionDefault:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 69)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 70)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 71))
			params->param2 = 1;
		break;

	case kCharacterActionDrawScene:
		if (!params->param2) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 69)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 70)
			 || getEntities()->isPlayerPosition(kCarRestaurant, 71))
				params->param2 = 1;

			if (!params->param2 && getEntities()->isPlayerPosition(kCarRestaurant, 61))
				params->param1 = 1;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback1;

		case 2:
			goto label_callback2;

		case 3:
			params->param1 = 0;
			params->param2 = 1;
			goto label_callback3;
		}
		break;

	case kCharacterAction201431954:
		ENTITY_PARAM(0, 4) = 0;
		ENTITY_PARAM(0, 8) = 0;

		getSavePoints()->push(kCharacterHeadWait, kCharacterTableA, kCharacterActionDrawTablesWithChairs, "001P");
		getSavePoints()->push(kCharacterHeadWait, kCharacterTableB, kCharacterActionDrawTablesWithChairs, "005J");
		getSavePoints()->push(kCharacterHeadWait, kCharacterTableC, kCharacterActionDrawTablesWithChairs, "009G");
		getSavePoints()->push(kCharacterHeadWait, kCharacterTableD, kCharacterActionDrawTablesWithChairs, "010M");
		getSavePoints()->push(kCharacterHeadWait, kCharacterTableE, kCharacterActionDrawTablesWithChairs, "014F");
		getSavePoints()->push(kCharacterHeadWait, kCharacterTableF, kCharacterActionDrawTablesWithChairs, "024D");

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Pascale, meetCoudert)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (ENTITY_PARAM(1, 1)) {
			setCallback(2);
			setup_updateFromTime(450);
		}
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_walkDownTrain();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kCharacterHeadWait);
			break;

		case 2:
			getSavePoints()->push(kCharacterHeadWait, kCharacterCond2, kCharacterAction123712592);

			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			setCallback(4);
			setup_walkUpTrain();
			break;

		case 4:
			getEntities()->clearSequences(kCharacterHeadWait);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 8) = 0;
			ENTITY_PARAM(1, 1) = 0;
			ENTITY_PARAM(1, 2) = 1;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Pascale, tellAugust)
	// Tell August the message from Anna
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("902");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterHeadWait, kCharacterAugust, kCharacterAction122358304);
			getEntities()->drawSequenceLeft(kCharacterHeadWait, "010E2");

			setCallback(2);
			setup_playSound("Aug4001");
			break;

		case 2:
			getSavePoints()->push(kCharacterHeadWait, kCharacterAugust, kCharacterAction123793792);

			setCallback(3);
			setup_draw("905");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterHeadWait);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(1, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Pascale, walkDownTrain)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_1540;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("817DD");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kCharacterHeadWait, "817DS");
			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterHeadWait);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getData()->entityPosition = kPosition_850;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Pascale, walkUpTrain)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_9270;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("817US");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kCharacterHeadWait, "817UD");
			if (getEntities()->isInSalon(kCharacterCath))
				getEntities()->updateFrame(kCharacterHeadWait);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getData()->entityPosition = kPosition_5900;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Pascale, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterHeadWait);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Pascale, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5)
		setup_hiding();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Pascale, hiding)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param4) {
			if (Entity::updateParameter(params->param5, getState()->time, 4500)) {
				getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorNormal, kCursorNormal);

				setCallback(1);
				setup_playSound("Wat5010");
				break;
			}
		}

label_callback1:
		if (params->param1) {
			if (!Entity::updateParameter(params->param6, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 1;

			getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param6 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorNormal, kCursorNormal);
			params->param1 = 0;

			setCallback(2);
			setup_playSound(getSound()->justCheckingCath());
		} else {
			setCallback(savepoint.action == kCharacterActionKnock ? 3 : 4);
			setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kCharacterActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;

		getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (params->param2 || params->param1) {
			params->param1 = 0;
			params->param2 = 0;
			params->param3 = 0;

			getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorHandKnock, kCursorHand);
			goto label_callback1;

		case 2:
			getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 3:
		case 4:
			params->param3++;

			if (params->param3 == 1 || params->param3 == 2) {
				getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorNormal, kCursorNormal);
				setCallback(params->param3 == 1 ? 5 : 6);
				setup_playSound(params->param3 == 1 ? "Wat5001" : "Wat5001A");
			}
			break;

		case 5:
			params->param1 = 1;
			getObjects()->update(kObjectCompartmentG, kCharacterHeadWait, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;

		case 6:
			params->param2 = 1;
			break;

		case 7:
			params->param4 = 1;
			break;
		}
		break;

	case kCharacterAction135800432:
		setup_nullfunction();
		break;

	case kCharacterAction169750080:
		if (getSoundQueue()->isBuffered(kCharacterHeadWait)) {
			params->param4 = 1;
		} else {
			setCallback(7);
			setup_playSound("Wat5002");
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(34, Pascale)

} // End of namespace LastExpress
