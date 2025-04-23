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

#include "lastexpress/entities/tatiana.h"

#include "lastexpress/entities/alexei.h"
#include "lastexpress/entities/coudert.h"

#include "lastexpress/game/action.h"
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

Tatiana::Tatiana(LastExpressEngine *engine) : Entity(engine, kCharacterTatiana) {
	ADD_CALLBACK_FUNCTION(Tatiana, reset);
	ADD_CALLBACK_FUNCTION_S(Tatiana, playSound);
	ADD_CALLBACK_FUNCTION_S(Tatiana, draw);
	ADD_CALLBACK_FUNCTION_SII(Tatiana, updatePosition);
	ADD_CALLBACK_FUNCTION_SI(Tatiana, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_SI(Tatiana, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION_SIIS(Tatiana, callSavepoint);
	ADD_CALLBACK_FUNCTION(Tatiana, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION_I(Tatiana, updateFromTicks);
	ADD_CALLBACK_FUNCTION_I(Tatiana, updateFromTime);
	ADD_CALLBACK_FUNCTION(Tatiana, callbackActionRestaurantOrSalon);
	ADD_CALLBACK_FUNCTION_II(Tatiana, savegame);
	ADD_CALLBACK_FUNCTION_II(Tatiana, updateEntity);
	ADD_CALLBACK_FUNCTION(Tatiana, enterCompartment);
	ADD_CALLBACK_FUNCTION(Tatiana, exitCompartment);
	ADD_CALLBACK_FUNCTION_I(Tatiana, handleCompartment);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter1);
	ADD_CALLBACK_FUNCTION(Tatiana, function18);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, exitDining);
	ADD_CALLBACK_FUNCTION(Tatiana, returnToCompartment);
	ADD_CALLBACK_FUNCTION(Tatiana, getSomeAir);
	ADD_CALLBACK_FUNCTION(Tatiana, returnToCompartmentAgain);
	ADD_CALLBACK_FUNCTION(Tatiana, function24);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter2);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter2Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, breakfastClick);
	ADD_CALLBACK_FUNCTION(Tatiana, joinAlexei);
	ADD_CALLBACK_FUNCTION(Tatiana, leaveBreakfast);
	ADD_CALLBACK_FUNCTION(Tatiana, returnToCompartment2);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter3);
	ADD_CALLBACK_FUNCTION_TYPE2(Tatiana, playChess, EntityParametersI5S, EntityParametersSIII);
	ADD_CALLBACK_FUNCTION(Tatiana, returnToCompartment3);
	ADD_CALLBACK_FUNCTION(Tatiana, beforeConcert);
	ADD_CALLBACK_FUNCTION(Tatiana, concert);
	ADD_CALLBACK_FUNCTION(Tatiana, leaveConcert);
	ADD_CALLBACK_FUNCTION(Tatiana, afterConcert);
	ADD_CALLBACK_FUNCTION(Tatiana, cryAnna);
	ADD_CALLBACK_FUNCTION(Tatiana, function39);
	ADD_CALLBACK_FUNCTION(Tatiana, function40);
	ADD_CALLBACK_FUNCTION(Tatiana, trapCath);
	ADD_CALLBACK_FUNCTION_II(Tatiana, function42);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter4);
	ADD_CALLBACK_FUNCTION(Tatiana, inCompartment4);
	ADD_CALLBACK_FUNCTION(Tatiana, meetAlexei);
	ADD_CALLBACK_FUNCTION_TYPE(Tatiana, withAlexei, EntityParametersI5S);
	ADD_CALLBACK_FUNCTION(Tatiana, thinking);
	ADD_CALLBACK_FUNCTION(Tatiana, seekCath);
	ADD_CALLBACK_FUNCTION(Tatiana, function49);
	ADD_CALLBACK_FUNCTION(Tatiana, alexeiDead);
	ADD_CALLBACK_FUNCTION(Tatiana, function51);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter5);
	ADD_CALLBACK_FUNCTION(Tatiana, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Tatiana, autistic);
	ADD_CALLBACK_FUNCTION(Tatiana, function55);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Tatiana, reset)
	Entity::reset(savepoint, kClothes3, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Tatiana, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Tatiana, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(4, Tatiana, updatePosition, CarIndex, PositionOld)
	Entity::updatePosition(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(5, Tatiana, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(6, Tatiana, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_7500, kPosition_7850, kCarRedSleeping, kObjectCompartmentB);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(7, Tatiana, callSavepoint, CharacterIndex, CharacterActions)
	Entity::callSavepoint(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Tatiana, callbackActionOnDirection)
	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(9, Tatiana, updateFromTicks)
	Entity::updateFromTicks(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(10, Tatiana, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Tatiana, callbackActionRestaurantOrSalon)
	Entity::callbackActionRestaurantOrSalon(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(12, Tatiana, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(13, Tatiana, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath) {
		if (HELPERgetEvent(kEventTatianaAskMatchSpeakRussian) || HELPERgetEvent(kEventTatianaAskMatch) || HELPERgetEvent(kEventVassiliSeizure)) {
			getSound()->playSound(kCharacterCath, rnd(2) ? "CAT1010" : "CAT1010A");
		} else {
			getSound()->excuseMeCath();
		}
		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Tatiana, enterCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterTatiana, kCharacterCond2, kCharacterAction326348944);
		getEntities()->drawSequenceLeft(kCharacterTatiana, getProgress().chapter == kChapter1 ? "603Fb" : "673Fb");
		getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartmentB, true);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1 || getCallback() == 2) {
			getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentB, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterTatiana);

			callbackAction();
		}
		break;

	case kCharacterAction69239528:
		setCallback(getProgress().chapter == kChapter1 ? 1 : 2);
		setup_enterExitCompartment2(getProgress().chapter == kChapter1 ? "603Db" : "673Db", kObjectCompartmentB);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Tatiana, exitCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(getProgress().chapter == kChapter1 ? 1 : 2);
		setup_enterExitCompartment(getProgress().chapter == kChapter1 ? "603Bb" : "673Bb", kObjectCompartmentB);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1 || getCallback() == 2) {
			getData()->location = kLocationOutsideCompartment;
			getSavePoints()->push(kCharacterTatiana, kCharacterCond2, kCharacterAction292048641);

			getEntities()->drawSequenceLeft(kCharacterTatiana, getProgress().chapter == kChapter1 ? "603Fb" : "673Fb");
			getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartmentB, true);
		}
		break;

	case kCharacterAction69239528:
		getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentB, true);
		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

		callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(16, Tatiana, handleCompartment, uint32)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1 < getState()->time && !params->param4) {
			params->param4 = 1;

			getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}

		if (params->param2) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			params->param2 = 0;
			params->param3 = 1;

			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
		}

		params->param5 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (params->param2) {
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			if (savepoint.param.intValue == 49) {
				setCallback(4);
				setup_playSound(getSound()->justAMinuteCath());
				break;
			}

			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1512" : getSound()->wrongDoorCath());
				break;
			}

			setCallback(6);
			setup_playSound(getSound()->wrongDoorCath());
		} else {
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(savepoint.action == kCharacterActionKnock ? 1 : 2);
			setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (params->param2 || params->param3) {
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);

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
			setup_playSound(rnd(2) ? "TAT1133A" : "TAT1133B");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param2 = 1;
			break;

		case 4:
		case 5:
		case 6:
			params->param2 = 0;
			params->param3 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Tatiana, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Tatiana, setup_chapter1Handler));
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterTatiana, kCharacterAction191198209, 0);

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject41, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_5419;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Tatiana, function18)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1) {

			if (getState()->time > kTime1143000 && !params->param2) {
				params->param2 = 1;
				getEntities()->drawSequenceRight(kCharacterTatiana, "806DS");
				params->param1 = 1;
			}

			if (!params->param1) {
				if (Entity::updateParameter(params->param3, getState()->time, 4500)) {
					getEntities()->drawSequenceRight(kCharacterTatiana, "806DS");
					params->param1 = 1;
				}
			}
		}

		if (getData()->entityPosition <= kPosition_2330) {
			getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction157159392);
			getEntities()->clearSequences(kCharacterTatiana);

			callbackAction();
		}
		break;

	case kCharacterActionExitCompartment:
		getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction188784532);

		callbackAction();
		break;

	case kCharacterActionDefault:
		if (getEntities()->isInSalon(kCharacterCath)) {
			getEntities()->drawSequenceRight(kCharacterTatiana, "806DS");
			params->param1 = 1;
		} else {
			getEntities()->clearSequences(kCharacterTatiana);
		}
		break;

	case kCharacterActionDrawScene:
		if (!params->param1 && getEntities()->isInSalon(kCharacterCath)) {
			getEntities()->drawSequenceRight(kCharacterTatiana, "806DS");
			getEntities()->updateFrame(kCharacterTatiana);
			params->param1 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Tatiana, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getSoundQueue()->isBuffered(kCharacterTatiana) || !params->param4 || params->param3 >= 2 || getSoundQueue()->isBuffered("TAT1066"))
			goto label_tatiana_chapter1_2;

		if (Entity::updateParameter(params->param5, getState()->timeTicks, 450)) {
			getSound()->playSound(kCharacterTatiana, params->param3 ? "TAT1069B" : "TAT1069A");
			getProgress().field_64 = 1;
			params->param3++;
			params->param5 = 0;
		}

		if (getEntities()->isPlayerPosition(kCarRestaurant, 71)) {
			if (Entity::updateParameter(params->param6, getState()->timeTicks, 75)) {
				getSound()->playSound(kCharacterTatiana, params->param3 ? "TAT1069B" : "TAT1069A");
				getProgress().field_64 = 1;
				params->param3++;
				params->param6 = 0;
			}
		}

