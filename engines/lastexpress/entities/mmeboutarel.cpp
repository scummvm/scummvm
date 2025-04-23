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

#include "lastexpress/entities/mmeboutarel.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

MmeBoutarel::MmeBoutarel(LastExpressEngine *engine) : Entity(engine, kCharacterMadame) {
	ADD_CALLBACK_FUNCTION(MmeBoutarel, reset);
	ADD_CALLBACK_FUNCTION_S(MmeBoutarel, playSound);
	ADD_CALLBACK_FUNCTION_S(MmeBoutarel, draw);
	ADD_CALLBACK_FUNCTION_I(MmeBoutarel, updateFromTime);
	ADD_CALLBACK_FUNCTION_SI(MmeBoutarel, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_SI(MmeBoutarel, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION_II(MmeBoutarel, updateEntity);
	ADD_CALLBACK_FUNCTION_S(MmeBoutarel, function8);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function9);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter1);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function11);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter1Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function13);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function14);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function15);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function16);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter2);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter2Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function19);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter3);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter3Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter4);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter4Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function24);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function25);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter5);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, chapter5Handler);
	ADD_CALLBACK_FUNCTION(MmeBoutarel, function28);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, MmeBoutarel, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, MmeBoutarel, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, MmeBoutarel, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, MmeBoutarel, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(5, MmeBoutarel, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, MmeBoutarel, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_5790, kPosition_6130, kCarRedSleeping, kObjectCompartmentD, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, MmeBoutarel, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath) {
		getInventory()->hasItem(kItemPassengerList) ? getSound()->playSound(kCharacterCath, "CAT1021") : getSound()->excuseMeCath();

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(8, MmeBoutarel, function8)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param4 && params->param5) {
			getSavePoints()->push(kCharacterMadame, kCharacterCond2, kCharacterAction125499160);

			if (!getEntities()->isPlayerPosition(kCarRedSleeping, 2))
				getData()->entityPosition = kPosition_2088;

			callbackAction();
		}
		break;

	case kCharacterActionEndSound:
		params->param5 = 1;
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterMadame, "606U");
		getSavePoints()->push(kCharacterMadame, kCharacterCond2, kCharacterAction169557824);
		break;

	case kCharacterAction155853632:
		params->param4 = 1;
		break;

	case kCharacterAction202558662:
		getEntities()->drawSequenceLeft(kCharacterMadame, "606L");
		getSound()->playSound(kCharacterMadame, (char *)&params->seq1);

		if (getEntities()->hasValidFrame(kCharacterMadame) || getEntities()->isDistanceBetweenEntities(kCharacterMadame, kCharacterCath, 2000)) {
			if (getProgress().chapter == kChapter1)
				getProgress().field_A8 = 1;
			else if (getProgress().chapter == kChapter3)
				getProgress().field_A4 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, MmeBoutarel, function9)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1) {
			getData()->entityPosition = getEntityData(kCharacterMonsieur)->entityPosition;
			getData()->location = getEntityData(kCharacterMonsieur)->location;
			getData()->car = getEntityData(kCharacterMonsieur)->car;
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		setCallback(1);
		setup_enterExitCompartment("606Rd", kObjectCompartmentD);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getEntities()->clearSequences(kCharacterMadame);
			getSavePoints()->push(kCharacterMadame, kCharacterMonsieur, kCharacterAction203520448);
			break;

		case 3:
			if (getEntities()->isInsideCompartment(kCharacterFrancois, kCarRedSleeping, kPosition_5790)) {
				getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorNormal);

				setCallback(4);
				setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
			} else {
				params->param1 = 1;
				getEntities()->drawSequenceLeft(kCharacterMadame, "606Md");
				getEntities()->enterCompartment(kCharacterMadame, kObjectCompartmentD, true);
			}
			break;

		case 4:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMadame);

			callbackAction();
			break;

		case 5:
			getEntities()->exitCompartment(kCharacterMadame, kObjectCompartmentD, true);
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMadame);

			callbackAction();
			break;
		}
		break;

	case kCharacterAction100901266:
		setCallback(3);
		setup_updateEntity(kCarRedSleeping, kPosition_5790);
		break;

	case kCharacterAction100957716:
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorNormal);

		setCallback(5);
		setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, MmeBoutarel, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(MmeBoutarel, setup_chapter1Handler));
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterMadame, kCharacterAction242526416, 0);

		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, MmeBoutarel, function11)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param2 == kTimeInvalid)
			break;

		if (params->param1 >= getState()->time) {
			if (!getEntities()->isDistanceBetweenEntities(kCharacterMadame, kCharacterCath, 1000) || !params->param2)
				params->param2 = (uint)getState()->time + 150;

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

		setCallback(1);
		setup_playSound("MME1040");
		break;

	case kCharacterActionDefault:
		params->param1 = (uint)getState()->time + 1800;
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_playSound("MME1040A");
			break;

		case 2:
			setCallback(3);
			setup_playSound("MME1041");
			break;

		case 3:
			setCallback(4);
			setup_updateFromTime(900);
			break;

		case 4:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, MmeBoutarel, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		params->param1 = 1;
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		break;

	case kCharacterActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRedSleeping, 44)) {
			setCallback(1);
			setup_draw("502B");
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterMadame, "502A");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterMadame, "606Qd");
			getEntities()->enterCompartment(kCharacterMadame, kObjectCompartmentD, true);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			params->param1 = 1;
			getEntities()->clearSequences(kCharacterMadame);
			setup_function13();
			break;
		}
		break;

	case kCharacterAction102484312:
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		params->param1 = 1;
		break;

	case kCharacterAction134289824:
		getEntities()->drawSequenceLeft(kCharacterMadame, "502A");
		params->param1 = 0;
		break;

	case kCharacterAction168986720:
		getSavePoints()->push(kCharacterMadame, kCharacterFrancois, kCharacterAction102752636);
		getSound()->playSound(kCharacterMadame, "MME1036");
		getEntities()->exitCompartment(kCharacterMadame, kObjectCompartmentD, true);

		setCallback(3);
		setup_enterExitCompartment("606Fd", kObjectCompartmentD);
		break;

	case kCharacterAction202221040:
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getData()->location = kLocationOutsideCompartment;

		getSound()->playSound(kCharacterMadame, "MME1035A");

		if (getEntities()->hasValidFrame(kCharacterMadame) || getEntities()->isDistanceBetweenEntities(kCharacterMadame, kCharacterCath, 2000) )
			getProgress().field_AC = 1;

		setCallback(2);
		setup_enterExitCompartment("606Ed", kObjectCompartmentD);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, MmeBoutarel, function13)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!getSoundQueue()->isBuffered(kCharacterMadame) && params->param6 != kTimeInvalid) {
			if (Entity::updateParameterTime((TimeValue)params->param1, !getEntities()->isDistanceBetweenEntities(kCharacterMadame, kCharacterCath, 2000), params->param6, 0)) {
				getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
				getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);

				if (getEntities()->isDistanceBetweenEntities(kCharacterMadame, kCharacterCath, 2000))
					getProgress().field_A0 = 1;

				params->param5 = 1;

				setCallback(1);
				setup_playSound("MME1037");
				break;
			}
		}

