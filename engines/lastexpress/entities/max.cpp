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

#include "lastexpress/entities/max.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Max::Max(LastExpressEngine *engine) : Entity(engine, kCharacterMax) {
	ADD_CALLBACK_FUNCTION(Max, reset);
	ADD_CALLBACK_FUNCTION_S(Max, playSound);
	ADD_CALLBACK_FUNCTION_S(Max, draw);
	ADD_CALLBACK_FUNCTION_SI(Max, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_II(Max, savegame);
	ADD_CALLBACK_FUNCTION(Max, withAnna);
	ADD_CALLBACK_FUNCTION(Max, guardingCompartment);
	ADD_CALLBACK_FUNCTION(Max, inCageFriendly);
	ADD_CALLBACK_FUNCTION(Max, function9);
	ADD_CALLBACK_FUNCTION(Max, chapter1);
	ADD_CALLBACK_FUNCTION(Max, chapter2);
	ADD_CALLBACK_FUNCTION(Max, chapter3);
	ADD_CALLBACK_FUNCTION(Max, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Max, inCageMad);
	ADD_CALLBACK_FUNCTION(Max, letMeIn);
	ADD_CALLBACK_FUNCTION(Max, chapter4);
	ADD_CALLBACK_FUNCTION(Max, function17);
	ADD_CALLBACK_FUNCTION(Max, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Max, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Max, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(3, Max, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(4, Max, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(5, Max, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Max, withAnna)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max1122");

		params->param1 = 225 * (4 * rnd(20) + 40);
		params->param2 = 0;
		break;

	case kCharacterActionDefault:
		params->param1 = 225 * (4 * rnd(20) + 40);
		break;

	case kCharacterAction71277948:
		setCallback(1);
		setup_guardingCompartment();
		break;

	case kCharacterAction158007856:
		if (!getSoundQueue()->isBuffered(kCharacterMax)) {
			getSound()->playSound(kCharacterMax, "Max1122");
			params->param1 = 225 * (4 * rnd(20) + 40);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Max, guardingCompartment)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param2, getState()->time, params->param1))
			break;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max1122");

		params->param1 = 225 * (4 * rnd(20) + 40);
		params->param2 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		getObjects()->update(kObjectCompartmentF, kCharacterMax, kObjectLocation1, kCursorNormal, kCursorNormal);
		getObjects()->update(kObject53, kCharacterMax, kObjectLocation1, kCursorNormal, kCursorNormal);

		if (getSoundQueue()->isBuffered(kCharacterMax))
			getSoundQueue()->fade(kCharacterMax);

		setCallback((savepoint.action == kCharacterActionKnock) ? 1 : 2);
		setup_playSound((savepoint.action == kCharacterActionKnock) ? "LIB012" : "LIB013");
		break;

	case kCharacterActionDefault:
		params->param1 = 225 * (4 * rnd(20) + 40);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;

		getObjects()->update(kObjectCompartmentF, kCharacterMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject53, kCharacterMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (!getSoundQueue()->isBuffered(kCharacterMax)) {
			if (getEntities()->isPlayerPosition(kCarRedSleeping, 56) || getEntities()->isPlayerPosition(kCarRedSleeping, 78))
				getSound()->playSound(kCharacterMax, "MAX1120");
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		case 0:
		default:
			break;

		case 1:
		case 2:
			setCallback(3);
			setup_playSound("MAX1122");
			break;

		case 3:
			getObjects()->update(kObjectCompartmentF, kCharacterMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObject53, kCharacterMax, kObjectLocation1, kCursorHandKnock, kCursorHand);
			break;
		}
		break;

	case kCharacterAction101687594:
		getEntities()->clearSequences(kCharacterMax);

		callbackAction();
		break;

	case kCharacterAction122358304:
	case kCharacterActionMaxFreeFromCage:
		getSavePoints()->push(kCharacterMax, kCharacterMax, kCharacterActionMaxFreeFromCage);
		getObjects()->update(kObjectCompartmentF, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject53, kCharacterCath, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		callbackAction();
		break;

	case kCharacterAction158007856:
		if (!getSoundQueue()->isBuffered(kCharacterMax)) {
			getSound()->playSound(kCharacterMax, "Max1122");
			params->param1 = 225 * (4 * rnd(20) + 40);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Max, inCageFriendly)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param3, getState()->time, params->param2))
			break;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max3101");

		params->param2 = 225 * (4 * rnd(4) + 8);
		params->param3 = 0;
		break;

	case kCharacterActionOpenDoor:
		if (params->param1) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventCathMaxLickHand);
			break;
		}

		if (getSoundQueue()->isBuffered(kCharacterMax))
			getSoundQueue()->fade(kCharacterMax);

		getActionOld()->playAnimation(kEventCathMaxLickHand);
		getScenes()->processScene();

		params->param1 = 1;
		break;

	case kCharacterActionDefault:
		params->param2 = 225 * (4 * rnd(4) + 8);

		getObjects()->update(kObjectCageMax, kCharacterMax, kObjectLocationNone, kCursorNormal, kCursorHand);
		getEntities()->clearSequences(kCharacterMax);

		getData()->entityPosition = kPosition_8000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggage;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max3101");
		break;

	case kCharacterActionCallback:
		if (getCallback() != 1)
			break;

		if (getSoundQueue()->isBuffered(kCharacterMax))
			getSoundQueue()->fade(kCharacterMax);

		getSound()->playSound(kCharacterCath, "LIB026");
		getActionOld()->playAnimation(kEventCathMaxFree);
		getScenes()->loadSceneFromPosition(kCarBaggage, 92);
		getObjects()->update(kObjectCageMax, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorHand);
		setup_function9();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Max, function9)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param2 == kTimeInvalid || !getState()->time)
			break;

		if (params->param1 >= getState()->time) {
			if (!getEntities()->hasValidFrame(kCharacterMax) || !params->param2) {

				params->param2 = (uint)getState()->time;
				if (!params->param2)
					goto setup_functions;
			}

			if (params->param2 >= getState()->time)
				break;
		}

		params->param2 = kTimeInvalid;