label_tatiana_chapter1_2:
		Entity::timeCheckSavepoint(kTime1084500, params->param7, kCharacterTatiana, kCharacterHeadWait, kCharacterAction257489762);

		if (params->param1) {
			if (!Entity::updateParameter(params->param8, getState()->timeTicks, 90))
				break;

			getScenes()->loadSceneFromPosition(kCarRestaurant, 65);
		} else {
			params->param8 = 0;
		}
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterTatiana, kCharacterTableE, kCharacterAction136455232);
		getEntities()->drawSequenceLeft(kCharacterTatiana, "014A");
		break;

	case kCharacterActionDrawScene:
		params->param1 = getEntities()->isPlayerPosition(kCarRestaurant, 67) ? 1 : 0;
		params->param4 = getEntities()->isPlayerPosition(kCarRestaurant, 69)
		              || getEntities()->isPlayerPosition(kCarRestaurant, 70)
		              || getEntities()->isPlayerPosition(kCarRestaurant, 71);
		break;

	case kCharacterAction122288808:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "014A");
		break;

	case kCharacterAction122358304:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "BLANK");
		break;

	case kCharacterAction124973510:
		setup_exitDining();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Tatiana, exitDining)
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
			getSavePoints()->push(kCharacterTatiana, kCharacterAugust, kCharacterAction223183000);
			getEntities()->updatePositionEnter(kCharacterTatiana, kCarRestaurant, 67);
			getSound()->playSound(kCharacterTatiana, "TAT1070");

			setCallback(2);
			setup_callSavepoint("014C", kCharacterTableE, kCharacterActionDrawTablesWithChairs, "014D");
			break;

		case 2:
			getEntities()->updatePositionExit(kCharacterTatiana, kCarRestaurant, 67);
			getSavePoints()->push(kCharacterTatiana, kCharacterWaiter1, kCharacterAction188893625);

			setCallback(3);
			setup_function18();
			break;

		case 3:
			getSavePoints()->push(kCharacterTatiana, kCharacterAugust, kCharacterAction268620864);
			setup_returnToCompartment();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Tatiana, returnToCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->clothes = kClothes1;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_8513);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->clothes = kClothesDefault;

			getSound()->playSound(kCharacterTatiana, "TAT1071");
			getEntities()->drawSequenceRight(kCharacterTatiana, "604Aa");
			getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartmentA);

			getData()->location = kLocationInsideCompartment;

			if (getEntities()->checkFields19(kCharacterCath, kCarRedSleeping, kPosition_7850)) {
				getActionOld()->playAnimation(isNightOld() ? kEventCathTurningNight : kEventCathTurningDay);
				getSound()->playSound(kCharacterCath, "BUMP");
				getScenes()->loadSceneFromObject(kObjectCompartmentA, true);
			}

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentA);

			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kCharacterTatiana);
			getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction135854208);
			getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
			// fall through

		case 3:
			if (getSoundQueue()->isBuffered(kCharacterTatiana)) {
				setCallback(3);
				setup_updateFromTime(75);
			} else {
				setCallback(4);
				setup_playSound("TAT1071A");
			}
			break;

		case 4:
			getData()->entityPosition = kPosition_7500;

			getSavePoints()->push(kCharacterTatiana, kCharacterVassili, kCharacterAction168459827);

			setCallback(5);
			setup_handleCompartment(kTime1156500);
			break;

		case 5:
		case 6:
			if (getProgress().field_14 == 29) {
				setCallback(6);
				setup_handleCompartment((uint)getState()->time + 900);
			} else {
				getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

				setup_getSomeAir();
			}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Tatiana, getSomeAir)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1 == kTimeInvalid || getState()->time <= kTime1179000)
			goto label_update;

		if (!Entity::updateParameterTime(kTime1233000, ((!HELPERgetEvent(kEventTatianaAskMatchSpeakRussian) && !HELPERgetEvent(kEventTatianaAskMatch)) || getEntities()->isInGreenCarEntrance(kCharacterCath)), params->param1, 0)) {

label_update:
			if (!HELPERgetEvent(kEventTatianaAskMatchSpeakRussian)
			 && !HELPERgetEvent(kEventTatianaAskMatch)
			 && getInventory()->hasItem(kItemMatchBox)
			 && getEntities()->isInGreenCarEntrance(kCharacterCath)) {
				getObjects()->update(kObject25, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorForward);
				getObjects()->update(kObjectTrainTimeTable, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorForward);
			}
			break;
		}

		getObjects()->update(kObject25, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObjectTrainTimeTable, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getEntities()->updatePositionExit(kCharacterTatiana, kCarGreenSleeping, 70);
		getEntities()->updatePositionExit(kCharacterTatiana, kCarGreenSleeping, 71);

		if (getEntities()->isInGreenCarEntrance(kCharacterCath)) {
			getSound()->excuseMe(kCharacterTatiana);

			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
		}

		getData()->inventoryItem = kItemNone;

		setup_returnToCompartmentAgain();
		break;

	case kCharacterAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(4);
		setup_savegame(kSavegameTypeEvent, kEventTatianaGivePoem);
		break;

	case kCharacterActionOpenDoor:
		setCallback(3);
		setup_savegame(kSavegameTypeEvent, kEventTatianaAskMatchSpeakRussian);
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterTatiana, kCharacterVassili, kCharacterAction122732000);

		setCallback(1);
		setup_exitCompartment();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;

		case 2:
			if (getEntities()->isInGreenCarEntrance(kCharacterCath)) {
				getSound()->excuseMe(kCharacterTatiana);

				if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
					getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
			}

			getEntities()->drawSequenceLeft(kCharacterTatiana, "306B");
			getEntities()->updatePositionEnter(kCharacterTatiana, kCarGreenSleeping, 70);
			getEntities()->updatePositionEnter(kCharacterTatiana, kCarGreenSleeping, 71);
			break;

		case 3:
			getActionOld()->playAnimation(HELPERgetEvent(kEventAlexeiSalonVassili) ? kEventTatianaAskMatchSpeakRussian : kEventTatianaAskMatch);
			getScenes()->loadSceneFromPosition(kCarGreenSleeping, 62);
			getData()->inventoryItem = kItemParchemin;

			getObjects()->update(kObject25, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getObjects()->update(kObjectTrainTimeTable, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			break;

		case 4:
			getActionOld()->playAnimation(kEventTatianaGivePoem);
			getInventory()->removeItem(kItemParchemin);
			getScenes()->processScene();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Tatiana, returnToCompartmentAgain)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterCompartment();
			break;

		case 2:
			setup_function24();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Tatiana, function24)
	if (savepoint.action == kCharacterActionDefault) {

		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObject25, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		getObjects()->update(kObjectTrainTimeTable, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getEntities()->updatePositionExit(kCharacterTatiana, kCarGreenSleeping, 70);
		getEntities()->updatePositionExit(kCharacterTatiana, kCarGreenSleeping, 71);
		getEntities()->clearSequences(kCharacterTatiana);

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject41, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Tatiana, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter2Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterTatiana);

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject41, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		getData()->entityPosition = kPosition_5420;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes2;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Tatiana, chapter2Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime1800000 && params->param1 && getEntities()->isSomebodyInsideRestaurantOrSalon()) {
			getData()->inventoryItem = kItemNone;
			setup_joinAlexei();
		}
		break;

	case kCharacterAction1:
		getData()->inventoryItem = kItemNone;
		setup_breakfastClick();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "024A");
		getSavePoints()->push(kCharacterTatiana, kCharacterTableF, kCharacterAction136455232);
		getData()->inventoryItem = kItemInvalid;
		break;

	case kCharacterActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 64) || getEntities()->isPlayerPosition(kCarRestaurant, 65)) {
			getData()->inventoryItem = kItemNone;
			setup_breakfastClick();
		}
		break;

	case kCharacterAction290869168:
		params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Tatiana, breakfastClick)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(HELPERgetEvent(kEventTatianaGivePoem) ? 1 : 2);
		setup_savegame(kSavegameTypeEvent, HELPERgetEvent(kEventTatianaGivePoem) ? kEventTatianaBreakfastAlexei : kEventTatianaBreakfast);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			RESET_ENTITY_STATE(kCharacterAlexei, Alexei, setup_atBreakfast);
			getActionOld()->playAnimation(kEventTatianaBreakfastAlexei);
			getInventory()->addItem(kItemParchemin);
			getInventory()->setLocationAndProcess(kItem11, kObjectLocation1);
			setup_joinAlexei();
			break;

		case 2:
			RESET_ENTITY_STATE(kCharacterAlexei, Alexei, setup_atBreakfast);
			getActionOld()->playAnimation(kEventTatianaBreakfast);
			if (getInventory()->hasItem(kItemParchemin)) {
				getActionOld()->playAnimation(kEventTatianaBreakfastGivePoem);
				getInventory()->removeItem(kItemParchemin);
			} else {
				getActionOld()->playAnimation(kEventTatianaAlexei);
			}
			setup_joinAlexei();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Tatiana, joinAlexei)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->inventoryItem = kItemNone;
		getData()->location = kLocationOutsideCompartment;

		getSavePoints()->push(kCharacterTatiana, kCharacterTableF, kCharacterActionDrawTablesWithChairs, "024D");
		getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction236053296, (HELPERgetEvent(kEventTatianaBreakfastAlexei) || HELPERgetEvent(kEventTatianaBreakfast)) ? 69 : 0);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			setup_leaveBreakfast();
		break;

	case kCharacterAction123857088:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "018G");

		setCallback(1);
		setup_updateFromTime(1800);
		break;

	case kCharacterAction156444784:
		getData()->location = kLocationInsideCompartment;
		getEntities()->drawSequenceLeft(kCharacterTatiana, "BLANK");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Tatiana, leaveBreakfast)
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
			getEntities()->updatePositionEnter(kCharacterTatiana, kCarRestaurant, 63);

			setCallback(2);
			setup_callSavepoint("018H", kCharacterTableB, kCharacterActionDrawTablesWithChairs, "018A");
			break;

		case 2:
			getEntities()->updatePositionExit(kCharacterTatiana, kCarRestaurant, 63);
			getSavePoints()->push(kCharacterTatiana, kCharacterWaiter2, kCharacterAction302203328);
			getEntities()->drawSequenceRight(kCharacterTatiana, "805DS");

			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterTatiana);

			setCallback(3);
			setup_callbackActionOnDirection();
			break;

		case 3:
			setup_returnToCompartment2();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Tatiana, returnToCompartment2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterCompartment();
			break;

		case 2:
			setCallback(3);
			setup_handleCompartment(kTimeEnd);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Tatiana, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_playChess();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterTatiana);

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_1750;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes2;
		getData()->inventoryItem = kItemNone;

		// Update inventory
		getInventory()->get(kItemFirebird)->location = kObjectLocation2;

		if (HELPERgetEvent(kEventTatianaBreakfastGivePoem) || (HELPERgetEvent(kEventTatianaGivePoem) && !HELPERgetEvent(kEventTatianaBreakfastAlexei)))
			getInventory()->get(kItemParchemin)->location = kObjectLocation2;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Tatiana, playChess)
	EntityData::EntityParametersI5S  *parameters = (EntityData::EntityParametersI5S*)_data->getCurrentParameters();
	EntityData::EntityParametersSIII *parameters1 = (EntityData::EntityParametersSIII*)_data->getCurrentParameters(1);

	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!parameters->param2 && !parameters->param5) {
			parameters->param1 -= getState()->timeDelta;

			if (getState()->timeDelta > parameters->param1) {

				getEntities()->drawSequenceLeft(kCharacterTatiana, parameters1->seq);
				getSound()->playSound(kCharacterTatiana, parameters->seq);

				if (parameters->param3 == 4 && getEntities()->isInSalon(kCharacterCath))
					getProgress().field_90 = 1;

				parameters->param2 = 1;
			}
		}

		if (parameters->param4 && parameters->param5) {
			if (Entity::updateParameterCheck(parameters1->param4, getState()->time, 6300)) {
				if (getEntities()->isSomebodyInsideRestaurantOrSalon()) {
					getData()->location = kLocationOutsideCompartment;

					setCallback(1);
					setup_updatePosition("110E", kCarRestaurant, 52);
				}
			}
		}
		break;

	case  kCharacterActionEndSound:
		parameters->param2 = 0;
		++parameters->param3;

		switch (parameters->param3) {
		default:
			parameters->param5 = 1;
			break;

		case 1:
			parameters->param1 = 900;
			getEntities()->drawSequenceLeft(kCharacterTatiana, "110A");
			Common::strcpy_s(parameters->seq, "Tat3160B");
			Common::strcpy_s(parameters1->seq, "110A");
			break;

		case 2:
			parameters->param1 = 9000;
			Common::strcpy_s(parameters->seq, "Tat3160C");
			Common::strcpy_s(parameters1->seq, "110C");
			break;

		case 3:
			parameters->param1 = 13500;
			getEntities()->drawSequenceLeft(kCharacterTatiana, "110B");
			Common::strcpy_s(parameters->seq, "Tat3160D");
			Common::strcpy_s(parameters1->seq, "110D");
			break;

		case 4:
			parameters->param1 = 9000;
			getEntities()->drawSequenceLeft(kCharacterTatiana, "110B");
			Common::strcpy_s(parameters->seq, "Tat3160E");
			Common::strcpy_s(parameters1->seq, "110D");
			break;

		case 5:
			parameters->param1 = 4500;
			getEntities()->drawSequenceLeft(kCharacterTatiana, "110B");
			Common::strcpy_s(parameters->seq, "Tat3160G");
			Common::strcpy_s(parameters1->seq, "110D");
			break;

		case 6:
			parameters->param1 = 4500;
			getEntities()->drawSequenceLeft(kCharacterTatiana, "110B");
			Common::strcpy_s(parameters->seq, "Tat3160F");
			break;
		}
		break;

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction122358304);
		getSavePoints()->push(kCharacterTatiana, kCharacterKronos, kCharacterAction157159392);
		getEntities()->drawSequenceLeft(kCharacterTatiana, "110C");
		getSound()->playSound(kCharacterTatiana, "Tat3160A");

		parameters->param2 = 1;
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction122288808);
			setup_returnToCompartment3();
		}
		break;

	case kCharacterAction101169422:
		parameters->param4 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Tatiana, returnToCompartment3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterTatiana);
		setCallback(1);
		setup_updateFromTime(75);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarRedSleeping, kPosition_7500);
			break;

		case 2:
			setCallback(3);
			setup_enterCompartment();
			break;

		case 3:
			setup_beforeConcert();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Tatiana, beforeConcert)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_handleCompartment(kTime2097000);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getInventory()->get(kItemFirebird)->location = kObjectLocation1;
			if (getEntities()->checkFields19(kCharacterCath, kCarRedSleeping, kPosition_7850))
				getScenes()->loadSceneFromObject(kObjectCompartmentB);

			getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			setCallback(2);
			setup_exitCompartment();
			break;

		case 2:
			setCallback(3);
			setup_updateEntity(kCarKronos, kPosition_9270);
			break;

		case 3:
			setup_concert();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Tatiana, concert)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1
		 && getInventory()->hasItem(kItemFirebird)
		 && getEntities()->checkFields19(kCharacterCath, kCarRedSleeping, kPosition_7850)
		 && (getState()->time < kTime2133000 || getProgress().field_40)) {
			setCallback(1);
			setup_trapCath();
			break;
		}

