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

#include "lastexpress/data/scene.h"

#include "lastexpress/entities/mahmud.h"

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

Mahmud::Mahmud(LastExpressEngine *engine) : Entity(engine, kCharacterMahmud) {
	ADD_CALLBACK_FUNCTION(Mahmud, reset);
	ADD_CALLBACK_FUNCTION_S(Mahmud, draw);
	ADD_CALLBACK_FUNCTION_SI(Mahmud, enterExitCompartment);
	ADD_CALLBACK_FUNCTION_SIII(Mahmud, enterExitCompartment2);
	ADD_CALLBACK_FUNCTION_S(Mahmud, playSound);
	ADD_CALLBACK_FUNCTION_S(Mahmud, playSoundMertens);
	ADD_CALLBACK_FUNCTION_I(Mahmud, updateFromTime);
	ADD_CALLBACK_FUNCTION_II(Mahmud, savegame);
	ADD_CALLBACK_FUNCTION_II(Mahmud, updateEntity);
	ADD_CALLBACK_FUNCTION_II(Mahmud, function10);
	ADD_CALLBACK_FUNCTION(Mahmud, function11);
	ADD_CALLBACK_FUNCTION(Mahmud, function12);
	ADD_CALLBACK_FUNCTION(Mahmud, function13);
	ADD_CALLBACK_FUNCTION(Mahmud, chaptersHandler);
	ADD_CALLBACK_FUNCTION(Mahmud, chapter1);
	ADD_CALLBACK_FUNCTION(Mahmud, resetChapter);
	ADD_CALLBACK_FUNCTION(Mahmud, chapter2);
	ADD_CALLBACK_FUNCTION(Mahmud, chapter3);
	ADD_CALLBACK_FUNCTION(Mahmud, chapter4);
	ADD_CALLBACK_FUNCTION(Mahmud, chapter5);
}

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(1, Mahmud, reset)
	Entity::reset(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(2, Mahmud, draw)
	Entity::draw(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SI(3, Mahmud, enterExitCompartment, ObjectIndex)
	Entity::enterExitCompartment(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_SIII(4, Mahmud, enterExitCompartment2, ObjectIndex, uint32, ObjectIndex)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param7, getState()->timeTicks, params->param5))
			break;

		if (!getScenes()->checkPosition(kSceneNone, SceneManager::kCheckPositionLookingUp))
			getScenes()->loadSceneFromObject((ObjectIndex)params->param6, true);
		break;

	case kCharacterActionExitCompartment:
		getEntities()->exitCompartment(kCharacterMahmud, (ObjectIndex)params->param4);

		callbackAction();
		break;

	case kCharacterActionDefault:
		getEntities()->drawSequenceRight(kCharacterMahmud, (char *)&params->seq);
		getEntities()->enterCompartment(kCharacterMahmud, (ObjectIndex)params->param4);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(5, Mahmud, playSound)
	Entity::playSound(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_S(6, Mahmud, playSoundMertens)
	Entity::playSound(savepoint, false, getSound()->getSoundFlag(kCharacterCond1));
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_NOSETUP(7, Mahmud, updateFromTime)
	Entity::updateFromTime(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(8, Mahmud, savegame, SavegameType, uint32)
	Entity::savegame(savepoint);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(9, Mahmud, updateEntity, CarIndex, EntityPosition)
	if (savepoint.action == kCharacterActionExcuseMeCath) {
		if (getInventory()->hasItem(kItemPassengerList))
			getSound()->playSound(kCharacterCath, rnd(2) ? "CAT1025" : "CAT1025A");
		else
			getSound()->excuseMeCath();

		return;
	}

	Entity::updateEntity(savepoint, true);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION_II(10, Mahmud, function10, ObjectIndex, bool)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (!Entity::updateParameter(params->param6, getState()->time, 13500))
			break;

		getObjects()->update(kObjectCompartment5, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment6, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment7, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment8, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);

		setCallback(2);
		setup_enterExitCompartment("614Ed", kObjectCompartment4);
		break;

	case kCharacterActionEndSound:
	case kCharacterActionDrawScene:
		if (!getSoundQueue()->isBuffered(kCharacterMahmud)) {
			EntityPosition position = getEntityData(kCharacterCath)->entityPosition;
			if (position < kPosition_1500 || position >= kPosition_5790 || (position > kPosition_4455 && params->param5 != 5)) {
				getObjects()->update(kObjectCompartment5, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
				getObjects()->update(kObjectCompartment6, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
				getObjects()->update(kObjectCompartment7, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
				getObjects()->update(kObjectCompartment8, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);

				setCallback(3);
				setup_enterExitCompartment("614Ed", kObjectCompartment4);
			}
		}
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (!getSoundQueue()->isBuffered((savepoint.action == kCharacterActionKnock) ? "LIB012" : "LIB013", true))
			getSound()->playSound(kCharacterCath, (savepoint.action == kCharacterActionKnock) ? "LIB012" : "LIB013");

		params->param5 = savepoint.param.intValue;

		if (!getSoundQueue()->isBuffered(kCharacterMahmud)) {
			params->param3++;

			switch(params->param3) {
			default:
				params->param4 = 1;
				break;

			case 1:
				getSound()->playSound(kCharacterMahmud, params->param2 ? "MAH1170E" : "MAH1173A");
				break;

			case 2:
				getSound()->playSound(kCharacterMahmud, "MAH1173B");
				break;

			case 3:
				getSound()->playSound(kCharacterMahmud, "MAH1174");
				break;
			}
		}

		if (params->param4) {
			if (getState()->time >= kTimeCityGalanta) {
				params->param3 = 0;
			} else {
				getSound()->playSound(kCharacterClerk, "LIB050", kVolumeFull);
				getLogic()->gameOver(kSavegameTypeIndex, 0, (getProgress().chapter == kChapter1) ? kSceneGameOverPolice1 : kSceneGameOverPolice2, true);
			}
			break;
		}

		getActionOld()->handleOtherCompartment((ObjectIndex)savepoint.param.intValue, false, false);

		switch (getScenes()->get(getState()->scene)->position) {
		default:
			break;

		case 55:
			getScenes()->loadSceneFromObject(kObjectCompartment5, true);
			break;

		case 56:
			getScenes()->loadSceneFromObject(kObjectCompartment6, true);
			break;

		case 57:
			getScenes()->loadSceneFromObject(kObjectCompartment7, true);
			break;

		case 58:
			getScenes()->loadSceneFromObject(kObjectCompartment8, true);
			break;
		}
		break;

	case kCharacterActionDefault:
		getSound()->playSound(kCharacterMahmud, params->param2 ? "MAH1170A" : "MAH1173", kSoundVolumeEntityDefault, 45);
		getProgress().field_C4 = 1;

		setCallback(1);
		setup_enterExitCompartment2("614Dd", kObjectCompartment4, 30, (ObjectIndex)params->param1);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getObjects()->update(kObjectCompartment5, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment6, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment7, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment8, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);

			getData()->location = kLocationOutsideCompartment;

			getEntities()->drawSequenceLeft(kCharacterMahmud, "614Md");
			getEntities()->enterCompartment(kCharacterMahmud, kObjectCompartment4, true);
			break;

		case 2:
		case 3:
			getEntities()->exitCompartment(kCharacterMahmud, kObjectCompartment4, true);
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMahmud);

			callbackAction();
			break;
		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(11, Mahmud, function11)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor: {
		getSound()->playSound(kCharacterCath, (savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013"));

		if (!getSoundQueue()->isBuffered(kCharacterMahmud)) {
			params->param1++;

			getSound()->playSound(kCharacterMahmud, (params->param1 == 1 ? "MAH1170E" : (params->param1 == 2 ? "MAH1173B" : "MAH1174")));
		}

		switch (getScenes()->get(getState()->scene)->position) {
		default:
			break;

		case 55:
			getScenes()->loadSceneFromObject(kObjectCompartment5, true);
			break;

		case 56:
			getScenes()->loadSceneFromObject(kObjectCompartment6, true);
			break;

		case 57:
			getScenes()->loadSceneFromObject(kObjectCompartment7, true);
			break;

		case 58:
			getScenes()->loadSceneFromObject(kObjectCompartment8, true);
			break;
		}
		break;
		}

	case kCharacterActionDefault:
		getSavePoints()->push(kCharacterMahmud, kCharacterCond1, kCharacterAction102227384);
		setCallback(1);
		setup_enterExitCompartment("614Ad", kObjectCompartment4);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartment4, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getEntities()->drawSequenceLeft(kCharacterMahmud, "614Kd");
			getEntities()->enterCompartment(kCharacterMahmud, kObjectCompartment4, true);

			setCallback(2);
			setup_playSound("MAH1170A");
			break;

		case 2:
			setCallback(3);
			setup_playSoundMertens("MAH1170B");
			break;

		case 3:
			setCallback(4);
			setup_playSound("MAH1170C");
			break;

		case 4:
			setCallback(5);
			setup_playSoundMertens("MAH1170D");
			break;

		case 5:
			setCallback(6);
			setup_playSound("MAH1170E");
			break;

		case 6:
			setCallback(7);
			setup_playSoundMertens("MAH1170F");
			break;

		case 7:
			setCallback(8);
			setup_enterExitCompartment("614Ld", kObjectCompartment4);
			break;

		case 8:
			getSavePoints()->push(kCharacterMahmud, kCharacterCond1, kCharacterAction156567128);
			getEntities()->drawSequenceLeft(kCharacterMahmud, "614Bd");
			getEntities()->enterCompartment(kCharacterMahmud, kObjectCompartment4, true);

			setCallback(9);
			setup_playSound("MAH1170G");
			break;

		case 9:
			setCallback(10);
			setup_playSoundMertens("MAH1170H");
			break;

		case 10:
			getObjects()->update(kObjectCompartment5, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment6, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment7, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			getObjects()->update(kObjectCompartment8, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			break;

		case 11:
			getEntities()->exitCompartment(kCharacterMahmud, kObjectCompartment4, true);
			getData()->location = kLocationInsideCompartment;

			getEntities()->clearSequences(kCharacterMahmud);
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);

			callbackAction();
			break;
		}
		break;

	case kCharacterAction123852928:
		if (getSoundQueue()->isBuffered(kCharacterMahmud))
			getSoundQueue()->fade(kCharacterMahmud);

		getObjects()->update(kObjectCompartment5, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment6, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment7, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObjectCompartment8, kCharacterClerk, kObjectLocation3, kCursorHandKnock, kCursorHand);

		setCallback(11);
		setup_enterExitCompartment("614Cd", kObjectCompartment4);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
// TODO: factorize code between function12 & function13
IMPLEMENT_FUNCTION(12, Mahmud, function12)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("614Gd", kObjectCompartment4);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartment4, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_4070);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment("614Ff", kObjectCompartment6);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMahmud);

			setCallback(4);
			setup_playSound("Har1105");
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("614Gf", kObjectCompartment6);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			setCallback(6);
			setup_updateEntity(kCarGreenSleeping, kPosition_5790);
			break;

		case 6:
			setCallback(7);
			setup_enterExitCompartment("614Fd", kObjectCompartment4);
			break;

		case 7:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMahmud);

			callbackAction();
			break;

		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(13, Mahmud, function13)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionDefault:
		setCallback(1);
		setup_enterExitCompartment("614Gd", kObjectCompartment4);
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			break;

		case 1:
			getData()->location = kLocationOutsideCompartment;
			getObjects()->update(kObjectCompartment4, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);

			setCallback(2);
			setup_updateEntity(kCarGreenSleeping, kPosition_2740);
			break;

		case 2:
			setCallback(3);
			setup_enterExitCompartment("614Fh", kObjectCompartment8);
			break;

		case 3:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMahmud);

			setCallback(4);
			setup_playSound("Har1107");
			break;

		case 4:
			setCallback(5);
			setup_enterExitCompartment("614Gh", kObjectCompartment8);
			break;

		case 5:
			getData()->location = kLocationOutsideCompartment;

			setCallback(6);
			setup_updateEntity(kCarGreenSleeping, kPosition_5790);
			break;

		case 6:
			setCallback(7);
			setup_enterExitCompartment("614Fd", kObjectCompartment4);
			break;

		case 7:
			getData()->location = kLocationInsideCompartment;
			getEntities()->clearSequences(kCharacterMahmud);

			callbackAction();
			break;

		}
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(14, Mahmud, chaptersHandler)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		if (ENTITY_PARAM(0, 1)) {
			params->param2 = 1;
			getSavePoints()->push(kCharacterMahmud, kCharacterCond1, kCharacterAction204379649);
			ENTITY_PARAM(0, 1) = 0;
		}

		if (!params->param2 && getProgress().chapter == kChapter1) {

			if (Entity::timeCheckCallback(kTime1098000, params->param6, 1, WRAP_SETUP_FUNCTION(Mahmud, setup_function13)))
				break;

			if (!getSoundQueue()->isBuffered("HAR1104") && getState()->time > kTime1167300 && !params->param7) {
				params->param7 = 1;

				setCallback(2);
				setup_function12();
				break;
			}
		}

		if (params->param5) {
			if (!Entity::updateParameter(params->param8, getState()->timeTicks, 75))
				break;

			params->param4 = 1;
			params->param5 = 0;

			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorNormal, kCursorNormal);
		}

		params->param8 = 0;
		break;

	case kCharacterActionKnock:
	case kCharacterActionOpenDoor:
		if (params->param5) {
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorNormal, kCursorNormal);

			if (getProgress().jacket == kJacketBlood || HELPERgetEvent(kEventMahmudWrongDoor) || HELPERgetEvent(kEventMahmudWrongDoorOriginalJacket) || HELPERgetEvent(kEventMahmudWrongDoorDay)) {
				// Check if we have the passenger list
				if (getInventory()->hasItem(kItemPassengerList)) {
					setCallback(6);
					setup_playSound(rnd(2) == 0 ? "CAT1501" : getSound()->wrongDoorCath());
				} else {
					setCallback(7);
					setup_playSound(getSound()->wrongDoorCath());
				}
			} else {
				setCallback(savepoint.action == kCharacterActionKnock ? 8 : 9);
				setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
			}
		} else {
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation1, kCursorNormal, kCursorNormal);

			setCallback(savepoint.action == kCharacterActionKnock ? 3 : 4);
			setup_playSound(savepoint.action == kCharacterActionKnock ? "LIB012" : "LIB013");
		}
		break;

	case kCharacterActionDefault:
		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;

		getEntities()->clearSequences(kCharacterMahmud);
		params->param3 = 1;

		getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;

	case kCharacterActionDrawScene:
		if (params->param4 || params->param5) {
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param4 = 0;
			params->param5 = 0;
		}
		break;

	case kCharacterActionCallback:
		switch (getCallback()) {
		default:
			return;

		case 1:
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param4 = 0;
			params->param5 = 0;

			if (!getSoundQueue()->isBuffered("HAR1104") && getState()->time > kTime1167300 && !params->param7) {
				params->param7 = 1;
				setCallback(2);
				setup_function12();
				break;
			}

			params->param8 = 0;
			break;

		case 2:
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param4 = 0;
			params->param5 = 0;
			params->param8 = 0;
			break;

		case 3:
		case 4:
			setCallback(5);
			setup_playSound("MAH1175");
			break;

		case 5: {
			CursorStyle cursor = kCursorHandKnock;
			CursorStyle cursor2 = kCursorHand;

			if (getProgress().jacket == kJacketBlood
			 || HELPERgetEvent(kEventMahmudWrongDoor)
			 || HELPERgetEvent(kEventMahmudWrongDoorOriginalJacket)
			 || HELPERgetEvent(kEventMahmudWrongDoorDay)) {
				cursor = kCursorTalk;
				cursor2 = kCursorNormal;
			}

			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation1, cursor, cursor2);
			params->param5 = 1;
			break;
			}

		case 6:
		case 7:
			params->param4 = 1;
			break;

		case 8:
		case 9:
			setCallback(10);
			setup_savegame(kSavegameTypeEvent, kEventMahmudWrongDoor);
			return;

		case 10:
			getActionOld()->playAnimation((getProgress().jacket == kJacketGreen) ? (isNightOld() ? kEventMahmudWrongDoor : kEventMahmudWrongDoorDay) : kEventMahmudWrongDoorOriginalJacket);
			getSound()->playSound(kCharacterCath, "LIB015");
			getScenes()->processScene();

			params->param4 = 1;
			break;

		case 11:
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param4 = 0;
			params->param5 = 0;
			break;

		case 12:
			getObjects()->update(kObjectCompartment4, kCharacterMahmud, kObjectLocation3, kCursorHandKnock, kCursorHand);
			params->param2 = 0;
			params->param4 = 0;
			params->param5 = 0;
			break;
		}
		break;

	case kCharacterAction225563840:
		setCallback(12);
		setup_function11();
		break;

	case kCharacterAction290410610:
		params->param3 = (params->param3 == 0) ? 1 : 0;
		setCallback(11);
		setup_function10((ObjectIndex)savepoint.param.intValue, (bool)params->param3);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(15, Mahmud, chapter1)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		Entity::timeCheck(kTimeChapter1, params->param1, WRAP_SETUP_FUNCTION(Mahmud, setup_chaptersHandler));
		break;

	case kCharacterActionDefault:
		getSavePoints()->addData(kCharacterMahmud, kCharacterAction170483072, 0);

		getData()->entityPosition = kPosition_540;
		getData()->location = kLocationOutsideCompartment;
		getData()->car = kCarGreenSleeping;

		getObjects()->update(kObjectCompartment4, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
		getObjects()->update(kObject20, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(16, Mahmud, resetChapter)
	if (savepoint.action != kCharacterActionDefault)
		return;

	getData()->entityPosition = kPosition_5790;
	getData()->location = kLocationInsideCompartment;
	getData()->car = kCarGreenSleeping;

	getObjects()->update(kObjectCompartment4, kCharacterCath, kObjectLocation3, kCursorHandKnock, kCursorHand);
	getEntities()->clearSequences(kCharacterMahmud);
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(17, Mahmud, chapter2)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chaptersHandler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMahmud);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;
		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(18, Mahmud, chapter3)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chaptersHandler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMahmud);

		getData()->entityPosition = kPosition_5790;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone;

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(19, Mahmud, chapter4)
	switch (savepoint.action) {
	default:
		break;

	case kCharacterActionNone:
		setup_chaptersHandler();
		break;

	case kCharacterActionDefault:
		getEntities()->clearSequences(kCharacterMahmud);

		getData()->entityPosition = kPosition_2740;
		getData()->location = kLocationInsideCompartment;
		getData()->car = kCarGreenSleeping;
		getData()->clothes = kClothesDefault;
		getData()->inventoryItem = kItemNone; // not in the original game, but it does no harm, I suppose?

		break;
	}
IMPLEMENT_FUNCTION_END

//////////////////////////////////////////////////////////////////////////
IMPLEMENT_FUNCTION(20, Mahmud, chapter5)
	if (savepoint.action == kCharacterActionDefault)
		getEntities()->clearSequences(kCharacterMahmud);
IMPLEMENT_FUNCTION_END

} // End of namespace LastExpress
