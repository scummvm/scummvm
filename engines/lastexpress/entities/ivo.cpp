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

#include "lastexpress/entities/ivo.h"

#include "lastexpress/fight/fight.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Ivo::Ivo(LastExpressEngine *engine) : Entity(engine, kCharacterIvo) {
	ADD_CALLBACK_FUNCTION(Ivo, reset);
	ADD_CALLBACK_FUNCTION_S(Ivo, draw);
	ADD_CALLBACK_FUNCTION_SI(Ivo, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_I(Ivo, updateFromTime);
	ADD_CALLBACK_FUNCTION_I(Ivo, updateFromTicks);
	ADD_CALLBACK_FUNCTION_II(Ivo, updateEntity);
	ADD_CALLBACK_FUNCTION(Ivo, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION_S(Ivo, playSound);
	ADD_CALLBACK_FUNCTION(Ivo, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION_II(Ivo, savegame);
	ADD_CALLBACK_FUNCTION(Ivo, goCompartment);
	ADD_CALLBACK_FUNCTION(Ivo, sitAtTableWithSalko);
	ADD_CALLBACK_FUNCTION(Ivo, leaveTableWithSalko);
	ADD_CALLBACK_FUNCTION(Ivo, chapter1);
	ADD_CALLBACK_FUNCTION(Ivo, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Ivo, inCompartment);
	ADD_CALLBACK_FUNCTION(Ivo, function17);
	ADD_CALLBACK_FUNCTION(Ivo, chapter2);
	ADD_CALLBACK_FUNCTION(Ivo, goBreakfast);
	ADD_CALLBACK_FUNCTION(Ivo, atBreakfast);
	ADD_CALLBACK_FUNCTION(Ivo, function21);
	ADD_CALLBACK_FUNCTION(Ivo, chapter3);
	ADD_CALLBACK_FUNCTION(Ivo, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Ivo, chapter4);
	ADD_CALLBACK_FUNCTION(Ivo, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Ivo, returnCompartment4);
	ADD_CALLBACK_FUNCTION(Ivo, inCompartment4);
	ADD_CALLBACK_FUNCTION(Ivo, hiding);
	ADD_CALLBACK_FUNCTION(Ivo, function29);
	ADD_CALLBACK_FUNCTION(Ivo, chapter5);
	ADD_CALLBACK_FUNCTION(Ivo, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Ivo, fightCath);
	ADD_CALLBACK_FUNCTION(Ivo, knockedOut);
	ADD_CALLBACK_FUNCTION(Ivo, function34);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Ivo, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Ivo, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Ivo, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(4, Ivo, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(5, Ivo, updateFromTicks, uint32)
	Entity::updateFromTicks(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(6, Ivo, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath || savepoint.action == kCharacterActionExcuseMe) {
		getSound()->playSound(kCharacterCath, "CAT1127A");
		return;
	}

	Entity::updateEntity(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Ivo, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(8, Ivo, playSound)
	Entity::playSound(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Ivo, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Ivo, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Ivo, goCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isDistanceBetweenEntities(kCharacterIvo, kCharacterSalko, 750) || getEntities()->checkDistanceFromPosition(kCharacterSalko, kPosition_2740, 500)) {
			getSavePoints()->push(kCharacterIvo, kCharacterSalko, kCharacterAction123668192);

			setCallback(4);
			setup_enterExitCompartment("613Ah", kObjectCompartmentH);
		}
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterIvo, "809DS");
		if (getEntities()->isInRestaurant(kCharacterCath))
			getEntities()->updateFrame(kCharacterIvo);

		setCallback(1);
		setup_callbackActionOnDirection();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterIvo, kCharacterSalko, kCharacterAction125242096);

			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_2740);
			break;

		case 2:
			if (getEntities()->isDistanceBetweenEntities(kCharacterIvo, kCharacterSalko, 750) || getEntities()->checkDistanceFromPosition(kCharacterSalko, kPosition_2740, 500)) {
				getSavePoints()->push(kCharacterIvo, kCharacterSalko, kCharacterAction123668192);

				setCallback(3);
				setup_enterExitCompartment("613Ah", kObjectCompartmentH);
			} else {
				getEntities()->drawSequenceLeft(kCharacterIvo, "613Hh");
				getEntities()->enterCompartment(kCharacterIvo, kObjectCompartmentH, true);
			}
			break;

		case 3:
			getData()->entityPosition = kPosition_2740;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterIvo);

			callbackAction();
			break;

		case 4:
			getEntities()->exitCompartment(kCharacterIvo, kObjectCompartmentH, true);
			getData()->entityPosition = kPosition_2740;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterIvo);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Ivo, sitAtTableWithSalko)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionExitCompartment:
		getEntities()->clearSequences(kCharacterSalko);
		getSavePoints()->push(kCharacterIvo, kCharacterTableC, kCharacterAction136455232);

		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterIvo, "023A1");
		getEntities()->drawSequenceRight(kCharacterSalko, "023A2");
		getEntities()->drawSequenceRight(kCharacterTableC, "023A3");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Ivo, leaveTableWithSalko)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionExitCompartment:
		getSavePoints()->push(kCharacterIvo, kCharacterTableC, kCharacterActionDrawTablesWithChairs, "009E");
		getEntities()->clearSequences(kCharacterSalko);

		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterIvo, "023D1");
		getEntities()->drawSequenceRight(kCharacterSalko, "023D2");
		getEntities()->drawSequenceRight(kCharacterTableC, "023D3");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Ivo, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Ivo, setup_chapter1Handler));
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject47, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_4691;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Ivo, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		getData()->entityPosition = getEntityData(kCharacterMilos)->entityPosition;
		getData()->location = getEntityData(kCharacterMilos)->location;
		getData()->car = getEntityData(kCharacterMilos)->car;
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_goCompartment();
			break;

		case 2:
			getSavePoints()->push(kCharacterIvo, kCharacterMilos, kCharacterAction135024800);
			setup_inCompartment();
			break;
		}
		break;

	case kCharacterAction125242096:
		setCallback(1);
		setup_updateFromTicks(75);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Ivo, inCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getEntities()->clearSequences(kCharacterIvo);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterIvo, "613Ch");
			getEntities()->enterCompartment(kCharacterIvo, kObjectCompartmentH, true);
			getSavePoints()->push(kCharacterIvo, kCharacterCond2, kCharacterAction88652208);
			break;

		case 2:
			getData()->entityPosition = kPosition_2740;
			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kCharacterIvo);
			getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kCharacterAction122865568:
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_enterExitCompartment("613Bh", kObjectCompartmentH);
		break;

	case kCharacterAction123852928:
		getEntities()->exitCompartment(kCharacterIvo, kObjectCompartmentH, true);

		setCallback(2);
		setup_enterExitCompartment("613Dh", kObjectCompartmentH);
		break;

	case kCharacterAction221683008:
		getSavePoints()->push(kCharacterIvo, kCharacterCond2, kCharacterAction123199584);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Ivo, function17)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kCharacterIvo);
		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Ivo, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTime1777500, params->param1, WRAP_SETUP_FUNCTION(Ivo, setup_goBreakfast));
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterIvo);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject47, kCharacterCath, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Ivo, goBreakfast)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("613FH", kObjectCompartmentH);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getSavePoints()->push(kCharacterIvo, kCharacterSalko, kCharacterAction136184016);
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("809US");
			break;

		case 4:
			setCallback(5);
			setup_sitAtTableWithSalko();
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			setup_atBreakfast();
			break;
		}
		break;

	case kCharacterAction102675536:
		setCallback(3);
		setup_callbackActionRestaurantOrSalon();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Ivo, atBreakfast)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime1809000 && params->param1) {
			if (getEntities()->isSomebodyInsideRestaurantOrSalon()) {
				getData()->location = kLocationOutsideCompartment;

				setCallback(2);
				setup_leaveTableWithSalko();
			}
		}
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterIvo, kCharacterWaiter2, kCharacterAction189688608);
		getEntities()->drawSequenceLeft(kCharacterIvo, "023B");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			 getSavePoints()->push(kCharacterIvo, kCharacterWaiter2, kCharacterAction101106391);
			 getEntities()->drawSequenceLeft(kCharacterIvo, "023B");
			 params->param1 = 1;
			break;

		case 2:
			setCallback(3);
			setup_goCompartment();
			break;

		case 3:
			getSavePoints()->push(kCharacterIvo, kCharacterWaiter2, kCharacterAction236237423);
			setup_function21();
			break;
		}
		break;

	case kCharacterAction123712592:
		getEntities()->drawSequenceLeft(kCharacterIvo, "023C2");

		setCallback(1);
		setup_updateFromTime(450);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Ivo, function21)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Ivo, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter3Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterIvo);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Ivo, chapter3Handler)
	if (savepoint.action == kCharacterActionDefault)
		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Ivo, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter4Handler();
		break;

	case kCharacterActionDefault:
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Ivo, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime2361600 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			getData()->location = kLocationOutsideCompartment;
			setup_returnCompartment4();
		}
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterIvo, kCharacterTableC, kCharacterAction136455232);
		getEntities()->drawSequenceLeft(kCharacterIvo, "023B");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Ivo, returnCompartment4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_leaveTableWithSalko();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_goCompartment();
			break;

		case 2:
			setup_inCompartment4();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Ivo, inCompartment4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;

			if (getData()->entityPosition < kPosition_2087)
				getData()->entityPosition = kPosition_2088;

			setCallback(2);
			setup_updateEntity(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getEntities()->clearSequences(kCharacterIvo);
			setup_hiding();
			break;

		case 3:
			getEntities()->drawSequenceLeft(kCharacterIvo, "613Ch");
			getEntities()->enterCompartment(kCharacterIvo, kObjectCompartmentH, true);
			getSavePoints()->push(kCharacterIvo, kCharacterCond2, kCharacterAction88652208);
			break;

		case 4:
			getEntities()->exitCompartment(kCharacterIvo, kObjectCompartmentH, true);
			getData()->entityPosition = kPosition_2740;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterIvo);
			break;
		}
		break;

	case kCharacterAction55996766:
		setCallback(1);
		setup_enterExitCompartment("613FH", kObjectCompartmentH);
		break;

	case kCharacterAction122865568:
		getData()->location = kLocationOutsideCompartment;

		setCallback(3);
		setup_enterExitCompartment("613Bh", kObjectCompartmentH);
		break;

	case kCharacterAction123852928:
		setCallback(4);
		setup_enterExitCompartment("613Dh", kObjectCompartmentH);
		break;

	case kCharacterAction221683008:
		getSavePoints()->push(kCharacterIvo, kCharacterCond2, kCharacterAction123199584);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Ivo, hiding)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime2425500 && !params->param1) {
			params->param1 = 1;
			setCallback(1);
			setup_updateEntity(kCarRedSleeping, kPosition_2740);
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("613EH", kObjectCompartmentH);
			break;

		case 2:
			setup_function29();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Ivo, function29)
	if (savepoint.action == kCharacterActionDefault) {
		getEntities()->clearSequences(kCharacterIvo);
		getObjects()->update(kObjectCompartmentH, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Ivo, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterIvo);

		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggageRear;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Ivo, chapter5Handler)
	if (savepoint.action == kCharacterAction192637492)
		setup_fightCath();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Ivo, fightCath)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->location = kLocationOutsideCompartment;
		getData()->entityPosition = kPosition_540;
		getData()->car = kCarBaggageRear;
		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventCathIvoFight);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSound()->playSound(kCharacterCath, "LIB090");
			getActionOld()->playAnimation(kEventCathIvoFight);

			setCallback(2);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 2:
			//params->param1 = getFight()->setup(kFightIvo);
			//if (params->param1) {
			//	getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, params->param1 == Fight::kFightEndLost);
			//} else {
			//	getScenes()->loadSceneFromPosition(kCarBaggageRear, 96);
			//	setup_knockedOut();
			//}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Ivo, knockedOut)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getState()->time = (TimeValue)(getState()->time + 1800);

		setCallback(1);
		setup_savegame(kSavegameTypeTime, kTimeNone);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			getObjects()->update(kObject94, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);

		break;

	case kCharacterAction135800432:
		setup_function34();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Ivo, function34)
	if (savepoint.action == kCharacterActionDefault)
		getEntities()->clearSequences(kCharacterIvo);
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
