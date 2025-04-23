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

#include "lastexpress/entities/vesna.h"

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

Vesna::Vesna(LastExpressEngine *engine) : Entity(engine, kCharacterVesna) {
	ADD_CALLBACK_FUNCTION(Vesna, reset);
	ADD_CALLBACK_FUNCTION_S(Vesna, playSound);
	ADD_CALLBACK_FUNCTION_SI(Vesna, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_S(Vesna, draw);
	ADD_CALLBACK_FUNCTION_II(Vesna, updateEntity);
	ADD_CALLBACK_FUNCTION_I(Vesna, updateFromTime);
	ADD_CALLBACK_FUNCTION_II(Vesna, updateEntity2);
	ADD_CALLBACK_FUNCTION(Vesna, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION(Vesna, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION_II(Vesna, savegame);
	ADD_CALLBACK_FUNCTION_TYPE(Vesna, homeAlone, EntityParametersIIIS);
	ADD_CALLBACK_FUNCTION(Vesna, chapter1);
	ADD_CALLBACK_FUNCTION(Vesna, withMilos);
	ADD_CALLBACK_FUNCTION(Vesna, homeTogether);
	ADD_CALLBACK_FUNCTION(Vesna, function15);
	ADD_CALLBACK_FUNCTION(Vesna, chapter2);
	ADD_CALLBACK_FUNCTION(Vesna, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Vesna, checkTrain);
	ADD_CALLBACK_FUNCTION(Vesna, chapter3);
	ADD_CALLBACK_FUNCTION_TYPE(Vesna, inCompartment, EntityParametersIIIS);
	ADD_CALLBACK_FUNCTION(Vesna, takeAWalk);
	ADD_CALLBACK_FUNCTION(Vesna, killAnna);
	ADD_CALLBACK_FUNCTION(Vesna, killedAnna);
	ADD_CALLBACK_FUNCTION(Vesna, chapter4);
	ADD_CALLBACK_FUNCTION(Vesna, exitLocation);
	ADD_CALLBACK_FUNCTION(Vesna, done);
	ADD_CALLBACK_FUNCTION(Vesna, function27);
	ADD_CALLBACK_FUNCTION(Vesna, chapter5);
	ADD_CALLBACK_FUNCTION(Vesna, guarding);
	ADD_CALLBACK_FUNCTION(Vesna, climbing);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Vesna, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Vesna, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Vesna, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(4, Vesna, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(5, Vesna, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath) {
		getSound()->playSound(kCharacterCath, rnd(2) ? "CAT1015" : "CAT1015A");

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(6, Vesna, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Vesna, updateEntity2, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		params->param3 = 0;

		if (getEntities()->isDistanceBetweenEntities(kCharacterVesna, kCharacterMilos, 500)
		 || (getData()->direction == kDirectionUp && (getData()->car > getEntityData(kCharacterMilos)->car || (getData()->car == getEntityData(kCharacterMilos)->car && getData()->entityPosition > getEntityData(kCharacterMilos)->entityPosition)))
		 || (getData()->direction == kDirectionDown && (getData()->car < getEntityData(kCharacterMilos)->car || (getData()->car == getEntityData(kCharacterMilos)->car && getData()->entityPosition < getEntityData(kCharacterMilos)->entityPosition)))) {
			getData()->field_49B = 0;
			params->param3 = 1;
		}

		if (!params->param3)
			getEntities()->updateEntity(kCharacterVesna, (CarIndex)params->param1, (EntityPosition)params->param2);
		break;

	case kCharacterActionDefault:
		getEntities()->updateEntity(kCharacterVesna, (CarIndex)params->param1, (EntityPosition)params->param2);
		break;

	case kCharacterAction123668192:
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Vesna, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Vesna, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Vesna, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Vesna, homeAlone)
	// Expose parameters as IIIS and ignore the default exposed parameters
	EntityData::EntityParametersIIIS *parameters = (EntityData::EntityParametersIIIS*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (parameters->param3) {
			if (!Entity::updateParameter(parameters->param7, getState()->timeTicks, 75))
				break;

			parameters->param2 = 1;
			parameters->param3 = 0;
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		parameters->param7 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (parameters->param3) {
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

			setCallback(4);
			setup_playSound(getSound()->wrongDoorCath());
			break;
		}

		parameters->param1++;
		switch (parameters->param1) {
		default:
			Common::strcpy_s(parameters->seq, "VES1015C");
			parameters->param1 = 0;
			break;

		case 1:
			Common::strcpy_s(parameters->seq, "VES1015A");
			break;

		case 2:
			Common::strcpy_s(parameters->seq, "VES1015B");
			break;
		}

		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kCharacterActionKnock ? 2 : 1);
		setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (parameters->param2 || parameters->param3) {
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation1, kCursorHandKnock, kCursorHand);

			parameters->param2 = 0;
			parameters->param3 = 0;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound(parameters->seq);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorTalk, kCursorNormal);
			parameters->param3 = 1;
			break;

		case 4:
			parameters->param2 = 1;
			parameters->param3 = 0;
			break;
		}
		break;

	case kCharacterAction55996766:
	case kCharacterAction101687594:
		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Vesna, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Vesna, setup_withMilos));
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterVesna, kCharacterAction124190740, 0);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Vesna, withMilos)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		getData()->entityPosition = getEntityData(kCharacterMilos)->entityPosition;
		getData()->location = getEntityData(kCharacterMilos)->location;
		getData()->car = getEntityData(kCharacterMilos)->car;
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getEntities()->clearSequences(kCharacterVesna);
			setup_homeTogether();
		}
		break;

	case kCharacterAction204832737:
		setCallback(1);
		setup_updateEntity2(kCarRedSleeping, kPosition_3050);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Vesna, homeTogether)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kCharacterAction190412928:
		setCallback(1);
		setup_homeAlone();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Vesna, function15)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kCharacterVesna);
		getObjects()->update(kObjectCompartmentG, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Vesna, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter2Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVesna);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Vesna, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterAction135024800:
		setCallback(2);
		setup_checkTrain();
		break;

	case kCharacterAction137165825:
		setCallback(1);
		setup_homeAlone();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Vesna, checkTrain)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("610BG", kObjectCompartmentG);
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
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kCharacterVesna, "808UD");
			if (getEntities()->isInSalon(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterVesna);

			setCallback(6);
			setup_updateFromTime(4500);
			break;

		case 6:
			setCallback(7);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 7:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(8);
			setup_draw("808DD");
			break;

		case 8:
			getEntities()->drawSequenceRight(kCharacterVesna, "808DS");
			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(9);
			setup_callbackActionOnDirection();
			break;

		case 9:
			setCallback(10);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 10:
			setCallback(11);
			setup_enterExitCompartment("610AG", kObjectCompartmentG);
			break;

		case 11:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterVesna);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Vesna, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inCompartment();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVesna);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Vesna, inCompartment)
	EntityData::EntityParametersIIIS *parameters = (EntityData::EntityParametersIIIS*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getProgress().field_54 && parameters->param7 != kTimeInvalid) {
			if (getState()->time > kTime2250000) {
				parameters->param7 = kTimeInvalid;
				setup_killAnna();
				break;
			}

			if (!getEntities()->isPlayerInCar(kCarRedSleeping) || !parameters->param7)
				parameters->param7 = (uint)getState()->time;

			if (parameters->param7 < getState()->time) {
				parameters->param7 = kTimeInvalid;
				setup_killAnna();
				break;
			}
		}

		if (parameters->param2) {
			if (!Entity::updateParameter(parameters->param8, getState()->timeTicks, 75))
				break;

			parameters->param1 = 1;
			parameters->param2 = 0;

			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		parameters->param8 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (parameters->param2) {
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

			setCallback(4);
			setup_playSound(getSound()->wrongDoorCath());
			break;
		}

		++parameters->param3;

		switch (parameters->param3) {
		default:
			Common::strcpy_s(parameters->seq, "VES1015C");
			parameters->param3 = 0;
			break;

		case 1:
			Common::strcpy_s(parameters->seq, "VES1015A");
			break;

		case 2:
			Common::strcpy_s(parameters->seq, "VES1015B");
			break;
		}

		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kCharacterActionKnock ? 2 : 1);
		setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getEntities()->clearSequences(kCharacterVesna);
		break;

	case kCharacterActionDrawScene:
		if (parameters->param1 || parameters->param2) {
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation1, kCursorHandKnock, kCursorHand);
			parameters->param1 = 0;
			parameters->param2 = 0;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound(parameters->seq);
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorTalk, kCursorNormal);
			parameters->param2 = 1;
			break;

		case 4:
			parameters->param1 = 1;
			parameters->param2 = 0;
			break;
		}
		break;

	case kCharacterAction137165825:
		setCallback(5);
		setup_homeAlone();
		break;

	case kCharacterAction155913424:
		setCallback(6);
		setup_takeAWalk();
		break;

	case kCharacterAction203663744:
		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Vesna, takeAWalk)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("610Bg", kObjectCompartmentG);
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
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kCharacterVesna, "808UD");
			if (getEntities()->isInSalon(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kCharacterVesna);
			getData()->entityPosition = kPosition_5900;
			getData()->location = kLocationInsideCompartment;

			setCallback(6);
			setup_updateFromTime(4500);
			break;

		case 6:
			setCallback(7);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 7:
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(8);
			setup_draw("808DD");
			break;

		case 8:
			getEntities()->drawSequenceRight(kCharacterVesna, "808DS");
			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(9);
			setup_callbackActionOnDirection();
			break;

		case 9:
			setCallback(10);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 10:
			setCallback(11);
			setup_enterExitCompartment("610Ag", kObjectCompartmentG); /* BUG the original engine passes 3050 here */
			break;

		case 11:
			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterVesna);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Vesna, killAnna)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterVesna, kCharacterMilos, kCharacterAction259125998);

		setCallback(1);
		setup_enterExitCompartment("610Bg", kObjectCompartmentG);
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
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kCharacterVesna, "808UD");
			if (getEntities()->isInSalon(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kCharacterVesna);
			getData()->car = kCarBaggage;
			getSavePoints()->push(kCharacterVesna, kCharacterAnna, kCharacterAction235856512);
			break;

		case 6:
			getData()->car = kCarRestaurant;
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(7);
			setup_draw("808DD");
			break;

		case 7:
			getEntities()->drawSequenceRight(kCharacterVesna, "808DS");
			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(8);
			setup_callbackActionOnDirection();
			break;

		case 8:
			setCallback(9);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 9:
			setCallback(10);
			setup_enterExitCompartment("610Ag", kObjectCompartmentG); /* BUG the original engine passes 3050 here */
			break;

		case 10:
			getData()->entityPosition = kPosition_3050;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterVesna);

			setup_killedAnna();
			break;
		}
		break;

	case kCharacterAction189299008:
		setCallback(6);
		setup_callbackActionRestaurantOrSalon();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Vesna, killedAnna)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorNormal, kCursorNormal);
		setCallback(savepoint.action == kCharacterActionKnock ? 1 : 2);
		setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound("VES1015A");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kCharacterAction203663744:
		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Vesna, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setCallback(1);
		setup_homeAlone();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVesna);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentG, kCharacterVesna, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			setup_exitLocation();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Vesna, exitLocation)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime2428200 && !params->param1) {
			params->param1 = 1;
			setup_done();
		}
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterVesna, kCharacterMilos, kCharacterAction135600432);

		setCallback(1);
		setup_enterExitCompartment("610BG", kObjectCompartmentG);
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
			setCallback(3);
			setup_callbackActionRestaurantOrSalon();
			break;

		case 3:
			getData()->entityPosition = kPosition_1540;
			getData()->location = kLocationOutsideCompartment;

			setCallback(4);
			setup_draw("808US");
			break;

		case 4:
			getEntities()->drawSequenceRight(kCharacterVesna, "808UD");
			if (getEntities()->isInSalon(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kCharacterVesna);
			getData()->entityPosition = kPosition_5900;
			getData()->location = kLocationInsideCompartment;

			// Original game calls clearSequences a second time
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Vesna, done)
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
			getData()->car = kCarRestaurant;
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_draw("808DD");
			break;

		case 2:
			getEntities()->drawSequenceRight(kCharacterVesna, "808DS");

			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterVesna);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setCallback(4);
			setup_updateEntity(kCarRedSleeping, kPosition_3050);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("610AG", kObjectCompartmentG);
			break;

		case 5:
			setup_function27();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Vesna, function27)
	if (savepoint.action == kCharacterActionDefault) {
		getEntities()->clearSequences(kCharacterVesna);
		getObjects()->update(kObjectCompartmentG, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Vesna, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_guarding();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVesna);

		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Vesna, guarding)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionOpenDoor:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventCathVesnaRestaurantKilled);
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObject64, kCharacterVesna, kObjectLocationNone, kCursorNormal, kCursorForward);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getActionOld()->playAnimation(kEventCathVesnaRestaurantKilled);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
		}
		break;

	case kCharacterAction134427424:
		getObjects()->update(kObject64, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorForward);
		setup_climbing();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Vesna, climbing)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1) {
			if (Entity::updateParameter(params->param3, getState()->timeTicks, 120)) {
				getSound()->playSound(kCharacterVesna, "Ves5001", kVolumeFull);
				params->param1 = 1;
			}
		}

		if (!Entity::updateParameter(params->param4, getState()->timeTicks, 180))
			break;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopKilled);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			getActionOld()->playAnimation(kEventCathVesnaTrainTopKilled);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;

		case 3:
			getActionOld()->playAnimation(kEventCathVesnaTrainTopFight);

			setCallback(4);
			setup_savegame(kSavegameTypeTime, kTimeNone);
			break;

		case 4:
			//params->param2 = getFight()->setup(kFightVesna);

			//if (params->param2) {
			//	getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, params->param2 == Fight::kFightEndLost);
			//} else {
			//	getSound()->playSound(kCharacterCath, "TUNNEL");
			//	// TODO: fade to black screen
			//
			//	getState()->time = (TimeValue)(getState()->time + 1800);
			//
			//	setCallback(5);
			//	setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopWin);
			//}
			break;

		case 5:
			getActionOld()->playAnimation(kEventCathVesnaTrainTopWin);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 11);

			setup_nullfunction();
			break;
		}
		break;

	case kCharacterAction167992577:
		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopFight);
		break;

	case kCharacterAction202884544:
		if (params->param1) {
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventCathVesnaTrainTopKilled);
		} else {
			getSound()->playSound(kCharacterVesna, "Ves5001", kVolumeFull);
			params->param1 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(31, Vesna)

} // End of namespace LastExpress
