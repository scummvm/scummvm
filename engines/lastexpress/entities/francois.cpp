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

#include "lastexpress/entities/francois.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Francois::Francois(LastExpressEngine *engine) : Entity(engine, kCharacterFrancois) {
	ADD_CALLBACK_FUNCTION(Francois, reset);
	ADD_CALLBACK_FUNCTION_I(Francois, updateFromTime);
	ADD_CALLBACK_FUNCTION_S(Francois, draw);
	ADD_CALLBACK_FUNCTION_SI(Francois, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_SI(Francois, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION_S(Francois, playSound);
	ADD_CALLBACK_FUNCTION_II(Francois, savegame);
	ADD_CALLBACK_FUNCTION_II(Francois, doWalk);
	ADD_CALLBACK_FUNCTION(Francois, exitCompartment);
	ADD_CALLBACK_FUNCTION(Francois, enterCompartment);
	ADD_CALLBACK_FUNCTION_I(Francois, rampage);
	ADD_CALLBACK_FUNCTION(Francois, takeWalk);
	ADD_CALLBACK_FUNCTION(Francois, haremVisit);
	ADD_CALLBACK_FUNCTION_TYPE(Francois, chaseBeetle, EntityParametersIISS);
	ADD_CALLBACK_FUNCTION(Francois, findCath);
	ADD_CALLBACK_FUNCTION(Francois, letsGo);
	ADD_CALLBACK_FUNCTION(Francois, chapter1);
	ADD_CALLBACK_FUNCTION(Francois, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Francois, inCompartment);
	ADD_CALLBACK_FUNCTION(Francois, function20);
	ADD_CALLBACK_FUNCTION(Francois, chapter2);
	ADD_CALLBACK_FUNCTION(Francois, atBreakfast);
	ADD_CALLBACK_FUNCTION(Francois, withMama);
	ADD_CALLBACK_FUNCTION(Francois, chapter3);
	ADD_CALLBACK_FUNCTION(Francois, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Francois, chapter4);
	ADD_CALLBACK_FUNCTION(Francois, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Francois, chapter5);
	ADD_CALLBACK_FUNCTION(Francois, chapter5Handler);
	ADD_CALLBACK_FUNCTION(Francois, function30);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Francois, reset)
	Entity::reset(savepoint, kClothes1, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(2, Francois, updateFromTime, uint32)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Francois, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(4, Francois, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(5, Francois, enterExitCompartment2, ObjectIndex)
	Entity::enterExitCompartment(savepoint, kPosition_5790, kPosition_6130, kCarRedSleeping, kObjectCompartmentD, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Francois, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(7, Francois, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Francois, doWalk, CarIndex, EntityPosition)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2)) {
			getData()->inventoryItem = kItemNone;
			callbackAction();
		} else {
			if (!getEntities()->isDistanceBetweenEntities(kCharacterFrancois, kCharacterCath, 2000)
			 || !getInventory()->hasItem(kItemFirebird)
			 || HELPERgetEvent(kEventFrancoisShowEgg)
			 || HELPERgetEvent(kEventFrancoisShowEggD)
			 || HELPERgetEvent(kEventFrancoisShowEggNight)
			 || HELPERgetEvent(kEventFrancoisShowEggNightD)) {
				if (getEntities()->isDistanceBetweenEntities(kCharacterFrancois, kCharacterCath, 2000)
				 && getInventory()->get(kItemBeetle)->location == kObjectLocation1
				 && !HELPERgetEvent(kEventFrancoisShowBeetle)
				 && !HELPERgetEvent(kEventFrancoisShowBeetleD))
					getData()->inventoryItem = kItemMatchBox;
				else
					getData()->inventoryItem = kItemNone;
			} else {
				getData()->inventoryItem = kItemFirebird;
			}

			if (ENTITY_PARAM(0, 1)
			 && getEntities()->isDistanceBetweenEntities(kCharacterFrancois, kCharacterCath, 1000)
			 && !getEntities()->isInsideCompartments(kCharacterCath)
			 && !getEntities()->checkFields10(kCharacterCath)) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventFrancoisTradeWhistle);
			}
		}
		break;

	case kCharacterAction1:
		switch (savepoint.param.intValue) {
		default:
			break;

		case kItemMatchBox:
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventFrancoisShowBeetle);
			break;

		case kItemFirebird:
			if (isNightOld())
				getActionOld()->playAnimation(getData()->entityPosition < getEntityData(kCharacterCath)->entityPosition ? kEventFrancoisShowEggNightD : kEventFrancoisShowEggNight);
			else
				getActionOld()->playAnimation(getData()->entityPosition < getEntityData(kCharacterCath)->entityPosition ? kEventFrancoisShowEggD : kEventFrancoisShowEgg);

			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionUp ? -1 : 1))), getData()->direction == kDirectionUp);
			break;
		}
		break;

	case kCharacterActionExcuseMeCath:
	case kCharacterActionExcuseMe:
		getSound()->excuseMe(_entityIndex);
		break;

	case kCharacterActionDefault:
		if (getEntities()->updateEntity(_entityIndex, (CarIndex)params->param1, (EntityPosition)params->param2))
			callbackAction();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getActionOld()->playAnimation(getData()->entityPosition < getEntityData(kCharacterCath)->entityPosition ? kEventFrancoisTradeWhistleD : kEventFrancoisTradeWhistle);
			getInventory()->addItem(kItemWhistle);
			getInventory()->removeItem(kItemMatchBox);
			getInventory()->get(kItemBeetle)->location = kObjectLocation2;
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionUp ? -1 : 1))), getData()->direction == kDirectionUp);
			ENTITY_PARAM(0, 1) = 0;
			break;

		case 2:
			getActionOld()->playAnimation(getData()->entityPosition < getEntityData(kCharacterCath)->entityPosition ? kEventFrancoisShowBeetleD : kEventFrancoisShowBeetle);
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + (750 * (getData()->direction == kDirectionUp ? -1 : 1))), getData()->direction == kDirectionUp);
			getData()->inventoryItem = kItemNone;
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Francois, exitCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		if (getObjects()->get(kObjectCompartmentD).status == kObjectLocation2) {
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction134289824);
			setCallback(1);
			setup_enterExitCompartment("605Cd", kObjectCompartmentD);
		} else {
			setCallback(2);
			setup_enterExitCompartment("605Ed", kObjectCompartmentD);
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			// fall through

		case 2:
			getData()->location = kLocationOutsideCompartment;
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Francois, enterCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		if (getObjects()->get(kObjectCompartmentD).status == kObjectLocation2) {
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
			setCallback(1);
			setup_enterExitCompartment2("605Bd", kObjectCompartmentD);
		} else {
			setCallback(2);
			setup_enterExitCompartment2("605Dd", kObjectCompartmentD);
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction102484312);
			// fall through

		case 2:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterFrancois);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_I(11, Francois, rampage, TimeValue)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!getSoundQueue()->isBuffered(kCharacterFrancois)) {

			if (Entity::updateParameter(CURRENT_PARAM(1, 1), getState()->timeTicks, params->param6)) {
				switch (rnd(7)) {
				default:
					break;

				case 0:
					getSound()->playSound(kCharacterFrancois, "Fra1002A");
					break;

				case 1:
					getSound()->playSound(kCharacterFrancois, "Fra1002B");
					break;

				case 2:
					getSound()->playSound(kCharacterFrancois, "Fra1002C");
					break;

				case 3:
					getSound()->playSound(kCharacterFrancois, "Fra1002D");
					break;

				case 4:
					getSound()->playSound(kCharacterFrancois, "Fra1002E");
					break;

				case 5:
				case 6:
					getSound()->playSound(kCharacterFrancois, "Fra1002F");
					break;
				}

				params->param6 = 15 * rnd(7);
				CURRENT_PARAM(1, 1) = 0;
			}
		}

		if (!getEntities()->hasValidFrame(kCharacterFrancois) || !getEntities()->isWalkingOppositeToPlayer(kCharacterFrancois))
			getData()->inventoryItem = kItemNone;

		if (getEntities()->updateEntity(kCharacterFrancois, (CarIndex)params->param2, (EntityPosition)params->param3)) {
			params->param5 = 0;

			if (params->param3 == kPosition_540) {
				params->param2 = (getProgress().chapter == kChapter1) ? kCarRedSleeping : kCarGreenSleeping;
				params->param3 = kPosition_9460;
			} else {
				params->param2 = kCarGreenSleeping;
				params->param3 = kPosition_540;
				params->param7 = 0;
				params->param8 = 0;

				getSavePoints()->push(kCharacterFrancois, kCharacterCond2, kCharacterAction225932896);
				getSavePoints()->push(kCharacterFrancois, kCharacterCond1, kCharacterAction225932896);
			}
		}

		if (getEntities()->checkDistanceFromPosition(kCharacterFrancois, kPosition_2000, 500) && getData()->direction == kDirectionDown) {

			if (getEntities()->isInsideTrainCar(kCharacterFrancois, kCarRedSleeping) && params->param8) {
				setCallback(2);
				setup_draw("605A");
				break;
			}

			if (getEntities()->isInsideTrainCar(kCharacterFrancois, kCarGreenSleeping) && params->param7) {
				setCallback(3);
				setup_draw("605A");
				break;
			}
		}