label_callback_1:
		if (getState()->time > kTime2133000) {
			if (getEntityData(kCharacterAugust)->car >= kCarRedSleeping || (getEntityData(kCharacterAugust)->car == kCarGreenSleeping && getEntityData(kCharacterAugust)->entityPosition > kPosition_5790))
				setup_leaveConcert();
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getEntities()->clearSequences(kCharacterTatiana);

		getData()->car = kCarKronos;
		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationInsideCompartment;
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			params->param1 = 1;
			goto label_callback_1;
		}
		break;

	case kCharacterAction191668032:
		setup_leaveConcert();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Tatiana, leaveConcert)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_850;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (!getEntities()->checkFields19(kCharacterCath, kCarRedSleeping, kPosition_7850) || getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200)) {
				setCallback(2);
				setup_enterCompartment();
				break;
			}

			if (getInventory()->hasItem(kItemFirebird)) {
				getActionOld()->playAnimation(kEventTatianaCompartmentStealEgg);
				getInventory()->removeItem(kItemFirebird);
				getInventory()->get(kItemFirebird)->location = kObjectLocation2;
			} else {
				getActionOld()->playAnimation(kEventTatianaCompartment);
			}

			getScenes()->loadSceneFromObject(kObjectCompartmentB);
			setup_afterConcert();
			break;

		case 2:
			setup_afterConcert();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Tatiana, afterConcert)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getInventory()->get(kItemFirebird)->location != kObjectLocation1 && getInventory()->get(kItemFirebird)->location != kObjectLocation2) {
			if(!params->param3)
				params->param3 = (uint)getState()->time + 900;

			if (params->param4 != kTimeInvalid && params->param3 < getState()->time) {
				if (Entity::updateParameterTime(kTime2227500, !getEntities()->isPlayerInCar(kCarRedSleeping), params->param4, 450)) {
					getProgress().field_5C = 1;
					if (getEntities()->isInsideCompartment(kCharacterAnna, kCarRedSleeping, kPosition_4070)) {
						setup_cryAnna();
						break;
					}
				}
			}
		}

		if (params->param1) {
			if (!Entity::updateParameter(params->param5, getState()->timeTicks, 75))
				break;

			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			params->param1 = 0;
			params->param2 = 1;
		}

		params->param5 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (params->param1) {
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

			if (savepoint.param.intValue == 49) {
				setCallback(4);
				setup_playSound(getSound()->justAMinuteCath());
				break;
			}

			if (getInventory()->hasItem(kItemPassengerList)) {
				setCallback(5);
				setup_playSound(rnd(2) ? "CAT1512" : getSound()->wrongDoorCath());
				break;
			}

			setCallback(6);
			setup_playSound(getSound()->wrongDoorCath());
			break;
		}

		if (savepoint.param.intValue == 49) {

			if (getInventory()->hasItem(kItemFirebird)) {
				getActionOld()->playAnimation(kEventTatianaCompartmentStealEgg);
				getInventory()->removeItem(kItemFirebird);
				getInventory()->get(kItemFirebird)->location = kObjectLocation2;
			} else {
				getActionOld()->playAnimation(kEventTatianaCompartment);
			}

			getScenes()->loadSceneFromObject(kObjectCompartmentB);
			break;
		}

		getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(savepoint.action == kCharacterActionKnock ? 1 : 2);
		setup_playSound(savepoint.action == kCharacterActionKnock ?  "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorHand);

		getData()->location = kLocationInsideCompartment;
		getEntities()->clearSequences(kCharacterTatiana);
		break;

	case kCharacterActionDrawScene:
		if (params->param1 || params->param2) {
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorNormal, kCursorHand);

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
			setCallback(3);
			setup_playSound(rnd(2) ? "TAT1133A" : "TAT1133B");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentB, kCharacterTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			getObjects()->update(kObject49, kCharacterTatiana, kObjectLocation1, kCursorTalk, kCursorNormal);
			params->param1 = 1;
			break;

		case 4:
		case 5:
		case 6:
			params->param1 = 0;
			params->param2 = 1;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(38, Tatiana, cryAnna)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param1, getState()->time, 450))
			break;

		getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentF, true);

		setCallback(4);
		setup_function42(kCarRedSleeping, kPosition_7500);
		break;

	case kCharacterActionDefault:
		getData()->clothes = kClothes3;

		setCallback(1);
		setup_enterExitCompartment("673Jb", kObjectCompartmentB);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject49, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_function42(kCarRedSleeping, kPosition_4070);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterTatiana, "673Gf");
			getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartmentF, true);

			setCallback(3);
			setup_playSound("Tat3164");
			break;

		case 3:
			getSavePoints()->push(kCharacterTatiana, kCharacterAnna, kCharacterAction236241630);
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 5:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterTatiana);

			setup_function39();
			break;

		case 6:
			getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentF, true);
			getEntities()->clearSequences(kCharacterTatiana);
			getData()->location = kLocationInsideCompartment;

			setCallback(7);
			setup_playSound("ANN3011");
			break;

		case 7:
			setCallback(8);
			setup_updateFromTime(900);
			break;

		case 8:
			setCallback(9);
			setup_enterExitCompartment("673Jf", kObjectCompartmentF);
			break;

		case 9:
			getData()->location = kLocationOutsideCompartment;

			setCallback(10);
			setup_function42(kCarRedSleeping, kPosition_7500);
			break;

		case 10:
			getSavePoints()->push(kCharacterTatiana, kCharacterAnna, kCharacterAction236517970);

			setCallback(11);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 11:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterTatiana);

			setup_function39();
			break;
		}
		break;

	case kCharacterAction100906246:
		setCallback(6);
		setup_enterExitCompartment("673Df", kObjectCompartmentF);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(39, Tatiana, function39)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1 && getEntities()->isDistanceBetweenEntities(kCharacterTatiana, kCharacterCath, 1000)) {
			params->param1 = 1;
			getSound()->playSound(kCharacterTatiana, "Tat3164");	// Tatiana weeping
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(40, Tatiana, function40)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isInsideTrainCar(kCharacterCath, kCarKronos)
		 || getData()->car != getEntityData(kCharacterCath)->car
		 || getEntities()->updateEntity(kCharacterTatiana, kCarKronos, kPosition_9270))
			callbackAction();
		break;

	case kCharacterActionExcuseMe:
		if (HELPERgetEvent(kEventTatianaAskMatchSpeakRussian) || HELPERgetEvent(kEventTatianaAskMatch) || HELPERgetEvent(kEventVassiliSeizure))
			getSound()->playSound(kCharacterCath, rnd(2) ? "CAT1010A" : "CAT1010");
		else
			getSound()->excuseMeCath();
		break;

	case kCharacterActionDefault:
		if (getEntities()->updateEntity(kCharacterTatiana, kCarKronos, kPosition_9270))
			callbackAction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(41, Tatiana, trapCath)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1)
			break;

		if (getEntities()->checkFields19(kCharacterCath, kCarRedSleeping, kPosition_7850)
		 && !HELPERgetEvent(kEventVassiliCompartmentStealEgg)
		 && (getState()->time <= kTime2133000 || getProgress().field_40)) {
			if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_7500)) {

				getSavePoints()->push(kCharacterTatiana, kCharacterCond2, kCharacterAction235061888);
				getEntities()->clearSequences(kCharacterTatiana);
				getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentB, true);
				getData()->location = kLocationInsideCompartment;

				if (getInventory()->hasItem(kItemFirebird)) {
					getActionOld()->playAnimation(kEventTatianaCompartmentStealEgg);
					getInventory()->removeItem(kItemFirebird);
					getInventory()->get(kItemFirebird)->location = kObjectLocation2;
				} else {
					getActionOld()->playAnimation(kEventTatianaCompartment);
				}

				getScenes()->loadSceneFromObject(kObjectCompartmentB);

				setCallback(4);
				setup_updateFromTime(150);
			}
		} else {
			getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartmentB, true);

			if (getState()->time < kTime2133000 || getProgress().field_40) {
				setCallback(3);
				setup_function40();
				break;
			}

			getEntities()->clearSequences(kCharacterTatiana);
			callbackAction();
		}
		break;

	case kCharacterActionDefault:
		getData()->car = kCarRedSleeping;
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationOutsideCompartment;

		RESET_ENTITY_STATE(kCharacterCond2, Coudert, setup_function51);

		getEntities()->drawSequenceLeft(kCharacterTatiana, "673Fb");
		getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartmentB, true);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_playSound("Tat3161B");
			break;

		case 2:
			getSavePoints()->push(kCharacterTatiana, kCharacterCond2, kCharacterAction168316032);
			params->param1 = 1;
			break;

		case 3:
		case 6:
			getEntities()->clearSequences(kCharacterTatiana);

			callbackAction();
			break;

		case 4:
			setCallback(5);
			setup_exitCompartment();
			break;

		case 5:
			setCallback(6);
			setup_function40();
			break;
		}
		break;

	case kCharacterAction154071333:
		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);

		setCallback(1);
		setup_savegame(kSavegameTypeTime, kTimeNone);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(42, Tatiana, function42, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath || savepoint.action == kCharacterActionExcuseMe) {
		getSound()->playSound(kCharacterCath, "Tat3124", getSound()->getSoundFlag(kCharacterTatiana));
		return;
	}

	Entity::updateEntity(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(43, Tatiana, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inCompartment4();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterTatiana);

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);

		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothes2;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 1) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(44, Tatiana, inCompartment4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_handleCompartment(kTime2362500);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			setup_meetAlexei();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(45, Tatiana, meetAlexei)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("673Bb", kObjectCompartmentB);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_540);
			break;

		case 2:
			if (getEntities()->isInGreenCarEntrance(kCharacterCath)) {
				getSound()->excuseMe(kCharacterTatiana);

				if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
					getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
			}

			getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction123712592);
			setup_withAlexei();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(46, Tatiana, withAlexei)
	// Expose parameters as IIIIIS and ignore the default exposed parameters
	EntityData::EntityParametersI5S  *parameters  = (EntityData::EntityParametersI5S*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!parameters->param2 && !parameters->param3) {
			parameters->param1 -= getState()->timeDelta;

			if (parameters->param1 < getState()->timeDelta) {
				getSound()->playSound(kCharacterTatiana, parameters->seq);

				if (getEntities()->isDistanceBetweenEntities(kCharacterTatiana, kCharacterCath, 2000)) {
					if (parameters->param4 == 4)
						getProgress().field_8C = 1;
					else if (parameters->param4 == 7)
						getProgress().field_88 = 1;
				}

				parameters->param2 = 1;
			}
		}

		if (CURRENT_PARAM(1, 1) == kTimeInvalid || getState()->time <= kTime2394000)
			break;

		if (getState()->time > kTime2398500) {
			CURRENT_PARAM(1, 1) = kTimeInvalid;
		} else {
			if (getEntities()->isInGreenCarEntrance(kCharacterCath) || !CURRENT_PARAM(1, 1))
				CURRENT_PARAM(1, 1) = (uint)getState()->time;

			if (CURRENT_PARAM(1, 1) >= getState()->time)
				break;

			CURRENT_PARAM(1, 1) = kTimeInvalid;
		}

		if (getEntities()->isInGreenCarEntrance(kCharacterCath)) {
			getSound()->excuseMe(kCharacterTatiana);

			if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62))
				getScenes()->loadSceneFromPosition(kCarGreenSleeping, 72);
		}

		getSavePoints()->push(kCharacterTatiana, kCharacterAlexei, kCharacterAction123536024);

		setup_thinking();
		break;

	case kCharacterActionEndSound:
		parameters->param2 = 0;
		++parameters->param4;

		switch(parameters->param4) {
		default:
			parameters->param1 = 162000;
			break;

		case 1:
			parameters->param1 = 900;
			Common::strcpy_s(parameters->seq, "Tat4165F");
			break;

		case 2:
			parameters->param1 = 900;
			Common::strcpy_s(parameters->seq, "Tat4165B");
			break;

		case 3:
			parameters->param1 = 1800;
			Common::strcpy_s(parameters->seq, "Tat4165G");
			break;

		case 4:
			parameters->param1 = 900;
			Common::strcpy_s(parameters->seq, "Tat4165H");
			break;

		case 5:
			parameters->param1 = 2700;
			Common::strcpy_s(parameters->seq, "Tat4165C");
			break;

		case 6:
			parameters->param1 = 900;
			Common::strcpy_s(parameters->seq, "Tat4165D");
			break;

		case 7:
			parameters->param1 = 900;
			Common::strcpy_s(parameters->seq, "Tat4165E");
			break;
		}
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "306E");
		parameters->param1 = 450;
		Common::strcpy_s(parameters->seq, "Tat4165A");
		break;

	case kCharacterActionDrawScene:
		if (getEntities()->isInGreenCarEntrance(kCharacterCath)) {
			parameters->param3 = 1;

			if (parameters->param2) {
				getSoundQueue()->stop(kCharacterTatiana);
				getSavePoints()->call(kCharacterTatiana, kCharacterTatiana, kCharacterActionEndSound);
			}
		} else {
			parameters->param3 = 0;
			parameters->param5 = 0;
		}

		if (getEntities()->isPlayerPosition(kCarGreenSleeping, 62) && !parameters->param5) {
			setCallback(1);
			setup_draw("306D");
		}
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kCharacterTatiana, "306E");
			parameters->param5 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(47, Tatiana, thinking)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_updateEntity(kCarRedSleeping, kPosition_7500);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 2:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterTatiana);

			setCallback(3);
			setup_handleCompartment(kTime2407500);
			break;

		case 3:
		case 4:
			if (ENTITY_PARAM(0, 1) && getObjects()->get(kObjectCompartment1).model == kObjectModel1) {
				setup_seekCath();
			} else {
				setCallback(4);
				setup_handleCompartment(900);
			}
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(48, Tatiana, seekCath)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param1) {
			if (!HELPERgetEvent(kEventTatianaTylerCompartment) && getEntities()->isInsideCompartment(kCharacterCath, kCarGreenSleeping, kPosition_8200)) {
				params->param1 = 1;
				getProgress().field_E4 = 1;
				getObjects()->update(kObjectCompartment1, kCharacterTatiana, getObjects()->get(kObjectCompartment1).status, kCursorNormal, kCursorHand);
			}

			if (!params->param1)
				goto label_end;
		}

		if (!getEntities()->checkFields19(kCharacterCath, kCarGreenSleeping, kPosition_7850)) {
			getObjects()->update(kObjectCompartment1, kCharacterCath, getObjects()->get(kObjectCompartment1).status, kCursorHandKnock, kCursorHand);
			params->param1 = 0;
		}

		if (!params->param1 || getSoundQueue()->isBuffered(kCharacterTatiana))
			goto label_end;

		if (!Entity::updateParameter(params->param2, getState()->timeTicks, 5 * (3 * rnd(5) + 30)))
			goto label_end;

		getSound()->playSound(kCharacterTatiana, "LIB012", kVolumeFull);
		params->param2 = 0;

