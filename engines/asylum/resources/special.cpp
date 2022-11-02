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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/resources/special.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/object.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/polygons.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

static const int16 actorRects[13][4] = {
	{1100,  150, 1130,  245}, { 848,  161,  884,  216},
	{ 861,  334,  884,  413}, { 600,  193,  621,  229},
	{ 805,  156,  957,  225}, { 641,  162,  791,  226},
	{ 695,  159,  808,  213}, { 925,  158, 1054,  224},
	{1158,  162, 1293,  248}, {1319,  218, 1469,  351},
	{ 430,  351,  508,  407}, { 762,  339,  864,  398},
	{ 962,  328, 1080,  373},
};

static const int32 zapPatterns[9][16] = {
	{2292, 2299, 2301, 2302, 2303, 2304, 2306, 2307, 2310, 2311, 2312, 2313, 2314},
	{2291, 2294, 2295, 2296, 2297, 2298, 2300, 2301, 2306, 2309, 2311, 2313, 2316},
	{2292, 2296, 2297, 2299, 2302, 2303, 2304, 2305, 2307, 2309, 2310},
	{2291, 2292, 2294, 2295, 2299, 2302, 2303, 2305, 2309, 2311, 2312, 2315},
	{2293, 2294, 2296, 2297, 2298, 2300, 2302, 2306, 2307, 2309, 2311, 2313, 2316},
	{2296, 2297, 2299, 2300, 2302, 2304, 2305, 2307, 2309, 2310, 2314, 2315},
	{2295, 2297, 2298, 2300, 2302, 2304, 2305, 2309, 2310, 2312, 2313, 2314},
	{2292, 2293, 2294, 2295, 2296, 2301, 2302, 2303, 2304, 2305, 2306, 2313, 2314, 2315},
	{2293, 2296, 2297, 2298, 2299, 2300, 2302, 2304, 2306, 2307, 2309, 2310, 2311}
};

Special::Special(AsylumEngine *engine) : _vm(engine) {
	// Flags
	_paletteFlag = false;
	_paletteTick1 = 0;
	_paletteTick2 = 0;

	_chapter5Counter = 0;
}

void Special::run(Object *object, ActorIndex index) {
	switch (getWorld()->chapter) {
	default:
	case kChapterNone:
		// Nothing to do
		break;

	case kChapter1:
		chapter1(object, index);
		break;

	case kChapter2:
		chapter2(object, index);
		break;

	case kChapter3:
		chapter3(object, index);
		break;

	case kChapter4:
		chapter4(object, index);
		break;

	case kChapter5:
		chapter5(object, index);
		break;

	case kChapter6:
		chapter6(object, index);
		break;

	case kChapter7:
		chapter7(object, index);
		break;

	case kChapter8:
		chapter8(object, index);
		break;

	case kChapter9:
		chapter9(object, index);
		break;

	case kChapter11:
		chapter11(object, index);
		break;

	case kChapter12:
		chapter12(object, index);
		break;

	case kChapter13:
		chapter13(object, index);
		break;
	}
}

void Special::reset(bool resetPaletteTick2) {
	_chapter5Counter = 0;
	_paletteFlag = false;
	_paletteTick1 = 0;

	if (resetPaletteTick2)
		_paletteTick2 = 0;
}

//////////////////////////////////////////////////////////////////////////
// Spec functions
//////////////////////////////////////////////////////////////////////////
void Special::chapter1(Object *object, ActorIndex actorIndex) {
	// Play chapter sound
	playChapterSound(object, actorIndex);

	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectHeadBanger:
			if (object->getFrameIndex() == 2)
				object->getFrameSoundItem(0)->resourceId = getWorld()->graphicResourceIds[rnd(254) ? 38 : 37];
			break;

		case kObjectAngelFlares:
			if (object->getFrameIndex() == 5)
				getSpeech()->playPlayer(81);
			break;

		case kObjectGuyFalls:
			if (object->getFrameIndex() == 23)
				getSpeech()->playPlayer(82);
			break;
		}
	}
}

void Special::chapter2(Object *object, ActorIndex actorIndex) {
	// Check objects
	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectOpeningGate:
			if (_vm->isGameFlagSet(kGameFlag1137)) {
				_vm->clearGameFlag(kGameFlag1137);
				Actor::crowsReturn(_vm);
			}

			if (_vm->isGameFlagSet(kGameFlag1131) && !getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
				_vm->clearGameFlag(kGameFlag219);
				_vm->setGameFlag(kGameFlag1131);
			}
			break;

		case kObjectCrow1FlysAway:
			if (rnd(75) < 3)
				checkOtherObject(object, kObjectCrow1Pecks, kGameFlag447, kGameFlag448);
			break;

		case kObjectCrow1Pecks:
			checkObject(object, kGameFlag447, kGameFlag448, getWorld()->objects[0]->getId());
			break;

		case kObjectCrow2AmbientPecks:
			checkObject(object, kGameFlag450, kGameFlag451);
			break;

		case kObjectCrow2FlysAway:
			if (rnd(50) < 3)
				checkOtherObject(object, kObjectCrow2AmbientPecks, kGameFlag450, kGameFlag451);
			break;

		case kObjectCrow3FlysAway:
			if (rnd(50) < 3)
				checkOtherObject(object, kObjectCrow3Pecks, kGameFlag452, kGameFlag453);
			break;

		case kObjectCrow3Pecks:
			checkObject(object, kGameFlag452, kGameFlag453);
			break;

		case kObjectCrow4FlysAway:
			if (rnd(50) < 3)
				checkOtherObject(object, kObjectCrow4Pecks, kGameFlag454, kGameFlag455);
			break;

		case kObjectCrow4Pecks:
			checkObject(object, kGameFlag454, kGameFlag455);
		}
	}

	// Check flags
	if (_vm->isGameFlagSet(kGameFlag371)) {
		_vm->clearGameFlag(kGameFlag371);

		for (int i = 13; i < 22; i++) {
			if (getScene()->getActor(i)->isVisible()) {
				getSharedData()->setChapter2Counter(6, 2);
				break;
			}
		}

		for (int i = 13; i < 22; i++)
			getScene()->getActor(i)->changeStatus(kActorStatusAttacking);
	}

	// Play chapter sound
	playChapterSound(object, actorIndex);

	// Check actors
	Actor *actor = getScene()->getActor(actorIndex);
	switch (actorIndex) {
	default:
		break;

	case 13:
	case 15:
	case 17:
	case 18:
		if (actor->getStatus() == kActorStatusWalking) {
			int32 counter = getSharedData()->getChapter2Counter(getCounter(actorIndex));
			counter++;

			if (counter >= 5) {
				counter = 0;
				actor->changeDirection((ActorDirection)((actor->getDirection() + 1) & 7));
			}

			getSharedData()->setChapter2Counter(getCounter(actorIndex), counter);

		} else if (actor->getStatus() == kActorStatusEnabled) {
			actor->changeStatus(kActorStatusWalking);
		}
		break;

	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
		if (actor->isVisible()) {
			// Update actor
			Actor *refActor = getScene()->getActor(actorIndex - 9); // Original uses offset to object array (+ offset to actor).

			*actor->getPoint1() = *refActor->getPoint1();
			actor->getPoint1()->y += (int16)getSharedData()->crowsData[actorIndex - 11];
			actor->setFrameIndex(refActor->getFrameIndex());
			actor->setDirection(refActor->getDirection());

			// Get the resource Id
			Actor *actor13 = getScene()->getActor(13);
			int32 direction = (actor13->getDirection() > kDirectionS) ? 8 - actor13->getDirection() : actor13->getDirection();
			ResourceId id = actor->getResourcesId((uint32)(actorIndex + direction));

			actor->setResourceId(id);
		}
		break;

	case 38:
	case 39:
		if (actor->getFrameIndex() == 9) {
			actor->getPoint1()->x = -1000;
			actor->setFrameIndex(0);
		}
		break;

	case 40:
		if (actor->getFrameIndex() == 9) {
			actor->getPoint1()->x = -1000;
			actor->setFrameIndex(0);

			if (actor->isVisible())
				if (getSharedData()->getChapter2Counter(5) < 7)
					getSpeech()->playPlayer(452);

			_vm->setGameFlag(kGameFlag219);

			actor->hide();
		}
		break;
	}
}

