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

#include "asylum/resources/special.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/object.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

Special::Special(AsylumEngine *engine) : _vm(engine) {

	// Counters & flags
	_chapter2Counter = 0;
	_chapter5Counter = 0;

	_paletteFlag = false;
	_paletteTick1 = 0;
	_paletteTick2 = 0;
}

Special::~Special() {
	// Zero-out passed pointers
	_vm = NULL;
}


void Special::run(Object* object, ActorIndex index) {
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
	}
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
				object->getFrameSoundItem(0)->resourceId = getWorld()->graphicResourceIds[rnd(2) ? 37 : 38];
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
				Actor::enableActorsChapter2(_vm);
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
			checkObject(object, kGameFlag447, kGameFlag448, kObjectNone);
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
				getSharedData()->setActorUpdateFlag(2);
				break;
			}
		}

		for (int i = 13; i < 22; i++)
			getScene()->getActor(i)->updateStatus(kActorStatus15);
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
		if (actor->getStatus() == kActorStatus1) {
			++_chapter2Counter;

			if (_chapter2Counter >= 5) {
				_chapter2Counter = 0;
				actor->updateFromDirection((ActorDirection)((actor->getDirection() + 1) % ~7)); // TODO check
			}
		} else if (actor->getStatus() == kActorStatusEnabled) {
			actor->updateStatus(kActorStatus1);
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
			error("[Special::chapter2] Not implemented!");
			// FIXME: This is wrong, the 386th object is used as an offset to the proper actor
			//Object *obj386 = getWorld()->objects[386];

			//actor->getPoint1()->x = obj386->getSoundY();
			//actor->getPoint1()->y = obj386->getField688() + getWorld()->coordinates[actorIndex - 22]; // FIXME out of bound access for actorIndex == 29
			//actor->setFrameIndex(obj386->getField67C());
			//actor->setDirection(obj386->getField6A4());

			//// Get the resource Id
			//Actor *actor0 = getScene()->getActor(0);
			//Actor *actor13 = getScene()->getActor(13);
			//int32 index = (actor13->getDirection() > 4) ? 8 - actor13->getDirection() : actor13->getDirection();
			//ResourceId id = actor0->getResourcesId((uint32)index);

			//actor->setResourceId(id);
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
				if (getSharedData()->getActorUpdateFlag2() < 7)
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
	setPaletteGamma(MAKE_RESOURCE(kResourcePackShared, 19), getWorld()->currentPaletteId);

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

		getWorld()->ambientSounds[0].x = actor->getPoint2()->x + actor->getPoint1()->x;
		getWorld()->ambientSounds[0].y = actor->getPoint2()->y + actor->getPoint1()->y;
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
				if (player->getReaction(0)) {

					if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10) {
						getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 2));
						player->enable();
					} else {
						getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
						player->updateStatus(kActorStatus6);
					}

					_vm->setGameFlag(kGameFlag1023);
				} else {
					_vm->setGameFlag(kGameFlag1022);
				}

				_vm->clearGameFlag(kGameFlag1021);
			}

			if (_vm->isGameFlagSet(kGameFlag1023)) {
				if (player->getField638()) {
					getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(player->getField638() == 3 ? 2447 : 2448)]->scriptIndex,
					                         getScene()->getPlayerIndex());
					_vm->clearGameFlag(kGameFlag1023);
				} else if (player->getStatus() != kActorStatus6) {
					_vm->clearGameFlag(kGameFlag1023);
					_vm->setGameFlag(kGameFlag1022);
				}
			}

			if (_vm->isGameFlagSet(kGameFlag1022)) {
				_vm->clearGameFlag(kGameFlag1022);
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2445)]->scriptIndex,
				                         getScene()->getPlayerIndex());
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
					actor0->process_41BC00(1, 0);

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

	switch(object->getId()) {
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

//////////////////////////////////////////////////////////////////////////
// Sound logic
//////////////////////////////////////////////////////////////////////////
void Special::playChapterSound(Object *object, ActorIndex actorIndex) {
	ResourceId id = getResourceId(object, actorIndex);

	if (_vm->encounter()->isRunning())
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
	if (actorIndex == kActorInvalid) {
		switch(object->getId()) {
		default:
			break;

		case kObjectRocker:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1847), 14, object);
			break;

		case kObjectGuyWobbles:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1887), 14, object);
			break;

		case kObjectPreAlphaNut:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1889 + rnd(3)), 14, object);
			break;

		case kObjectPreAlphaNut2:
			if (_vm->getRandomBit())
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
		switch(object->getId()) {
		default:
			break;

		case kObjectJessieStatusQuo:
		case kObjectTicaTac01:
			if (_vm->getRandomBit()) {
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
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1856), 16, object);
			break;

		case kObjectSuckerSittingStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1859), 15, object);
			break;

		case kObjectSailorBoy:
		case kObjectSailorStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1858), 19, object);
			break;

		case kObjectFishingBoy:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1860), 19, object);
			break;

		case kObjectMarty02:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1854), 15, object);
			break;

		case kObjectEleenOnGround:
		case kObjectEileenOnBench:
		case kObject994:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1857), 28, object);
			break;
		}
	} else {
		switch (actorIndex) {
		default:
			break;

		case 1:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1855), 18, actorIndex);
			break;

		case 5:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1859), 15, actorIndex);
			break;
		}
	}
}