setup_functions:
		if (getProgress().chapter == kChapter3)
			setup_letMeIn();

		if (getProgress().chapter == kChapter4)
			setup_function17();
		break;

	//////////////////////////////////////////////////////////////////////////
	// Draw Max outside of cage
	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->drawSequenceLeft(kCharacterMax, "630Af");
		getEntities()->enterCompartment(kCharacterMax, kObjectCompartmentF, true);

		params->param1 = (uint)(getState()->time + 2700);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Max, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Max, setup_withAnna));
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Max, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_withAnna();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMax);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Max, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter3Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMax);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Max, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param2) {
			getData()->entityPosition = getEntityData(kCharacterCond2)->entityPosition;
			getData()->location = getEntityData(kCharacterCond2)->location;
			getData()->car = getEntityData(kCharacterCond2)->car;
			break;
		}

		if (!Entity::updateParameter(params->param3, getState()->time, params->param1))
			break;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max1122");

		params->param1 = 225 * (4 * rnd(20) + 40);
		params->param3 = 0;
		break;

	case kCharacterActionDefault:
		params->param1 = 225 * (4 * rnd(20) + 40);

		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRedSleeping;
		break;

	case kCharacterAction71277948:
		setCallback(1);
		setup_guardingCompartment();
		break;

	case kCharacterAction122358304:
		params->param2 = 1;
		break;

	case kCharacterActionMaxFreeFromCage:
		setup_inCageMad();
		break;

	case kCharacterAction158007856:
		if (params->param2)
			break;

		if (!getSoundQueue()->isBuffered(kCharacterMax)) {
			getSound()->playSound(kCharacterMax, "Max1122");
			params->param1 = 225 * (4 * rnd(20) + 40);
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Max, inCageMad)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionEndSound:
		getSound()->playSound(kCharacterMax, "Max1122");
		break;

	//////////////////////////////////////////////////////////////////////////
	// Save game after freeing Max from his cage
	case kCharacterActionOpenDoor:
		if (HELPERgetEvent(kEventCathMaxCage)) {
			if (!HELPERgetEvent(kEventCathMaxFree)) {
				setCallback(2);
				setup_savegame(kSavegameTypeEvent, kEventCathMaxFree);
			}

		} else {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventCathMaxCage);
		}
		break;

	case kCharacterActionDefault:
		getObjects()->update(kObjectCageMax, kCharacterMax, kObjectLocationNone, kCursorNormal, kCursorHand);

		getData()->entityPosition = kPosition_8000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggage;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max1122");
		break;

	//////////////////////////////////////////////////////////////////////////
	// Play animation for Max in the cage and after opening it
	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (getSoundQueue()->isBuffered(kCharacterMax))
				getSoundQueue()->stop(kCharacterMax);

			getActionOld()->playAnimation(kEventCathMaxCage);
			getSoundQueue()->assignNISLink(kCharacterMax);
			getScenes()->processScene();
			break;

		case 2:
			if (getSoundQueue()->isBuffered(kCharacterMax))
				getSoundQueue()->fade(kCharacterMax);

			getSound()->playSound(kCharacterCath, "LIB026");
			getActionOld()->playAnimation(kEventCathMaxFree);
			getScenes()->loadSceneFromPosition(kCarBaggage, 92);
			getObjects()->update(kObjectCageMax, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorHand);
			setup_function9();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Max, letMeIn)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param2) {
			getData()->entityPosition = getEntityData(kCharacterCond2)->entityPosition;
			getData()->location = getEntityData(kCharacterCond2)->location;
			getData()->car = getEntityData(kCharacterCond2)->car;
		}

		if (!params->param1) {
			if (!Entity::updateParameter(params->param3, getState()->time, 900))
				break;

			getSavePoints()->push(kCharacterMax, kCharacterCond2, kCharacterAction157026693);
		}
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		if (!getSoundQueue()->isBuffered(kCharacterMax))
			getSound()->playSound(kCharacterMax, "Max3010");

		setCallback(1);
		setup_enterExitCompartment("630Bf", kObjectCompartmentF);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getEntities()->drawSequenceLeft(kCharacterMax, "630Af");
			getEntities()->enterCompartment(kCharacterMax, kObjectCompartmentF, true);
			getSavePoints()->push(kCharacterMax, kCharacterAnna, kCharacterAction156622016);
		}
		break;

	case kCharacterAction122358304:
		(savepoint.entity2 == kCharacterAnna) ? (params->param1 = 1) : (params->param2 = 1);
		getEntities()->exitCompartment(kCharacterMax, kObjectCompartmentF, true);
		getEntities()->drawSequenceLeft(kCharacterMax, "BLANK");
		break;

	case kCharacterActionMaxFreeFromCage:
		getEntities()->exitCompartment(kCharacterMax, kObjectCompartmentF, true);
		setup_inCageFriendly();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Max, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inCageFriendly();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMax);

		getData()->entityPosition = kPosition_8000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarBaggage;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Max, function17)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1) {
			getData()->entityPosition = getEntityData(kCharacterCond2)->entityPosition;
			getData()->location = getEntityData(kCharacterCond2)->location;
			getData()->car = getEntityData(kCharacterCond2)->car;
		}
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_4070;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		getEntities()->drawSequenceLeft(kCharacterMax, "630Af");
		getSavePoints()->push(kCharacterMax, kCharacterCond2, kCharacterAction157026693);
		break;

	case kCharacterAction122358304:
		params->param1 = 1;
		getEntities()->exitCompartment(kCharacterMax, kObjectCompartmentF, true);
		getEntities()->drawSequenceLeft(kCharacterMax, "BLANK");
		break;

	case kCharacterActionMaxFreeFromCage:
		getEntities()->exitCompartment(kCharacterMax, kObjectCompartmentF, true);
		setup_inCageFriendly();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Max, chapter5)
	if (savepoint.action == kCharacterActionDefault) {
		getEntities()->clearSequences(kCharacterMax);

		getData()->entityPosition = kPositionNone;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarNone;

		getObjects()->update(kObjectCageMax, kCharacterCath, kObjectLocationNone, kCursorNormal, kCursorHand);
	}
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