void Special::chapter3(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);
}

void Special::chapter4(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);

	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		break;

	case kObjectNPC033StartEnc:
		if (object->getFrameIndex() == 15)
			_vm->setGameFlag(kGameFlag387);
		break;

	case kObjectNPC033GetBook:
		if (object->getFrameIndex() == 6)
			_vm->clearGameFlag(kGameFlag387);
		break;
	}
}

void Special::chapter5(Object *object, ActorIndex actorIndex) {
	setPaletteGamma(MAKE_RESOURCE(kResourcePackShared, 59), getWorld()->currentPaletteId);

	playChapterSound(object, actorIndex);

	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		break;

	case kObjectBubbles:
		if (_vm->isGameFlagSet(kGameFlag244)) {
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				_vm->clearGameFlag(kGameFlag244);
				_vm->setGameFlag(kGameFlag245);

				Object *glow = getWorld()->getObjectById(kObjectGlow);
				glow->setNextFrame(glow->flags);
			}
		}
		break;

	case kObjectGlow:
		if (_vm->isGameFlagSet(kGameFlag245)) {
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				_vm->clearGameFlag(kGameFlag245);
				_vm->setGameFlag(kGameFlag246);

				Object *dome = getWorld()->getObjectById(kObjectDome);
				dome->setNextFrame(dome->flags);
			}
		} else {
			if (object->getFrameIndex() == 1) {
				FrameSoundItem *item = object->getFrameSoundItem(0);

				item->resourceId = kResourceNone;
				item->frameIndex = 0;
				item->index      = 0;
			}
		}
		break;

	case kObjectDome:
		if (_vm->isGameFlagSet(kGameFlag246)) {
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				_vm->clearGameFlag(kGameFlag246);
				_vm->setGameFlag(kGameFlag247);

				Object *redlight = getWorld()->getObjectById(kObjectRedLight);
				redlight->setNextFrame(redlight->flags);
			}
		}
		break;

	case kObjectRedLight:
		if (_vm->isGameFlagSet(kGameFlag247)) {
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				_vm->clearGameFlag(kGameFlag247);
				_vm->setGameFlag(kGameFlag248);

				Object *ring = getWorld()->getObjectById(kObjectRing);
				ring->setNextFrame(ring->flags);
			}
		}
		break;

	case kObjectRing:
		if (_vm->isGameFlagSet(kGameFlag248)) {
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				_vm->clearGameFlag(kGameFlag248);

				Object *ball = getWorld()->getObjectById(kObjectBallMovesUpright);
				ball->setNextFrame(ball->flags);
			}
		}
		break;

	case kObjectBallMovesUpright:
		if (object->getFrameIndex() == 7) {
			Object *gears = getWorld()->getObjectById(kObjectGearsLightUp);
			gears->setNextFrame(gears->flags);
		} else {
			if (object->getFrameIndex() == 1) {
				FrameSoundItem *item = object->getFrameSoundItem(0);

				item->resourceId = kResourceNone;
				item->frameIndex = 0;
				item->index      = 0;
			}
		}
		break;

	case kObjectGearsLightUp:
		if (object->getFrameIndex() == object->getFrameCount() - 1)
			_vm->setGameFlag(kGameFlag243);
		break;
	}
}

void Special::chapter6(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);

	if (actorIndex == 2  || actorIndex == 3) {
		Actor *actor = getScene()->getActor(actorIndex);

		getWorld()->ambientSounds[0].point.x = actor->getPoint2()->x + actor->getPoint1()->x;
		getWorld()->ambientSounds[0].point.y = actor->getPoint2()->y + actor->getPoint1()->y;
	}
}

void Special::chapter7(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);

	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectGlobe:
			if (!getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
				_vm->clearGameFlag(kGameFlag1009);
				getCursor()->show();
			}
			break;

		case kObjectFreezerHallInterior:
			Actor *player = getScene()->getActor();

			if (_vm->isGameFlagSet(kGameFlag1021)) {
				if (player->inventory[0]) {

					if (player->getStatus() == kActorStatusShowingInventory || player->getStatus() == kActorStatus10) {
						getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 2));
						player->enable();
					} else {
						getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
						player->changeStatus(kActorStatusShowingInventory);
					}

					_vm->setGameFlag(kGameFlag1023);
				} else {
					_vm->setGameFlag(kGameFlag1022);
				}

				_vm->clearGameFlag(kGameFlag1021);
			}

			if (_vm->isGameFlagSet(kGameFlag1023)) {
				if (player->inventory.getSelectedItem()) {
					int32 areaIndex = getWorld()->getActionAreaIndexById(player->inventory.getSelectedItem() == 3 ? 2447 : 2448);
					getScript()->queueScript(getWorld()->actions[areaIndex]->scriptIndex, getSharedData()->getPlayerIndex());
					_vm->clearGameFlag(kGameFlag1023);
				} else if (player->getStatus() != kActorStatusShowingInventory) {
					_vm->clearGameFlag(kGameFlag1023);
					_vm->setGameFlag(kGameFlag1022);
				}
			}

			if (_vm->isGameFlagSet(kGameFlag1022)) {
				_vm->clearGameFlag(kGameFlag1022);
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2445)]->scriptIndex,
				                         getSharedData()->getPlayerIndex());
			}
			break;
		}
	} else {
		if (_vm->isGameFlagSet(kGameFlag1108))
			return;

		Actor *actor0 = getScene()->getActor(0);
		Actor *actor1 = getScene()->getActor(1);
		Actor *actor2 = getScene()->getActor(2);

		switch (actorIndex) {
		default:
			break;

		case 1:
			if (actor0->getDirection() < 2 || actor0->getDirection() == 7) {
				actor1->hide();
				actor2->show();
			} else if (actor0->getDirection() == 2 || actor0->getDirection() == 3) {
				actor1->getPoint1()->x = actor0->getPoint1()->x;
				actor1->getPoint1()->y = actor0->getPoint1()->y - 15;
			} else if (actor0->getDirection() == 5 || actor0->getDirection() == 6) {
				actor1->getPoint1()->x = actor0->getPoint1()->x + 20;
				actor1->getPoint1()->y = actor0->getPoint1()->y - 15;
			} else {
				actor1->getPoint1()->x = actor0->getPoint1()->x + 5;
				actor1->getPoint1()->y = actor0->getPoint1()->y - 10;
			}
			break;

		case 2:
			if (actor0->getDirection() <= 2 || actor0->getDirection() >= 7) {
				actor2->getPoint1()->x = actor0->getPoint1()->x + 10;
				actor2->getPoint1()->y = actor0->getPoint1()->y - 10;
			} else {
				actor2->hide();
				actor1->show();
			}
			break;
		}
	}
}

