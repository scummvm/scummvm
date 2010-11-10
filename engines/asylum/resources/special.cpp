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

#include "asylum/resources/actionlist.h"
#include "asylum/resources/actor.h"
#include "asylum/resources/object.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/speech.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"

namespace Asylum {

Special::Special(AsylumEngine *engine) : _vm(engine) {

	// Counters
	_chapter2Counter = 0;
}

Special::~Special() {}


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
				getScene()->setActorUpdateFlag(2);
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
				actor->updateFromDirection((actor->getDirection() + 1) % ~7); // TODO check
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
			Object *obj386 = getWorld()->objects[386];

			actor->x1 = obj386->getSoundY();
			actor->y1 = obj386->getField688() + getWorld()->coordinates[22 - actorIndex];
			actor->setFrameIndex(obj386->getField67C());
			actor->setDirection(obj386->getField6A4());

			// Get the resource Id
			Actor *actor0 = getScene()->getActor(0);
			Actor *actor13 = getScene()->getActor(13);
			ResourceId id = actor0->getResourcesId(actor13->getDirection() > 4 ? 8 - actor13->getDirection() : actor13->getDirection());

			actor->setResourceId(id);
		}
		break;

	case 38:
	case 39:
		if (actor->getFrameIndex() == 9) {
			actor->x1 = -1000;
			actor->setFrameIndex(0);
		}
		break;

	case 40:
		if (actor->getFrameIndex() == 9) {
			actor->x1 = -1000;
			actor->setFrameIndex(0);

			if (actor->isVisible())
				if (getScene()->getActorUpdateFlag2() < 7)
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

		getWorld()->ambientSounds[0].x = actor->x2 + actor->x1;
		getWorld()->ambientSounds[0].y = actor->y2 + actor->y1;
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
						player->updateStatus(kActorStatusEnabled);
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
					                         getScene()->getPlayerActorIndex());
					_vm->clearGameFlag(kGameFlag1023);
				} else if (player->getStatus() != kActorStatus6) {
					_vm->clearGameFlag(kGameFlag1023);
					_vm->setGameFlag(kGameFlag1022);
				}
			}

			if (_vm->isGameFlagSet(kGameFlag1022)) {
				_vm->clearGameFlag(kGameFlag1022);
				getScript()->queueScript(getWorld()->actions[getWorld()->getActionAreaIndexById(2445)]->scriptIndex,
				                         getScene()->getPlayerActorIndex());
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
				actor1->x1 = actor0->x1;
				actor1->y1 = actor0->y1 - 15;
			} else if (actor0->getDirection() == 5 || actor0->getDirection() == 6) {
				actor1->x1 = actor0->x1 + 20;
				actor1->y1 = actor0->y1 - 15;
			} else {
				actor1->x1 = actor0->x1 + 5;
				actor1->y1 = actor0->y1 - 10;
			}
			break;

		case 2:
			if (actor0->getDirection() <= 2 || actor0->getDirection() >= 7) {
				actor2->x1 = actor0->x1 + 10;
				actor2->y1 = actor0->y1 - 10;
			} else {
				actor2->hide();
				actor1->show();
			}
			break;
		}
	}
}

void Special::chapter8(Object *object, ActorIndex actorIndex) {

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

	if (getEncounter()->getFlag(kEncounterFlag2) != 0)
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
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1847), 14, object);
			break;

		case kObjectGuyWobbles:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1887), 14, object);
			break;

		case kObjectPreAlphaNut:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1889 + rnd(3)), 14, object);
			break;

		case kObjectPreAlphaNut2:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1924 + rnd(3)), 14, object);
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
	error("[Special::playSoundChapter2] Not implemented!");
}