label_callback:
		if (getProgress().chapter == kChapter1) {

			if (getEntities()->isInsideTrainCar(kCharacterFrancois, kCarRedSleeping)
			 && (getEntities()->hasValidFrame(kCharacterFrancois) || params->param1 < getState()->time || params->param4)
			 && !params->param5
			 && getData()->entityPosition < getEntityData(kCharacterMadame)->entityPosition) {

				if (getData()->direction == kDirectionDown) {
					getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction202221040);
					params->param4 = 1;
					params->param5 = 1;
				} else if (params->param4 && getEntities()->isDistanceBetweenEntities(kCharacterFrancois, kCharacterMadame, 1000)) {
					getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction168986720);
					params->param5 = 1;
				}
			}
		} else if (params->param1 < getState()->time) {
			getData()->clothes = kClothesDefault;
			getData()->field_4A3 = 30;
			getData()->inventoryItem = kItemNone;

			if (getSoundQueue()->isBuffered(kCharacterFrancois))
				getSoundQueue()->fade(kCharacterFrancois);

			setCallback(4);
			setup_doWalk(kCarRedSleeping, kPosition_5790);
		}
		break;

	case kCharacterAction1:
		getData()->inventoryItem = kItemNone;

		if (getSoundQueue()->isBuffered(kCharacterFrancois))
			getSoundQueue()->fade(kCharacterFrancois);

		setCallback(6);
		setup_savegame(kSavegameTypeEvent, kEventFrancoisWhistle);
		break;

	case kCharacterActionExcuseMeCath:
		if (getProgress().jacket == kJacketGreen
		 && !HELPERgetEvent(kEventFrancoisWhistle)
		 && !HELPERgetEvent(kEventFrancoisWhistleD)
		 && !HELPERgetEvent(kEventFrancoisWhistleNight)
		 && !HELPERgetEvent(kEventFrancoisWhistleNightD))
			getData()->inventoryItem = kItemInvalid;
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_exitCompartment();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->clothes = kClothes1;
			getData()->field_4A3 = 100;
			getData()->inventoryItem = kItemNone;

			params->param2 = kCarGreenSleeping;
			params->param3 = kPosition_540;

			getEntities()->updateEntity(kCharacterFrancois, kCarGreenSleeping, kPosition_540);

			params->param6 = 15 * rnd(7);
			break;

		case 2:
			getSavePoints()->push(kCharacterFrancois, kCharacterCond2, kCharacterAction168253822);
			// fall through

		case 3:
			params->param2 = kCarRedSleeping;
			params->param3 = kPosition_9460;
			params->param5 = 0;

			getData()->entityPosition = kPosition_2088;

			getEntities()->updateEntity(kCharacterFrancois, kCarRedSleeping, kPosition_9460);
			goto label_callback;

		case 4:
			setCallback(5);
			setup_enterCompartment();
			break;

		case 5:
			callbackAction();
			break;

		case 6:
			if (getProgress().jacket == kJacketGreen) {
				if (isNightOld())
					getActionOld()->playAnimation(getData()->entityPosition <= getEntityData(kCharacterCath)->entityPosition ? kEventFrancoisWhistleNightD : kEventFrancoisWhistleNight);
				else
					getActionOld()->playAnimation(getData()->entityPosition <= getEntityData(kCharacterCath)->entityPosition ? kEventFrancoisWhistleD : kEventFrancoisWhistle);
			}
			getEntities()->loadSceneFromEntityPosition(getData()->car, (EntityPosition)(getData()->entityPosition + 750 * (getData()->direction == kDirectionUp ? -1 : 1)), getData()->direction == kDirectionUp);
			break;
		}
		break;

	case kCharacterAction102752636:
		getEntities()->clearSequences(kCharacterFrancois);
		getData()->location = kLocationInsideCompartment;
		getData()->entityPosition = kPosition_5790;
		getData()->clothes = kClothesDefault;
		getData()->field_4A3 = 30;
		getData()->inventoryItem = kItemNone;

		callbackAction();
		break;

	case kCharacterAction205346192:
		if (savepoint.entity2 == kCharacterCond2)
			params->param8 = 1;
		else if (savepoint.entity2 == kCharacterCond1)
			params->param7 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Francois, takeWalk)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_exitCompartment();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_doWalk(kCarRedSleeping, kPosition_9460);
			break;

		case 2:
			setCallback(3);
			setup_updateFromTime(675);
			break;

		case 3:
			setCallback(4);
			setup_doWalk(kCarRedSleeping, kPosition_540);
			break;

		case 4:
			setCallback(5);
			setup_updateFromTime(675);
			break;

		case 5:
			setCallback(6);
			setup_doWalk(kCarRedSleeping, kPosition_5790);
			break;

		case 6:
			setCallback(7);
			setup_enterCompartment();
			break;

		case 7:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Francois, haremVisit)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_exitCompartment();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_doWalk(kCarRedSleeping, kPosition_540);
			break;

		case 2:
			setCallback(3);
			setup_doWalk(kCarGreenSleeping, kPosition_4070);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("605Df", kObjectCompartment6);
			break;

		case 4:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterFrancois);

			setCallback(5);
			setup_playSound("Har2010");
			break;

		case 5:
			getSavePoints()->push(kCharacterFrancois, kCharacterAlouan, kCharacterAction189489753);
			break;

		case 6:
			getData()->location = kLocationOutsideCompartment;

			setCallback(7);
			setup_doWalk(kCarRedSleeping, kPosition_4840);
			break;

		case 7:
			if (getInventory()->hasItem(kItemWhistle) || getInventory()->get(kItemWhistle)->location == kObjectLocation3) {
				setCallback(10);
				setup_doWalk(kCarRedSleeping, kPosition_5790);
				break;
			}

			getEntities()->drawSequenceLeft(kCharacterFrancois, "605He");
			getEntities()->enterCompartment(kCharacterFrancois, kObjectCompartmentE, true);
			setCallback(8);
			setup_playSound(rnd(2) ? "Fra2005B" : "Fra2005C");
			break;

		case 8:
			setCallback(9);
			setup_updateFromTime(450);
			break;

		case 9:
			getEntities()->exitCompartment(kCharacterFrancois, kObjectCompartmentE, true);

			setCallback(10);
			setup_doWalk(kCarRedSleeping, kPosition_5790);
			break;

		case 10:
			setCallback(11);
			setup_enterCompartment();
			break;

		case 11:
			callbackAction();
			break;
		}
		break;

	case kCharacterAction190219584:
		setCallback(6);
		setup_enterExitCompartment("605Ef", kObjectCompartment6);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_IIS(14, Francois, chaseBeetle, ObjectIndex, EntityPosition)
	// Expose parameters as IISS and ignore the default exposed parameters
	EntityData::EntityParametersIISS *parameters = (EntityData::EntityParametersIISS*)_data->getCurrentParameters();

	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		Common::strcpy_s(parameters->seq2, "605H");
		Common::strcat_s(parameters->seq2, parameters->seq1);

		setCallback(1);
		setup_exitCompartment();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_doWalk(kCarRedSleeping, (EntityPosition)parameters->param2);
			break;

		case 2:
			if (getInventory()->get(kItemBeetle)->location == kObjectLocation3) {
				getEntities()->drawSequenceLeft(kCharacterFrancois, parameters->seq2);
				getEntities()->enterCompartment(kCharacterFrancois, (ObjectIndex)parameters->param1, true);

				setCallback(3);
				setup_playSound("Fra2005A");
			} else {
				if (parameters->param2 >= kPosition_5790) {
					setCallback(10);
					setup_doWalk(kCarRedSleeping, kPosition_9460);
				} else {
					setCallback(9);
					setup_doWalk(kCarRedSleeping, kPosition_540);
				}
			}
			break;

		case 3:
		case 5:
			setCallback(getCallback() + 1);
			setup_updateFromTime(rnd(450));
			break;

		case 4:
		case 6:
			setCallback(getCallback() + 1);
			setup_playSound(rnd(2) ? "Fra2005B" : "Fra2005C");
			break;

		case 7:
			setCallback(8);
			setup_updateFromTime(rnd(150));
			break;

		case 8:
			getEntities()->exitCompartment(kCharacterFrancois, (ObjectIndex)parameters->param1, true);
			// fall through

		case 9:
			setCallback(10);
			setup_doWalk(kCarRedSleeping, kPosition_9460);
			break;

		case 10:
			setCallback(11);
			setup_updateFromTime(900);
			break;

		case 11:
			setCallback(12);
			setup_doWalk(kCarRedSleeping, kPosition_5790);
			break;

		case 12:
			setCallback(13);
			setup_enterCompartment();
			break;

		case 13:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Francois, findCath)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_exitCompartment();
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getData()->entityPosition >= getEntityData(kCharacterCath)->entityPosition) {
				setCallback(3);
				setup_doWalk(kCarRedSleeping, kPosition_540);
			} else {
				setCallback(2);
				setup_doWalk(kCarRedSleeping, kPosition_9460);
			}
			break;

		case 2:
		case 3:
			setCallback(4);
			setup_updateFromTime(450);
			break;

		case 4:
			setCallback(5);
			setup_doWalk(kCarRedSleeping, kPosition_5790);
			break;

		case 5:
			setCallback(6);
			setup_enterCompartment();
			break;

		case 6:
			setCallback(7);
			setup_updateFromTime(900);
			break;

		case 7:
			if (!getEntities()->isInsideCompartment(kCharacterMadame, kCarRedSleeping, kPosition_5790)) {
				callbackAction();
				break;
			}

			setCallback(8);
			setup_playSound("Fra2012");
			break;

		case 8:
			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Francois, letsGo)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		getData()->entityPosition = getEntityData(kCharacterMonsieur)->entityPosition;
		getData()->location = getEntityData(kCharacterMonsieur)->location;
		getData()->car = getEntityData(kCharacterMonsieur)->car;
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorNormal);

		setCallback(1);
		setup_enterExitCompartment("605Cd", kObjectCompartmentD);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getData()->entityPosition = kPosition_5890;

			getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction101107728);

			setCallback(2);
			setup_doWalk(kCarRestaurant, kPosition_850);
			break;

		case 2:
			getEntities()->clearSequences(kCharacterFrancois);
			getSavePoints()->push(kCharacterFrancois, kCharacterMonsieur, kCharacterAction237889408);
			break;

		case 3:
			setCallback(4);
			setup_enterExitCompartment("605Id", kObjectCompartmentD);
			break;

		case 4:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction100957716);

			getData()->entityPosition = kPosition_5790;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterFrancois);

			callbackAction();
			break;
		}
		break;

	case kCharacterAction100901266:
		setCallback(3);
		setup_doWalk(kCarRedSleeping, kPosition_5790);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Francois, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Francois, setup_chapter1Handler));
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Francois, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		timeCheckCallback(kTimeParisEpernay, params->param1, 1, kTime1093500);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			setup_inCompartment();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Francois, inCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheckCallback(kTime1161000, params->param1, 2, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk));
		break;

	case kCharacterAction101107728:
		setCallback(1);
		setup_letsGo();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Francois, function20)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kCharacterFrancois);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Francois, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_atBreakfast();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterFrancois);

		getData()->entityPosition = kPosition_4689;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Francois, atBreakfast)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("605Id", kObjectCompartmentD);
			break;

		case 2:
			getObjects()->update(kObjectCompartmentD, kCharacterCath, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
			getSavePoints()->push(kCharacterFrancois, kCharacterMadame, kCharacterAction100957716);
			getData()->entityPosition = kPosition_5790;
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterFrancois);
			setup_withMama();
			break;
		}
		break;

	case kCharacterAction100901266:
		setCallback(1);
		setup_doWalk(kCarRedSleeping, kPosition_5790);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Francois, withMama)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (HELPERgetEvent(kEventFrancoisShowBeetle) || HELPERgetEvent(kEventFrancoisShowBeetleD))
			if (!HELPERgetEvent(kEventFrancoisTradeWhistle) && !HELPERgetEvent(kEventFrancoisTradeWhistleD))
				ENTITY_PARAM(0, 1) = 1;

		if (ENTITY_PARAM(0, 1) && getEntities()->isPlayerInCar(kCarRedSleeping)) {
			setCallback(1);
			setup_findCath();
			break;
		}