void Special::chapter8(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);

	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectRitualLoop:
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				if (!getSound()->isPlaying(getSpeech()->getSoundResourceId()))
					_vm->setGameFlag(kGameFlag897);
			}
			break;

		case kObjectWadeThroughLava: {
			Actor *actor0 = getScene()->getActor(0);

			if (object->getFrameIndex() == 23) {
				if (_vm->isGameFlagNotSet(kGameFlag815))
					actor0->inventory.add(1, 0);

				_vm->setGameFlag(kGameFlag815);
			}

			if (object->getFrameIndex() == 50) {
				object->disableAndRemoveFromQueue();
				actor0->setDirection(kDirectionS);

				getCursor()->show();
				getWorld()->motionStatus = 1;

				getSharedData()->setFlag(kFlag1, false);

				actor0->show();
			}
			}
			break;

		case kObjectLavaBridge:
			if (getWorld()->ambientSounds[4].delta > -100)
				getWorld()->ambientSounds[4].delta -= 5;

			if (_vm->isGameFlagSet(kGameFlag937)) {
				if (getWorld()->ambientSounds[5].delta > -100)
					getWorld()->ambientSounds[5].delta -= 5;
			}
			break;

		case kObjectFlamingHeadLeftSide:
			checkFlags(kObjectGhost1, kGameFlag543, kGameFlag544, kGameFlag545, kGameFlag875, &getWorld()->field_E8610[0], &getWorld()->field_E8628[0], kGameFlag1062, &getWorld()->field_E8660[0]);
			checkFlags(_vm->isGameFlagSet(kGameFlag881) ? kObjectGhost2b : kObjectGhost2, kGameFlag816, kGameFlag817, kGameFlag818, kGameFlag876, &getWorld()->field_E8610[1], &getWorld()->field_E8628[1], kGameFlag1063, &getWorld()->field_E8660[1]);
			checkFlags(kObjectGhost3, kGameFlag819, kGameFlag820, kGameFlag821, kGameFlag877, &getWorld()->field_E8610[2], &getWorld()->field_E8628[2], kGameFlag1064, &getWorld()->field_E8660[2]);
			checkFlags(kObjectGhost4, kGameFlag822, kGameFlag823, kGameFlag824, kGameFlag878, &getWorld()->field_E8610[3], &getWorld()->field_E8628[3], kGameFlag1065, &getWorld()->field_E8660[3]);
			checkFlags(kObjectGhost5, kGameFlag825, kGameFlag826, kGameFlag827, kGameFlag879, &getWorld()->field_E8610[4], &getWorld()->field_E8628[4], kGameFlag1066, &getWorld()->field_E8660[4]);
			checkFlags(kObjectGhost6, kGameFlag828, kGameFlag829, kGameFlag830, kGameFlag880, &getWorld()->field_E8610[5], &getWorld()->field_E8628[5], kGameFlag1067, &getWorld()->field_E8660[5]);

			updateObjectFlag(kObjectGong4);
			updateObjectFlag(kObjectGong1);
			updateObjectFlag(kObjectGong5);
			updateObjectFlag(kObjectGong6);
			updateObjectFlag(kObjectGong2);
			updateObjectFlag(kObjectGong3);
			break;

		case kObjectWheel1:
		case kObjectWheel2:
		case kObjectWheel3:
		case kObjectWheel4:
		case kObjectWheel5:
		case kObjectWheel6:
		case kObjectWheel7: {
			GameFlag wheelFlag = (GameFlag)(612 + object->getId() - kObjectWheel1);
			uint32 frameIndex = object->getFrameIndex();

			if (_vm->isGameFlagSet(wheelFlag)) {
				_vm->clearGameFlag(wheelFlag);
				++frameIndex;
			}

			if (frameIndex >= 17) {
				if ((frameIndex + 1) % 4)
					++frameIndex;
			} else if (frameIndex % 4) {
				++frameIndex;
			}

			object->setFrameIndex(frameIndex >= object->getFrameCount() ? 0 : frameIndex);
			}
			break;
		}
	}

	if (_vm->isGameFlagNotSet(kGameFlag866)) {
		// Check wheel objects frame index
		if (getWorld()->wheels[0]->getFrameIndex() == 0
		 && getWorld()->wheels[1]->getFrameIndex() == 4
		 && getWorld()->wheels[2]->getFrameIndex() == 8
		 && getWorld()->wheels[3]->getFrameIndex() == 12
		 && getWorld()->wheels[4]->getFrameIndex() == 16
		 && getWorld()->wheels[5]->getFrameIndex() == 23
		 && getWorld()->wheels[6]->getFrameIndex() == 27) {
			_vm->setGameFlag(kGameFlag866);

			getSound()->playSound(getWorld()->soundResourceIds[6], false, Config.sfxVolume - 10);
		}
	}
}

void Special::chapter9(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);

	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		break;

	case kObjectBodySlides1:
	case kObjectBodySlides2:
	case kObjectBodySlides3:
	case kObjectBodySlides4:
	case kObjectBodySlides5:
	case kObjectBodySlides6:
		if (object->getFrameIndex() == 3) {
			int counter = 0;

			for (int i = 0; i < 6; ++i)
				if (_vm->isGameFlagSet((GameFlag)(kGameFlag776 + i)))
					++counter;

			switch (counter) {
			default:
				break;

			case 0:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2524)]->scriptIndex, kActorMax);
				break;

			case 1:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2518)]->scriptIndex, kActorMax);
				break;

			case 2:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2519)]->scriptIndex, kActorMax);
				break;

			case 3:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2520)]->scriptIndex, kActorMax);
				break;

			case 4:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2521)]->scriptIndex, kActorMax);
				break;

			case 5:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2522)]->scriptIndex, kActorMax);
				break;

			case 6:
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2523)]->scriptIndex, kActorMax);
				break;
			}
		}
		break;
	}
}

