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

#include "lastexpress/entities/vassili.h"

#include "lastexpress/entities/anna.h"
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

Vassili::Vassili(LastExpressEngine *engine) : Entity(engine, kCharacterVassili) {
	ADD_CALLBACK_FUNCTION(Vassili, reset);
	ADD_CALLBACK_FUNCTION_S(Vassili, draw);
	ADD_CALLBACK_FUNCTION_II(Vassili, savegame);
	ADD_CALLBACK_FUNCTION(Vassili, chapter1);
	ADD_CALLBACK_FUNCTION(Vassili, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Vassili, inBed);
	ADD_CALLBACK_FUNCTION(Vassili, function7);
	ADD_CALLBACK_FUNCTION(Vassili, function8);
	ADD_CALLBACK_FUNCTION(Vassili, function9);
	ADD_CALLBACK_FUNCTION(Vassili, seizure);
	ADD_CALLBACK_FUNCTION(Vassili, drawInBed);
	ADD_CALLBACK_FUNCTION(Vassili, chapter2);
	ADD_CALLBACK_FUNCTION(Vassili, sleeping);
	ADD_CALLBACK_FUNCTION(Vassili, chapter3);
	ADD_CALLBACK_FUNCTION(Vassili, stealEgg);
	ADD_CALLBACK_FUNCTION(Vassili, chapter4);
	ADD_CALLBACK_FUNCTION(Vassili, chapter4Handler);
	ADD_CALLBACK_FUNCTION(Vassili, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Vassili, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Vassili, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(3, Vassili, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Vassili, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Vassili, setup_chapter1Handler));
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject40, kCharacterCath, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Vassili, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1) {
			getData()->entityPosition = getEntityData(kCharacterTatiana)->entityPosition;
			getData()->location = getEntityData(kCharacterTatiana)->location;
			getData()->car = getEntityData(kCharacterTatiana)->car;
		} else {
			if (!Entity::updateParameterCheck(params->param3, getState()->time, 450))
				break;

			if (!params->param2 && getObjects()->get(kObjectCompartmentA).model == kObjectModel1) {
				params->param2 = 1;
				getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
				getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
			}
			break;
		}
		break;

	case kCharacterActionDefault:
		params->param1 = 1;
		break;

	case kCharacterAction122732000:
		setup_inBed();
		break;

	case kCharacterAction168459827:
		params->param1 = 0;
		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Vassili, inBed)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				goto label_function7;

			setCallback(1);
			setup_draw("303B");
			break;
		}

		params->param3 = 0;

		if (params->param2)
			getEntities()->drawSequenceLeft(kCharacterVassili, "303A");

