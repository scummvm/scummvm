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

#include "lastexpress/entities/waiter1.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

#define HANDLE_TABLE(index, param, callback, function) \
	if (ENTITY_PARAM(index, param)) { \
		setCallback(callback); \
		function(); \
		break; \
	}

Waiter1::Waiter1(LastExpressEngine *engine) : Entity(engine, kCharacterWaiter1) {
	ADD_CALLBACK_FUNCTION_SIIS(Waiter1, callSavepoint);
	ADD_CALLBACK_FUNCTION_I(Waiter1, updateFromTime);
	ADD_CALLBACK_FUNCTION_S(Waiter1, draw);
	ADD_CALLBACK_FUNCTION_SII(Waiter1, updatePosition);
	ADD_CALLBACK_FUNCTION(Waiter1, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION_S(Waiter1, playSound);
	ADD_CALLBACK_FUNCTION(Waiter1, rebeccaFeedUs);
	ADD_CALLBACK_FUNCTION(Waiter1, rebeccaClearOurTable);
	ADD_CALLBACK_FUNCTION(Waiter1, abbotCheckMe);
	ADD_CALLBACK_FUNCTION(Waiter1, abbotClearTable);
	ADD_CALLBACK_FUNCTION(Waiter1, chapter1);
	ADD_CALLBACK_FUNCTION(Waiter1, annaOrder);
	ADD_CALLBACK_FUNCTION(Waiter1, augustOrder);
	ADD_CALLBACK_FUNCTION(Waiter1, serveAnna);
	ADD_CALLBACK_FUNCTION(Waiter1, serveAugust);
	ADD_CALLBACK_FUNCTION(Waiter1, clearAnna);
	ADD_CALLBACK_FUNCTION(Waiter1, clearTatiana);
	ADD_CALLBACK_FUNCTION(Waiter1, clearAugust1);
	ADD_CALLBACK_FUNCTION(Waiter1, clearAugust2);
	ADD_CALLBACK_FUNCTION(Waiter1, servingDinner);
	ADD_CALLBACK_FUNCTION(Waiter1, function21);
	ADD_CALLBACK_FUNCTION(Waiter1, function22);
	ADD_CALLBACK_FUNCTION(Waiter1, chapter2);
	ADD_CALLBACK_FUNCTION(Waiter1, inKitchen);
	ADD_CALLBACK_FUNCTION(Waiter1, augustComeHere2);
	ADD_CALLBACK_FUNCTION(Waiter1, augustClearTable2);
	ADD_CALLBACK_FUNCTION(Waiter1, chapter3);
	ADD_CALLBACK_FUNCTION(Waiter1, serving3);
	ADD_CALLBACK_FUNCTION(Waiter1, annaComeHere3);
	ADD_CALLBACK_FUNCTION(Waiter1, abbotServeLunch3);
	ADD_CALLBACK_FUNCTION(Waiter1, chapter4);
	ADD_CALLBACK_FUNCTION(Waiter1, serving4);
	ADD_CALLBACK_FUNCTION(Waiter1, augustOrder4);
	ADD_CALLBACK_FUNCTION(Waiter1, serveAugust4);
	ADD_CALLBACK_FUNCTION(Waiter1, augustClearTable);
	ADD_CALLBACK_FUNCTION(Waiter1, chapter5);
	ADD_CALLBACK_FUNCTION(Waiter1, chapter5Handler);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(1, Waiter1, callSavepoint, CharacterIndex, CharacterActions)
	Entity::callSavepoint(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(2, Waiter1, updateFromTime)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Waiter1, draw)
	Entity::draw(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(4, Waiter1, updatePosition)
	Entity::updatePosition(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(5, Waiter1, callbackActionOnDirection)
	EXPOSE_PARAMS(EntityData::EntityParametersIIII);

	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (getData()->direction != kDirectionRight)
			callbackAction();
		break;

	case kCharacterActionExitCompartment:
		callbackAction();
		break;

	case kCharacterActionExcuseMeCath:
		if (!params->param1) {
			getSound()->excuseMe(kCharacterWaiter1);
			params->param1 = 1;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Waiter1, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Waiter1, rebeccaFeedUs)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		ENTITY_PARAM(0, 3) = 0;

		setCallback(1);
		setup_draw("911");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->clearSequences(kCharacterWaiter1);
			getSavePoints()->push(kCharacterWaiter1, kCharacterRebecca, kCharacterAction123712592);
			break;

		case 2:
			getEntities()->clearSequences(kCharacterWaiter1);
			getData()->entityPosition = kPosition_5900;
			callbackAction();
			break;
		}
		break;

	case kCharacterAction136702400:
		setCallback(2);
		setup_draw("913");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Waiter1, rebeccaClearOurTable)
	serveTable(savepoint, "911", kCharacterTableD, "010L", "010M", "913", &ENTITY_PARAM(1, 2));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Waiter1, abbotCheckMe)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("915");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAbbot, kCharacterAction122358304);
			getEntities()->drawSequenceLeft(kCharacterWaiter1, "029D");

			setCallback(2);
			setup_playSound(getProgress().chapter == kChapter3 ? "Abb3016" : "Abb4001");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAbbot, kCharacterAction122288808);

			setCallback(3);
			setup_draw("917");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter1);
			ENTITY_PARAM(2, 2) = 0;
			ENTITY_PARAM(1, 6) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Waiter1, abbotClearTable)
	serveTable(savepoint, "916", kCharacterTableE, "014E", "014F", "918", &ENTITY_PARAM(2, 3), false);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Waiter1, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_servingDinner();
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction270410280, 0);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction304061224, 1);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction252568704, 10);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction286534136, 11);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction218983616, 12);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction218586752, 13);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction207330561, 14);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction286403504, 16);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction218128129, 17);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction270068760, 18);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction223712416, 2);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction237485916, 5);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction188893625, 8);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction204704037, 6);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction292758554, 7);
		getSavePoints()->addData(kCharacterWaiter1, kCharacterAction337548856, 9);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Waiter1, annaOrder)
	handleServer(savepoint, "907", kCharacterAnna, kCharacterAction268773672, &ENTITY_PARAM(0, 1));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Waiter1, augustOrder)
	handleServer(savepoint, "911", kCharacterAugust, kCharacterAction268773672, &ENTITY_PARAM(0, 2), "010F");
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Waiter1, serveAnna)
	handleServer(savepoint, "908", kCharacterAnna, kCharacterAction170016384, &ENTITY_PARAM(0, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Waiter1, serveAugust)
	handleServer(savepoint, "912", kCharacterAugust, kCharacterAction170016384, &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Waiter1, clearAnna)
	serveTable(savepoint, "907", kCharacterTableA, "001N", "001P", "909", &ENTITY_PARAM(0, 6));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Waiter1, clearTatiana)
	serveTable(savepoint, "915", kCharacterTableE, "014E", "014F", "917", &ENTITY_PARAM(1, 1), true, false, 67);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Waiter1, clearAugust1)
	serveTable(savepoint, "911", kCharacterTableD, "010L", "010M", "913", &ENTITY_PARAM(0, 7));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Waiter1, clearAugust2)
	serveTable(savepoint, "911", kCharacterTableD, "010L", "010M", "913", &ENTITY_PARAM(0, 8), true, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Waiter1, servingDinner)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param2) {
			if (Entity::updateParameter(params->param3, getState()->time, 2700)) {
				ENTITY_PARAM(0, 4) = 1;
				params->param2 = 0;
			}
		}

		if (params->param1) {
			if (Entity::updateParameter(params->param4, getState()->time, 4500)) {
				ENTITY_PARAM(0, 5) = 1;
				params->param1 = 0;
			}
		}

		if (!getEntities()->isInKitchen(kCharacterWaiter1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		HANDLE_TABLE(0, 1, 1, setup_annaOrder);
		HANDLE_TABLE(0, 2, 2, setup_augustOrder);
		HANDLE_TABLE(0, 3, 3, setup_rebeccaFeedUs);
		HANDLE_TABLE(0, 4, 4, setup_serveAnna);
		HANDLE_TABLE(0, 5, 5, setup_serveAugust);
		HANDLE_TABLE(0, 6, 6, setup_clearAnna);
		HANDLE_TABLE(1, 1, 7, setup_clearTatiana);
		HANDLE_TABLE(0, 7, 8, setup_clearAugust1);
		HANDLE_TABLE(0, 8, 9, setup_clearAugust2);
		HANDLE_TABLE(1, 2, 10, setup_rebeccaClearOurTable);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 10:
			getSavePoints()->push(kCharacterWaiter1, kCharacterHeadWait, kCharacterAction352703104);
			setup_function21();
			break;

		case 11:
		case 12:
			getEntities()->clearSequences(kCharacterWaiter1);
			getData()->entityPosition = kPosition_5900;

			if (getCallback() == 11)
				params->param2 = 1;
			else
				params->param1 = 1;
			break;

		case 13:
		case 14:
			getEntities()->clearSequences(kCharacterWaiter1);
			getData()->entityPosition = kPosition_5900;
			break;
		}
		break;

	case kCharacterAction136702400:
		setCallback(savepoint.entity2 == kCharacterAnna ? 13 : 14);
		setup_draw(savepoint.entity2 == kCharacterAnna ? "909" : "913");
		break;

	case kCharacterAction203859488:
		setCallback(savepoint.entity2 == kCharacterAnna ? 11 : 12);
		setup_draw(savepoint.entity2 == kCharacterAnna ? "910" : "913");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Waiter1, function21)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5900;
		break;

	case kCharacterAction101632192:
		setup_function22();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Waiter1, function22)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;

		getEntities()->clearSequences(kCharacterWaiter1);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Waiter1, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inKitchen();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter1);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Waiter1, inKitchen)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!getEntities()->isInKitchen(kCharacterWaiter1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		HANDLE_TABLE(1, 3, 1, setup_augustComeHere2);
		HANDLE_TABLE(1, 4, 2, setup_augustClearTable2);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1)
			HANDLE_TABLE(1, 4, 2, setup_augustClearTable2);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Waiter1, augustComeHere2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("957");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAugust, kCharacterAction123712592);
			getEntities()->drawSequenceLeft(kCharacterWaiter1, "BLANK");
			break;

		case 2:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter1);
			ENTITY_PARAM(1, 3) = 0;

			callbackAction();
			break;
		}
		break;

	case kCharacterAction219522616:
		setCallback(2);
		setup_draw("959");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Waiter1, augustClearTable2)
	serveTable(savepoint, "957", kCharacterTableA, "016E", "016D", "959", &ENTITY_PARAM(1, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Waiter1, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_serving3();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter1);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 5) = 0;
		ENTITY_PARAM(1, 6) = 0;
		ENTITY_PARAM(2, 3) = 0;
		ENTITY_PARAM(2, 4) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Waiter1, serving3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!getEntities()->isInKitchen(kCharacterWaiter1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(1, 5)) {
			setCallback(1);
			setup_annaComeHere3();
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(1, 6)) {
			setCallback(2);
			setup_abbotCheckMe();
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(2, 4)) {
			setCallback(3);
			setup_abbotServeLunch3();
			break;
		}