void Special::chapter11(Object *object, ActorIndex actorIndex) {
	Actor *actor0 = getScene()->getActor(0), *actor1 = getScene()->getActor(1);
	Actor *player = getScene()->getActor();
	Common::Point sum;

	playChapterSound(object, actorIndex);

	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			return;

		case kObjectABarrier:
			if (actor0->isVisible() && _vm->isGameFlagSet(kGameFlag708)) {
				if (!getSound()->isPlaying(getSpeech()->getSoundResourceId()))
					_vm->clearGameFlag(kGameFlag219);
				if (!getWorld()->field_E848C && !getSound()->isPlaying(getWorld()->soundResourceIds[3]))
					getSound()->playSound(getWorld()->soundResourceIds[3], false, Config.sfxVolume);
				if (getWorld()->field_E848C == 1 && !getSound()->isPlaying(getWorld()->soundResourceIds[4]))
					getSound()->playSound(getWorld()->soundResourceIds[4], false, Config.sfxVolume);
				if (getWorld()->field_E848C == 2 && !getSound()->isPlaying(getWorld()->soundResourceIds[5]))
					getSound()->playSound(getWorld()->soundResourceIds[5], false, Config.sfxVolume);
			}

			if (_vm->isGameFlagNotSet(kGameFlag1099)) {
				_vm->setGameFlag(kGameFlag1099);
				getScene()->getActor(9)->inventory[0] = 1;
				getScene()->getActor(9)->inventory[1] = 2;
				getScene()->getActor(9)->inventory[2] = 3;
			}

			if (_vm->isGameFlagSet(kGameFlag561) && _vm->isGameFlagNotSet(kGameFlag562)) {
				ActorStatus playerStatus = player->getStatus();
				if (playerStatus != kActorStatusGettingHurt && playerStatus != kActorStatusRestarting && playerStatus != kActorStatusAttacking && playerStatus != kActorStatusWalkingTo2)
					actor0->changeStatus(kActorStatusAttacking);
				_vm->clearGameFlag(kGameFlag561);
			}

			if (actor1->getTickCount() != -1 && (uint32)actor1->getTickCount() < _vm->getTick()) {
				actor1->setTickCount(-1);
				actor1->show();
				actor1->getPoint1()->x = actor0->getPoint2()->x + actor0->getPoint1()->x - actor1->getPoint2()->x;
				actor1->getPoint1()->y = actor0->getPoint2()->y + actor0->getPoint1()->y - actor1->getPoint2()->y;
				actor1->changeStatus(kActorStatusWalking2);
			}

			tentacle(10, kGameFlag557, kGameFlag558, kGameFlag563, &actorRects[0]);
			tentacle(11, kGameFlag722, kGameFlag723, kGameFlag724, &actorRects[1]);
			tentacle(12, kGameFlag725, kGameFlag726, kGameFlag727, &actorRects[2]);
			tentacle(13, kGameFlag728, kGameFlag729, kGameFlag730, &actorRects[3]);
			rock(2, kGameFlag597, kGameFlag598, kGameFlag599, kGameFlag600, &actorRects[6]);
			rock(3, kGameFlag684, kGameFlag685, kGameFlag686, kGameFlag687, &actorRects[7]);
			rock(4, kGameFlag688, kGameFlag689, kGameFlag690, kGameFlag691, &actorRects[8]);
			rock(5, kGameFlag692, kGameFlag693, kGameFlag694, kGameFlag695, &actorRects[9]);
			rock(6, kGameFlag696, kGameFlag697, kGameFlag698, kGameFlag699, &actorRects[10]);
			rock(7, kGameFlag700, kGameFlag701, kGameFlag702, kGameFlag703, &actorRects[11]);
			rock(8, kGameFlag704, kGameFlag705, kGameFlag706, kGameFlag707, &actorRects[12]);
			rock(16, kGameFlag1054, kGameFlag1055, kGameFlag1056, kGameFlag1057, &actorRects[4]);
			rock(17, kGameFlag1058, kGameFlag1059, kGameFlag1060, kGameFlag1061, &actorRects[5]);
			break;

		case kObjectMonsterHurt:
			object->setFrameIndex(object->getFrameIndex() + 1);
			if (object->getFrameIndex() == 4) {
				if (getWorld()->field_E8518 <= 2) {
					object->setFrameIndex(0);
					_vm->clearGameFlag(kGameFlag582);
					_vm->setGameFlag(kGameFlag565);
				} else {
					_vm->clearGameFlag(kGameFlag582);
					_vm->setGameFlag(kGameFlag566);
				}
			}
			break;

		case kObjectMonsterDeath:
			if (object->getFrameIndex() == object->getFrameCount() - 1) {
				_vm->clearGameFlag(kGameFlag566);
				_vm->setGameFlag(kGameFlag596);
			} else {
				if (object->getFrameIndex() == 40) {
					_vm->setGameFlag(kGameFlag219);
					getSpeech()->playPlayer(134);
				}
			}
			break;

		case kObjectMonsterUp:
			object->setFrameIndex(object->getFrameIndex() + 1);
			if (object->getFrameIndex() >= object->getFrameCount()) {
				_vm->clearGameFlag(kGameFlag564);
				getSpeech()->playPlayer(0);
				object->setFrameIndex(0);
				_vm->setGameFlag(kGameFlag565);
				getWorld()->field_E8490 = -666;
			}
			break;

		case kObjectMonsterAttack:
			if (object->getFrameIndex() != 9)
				object->setFrameIndex(object->getFrameIndex() + 1);
			if (object->getFrameIndex() == 8) {
				getSound()->playSound(object->getSoundResourceId(), false, Config.sfxVolume);

				switch (getWorld()->field_E8494) {
				default:
					break;

				case 0:
					_vm->setGameFlag(kGameFlag567);
					break;

				case 1:
					_vm->setGameFlag(kGameFlag568);
					break;

				case 2:
					_vm->setGameFlag(kGameFlag569);
					break;

				case 3:
					_vm->setGameFlag(kGameFlag567);
					_vm->setGameFlag(kGameFlag568);
					_vm->setGameFlag(kGameFlag569);
					break;
				}

				if (++getWorld()->field_E8494 > 3)
					getWorld()->field_E8494 = 0;
			}

			if (object->getFrameIndex() >= object->getFrameCount()) {
				_vm->clearGameFlag(kGameFlag570);
				object->setFrameIndex(0);
				_vm->setGameFlag(kGameFlag565);
			}
			break;

		case kObjectPuke1:
			sum = *player->getPoint1() + *player->getPoint2();
			getSharedData()->vector1 = sum;

			if (getScene()->polygons()->get(getWorld()->actions[getWorld()->getActionAreaIndexById(1591)]->polygonIndex).contains(sum)) {
				ActorStatus playerStatus = getScene()->getActor(getSharedData()->getPlayerIndex())->getStatus();
				if (playerStatus == kActorStatusWalking2 || playerStatus == kActorStatusAttacking || playerStatus == kActorStatusEnabled2) {
					actor0->changeStatus(kActorStatusGettingHurt);
					getSpeech()->playPlayer(131);
					++getWorld()->field_E848C;
					getSound()->stop(getWorld()->soundResourceIds[3]);
					getSound()->stop(getWorld()->soundResourceIds[4]);
					getSound()->stop(getWorld()->soundResourceIds[5]);
				}
			}

			object->setFrameIndex(object->getFrameIndex() + 1);
			if (object->getFrameIndex() == 15) {
				Object *otherObject = getWorld()->getObjectById(kObjectMonsterAttack);
				otherObject->setFrameIndex(otherObject->getFrameIndex() + 1);
			}

			if (object->getFrameIndex() >= object->getFrameCount()) {
				_vm->clearGameFlag(kGameFlag567);
				object->setFrameIndex(0);
			}
			break;

		case kObjectPuke2:
			sum = *player->getPoint1() + *player->getPoint2();
			getSharedData()->vector1 = sum;

			if (getScene()->polygons()->get(getWorld()->actions[getWorld()->getActionAreaIndexById(1590)]->polygonIndex).contains(sum)) {
				ActorStatus playerStatus = getScene()->getActor(getSharedData()->getPlayerIndex())->getStatus();
				if (playerStatus == kActorStatusWalking2 || playerStatus == kActorStatusAttacking || playerStatus == kActorStatusEnabled2) {
					actor0->changeStatus(kActorStatusGettingHurt);
					++getWorld()->field_E848C;
					getSound()->stop(getWorld()->soundResourceIds[3]);
					getSound()->stop(getWorld()->soundResourceIds[4]);
					getSound()->stop(getWorld()->soundResourceIds[5]);
					getSpeech()->playPlayer(131);
				}
			}

			object->setFrameIndex(object->getFrameIndex() + 1);
			if (object->getFrameIndex() == 15 && _vm->isGameFlagNotSet(kGameFlag567)) {
				Object *otherObject = getWorld()->getObjectById(kObjectMonsterAttack);
				otherObject->setFrameIndex(otherObject->getFrameIndex() + 1);
			}

			if (object->getFrameIndex() >= object->getFrameCount()) {
				_vm->clearGameFlag(kGameFlag568);
				object->setFrameIndex(0);
			}
			break;

		case kObjectPuke3:
			sum = *player->getPoint1() + *player->getPoint2();
			getSharedData()->vector1 = sum;

			if (getScene()->polygons()->get(getWorld()->actions[getWorld()->getActionAreaIndexById(1589)]->polygonIndex).contains(sum)) {
				ActorStatus playerStatus = getScene()->getActor(getSharedData()->getPlayerIndex())->getStatus();
				if (playerStatus == kActorStatusWalking2 || playerStatus == kActorStatusAttacking || playerStatus == kActorStatusEnabled2) {
					actor0->changeStatus(kActorStatusGettingHurt);
					++getWorld()->field_E848C;
					getSound()->stop(getWorld()->soundResourceIds[3]);
					getSound()->stop(getWorld()->soundResourceIds[4]);
					getSound()->stop(getWorld()->soundResourceIds[5]);
					getSpeech()->playPlayer(131);
				}
			}

			object->setFrameIndex(object->getFrameIndex() + 1);
			if (object->getFrameIndex() == 15 && _vm->isGameFlagNotSet(kGameFlag567)) {
				Object *otherObject = getWorld()->getObjectById(kObjectMonsterAttack);
				otherObject->setFrameIndex(otherObject->getFrameIndex() + 1);
			}

			if (object->getFrameIndex() >= object->getFrameCount()) {
				_vm->clearGameFlag(kGameFlag569);
				object->setFrameIndex(0);
			}
			break;

		case kObjectMonsterStatus:
			_vm->setGameFlag(kGameFlag572);
			if (getWorld()->dword_4563A0 == -1 && !object->getFrameIndex()) {
				getWorld()->dword_4563A0 = 1;
			} else {
				object->setFrameIndex(getWorld()->dword_4563A0 + object->getFrameIndex());
				if (object->getFrameIndex() >= object->getFrameCount()) {
					object->setFrameIndex(object->getFrameCount() - 1);
					getWorld()->dword_4563A0 = -1;
				}
			}

			if (getWorld()->field_E8490 == -666) {
				getWorld()->field_E8490 = _vm->getTick() + 3000;
			} else if ((uint32)getWorld()->field_E8490 < _vm->getTick()) {
				getWorld()->field_E8490 = -666;
				if (_vm->isGameFlagSet(kGameFlag572)) {
					_vm->clearGameFlag(kGameFlag565);
					_vm->setGameFlag(kGameFlag570);
				}
			}
			break;

		}
	} else {
		if (actorIndex == 1 && !actor0->isVisible()) {
			if (_vm->isGameFlagNotSet(kGameFlag560))
				actor1->setFrameIndex((actor1->getFrameIndex() + 1) % actor1->getFrameCount());
			if (getWorld()->tickCount1 < _vm->getTick() && !actor1->getFrameIndex()) {
				if (_vm->isGameFlagNotSet(kGameFlag560)) {
					_vm->setGameFlag(kGameFlag560);
					actor1->hide();
					actor1->changeStatus(kActorStatusEnabled);
					actor0->changeStatus(kActorStatusEnabled);
					getWorld()->field_E848C = 0;
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1574)]->scriptIndex, kActorSarah);
				}
			}
		}
	}
}

