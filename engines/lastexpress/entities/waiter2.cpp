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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "lastexpress/entities/waiter2.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Waiter2::Waiter2(LastExpressEngine *engine) : Entity(engine, kEntityWaiter2) {
	ADD_CALLBACK_FUNCTION_I(Waiter2, updateFromTime);
	ADD_CALLBACK_FUNCTION_S(Waiter2, draw);
	ADD_CALLBACK_FUNCTION_SII(Waiter2, updatePosition);
	ADD_CALLBACK_FUNCTION(Waiter2, callbackActionOnDirection);
	ADD_CALLBACK_FUNCTION_SIIS(Waiter2, callSavepoint);
	ADD_CALLBACK_FUNCTION_S(Waiter2, playSound);
	ADD_CALLBACK_FUNCTION(Waiter2, monsieurServeUs);
	ADD_CALLBACK_FUNCTION(Waiter2, chapter1);
	ADD_CALLBACK_FUNCTION(Waiter2, milosOrder);
	ADD_CALLBACK_FUNCTION(Waiter2, monsieurOrder);
	ADD_CALLBACK_FUNCTION(Waiter2, clearAlexei);
	ADD_CALLBACK_FUNCTION(Waiter2, clearMilos);
	ADD_CALLBACK_FUNCTION(Waiter2, clearMonsieur);
	ADD_CALLBACK_FUNCTION(Waiter2, servingDinner);
	ADD_CALLBACK_FUNCTION(Waiter2, function15);
	ADD_CALLBACK_FUNCTION(Waiter2, function16);
	ADD_CALLBACK_FUNCTION(Waiter2, chapter2);
	ADD_CALLBACK_FUNCTION(Waiter2, inKitchen);
	ADD_CALLBACK_FUNCTION(Waiter2, tatianaClearTableB);
	ADD_CALLBACK_FUNCTION(Waiter2, ivoComeHere);
	ADD_CALLBACK_FUNCTION(Waiter2, ivoClearTableC);
	ADD_CALLBACK_FUNCTION(Waiter2, chapter3);
	ADD_CALLBACK_FUNCTION(Waiter2, serving3);
	ADD_CALLBACK_FUNCTION(Waiter2, annaBringTea3);
	ADD_CALLBACK_FUNCTION(Waiter2, chapter4);
	ADD_CALLBACK_FUNCTION(Waiter2, serving4);
	ADD_CALLBACK_FUNCTION(Waiter2, augustNeedsADrink);
	ADD_CALLBACK_FUNCTION(Waiter2, serveAugustADrink);
	ADD_CALLBACK_FUNCTION(Waiter2, annaNeedsADrink);
	ADD_CALLBACK_FUNCTION(Waiter2, chapter5);
	ADD_CALLBACK_FUNCTION(Waiter2, chapter5Handler);
	ADD_NULL_FUNCTION()
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(1, Waiter2, updateFromTime)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Waiter2, draw)
	Entity::draw(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SII(3, Waiter2, updatePosition, CarIndex, Position)
	Entity::updatePosition(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Waiter2, callbackActionOnDirection)
	if (savepoint.action == kActionExcuseMeCath) {
		if (!params->param1) {
			getSound()->excuseMe(kEntityWaiter2);
			params->param1 = 1;
		}
	}

	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(5, Waiter2, callSavepoint, EntityIndex, ActionIndex)
	Entity::callSavepoint(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Waiter2, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Waiter2, monsieurServeUs)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kEntityWaiter2, kEntityBoutarel, kAction122358304);
			setCallback(2);
			setup_draw("008C");
			break;

		case 2:
			getSavePoints()->push(kEntityWaiter2, kEntityBoutarel, kAction122288808);
			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kEntityWaiter2);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(1, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Waiter2, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_servingDinner();
		break;

	case kActionDefault:
		getSavePoints()->addData(kEntityWaiter2, kAction223002560, 0);
		getSavePoints()->addData(kEntityWaiter2, kAction302996448, 2);
		getSavePoints()->addData(kEntityWaiter2, kAction269485588, 3);
		getSavePoints()->addData(kEntityWaiter2, kAction326144276, 4);
		getSavePoints()->addData(kEntityWaiter2, kAction302203328, 5);
		getSavePoints()->addData(kEntityWaiter2, kAction189688608, 6);
		getSavePoints()->addData(kEntityWaiter2, kAction236237423, 7);
		getSavePoints()->addData(kEntityWaiter2, kAction219377792, 8);
		getSavePoints()->addData(kEntityWaiter2, kAction256200848, 9);
		getSavePoints()->addData(kEntityWaiter2, kAction291721418, 10);
		getSavePoints()->addData(kEntityWaiter2, kAction258136010, 11);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Waiter2, milosOrder)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityMilos, "BLANK");
			getEntities()->drawSequenceLeft(kEntityWaiter2, "009B");

			setCallback(2);
			setup_playSound("WAT1001");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kEntityMilos, "009A");

			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kEntityWaiter2);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(0, 1) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Waiter2, monsieurOrder)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kEntityBoutarel, "BLANK");
			getEntities()->drawSequenceLeft(kEntityWaiter2, "008C");

			setCallback(2);
			setup_playSound("MRB1077");
			break;

		case 2:
			getSavePoints()->push(kEntityWaiter2, kEntityBoutarel, kAction168717392);

			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kEntityWaiter2);
			getData()->entityPosition = kPosition_5900;
			ENTITY_PARAM(1, 2) = 0;

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Waiter2, clearAlexei)
	serveTable(savepoint, "919", kEntityTables1, "005H", "005J", "921", &ENTITY_PARAM(0, 3), 63);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Waiter2, clearMilos)
	serveTable(savepoint, "923", kEntityTables2, "009F", "009G", "926", &ENTITY_PARAM(0, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Waiter2, clearMonsieur)
	serveTable(savepoint, "923", kEntityTables2, "009F", "009G", "926", &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Waiter2, servingDinner)
switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getEntities()->isInKitchen(kEntityWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(0, 1)) {
			setCallback(1);
			setup_milosOrder();
			break;
		}

		if (ENTITY_PARAM(1, 2)) {
			setCallback(2);
			setup_monsieurOrder();
			break;
		}

		if (ENTITY_PARAM(0, 3)) {
			setCallback(3);
			setup_clearAlexei();
			break;
		}

		if (ENTITY_PARAM(0, 4)) {
			setCallback(4);
			setup_clearMilos();
			break;
		}

		if (ENTITY_PARAM(0, 5)) {
			setCallback(5);
			setup_clearMonsieur();
		}
		break;

	case kActionCallback:
		if (getCallback() == 5) {
			getSavePoints()->push(kEntityWaiter2, kEntityPascale, kAction352768896);
			setup_function15();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Waiter2, function15)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5900;
		break;

	case kAction101632192:
		setup_function16();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Waiter2, function16)
	if (savepoint.action == kActionDefault) {
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;

		getEntities()->clearSequences(kEntityWaiter2);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Waiter2, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_inKitchen();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityWaiter2);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(0, 6) = 0;
		ENTITY_PARAM(0, 7) = 0;
		ENTITY_PARAM(0, 8) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Waiter2, inKitchen)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getEntities()->isInKitchen(kEntityWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(0, 6)) {
			setCallback(1);
			setup_tatianaClearTableB();
			break;
		}