label_end:
		if (HELPERgetEvent(kEventTatianaTylerCompartment) || getState()->time > kTime2475000) {
			if (params->param1)
				getObjects()->update(kObjectCompartment1, kCharacterCath, getObjects()->get(kObjectCompartment1).status, kCursorHandKnock, kCursorHand);

			getProgress().field_E4 = 0;
			getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartment2, true);

			setCallback(3);
			setup_updateEntity(kCarRedSleeping, kPosition_7500);
		}
		break;

	case kCharacterActionOpenDoor:
		params->param1 = 0;

		getObjects()->update(kObjectCompartment1, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		setCallback(5);
		setup_savegame(kSavegameTypeEvent, kEventTatianaTylerCompartment);
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("673Bb", kObjectCompartmentB);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			getData()->location = kLocationOutsideCompartment;

			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_7500);
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterTatiana, "673Fb");
			getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartment2, true);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment2("673Db", kObjectCompartmentB);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterTatiana);

			setup_function49();
			break;

		case 5:
			getObjects()->update(kObjectOutsideTylerCompartment, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getActionOld()->playAnimation(kEventTatianaTylerCompartment);
			getSound()->playSound(kCharacterCath, "LIB015");
			getScenes()->loadScene(kScene41);
			break;

		case 6:
			setCallback(7);
			setup_updateEntity(kCarGreenSleeping, kPosition_7500);
			break;

		case 7:
			getEntities()->drawSequenceLeft(kCharacterTatiana, "673Fb");
			getEntities()->enterCompartment(kCharacterTatiana, kObjectCompartment2, true);
			break;
		}
		break;

	case kCharacterAction238790488:
		params->param1 = 0;

		getObjects()->update(kObjectCompartment1, kCharacterCath, getObjects()->get(kObjectCompartment1).status, kCursorHandKnock, kCursorHand);
		getEntities()->exitCompartment(kCharacterTatiana, kObjectCompartment2, true);
		getEntities()->clearSequences(kCharacterTatiana);

		getData()->car = kCarGreenSleeping;
		getData()->entityPosition = kPosition_9460;

		setCallback(6);
		setup_updateFromTime(1800);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(49, Tatiana, function49)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterAction169360385:
		setup_alexeiDead();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(50, Tatiana, alexeiDead)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getState()->time > kTime2520000 && !params->param1) {
			params->param1 = 1;
			setup_function51();
		}
		break;

	case kCharacterActionEndSound:
		getSound()->playSound(kCharacterTatiana, "Tat4166");
		break;

	case kCharacterActionKnock:
		if (!getSoundQueue()->isBuffered("LIB012", true))
			getSound()->playSound(kCharacterCath, "LIB012");
		break;

	case kCharacterActionOpenDoor:
		getSound()->playSound(kCharacterCath, "LIB014");

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventVassiliDeadAlexei);
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentB, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject49, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject48, kCharacterTatiana, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartmentA, kCharacterTatiana, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		if (!getSoundQueue()->isBuffered(kCharacterTatiana))
			getSound()->playSound(kCharacterTatiana, "Tat4166");
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			if (getSoundQueue()->isBuffered("MUS013"))
				getSoundQueue()->fade("MUS013");

			getActionOld()->playAnimation(kEventVassiliDeadAlexei);
			getSavePoints()->push(kCharacterTatiana, kCharacterAbbot, kCharacterAction104060776);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 38);

			setup_function51();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(51, Tatiana, function51)
	if (savepoint.action == kCharacterActionDefault) {
		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject48, kCharacterCath, kObjectLocation1, kCursorNormal, kCursorNormal);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(52, Tatiana, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterTatiana);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(53, Tatiana, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5)
		setup_autistic();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(54, Tatiana, autistic)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!params->param2) {
			switch (params->param1) {
			default:
				break;

			case 0:
				getSound()->playSound(kCharacterTatiana, "Tat5167A");
				params->param2 = 1;
				break;

			case 1:
				getSound()->playSound(kCharacterTatiana, "Tat5167B");
				params->param2 = 1;
				break;

			case 2:
				getSound()->playSound(kCharacterTatiana, "Tat5167C");
				params->param2 = 1;
				break;

			case 3:
				getSound()->playSound(kCharacterTatiana, "Tat5167D");
				params->param2 = 1;
				break;
			}
		}

		if (params->param1 > 3) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, 225))
				break;

			params->param1 = 0;
			params->param3 = 0;
		}
		break;

	case kCharacterAction1:
		getData()->inventoryItem = kItemNone;

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventTatianaVassiliTalk);
		break;

	case kCharacterActionEndSound:
		++params->param1;
		params->param2 = 0;
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "033A");
		getData()->inventoryItem = kItemInvalid;
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			if (getSoundQueue()->isBuffered("MUS050"))
				getSoundQueue()->fade("MUS050");

			if (getSoundQueue()->isBuffered(kCharacterTatiana))
				getSoundQueue()->fade(kCharacterTatiana);

			getActionOld()->playAnimation(isNightOld() ? kEventTatianaVassiliTalkNight : kEventTatianaVassiliTalk);
			getScenes()->processScene();

			params->param1 = 4;
			params->param2 = 0;
			params->param3 = 0;
		}
		break;

	case kCharacterAction203078272:
		getEntities()->drawSequenceLeft(kCharacterTatiana, "033E");
		break;

	case kCharacterAction236060709:
		getData()->inventoryItem = kItemNone;
		setup_function55();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(55, Tatiana, function55)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterTatiana);
		// fall through

	case kCharacterActionDrawScene:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 72))
			getScenes()->loadSceneFromPosition(kCarRestaurant, 86);
		break;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