void Special::chapter12(Object *object, ActorIndex actorIndex) {
	playChapterSound(object, actorIndex);

	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		return;

	case kObjectFloorTrap9:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag809)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag809);
		}
		break;

	case kObjectFloorTrap8:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag810)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag810);
		}
		break;

	case kObjectFloorTrap4:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag794)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag794);
		}
		break;

	case kObjectFloorTrap3:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag795)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag795);
		}
		break;

	case kObjectFloorTrap2:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag796)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag796);
		}
		break;

	case kObjectFloorTrap1:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag797)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag797);
		}
		break;

	case kObjectFloorTrap5:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag792)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag792);
		}
		break;

	case kObjectFloorTrap6:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag791)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag791);
		}
		break;

	case kObjectFloorTrap7:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag790)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag790);
		}
		break;

	case kObjectSparkPuzzleRight:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag761)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag761);
		}
		break;

	case kObjectSparkPuzzleMiddle:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag760)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag760);
		}
		break;

	case kObjectSparkPuzzleLeft:
		if (object->getFrameIndex() > 1 && _vm->isGameFlagSet(kGameFlag759)) {
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(1971)]->scriptIndex, kActorMax);
			_vm->clearGameFlag(kGameFlag759);
		}
		break;
	}
}

void Special::chapter13(Object *object, ActorIndex actorIndex) {
	uint32 i = 0;

	playChapterSound(object, actorIndex);

	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		return;

	case kObjectLine:
		if (!(getScene()->getFrameCounter() % 50) && _vm->isGameFlagNotSet(kGameFlag1122)) {
			if (object->getFrameIndex() >= 59) {
				_vm->setGameFlag(kGameFlag1121);
				object->setFrameIndex(0);
				getScript()->removeFromQueue(getWorld()->scriptIndex);
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2578)]->scriptIndex, kActorMax);
			} else if (object->getFrameIndex() < object->getFrameCount()) {
				object->setFrameIndex(object->getFrameIndex() + 1);
			}
		}
		break;

	case kObjectZapPattern1:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[0][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[0][i++]) {
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2237)]->scriptIndex, kActorMax);
				}
			} while (zapPatterns[0][i]);
		}
		break;

	case kObjectZapPattern2:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[1][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[1][i++])
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2237)]->scriptIndex, kActorMax);
			} while (zapPatterns[1][i]);
		}
		break;

	case kObjectZapPattern3:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[2][0]) {
			while (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id != zapPatterns[2][i]) {
				if (!zapPatterns[2][i++])
					return;
			}
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2237)]->scriptIndex, kActorMax);
		}
		break;

	case kObjectZapPattern4:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[3][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[3][i++])
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2329)]->scriptIndex, kActorMax);
			} while (zapPatterns[3][i]);
		}
		break;

	case kObjectZapPattern5:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[4][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[4][i++])
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2329)]->scriptIndex, kActorMax);
			} while (zapPatterns[4][i]);
		}
		break;

	case kObjectZapPattern6:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[5][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[5][i++]) {
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2329)]->scriptIndex, kActorMax);
				}
			} while (zapPatterns[5][i]);
		}
		break;

	case kObjectZapPattern7:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[6][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[6][i++])
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2331)]->scriptIndex, kActorMax);
			} while (zapPatterns[6][i]);
		}
		break;

	case kObjectZapPattern8:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[7][0]) {
			do {
				if (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id == zapPatterns[7][i++])
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2331)]->scriptIndex, kActorMax);
			} while (zapPatterns[7][i]);
		}
		break;

	case kObjectZapPattern9:
		if (_vm->isGameFlagNotSet(kGameFlag1122) && !object->getFrameIndex() && zapPatterns[8][0]) {
			while (getWorld()->actions[getScene()->getActor(0)->getActionIndex3()]->id != zapPatterns[8][i++]) {
				if (!zapPatterns[8][i])
					return;
			}
			getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2331)]->scriptIndex, kActorMax);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Sound logic
