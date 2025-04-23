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

#include "lastexpress/entities/cooks.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"

namespace LastExpress {

Cooks::Cooks(LastExpressEngine *engine) : Entity(engine, kCharacterCook) {
	ADD_CALLBACK_FUNCTION_S(Cooks, draw);
	ADD_CALLBACK_FUNCTION_S(Cooks, playSound);
	ADD_CALLBACK_FUNCTION(Cooks, uptrainVersion);
	ADD_CALLBACK_FUNCTION(Cooks, downtrainVersion);
	ADD_CALLBACK_FUNCTION(Cooks, chapter1);
	ADD_CALLBACK_FUNCTION(Cooks, inKitchenDinner);
	ADD_CALLBACK_FUNCTION(Cooks, lockUp);
	ADD_CALLBACK_FUNCTION(Cooks, chapter2);
	ADD_CALLBACK_FUNCTION(Cooks, inKitchenBreakfast);
	ADD_CALLBACK_FUNCTION(Cooks, chapter3);
	ADD_CALLBACK_FUNCTION(Cooks, inKitchenLunch);
	ADD_CALLBACK_FUNCTION(Cooks, chapter4);
	ADD_CALLBACK_FUNCTION(Cooks, inKitchenDinner2);
	ADD_CALLBACK_FUNCTION(Cooks, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(1, Cooks, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(2, Cooks, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(3, Cooks, uptrainVersion)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterCook, "308A");
		getEntities()->updatePositionEnter(kCharacterCook, kCarRestaurant, 75);
		getEntities()->updatePositionEnter(kCharacterCook, kCarRestaurant, 78);

		switch (getProgress().chapter) {
		default:
			getSound()->playSound(kCharacterCook, "KIT1011");
			setCallback(3);
			setup_draw("308B");
			break;

		case kChapter1:
			setCallback(1);
			setup_playSound("KIT1010");
			break;

		case kChapter3:
			setCallback(2);
			setup_playSound("KIT1012");
			break;
		}
		break;

	case kCharacterActionDrawScene:
		if (!getEntities()->isInKitchen(kCharacterCath)) {
			getEntities()->clearSequences(kCharacterCook);
			callbackAction();
			break;
		}

		if (getEntities()->isPlayerPosition(kCarRestaurant, 76)) {
			getEntities()->drawSequenceLeft(kCharacterCook, "308D");

			if (!getSoundQueue()->isBuffered(kCharacterCook) && !params->param1) {
				// Kitchen apprentice getting a lesson :D
				getSound()->playSound(kCharacterCook, "KIT1011A");
				params->param1 = 1;
			}
		}

		if (params->param1 && !getEntities()->hasValidFrame(kCharacterCook) && !getSoundQueue()->isBuffered(kCharacterCook)) {
			getSound()->playSound(kCharacterCook, "LIB015");
			getEntities()->clearSequences(kCharacterCook);
			callbackAction();
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			getSound()->playSound(kCharacterCook, "KIT1011");
			setCallback(3);
			setup_draw("308B");
			break;

		case 3:
			getEntities()->drawSequenceLeft(kCharacterCook, "308C");
			getEntities()->updatePositionExit(kCharacterCook, kCarRestaurant, 75);
			getEntities()->updatePositionExit(kCharacterCook, kCarRestaurant, 78);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(4, Cooks, downtrainVersion)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceLeft(kCharacterCook, "308A");
		getEntities()->updatePositionEnter(kCharacterCook, kCarRestaurant, 75);
		getEntities()->updatePositionEnter(kCharacterCook, kCarRestaurant, 78);

		switch (getProgress().chapter) {
		default:
			getSound()->playSound(kCharacterCook, "KIT1011");
			setCallback(3);
			setup_draw("308B");
			break;

		case kChapter1:
			setCallback(1);
			setup_playSound("KIT1010");
			break;

		case kChapter3:
			setCallback(2);
			setup_playSound("KIT1012");
			break;
		}
		break;

	case kCharacterActionDrawScene:
		if (!getEntities()->isInKitchen(kCharacterCath)) {
			getEntities()->clearSequences(kCharacterCook);
			callbackAction();
			break;
		}

		if (getEntities()->isPlayerPosition(kCarRestaurant, 80)) {
			getEntities()->drawSequenceLeft(kCharacterCook, "308D");

			if (!getSoundQueue()->isBuffered(kCharacterCook) && !params->param1) {
				// Kitchen apprentice getting a lesson :D
				getSound()->playSound(kCharacterCook, "KIT1011A");
				params->param1 = 1;
			}
		}

		if (params->param1 && !getEntities()->hasValidFrame(kCharacterCook) && !getSoundQueue()->isBuffered(kCharacterCook)) {
			getSound()->playSound(kCharacterCook, "LIB015");
			getEntities()->clearSequences(kCharacterCook);
			callbackAction();
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
		case 2:
			getSound()->playSound(kCharacterCook, "KIT1011");
			setCallback(3);
			setup_draw("308B");
			break;

		case 3:
			getEntities()->drawSequenceLeft(kCharacterCook, "308C");
			getEntities()->updatePositionExit(kCharacterCook, kCarRestaurant, 75);
			getEntities()->updatePositionExit(kCharacterCook, kCarRestaurant, 78);
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(5, Cooks, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Cooks, setup_inKitchenDinner));
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;

		getProgress().field_4C = 0;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(6, Cooks, inKitchenDinner)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param4, getState()->time, params->param2))
			break;

		// Broken plate sound
		getSound()->playSound(kCharacterCath, "LIB122",  getSound()->getSoundFlag(kCharacterCook));
		params->param2 = 225 * (4 * rnd(30) + 120);
		params->param4 = 0;
		break;

	case kCharacterActionDefault:
		params->param1 = 1;
		params->param2 = 225 * (4 * rnd(30) + 120);
		break;

	case kCharacterActionDrawScene:
		if (!getEntities()->isInKitchen(kCharacterCath))
			break;

		if (params->param1) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 73)) {
				setCallback(1);
				setup_uptrainVersion();
			}
		} else {
			if (params->param3) {
				setCallback(2);
				setup_playSound("ZFX1011");
			} else {
				setCallback(3);
				setup_playSound("ZFX1012");
			}
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 0;
			break;

		case 2:
		case 3:
			params->param3 = !params->param3;
			break;
		}
		break;

	case kCharacterAction101632192:
		setup_lockUp();
		break;

	case kCharacterAction224849280:
		getProgress().field_4C = 1;
		params->param1 = 1;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(7, Cooks, lockUp)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		// Snoring...
		setCallback(1);
		setup_playSound("WAT1200");
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_3650;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;

		getEntities()->clearSequences(kCharacterCook);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(8, Cooks, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inKitchenBreakfast();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterCook);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getProgress().field_4C = 1;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(9, Cooks, inKitchenBreakfast)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param3, getState()->time, params->param1))
			break;

		// Broken plate sound
		getSound()->playSound(kCharacterCath, "LIB122",  getSound()->getSoundFlag(kCharacterCook));
		params->param1 = 225 * (4 * rnd(30) + 120);
		params->param3 = 0;
		break;

	case kCharacterActionDefault:
		params->param1 = 225 * (4 * rnd(30) + 120);
		break;

	case kCharacterActionDrawScene:
		if (!getEntities()->isInKitchen(kCharacterCath))
			break;

		if (params->param2) {
			setCallback(1);
			setup_playSound("ZFX1011");
		} else {
			setCallback(2);
			setup_playSound("ZFX1012");
		}
		break;

	case kCharacterActionCallback:
		if (getCallback() == 1 || getCallback() == 2)
			params->param2 = !params->param2;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(10, Cooks, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inKitchenLunch();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterCook);

		getData()->entityPosition = kPosition_5900;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone; // not in the original version, but it does no harm, I suppose?

		getProgress().field_4C = 0;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Cooks, inKitchenLunch)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (Entity::updateParameter(params->param4, getState()->time, params->param2)) {
			// Broken plate sound
			getSound()->playSound(kCharacterCath, "LIB122",  getSound()->getSoundFlag(kCharacterCook));
			params->param2 = 225 * (4 * rnd(30) + 120);
			params->param4 = 0;
		}

		if (getState()->time > kTime2079000 && !params->param5) {
			params->param1 = 0;
			params->param5 = 1;
		}
		break;

	case kCharacterActionDefault:
		params->param1 = 1;
		params->param2 = 225 * (4 * rnd(30) + 120);
		break;

	case kCharacterActionDrawScene:
		if (!getEntities()->isInKitchen(kCharacterCath))
			break;

		if (params->param1) {
			if (getEntities()->isPlayerPosition(kCarRestaurant, 80)) {
				setCallback(1);
				setup_downtrainVersion();
			}
		} else {
			if (params->param3) {
				setCallback(2);
				setup_playSound("ZFX1011");
			} else {
				setCallback(3);
				setup_playSound("ZFX1012");
			}
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			params->param1 = 0;
			break;

		case 2:
		case 3:
			params->param3 = !params->param3;
			break;
		}
		break;

	case kCharacterAction236976550:
		getProgress().field_4C = 1;
		break;

	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(12, Cooks, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_inKitchenDinner2();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterCook);

		getData()->entityPosition = kPosition_5900;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarRestaurant;
		getData()->inventoryItem = kItemNone;

		getProgress().field_4C = 1;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Cooks, inKitchenDinner2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param3, getState()->time, params->param1))
			break;

		// Broken plate sound
		getSound()->playSound(kCharacterCath, "LIB122",  getSound()->getSoundFlag(kCharacterCook));
		params->param1 = 225 * (4 * rnd(30) + 120);
		params->param3 = 0;
		break;

	case kCharacterActionDefault:
		params->param1 = 225 * (4 * rnd(30) + 120);
		break;

	case kCharacterActionDrawScene:
		if (!getEntities()->isInKitchen(kCharacterCath))
			break;

		// Kitchen background sound
		if (params->param2) {
			setCallback(1);
			setup_playSound("ZFX1011");
		} else {
			setCallback(2);
			setup_playSound("ZFX1012");
		}
		break;

	case kCharacterActionCallback:
		// Play the next part of background sound
		if (getCallback() == 1 || getCallback() == 2) {
			params->param2 = !params->param2;
		}
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Cooks, chapter5)
	if (savepoint.action == kCharacterActionDefault)
		getEntities()->clearSequences(kCharacterCook);
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