label_callback_1:
		if (ENTITY_PARAM(0, 7)) {
			setCallback(2);
			setup_ivoComeHere();
			break;
		}

label_callback_2:
		if (ENTITY_PARAM(0, 8) || ENTITY_PARAM(0, 5)) {
			setCallback(3);
			setup_ivoClearTableC();
		}
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 4:
			getEntities()->clearSequences(kEntityWaiter2);
			getData()->entityPosition = kPosition_5900;
			break;
		}
		break;

	case kAction101106391:
		setCallback(4);
		setup_draw("975");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Waiter2, tatianaClearTableB)
	serveTable(savepoint, "969", kEntityTables1, "005H2", "018A", "971", &ENTITY_PARAM(0, 6), 63);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Waiter2, ivoComeHere)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("973");
		break;

	case kActionCallback:
		if (getCallback() == 1) {
			getSavePoints()->push(kEntityWaiter2, kEntityIvo, kAction123712592);
			getEntities()->drawSequenceLeft(kEntityWaiter2, "BLANK");
			ENTITY_PARAM(0, 7) = 0;

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Waiter2, ivoClearTableC)
	serveTable(savepoint, "974", kEntityTables2, "009F2", "009G", "976", &ENTITY_PARAM(0, 8), 0, true, &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Waiter2, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_serving3();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityWaiter2);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothes1;
		getData()->inventoryItem = kItemNone;

		ENTITY_PARAM(1, 1) = 0;
		ENTITY_PARAM(1, 2) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(23, Waiter2, serving3)
	if (savepoint.action != kActionNone)
		return;

	if (!getEntities()->isInKitchen(kEntityWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
		return;

	if (ENTITY_PARAM(1, 1)) {
		setCallback(1);
		setup_annaBringTea3();
		return;
	}

	if (ENTITY_PARAM(1, 2)) {
		setCallback(2);
		setup_monsieurServeUs();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(24, Waiter2, annaBringTea3)
	serveSalon(savepoint, "927", "Ann3143A", kEntityAnna, "Ann3144", "112C", kAction122288808, "928", &ENTITY_PARAM(1, 1));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Waiter2, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_serving4();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityWaiter2);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getEntities()->clearSequences(kEntityWaiter2);

		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(26, Waiter2, serving4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (params->param1) {
			if (Entity::updateParameter(params->param2, getState()->time, 900)) {
				ENTITY_PARAM(1, 5) = 1;
				params->param1 = 0;
			}
		}

		if (!getEntities()->isInKitchen(kEntityWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
			break;

		if (ENTITY_PARAM(1, 3)) {
			setCallback(1);
			setup_augustNeedsADrink();
			break;
		}

		if (ENTITY_PARAM(1, 5)) {
			setCallback(2);
			setup_serveAugustADrink();
			break;
		}

		if (ENTITY_PARAM(1, 4)) {
			setCallback(3);
			setup_annaNeedsADrink();
			break;
		}

		if (ENTITY_PARAM(1, 2)) {
			setCallback(4);
			setup_monsieurServeUs();
		}
		break;

	case kActionCallback:
		if (getCallback() == 1)
			params->param1 = 1;
		break;

	case kAction201431954:
		ENTITY_PARAM(1, 2) = 0;
		ENTITY_PARAM(1, 3) = 0;
		ENTITY_PARAM(1, 4) = 0;
		ENTITY_PARAM(1, 5) = 0;

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(27, Waiter2, augustNeedsADrink)
	serveSalon(savepoint, "929", "", kEntityAugust, "Aug4003", "122D", kAction134486752, "930", &ENTITY_PARAM(1, 3));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Waiter2, serveAugustADrink)
	serveSalon(savepoint, "931", "", kEntityAugust, "Aug4004", "122E", kAction125826561, "930", &ENTITY_PARAM(1, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Waiter2, annaNeedsADrink)
	serveSalon(savepoint, "932", "", kEntityAnna, "Ann4151", "127D", kAction122288808, "930", &ENTITY_PARAM(1, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Waiter2, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_chapter5Handler();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityWaiter2);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Waiter2, chapter5Handler)
	if (savepoint.action == kActionProceedChapter5)
		setup_nullfunction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(32, Waiter2)


//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Waiter2::serveTable(const SavePoint &savepoint, const char *seq1, EntityIndex entity, const char *seq2, const char *seq3, const char *seq4, uint *parameter, Position position, bool shouldUpdatePosition, uint *parameter2) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		if (shouldUpdatePosition) {
			getData()->entityPosition = kPosition_5800;
			getData()->location = kLocationOutsideCompartment;
		}

		setCallback(1);
		setup_draw(seq1);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (position)
				getEntities()->updatePositionEnter(kEntityWaiter2, kCarRestaurant, position);

			getSavePoints()->push(kEntityWaiter2, entity, kAction136455232);

			setCallback(2);
			setup_callSavepoint(seq2, entity, kActionDrawTablesWithChairs, seq3);
			break;

		case 2:
			if (position)
				getEntities()->updatePositionExit(kEntityWaiter2, kCarRestaurant, position);

			setCallback(3);
			setup_draw(seq4);
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kEntityWaiter2);
			*parameter = 0;

			if (parameter2 != NULL)
				*parameter2 = 0;

			callbackAction();
			break;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void Waiter2::serveSalon(const SavePoint &savepoint, const char *seq1, const char *snd1, EntityIndex entity, const char *snd2, const char *seq2, ActionIndex action, const char *seq3, uint *parameter) {
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("816DD");
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kEntityWaiter2, seq1);

			if (getEntities()->isInRestaurant(kEntityPlayer))
				getEntities()->updateFrame(kEntityWaiter2);

			if (strcmp(snd1, ""))
				getSound()->playSound(kEntityWaiter2, snd1);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getSavePoints()->push(kEntityWaiter2, entity, kAction122358304);

			getSound()->playSound(kEntityWaiter2, snd2);

			setCallback(3);
			// the last arg is actually a constant varying between calls,
			// but this function already has too many args to add yet another one
			setup_updatePosition(seq2, kCarRestaurant, strcmp(seq2, "127D") ? 57 : 56);
			break;

		case 3:
			getSavePoints()->push(kEntityWaiter2, entity, action);

			setCallback(4);
			setup_draw(seq3);
			break;

		case 4:
			getEntities()->drawSequenceRight(kEntityWaiter2, "816UD");

			if (getEntities()->isInSalon(kEntityPlayer))
				getEntities()->updateFrame(kEntityWaiter2);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kEntityWaiter2);
			getData()->entityPosition = kPosition_5900;
			*parameter = 0;

			callbackAction();
			break;
		}
		break;
	}
}

} // End of namespace LastExpress