//////////////////////////////////////////////////////////////////////////
void Special::playChapterSound(Object *object, ActorIndex actorIndex) {
	ResourceId id = getResourceId(object, actorIndex);

	if (getSharedData()->getFlag(kFlagIsEncounterRunning))
		return;

	if (id != kResourceNone && getSound()->isPlaying(id))
		return;

	if (Config.performance <= 2)
		return;

	switch (getWorld()->chapter) {
	default:
	case kChapterNone:
	case kChapter5:
	case kChapter9:
	case kChapter10:
		// Nothing to do here
		break;

	case kChapter1:
		playSoundChapter1(object, actorIndex);
		break;

	case kChapter2:
		playSoundChapter2(object, actorIndex);
		break;

	case kChapter3:
		playSoundChapter3(object, actorIndex);
		break;

	case kChapter4:
		playSoundChapter4(object, actorIndex);
		break;

	case kChapter6:
		playSoundChapter6(object, actorIndex);
		break;

	case kChapter7:
		playSoundChapter7(object, actorIndex);
		break;

	case kChapter8:
		playSoundChapter8(object, actorIndex);
		break;
	}
}

void Special::playSoundChapter1(Object *object, ActorIndex actorIndex) {
	if (_vm->checkGameVersion("Demo"))
		return;

	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectRocker:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1847), 14, object);
			break;

		case kObjectGuyWobbles:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1887), 14, object);
			break;

		case kObjectPreAlphaNut:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1889 + rnd(3)), 14, object);
			break;

		case kObjectPreAlphaNut2:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1924 + rnd(3)), 14, object);
			break;
		}
	} else {
		// The original checks for actor indexes 1582 & 1584,
		// but the scene data only has a maximum of 50 actors
		if (actorIndex == 1582 || actorIndex == 1584)
			error("[Special::playSoundChapter1] Called with invalid actor indexes!");
	}
}

void Special::playSoundChapter2(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectJessieStatusQuo:
		case kObjectTicaTac01:
			if (chance()) {
				if (!getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSharedSound, 1848))
				 && !getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSharedSound, 1849))
				 && !getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSharedSound, 1850))
				 && !getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSharedSound, 1844))
				 && !getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSharedSound, 1845))
				 && !getSound()->isPlaying(MAKE_RESOURCE(kResourcePackSharedSound, 1846))) {
					if (object->getId() == kObjectJessieStatusQuo)
						playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1848 + rnd(3)), 13, object);
					else
						playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1844 + rnd(3)), 19, object);
				}
			}
			break;

		case kObjectDennisStatusQuo:
		case kObjectDennisStatus2:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1856), 16, object);
			break;

		case kObjectSuckerSittingStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1859), 15, object);
			break;

		case kObjectSailorBoy:
		case kObjectSailorStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1858), 19, object);
			break;

		case kObjectFishingBoy:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1860), 19, object);
			break;

		case kObjectMarty02:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1854), 15, object);
			break;

		case kObjectEleenOnGround:
		case kObjectEileenOnBench:
		case kObject994:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1857), 28, object);
			break;
		}
	} else {
		switch (actorIndex) {
		default:
			break;

		case 1:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1855), 18, actorIndex);
			break;

		case 5:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1859), 15, actorIndex);
			break;
		}
	}
}

void Special::playSoundChapter3(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectNPC024Church:
		case kObjectNPC024Fountain:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1895), 17, object);
			break;

		case kObjectPreacherBob:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1896 + rnd(2)), 14, object);
			break;

		case kObjectPreacherBobAssistant:
		case kObjectNPC026OutOfWay:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1898), 18, object);
			break;

		case kObjectNPC027Sit:
		case kObjectNPC027Dancing:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1899), 15, object);
			break;

		case kObjectNPC028Sit:
		case kObjectNPC028Dancing:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1900), 15, object);
			break;

		case kObjectNPC029Sit:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1901), 8, object);
			break;
		}
	} else {
		Actor *actor = getScene()->getActor(actorIndex);

		switch (actorIndex) {
		default:
			break;

		case 1:
			if (actor->getStatus() == kActorStatusFidget
			 && actor->checkBoredStatus()
			 && actor->getFrameIndex() == 0)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1861), 22, actorIndex);
			break;

		case 2:
			if (actor->getStatus() == kActorStatusFidget
			 && actor->checkBoredStatus()
			 && actor->getFrameIndex() == 1)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1892), 16, actorIndex);
			break;

		case 3:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1893), 17, actorIndex);
			break;

		case 4:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1894), 17, actorIndex);
			break;

		case 5:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1902), 15, actorIndex);
			break;
		}
	}
}

void Special::playSoundChapter4(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObjectNPC032Sleeping:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1903), 14, object);
			break;

		case kObjectNPC032StatusQuoOutside:
		case kObjectNPC032StatusQuoBigTop:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1904), 14, object);
			break;

		case kObjectNPC033Reading:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1879), 16, object);
			break;

		case kObjectTattooManStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1884), 14, object);
			break;

		case kObjectStrongmanStatusQuo:
		case kObjectStrongmanLeft:
		case kObjectStrongmanRight:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1885), 16, object);
			break;

		case kObjectStrongmanStatusQuo2:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1886), 16, object);
			break;

		case kObjectInfernoStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1905), 12, object);
			break;

		case kObjectJuggler:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1906), 12, object);
			break;

		case kObjectClownStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1907), 10, object);
			break;

		case kObjectTrixieStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1908), 12, object);
			break;

		case kObjectSimonStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1909), 12, object);
			break;

		case kObjectFunTixStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1913), 10, object);
			break;

		case kObjectFreakTixStatusQuoUp:
		case kObjectFreakTixStatusQuoDown:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1914), 9, object);
			break;

		case kObjectFortTellerStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1915), 15, object);
			break;

		case kObjectRingTossStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1916), 10, object);
			break;

		case kObjectPigShootStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1917), 10, object);
			break;

		case kObjectKnockDownStatusQuo:
			if (_vm->isGameFlagNotSet(kGameFlag358) && chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1918), 10, object);
			break;

		case kObjectPretZoolStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1919), 14, object);
			break;

		case kObjectTimberStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1920), 14, object);
			break;

		case kObjectTwinsStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1921), 20, object);
			break;

		case kObjectSeanStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1923), 16, object);
			break;

		case kObjectMomAndPopStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1852), 15, object);

			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1862), 15, object);
			break;
		}
	} else {
		switch (actorIndex) {
		default:
			break;

		case 1:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1910), 14, actorIndex);
			break;

		case 2:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1911), 14, actorIndex);
			break;

		case 3:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1912), 14, actorIndex);
			break;
		}
	}
}

void Special::playSoundChapter6(Object *object, ActorIndex actorIndex) {
	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		break;

	case kObjectGravinStatusQuoCyber:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1880), 14, object);
		break;

	case kObjectGravinWorkMachine:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1880), 14, object);
		break;

	case kObjectNPC062GritzaStatusQuo:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1881), 12, object);
		break;

	case kObjectNPC063GrundleStatusQuo:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1882), 12, object);
		break;

	case kObjectNPC064GrellaStatusQuo:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1883), 12, object);
		break;

	case kObjectNPC065StatusQuo:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1868), 12, object);
		break;

	case kObjectNPC066StatusQuo:
		if (chance())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1867), 12, object);
		break;
	}
}

void Special::playSoundChapter7(Object *object, ActorIndex actorIndex) {
	if (actorIndex != kActorInvalid)
		return;

	switch (object->getId()) {
	default:
		break;

	case kObjectDrawers3:
		if (_vm->isGameFlagNotSet(kGameFlag405) && _vm->isGameFlagSet(kGameFlag423))
			if (rnd(100) < 20)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1865), 18, object);
		break;

	case kObjectOldMan3:
		if (rnd(100) < 20)
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1866), 18, object);
		break;
	}
}