label_callback_1:
		if (getProgress().field_24 && params->param7 != kTimeInvalid) {
			if (Entity::updateParameterTime(kTime1093500, (!params->param5 || !getEntities()->isPlayerInCar(kCarRedSleeping)), params->param7, 0)) {
				setCallback(2);
				setup_function11();
				break;
			}
		}

		if (Entity::timeCheck(kTime1094400, params->param8, WRAP_SETUP_FUNCTION(MmeBoutarel, setup_function14)))
			break;

		if (params->param4) {
			if (!Entity::updateParameter(CURRENT_PARAM(1, 1), getState()->timeTicks, 75))
				break;

			params->param3 = 1;
			params->param4 = 0;

			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		CURRENT_PARAM(1, 1) = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param4) {
			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(7);
				setup_playSound(rnd(2) ? "CAT1510" : getSound()->wrongDoorCath());
			} else {
				setCallback(8);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			++params->param2;

			setCallback(savepoint.action == kCharacterActionKnock ? 4 : 3);
			setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kCharacterActionDefault:
		params->param1 = (uint)getState()->time + 900;
		getData()->entityPosition = kPosition_5790;

		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (params->param3 || params->param4) {
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param2 = 0;
			params->param3 = 0;
			params->param4 = 0;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			goto label_callback_1;

		case 2:
			setup_function14();
			break;

		case 3:
		case 4:
			setCallback(params->param2 <= 1 ? 6 : 5);
			setup_playSound(params->param2 <= 1 ? "MME1038" : "MME1038C");
			break;

		case 5:
		case 6:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param4 = 1;
			break;

		case 7:
		case 8:
			params->param3 = 1;
			params->param4 = 0;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, MmeBoutarel, function14)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("606Dd", kObjectCompartmentD);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getEntities()->drawSequenceLeft(kCharacterMadame, "503");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterMadame, "503");

			setCallback(3);
			setup_playSound("MRB1080");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(4);
			setup_enterExitCompartment("606Cd", kObjectCompartmentD);
			break;

		case 4:
			getEntities()->clearSequences(kCharacterMadame);

			setup_function15();
			break;
		}
		break;

	case kCharacterAction101107728:
		setCallback(2);
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, MmeBoutarel, function15)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTimeEnterChalons && !params->param4) {
			params->param4 = 1;

			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(1);
			setup_enterExitCompartment("606Rd", kObjectCompartmentD);
			break;
		}