void Special::playSoundChapter3(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch(object->getId()) {
		default:
			break;

		case kObjectNPC024Church:
		case kObjectNPC024Fountain:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1895), 17, object);
			break;

		case kObjectPreacherBob:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1896 + _vm->getRandomBit()), 14, object);
			break;

		case kObjectPreacherBobAssistant:
		case kObjectNPC026OutOfWay:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1898), 18, object);
			break;

		case kObjectNPC027Sit:
		case kObjectNPC027Dancing:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1899), 15, object);
			break;

		case kObjectNPC028Sit:
		case kObjectNPC028Dancing:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1900), 15, object);
			break;

		case kObjectNPC029Sit:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1901), 8, object);
			break;
		}
	} else {
		Actor *actor = getScene()->getActor(actorIndex);

		switch (actorIndex) {
		default:
			break;

		case 1:
			if (actor->getStatus() == kActorStatus9
			 && actor->isResourcePresent()
			 && actor->getFrameIndex() == 0)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1861), 22, actorIndex);
			break;

		case 2:
			if (actor->getStatus() == kActorStatus9
			 && actor->isResourcePresent()
			 && actor->getFrameIndex() == 1)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1892), 16, actorIndex);
			break;

		case 3:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1893), 17, actorIndex);
			break;

		case 4:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1894), 17, actorIndex);
			break;

		case 5:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1902), 15, actorIndex);
			break;
		}
	}
}

void Special::playSoundChapter4(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch(object->getId()) {
		default:
			break;

		case kObjectNPC032Sleeping:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1903), 14, object);
			break;

		case kObjectNPC032StatusQuoOutside:
		case kObjectNPC032StatusQuoBigTop:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1904), 14, object);
			break;

		case kObjectNPC033Reading:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1879), 16, object);
			break;

		case kObjectTattooManStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1884), 14, object);
			break;

		case kObjectStrongmanStatusQuo:
		case kObjectStrongmanLeft:
		case kObjectStrongmanRight:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1885), 16, object);
			break;

		case kObjectStrongmanStatusQuo2:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1886), 16, object);
			break;

		case kObjectInfernoStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1905), 12, object);
			break;

		case kObjectJuggler:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1906), 12, object);
			break;

		case kObjectClownStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1907), 10, object);
			break;

		case kObjectTrixieStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1908), 12, object);
			break;

		case kObjectSimonStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1909), 12, object);
			break;

		case kObjectFunTixStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1913), 10, object);
			break;

		case kObjectFreakTixStatusQuoUp:
		case kObjectFreakTixStatusQuoDown:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1914), 9, object);
			break;

		case kObjectFortTellerStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1915), 15, object);
			break;

		case kObjectRingTossStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1916), 10, object);
			break;

		case kObjectPigShootStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1917), 10, object);
			break;

		case kObjectKnockDownStatusQuo:
			if (_vm->isGameFlagNotSet(kGameFlag358) && _vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1918), 10, object);
			break;

		case kObjectPretZoolStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1919), 14, object);
			break;

		case kObjectTimberStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1920), 14, object);
			break;

		case kObjectTwinsStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1921), 20, object);
			break;

		case kObjectSeanStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1923), 16, object);
			break;

		case kObjectMomAndPopStatusQuo:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1852), 15, object);

			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1862), 15, object);
			break;
		}
	} else {
		switch (actorIndex) {
		default:
			break;

		case 1:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1910), 14, actorIndex);
			break;

		case 2:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1911), 14, actorIndex);
			break;

		case 3:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1912), 14, actorIndex);
			break;
		}
	}
}

void Special::playSoundChapter6(Object *object, ActorIndex actorIndex) {
	if (actorIndex != kActorInvalid)
		return;

	switch(object->getId()) {
	default:
		break;

	case kObjectGravinStatusQuoCyber:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1880), 14, object);
		break;

	case kObjectGravinWorkMachine:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1880), 14, object);
		break;

	case kObjectNPC062GritzaStatusQuo:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1881), 12, object);
		break;

	case kObjectNPC063GrundleStatusQuo:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1882), 12, object);
		break;

	case kObjectNPC064GrellaStatusQuo:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1883), 12, object);
		break;

	case kObjectNPC065StatusQuo:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1868), 12, object);
		break;

	case kObjectNPC066StatusQuo:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1867), 12, object);
		break;
	}
}