void Special::playSoundChapter8(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch (object->getId()) {
		default:
			break;

		case kObject703:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1874), 10, object);
			break;

		case kObjectTree8:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1875), 10, object);
			break;

		case kObjectSlab:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1922), 10, object);
			break;

		case kObjectWitchDoctor:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1863), 16, object);
			break;

		case kObjectStoneWifeStatuQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1872), 15, object);
			break;

		case kObjectFishermanWidowStatusQuo:
			if (_vm->isGameFlagNotSet(kGameFlag553) && !getSound()->isPlaying(getSpeech()->getSoundResourceId()))
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1870), 10, object);
			break;

		case kObjectStoneMasonStatusHitt:
			if (rnd(100) < 30)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1873), 15, object);
			break;

		case kObjectGirlStatusQuo:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1871), 15, object);
			break;

		case kObjectWitchWifeStirring:
			if (rnd(100) < 30)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1869), 17, object);
			break;

		case kObjectQuetzlcaotlStatusQuo:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1864), 3, object);
			break;

		case kObjectBlanklPixelForWarrior:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1876), 10, object);
			break;

		case kObjectPixelForHutWarrior:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1878), 10, object);
			break;

		case kObjectPixelForWaterfallGhost:
			if (chance())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1877), 10, object);
			break;
		}
	} else {
		if (!_vm->isGameFlagNotSet(kGameFlag528))
			return;

		switch (actorIndex) {
		default:
			break;

		case 1:
			if (!getSound()->isPlaying(getWorld()->soundResourceIds[8]))
				playSoundPanning(getWorld()->soundResourceIds[8], 15, actorIndex);
			break;

		case 2:
			if (!getSound()->isPlaying(getWorld()->soundResourceIds[7]))
				playSoundPanning(getWorld()->soundResourceIds[7], 15, actorIndex);
			break;

		case 3:
			if (!getSound()->isPlaying(getWorld()->soundResourceIds[11]))
				playSoundPanning(getWorld()->soundResourceIds[11], 15, actorIndex);
			break;

		case 4:
			if (!getSound()->isPlaying(getWorld()->soundResourceIds[12]))
				playSoundPanning(getWorld()->soundResourceIds[12], 15, actorIndex);
			break;

		case 5:
			if (!getSound()->isPlaying(getWorld()->soundResourceIds[13]))
				playSoundPanning(getWorld()->soundResourceIds[13], 15, actorIndex);
			break;

		case 6:
			if (!getSound()->isPlaying(getWorld()->soundResourceIds[14]))
				playSoundPanning(getWorld()->soundResourceIds[14], 15, actorIndex);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Special::setPaletteGamma(ResourceId palette1, ResourceId palette2) {
	if (_paletteFlag) {
		if (_paletteTick2 < _vm->getTick()) {
			_paletteFlag = false;

			getSound()->playSound(getWorld()->ambientSounds[rnd(5) + 2].resourceId, false, Config.ambientVolume);

			_paletteTick1 = _vm->getTick() + 5000;
		}
	} else {
		switch (_chapter5Counter) {
		default:
			if (_chapter5Counter > 0) {
				++_chapter5Counter;
			} else {
				if (_paletteTick1 < _vm->getTick()) {
					if (rnd(100) >= 20) {
						_paletteTick1 = _vm->getTick() + 5000;
					} else {
						getScreen()->setPalette(palette1);
						getScreen()->setGammaLevel(palette1);
						_chapter5Counter = 1;
					}
				}
			}
			break;

		case 500:
			getScreen()->setPalette(palette2);
			getScreen()->setGammaLevel(palette2);
			++_chapter5Counter;
			break;

		case 1000:
			getScreen()->setPalette(palette1);
			getScreen()->setGammaLevel(palette1);
			++_chapter5Counter;
			break;

		case 1500:
			getScreen()->setPalette(palette2);
			getScreen()->setGammaLevel(palette2);
			_chapter5Counter = 0;
			_paletteFlag = true;
			_paletteTick2 = _vm->getTick() + rnd(1000) + 1000;
			break;
		}
	}
}

void Special::playSoundPanning(ResourceId resourceId, int32 attenuation, Object *object) {
	int32 adjustedVolume = Config.voiceVolume;
	Common::Point point;

	switch (object->getId()) {
	default: {
		Common::Rect frameRect = GraphicResource::getFrameRect(_vm, object->getResourceId(), object->getFrameIndex());

		point.x = (int16)(Common::Rational(frameRect.width(), 2).toInt() + object->x);
		point.y = (int16)(Common::Rational(frameRect.height(), 2).toInt() + object->y);
		}
		break;

	case kObjectDennisStatusQuo:
		point.x = 1382;
		point.y = 1041;
		break;

	case kObjectSailorBoy:
		point.x = 1646;
		point.y = 1220;
		break;

	case kObjectSuckerSittingStatusQuo:
		point.x = 1376;
		point.y = 1148;
		break;

	case kObjectDennisStatus2:
		point.x = 175;
		point.y = 617;
		break;
	}

	// Calculate volume adjustment
	adjustedVolume += getSound()->calculateVolumeAdjustement(point, attenuation, 0);

	// Calculate panning
	int32 panning = getSound()->calculatePanningAtPoint(point);

	// Adjust object properties
	object->setSoundResourceId(resourceId);
	object->setField6A4((ActorDirection)attenuation);

	getSound()->playSound(resourceId, false, adjustedVolume, panning);
}

void Special::playSoundPanning(ResourceId resourceId, int32 attenuation, ActorIndex actorIndex) {
	Actor *actor = getScene()->getActor(actorIndex);

	// Calculate volume adjustment
	int32 adjustedVolume = Config.voiceVolume;
	adjustedVolume += getSound()->calculateVolumeAdjustement(*actor->getPoint1(), attenuation, 0);

	// Calculate panning
	int32 panning = getSound()->calculatePanningAtPoint(*actor->getPoint1() + *actor->getPoint2());

	getSound()->playSound(resourceId, false, adjustedVolume, panning);
}

void Special::updateObjectFlag(ObjectId id) {
	// Check if any of the scene sound resources are playing
	for (int i = 0; i < 7; i++) {
		if (getSound()->isPlaying(getWorld()->soundResourceIds[i]))
			return;
	}

	getWorld()->getObjectById(id)->flags &= ~kObjectFlag10E38;
}

void Special::checkFlags(ObjectId id, GameFlag flag1, GameFlag flag2, GameFlag flag3, GameFlag flag4, uint32 *val1, uint32 *val2, GameFlag flag5, uint32 *val3) {
	if (_vm->isGameFlagSet(flag5)
	 && _vm->isGameFlagNotSet(flag2)
	 && _vm->isGameFlagNotSet(flag1)) {
		_vm->clearGameFlag(flag5);

		*val1 = 1;
	}

	if (!*val1) {
		*val1 = _vm->getTick() - rnd(2000);
		return;
	}

	if (!_vm->isGameFlagNotSet(flag3)) {
		*val1 = _vm->getTick();
		return;
	}

	if ((_vm->getTick() - *val1) > 10000) {
		if (_vm->isGameFlagSet(flag2)) {

			_vm->clearGameFlag(flag4);
			_vm->clearGameFlag(flag5);

			if (*val2 >= 8) {
				*val1 = 0;
				*val2 = 0;

				_vm->clearGameFlag(flag1);
				_vm->clearGameFlag(flag2);

				getWorld()->getObjectById(id)->setTransparency(0);

			} else {
				(*val2)++;

				getWorld()->getObjectById(id)->setTransparency(6 - Common::Rational(*val2, 4).toInt());
			}
		} else {

			_vm->setGameFlag(flag1);

			*val3 = 0;

			if (*val2 >= 8) {
				*val1 = 0;
				*val2 = 0;

				_vm->setGameFlag(flag2);
				_vm->setGameFlag(flag4);

				getWorld()->getObjectById(id)->setTransparency(0);

			} else {
				(*val2)++;

				getWorld()->getObjectById(id)->setTransparency(Common::Rational(*val2, 4).toInt() + 4);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
ResourceId Special::getResourceId(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid && object == nullptr)
		error("[Special::getResourceId] Both arguments cannot be NULL/empty!");

	return (actorIndex == kActorInvalid) ? object->getSoundResourceId() : getScene()->getActor(actorIndex)->getSoundResourceId();
}

uint32 Special::getCounter(ActorIndex index) const {
	switch (index) {
	default:
		error("[Special::getCounter] Invalid actor index (was: %d, valid: 13, 15, 17, 18)", index);

	case 13:
		return 1;

	case 15:
		return 2;

	case 17:
		return 3;

	case 18:
		return 4;
	}
}

void Special::checkObject(Object *object, GameFlag flagToSet, GameFlag flagToClear, ObjectId objectId) {
	if (object->getFrameIndex() == 15) {
		Object *otherObject = getWorld()->getObjectById(objectId);
		debugC(kDebugLevelObjects, "[checkObject] %s -> %s (flags: set %d, clear %d)",
				object->getName(), otherObject->getName(), flagToSet, flagToClear);

		_vm->setGameFlag(flagToSet);
		_vm->clearGameFlag(flagToClear);

		if (objectId == kObjectNone)
			object->setFrameIndex(0);
		else
			otherObject->setFrameIndex(0);
	}
}

void Special::checkOtherObject(Object *object, ObjectId otherObjectId, GameFlag flagToClear, GameFlag flagToSet) {
	Object *otherObject = getWorld()->getObjectById(otherObjectId);

	debugC(kDebugLevelObjects, "[checkOtherObject] %s -> %s (flags: set %d, clear %d)",
			object->getName(), otherObject->getName(), flagToSet, flagToClear);

	if (!otherObject->getFrameIndex() && !object->getFrameIndex()) {
		_vm->clearGameFlag(flagToClear);
		_vm->setGameFlag(flagToSet);
		otherObject->setNextFrame(kObjectFlag8);
	}
}

//////////////////////////////////////////////////////////////////////////
// Chapter 11 helpers
//////////////////////////////////////////////////////////////////////////

void Special::rock(ActorIndex actorIndex, GameFlag flag1, GameFlag flag2, GameFlag flag3, GameFlag flag4, const int16 (*rectPtr)[4]) {
	Common::Rect rect((*rectPtr)[0], (*rectPtr)[1], (*rectPtr)[2], (*rectPtr)[3]);
	Actor *actor = getScene()->getActor(actorIndex);

	if (!(_vm->isGameFlagNotSet(flag1) && _vm->isGameFlagSet(flag2)))
		return;

	if (_vm->isGameFlagNotSet(flag3)) {
		getWorld()->tickValueArray[actorIndex] = _vm->getTick() + 1500;

		actor->getPoint1()->x = rect.left + rnd(rect.right - rect.left) - actor->getPoint2()->x;
		actor->getPoint1()->y = rect.top + rnd(rect.bottom - rect.top) - actor->getPoint2()->y;

		getWorld()->field_E8594[actorIndex] = actor->getPoint1()->y;
		actor->getPoint1()->y -= 160;

		actor->changeStatus(kActorStatusEnabled2);
		getSound()->playSound(getWorld()->soundResourceIds[0], false, Config.sfxVolume - 10);
		_vm->setGameFlag(flag3);
		getScene()->getActor(actorIndex)->show();
	} else if (getWorld()->tickValueArray[actorIndex] == -666 || _vm->getTick() > (uint32)getWorld()->tickValueArray[actorIndex]) {
		if (_vm->isGameFlagNotSet(flag4)) {
			_vm->setGameFlag(flag4);
			actor->setFrameIndex(0);
			actor->changeStatus(kActorStatusWalking2);
			if (actorIndex == 8 || actorIndex == 9)
				actor->setField944(1);
			actor->getPoint1()->y = 0;
		} else {
			if (actor->getPoint1()->y <= getWorld()->field_E8594[actorIndex]) {
				actor->getPoint1()->y += 27;
				actor->setFrameIndex((actor->getFrameIndex() + 1) % actor->getFrameCount());
			} else if (actor->getStatus() == kActorStatusAttacking) {
				actor->setFrameIndex((actor->getFrameIndex() + 1) % actor->getFrameCount());
				if (actor->getFrameIndex() == actor->getFrameCount() - 1)
					actor->hide();
			} else {
				actor->setField944(3);
				getSound()->playSound(getWorld()->soundResourceIds[1], false, Config.sfxVolume - 10);
				actor->changeStatus(kActorStatusAttacking);
				actor->setFrameIndex(4);

				Common::Point sum = *actor->getPoint1() + *actor->getPoint2();
				Common::Point playerSum = *getScene()->getActor()->getPoint1() + *getScene()->getActor()->getPoint2();

				getSharedData()->vector1 = playerSum;
				getSharedData()->vector2 = sum;

				if (Actor::euclidianDistance(sum, playerSum) < 30) {
					getScene()->getActor(0)->changeStatus(kActorStatusGettingHurt);
					++getWorld()->field_E848C;
					getSound()->stop(getWorld()->soundResourceIds[3]);
					getSound()->stop(getWorld()->soundResourceIds[4]);
					getSound()->stop(getWorld()->soundResourceIds[5]);
					getSpeech()->playPlayer(131);
				}
			}
		}
	}
}

void Special::tentacle(ActorIndex actorIndex, GameFlag flag1, GameFlag flag2, GameFlag flag3, const int16 (*rectPtr)[4]) {
	Common::Rect rect((*rectPtr)[0], (*rectPtr)[1], (*rectPtr)[2], (*rectPtr)[3]);
	Actor *actor = getScene()->getActor(actorIndex);

	if (_vm->isGameFlagSet(flag1)
	 && _vm->isGameFlagNotSet(flag3)
	 && !_vm->isGameFlagSet(flag2)
	 && (getWorld()->tickValueArray[actorIndex + 10] == -666 || _vm->getTick() > (uint32)getWorld()->tickValueArray[actorIndex + 10])) {
		actor->getPoint1()->x = rect.left + rnd(rect.right - rect.left) - actor->getPoint2()->x;
		actor->getPoint1()->y = rect.top + rnd(rect.bottom - rect.top) - actor->getPoint2()->y;

		Common::Point sum = *actor->getPoint1() + *actor->getPoint2();
		Common::Point playerSum = *getScene()->getActor()->getPoint1() + *getScene()->getActor()->getPoint2();

		getSharedData()->vector1 = playerSum;
		getSharedData()->vector2 = sum;

		if (Actor::euclidianDistance(sum, playerSum) > 40) {
			getWorld()->tickValueArray[actorIndex + 10] = 0;
			actor->show();
			actor->changeStatus(kActorStatusWalking2);
			actor->setFrameIndex(0);
			_vm->setGameFlag(flag2);
		}
	}
}

} // End of namespace Asylum
