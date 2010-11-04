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

#include "asylum/resources/actor.h"

#include "asylum/resources/actionlist.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/screen.h"
#include "asylum/system/config.h"

#include "asylum/staticres.h"

#include "common/endian.h"

namespace Asylum {

Actor::Actor(AsylumEngine *engine, ActorIndex index) : _vm(engine), _index(index) {
	// TODO Init all variables

	// update-related variables
	_actorUpdateCounter = 0;
	_enableFromStatus7 = false;
}

Actor::~Actor() {
	// TODO destroy data

	// Zero passed pointers
	_vm = NULL;
}

/////////////////////////////////////////////////////////////////////////
// Loading
//////////////////////////////////////////////////////////////////////////
void Actor::load(Common::SeekableReadStream *stream) {
	if (!stream)
		error("[Actor::load] invalid stream");

	x                 = stream->readSint32LE();
	y                 = stream->readSint32LE();
	_resourceId       = stream->readSint32LE();
	_objectIndex     = stream->readSint32LE();
	_frameIndex      = stream->readSint32LE();
	_frameCount       = stream->readSint32LE();
	x1                = stream->readSint32LE();
	y1                = stream->readSint32LE();
	x2                = stream->readSint32LE();
	y2                = stream->readSint32LE();

	_boundingRect.left   = stream->readSint32LE() & 0xFFFF;
	_boundingRect.top    = stream->readSint32LE() & 0xFFFF;
	_boundingRect.right  = stream->readSint32LE() & 0xFFFF;
	_boundingRect.bottom = stream->readSint32LE() & 0xFFFF;

	_direction  = stream->readSint32LE();
	_field_3C   = stream->readSint32LE();
	_status     = (ActorStatus)stream->readSint32LE();
	_field_44   = stream->readSint32LE();
	_priority   = stream->readSint32LE();
	flags       = stream->readSint32LE();
	_field_50   = stream->readSint32LE();
	_field_54   = stream->readSint32LE();
	_field_58   = stream->readSint32LE();
	_field_5C   = stream->readSint32LE();
	_field_60   = stream->readSint32LE();
	_actionIdx3 = stream->readSint32LE();

	// TODO skip field_68 till field_617
	stream->skip(0x5B0);

	for (int32 i = 0; i < 8; i++)
		_reaction[i] = stream->readSint32LE();

	_field_638     = stream->readSint32LE();
	_walkingSound1 = stream->readSint32LE();
	_walkingSound2 = stream->readSint32LE();
	_walkingSound3 = stream->readSint32LE();
	_walkingSound4 = stream->readSint32LE();
	_field_64C     = stream->readSint32LE();
	_field_650     = stream->readSint32LE();

	for (int32 i = 0; i < 55; i++)
		_graphicResourceIds[i] = stream->readSint32LE();

	stream->read(_name, sizeof(_name));

	for (int32 i = 0; i < 20; i++)
		_field_830[i] = stream->readSint32LE();

	for (int32 i = 0; i < 20; i++)
		_field_880[i] = stream->readSint32LE();

	for (int32 i = 0; i < 20; i++)
		_field_8D0[i] = stream->readSint32LE();

	_actionIdx2 = stream->readSint32LE();
	_field_924  = stream->readSint32LE();
	_lastScreenUpdate = stream->readSint32LE();
	_field_92C  = stream->readSint32LE();
	actionType     = stream->readSint32LE();
	_field_934  = stream->readSint32LE();
	_field_938  = stream->readSint32LE();
	_soundResourceId = stream->readSint32LE();
	_numberValue01 = stream->readSint32LE();
	_field_944  = stream->readSint32LE();
	_field_948  = stream->readSint32LE();
	_field_94C  = stream->readSint32LE();
	_numberFlag01 = stream->readSint32LE();
	_numberStringWidth  = stream->readSint32LE();
	_numberStringX  = stream->readSint32LE();
	_numberStringY  = stream->readSint32LE();
	stream->read(_numberString01, sizeof(_numberString01));
	_field_964  = stream->readSint32LE();
	_field_968  = stream->readSint32LE();
	_field_96C  = stream->readSint32LE();
	_field_970  = stream->readSint32LE();
	_field_974  = stream->readSint32LE();
	_field_978  = stream->readSint32LE();
	_actionIdx1 = stream->readSint32LE();
	_field_980  = stream->readSint32LE();
	_field_984  = stream->readSint32LE();
	_field_988  = stream->readSint32LE();
	_field_98C  = stream->readSint32LE();
	_field_990  = stream->readSint32LE();
	_field_994  = stream->readSint32LE();
	_field_998  = stream->readSint32LE();
	_field_99C  = stream->readSint32LE();
	_field_9A0  = stream->readSint32LE();

	// TODO skip field_980 till field_9A0
	stream->skip(0x24);
}

/////////////////////////////////////////////////////////////////////////
// Visibility
//////////////////////////////////////////////////////////////////////////
void Actor::setVisible(bool value) {
	if (value)
		flags |= kActorFlagVisible;
	else
		flags &= ~kActorFlagVisible;

	stopSound();
}

/////////////////////////////////////////////////////////////////////////
// Update & status
//////////////////////////////////////////////////////////////////////////
void Actor::draw() {
	if (!isVisible())
		return;

	// Draw the actor
	Common::Point point;
	getScene()->adjustCoordinates(x + x1, y + y1, &point);

	// Compute frame index
	int32 frameIndex = _frameIndex;
	if (_frameIndex >= _frameCount)
		frameIndex = 2 * _frameCount - _frameIndex - 1;

	if (LOBYTE(flags) & kActorFlagMasked) {
		Object *object = getWorld()->objects[_objectIndex];
		getScene()->adjustCoordinates(object->x, object->y, &point);

		error("[Actor::draw] Cross fade not implemented!");
		//getScreen()->addGraphicToQueue(_resourceId, frameIndex, point.x, point.y, object->_resourceId, point.x, point.y, getGraphicsFlags(), _priority);

		// Update flags
		flags &= ~kActorFlagMasked;
	} else {
		getScreen()->addGraphicToQueue(_resourceId, frameIndex, point.x, point.y, getGraphicsFlags(), _field_96C, _priority);
	}
}

void Actor::update() {
	if (!isVisible())
		return;

	switch (_status) {
	default:
		break;

	case kActorStatus16:
		if (getWorld()->numChapter == 2) {
			updateStatus16_Chapter2();
		} else if (getWorld()->numChapter == 11 && _index == getScene()->getPlayerActorIndex()) {
			updateStatus16_Chapter11();
		}
		break;

	case kActorStatus17:
		if (getWorld()->numChapter == 2) {
			if (_index > 12) {
				if (_frameIndex <= _frameCount - 1) {
					++_frameIndex;
				} else {
					setVisible(false);
					getScene()->getActor(_index + 9)->setVisible(false);
				}
			}

			if (_index == 11) {
				if (_frameIndex <= _frameCount - 1) {
					// Looks like a simple check using the counter, since it doesn't seem to be used anywhere else
					if (_actorUpdateCounter <= 0) {
						++_actorUpdateCounter;
					} else {
						_actorUpdateCounter = 0;
						++_frameIndex;
					}
				} else {
					if (_vm->isGameFlagSet(kGameFlag556)) {
						Actor *player = getScene()->getActor();

						getScene()->speech()->play(453);
						setVisible(false);

						player->updateStatus(kActorStatus3);
						player->setResourceId(player->getResourcesId(35));
						player->setDirection(4);
						GraphicResource *resource = new GraphicResource(getScene()->getResourcePack(), player->getResourceId());
						player->setFrameCount(resource->getFrameCount());
						delete resource;

						getScene()->getCursor()->hide();
						getScene()->getActor(0)->updateFromDirection(4);

						// Queue script
						getScene()->actions()->queueScript(getWorld()->getActionAreaById(2696)->scriptIndex, getScene()->getPlayerActorIndex());

						_vm->setGameFlag(kGameFlag279);
						_vm->setGameFlag(kGameFlag368);

						player->setFrameIndex(0);
						getScene()->getActor(0)->setLastScreenUpdate(_vm->getTick());

						Sound *sound  = _vm->sound();
						if (sound->isCacheOk())
							sound->playMusic(getScene()->getResourcePack(), (int)kResourceMusic_80020001);

						getWorld()->musicCurrentResourceId = 1;

						if (sound->isPlaying(getWorld()->soundResourceIds[7]))
							sound->stopSound(getWorld()->soundResourceIds[7]);

						if (sound->isPlaying(getWorld()->soundResourceIds[6]))
							sound->stopSound(getWorld()->soundResourceIds[6]);

						if (sound->isPlaying(getWorld()->soundResourceIds[5]))
							sound->stopSound(getWorld()->soundResourceIds[5]);

						_vm->setGameFlag(kGameFlag1131);
					} else {
						updateGraphicData(25);
						_vm->setGameFlag(kGameFlag556);
					}
				}
			}

			if (_index == getScene()->getPlayerActorIndex()) {
				if (_frameIndex <= _frameCount - 1) {
					++_frameIndex;
				} else {
					_vm->clearGameFlag(kGameFlag239);
					getScene()->getActor(10)->updateStatus(kActorStatus14);
					setVisible(false);
					_vm->setGameFlag(kGameFlag238);

					// Queue script
					getScene()->actions()->queueScript(getWorld()->getActionAreaById(1000)->scriptIndex, getScene()->getPlayerActorIndex());
				}
			}

		} else if (getWorld()->numChapter == 11) {
			if (_index == getScene()->getPlayerActorIndex()) {
				if (_frameIndex <= _frameCount - 1)
					++_frameIndex;
				else
					getScene()->resetActor0();
			}

			if (_index >= 10)
				updateStatus17_Chapter2();
		}
		break;

	case kActorStatus15:
		if (getWorld()->numChapter == 2) {
			if (_index > 12)
				updateStatus15_Chapter2();

			if (_index == getScene()->getPlayerActorIndex())
				updateStatus15_Chapter2_Player();

			if (_index == 11)
				updateStatus15_Chapter2_Actor11();

		} else if (getWorld()->numChapter == 11) {
			if (_index >= 10 && _index < 16)
				updateStatus15_Chapter11();

			if (_index == getScene()->getPlayerActorIndex())
				updateStatus15_Chapter11_Player();
		}
		break;

	case kActorStatus18:
		if (getWorld()->numChapter == 2) {
			if (_index > 12)
				updateStatus18_Chapter2();

			if (_index == 11)
				updateStatus18_Chapter2_Actor11();
		}
		break;

	case kActorStatusDisabled:
		_frameIndex = (_frameIndex + 1) % _frameCount;

		if (_vm->screenUpdatesCount - _lastScreenUpdate > 300) {
			if (_vm->getRandom(100) < 50) {
				if (!getSound()->soundResourceId || !getSound()->isPlaying(getSound()->soundResourceId)) {
					if (isDefaultDirection(10))
						updateStatus(kActorStatus9);
				}
			}
			_lastScreenUpdate = _vm->screenUpdatesCount;
		}
		break;

	case kActorStatus12:
		if (getWorld()->numChapter == 2) {
			if (_index > 12)
				updateStatus12_Chapter2();

			if (_index == 11)
				updateStatus12_Chapter2_Actor11();

			return;
		} else if (getWorld()->numChapter == 11) {
			switch (_index)	{
			default:
				break;

			case 1:
				updateStatus12_Chapter11_Actor1();
				return;

			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				updateStatus12_Chapter11();
				return;

			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
				return;
			}
		}
		// Fallback to next case

	case kActorStatus1:
		error("[Actor::update] kActorStatus1 / kActorStatus12 case not implemented");
		break;

	case kActorStatus2:
	case kActorStatus13:
		// TODO: do actor direction
		error("[Actor::update] kActorStatus2 / kActorStatus13 case not implemented");
		break;

	case kActorStatus3:
	case kActorStatus19:
		updateStatus3_19();
		break;

	case kActorStatus7:
		if (_enableFromStatus7) {
			_enableFromStatus7 = false;
			updateStatus(kActorStatusEnabled);
		}
		break;

	case kActorStatusEnabled:
		if (_field_944 != 5)
			updateStatusEnabled();
		break;

	case kActorStatus14:
		void updateStatus14();
		break;

	case kActorStatus21:
		void updateStatus21();
		break;

	case kActorStatus9:
		void updateStatus9();
		break;

	case kActorStatus6:
	case kActorStatus10:
		_frameIndex = (_frameIndex + 1) % _frameCount;
		break;

	case kActorStatus8:
		if (_vm->encounter()->getFlag(kEncounterFlag2)
		 || !_soundResourceId
		 || getSound()->isPlaying(_soundResourceId)) {
			_frameIndex = (_frameIndex + 1) % _frameCount;
		} else {
			updateStatus(kActorStatusEnabled);
			_soundResourceId = kResourceNone;
		}
		break;
	}

	if (_soundResourceId && getSound()->isPlaying(_soundResourceId))
		setVolume();

	if (_index != getScene()->getPlayerActorIndex() && getWorld()->numChapter != 9)
		error("[Actor::update] call to actor sound functions missing!");

	updateDirection();

	if (_field_944 != 5)
		updateFinish();
}


void Actor::updateStatus(ActorStatus actorStatus) {
	switch (actorStatus) {
	default:
		break;

	case kActorStatus1:
	case kActorStatus12:
		if ((getWorld()->numChapter == 2
		 && _index == getScene()->getPlayerActorIndex() && (_status == kActorStatus18 || _status == kActorStatus16 || kActorStatus17))
		 || (_status != kActorStatusEnabled && _status != kActorStatus9 && _status != kActorStatus14 && _status != kActorStatus15 && _status != kActorStatus18))
			return;

		updateGraphicData(0);

		// Force status in some cases
		if (_status == kActorStatus14 || _status == kActorStatus15 || _status == kActorStatus18) {
			_status = kActorStatus12;
			return;
		}
		break;

	case kActorStatus2:
	case kActorStatus13:
		updateGraphicData(0);
		break;

	case kActorStatus3:
	case kActorStatus19:
		if (!strcmp(_name, "Big Crow"))
			_status = kActorStatusEnabled;
		break;

	case kActorStatusEnabled:
	case kActorStatus6:
	case kActorStatus14:
		updateGraphicData(5);
		break;

	case kActorStatusDisabled:
		updateGraphicData(15);
		_resourceId = _graphicResourceIds[(_direction > 4 ? 8 - _direction : _direction) + 15];

		// TODO set word_446EE4 to -1. This global seems to be used with screen blitting
		break;

	case kActorStatus7:
		if (getWorld()->numChapter == 2 && _index == 10 && _vm->isGameFlagSet(kGameFlag279)) {
			Actor *actor = getScene()->getActor(0);
			actor->x1 = x2 + x1 - actor->x2;
			actor->y1 = y2 + y1 - actor->y2;
			actor->setDirection(4);

			getScene()->setPlayerActorIndex(0);

			// Hide this actor and the show the other one
			setVisible(false);
			actor->setVisible(true);

			_vm->clearGameFlag(kGameFlag279);

			getScene()->getCursor()->show();
		}
		break;

	case kActorStatus8:
	case kActorStatus10:
	case kActorStatus17:
		updateGraphicData(20);
		break;

	case kActorStatus9:
		if (_vm->encounter()->getFlag(kEncounterFlag2))
			return;

		if (_vm->getRandomBit() == 1 && isDefaultDirection(15))
			updateGraphicData(15);
		else
			updateGraphicData(10);
		break;

	case kActorStatus15:
	case kActorStatus16:
		updateGraphicData(actorStatus == kActorStatus15 ? 10 : 15);
		break;

	case kActorStatus18:
		if (getWorld()->numChapter == 2) {
			GraphicResource *resource = new GraphicResource();
			_frameIndex = 0;

			if (_index > 12)
				_resourceId = _graphicResourceIds[_direction + 30];

			if (getScene()->getPlayerActorIndex() == _index) {
				resource->load(getScene()->getResourcePack(), _resourceId);
				_frameIndex = resource->getFrameCount() - 1;
			}

			if (_index == 11)
				_resourceId = _graphicResourceIds[getScene()->getGlobalDirection() > 4 ? 8 - getScene()->getGlobalDirection() : getScene()->getGlobalDirection()];

			// Reload the graphic resource if the resource ID has changed
			if (resource->getResourceId() != _resourceId)
				resource->load(getScene()->getResourcePack(), _resourceId);

			_frameCount = resource->getFrameCount();
		}
		break;
	}

	_status = actorStatus;
}

/////////////////////////////////////////////////////////////////////////
// Direction & position
//////////////////////////////////////////////////////////////////////////

void Actor::updateDirection() {
	if(_field_970) {
		// TODO
		// This update is only ever done if action script 0x5D is called, and
		// the resulting switch sets field_970. Investigate 401A30 for further
		// details
		error("[Actor::updateDirection] logic not implemented");
	}
}

void Actor::updateFromDirection(ActorDirection actorDirection) {
	_direction = actorDirection;

	if (_field_944 == 5)
		return;

	switch (_status) {
	default:
		break;

	case kActorStatusDisabled:
	case kActorStatusEnabled:
	case kActorStatus14: {
		_resourceId = _graphicResourceIds[(actorDirection > 4 ? 8 - actorDirection : actorDirection) + 5];

		// FIXME this seems kind of wasteful just to grab a frame count
		GraphicResource *gra = new GraphicResource(getScene()->getResourcePack(), _resourceId);
		_frameCount = gra->getFrameCount();
		delete gra;
		}
		break;

	case kActorStatus18:
		if (getWorld()->numChapter == 2) {
			if (_index == 11) { // we are actor 11
				if (actorDirection > 4)
					_resourceId = _graphicResourceIds[8 - actorDirection];
				else
					_resourceId = _graphicResourceIds[actorDirection];
			}
		}
		break;

	case kActorStatus1:
	case kActorStatus2:
	case kActorStatus12:
		_resourceId = _graphicResourceIds[(actorDirection > 4 ? 8 - actorDirection : actorDirection)];
		break;

	case kActorStatus8:
		_resourceId = _graphicResourceIds[(actorDirection > 4 ? 8 - actorDirection : actorDirection) + 20];
		break;
	}
}

void Actor::faceTarget(ObjectId id, DirectionFrom from) {
	debugC(kDebugLevelActor, "[Actor::faceTarget] Facing target %d using direction from %d", id, from);

	int32 newX, newY;

	switch (from) {
	default:
		error("[Actor::faceTarget] Invalid direction input: %d (should be 0-3)", from);
		return;

	case kDirectionFromObject: {
		Object *object = getWorld()->getObjectById(id);
		if (!object) {
			warning("[Actor::faceTarget] No Object found for id %d", id);
			return;
		}



		GraphicResource *resource = new GraphicResource(getScene()->getResourcePack(), object->getResourceId());
		GraphicFrame *frame = resource->getFrame(object->getFrameIndex());

		newX = (frame->surface.w >> 1) + object->x;
		newY = (frame->surface.h >> 1) + object->y;

		delete resource;
		}
		break;

	case kDirectionFromPolygons: {
		int32 actionIndex = getWorld()->getActionAreaIndexById(id);
		if (actionIndex == -1) {
			warning("[Actor::faceTarget] No ActionArea found for id %d", id);
			return;
		}

		PolyDefinitions *polygon = &getScene()->polygons()->entries[getWorld()->actions[actionIndex]->polyIdx];

		newX = polygon->boundingRect.left + (polygon->boundingRect.right  - polygon->boundingRect.left) / 2;
		newY = polygon->boundingRect.top  + (polygon->boundingRect.bottom - polygon->boundingRect.top)  / 2;
		}
		break;

	case kDirectionFromActor:
		newX = x2 + x1;
		newY = y2 + y1;
		break;

	case kDirectionFromParameters:
		newX = newY = id;
		break;
	}

	updateFromDirection(getDirection(x2 + x1, y2 + y1, newX, newY));
}

void Actor::setPosition(int32 newX, int32 newY, int32 newDirection, int32 frame) {
	x1 = newX - x2;
	y1 = newY - y2;

	if (_direction != 8)
		updateFromDirection(newDirection);

	if (frame > 0)
		_frameIndex = frame;
}


/////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Actor::stopSound() {
	if (_soundResourceId && getSound()->isPlaying(_soundResourceId))
		getSound()->stopSound(_soundResourceId);
}