label_function7:
		if (params->param4 != kTimeInvalid && getState()->time > kTime1489500) {

			if (getState()->time <= kTime1503000) {

				if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200) || !params->param4) {

					params->param4 = (uint)getState()->time;
					if (!params->param4) {
						setup_function7();
						break;
					}
				}

				if (params->param4 >= getState()->time)
					break;
			}

			params->param4 = kTimeInvalid;
			setup_function7();
		}
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		params->param1 = 5 * (3 * rnd(25) + 15);

		getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kCharacterVassili, "303C");
			params->param1 = 5 * (3 * rnd(25) + 15);
			params->param2 = 1;

			// Shared part with kCharacterActionNone
			goto label_function7;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Vassili, function7)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1 != kTimeInvalid && getState()->time > kTime1503000) {

			 if (getState()->time <= kTime1512000) {
				 if (!getEntities()->isPlayerInCar(kCarRedSleeping) || !params->param1) {
					 params->param1 = (uint)getState()->time + 150;
					 if (!params->param1) {
						 setup_function8();
						 break;
					 }
				 }

				 if (params->param1 >= getState()->time)
					 break;
			 }

			 params->param1 = kTimeInvalid;
			 setup_function8();
		}
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->clearSequences(kCharacterVassili);
		if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200))
			getScenes()->loadSceneFromObject(kObjectCompartmentA);

		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterAction339669520:
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Vassili, function8)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionEndSound:
		setup_function9();
		break;

	case kCharacterActionDefault:
		if (!getEntities()->isInsideTrainCar(kCharacterCath, kCarRedSleeping)) {
			getSound()->playSound(kCharacterCath, "BUMP");
			getScenes()->loadSceneFromPosition(kCarRedSleeping, (getEntityData(kCharacterCath)->car <= kCarRedSleeping) ? 1 : 40);
		}

		getSavePoints()->push(kCharacterVassili, kCharacterAnna, kCharacterAction226031488);
		getSavePoints()->push(kCharacterVassili, kCharacterTrainM, kCharacterAction226031488);
		getSavePoints()->push(kCharacterVassili, kCharacterCond2, kCharacterAction226031488);
		getSound()->playSound(kCharacterVassili, "VAS1027", kVolumeFull);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Vassili, function9)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionEndSound:
		if (!getEntities()->isDistanceBetweenEntities(kCharacterVassili, kCharacterCath, 2500))
			getSound()->playSound(kCharacterCath, "BUMP");

		setup_seizure();
		break;

	case kCharacterActionDefault:
	case kCharacterActionDrawScene:
		if ((getObjects()->get(kObjectCompartmentA).status == kObjectLocation2 && getEntities()->isPlayerPosition(kCarRedSleeping, 17))
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 18)
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 37)
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 38)
		|| getEntities()->isPlayerPosition(kCarRedSleeping, 41)) {

			if (savepoint.action == kCharacterActionDrawScene)
				getSoundQueue()->fade(kCharacterVassili);

			setup_seizure();
		} else {
			if (savepoint.action == kCharacterActionDefault)
				getSound()->playSound(kCharacterVassili, "VAS1028", kVolumeFull);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Vassili, seizure)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		// Check that we have removed the body from the train and changed jacket
		if (!getProgress().eventCorpseMovedFromFloor) {
			getActionOld()->playAnimation(kEventMertensCorpseFloor);
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, true);
			break;
		}

		if (!getProgress().eventCorpseThrown) {
			getActionOld()->playAnimation(kEventMertensCorpseBed);
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, true);
			break;
		}

		if (getProgress().jacket == kJacketBlood) {
			getActionOld()->playAnimation(kEventMertensBloodJacket);
			getLogic()->gameOver(kSavegameTypeIndex, 0, kSceneNone, true);
			break;
		}

		// Setup Anna & Coudert
		RESET_ENTITY_STATE(kCharacterAnna, Anna, setup_function37);
		RESET_ENTITY_STATE(kCharacterCond2, Coudert, setup_function38);

		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventVassiliSeizure);
		break;

	case kCharacterActionCallback:
		if (getCallback() != 1)
			break;

		getEntityData(kCharacterCath)->location = kLocationInsideCompartment;
		getActionOld()->playAnimation(kEventVassiliSeizure);

		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment1, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getProgress().field_18 = 2;

		getSavePoints()->push(kCharacterVassili, kCharacterAnna, kCharacterAction191477936);
		getSavePoints()->push(kCharacterVassili, kCharacterTrainM, kCharacterAction191477936);
		getSavePoints()->push(kCharacterVassili, kCharacterCond2, kCharacterAction191477936);
		getScenes()->loadSceneFromObject(kObjectCompartmentA);

		setup_drawInBed();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Vassili, drawInBed)
	if (savepoint.action == kCharacterActionDefault)
		getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Vassili, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_sleeping();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVassili);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->updateModel(kObjectCompartmentA, kObjectModel1);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Vassili, sleeping)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				break;

			setCallback(1);
			setup_draw("303B");
		} else {
			params->param3 = 0;
			if (params->param2)
				getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		}
		break;

	case kCharacterActionDefault:
		params->param1 = 5 * (3 * rnd(25) + 15);
		getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		break;

	case kCharacterActionCallback:
		if (getCallback() != 1)
			break;

		getEntities()->drawSequenceLeft(kCharacterVassili, "303C");
		params->param1 = 5 * (3 * rnd(25) + 15);
		params->param2 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Vassili, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_stealEgg();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVassili);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Vassili, stealEgg)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				break;

			setCallback(1);
			setup_draw("303B");
		} else {
			params->param3 = 0;
			if (params->param2)
				getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		}
		break;

	case kCharacterActionOpenDoor:
		setCallback(2);
		setup_savegame(kSavegameTypeEvent, kEventVassiliCompartmentStealEgg);
		break;

	case kCharacterActionDefault:
		params->param1 = 5 * (3 * rnd(25) + 15);
		getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		break;

	case kCharacterActionDrawScene:
		if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_7850)
		 && getInventory()->hasItem(kItemFirebird)
		 && !HELPERgetEvent(kEventVassiliCompartmentStealEgg))
			getObjects()->update(kObject48, kCharacterVassili, kObjectLocationNone, kCursorNormal, kCursorHand);
		else
			getObjects()->update(kObject48, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorHand);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterVassili, "303C");
			params->param1 = 5 * (3 * rnd(25) + 15);
			params->param2 = 1;
			break;

		case 2:
			getActionOld()->playAnimation(kEventVassiliCompartmentStealEgg);
			getScenes()->loadSceneFromPosition(kCarRedSleeping, 67);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Vassili, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter4Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterVassili);

		getData()->entityPosition = kPosition_8200;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCompartmentA, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->updateModel(kObjectCompartmentA, kObjectModel1);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// Looks identical to sleeping (#13)
IMPLEMENT_FUNCTION(17, Vassili, chapter4Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getEntities()->isInsideCompartment(kCharacterCath, kCarRedSleeping, kPosition_8200)) {
			if (!Entity::updateParameter(params->param3, getState()->timeTicks, params->param1))
				break;

			setCallback(1);
			setup_draw("303B");
		} else {
			params->param3 = 0;
			if (params->param2)
				getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		}
		break;

	case kCharacterActionDefault:
		params->param1 = 5 * (3 * rnd(25) + 15);
		getEntities()->drawSequenceLeft(kCharacterVassili, "303A");
		break;

	case kCharacterActionCallback:
		if (getCallback() != 1)
			break;

		getEntities()->drawSequenceLeft(kCharacterVassili, "303C");
		params->param1 = 5 * (3 * rnd(25) + 15);
		params->param2 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Vassili, chapter5)
	if (savepoint.action == kCharacterActionDefault) {
		getEntities()->clearSequences(kCharacterVassili);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
