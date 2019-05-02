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

#include "lastexpress/entities/yasmin.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Yasmin::Yasmin(LastExpressEngine *engine) : Entity(engine, kEntityYasmin) {
	ADD_CALLBACK_FUNCTION(Yasmin, reset);
	ADD_CALLBACK_FUNCTION_SI(Yasmin, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_S(Yasmin, playSound);
	ADD_CALLBACK_FUNCTION_I(Yasmin, updateFromTime);
	ADD_CALLBACK_FUNCTION_II(Yasmin, updateEntity);
	ADD_CALLBACK_FUNCTION(Yasmin, goEtoG);
	ADD_CALLBACK_FUNCTION(Yasmin, goGtoE);
	ADD_CALLBACK_FUNCTION(Yasmin, chapter1);
	ADD_CALLBACK_FUNCTION(Yasmin, part1);
	ADD_CALLBACK_FUNCTION(Yasmin, function10);
	ADD_CALLBACK_FUNCTION(Yasmin, chapter2);
	ADD_CALLBACK_FUNCTION(Yasmin, part2);
	ADD_CALLBACK_FUNCTION(Yasmin, chapter3);
	ADD_CALLBACK_FUNCTION(Yasmin, part3);
	ADD_CALLBACK_FUNCTION(Yasmin, chapter4);
	ADD_CALLBACK_FUNCTION(Yasmin, part4);
	ADD_CALLBACK_FUNCTION(Yasmin, function17);
	ADD_CALLBACK_FUNCTION(Yasmin, chapter5);
	ADD_CALLBACK_FUNCTION(Yasmin, part5);
	ADD_CALLBACK_FUNCTION(Yasmin, function20);
	ADD_CALLBACK_FUNCTION(Yasmin, hiding);
	ADD_NULL_FUNCTION();
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Yasmin, reset)
	switch (savepoint.action) {
	default:
		break;

	case kActionExcuseMeCath:
		getSound()->excuseMeCath();
		break;

	case kActionExcuseMe:
		getSound()->excuseMe(kEntityYasmin);
		break;
	}

	// Process default actions
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(2, Yasmin, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(3, Yasmin, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(4, Yasmin, updateFromTime)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(5, Yasmin, updateEntity, CarIndex, EntityPosition)
	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Yasmin, goEtoG)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4840;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_enterExitCompartment("615Be", kObjectCompartment5);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_3050);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment("615Ag", kObjectCompartment7);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityYasmin);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Yasmin, goGtoE)
	switch (savepoint.action) {
	default:
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationOutsideCompartment;

		setCallback(1);
		setup_enterExitCompartment("615Bg", kObjectCompartment7);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_4840);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment("615Ae", kObjectCompartment5);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kEntityYasmin);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Yasmin, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Yasmin, setup_part1));
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_4840;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Yasmin, part1)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheckCallback(kTime1093500, params->param1, 1, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG)))
			break;

		if (Entity::timeCheckCallback(kTime1161000, params->param2, 3, WRAP_SETUP_FUNCTION(Yasmin, setup_goGtoE)))
			break;

		if (Entity::timeCheckPlaySoundUpdatePosition(kTime1162800, params->param3, 4, "Har1102", kPosition_4070))
			break;

		if (Entity::timeCheckCallback(kTime1165500, params->param4, 5, "Har1104", WRAP_SETUP_FUNCTION_S(Yasmin, setup_playSound)))
			break;

		if (Entity::timeCheckCallback(kTime1174500, params->param5, 6, "Har1106", WRAP_SETUP_FUNCTION_S(Yasmin, setup_playSound)))
			break;

		Entity::timeCheckCallback(kTime1183500, params->param6, 7, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG));
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_2740;
			setCallback(2);
			setup_playSound("Har1102");
			break;

		case 2:
			if (Entity::timeCheckCallback(kTime1161000, params->param2, 3, WRAP_SETUP_FUNCTION(Yasmin, setup_goGtoE)))
				break;
			// fall through

		case 3:
			if (Entity::timeCheckPlaySoundUpdatePosition(kTime1162800, params->param3, 4, "Har1102", kPosition_4070))
				break;
			// fall through

		case 4:
			if (Entity::timeCheckCallback(kTime1165500, params->param4, 5, "Har1104", WRAP_SETUP_FUNCTION_S(Yasmin, setup_playSound)))
				break;
			// fall through

		case 5:
			if (Entity::timeCheckCallback(kTime1174500, params->param5, 6, "Har1106", WRAP_SETUP_FUNCTION_S(Yasmin, setup_playSound)))
				break;
			// fall through

		case 6:
			Entity::timeCheckCallback(kTime1183500, params->param6, 7, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG));
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Yasmin, function10)
	if (savepoint.action == kActionDefault) {
		getObjects()->update(kObjectCompartment7, kEntityPlayer, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getEntities()->clearSequences(kEntityYasmin);
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Yasmin, chapter2)
	if (savepoint.action == kActionDefault) {
		getEntities()->clearSequences(kEntityYasmin);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		setup_part2();
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Yasmin, part2)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheckCallback(kTime1759500, params->param1, 1, WRAP_SETUP_FUNCTION(Yasmin, setup_goGtoE)))
			break;

		if (getState()->time > kTime1800000 && !params->param2) {
			params->param2 = 1;
			getData()->entityPosition = kPosition_4070;

			getSavePoints()->push(kEntityYasmin, kEntityTrain, kAction191070912, kPosition_4070);
		}
		break;

	case kActionCallback:

		if (getCallback() != 1)
			break;

		if (getState()->time > kTime1800000 && !params->param2) {
			params->param2 = 1;
			getData()->entityPosition = kPosition_4070;

			getSavePoints()->push(kEntityYasmin, kEntityTrain, kAction191070912, kPosition_4070);
		}

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Yasmin, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_part3();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityYasmin);

		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Yasmin, part3)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheckCallback(kTime2062800, params->param1, 1, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG)))
			break;

		if (Entity::timeCheckCallback(kTime2106000, params->param2, 2, WRAP_SETUP_FUNCTION(Yasmin, setup_goGtoE)))
			break;

		Entity::timeCheckCallback(kTime2160000, params->param3, 3, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG));
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			if (Entity::timeCheckCallback(kTime2106000, params->param2, 2, WRAP_SETUP_FUNCTION(Yasmin, setup_goGtoE)))
				break;
			// fall through

		case 2:
			Entity::timeCheckCallback(kTime2160000, params->param3, 3, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG));
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Yasmin, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_part4();
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_3050;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Yasmin, part4)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (Entity::timeCheckCallback(kTime2457000, params->param1, 1, WRAP_SETUP_FUNCTION(Yasmin, setup_goGtoE)))
			break;

		Entity::timeCheckCallback(kTime2479500, params->param2, 3, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG));
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->entityPosition = kPosition_4070;
			setCallback(2);
			setup_playSound("Har1110");
			break;

		case 2:
			Entity::timeCheckCallback(kTime2479500, params->param2, 3, WRAP_SETUP_FUNCTION(Yasmin, setup_goEtoG));
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Yasmin, function17)
	// Same as existing function 10 ?
	function10(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Yasmin, chapter5)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		setup_part5();
		break;

	case kActionDefault:
		getEntities()->clearSequences(kEntityYasmin);

		getData()->entityPosition = kPosition_3969;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Yasmin, part5)
	if (savepoint.action == kActionProceedChapter5)
		setup_function20();
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Yasmin, function20)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!Entity::updateParameter(params->param1, getState()->time, 2700))
			break;

		setup_hiding();
		break;

	case kActionDefault:
		getData()->entityPosition = kPosition_2500;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;
		break;

	case kActionDrawScene:
		if (getEntities()->isInsideTrainCar(kEntityPlayer, kCarGreenSleeping)) {
			setup_hiding();
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(21, Yasmin, hiding)
	switch (savepoint.action) {
	default:
		break;

	case kActionNone:
		if (!getSoundQueue()->isBuffered(kEntityYasmin)) {
			if (Entity::updateParameter(params->param1, getState()->timeTicks, 450)) {
				getSound()->playSound(kEntityYasmin, "Har5001");
				params->param1 = 0;
			}
		}
		break;

	case kActionDefault:
		setCallback(1);
		setup_updateEntity(kCarGreenSleeping, kPosition_4840);
		break;

	case kActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			setCallback(2);
			setup_enterExitCompartment("615BE", kObjectCompartment5);
			break;

		case 2:
			getEntities()->clearSequences(kEntityYasmin);
			getData()->location = kLocationInsideCompartment;
			getData()->entityPosition = kPosition_3050;
			getObjects()->update(kObjectCompartment7, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
			getSound()->playSound(kEntityYasmin, "Har5001");
			break;
		}
		break;

	case kAction135800432:
		setup_nullfunction();
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_NULL_FUNCTION(22, Yasmin)

} // End of namespace LastExpress
