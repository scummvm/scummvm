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

#include "lastexpress/entities/boutarel.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Boutarel::Boutarel(LastExpressEngine *engine) : Entity(engine, kCharacterMonsieur) {
	ADD_CALLBACK_FUNCTION(Boutarel, reset);
	ADD_CALLBACK_FUNCTION_S(Boutarel, playSound);
	ADD_CALLBACK_FUNCTION_S(Boutarel, draw);
	ADD_CALLBACK_FUNCTION_I(Boutarel, updateFromTime);
	ADD_CALLBACK_FUNCTION_SII(Boutarel, updatePosition);
	ADD_CALLBACK_FUNCTION_SI(Boutarel, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_SI(Boutarel, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION(Boutarel, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION(Boutarel, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION_II(Boutarel, updateEntity);
	ADD_CALLBACK_FUNCTION_I(Boutarel, function11);
	ADD_CALLBACK_FUNCTION(Boutarel, enterTableWithMmeBoutarel);
	ADD_CALLBACK_FUNCTION(Boutarel, leaveTableWithMmeBoutarel);
	ADD_CALLBACK_FUNCTION_I(Boutarel, function14);
	ADD_CALLBACK_FUNCTION_IS(Boutarel, function15);
	ADD_CALLBACK_FUNCTION_IS(Boutarel, function16);
	ADD_CALLBACK_FUNCTION_IS(Boutarel, function17);
	ADD_CALLBACK_FUNCTION_I(Boutarel, function18);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter1);
	ADD_CALLBACK_FUNCTION(Boutarel, function20);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function22);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter2);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function25);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter3);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function28);
	ADD_CALLBACK_FUNCTION(Boutarel, function29);
	ADD_CALLBACK_FUNCTION(Boutarel, function30);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter4);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function33);
	ADD_CALLBACK_FUNCTION(Boutarel, function34);
	ADD_CALLBACK_FUNCTION(Boutarel, function35);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter5);
	ADD_CALLBACK_FUNCTION(Boutarel, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Boutarel, function38);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Boutarel, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Boutarel, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Boutarel, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Boutarel, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(5, Boutarel, updatePosition, CarIndex, PositionOld)
	Entity::updatePosition(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, Boutarel, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(7, Boutarel, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_6470, kPosition_6130, kCarRedSleeping, kObjectCompartmentC, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Boutarel, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Boutarel, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Boutarel, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath) {

		if (getInventory()->hasItem(kItemPassengerList) && getState()->time > kTime1089000)
			getSound()->playSound(kCharacterCath, "CAT1022");
		else
			getSound()->excuseMeCath();

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(11, Boutarel, function11, bool)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (ENTITY_PARAM(0, 1) && ENTITY_PARAM(0, 2)) {
			ENTITY_PARAM(0, 2) = 0;
			ENTITY_PARAM(0, 1) = 0;

			setCallback(5);
			setup_callbackActionRestaurantOrSalon();
		}
		break;

	case kCharacterActionDefault:
		if (params->param1) {
			if (getProgress().chapter == kChapter4) {
				getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

				setCallback(1);
				setup_enterExitCompartment("607Hc", kObjectCompartmentC);
			} else {
				getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

				setCallback(2);
				setup_enterExitCompartment("607Dc", kObjectCompartmentC);
			}
		} else {
			setCallback(3);
			setup_enterExitCompartment("607Bc", kObjectCompartmentC);
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 2:
		case 3:
			if (getCallback() == 2)
				getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			else
				getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			// fall through

		case 1:
			getObjects()->update(kObject50, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kCharacterMonsieur, kCharacterFrancois, kCharacterAction101107728);

			setCallback(4);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 4:
			getEntities()->clearSequences(kCharacterMonsieur);
			break;

		case 5:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(6);
			setup_draw("812US");
			break;

		case 6:
			switch (getProgress().chapter) {
			default:
				break;

			case kChapter1:
				getSound()->playSound(kCharacterMonsieur, "MRB1075", kSoundVolumeEntityDefault, 60);
				break;

			case kChapter3:
				getSound()->playSound(kCharacterMonsieur, "MRB3101");
				break;
			}

			setCallback(7);
			setup_enterTableWithMmeBoutarel();
			break;

		case 7:
			getData()->location = kLocationInsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Boutarel, enterTableWithMmeBoutarel)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionExitCompartment:
		getEntities()->clearSequences(kCharacterMadame);
		getSavePoints()->push(kCharacterMonsieur, kCharacterTableC, kCharacterAction136455232);
		getData()->location = kLocationInsideCompartment;

		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterTableC, "008A3");
		getEntities()->drawSequenceRight(kCharacterMadame, "008A2");
		getEntities()->drawSequenceRight(kCharacterMonsieur, "008A1");

		if (getEntities()->isInSalon(kCharacterCath)) {
			getEntities()->updateFrame(kCharacterMonsieur);
			getEntityData(kCharacterMadame)->field_4A1 = getData()->field_4A1;
			getEntityData(kCharacterTableC)->field_4A1 = getData()->field_4A1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Boutarel, leaveTableWithMmeBoutarel)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionExitCompartment:
		getSavePoints()->push(kCharacterMonsieur, kCharacterTableC, kCharacterActionDrawTablesWithChairs, "008F");
		getEntities()->clearSequences(kCharacterMadame);

		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterTableC, "008E3");
		getEntities()->drawSequenceRight(kCharacterMadame, "008E2");
		getEntities()->drawSequenceRight(kCharacterMonsieur, "008E1");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(14, Boutarel, function14, bool)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getSound()->playSound(kCharacterMonsieur, "MRB1079");

			setCallback(2);
			setup_leaveTableWithMmeBoutarel();
			break;

		case 2:
			getSavePoints()->push(kCharacterMonsieur, kCharacterWaiter2, kCharacterAction326144276);
			getEntities()->drawSequenceRight(kCharacterMonsieur, "812DS");
			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterMonsieur);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			getEntityData(kCharacterFrancois)->entityPosition = kPosition_540;
			getEntityData(kCharacterMadame)->entityPosition = kPosition_540;
			getData()->entityPosition = kPosition_540;

			getEntityData(kCharacterFrancois)->location = kLocationOutsideCompartment;
			getEntityData(kCharacterMadame)->location = kLocationOutsideCompartment;

			getEntities()->clearSequences(kCharacterMonsieur);
			getSavePoints()->push(kCharacterMonsieur, kCharacterMadame, kCharacterAction100901266);

			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			getSavePoints()->push(kCharacterMonsieur, kCharacterFrancois, kCharacterAction100901266);

			setCallback(5);
			setup_updateFromTime(450);
			break;

		case 5:
			setCallback(6);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 6:
			setCallback(params->param1 ?  7: 8);
			setup_enterExitCompartment2(params->param1 ? "607Gc" : "607Ac", kObjectCompartmentC);
			break;

		case 7:
		case 8:
			getEntities()->clearSequences(kCharacterMonsieur);
			getData()->location = kLocationInsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IS(15, Boutarel, function15, bool)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		if (params->param1)
			getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

		setCallback(params->param1 ? 1 : 2);
		setup_enterExitCompartment(params->param1 ? "607Dc" : "607Bc", kObjectCompartmentC);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getObjects()->update(kObject50, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(3);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(3);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 3:
			setCallback(4);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 4:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(5);
			setup_updatePosition(params->seq, kCarRestaurant, 52);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Parameters:
// bool
// const char *
IMPLEMENT_FUNCTION_IS(16, Boutarel, function16, bool)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_callbackActionRestaurantOrSalon();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updatePosition((const char *)&params->seq, kCarRestaurant, 52);
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_6470);
			break;

		case 3:
			setCallback(params->param1 ? 4 : 5);
			setup_enterExitCompartment2(params->param1 ? "607Gc" : "607Ac", kObjectCompartmentC);
			break;

		case 4:
		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMonsieur);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IS(17, Boutarel, function17, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (Entity::timeCheckCallbackAction((TimeValue)params->param1, params->param6))
			break;

		if (params->param5) {
			if (!Entity::updateParameter(params->param7, getState()->timeTicks, 90))
				break;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 51);
		} else {
			params->param7 = 0;
		}
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, (char *)&params->seq);
		break;

	case kCharacterActionDrawScene:
		params->param5 = (getEntities()->isPlayerPosition(kCarRestaurant, 52) ? 1 : 0);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(18, Boutarel, function18, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1 < getState()->time && !params->param4) {
			params->param4 = 1;

			getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}

		if (params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param2 = 0;
			params->param3 = 1;

			getObjects()->update(kObjectCompartmentC, kCharacterMonsieur, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject50, kCharacterMonsieur, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param5 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		getObjects()->update(kObjectCompartmentC, kCharacterMonsieur, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject50, kCharacterMonsieur, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (params->param2) {
			if (savepoint.param.intValue == 50) {
				setCallback(4);
				setup_playSound(getSound()->justAMinuteCath());
			} else if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1511" : getSound()->wrongDoorCath());
			} else {
				setCallback(6);
				setup_playSound(getSound()->wrongDoorCath());
			}
		} else {
			setCallback(savepoint.action == kCharacterActionKnock ? 1 : 2);
			setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentC, kCharacterMonsieur, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kCharacterMonsieur, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (params->param3 || params->param2) {
			getObjects()->update(kObjectCompartmentC, kCharacterMonsieur, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject50, kCharacterMonsieur, kObjectLocation1, kCursorHandKnock, kCursorHand);

			params->param2 = 0;
			params->param3 = 0;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound(rnd(2) ? "MRB1001" : "MRB1001A");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentC, kCharacterMonsieur, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject50, kCharacterMonsieur, kObjectLocation1, kCursorTalk, kCursorNormal);

			params->param2 = 1;
			break;

		case 4:
		case 5:
		case 6:
			params->param2 = 0;
			params->param3 = 1;
			break;

		case 7:
			getSavePoints()->push(kCharacterMonsieur, kCharacterCond2, kCharacterAction123199584);
			break;

		}
		break;

	case kCharacterAction122865568:
		getSavePoints()->push(kCharacterMonsieur, kCharacterCond2, kCharacterAction88652208);
		break;

	case kCharacterAction221683008:
		setCallback(7);
		setup_playSound("MRB1001");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Boutarel, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Boutarel, setup_chapter1Handler));
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterMonsieur, kCharacterAction203520448, 0);
		getSavePoints()->addData(kCharacterMonsieur, kCharacterAction237889408, 1);

		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject42, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_1750;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Boutarel, function20)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1)
			break;

		if (!params->param2) {
			if (Entity::updateParameter(params->param3, getState()->time, 4500)) {
				setCallback(3);
				setup_playSound("MRB1078A");
				break;
			}
		}

		Entity::timeCheckCallback(kTime1138500, params->param4, 4, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_function11(false);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterMonsieur, "008B");

			setCallback(2);
			setup_playSound("MRB1076");
			break;

		case 2:
			getSavePoints()->push(kCharacterMonsieur, kCharacterWaiter2, kCharacterAction256200848);
			break;

		case 3:
			Entity::timeCheckCallback(kTime1138500, params->param4, 4, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
			break;

		case 4:
			getSavePoints()->push(kCharacterMonsieur, kCharacterCook, kCharacterAction224849280);

			callbackAction();
			break;
		}
		break;

	case kCharacterAction134466544:
		params->param2 = 0;
		break;

	case kCharacterAction135854206:
		params->param2 = 1;
		break;

	case kCharacterAction168717392:
		params->param1 = 1;
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "008D");

		if (!params->param2) {
			setCallback(5);
			setup_playSound("MRB1078");
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Boutarel, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_function17(kTime1071000, "101A");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function16(false, "101B");
			break;

		case 2:
			setCallback(3);
			setup_function18(kTime1102500);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject50, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);

			if (getEntities()->isPlayerPosition(kCarRedSleeping, 54) || getEntities()->isPlayerPosition(kCarRedSleeping, 44))
				getScenes()->loadSceneFromPosition(kCarRedSleeping, 10);

			getEntities()->updatePositionEnter(kCharacterMonsieur, kCarRedSleeping, 54);
			getEntities()->updatePositionEnter(kCharacterMonsieur, kCarRedSleeping, 44);

			setCallback(4);
			setup_playSound("MRB1074");
			break;

		case 4:
			getEntities()->updatePositionExit(kCharacterMonsieur, kCarRedSleeping, 54);
			getEntities()->updatePositionExit(kCharacterMonsieur, kCarRedSleeping, 44);

			setCallback(5);
			setup_function20();
			break;

		case 5:
			setCallback(6);
			setup_function18(kTimeEnterChalons);
			break;

		case 6:
			setCallback(7);
			setup_function15(false, "102A");
			break;

		case 7:
			setCallback(8);
			setup_function17(kTime1183500, "102B");
			break;

		case 8:
			setCallback(9);
			setup_function16(false, "102C");
			break;

		case 9:
			setCallback(10);
			setup_function18(kTime1215000);
			break;

		case 10:
			setup_function22();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Boutarel, function22)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getEntities()->clearSequences(kCharacterMonsieur);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Boutarel, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter2Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMonsieur);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Boutarel, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheckCallback(kTime1759500, params->param2, 1, true, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "008D");
		break;

	case kCharacterActionDrawScene:
		if (getEntities()->isInRestaurant(kCharacterCath) && !params->param1) {
			getSound()->playSound(kCharacterMonsieur, "MRB2001");
			params->param1 = 1;
		}
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			setup_function25();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Boutarel, function25)
	if (savepoint.action == kCharacterActionDefault) {
		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "510");
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Boutarel, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter3Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMonsieur);

		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObject50, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Boutarel, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "510");
		break;

	case kCharacterAction122288808:
		setup_function28();
		break;

	case kCharacterAction122358304:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Boutarel, function28)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_function11(true);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			setup_function29();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Boutarel, function29)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (Entity::updateParameter(params->param2, getState()->time, 450)) {
			getSavePoints()->push(kCharacterMonsieur, kCharacterWaiter2, kCharacterAction256200848);
		}

		if (!params->param1)
			break;

		if (getEntities()->isInRestaurant(kCharacterAnna)
		 && getEntities()->isInRestaurant(kCharacterAugust)
		 && !getSoundQueue()->isBuffered(kCharacterMonsieur)
		 && params->param3 != kTimeInvalid) {

			if (getState()->time <= kTime1998000)
				if (!getEntities()->isInRestaurant(kCharacterCath) || !params->param3)
					params->param3 = (uint)(getState()->time + 450);

			if (params->param3 < getState()->time || getState()->time > kTime1998000) {
				params->param3 = kTimeInvalid;

				setCallback(1);
				setup_playSound("MRB3102");
				break;
			}
		}

		Entity::timeCheckCallback(kTime2002500, params->param4, 2, true, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "008B");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			Entity::timeCheckCallback(kTime2002500, params->param4, 2, true, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
			break;

		case 2:
			setup_function30();
			break;
		}
		break;

	case kCharacterAction122288808:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "008D");
		params->param1 = 1;
		break;

	case kCharacterAction122358304:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Boutarel, function30)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "510");
		break;

	case kCharacterAction122288808:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "510");
		break;

	case kCharacterAction122358304:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Boutarel, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter4Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMonsieur);

		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObject50, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Boutarel, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTime2367000, params->param1, WRAP_SETUP_FUNCTION(Boutarel, setup_function33));
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "510");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Boutarel, function33)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1)
			Entity::timeCheckCallback(kTime2389500, params->param2, 3, false, WRAP_SETUP_FUNCTION_B(Boutarel, setup_function14));
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_function11(true);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterMonsieur, "008B");

			setCallback(2);
			setup_updateFromTime(450);
			break;

		case 2:
			getSavePoints()->push(kCharacterMonsieur, kCharacterWaiter2, kCharacterAction256200848);
			break;

		case 3:
			setup_function34();
			break;
		}
		break;

	case kCharacterAction122288808:
		params->param1 = 1;
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "008D");
		break;

	case kCharacterAction122358304:
		getEntities()->drawSequenceLeft(kCharacterMonsieur, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Boutarel, function34)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (Entity::timeCheck(kTime2470500, params->param1, WRAP_SETUP_FUNCTION(Boutarel, setup_function35)))
			break;

		if (getState()->time > kTime2457000 && HELPERgetEvent(kEventAugustDrink)) {
			getSavePoints()->push(kCharacterMonsieur, kCharacterAbbot, kCharacterAction159003408);

			setCallback(1);
			setup_function15(false, "102A");
		}
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterMonsieur, kCharacterAbbot, kCharacterAction101687594);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_function17(kTime2479500, "102B");
			break;

		case 2:
			setCallback(3);
			setup_function16(false, "102C");
			break;

		case 3:
		case 7:
			setup_function35();
			break;

		case 4:
		case 8:
			if (getState()->time >= kTime2470500) {
				setup_function35();
				break;
			}

			if (HELPERgetEvent(kEventAugustDrink)) {
				setCallback(5);
				setup_function15(false, "102A");
			} else {
				setCallback(8);
				setup_function18((TimeValue)(getState()->time + 900));
			}
			break;

		case 5:
			setCallback(6);
			setup_function17(kTime2479500, "102B");
			break;

		case 6:
			setCallback(7);
			setup_function16(false, "102C");
			break;

		case 9:
			getSavePoints()->push(kCharacterMonsieur, kCharacterCond2, kCharacterAction123199584);
			break;
		}
		break;

	case kCharacterAction122865568:
		getSavePoints()->push(kCharacterMonsieur, kCharacterCond2, kCharacterAction88652208);
		break;

	case kCharacterAction125039808:
		setCallback(4);
		setup_function18(kTime2457000);
		break;

	case kCharacterAction221683008:
		setCallback(9);
		setup_playSound("Mrb1001");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Boutarel, function35)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_6470;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kCharacterMonsieur);

		getObjects()->update(kObjectCompartmentC, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject50, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Boutarel, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMonsieur);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Boutarel, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5)
		setup_function38();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Boutarel, function38)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kCharacterAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(39, Boutarel)

} // End of namespace LastExpress