void Special::playSoundChapter3(Object *object, ActorIndex actorIndex) {
	if (actorIndex == kActorInvalid) {
		switch(object->getId()) {
		default:
			break;

		case kObjectNPC024Church:
		case kObjectNPC024Fountain:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1895), 17, object);
			break;

		case kObjectPreacherBob:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1896 + _vm->getRandomBit()), 14, object);
			break;

		case kObjectPreacherBobAssistant:
		case kObjectNPC026OutOfWay:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1898), 18, object);
			break;

		case kObjectNPC027Sit:
		case kObjectNPC027Dancing:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1899), 15, object);
			break;

		case kObjectNPC028Sit:
		case kObjectNPC028Dancing:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1900), 15, object);
			break;

		case kObjectNPC029Sit:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1901), 8, object);
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
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1861), 22, actorIndex);
			break;

		case 2:
			if (actor->getStatus() == kActorStatus9
			 && actor->isResourcePresent()
			 && actor->getFrameIndex() == 1)
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1892), 16, actorIndex);
			break;

		case 3:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1893), 17, actorIndex);
			break;

		case 4:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1894), 17, actorIndex);
			break;

		case 5:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1902), 15, actorIndex);
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
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1903), 14, object);
			break;

		case kObjectNPC032SQOutside:
		case kObjectNPC032SQBigTop:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1904), 14, object);
			break;

		case kObjectNPC033Reading:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1879), 16, object);
			break;

		case kObjectTattooManSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1884), 14, object);
			break;

		case kObjectStrongmanSQ:
		case kObjectStrongmanLeft:
		case kObjectStrongmanRight:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1885), 16, object);
			break;

		case kObjectStrongmanSQ2:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1886), 16, object);
			break;

		case kObjectInfernoSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1905), 12, object);
			break;

		case kObjectJuggler:
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1906), 12, object);
			break;

		case kObjectClownSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1907), 10, object);
			break;

		case kObjectTrixieSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1908), 12, object);
			break;

		case kObjectSimonSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1909), 12, object);
			break;

		case kObjectFunTixSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1913), 10, object);
			break;

		case kObjectFreakTixSQUp:
		case kObjectFreakTixSQDown:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1914), 9, object);
			break;

		case kObjectFortTellerSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1915), 15, object);
			break;

		case kObjectRingTossSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1916), 10, object);
			break;

		case kObjectPigShootSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1917), 10, object);
			break;

		case kObjectKnockDownSQ:
			if (_vm->isGameFlagNotSet(kGameFlag358) && _vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1918), 10, object);
			break;

		case kObjectPretZoolSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1919), 14, object);
			break;

		case kObjectTimberSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1920), 14, object);
			break;

		case kObjectTwinsSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1921), 20, object);
			break;

		case kObjectSeanSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1923), 16, object);
			break;

		case kObjectMomAndPopSQ:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1852), 15, object);

			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1862), 15, object);
			break;
		}
	} else {
		switch (actorIndex) {
		default:
			break;

		case 1:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1910), 14, actorIndex);
			break;

		case 2:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1911), 14, actorIndex);
			break;

		case 3:
			if (_vm->getRandomBit())
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1912), 14, actorIndex);
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

	case kObjectGravinSQCyber:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1880), 14, object);
		break;

	case kObjectGravinWorkMachine:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1880), 14, object);
		break;

	case kObjectNPC062GritzaSQ:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1881), 12, object);
		break;

	case kObjectNPC063GrundleSQ:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1882), 12, object);
		break;

	case kObjectNPC064GrellaSQ:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1883), 12, object);
		break;

	case kObjectNPC065SQ:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1868), 12, object);
		break;

	case kObjectNPC066SQ:
		if (_vm->getRandomBit())
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1867), 12, object);
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
				playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1865), 18, object);
		break;

	case kObjectOldMan3:
		if (rnd(100) < 20)
			playSoundPanning(MAKE_RESOURCE(kResourcePackSpeechOther, 1866), 18, object);
		break;
	}
}

void Special::playSoundChapter8(Object *object, ActorIndex actorIndex) {
	error("[Special::playSoundChapter8] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Special::setPaletteGamma(ResourceId palette1, ResourceId palette2) {
	error("[Special::setPaletteGamma] Not implemented!");
}

void Special::playSoundPanning(ResourceId resourceId, int32 attenuation, Object *object) {
	error("[Special::playSoundPanning] Not implemented!");
}

void Special::playSoundPanning(ResourceId resourceId, int32 attenuation, ActorIndex actorIndex) {
	error("[Special::playSoundPanning] Not implemented!");
}

void Special::updateObjectFlag(ObjectId id) {
	error("[Special::updateObjectFlag] Not implemented!");
}

void Special::checkFlags(ObjectId id, GameFlag flag1, GameFlag flag2, GameFlag flag3, GameFlag flag4, int *val1, int *val2, GameFlag flag5, int *val3) {
	error("[Special::checkFlags] Not implemented!");
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