void Special::playSoundChapter7(Object *object, ActorIndex actorIndex) {
	if (actorIndex != kActorInvalid)
		return;

	switch(object->getId()) {
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
		switch(object->getId()) {
		default:
			break;

		case kObject703:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1874), 10, object);
			break;

		case kObjectTree8:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1875), 10, object);
			break;

		case kObjectSlab:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1922), 10, object);
			break;

		case kObjectWitchDoctor:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1863), 16, object);
			break;

		case kObjectStoneWifeStatuQuo:
			if (_vm->getRandomBit())
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
			if (_vm->getRandomBit())
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
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1876), 10, object);
			break;

		case kObjectPixelForHutWarrior:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSharedSound, 1878), 10, object);
			break;

		case kObjectPixelForWaterfallGhost:
			if (_vm->getRandomBit())
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
				playSoundPanning(getWorld()->soundResourceIds[7], 15, actorIndex); // BUG: Are we really supposed to use that sound (7 instead of 14)
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
						getScreen()->setGammaLevel(palette1, 0);
						_chapter5Counter = 1;
					}
				}
			}
			break;

		case 500:
			getScreen()->setPalette(palette2);
			getScreen()->setGammaLevel(palette2, 0);
			++_chapter5Counter;
			break;

		case 1000:
			getScreen()->setPalette(palette1);
			getScreen()->setGammaLevel(palette1, 0);
			++_chapter5Counter;
			break;

		case 1500:
			getScreen()->setPalette(palette2);
			getScreen()->setGammaLevel(palette2, 0);
			_chapter5Counter = 0;
			_paletteFlag = true;
			_paletteTick2 = _vm->getTick() + rnd(1000) + 1000;
			break;
		}
	}
}

void Special::playSoundPanning(ResourceId resourceId, int32 attenuation, Object *object) {
	int32 adjustedVolume = Config.voiceVolume;
	int32 x = 0;
	int32 y = 0;

	switch (object->getId()) {
	default: {
		Common::Rect frameRect = GraphicResource::getFrameRect(_vm, object->getResourceId(), object->getFrameIndex());

		x = Common::Rational(frameRect.width(), 2).toInt() + object->x;
		y = Common::Rational(frameRect.height(), 2).toInt() + object->y;
		}
		break;

	case kObjectDennisStatusQuo:
		x = 1382;
		y = 1041;
		break;

	case kObjectSailorBoy:
		x = 1646;
		y = 1220;
		break;

	case kObjectSuckerSittingStatusQuo:
		x = 1376;
		y = 1148;
		break;

	case kObjectDennisStatus2:
		x = 175;
		y = 617;
		break;
	}

	// Calculate volume adjustment
	adjustedVolume += getSound()->calculateVolumeAdjustement(x, y, attenuation, 0);

	// Calculate panning
	int32 panning = getSound()->calculatePanningAtPoint(x, y);

	// Adjust object properties
	object->setSoundResourceId(resourceId);
	object->setField6A4((ActorDirection)attenuation);

	getSound()->playSound(resourceId, false, adjustedVolume, panning);
}

void Special::playSoundPanning(ResourceId resourceId, int32 attenuation, ActorIndex actorIndex) {
	Actor *actor = getScene()->getActor(actorIndex);

	// Calculate volume adjustment
	int32 adjustedVolume = Config.voiceVolume;
	adjustedVolume += getSound()->calculateVolumeAdjustement(actor->getPoint1()->x, actor->getPoint1()->y, attenuation, 0);

	// Calculate panning
	int32 panning = getSound()->calculatePanningAtPoint(actor->getPoint1()->x + actor->getPoint2()->x, actor->getPoint1()->y + actor->getPoint2()->y);

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

				getWorld()->getObjectById(id)->setField67C(0);

			} else {
				(*val2)++;

				getWorld()->getObjectById(id)->setField67C(6 - Common::Rational(*val2, 4).toInt());
			}
		} else {

			_vm->setGameFlag(flag1);

			*val3 = 0;

			if (*val2 >= 8) {
				*val1 = 0;
				*val2 = 0;

				_vm->clearGameFlag(flag2);
				_vm->clearGameFlag(flag4);

				getWorld()->getObjectById(id)->setField67C(0);

			} else {
				(*val2)++;

				getWorld()->getObjectById(id)->setField67C(Common::Rational(*val2, 4).toInt() + 4);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
ResourceId Special::getResourceId(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid && object == NULL)
		error("[Special::getResourceId] Both arguments cannot be NULL/empty!");

	return (actorIndex == kActorInvalid) ? object->getSoundResourceId() : getScene()->getActor(actorIndex)->getSoundResourceId();
}

void Special::checkObject(Object *object, GameFlag flagToSet, GameFlag flagToClear, ObjectId objectId) {
	if (object->getFrameIndex() == 15) {
		_vm->setGameFlag(flagToSet);
		_vm->clearGameFlag(flagToClear);

		if (objectId != kObjectNone)
			getWorld()->getObjectById(objectId)->setFrameIndex(0);
		else
			object->setFrameIndex(0);
	}
}

void Special::checkOtherObject(Object *object, ObjectId otherObjectId, GameFlag flagToClear, GameFlag flagToSet) {
	Object *otherObject = getWorld()->getObjectById(otherObjectId);

	if (!otherObject->getFrameIndex() && !object->getFrameIndex()) {
		_vm->clearGameFlag(flagToClear);
		_vm->setGameFlag(flagToSet);
		otherObject->setNextFrame(kObjectFlag8);
	}
}

} // End of namespace Asylum