Common::String Actor::toString() {
	Common::String output;

	output += Common::String::format("Actor %d: %s\n", _index, _name);

	return output;
}

void Actor::setRawResources(uint8 *data) {
	byte *dataPtr = data;

	for (int32 i = 0; i < 60; i++) {
		_resources[i] = (int32)READ_LE_UINT32(dataPtr);
		dataPtr += 4;
	}
}

//////////////////////////////////////////////////////////////////////////
// Unknown methods
//////////////////////////////////////////////////////////////////////////

bool Actor::process(int32 actorX, int32 actorY) {
	error("[Actor::process] not implemented!");
}

void Actor::processStatus(int32 actorX, int32 actorY, bool doSpeech) {
	if (process(actorX, actorY)) {
		if (_status <= kActorStatus11)
			updateStatus(kActorStatus2);
		else
			updateStatus(kActorStatus13);
	} else if (doSpeech) {
		getScene()->playSpeech(1);
	}
}

void Actor::process_401830(int32 field980, int32 actionAreaId, int32 field978, int field98C, int32 field990, int32 field974, int32 param8, int32 param9) {
	error("[Actor::process_401830] not implemented!");
}

bool Actor::process_408B20(Common::Point *point, ActorDirection direction, int count, bool hasDelta) {
	error("[Actor::process_408B20] not implemented!");
}