label_callback_1:
		if (Entity::timeCheckCallback(kTime1764000, params->param1, 2, "Fra2011", WRAP_SETUP_FUNCTION_S(Francois, setup_playSound)))
			break;

label_callback_2:
		if (Entity::timeCheckCallback(kTime1800000, params->param2, 3, WRAP_SETUP_FUNCTION(Francois, setup_haremVisit)))
			break;

label_callback_3:
		if (!getInventory()->hasItem(kItemWhistle) && getInventory()->get(kItemWhistle)->location != kObjectLocation3) {
			if (timeCheckCallback(kTime1768500, params->param3, 4, kTime1773000))
				break;

label_callback_4:
			if (timeCheckCallback(kTime1827000, params->param4, 5, kTime1831500))
				break;
		}

label_callback_5:
		if (getInventory()->get(kItemBeetle)->location != kObjectLocation3) {
			if (!getInventory()->hasItem(kItemWhistle) && getInventory()->get(kItemWhistle)->location != kObjectLocation3) {
				// BUG in the original game: condition is always false
				if (getState()->time < kTime1782000 && getState()->time > kTime1782000 && !params->param8) {
					params->param8 = 1;
					setCallback(9);
					setup_takeWalk();
					break;
				}
				if (getState()->time < kTime1813500 && getState()->time > kTime1813500 && !CURRENT_PARAM(1, 1)) {
					CURRENT_PARAM(1, 1) = 1;
					setCallback(10);
					setup_takeWalk();
					break;
				}
			}
			break;
		}

		if (params->param5 != kTimeInvalid) {
			if (Entity::updateParameterTime(kTimeEnd, !getEntities()->isDistanceBetweenEntities(kCharacterFrancois, kCharacterCath, 2000), params->param5, 75)) {
				setCallback(6);
				setup_playSound("Fra2010");
				break;
			}
		}

