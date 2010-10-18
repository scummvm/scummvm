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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lastexpress/entities/kronos.h"

#include "lastexpress/entities/anna.h"
#include "lastexpress/entities/august.h"
#include "lastexpress/entities/rebecca.h"
#include "lastexpress/entities/sophie.h"
#include "lastexpress/entities/tatiana.h"

#include "lastexpress/game/action.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/sound.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"
#include "lastexpress/helpers.h"

namespace LastExpress {

Kronos::Kronos(LastExpressEngine *engine) : Entity(engine, kEntityKronos) {
	ADD_CALLBACK_FUNCTION(Kronos, reset);
	ADD_CALLBACK_FUNCTION(Kronos, savegame);
	ADD_CALLBACK_FUNCTION(Kronos, updateEntity);
	ADD_CALLBACK_FUNCTION(Kronos, playSound);
	ADD_CALLBACK_FUNCTION(Kronos, updateFromTime);
	ADD_CALLBACK_FUNCTION(Kronos, updateFromTicks);
	ADD_CALLBACK_FUNCTION(Kronos, chapter1);
	ADD_CALLBACK_FUNCTION(Kronos, chapter1Handler);
	ADD_CALLBACK_FUNCTION(Kronos, function9);
	ADD_CALLBACK_FUNCTION(Kronos, function10);
	ADD_CALLBACK_FUNCTION(Kronos, function11);
	ADD_CALLBACK_FUNCTION(Kronos, chapter2);
	ADD_CALLBACK_FUNCTION(Kronos, chapter3);
	ADD_CALLBACK_FUNCTION(Kronos, chapter3Handler);
	ADD_CALLBACK_FUNCTION(Kronos, function15);
	ADD_CALLBACK_FUNCTION(Kronos, function16);
	ADD_CALLBACK_FUNCTION(Kronos, function17);
	ADD_CALLBACK_FUNCTION(Kronos, function18);
	ADD_CALLBACK_FUNCTION(Kronos, function19);
	ADD_CALLBACK_FUNCTION(Kronos, function20);
	ADD_CALLBACK_FUNCTION(Kronos, function21);
	ADD_CALLBACK_FUNCTION(Kronos, function22);
	ADD_CALLBACK_FUNCTION(Kronos, function23);
	ADD_CALLBACK_FUNCTION(Kronos, chapter4);
	ADD_CALLBACK_FUNCTION(Kronos, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Kronos, reset)
	Entity::reset(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(2, Kronos, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(3, Kronos, updateEntity, CarIndex, EntityPosition)
	Entity::updateEntity(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(4, Kronos, playSound)
	Entity::playSound(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Kronos, updateFromTime)
	Entity::updateFromTime(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(6, Kronos, updateFromTicks)
	Entity::updateFromTicks(savepoint);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Kronos, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		TIME_CHECK_CHAPTER1(setup_chapter1Handler);
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarKronos;

		getObjects()->update(kObjectCeiling, kEntityPlayer, kObjectLocation1, kCursorKeepValue, kCursorKeepValue);

		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Kronos, chapter1Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		TIME_CHECK(kTime1489500, params->param2, setup_function11);
		break;

	case kAction171849314:
		params->param1 = 1;
		break;

	case kAction202621266:
		setup_function9();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Kronos, function9)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventKronosConversation);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKronosConversation);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
            getSavePoints()->push(kEntityKronos, kEntityKahina, kAction137685712);
            setup_function10();
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Kronos, function10)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		TIME_CHECK(kTime1489500, params->param1, setup_function11);
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;

		getEntities()->clearSequences(kEntityKronos);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Kronos, function11)
	switch (savepoint.action) {
	default:
		break;

	case kActionEndSound:
		params->param1++;
		getSound()->playSound(kEntityKronos, (params->param1 & 1) ? "KRO1001" : "KRO1002");
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_7000;

		if (!getSound()->isBuffered(kEntityKronos))
			getSound()->playSound(kEntityKronos, "KRO1001");
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Kronos, chapter2)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKronos);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Kronos, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter3Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityKronos);

		getData()->entityPosition = kPosition_6000;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarKronos;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		getObjects()->update(kObjectCeiling, kEntityPlayer, kObjectLocationNone, kCursorKeepValue, kCursorKeepValue);

		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Kronos, chapter3Handler)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime1993500 && !params->param1 && !params->param2 && !params->param3)
			setup_function15();
		break;

	case kAction157159392:
		switch (savepoint.entity2) {
		case kEntityAnna:
			params->param1 = 1;
			break;

		case kEntityTatiana:
			params->param2 = 1;
			break;

		case kEntityAbbot:
			params->param3 = 1;
			break;

		default:
			break;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Kronos, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1 && !getEntities()->isInSalon(kEntityBoutarel)) {
			UPDATE_PARAM_PROC(params->param2, getState()->timeTicks, 75)
				setup_function16();
				break;
			UPDATE_PARAM_PROC_END
		}

		if (params->param3 != kTimeInvalid && getState()->time > kTime2002500) {
			if (getState()->time <= kTime2052000) {
				if (!getEntities()->isInSalon(kEntityPlayer) || getEntities()->isInSalon(kEntityPlayer) || !params->param3)
					params->param3 = getState()->time + 900;

				if (params->param3 >= getState()->time)
					break;
			}

			params->param3 = kTimeInvalid;

			if (getEntities()->isInSalon(kEntityPlayer)) {
				setup_function16();
			} else {
				getSavePoints()->push(kEntityKronos, kEntityAnna, kAction101169422);
				getSavePoints()->push(kEntityKronos, kEntityTatiana, kAction101169422);
				getSavePoints()->push(kEntityKronos, kEntityAbbot, kAction101169422);

				setup_function18();
			}
		}
		break;

	case kActionDefault:
		if (getEntities()->isPlayerPosition(kCarRestaurant, 60)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 59)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 83)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 81)
		 || getEntities()->isPlayerPosition(kCarRestaurant, 87))
			params->param1 = 1;
		break;

	case kActionDrawScene:
		if (params->param1 && getEntities()->isPlayerPosition(kCarRestaurant, 51) && !getEntities()->isInSalon(kEntityBoutarel))
			setup_function16();
		else
			params->param1 = getEntities()->isPlayerPosition(kCarRestaurant, 60)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 59)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 83)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 81)
			              || getEntities()->isPlayerPosition(kCarRestaurant, 87);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16 ,Kronos, function16)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		setCallback(1);
		setup_savegame(kSavegameTypeEvent, kEventKronosVisit);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKronosVisit);
			getSavePoints()->push(kEntityKronos, kEntityAnna, kAction101169422);
			getSavePoints()->push(kEntityKronos, kEntityTatiana, kAction101169422);
			getSavePoints()->push(kEntityKronos, kEntityAbbot, kAction101169422);
			getScenes()->loadSceneFromPosition(kCarRestaurant, 60);

			setup_function17();
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Kronos, function17)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_7500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRedSleeping;

		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_9270);
		break;

	case kActionCallback:
		if (getCallback() == 1)
			setup_function18();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Kronos, function18)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getState()->time > kTime2079000 && !params->param2) {
			getObjects()->updateLocation2(kObjectCompartmentKronos, kObjectLocation3);
			getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param1 = 1;
			params->param2 = 1;
		}

		TIME_CHECK(kTime2106000, params->param3, setup_function19)
		else {
			if (params->param1 && getEntities()->isInKronosSanctum(kEntityPlayer)) {
				setCallback(1);
				setup_savegame(kSavegameTypeEvent, kEventKahinaPunchSuite4);
			}
		}
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_6000;
		getData()->car = kCarKronos;
		getData()->location = kLocationOutsideCompartment;
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeEvent2, kEventCathJumpDownCeiling, kSceneNone, true);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Kronos, function19)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation1, kCursorNormal, kCursorNormal);
		break;

	case kActionDrawScene:
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeEvent2, kEventCathJumpDownCeiling, kSceneNone, true);
			break;

		case 2:
			getAction()->playAnimation(kEventConcertStart);
			getSound()->setupEntry(SoundManager::kSoundType7, kEntityKronos);
			getScenes()->loadSceneFromPosition(kCarKronos, 83);

			RESET_ENTITY_STATE(kEntityRebecca, Rebecca, setup_function39);
			RESET_ENTITY_STATE(kEntitySophie, Sophie, setup_chaptersHandler);
			RESET_ENTITY_STATE(kEntityAugust, August,  setup_function50);
			RESET_ENTITY_STATE(kEntityAnna, Anna, setup_function56);
			RESET_ENTITY_STATE(kEntityTatiana, Tatiana, setup_function35);

			setup_function20();
			break;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Kronos, function20)
	error("Kronos: callback function 20 not implemented!");
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Kronos, function21)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInKronosSanctum(kEntityPlayer)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
		}
		break;

	case kActionDefault:
		getProgress().field_40 = 0;
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorNormal, kCursorNormal);
		getSavePoints()->push(kEntityKronos, kEntityRebecca, kAction191668032);
		if (!getEvent(kEventConcertLeaveWithBriefcase))
			setup_function22();
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKahinaWrongDoor);

			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");

			getScenes()->loadSceneFromPosition(kCarKronos, 81);

			getSound()->playSound(kEntityPlayer, "LIB015");
		}
		break;

	case kAction235599361:
		setup_function22();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Kronos, function22)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getProgress().field_44) {
			setCallback(5);
			setup_savegame(kSavegameTypeEvent, kEventKahinaPunchBaggageCarEntrance);
		} else {
			setCallback(6);
			setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
		}
		break;

	case kActionKnock:
	case kActionOpenDoor:
		if (!getSound()->isBuffered(savepoint.action == kActionKnock ? "LIB012" : "LIB013", true))
			getSound()->playSound(kEntityPlayer, savepoint.action == kActionKnock ? "LIB012" : "LIB013");

		if (getEvent(kEventConcertLeaveWithBriefcase))
			getSavePoints()->call(kEntityKronos, kEntityKahina, kAction137503360);

		if (getInventory()->hasItem(kItemBriefcase)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKronosReturnBriefcase);
			break;
		}

		if (getInventory()->hasItem(kItemFirebird) && getEvent(kEventConcertLeaveWithBriefcase)) {
			setCallback(2);
			setup_savegame(kSavegameTypeEvent, kEventKronosBringEggCeiling);
			break;
		}

		if (getInventory()->hasItem(kItemFirebird)) {
			setCallback(3);
			setup_savegame(kSavegameTypeEvent, kEventKronosBringEggCeiling);
			break;
		}

		if (getEvent(kEventConcertLeaveWithBriefcase)) {
			setCallback(4);
			setup_savegame(kSavegameTypeEvent, kEventKronosBringNothing);
			break;
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityKronos, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getAction()->playAnimation(kEventKronosReturnBriefcase);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getInventory()->removeItem(kItemFirebird);
			getInventory()->removeItem(kItemScarf);

			setup_function23();
			break;

		case 2:
			getAction()->playAnimation(kEventKronosBringEggCeiling);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getInventory()->removeItem(kItemFirebird);
			getInventory()->get(kItemFirebird)->location = kObjectLocation5;

			setup_function23();
			break;

		case 3:
			getInventory()->removeItem(kItemFirebird);
			getInventory()->get(kItemFirebird)->location = kObjectLocation5;
			getAction()->playAnimation(kEventKronosBringEgg);
			getScenes()->loadSceneFromPosition(kCarKronos, 87);
			getInventory()->addItem(kItemBriefcase);
			setup_function23();
			break;

		case 4:
			getAction()->playAnimation(kEventKronosBringNothing);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;

		case 5:
			getAction()->playAnimation(kEventKahinaPunchSuite4);
			getLogic()->gameOver(kSavegameTypeIndex, 1, kSceneNone, true);
			break;

		case 6:
			getAction()->playAnimation(kEventKahinaWrongDoor);
			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");
			getScenes()->loadSceneFromPosition(kCarKronos, 81);
			getSound()->playSound(kEntityPlayer, "LIB015");
			break;
		}
		break;

	case kAction138085344:
		setup_function23();
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Kronos, function23)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (getEntities()->isInKronosSanctum(kEntityPlayer)) {
			setCallback(1);
			setup_savegame(kSavegameTypeEvent, kEventKahinaWrongDoor);
		}
		break;

	case kActionDefault:
		getObjects()->update(kObjectCompartmentKronos, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getAction()->playAnimation(kEventKahinaWrongDoor);

			if (getInventory()->hasItem(kItemBriefcase))
				getInventory()->removeItem(kItemBriefcase);

			getSound()->playSound(kEntityPlayer, "BUMP");

			getScenes()->loadSceneFromPosition(kCarKronos, 81);

			getSound()->playSound(kEntityPlayer, "LIB015");
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Kronos, chapter4)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKronos);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Kronos, chapter5)
	if (savepoint.action == kActionDefault)
		getEntities()->clearSequences(kEntityKronos);
}

} // End of namespace LastExpress