void Actor::process_41BC00(int32 reactionIndex, int32 numberValue01Add) {
	error("[Actor::process_41BC00] not implemented!");
}

void Actor::process_41BCC0(int32 reactionIndex, int32 numberValue01Substract) {
	error("[Actor::process_41BC00] not implemented!");
}

bool Actor::process_41BDB0(int32 reactionIndex, bool testNumberValue01) {
	error("[Actor::process_41BC00] not implemented!");
}

void Actor::update_40DE20() {
	error("[Actor::update_40DE20] not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Update methods
//////////////////////////////////////////////////////////////////////////

void Actor::updateStatus3_19() {
	error("[Actor::updateStatus3_19] not implemented!");
}

void Actor::updateStatusEnabled() {
	_frameIndex = (_frameIndex + 1) % _frameCount;

	// Actor: Crow
	if (_vm->screenUpdatesCount - _lastScreenUpdate > 300) {
		if (strcmp(_name, "Crow")) {
			if (_vm->getRandom(100) < 50
			 && (!getSound()->soundResourceId || !getSound()->isPlaying(getSound()->soundResourceId))
			 && isDefaultDirection(10))
				updateStatus(kActorStatus9);

			_lastScreenUpdate = _vm->screenUpdatesCount;
		}
	}

	// Actor: Player
	if (_index == getScene()->getPlayerActorIndex()) {
		if (_vm->globalTickValue_2 && (_vm->screenUpdatesCount - _vm->globalTickValue_2) > 500) {

			if (_vm->isGameFlagNotSet(kGameFlagScriptProcessing)
			 && isVisible()
			 && !_vm->encounter()->getFlag(kEncounterFlag2)
			 && !getSound()->soundResourceId) {
				if (_vm->getRandom(100) < 50) {
					if (getWorld()->numChapter == 13)
						getScene()->speech()->play(507);
					else
						getScene()->playSpeech(4);
				}
			}
			_lastScreenUpdate = _vm->screenUpdatesCount;
			_vm->globalTickValue_2 = _vm->screenUpdatesCount;
		}

		return;
	}

	// Actor:: BigCrow
	if (strcmp(_name, "Big Crow")) {
		error("[Actor::updateStatusEnabled] Big Crow logic not implemented!");
		return;
	}

	// All other actors
	if (_vm->getRandom(10) < 5) {
		switch (_vm->getRandom(4)) {
		default:
			break;

		case 0:
			setPosition(10, 1350, 0, 0);
			processStatus(1460, -100, false);
			break;

		case 1:
			setPosition(300, 0, 0, 0);
			processStatus(1700, 1400, false);
			break;

		case 2:
			setPosition(1560, -100, 0, 0);
			processStatus(-300, 1470, false);
			break;

		case 3:
			setPosition(1150, 1400, 0, 0);
			processStatus(-250, 0, false);
			break;
		}
	}
}

void Actor::updateStatus9() {
	error("[Actor::updateStatus9] not implemented!");
}

void Actor::updateStatus12_Chapter2() {
	error("[Actor::updateStatus12_Chapter2] not implemented!");
}

void Actor::updateStatus12_Chapter2_Actor11() {
	error("[Actor::updateStatus12_Chapter2_Actor11] not implemented!");
}

void Actor::updateStatus12_Chapter11_Actor1() {
	error("[Actor::updateStatus12_Chapter11_Actor1] not implemented!");
}

void Actor::updateStatus12_Chapter11() {
	error("[Actor::updateStatus12_Chapter11] not implemented!");
}

void Actor::updateStatus14() {
	error("[Actor::updateStatus14] not implemented!");
}

void Actor::updateStatus15_Chapter2() {
	error("[Actor::updateStatus15_Chapter2] not implemented!");
}

void Actor::updateStatus15_Chapter2_Player() {
	error("[Actor::updateStatus15_Chapter2_Player] not implemented!");
}

void Actor::updateStatus15_Chapter2_Actor11() {
	error("[Actor::updateStatus15_Chapter2_Actor11] not implemented!");
}

void Actor::updateStatus15_Chapter11() {
	error("[Actor::updateStatus15_Chapter11] not implemented!");
}

void Actor::updateStatus15_Chapter11_Player() {
	error("[Actor::updateStatus15_Chapter11_Player] not implemented!");
}

void Actor::updateStatus16_Chapter2() {
	error("[Actor::updateStatus16_Chapter2] not implemented!");
}

void Actor::updateStatus16_Chapter11() {
	error("[Actor::updateStatus16_Chapter11] not implemented!");
}

void Actor::updateStatus17_Chapter2() {
	error("[Actor::updateStatus17_Chapter2] not implemented!");
}

void Actor::updateStatus18_Chapter2() {
	error("[Actor::updateStatus18_Chapter2] not implemented!");
}

void Actor::updateStatus18_Chapter2_Actor11() {
	error("[Actor::updateStatus18_Chapter2_Actor11] not implemented!");
}

void Actor::updateStatus21() {
	error("[Actor::updateStatus21] not implemented!");
}

void Actor::updateFinish() {
	if (_field_944 == 4 || !isVisible())
		return;

	int32 areaIndex = getScene()->findActionArea(Common::Point(x2 + x1, y2 + y1));
	if (areaIndex == _actionIdx3 || areaIndex == -1)
		return;

	ActionArea *area = getWorld()->actions[areaIndex];
	ActionArea *actorArea = getWorld()->actions[_actionIdx3];
	if (!getScene()->actions()->isProcessingSkipped()) {
		getScene()->actions()->queueScript(actorArea->scriptIndex2, _index);
		getScene()->actions()->queueScript(area->scriptIndex, _index);
	}

	if (!area->paletteValue || area->paletteValue == actorArea->paletteValue || _index) {
		if (area->paletteValue != actorArea->paletteValue && !_index)
			_vm->screen()->startPaletteFade(area->paletteValue, 50, 3);

		_actionIdx3 = areaIndex;
	} else {
		_vm->screen()->startPaletteFade(area->paletteValue, 50, 3);
		_actionIdx3 = areaIndex;
	}
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Actor::setVolume() {
	if (!_soundResourceId || !getSound()->isPlaying(_soundResourceId))
		return;

	// Compute volume
	int32 volume = Config.voiceVolume + getSound()->calculateVolumeAdjustement(x2 + x1, y2 + y1, _field_968, 0);
	if (volume < -10000)
		volume = -10000;

	getSound()->setVolume(_soundResourceId, volume);
}

//////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////

ActorDirection Actor::getDirection(int32 ax1, int32 ay1, int32 ax2, int32 ay2) {
	int32 v5 = (ax2 << 16) - (ax1 << 16);
	int32 v6 = 0;
	int32 v4 = (ay1 << 16) - (ay2 << 16);

	if (v5 < 0) {
		v6 = 2;
		v5 = -v5;
	}

	if (v4 < 0) {
		v6 |= 1;
		v4 = -v4;
	}

	int32 v7;
	int32 v8 = -1;

	if (v5) {
		v7 = (v4 << 8) / v5;

		if (v7 < 0x100)
			v8 = angleTable01[v7];
		if (v7 < 0x1000 && v8 < 0)
			v8 = angleTable02[v7 >> 4];
		if (v7 < 0x10000 && v8 < 0)
			v8 = angleTable03[v7 >> 8];
	} else {
		v8 = 90;
	}

	switch (v6) {
	case 1:
		v8 = 360 - v8;
		break;
	case 2:
		v8 = 180 - v8;
		break;
	case 3:
		v8 += 180;
		break;
	}

	if (v8 >= 360)
		v8 -= 360;

	int32 result;

	if (v8 < 157 || v8 >= 202) {
		if (v8 < 112 || v8 >= 157) {
			if (v8 < 67 || v8 >= 112) {
				if (v8 < 22 || v8 >= 67) {
					if ((v8 < 0 || v8 >= 22) && (v8 < 337 || v8 > 359)) {
						if (v8 < 292 || v8 >= 337) {
							if (v8 < 247 || v8 >= 292) {
								if (v8 < 202 || v8 >= 247) {
									error("getAngle returned a bad angle: %d.", v8);
								} else {
									result = 3;
								}
							} else {
								result = 4;
							}
						} else {
							result = 5;
						}
					} else {
						result = 6;
					}
				} else {
					result = 7;
				}
			} else {
				result = 0;
			}
		} else {
			result = 1;
		}
	} else {
		result = 2;
	}

	return result;
}

void Actor::updateGraphicData(uint32 offset) {
	_resourceId = _graphicResourceIds[(_direction > 4 ? 8 - _direction : _direction) + offset];

	GraphicResource *resource = new GraphicResource(getScene()->getResourcePack(), _resourceId);
	_frameCount = resource->getFrameCount();
	delete resource;

	_frameIndex = 0;
}

bool Actor::isDefaultDirection(int index) {
	return _graphicResourceIds[index] != _graphicResourceIds[5];
}

int32 Actor::getGraphicsFlags() {
	if (getWorld()->numChapter == 11) {
		int res = strcmp((char *)&_name, "Dead Sarah");

		if (res == 0)
			return res;
	}

	// TODO replace by readable version
	return ((_direction < 5) - 1) & 2;
}

} // end of namespace Asylum