label_callback_5:
		if (params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param1 = 1;
			params->param2 = 0;

			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param5 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param2) {
			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(10);
				setup_playSound(rnd(2) ? "CAT1510" : getSound()->wrongDoorCath());
			} else {
				setCallback(11);
				setup_playSound(getSound()->wrongDoorCath());
			}
			break;
		}

		++params->param3;

		setCallback(savepoint.action == kCharacterActionKnock ? 7 : 6);
		setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->car = kCarRedSleeping;
		getData()->location = kLocationInsideCompartment;
		getData()->entityPosition = kPosition_5790;
		break;

	case kCharacterActionDrawScene:
		if (params->param1 || params->param2) {
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param1 = 0;
			params->param3 = 0; // BUG" why param3 when it's always param2?
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 2:
			setCallback(3);
			setup_function8("MME1101");
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_5790);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment2("606Td", kObjectCompartmentD);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getData()->entityPosition = kPosition_5790;

			getEntities()->clearSequences(kCharacterMadame);
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			goto label_callback_5;

		case 6:
		case 7:
			if (params->param3 <= 1) {
				setCallback(9);
				setup_playSound("MME1038");
			} else {
				setCallback(8);
				setup_playSound("MME1038C");
			}
			break;

		case 8:
		case 9:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param2 = 1;
			break;

		case 10:
		case 11:
			params->param1 = 1;
			params->param2 = 0;
			break;

		case 12:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kCharacterAction223068211:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(12);
		setup_playSound("MME1151B");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, MmeBoutarel, function16)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getEntities()->clearSequences(kCharacterMadame);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, MmeBoutarel, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter2Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMadame);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject43, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, MmeBoutarel, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getEntities()->isInsideCompartment(kCharacterFrancois, kCarRedSleeping, kPosition_5790)) {
				getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorNormal);

				setCallback(2);
				setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
			} else {
				getEntities()->drawSequenceLeft(kCharacterMadame, "606Md");
				getEntities()->enterCompartment(kCharacterMadame, kObjectCompartmentD, true);
			}
			break;

		case 2:
		case 3:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
			getData()->location = kLocationInsideCompartment;
			setup_function19();
			break;
		}
		break;

	case kCharacterAction100901266:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_5790);
		break;

	case kCharacterAction100957716:
		getEntities()->exitCompartment(kCharacterMadame, kObjectCompartmentD, true);
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorNormal);

		setCallback(3);
		setup_enterExitCompartment2("606Ad", kObjectCompartmentD);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, MmeBoutarel, function19)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isPlayerPosition(kCarRedSleeping, 44) && !params->param2) {
			if (params->param1) {
				setCallback(1);
				setup_draw("502B");
			} else {
				params->param1 = 1;
			}
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		params->param2 = 1;
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping , 44))
				getScenes()->loadSceneFromPosition(kCarRedSleeping, 11);
		}
		break;

	case kCharacterAction102484312:
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		params->param2 = 1;
		break;

	case kCharacterAction134289824:
		getEntities()->drawSequenceLeft(kCharacterMadame, "502A");
		params->param2 = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, MmeBoutarel, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter3Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMadame);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, MmeBoutarel, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (ENTITY_PARAM(0, 1) && params->param2 != kTimeInvalid) {

			if (getState()->time <= kTime2038500) {
				if (!getEntities()->isPlayerInCar(kCarRedSleeping)
				 || !params->param1
				 || getSoundQueue()->isBuffered("FRA2012") // the original game tests this sound twice. Maybe a bug?
				 || getSoundQueue()->isBuffered("FRA2010")
				 ||!params->param2)
					params->param2 = (uint)getState()->time;

				if (params->param2 >= getState()->time)
					break;
			}

			params->param2 = kTimeInvalid;

			getSavePoints()->push(kCharacterMadame, kCharacterFrancois, kCharacterAction189872836);
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(1);
			setup_enterExitCompartment("606Cd", kObjectCompartmentD);
		}
		break;

	case kCharacterActionDefault:
		params->param1 = 1;

		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject43, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		getSavePoints()->push(kCharacterMadame, kCharacterFrancois, kCharacterAction189872836);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("606Rd", kObjectCompartmentD);
			break;

		case 2:
			getData()->location = kLocationOutsideCompartment;

			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_2000);
			break;

		case 3:
			setCallback(4);
			setup_function8("MME3001");
			break;

		case 4:
			setCallback(5);
			setup_updateEntity(kCarRedSleeping, kPosition_5790);
			break;

		case 5:
			setCallback(6);
			setup_enterExitCompartment2("606Td", kObjectCompartmentD);
			break;

		case 6:
			getEntities()->clearSequences(kCharacterMadame);
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

			setCallback(7);
			setup_updateFromTime(150);
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("606Dd", kObjectCompartmentD);
			break;

		case 8:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
			getEntities()->drawSequenceLeft(kCharacterMadame, "501");
			getSavePoints()->push(kCharacterMadame, kCharacterFrancois, kCharacterAction190390860);
			break;

		case 9:
			getEntities()->drawSequenceLeft(kCharacterMadame, "501");
			params->param1 = 1;
			getSavePoints()->push(kCharacterMadame, kCharacterFrancois, kCharacterAction190390860);
			break;
		}
		break;

	case kCharacterAction101107728:
		setCallback(9);
		setup_function9();
		break;

	case kCharacterAction102484312:
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		params->param1 = 1;
		break;

	case kCharacterAction134289824:
		getEntities()->drawSequenceLeft(kCharacterMadame, "502A");
		params->param1 = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, MmeBoutarel, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter4Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMadame);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, MmeBoutarel, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1) {
			if (!Entity::updateParameter(params->param2, getState()->time, 900))
				break;

			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

			setCallback(1);
			setup_enterExitCompartment("606Cd", kObjectCompartmentD);
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getEntities()->drawSequenceLeft(kCharacterMadame, "501");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kCharacterMadame);
			setup_function24();
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterMadame, "501");
			params->param1 = 1;
			break;
		}
		break;

	case kCharacterAction101107728:
		setCallback(2);
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, MmeBoutarel, function24)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (Entity::timeCheck(kTime2470500, params->param4, WRAP_SETUP_FUNCTION(MmeBoutarel, setup_function25)))
			break;

		if (params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param1 = 1;
			params->param2 = 0;

			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}

		params->param5 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param2) {
			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1510" : getSound()->wrongDoorCath());
			} else {
				setCallback(6);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			++params->param3;

			setCallback(savepoint.action == kCharacterActionKnock ? 2 : 1);
			setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (params->param1 || params->param2)	{
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			params->param1 = 0;
			params->param2 = 0;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(params->param3 > 1 ? 3 : 4);
			setup_playSound(params->param3 > 1 ? "MME1038C" : "MME1038");
			break;

		case 3:
		case 4:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param2 = 1;
			break;

		case 5:
		case 6:
			params->param1 = 1;
			params->param2 = 0;
			break;

		case 7:
			getSavePoints()->push(kCharacterMadame, kCharacterCond2, kCharacterAction123199584);
			break;

		case 8:
			getSavePoints()->push(kCharacterMadame, kCharacterCond2, kCharacterAction88652208);
			break;
		}
		break;

	case kCharacterAction122865568:
		setCallback(8);
		setup_playSound("Mme1151A");
		break;

	case kCharacterAction221683008:
		setCallback(7);
		setup_playSound("Mme1038");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, MmeBoutarel, function25)
	if (savepoint.action == kCharacterActionDefault) {
		getEntities()->clearSequences(kCharacterMadame);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, MmeBoutarel, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMadame);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, MmeBoutarel, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5)
		setup_function28();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, MmeBoutarel, function28)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, 75))
				break;

			params->param1 = 0;
			params->param2 = 1;

			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param3 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			params->param1 = 0;

			setCallback(1);
			setup_playSound(getSound()->justCheckingCath());
			break;
		}

		setCallback(savepoint.action == kCharacterActionKnock ? 2 : 3);
		setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;

		getEntities()->clearSequences(kCharacterMadame);
		break;

	case kCharacterActionDrawScene:
		if (params->param1 || params->param2) {
			params->param1 = 0;
			params->param2 = 0;

			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;

		case 2:
		case 3:
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(4);
			setup_playSound("Mme5001");
			break;

		case 4:
			params->param1 = 1;
			getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorTalk, kCursorNormal);
			break;
		}
		break;

	case kCharacterAction135800432:
		setup_nullfunction();
		break;

	case kCharacterAction155604840:
		getObjects()->update(kObjectCompartmentD, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject51, kCharacterMadame, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(29, MmeBoutarel)

} // End of namespace LastExpress
