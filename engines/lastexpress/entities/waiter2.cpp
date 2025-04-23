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

#include "lastexpress/entities/waiter2.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Waiter2::Waiter2(LastExpressEngine *engine) : Entity(engine, kCharacterWaiter2) {
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
IMPLEMENT_FUNCTION_SII(3, Waiter2, updatePosition, CarIndex, PositionOld)
	Entity::updatePosition(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Waiter2, callbackActionOnDirection)
	if (savepoint.action == kCharacterActionExcuseMeCath) {
		if (!params->param1) {
			getSound()->excuseMe(kCharacterWaiter2);
			params->param1 = 1;
		}
	}

	Entity::callbackActionOnDirection(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIIS(5, Waiter2, callSavepoint, CharacterIndex, CharacterActions)
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

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getSavePoints()->push(kCharacterWaiter2, kCharacterMonsieur, kCharacterAction122358304);
			setCallback(2);
			setup_draw("008C");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter2, kCharacterMonsieur, kCharacterAction122288808);
			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterWaiter2);
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

	case kCharacterActionNone:
		setup_servingDinner();
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction223002560, 0);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction302996448, 2);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction269485588, 3);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction326144276, 4);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction302203328, 5);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction189688608, 6);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction236237423, 7);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction219377792, 8);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction256200848, 9);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction291721418, 10);
		getSavePoints()->addData(kCharacterWaiter2, kCharacterAction258136010, 11);

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

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterMilos, "BLANK");
			getEntities()->drawSequenceLeft(kCharacterWaiter2, "009B");

			setCallback(2);
			setup_playSound("WAT1001");
			break;

		case 2:
			getEntities()->drawSequenceLeft(kCharacterMilos, "009A");

			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterWaiter2);
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

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("924");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceLeft(kCharacterMonsieur, "BLANK");
			getEntities()->drawSequenceLeft(kCharacterWaiter2, "008C");

			setCallback(2);
			setup_playSound("MRB1077");
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter2, kCharacterMonsieur, kCharacterAction168717392);

			setCallback(3);
			setup_draw("926");
			break;

		case 3:
			getEntities()->clearSequences(kCharacterWaiter2);
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
	serveTable(savepoint, "919", kCharacterTableB, "005H", "005J", "921", &ENTITY_PARAM(0, 3), 63);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Waiter2, clearMilos)
	serveTable(savepoint, "923", kCharacterTableC, "009F", "009G", "926", &ENTITY_PARAM(0, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Waiter2, clearMonsieur)
	serveTable(savepoint, "923", kCharacterTableC, "009F", "009G", "926", &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Waiter2, servingDinner)
switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!getEntities()->isInKitchen(kCharacterWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
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

	case kCharacterActionCallback:
		if (getCallback() == 5) {
			getSavePoints()->push(kCharacterWaiter2, kCharacterHeadWait, kCharacterAction352768896);
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

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5900;
		break;

	case kCharacterAction101632192:
		setup_function16();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Waiter2, function16)
	if (savepoint.action == kCharacterActionDefault) {
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;

		getEntities()->clearSequences(kCharacterWaiter2);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Waiter2, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inKitchen();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter2);

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

	case kCharacterActionNone:
		if (!getEntities()->isInKitchen(kCharacterWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
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

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			goto label_callback_1;

		case 2:
			goto label_callback_2;

		case 4:
			getEntities()->clearSequences(kCharacterWaiter2);
			getData()->entityPosition = kPosition_5900;
			break;
		}
		break;

	case kCharacterAction101106391:
		setCallback(4);
		setup_draw("975");
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Waiter2, tatianaClearTableB)
	serveTable(savepoint, "969", kCharacterTableB, "005H2", "018A", "971", &ENTITY_PARAM(0, 6), 63);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Waiter2, ivoComeHere)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("973");
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1) {
			getSavePoints()->push(kCharacterWaiter2, kCharacterIvo, kCharacterAction123712592);
			getEntities()->drawSequenceLeft(kCharacterWaiter2, "BLANK");
			ENTITY_PARAM(0, 7) = 0;

			callbackAction();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Waiter2, ivoClearTableC)
	serveTable(savepoint, "974", kCharacterTableC, "009F2", "009G", "976", &ENTITY_PARAM(0, 8), 0, true, &ENTITY_PARAM(0, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(22, Waiter2, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_serving3();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter2);

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
	if (savepoint.action != kCharacterActionNone)
		return;

	if (!getEntities()->isInKitchen(kCharacterWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
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
	serveSalon(savepoint, "927", "Ann3143A", kCharacterAnna, "Ann3144", "112C", kCharacterAction122288808, "928", &ENTITY_PARAM(1, 1));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(25, Waiter2, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_serving4();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter2);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getEntities()->clearSequences(kCharacterWaiter2);

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

	case kCharacterActionNone:
		if (params->param1) {
			if (Entity::updateParameter(params->param2, getState()->time, 900)) {
				ENTITY_PARAM(1, 5) = 1;
				params->param1 = 0;
			}
		}

		if (!getEntities()->isInKitchen(kCharacterWaiter2) || !getEntities()->isSomebodyInsideRestaurantOrSalon())
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

	case kCharacterActionCallback:
		if (getCallback() == 1)
			params->param1 = 1;
		break;

	case kCharacterAction201431954:
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
	serveSalon(savepoint, "929", "", kCharacterAugust, "Aug4003", "122D", kCharacterAction134486752, "930", &ENTITY_PARAM(1, 3));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(28, Waiter2, serveAugustADrink)
	serveSalon(savepoint, "931", "", kCharacterAugust, "Aug4004", "122E", kCharacterAction125826561, "930", &ENTITY_PARAM(1, 5));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(29, Waiter2, annaNeedsADrink)
	serveSalon(savepoint, "932", "", kCharacterAnna, "Ann4151", "127D", kCharacterAction122288808, "930", &ENTITY_PARAM(1, 4));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(30, Waiter2, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chapter5Handler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterWaiter2);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(31, Waiter2, chapter5Handler)
	if (savepoint.action == kCharacterActionProceedChapter5)
		setup_nullfunction();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(32, Waiter2)


//////////////////////////////////////////////////////////////////////////
// Private functions
//////////////////////////////////////////////////////////////////////////
void Waiter2::serveTable(const SavePoint &savepoint, const char *seq1, CharacterIndex entity, const char *seq2, const char *seq3, const char *seq4, uint *parameter, PositionOld position, bool shouldUpdatePosition, uint *parameter2) {
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
				getEntities()->updatePositionEnter(kCharacterWaiter2, kCarRestaurant, position);

			getSavePoints()->push(kCharacterWaiter2, entity, kCharacterAction136455232);

			setCallback(2);
			setup_callSavepoint(seq2, entity, kCharacterActionDrawTablesWithChairs, seq3);
			break;

		case 2:
			if (position)
				getEntities()->updatePositionExit(kCharacterWaiter2, kCarRestaurant, position);

			setCallback(3);
			setup_draw(seq4);
			break;

		case 3:
			getData()->entityPosition = kPosition_5900;
			getEntities()->clearSequences(kCharacterWaiter2);
			*parameter = 0;

			if (parameter2 != nullptr)
				*parameter2 = 0;

			callbackAction();
			break;
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
void Waiter2::serveSalon(const SavePoint &savepoint, const char *seq1, const char *snd1, CharacterIndex entity, const char *snd2, const char *seq2, CharacterActions action, const char *seq3, uint *parameter) {
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5800;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_draw("816DD");
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getEntities()->drawSequenceRight(kCharacterWaiter2, seq1);

			if (getEntities()->isInRestaurant(kCharacterCath))
				getEntities()->updateFrame(kCharacterWaiter2);

			if (strcmp(snd1, ""))
				getSound()->playSound(kCharacterWaiter2, snd1);

			setCallback(2);
			setup_callbackActionOnDirection();
			break;

		case 2:
			getSavePoints()->push(kCharacterWaiter2, entity, kCharacterAction122358304);

			getSound()->playSound(kCharacterWaiter2, snd2);

			setCallback(3);
			// the last arg is actually a constant varying between calls,
			// but this function already has too many args to add yet another one
			setup_updatePosition(seq2, kCarRestaurant, strcmp(seq2, "127D") ? 57 : 56);
			break;

		case 3:
			getSavePoints()->push(kCharacterWaiter2, entity, action);

			setCallback(4);
			setup_draw(seq3);
			break;

		case 4:
			getEntities()->drawSequenceRight(kCharacterWaiter2, "816UD");

			if (getEntities()->isInSalon(kCharacterCath))
				getEntities()->updateFrame(kCharacterWaiter2);

			setCallback(5);
			setup_callbackActionOnDirection();
			break;

		case 5:
			getEntities()->clearSequences(kCharacterWaiter2);
			getData()->entityPosition = kPosition_5900;
			*parameter = 0;

			callbackAction();
			break;
		}
		break;
	}
}

} // End of namespace LastExpress