label_callback_6:
		if (timeCheckCallbackCompartment(kTime1782000, params->param6, 7, kObjectCompartmentF, kPosition_4070, "f"))
			break;

label_callback_7:
		timeCheckCallbackCompartment(kTime1813500, params->param7, 8, kObjectCompartmentC, kPosition_6470, "c");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			getProgress().field_9C = 1;
			goto label_callback_6;

		case 7:
			goto label_callback_7;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Francois, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter3Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterFrancois);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Francois, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (HELPERgetEvent(kEventFrancoisShowBeetle) || HELPERgetEvent(kEventFrancoisShowBeetleD))
			if (!HELPERgetEvent(kEventFrancoisTradeWhistle) && !HELPERgetEvent(kEventFrancoisTradeWhistleD))
				ENTITY_PARAM(0, 1) = 1;

		if (params->param2 && getEntities()->isInsideCompartment(kCharacterMadame, kCarRedSleeping, kPosition_5790) && !params->param1) {

			if (ENTITY_PARAM(0, 1) && getEntities()->isPlayerInCar(kCarRedSleeping)) {
				setCallback(2);
				setup_findCath();
				break;
			}

label_callback_2:
			if (Entity::timeCheckCallback(kTime2025000, params->param3, 3, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_3:
			if (Entity::timeCheckCallback(kTime2052000, params->param4, 4, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_4:
			if (Entity::timeCheckCallback(kTime2079000, params->param5, 5, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_5:
			if (Entity::timeCheckCallback(kTime2092500, params->param6, 6, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_6:
			if (Entity::timeCheckCallback(kTime2173500, params->param7, 7, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_7:
			if (Entity::timeCheckCallback(kTime2182500, params->param8, 8, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_8:
			if (Entity::timeCheckCallback(kTime2241000, CURRENT_PARAM(1, 1), 9, WRAP_SETUP_FUNCTION(Francois, setup_takeWalk)))
				break;

label_callback_9:
			if (!getInventory()->hasItem(kItemWhistle) && getInventory()->get(kItemWhistle)->location != kObjectLocation3) {
				if (timeCheckCallback(kTime2011500, CURRENT_PARAM(1, 2), 10, kTime2016000))
					break;

label_callback_10:
				if (timeCheckCallback(kTime2115000, CURRENT_PARAM(1, 3), 11, kTime2119500))
					break;
			}

label_callback_11:
			if (getInventory()->get(kItemBeetle)->location == kObjectLocation3) {
				if (CURRENT_PARAM(1, 4) != kTimeInvalid) {
					if (getState()->time <= kTimeEnd)
						if (!getEntities()->isDistanceBetweenEntities(kCharacterFrancois, kCharacterCath, 2000) || !CURRENT_PARAM(1, 4))
							CURRENT_PARAM(1, 4) = (uint)(getState()->time + 75);

					if (CURRENT_PARAM(1, 4) < getState()->time || getState()->time > kTimeEnd) {
						CURRENT_PARAM(1, 4) = kTimeInvalid;

						setCallback(12);
						setup_playSound("Fra2010");
						break;
					}
				}

label_callback_12:
				if (timeCheckCallbackCompartment(kTime2040300, CURRENT_PARAM(1, 5), 13, kObjectCompartmentE, kPosition_4840, "e"))
					break;

label_callback_13:
				if (timeCheckCallbackCompartment(kTime2146500, CURRENT_PARAM(1, 6), 14, kObjectCompartmentF, kPosition_4070, "f"))
					break;

label_callback_14:
				timeCheckCallbackCompartment(kTime2218500, CURRENT_PARAM(1, 7), 15, kObjectCompartmentB, kPosition_7500, "b");
			}
			// The original game has some code here similar to withMama marked as BUG:
			// if [kItemBeetle].location != kObjectLocation3 && !has(kItemWhistle) && [kItemWhistle].location != kObjectLocation3,
			// there are several always-false checks (time < N && time > N),
			// kTime2040300 with callback 16, kTime2119500 with callback 17, kTime2146500 with callback 18, kTime2218500 with callback 19,
			// with takeWalk as a payload.
			// No point in reproducing it here.
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param2 = 1;
			break;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;

		case 6:
			goto label_callback_6;

		case 7:
			goto label_callback_7;

		case 8:
			goto label_callback_8;

		case 9:
			goto label_callback_9;

		case 10:
			goto label_callback_10;

		case 11:
			goto label_callback_11;

		case 12:
			getProgress().field_9C = 1;
			goto label_callback_12;

		case 13:
			goto label_callback_13;

		case 14:
			goto label_callback_14;
		}
		break;

	case kCharacterAction101107728:
		setCallback(1);
		setup_letsGo();
		break;

	case kCharacterAction189872836:
		params->param1 = 1;
		break;
	case kCharacterAction190390860:
		params->param1 = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Francois, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter4Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterFrancois);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Francois, chapter4Handler)
	if (savepoint.action == kCharacterAction101107728) {
		setCallback(1);
		setup_letsGo();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Francois, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterFrancois);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Francois, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5) {
		if (!getInventory()->hasItem(kItemWhistle)
		  && getInventory()->get(kItemWhistle)->location != kObjectLocation3)
		  getInventory()->setLocationAndProcess(kItemWhistle, kObjectLocation1);

		setup_function30();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Francois, function30)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;

	case kCharacterAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(31, Francois)


//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////
bool Francois::timeCheckCallbackCompartment(TimeValue timeValue, uint &parameter, byte callback, ObjectIndex compartment, EntityPosition position, const char* sequenceSuffix) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		setCallback(callback);
		setup_chaseBeetle(compartment, position, sequenceSuffix);

		return true;
	}

	return false;
}

bool Francois::timeCheckCallback(TimeValue timeValue, uint &parameter, byte callback, TimeValue timeValue2) {
	if (getState()->time > timeValue && !parameter) {
		parameter = 1;
		setCallback(callback);
		setup_rampage(timeValue2);

		return true;
	}

	return false;
}


} // End of namespace LastExpress