label_callback_3:
		if (ENTITY_PARAM(2, 3)) {
			setCallback(4);
			setup_abbotClearTable();
			break;
		}

label_callback_4:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(5);
			setup_rebeccaFeedUs();
			break;
		}

label_callback_5:
		if (ENTITY_PARAM(1, 2)) {
			setCallback(6);
			setup_rebeccaClearOurTable();
			break;
		}
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
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Waiter1, annaComeHere3)
	// August and Anna order dinner
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("911");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAnna, kCharacterAction122358304);
			getEntities()->drawSequenceLeft(kCharacterWaiter1, "026D");

			setCallback(2);
			setup_playSound("Ann3138");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAnna, kCharacterAction122288808);

			setCallback(3);
			setup_draw("913");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter1);
			ENTITY_PARAM(1, 5) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Waiter1, abbotServeLunch3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("916");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAbbot, kCharacterAction122358304);
			getEntities()->drawSequenceLeft(kCharacterWaiter1, "029D");

			setCallback(2);
			setup_playSound("Abb3016a");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAbbot, kCharacterAction122288808);

			setCallback(3);
			setup_draw("918");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter1);
			ENTITY_PARAM(2, 4) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Waiter1, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_serving4();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter1);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(1, 7) = 0;
		ENTITY_PARAM(1, 8) = 0;
		ENTITY_PARAM(2, 1) = 0;
		ENTITY_PARAM(2, 2) = 0;
		ENTITY_PARAM(2, 3) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(32, Waiter1, serving4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (params->param1) {
			if (Entity::updateParameter(params->param2, getState()->time, 3600)) {
				ENTITY_PARAM(1, 8) = 1;
				params->param1 = 0;
			}
		}

		if (!getEntities()->isInKitchen(kCharacterWaiter1) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(1, 7)) {
			setCallback(1);
			setup_augustOrder4();
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(1, 8)) {
			setCallback(2);
			setup_serveAugust4();
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(2, 1)) {
			setCallback(3);
			setup_augustClearTable();
			break;
		}

label_callback_3:
		if (ENTITY_PARAM(2, 2)) {
			setCallback(4);
			setup_abbotCheckMe();
			break;
		}

label_callback_4:
		if (ENTITY_PARAM(2, 3)) {
			setCallback(5);
			setup_abbotClearTable();
			break;
		}

label_callback_5:
		if (ENTITY_PARAM(0, 3)) {
			setCallback(6);
			setup_rebeccaFeedUs();
			break;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 1;
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 3:
			goto label_callback_3;

		case 4:
			goto label_callback_4;

		case 5:
			goto label_callback_5;
		}
		break;

	case kCharacterAction201431954:
		ENTITY_PARAM(0, 3) = 0;
		ENTITY_PARAM(1, 7) = 0;
		ENTITY_PARAM(1, 8) = 0;
		ENTITY_PARAM(2, 1) = 0;
		ENTITY_PARAM(2, 3) = 0;
		params->param1 = 0;

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(33, Waiter1, augustOrder4)
	// August orders a steak
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_draw("911");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterWaiter1, "010F3");
			getEntities()->drawSequenceLeft(kCharacterAugust, "010D3");

			setCallback(2);
			setup_playSound("AUG4002");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAugust, kCharacterAction122288808);

			setCallback(3);
			setup_draw("913");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter1);
			ENTITY_PARAM(1, 7) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(34, Waiter1, serveAugust4)
	// August is being served
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_draw("912");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAugust, kCharacterAction122358304);
			getSound()->playSound(kCharacterWaiter1, "AUG1053");

			setCallback(2);
			setup_draw("010G3");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter1, kCharacterAugust, kCharacterAction201964801);

			setCallback(3);
			setup_draw("914");
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter1);
			ENTITY_PARAM(1, 8) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(35, Waiter1, augustClearTable)
	serveTable(savepoint, "911", kCharacterTableD, "010L", "010M", "914", &ENTITY_PARAM(2, 1), false, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(36, Waiter1, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter1);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(37, Waiter1, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5)
		setup_nullfunction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(38, Waiter1)


//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Waiter1::handleServer(const SavePoint &savepoint, const char *name, CharacterIndex entity, CharacterActions action, uint *parameter, const char *name2) {
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw(name);
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			// Prepare or draw sequences depending of value of string
			if (!strcmp(name2, ""))
				getEntities()->clearSequences(kCharacterWaiter1);
			else
				getEntities()->drawSequenceLeft(kCharacterWaiter1, name2);

			getSavePoints()->push(kCharacterWaiter1, entity, action);
			*parameter = 0;

			callbackAction();
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void Waiter1::serveTable(const SavePoint &savepoint, const char *seq1, CharacterIndex entity, const char *seq2, const char *seq3, const char *seq4, uint *parameter, bool shouldUpdatePosition, bool pushSavepoint, PositionOld position) {
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		if (shouldUpdatePosition) {
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;
		}

		setCallback(1);
		setup_draw(seq1);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (position)
				getEntities()->updatePositionEnter(kCharacterWaiter1, kCarRestaurant, position);

			getSavePoints()->push(kCharacterWaiter1, entity, kCharacterAction136455232);

			setCallback(2);
			setup_callSavepoint(seq2, entity, kCharacterActionDrawTablesWithChairs, seq3);
			break;

		case 2:
			if (position)
				getEntities()->updatePositionExit(kCharacterWaiter1, kCarRestaurant, position);

			setCallback(3);
			setup_draw(seq4);
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;

			// Special case for functions 19 & 35
			if (pushSavepoint)
				getSavePoints()->push(kCharacterWaiter1, kCharacterRebecca, kCharacterAction224253538);

			getEntities()->clearSequences(kCharacterWaiter1);
			*parameter = 0;

			callbackAction();
			break;
		}
		break;
	}
}

} // End of namespace LastExpress
