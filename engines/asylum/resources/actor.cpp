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
 */

#include "asylum/resources/actor.h"

#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/polygons.h"
#include "asylum/resources/reaction.h"
#include "asylum/resources/special.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/staticres.h"

#include "common/endian.h"

namespace Asylum {

Actor::Actor(AsylumEngine *engine, ActorIndex index) : _vm(engine), _index(index) {
 	// Init all variables
 	_resourceId = kResourceNone;
 	_objectIndex = 0;
 	_frameIndex = 0;
 	_frameCount = 0;
 	_direction = kDirectionN;
 	_field_3C = 0;
 	_status = kActorStatusNone;
 	_field_44 = 0;
 	_priority = 0;
 	flags = 0;
 	_field_50 = 0;
 	_field_54 = 0;
 	_field_58 = 0;
 	_field_5C = 0;
 	_field_60 = 0;
 	_actionIdx3 = 0;
 	// TODO field_68 till field_617
 	memset(&_reaction, 0, sizeof(_reaction));
 	_field_638 = 0;
 	_walkingSound1 = 0;
 	_walkingSound2 = 0;
 	_walkingSound3 = 0;
 	_walkingSound4 = 0;
 	_field_64C = 0;
 	_field_650 = 0;
 	memset(_graphicResourceIds, 0 , sizeof(_graphicResourceIds));
 	memset(&_name, 0, sizeof(_name));
 	memset(&_field_830, 0, sizeof(_field_830));
 	memset(&_field_880, 0, sizeof(_field_880));
 	memset(&_field_8D0, 0, sizeof(_field_8D0));
 	_actionIdx2 = 0;
 	_field_924 = 0;
 	_lastScreenUpdate = 0;
 	_scriptIndex = 0;
 	actionType = 0;
 	_field_934 = 0;
 	_field_938 = 0;
 	_soundResourceId = kResourceNone;
 	_numberValue01 = 0;
 	_field_944 = 0;
 	_field_948 = 0;
 	_field_94C = 0;
 	_numberFlag01 = 0;
 	_numberStringWidth = 0;
 	_numberStringX = 0;
 	_numberStringY = 0;
 	memset(&_numberString01, 0, 8);
 	_field_968 = 0;
 	_field_96C = 0;
 	_field_970 = 0;
 	_field_974 = 0;
 	_field_978 = 0;
 	_actionIdx1 = 0;
 	_field_980 = 0;
 	_field_984 = 0;
 	_field_988 = 0;
 	_field_98C = 0;
 	_field_990 = 0;
 	_field_994 = 0;
 	_field_998 = 0;
 	_field_99C = 0;
 	_field_9A0 = 0;

	// Instance data
	_tickCount = -1;
	_updateCounter = 0;

	// Path finding
	_frameNumber = 0;
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

	_point.x              = stream->readSint32LE();
	_point.y              = stream->readSint32LE();
	_resourceId          = (ResourceId)stream->readSint32LE();
	_objectIndex         = stream->readSint32LE();
	_frameIndex          = stream->readUint32LE();
	_frameCount          = stream->readUint32LE();
	_point1.x             = stream->readSint32LE();
	_point1.y             = stream->readSint32LE();
	_point2.x             = stream->readSint32LE();
	_point2.y             = stream->readSint32LE();

	_boundingRect.left   = (int16)(stream->readSint32LE() & 0xFFFF);
	_boundingRect.top    = (int16)(stream->readSint32LE() & 0xFFFF);
	_boundingRect.right  = (int16)(stream->readSint32LE() & 0xFFFF);
	_boundingRect.bottom = (int16)(stream->readSint32LE() & 0xFFFF);

	_direction  = (ActorDirection)stream->readSint32LE();
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
	_field_64C     = stream->readUint32LE();
	_field_650     = stream->readUint32LE();

	for (int32 i = 0; i < 55; i++)
		_graphicResourceIds[i] = (ResourceId)stream->readSint32LE();

	stream->read(_name, sizeof(_name));

	for (int32 i = 0; i < 20; i++)
		_field_830[i] = stream->readSint32LE();

	for (int32 i = 0; i < 20; i++)
		_field_880[i] = stream->readSint32LE();

	for (int32 i = 0; i < 20; i++)
		_field_8D0[i] = stream->readSint32LE();

	_actionIdx2 = stream->readSint32LE();
	_field_924  = stream->readSint32LE();
	_lastScreenUpdate = stream->readUint32LE();
	_scriptIndex  = stream->readSint32LE();
	actionType     = stream->readSint32LE();
	_field_934  = stream->readSint32LE();
	_field_938  = stream->readSint32LE();
	_soundResourceId = (ResourceId)stream->readSint32LE();
	_numberValue01 = stream->readSint32LE();
	_field_944  = stream->readSint32LE();
	_field_948  = stream->readSint32LE();
	_field_94C  = stream->readSint32LE();
	_numberFlag01 = stream->readSint32LE();
	_numberStringWidth  = stream->readSint32LE();
	_numberStringX  = stream->readSint32LE();
	_numberStringY  = stream->readSint32LE();
	stream->read(_numberString01, sizeof(_numberString01));
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
}

void Actor::loadData(Common::SeekableReadStream *stream) {
	_data.count = stream->readUint32LE();

	_data.current = stream->readUint32LE();

	for (int32 i = 0; i < 120; i++) {
		_data.points[i].x = stream->readSint32LE();
		_data.points[i].y = stream->readSint32LE();
	}

	for (int32 i = 0; i < 120; i++)
		_data.directions[i] = (ActorDirection)stream->readSint32LE();
}

/////////////////////////////////////////////////////////////////////////
// Update & status
//////////////////////////////////////////////////////////////////////////
void Actor::draw() {
	if (!isVisible())
		return;

	// Draw the actor
	Common::Point point;
	adjustCoordinates(&point);
	point.x += _point.x;
	point.y += _point.y;

	// Compute frame index
	uint32 frameIndex = _frameIndex;
	if (_frameIndex >= _frameCount)
		frameIndex = 2 * _frameCount - (_frameIndex + 1);

	if (flags & kActorFlagMasked) {
		Object *object = getWorld()->objects[_objectIndex];
		Common::Point objPoint;
		object->adjustCoordinates(&objPoint);

		getScreen()->addGraphicToQueueMasked(_resourceId, frameIndex, point, object->getResourceId(), objPoint, getGraphicsFlags(), _priority);

		// Update flags
		flags &= ~kActorFlagMasked;
	} else {
		getScreen()->addGraphicToQueue(_resourceId, frameIndex, point, getGraphicsFlags(), _field_96C, _priority);
	}
}

void Actor::drawNumber() {
	if (!_numberFlag01)
		return;

	getText()->loadFont(getWorld()->font1);
	getText()->drawCentered(_numberStringX, _numberStringY, _numberStringWidth, (char *)&_numberString01);
}

void Actor::update() {
	if (!isVisible())
		return;

	switch (_status) {
	default:
		break;

	case kActorStatus16:
		if (_index != getScene()->getPlayerIndex())
			break;

		if (getWorld()->chapter == 2) {
			updateStatus16_Chapter2();
		} else if (getWorld()->chapter == 11) {
			updateStatus16_Chapter11();
		}
		break;

	case kActorStatus17:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12) {
				if (_frameIndex <= _frameCount - 1) {
					++_frameIndex;
				} else {
					hide();
					getScene()->getActor(_index + 9)->hide();
				}
			}

			if (_index == 11) {
				if (_frameIndex <= _frameCount - 1) {
					// Looks like a simple check using the counter, since it doesn't seem to be used anywhere else
					if (_updateCounter <= 0) {
						++_updateCounter;
					} else {
						_updateCounter = 0;
						++_frameIndex;
					}
				} else {
					if (_vm->isGameFlagSet(kGameFlag556)) {
						Actor *player = getScene()->getActor();

						getSpeech()->playPlayer(453);
						hide();

						player->updateStatus(kActorStatus3);
						player->setResourceId(player->getResourcesId(35));
						player->setDirection(kDirectionS);
						player->setFrameCount(GraphicResource::getFrameCount(_vm, player->getResourceId()));

						getCursor()->hide();
						getScene()->getActor(0)->updateFromDirection(kDirectionS);

						// Queue script
						getScript()->queueScript(getWorld()->getActionAreaById(2696)->scriptIndex, getScene()->getPlayerIndex());

						_vm->setGameFlag(kGameFlag279);
						_vm->setGameFlag(kGameFlag368);

						player->setFrameIndex(0);
						getScene()->getActor(0)->setLastScreenUpdate(_vm->getTick());

						getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, 1));

						getWorld()->musicCurrentResourceIndex = 1;

						if (getSound()->isPlaying(getWorld()->soundResourceIds[7]))
							getSound()->stop(getWorld()->soundResourceIds[7]);

						if (getSound()->isPlaying(getWorld()->soundResourceIds[6]))
							getSound()->stop(getWorld()->soundResourceIds[6]);

						if (getSound()->isPlaying(getWorld()->soundResourceIds[5]))
							getSound()->stop(getWorld()->soundResourceIds[5]);

						_vm->setGameFlag(kGameFlag1131);
					} else {
						updateGraphicData(25);
						_vm->setGameFlag(kGameFlag556);
					}
				}
			}

			if (_index == getScene()->getPlayerIndex()) {
				if (_frameIndex <= _frameCount - 1) {
					++_frameIndex;
				} else {
					_vm->clearGameFlag(kGameFlag239);
					getScene()->getActor(10)->updateStatus(kActorStatus14);
					hide();
					_vm->setGameFlag(kGameFlag238);

					// Queue script
					getScript()->queueScript(getWorld()->getActionAreaById(1000)->scriptIndex, getScene()->getPlayerIndex());
				}
			}

		} else if (getWorld()->chapter == kChapter11) {
			if (_index == getScene()->getPlayerIndex()) {
				if (_frameIndex <= _frameCount - 1)
					++_frameIndex;
				else
					resetActors();
			}

			if (_index >= 10)
				updateStatus17_Chapter2();
		}
		break;

	case kActorStatus15:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12)
				updateStatus15_Chapter2();

			if (_index == getScene()->getPlayerIndex())
				updateStatus15_Chapter2_Player();

			if (_index == 11)
				updateStatus15_Chapter2_Actor11();

		} else if (getWorld()->chapter == kChapter11) {
			if (_index >= 10 && _index < 16)
				updateStatus15_Chapter11();

			if (_index == getScene()->getPlayerIndex())
				updateStatus15_Chapter11_Player();
		}
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12)
				updateStatus18_Chapter2();

			if (_index == 11)
				updateStatus18_Chapter2_Actor11();
		}
		break;

	case kActorStatusDisabled:
		_frameIndex = (_frameIndex + 1) % _frameCount;

		if (_vm->screenUpdateCount - _lastScreenUpdate > 300) {
			if (_vm->getRandom(100) < 50) {
				if (!getSpeech()->getSoundResourceId() || !getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
					if (isDefaultDirection(10))
						updateStatus(kActorStatus9);
				}
			}
			_lastScreenUpdate = _vm->screenUpdateCount;
		}
		break;

	case kActorStatus12:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12)
				updateStatus12_Chapter2();

			if (_index == 11)
				updateStatus12_Chapter2_Actor11();

			return;
		} else if (getWorld()->chapter == kChapter11) {
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

	case kActorStatus1: {
		uint32 index = (_frameIndex >= _frameCount) ? 2 * _frameCount - (_frameIndex + 1) : _frameIndex;

		// FIXME the original tests for != 0 and sets to an unknown value (offset to actor in structure?)
		uint32 dist = abs((double)getDistanceForFrame(_direction, index));
		if (!dist)
			error("[Actor::update] Invalid distance (kActorStatus1)");

		Common::Point point = _point1 + _point2;

		if (process_408B20(&point, _direction, dist, false)) {
			playSounds(_direction, dist);
		} else if (process_408B20(&point, (ActorDirection)((_direction + 1) % 7), dist, false)) {
			playSounds((ActorDirection)((_direction + 1) % 7), dist);
		} else if (process_408B20(&point, (ActorDirection)((_direction + 7) % 7), dist, false)) {
			playSounds((ActorDirection)((_direction + 7) % 7), dist);
		} else if (process_408B20(&point, (ActorDirection)((_direction + 2) % 7), dist, false)) {
			playSounds((ActorDirection)((_direction + 2) % 7), dist);
		} else if (process_408B20(&point, (ActorDirection)((_direction + 6) % 7), dist, false)) {
			playSounds((ActorDirection)((_direction + 6) % 7), dist);
		}

		// Finish
		if (_soundResourceId != kResourceNone && getSound()->isPlaying(_soundResourceId))
			setVolume();

		if (_index != getScene()->getPlayerIndex() && getWorld()->chapter != kChapter9)
			getSpecial()->run(NULL, _index);

		updateDirection();

		if (_field_944 != 5)
			updateFinish();

		}
		break;

	case kActorStatus2:
	case kActorStatus13: {
		uint32 index = (_frameIndex >= _frameCount) ? 2 * _frameCount - (_frameIndex + 1) : _frameIndex;

		// FIXME the original tests for != 0 and sets to an unknown value (offset to actor in structure?)
		uint32 dist = abs((double)getDistanceForFrame(_direction, index));
		if (!dist)
			error("[Actor::update] Invalid distance (kActorStatus2/kActorStatus13)");

		Common::Point point = _point1 + _point2;
		Common::Point current = _data.points[_data.current];

		if (point.x < (int16)(current.x - (dist - 1))
		 || point.x > (int16)(current.x + (dist - 1))
		 || point.y < (int16)(current.y - (dist - 1))
		 || point.y > (int16)(current.y + (dist - 1))) {
			if (process_408B20(&point, _direction, dist, false)) {
				playSounds(_direction, dist);
			} else {
				update_409230();
			}
		} else {
			int32 area = getScene()->findActionArea(kActionAreaType1, current);
			if (_field_944 == 1 || _field_944 == 4)
				area = 1;

			if (area == -1  || _data.current >= (int32)(_data.count - 1)) {
				update_409230();
			} else {
				_frameIndex = (_frameIndex + 1) % _frameCount;

				if (process_4103B0(&current, _direction)) {
					_point1.x = dist - _point2.x;
					_point1.y = current.y - _point2.y;

					if (_data.current < (int32)(_data.count - 1)) {
						_data.current++;

						updateFromDirection(_data.directions[_data.current]);
					} else {
						update_409230();
					}
				} else {
					update_409230();
				}
			}
		}

		// Finish
		if (_soundResourceId != kResourceNone && getSound()->isPlaying(_soundResourceId))
			setVolume();

		if (_index != getScene()->getPlayerIndex() && getWorld()->chapter != kChapter9)
			getSpecial()->run(NULL, _index);

		updateDirection();

		if (_field_944 != 5)
			updateFinish();

		}
		break;

	case kActorStatus3:
	case kActorStatus19:
		updateStatus3_19();
		break;

	case kActorStatus7:
		if (getSharedData()->actorEnableForStatus7) {
			getSharedData()->actorEnableForStatus7 = false;
			enable();
		}
		break;

	case kActorStatusEnabled:
		if (_field_944 != 5)
			updateStatusEnabled();
		break;

	case kActorStatus14:
		updateStatus14();
		break;

	case kActorStatus21:
		updateStatus21();
		break;

	case kActorStatus9:
		updateStatus9();
		break;

	case kActorStatus6:
	case kActorStatus10:
		_frameIndex = (_frameIndex + 1) % _frameCount;
		break;

	case kActorStatus8:
		if (_vm->encounter()->isRunning()
		 || !_soundResourceId
		 || getSound()->isPlaying(_soundResourceId)) {
			_frameIndex = (_frameIndex + 1) % _frameCount;
		} else {
			enable();
			_soundResourceId = kResourceNone;
		}
		break;
	}

	if (_soundResourceId && getSound()->isPlaying(_soundResourceId))
		setVolume();

	if (_index != getScene()->getPlayerIndex() && getWorld()->chapter != kChapter9)
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
		if ((getWorld()->chapter == kChapter2
		 && _index == getScene()->getPlayerIndex() && (_status == kActorStatus18 || _status == kActorStatus16 || _status == kActorStatus17))
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
		_resourceId = _graphicResourceIds[(_direction > kDirectionS ? kDirection8 - _direction : _direction) + 15];

		getScreen()->setFlag(-1);
		break;

	case kActorStatus7:
		if (getWorld()->chapter == kChapter2 && _index == 10 && _vm->isGameFlagSet(kGameFlag279)) {
			Actor *actor = getScene()->getActor(0);
			actor->getPoint1()->x = _point2.x + _point1.x - actor->getPoint2()->x;
			actor->getPoint1()->y = _point2.y + _point1.y - actor->getPoint2()->y;
			actor->setDirection(kDirectionS);

			getScene()->setPlayerActorIndex(0);

			// Hide this actor and the show the other one
			hide();
			actor->show();

			_vm->clearGameFlag(kGameFlag279);

			getCursor()->show();
		}
		break;

	case kActorStatus8:
	case kActorStatus10:
	case kActorStatus17:
		updateGraphicData(20);
		break;

	case kActorStatus9:
		if (_vm->encounter()->isRunning())
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
		if (getWorld()->chapter == kChapter2) {
			GraphicResource *resource = new GraphicResource(_vm);
			_frameIndex = 0;

			if (_index > 12)
				_resourceId = _graphicResourceIds[_direction + 30];

			if (getScene()->getPlayerIndex() == _index) {
				resource->load(_resourceId);
				_frameIndex = resource->count() - 1;
			}

			if (_index == 11)
				_resourceId = _graphicResourceIds[getSharedData()->globalDirection > 4 ? 8 - getSharedData()->globalDirection : getSharedData()->globalDirection];

			// Reload the graphic resource if the resource ID has changed
			if (resource->getResourceId() != _resourceId)
				resource->load(_resourceId);

			_frameCount = resource->count();

			delete resource;
		}
		break;
	}

	_status = actorStatus;
}

/////////////////////////////////////////////////////////////////////////
// Direction & position
//////////////////////////////////////////////////////////////////////////

void Actor::updateDirection() {
	// Called by Script::hideActor() / showActor() and process_401830()
	if(!_field_970)
		return;

	error("[Actor::updateDirection] Not implemented");
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
	case kActorStatus14:
		_resourceId = _graphicResourceIds[(actorDirection > kDirectionS ? kDirection8 - actorDirection : actorDirection) + 5];
		_frameCount = GraphicResource::getFrameCount(_vm, _resourceId);
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2)
			if (_index == 11) // we are actor 11
				_resourceId = _graphicResourceIds[(actorDirection > kDirectionS) ? kDirection8 - actorDirection : actorDirection];
		break;

	case kActorStatus1:
	case kActorStatus2:
	case kActorStatus12:
		_resourceId = _graphicResourceIds[(actorDirection > kDirectionS ? kDirection8 - actorDirection : actorDirection)];
		break;

	case kActorStatus8:
		_resourceId = _graphicResourceIds[(actorDirection > kDirectionS ? kDirection8 - actorDirection : actorDirection) + 20];
		break;
	}
}

void Actor::faceTarget(uint32 target, DirectionFrom from) {
	debugC(kDebugLevelActor, "[Actor::faceTarget] Facing target %d using direction from %d", target, from);

	Common::Point point;

	switch (from) {
	default:
		error("[Actor::faceTarget] Invalid direction input: %d (should be 0-3)", from);

	case kDirectionFromObject: {
		Object *object = getWorld()->getObjectById((ObjectId)target);
		if (!object) {
			warning("[Actor::faceTarget] No Object found for id %d", target);
			return;
		}

		Common::Rect frameRect = GraphicResource::getFrameRect(_vm, object->getResourceId(), object->getFrameIndex());

		point.x = Common::Rational(frameRect.width(), 2).toInt()  + object->x;
		point.y = Common::Rational(frameRect.height(), 2).toInt() + object->y;
		}
		break;

	case kDirectionFromPolygons: {
		int32 actionIndex = getWorld()->getActionAreaIndexById(target);
		if (actionIndex == -1) {
			warning("[Actor::faceTarget] No ActionArea found for id %d", target);
			return;
		}

		PolyDefinitions *polygon = &getScene()->polygons()->entries[getWorld()->actions[actionIndex]->polygonIndex];

		point.x = polygon->boundingRect.left + (polygon->boundingRect.right  - polygon->boundingRect.left) / 2;
		point.y = polygon->boundingRect.top  + (polygon->boundingRect.bottom - polygon->boundingRect.top)  / 2;
		}
		break;

	case kDirectionFromActor:
		point.x = _point1.x + _point2.x;
		point.y = _point1.y + _point2.y;
		break;

	case kDirectionFromParameters:
		point.x = point.y = target;
		break;
	}

	Common::Point mid(_point1.x + _point2.x, _point1.y + _point2.y);
	updateFromDirection(direction(mid, point));
}

void Actor::setPosition(int32 newX, int32 newY, ActorDirection newDirection, uint32 frame) {
	_point1.x = newX - _point2.x;
	_point1.y = newY - _point2.y;

	if (_direction != kDirection8)
		updateFromDirection(newDirection);

	if (frame > 0)
		_frameIndex = frame;
}

/////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
void Actor::stopSound() {
	if (_soundResourceId && getSound()->isPlaying(_soundResourceId))
		getSound()->stop(_soundResourceId);
}

Common::String Actor::toString(bool shortString) {
	Common::String output;

	output += Common::String::format("Actor %d: %s\n", _index, _name);
	if (!shortString) {
		output += Common::String::format("resourceId:   %d (0x%X): \n", _resourceId, _resourceId);
		output += Common::String::format("objectIndex:  %d: \n", _objectIndex);
		output += Common::String::format("frameIndex:   %d: \n", _frameIndex);
		output += Common::String::format("frameCount:   %d: \n", _frameCount);
		output += Common::String::format("(x, y):       (%d , %d): \n", _point.x, _point.y);
		output += Common::String::format("(x1, y1):     (%d , %d): \n", _point1.x, _point1.y);
		output += Common::String::format("(x2, y2):     (%d , %d): \n", _point2.x, _point2.y);
		output += Common::String::format("flags:        %d: \n", flags);
		output += Common::String::format("actionType:   %d: \n", actionType);
		output += Common::String::format("boundingRect: top[%d] left[%d] right[%d] bottom[%d]: \n", _boundingRect.top, _boundingRect.left, _boundingRect.right, _boundingRect.bottom);
		output += Common::String::format("direction:    %d: \n", _direction);
		output += Common::String::format("field_3C:     %d: \n", _field_3C);
		output += Common::String::format("status:       %d: \n", _status);
		output += Common::String::format("field_44:     %d: \n", _field_44);
		output += Common::String::format("priority:     %d: \n", _priority);
	}

	return output;
}

//////////////////////////////////////////////////////////////////////////
// Unknown methods
//////////////////////////////////////////////////////////////////////////

void Actor::clearFields() {
	_field_970 = 0;
	_field_974 = 0;
	_field_978 = 0;
	_actionIdx1 = 0;
	_field_980 = 0;
	_field_984 = 0;
	_field_988 = 0;
	_field_98C = 0;
	_field_990 = 0;
	_field_994 = 0;
	_field_998 = 0;
	_field_99C = 0;
	_field_9A0 = 0;
}

bool Actor::isResourcePresent() const {
	if (_status != kActorStatus9)
		return false;

	int index;
	for (index = 10; index < 20; index++) {
		if (_graphicResourceIds[index] == _resourceId)
			break;
	}

	return (index >= 15);
}

bool Actor::process(const Common::Point &point) {
	// Compute point and delta
	Common::Point sum(_point1.x + _point2.x, _point1.y + _point2.y);
	Common::Point delta = point - sum;

	// Compute modifiers
	int a1 = 0;
	int a2 = 0;
	int a3 = 0;

	if (delta.x <= 0) {
		if (delta.y >= 0) {
			a1 = -1;
			a2 = 1;
			a3 = 3;
		} else {
			a1 = -1;
			a2 = -1;
			a3 = 0;
		}
	} else {
		if (delta.y >= 0) {
			a1 = 1;
			a2 = 1;
			a3 = 2;
		} else {
			a1 = 1;
			a2 = -1;
			a3 = 1;
		}
	}

	if (point == sum) {
		if (process_408B20(&sum, a3 >= 2 ? kDirectionS : kDirectionN, abs(delta.y), false)) {
			_data.points[0] = point;
			_data.current   = 0;
			_data.count     = 1;

			return true;
		}
	}

	if (point.x == sum.x) {
		ActorDirection actorDir = a3 >= 2 ? kDirectionS : kDirectionN;
		if (process_408B20(&sum, actorDir, abs(delta.y), false)) {
			_data.points[0] = point;
			_data.current   = 0;
			_data.count     = 1;

			updateFromDirection(actorDir);

			return true;
		}

		return false;
	}

	if (point.y == sum.y) {
		ActorDirection actorDir = (a3 != 0 || a3 != 3) ? kDirectionE : kDirectionO;

		if (process_408B20(&sum, actorDir, abs(delta.x), true)) {
			_data.points[0] = point;
			_data.current   = 0;
			_data.count     = 1;

			updateFromDirection(actorDir);

			return true;
		}

		return false;
	}

	if (abs(delta.x) != abs(delta.y)) {
		Common::Array<int> actions;
		Common::Point point1;
		Common::Point point2;
		uint32 count1 = 0;
		uint32 count2 = 0;
		ActorDirection direction1 = kDirectionInvalid;
		ActorDirection direction2 = kDirectionInvalid;

		// Compute coordinates, directions and counts
		if (abs(delta.x) < abs(delta.y)) {
			point1 = Common::Point(sum.x + abs(delta.x) * a1, sum.y + abs(delta.x) * a2);
			point2 = Common::Point(sum.x                    , sum.y + abs(abs(delta.x) - abs(delta.y)) * a2);
			count1 = abs(point1.x - sum.x);
			count2 = abs(point1.y - point.y);

			switch (a3) {
			default:
				error("[Actor::process] Invalid value for a3");
				break;

			case 0:
				direction1 = kDirectionNO;
				direction2 = kDirectionN;
				break;

			case 1:
				direction1 = kDirectionNE;
				direction2 = kDirectionN;
				break;

			case 2:
				direction1 = kDirectionSE;
				direction2 = kDirectionS;
				break;

			case 3:
				direction1 = kDirectionSO;
				direction2 = kDirectionS;
				break;
			}
		} else {
			point1 = Common::Point(sum.x + abs(delta.y) * a1,                      sum.y + abs(delta.y) * a2);
			point2 = Common::Point(sum.x + abs(abs(delta.y) - abs(delta.x)) * a1 , sum.y);
			count1 = abs(abs(delta.y) * a2);
			count2 = abs(point1.y - point.x);

			switch (a3) {
			default:
				error("[Actor::process] Invalid value for a3");
				break;

			case 0:
				direction1 = kDirectionNO;
				direction2 = kDirectionO;
				break;

			case 1:
				direction1 = kDirectionNE;
				direction2 = kDirectionE;
				break;

			case 2:
				direction1 = kDirectionSE;
				direction2 = kDirectionE;
				break;

			case 3:
				direction1 = kDirectionSO;
				direction2 = kDirectionO;
				break;
			}
		}

		// Check scene rects
		if (getWorld()->chapter != kChapter2 || strcmp(_name, "Big Crow")) {
			error("[Actor::process] not implemented (scene rects checks)!");
		}

		if (process_408B20(&sum,    direction1, count1, true)
		 && process_408B20(&point1, direction2, count2, true)) {
			error("[Actor::process] not implemented (process actor data 1)!");
		}

		if (process_408B20(&sum,    direction2, count2, true)
		 && process_408B20(&point1, direction1, count1, true)) {
			error("[Actor::process] not implemented (process actor data 2)!");
		}

		error("[Actor::process] not implemented (compute actions)!");

		//////////////////////////////////////////////////////////////////////////
		// Process actions

		_frameNumber = 0;

		if (abs(sum.x - point.x) > abs(sum.y - point.y)) {
			if (sum.x <= point.x) {
				if (!processAction1(sum, point, &actions))
					return false;
			} else {
				if (!processAction2(sum, point, &actions))
					return false;
			}

			updateFromDirection(_data.directions[0]);

			return true;
		}

		if (sum.y > point.y) {
			if (!processAction3(sum, point, &actions))
				return false;

			updateFromDirection(_data.directions[0]);

			return true;
		}

		// last case: sum.y < point.y
		if (!processAction4(sum, point, &actions))
			return false;

		updateFromDirection(_data.directions[0]);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Last case: abs(delta.x) == abs(delta.y)

	// Compute direction
	ActorDirection actorDir = kDirectionSO;
	switch (a3) {
	default:
		break;

	case 0:
		actorDir = kDirectionNO;
		break;

	case 1:
		actorDir = kDirectionNE;
		break;

	case 2:
		actorDir = kDirectionSE;
		break;
	}

	if (!process_408B20(&sum, actorDir, abs(delta.y), true))
		return false;

	// Update actor data
	_data.points[0] = point;
	_data.current   = 0;
	_data.count     = 1;

	// Update actor from direction
	updateFromDirection(actorDir);

	return true;
}

void Actor::processStatus(int32 actorX, int32 actorY, bool doSpeech) {
	if (process(Common::Point(actorX, actorY))) {
		if (_status <= kActorStatus11)
			updateStatus(kActorStatus2);
		else
			updateStatus(kActorStatus13);
	} else if (doSpeech) {
		getSpeech()->playIndexed(1);
	}
}

void Actor::process_401830(int32 field980, int32 actionAreaId, int32 field978, int field98C, int32 field990, int32 field974, int32 field984, int32 field988) {
	_field_980 = field980;
	_actionIdx1 = (actionAreaId != -1) ? getWorld()->getActionAreaIndexById(actionAreaId) : -1;
	_field_978 = field978;
	_field_98C = field98C;
	_field_990 = field990;
	_field_974 = field974;

	int field_984 = 0;
	int field_988 = 0;
	if (actionAreaId != -1) {
		if (field984) {
			field_984 = field984;
			field_988 = field988;
		} else {
			PolyDefinitions *polygon = &getScene()->polygons()->entries[_actionIdx1];

			field_984 = polygon->points[0].x;
			field_988 = polygon->points[0].y;

			// Iterate through points
			if (polygon->count() > 1) {
				for (uint i = 1; i < polygon->count() - 1; i++) {
					Common::Point point = polygon->points[i];

					switch (field978) {
					default:
						break;

					case 0:
						if (field_988 > point.y)
							field_988 = point.y;
						break;

					case 1:
						if (field_988 > point.y)
							field_988 = point.y;

						if (field_984 > point.x)
							field_984 = point.x;
						break;

					case 2:
						if (field_984 > point.x)
							field_984 = point.x;
						break;

					case 3:
						if (field_988 < point.y)
							field_988 = point.y;

						if (field_984 > point.x)
							field_984 = point.x;
						break;

					case 4:
						if (field_988 < point.y)
							field_988 = point.y;
						break;

					case 5:
						if (field_988 < point.y)
							field_988 = point.y;

						if (field_984 < point.x)
							field_984 = point.x;
						break;

					case 6:
						if (field_984 < point.x)
							field_984 = point.x;
						break;

					case 7:
						if (field_988 > point.y)
							field_988 = point.y;

						if (field_984 < point.x)
							field_984 = point.x;
						break;

					case 8:
						field_984 = 0;
						field_988 = 0;
						break;
					}
				}
			}
		}
	}

	_field_984 = field_984;
	_field_988 = field_988;

	double cosValue = cos(0.523598775) * 1000.0;
	double sinValue = sin(0.523598775) * 1000.0;

	_field_994 = field_984 - cosValue;
	_field_998 = sinValue + field_988;
	_field_99C = field_984 + cosValue;
	_field_9A0 = field_988 - sinValue;
	_field_970 = 1;

	updateDirection();
}

bool Actor::process_4069B0(int32 *x, int32 *y) {
	error("[Actor::process_4069B0] Not implemented!");
}

bool Actor::process_408B20(Common::Point *point, ActorDirection dir, uint32 count, bool hasDelta) {
	if (_field_944 == 1 || _field_944 == 4)
		return true;

	int16 x = (hasDelta ? point->x : point->x + deltaPointsArray[dir].x);
	int16 y = (hasDelta ? point->y : point->y + deltaPointsArray[dir].y);

	// Check scene rect
	if (!_field_944) {
		Common::Rect rct = getWorld()->sceneRects[getWorld()->sceneRectIdx];

		if (x > rct.right)
			return false;

		if (x < rct.left)
			return false;

		if (y < rct.top)
			return false;

		if (y > rct.bottom)
			return false;

		if (!process_4103B0(point, dir))
			return false;
	}

	if (count > 0) {
		uint32 index = 0;

		while (getScene()->findActionArea(kActionAreaType1, Common::Point(x, y)) != -1) {
			x += deltaPointsArray[dir].x;
			y += deltaPointsArray[dir].y;

			++index;

			if (index >= count)
				return true;
		}

		return false;
	}

	return true;
}

void Actor::playSounds(ActorDirection actorDir, uint32 dist) {
	_lastScreenUpdate = _vm->screenUpdateCount;

	Common::Point sum(_point1.x + _point2.x, _point1.x + _point2.x);
	int32 panning = getSound()->calculatePanningAtPoint(sum);

	switch (_status) {
	default:
		break;

	case kActorStatus1:
	case kActorStatus2:
	case kActorStatus12:
	case kActorStatus13:
		updateCoordinatesForDirection(actorDir, dist, &_point1);

		_frameIndex = (_frameIndex + 1) % _frameCount;

		if (_walkingSound1 != kResourceNone) {

			// Compute volume
			int32 vol = sqrt((double)-Config.sfxVolume);
			if (_index != getScene()->getPlayerIndex())
				vol += sqrt((double)abs(getSound()->calculateVolumeAdjustement(sum, 10, 0)));

			int32 volume = (Config.sfxVolume + vol) * (Config.sfxVolume + vol);
			if (volume > 10000)
				volume = 10000;

			if (_field_944 != 1 && _field_944 != 4) {
				// Compute resource Id
				ResourceId resourceId = kResourceNone;
				if (getWorld()->actions[_actionIdx3]->soundResourceIdFrame != kResourceNone && strcmp((char *)&_name, "Crow") && strcmp((char *)&_name, "Big Crow")) {
					if (_frameIndex == _field_64C)
						resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceIdFrame + rnd(1));
					else if (_frameIndex == _field_650)
						resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceId + rnd(1));
				} else {
					if (_frameIndex == _field_64C)
						resourceId = (ResourceId)(_walkingSound1 + rnd(1));
					else if (_frameIndex == _field_650)
						resourceId = (ResourceId)(_walkingSound3 + rnd(1));
				}

				// Play sound
				getSound()->playSound(resourceId, false, -volume, panning);
			}
		}
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2) {
			updateCoordinatesForDirection(actorDir, dist, &_point1);

			if (_walkingSound1 == kResourceNone)
				break;

			// Compute volume
			int32 vol = getWorld()->actions[_actionIdx3]->volume;
			if (_index != getScene()->getPlayerIndex())
				vol += sqrt((double)abs(getSound()->calculateVolumeAdjustement(sum, 10, 0)));

			int32 volume = (Config.sfxVolume + vol) * (Config.sfxVolume + vol);
			if (volume > 10000)
				volume = 10000;

			// Compute resource Id
			ResourceId resourceId = kResourceNone;
			if (getWorld()->actions[_actionIdx3]->soundResourceIdFrame != kResourceNone && strcmp((char *)&_name, "Crow") && strcmp((char *)&_name, "Big Crow")) {
				if (_frameIndex == _field_64C)
					resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceIdFrame + rnd(1));
				else if (_frameIndex == _field_650)
					resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceId + rnd(1));
			} else {
				if (_frameIndex == _field_64C)
					resourceId = (ResourceId)(_walkingSound1 + rnd(1));
				else if (_frameIndex == _field_650)
					resourceId = (ResourceId)(_walkingSound3 + rnd(1));
			}

			// Play sound
			getSound()->playSound(resourceId, false, -volume, panning);
		}
		break;
	}
}

void Actor::process_41BC00(int32 reactionIndex, int32 numberValue01Add) {
	if (reactionIndex > 16)
		return;

	uint32 count = 0;
	for (int i = 0; i < 8; i++)
		if (_reaction[i])
			count++;

	if (count == 8)
		return;

	if (!process_41BDB0(reactionIndex, false))
		_reaction[count] = reactionIndex;

	if (numberValue01Add)
		_numberValue01 += numberValue01Add;

	getSound()->playSound(MAKE_RESOURCE(kResourcePackHive, 0));
}

void Actor::process_41BCC0(int32 reactionIndex, int32 numberValue01Substract) {
	if (reactionIndex > 16)
		return;

	if (numberValue01Substract) {
		_numberValue01 -= numberValue01Substract;
		if (_numberValue01 < 0)
			_numberValue01 = 0;
	}

	if (!numberValue01Substract || !_numberValue01) {

		uint32 count = 0;
		for (int i = 0; i < 8; i++)
			if (_reaction[i])
				count++;

		if (count == 8)
			return;

		if (count == 7) {
			_reaction[7] = 0;
		} else {
			memcpy(&_reaction[count], &_reaction[count + 1], 28 - 4 * count);
			_reaction[7] = 0;
		}
	}
}

bool Actor::process_41BDB0(int32 reactionIndex, int32 testNumberValue01) {
	if (reactionIndex > 16)
		return false;

	uint32 count = 0;
	for (int i = 0; i < 8; i++)
		if (_reaction[i])
			count++;

	if (count == 8)
		return false;

	if (testNumberValue01)
		return _numberValue01 >= testNumberValue01;

	return true;
}

bool Actor::process_4103B0(Common::Point *point, ActorDirection dir) {
	int32 dist = getDistanceForFrame(dir, (_frameIndex >= _frameCount) ? 2 * _frameCount - (_frameIndex + 1) : _frameIndex);

	int32 x = point->x + deltaPointsArray[dir].x * dist - _field_948 - 10;
	int32 y = point->y + deltaPointsArray[dir].y * dist - _field_94C - 10;
	int32 x1 = x + 2 * _field_948 + 20;
	int32 y1 = y + 2 * _field_94C + 20;

	for (int32 i = 0; i < (int32)getWorld()->actors.size(); i++) {
		if (i == _index)
			continue;

		Actor *actor = getScene()->getActor(i);

		if (!actor->isOnScreen())
			continue;

		int32 x2 = actor->getPoint1()->x + actor->getPoint2()->x - actor->getField948() - 15;
		int32 y2 = actor->getPoint1()->y + actor->getPoint2()->y - actor->getField94C() - 10;
		int32 x3 = actor->getPoint1()->x + actor->getPoint2()->x + 2 * actor->getField948() + 15;
		int32 y3 = actor->getPoint1()->y + actor->getPoint2()->y + 2 * actor->getField94C() + 10;

		if (i == getScene()->getPlayerIndex() && getWorld()->chapter == kChapter11) {
			x2 -= 10;
			y2 -= 10;
			x3 += 10;
			y3 += 10;
		}

		if (getScene()->rectIntersect(x, y, x1, y1, x2, y2, x3, y3)) {
			if (i)
				return false;

			int32 x4 = x2 + 10;
			int32 y4 = y2 + 10;
			int32 x5 = x3 - 10;
			int32 y5 = y3 - 10;

			switch (actor->getDirection()) {
			default:
				break;

			case kDirectionNO:
				if (x4 >= x)
					break;
				// Fallback to next case

			case kDirectionN:
				if (x4 >= y)
					break;

				return false;

			case kDirectionO:
				if (x4 < x)
					return false;

				break;

			case kDirectionSO:
				if (x4 < x && y4 > y)
					return false;

				break;

			case kDirectionS:
				if (y5 > y1)
					return false;

				break;

			case kDirectionE:
				if (x5 > x1)
					return false;

				break;

			case kDirectionNE:
				if (x5 > x1 && y4 < y)
					return false;

				break;
			}

			if (getScene()->rectIntersect(x, y, x1, y1, x4, y4, x5, y5))
				return false;
		}
	}

	return true;
}

void Actor::updateAndDraw() {
	Actor *player = getScene()->getActor();
	Common::Point mouse = getCursor()->position();
	bool keepField = false;

	// Get reaction count
	uint32 count = 0;
	for (int i = 0; i < 8; i++)
		if (_reaction[i])
			count++;

	for (uint32 i = 0; i < count; i++) {
		// Compute points
		Common::Point coords;
		adjustCoordinates(&coords);

		Common::Point sinCos = _vm->getSinCosValues(count, i);
		Common::Point point = coords + Common::Point(player->getPoint2()->x + sinCos.x, player->getPoint2()->y / 2 - sinCos.y);

		if (mouse.x < point.x || mouse.x > (point.x + 40) || mouse.y < point.y || mouse.y > (point.y + 40)) {
			getScreen()->addGraphicToQueue(getWorld()->cursorResourcesAlternate[_reaction[i] + 15],
			                               0,
			                               point,
			                               kDrawFlagNone,
			                               0,
			                               1);
		} else {
			if (getWorld()->field_120 != (int32)(i + 1)) {
				getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 3));
				getReaction()->run(_reaction[i] - 1);
			}

			getWorld()->field_120 = i + 1;
			keepField = true;

			getScreen()->addGraphicToQueue(getWorld()->cursorResourcesAlternate[_reaction[i]],
			                               0,
			                               point,
			                               kDrawFlagNone,
			                               0,
			                               1);
		}

		if (getWorld()->chapter == kChapter4)
			updateNumbers(_reaction[i] - 1, point);
	}

	if (!keepField)
		getWorld()->field_120 = 0;
}

void Actor::update_409230() {
	updateStatus(_status <= 11 ? kActorStatusEnabled : kActorStatus14);
	_data.current = 0;
}

//////////////////////////////////////////////////////////////////////////
// Static update methods
//////////////////////////////////////////////////////////////////////////
void Actor::enableActorsChapter2(AsylumEngine *engine) {
	engine->clearGameFlag(kGameFlag438);
	engine->clearGameFlag(kGameFlag439);
	engine->clearGameFlag(kGameFlag440);
	engine->clearGameFlag(kGameFlag441);
	engine->clearGameFlag(kGameFlag442);

	// Reset shared data
	engine->data()->resetActorData();

	engine->scene()->getActor(13)->enable();
	engine->scene()->getActor(13)->processStatus(2300, 71, false);

	engine->scene()->getActor(14)->enable();
	engine->scene()->getActor(14)->processStatus(2600, 1300, false);

	engine->scene()->getActor(15)->enable();
	engine->scene()->getActor(15)->processStatus(2742, 615, false);

	engine->scene()->getActor(16)->enable();
	engine->scene()->getActor(16)->processStatus(2700, 1200, false);

	engine->scene()->getActor(17)->enable();
	engine->scene()->getActor(17)->processStatus(2751, 347, false);

	engine->scene()->getActor(18)->enable();
	engine->scene()->getActor(18)->processStatus(2420, 284, false);

	engine->scene()->getActor(19)->enable();
	engine->scene()->getActor(19)->processStatus(2800, 370, false);

	engine->scene()->getActor(20)->enable();
	engine->scene()->getActor(20)->processStatus(1973, 1, false);

	engine->scene()->getActor(21)->enable();
	engine->scene()->getActor(21)->processStatus(2541, 40, false);

	for (uint32 i = 0; i < 9; i++) {
		engine->data()->setData(i, 160);
		engine->data()->setData(i + 18, 0);
		engine->data()->setData(i + 44, 0);
	}
}

void Actor::updatePlayerChapter9(AsylumEngine *engine, int nextPlayer) {
	WorldStats *world = engine->scene()->worldstats();
	if (world->chapter != kChapter9)
		return;

	Actor *player = engine->scene()->getActor();
	world->nextPlayer = nextPlayer;

	switch (engine->scene()->getPlayerIndex()) {
	default:
		break;

	case 1:
		if (nextPlayer == 2) {
			player->setResourceId(world->graphicResourceIds[7]);

			uint32 frameCount = GraphicResource::getFrameCount(engine, player->getResourceId());
			player->setFrameCount(frameCount);
			player->setFrameIndex(frameCount - 1);

		} else if (nextPlayer == 3) {
			player->setResourceId(world->graphicResourceIds[8]);

			uint32 frameCount = GraphicResource::getFrameCount(engine, player->getResourceId());
			player->setFrameCount(frameCount);
			player->setFrameIndex(frameCount - 1);
		}
		break;

	case 2:
		if (nextPlayer == 1) {
			player->setResourceId(world->graphicResourceIds[4]);

			uint32 frameCount = GraphicResource::getFrameCount(engine, player->getResourceId());
			player->setFrameCount(frameCount);
			player->setFrameIndex(0);

		} else if (nextPlayer == 3) {
			player->setResourceId(world->graphicResourceIds[3]);

			uint32 frameCount = GraphicResource::getFrameCount(engine, player->getResourceId());
			player->setFrameCount(frameCount);
			player->setFrameIndex(0);
		}
		break;

	case 3:
		if (nextPlayer == 1) {
			player->setResourceId(world->graphicResourceIds[5]);

			uint32 frameCount = GraphicResource::getFrameCount(engine, player->getResourceId());
			player->setFrameCount(frameCount);
			player->setFrameIndex(0);

		} else if (nextPlayer == 2) {
			player->setResourceId(world->graphicResourceIds[6]);

			uint32 frameCount = GraphicResource::getFrameCount(engine, player->getResourceId());
			player->setFrameCount(frameCount);
			player->setFrameIndex(frameCount - 1);
		}
		break;
	}

	player->updateStatus(kActorStatus21);
}

//////////////////////////////////////////////////////////////////////////
// Update methods
//////////////////////////////////////////////////////////////////////////

void Actor::updateStatus3_19() {
	if (getWorld()->chapter != kChapter2 || _frameIndex != 6 || _status == kActorStatus3) { /* Original check: _status  <= kActorStatus11 */
		if (_frameIndex < _frameCount - 1) {
			++_frameIndex;
		} else {
			if (_status == kActorStatus3)
				updateStatus(kActorStatus7);
			else
				updateStatus(kActorStatus20);
		}
	} else {
		if (_index == getScene()->getPlayerIndex())
			updateStatus19_Player();

		++_frameIndex;
	}
}

void Actor::updateStatus19_Player() {
	updatePumpkin(kGameFlag263, kGameFlag270, kObjectPumpkin2Dies, kObjectPumpkin2Loop);
	updatePumpkin(kGameFlag264, kGameFlag271, kObjectPumpkin3Dies, kObjectPumpkin3Loop);
	updatePumpkin(kGameFlag265, kGameFlag272, kObjectPumpkin4Dies, kObjectPumpkin4Loop);
	updatePumpkin(kGameFlag266, kGameFlag273, kObjectPumpkin5Dies, kObjectPumpkin5Loop);
	updatePumpkin(kGameFlag267, kGameFlag274, kObjectPumpkin6Dies, kObjectPumpkin6Loop);
	updatePumpkin(kGameFlag268, kGameFlag275, kObjectPumpkin7Dies, kObjectPumpkin7Loop);
	updatePumpkin(kGameFlag269, kGameFlag276, kObjectPumpkin1Dies, kObjectPumpkin1Loop);
}

void Actor::updatePumpkin(GameFlag flagToCheck, GameFlag flagToSet, ObjectId objectToUpdate, ObjectId objectToDisable) {
	if (_vm->isGameFlagSet(flagToCheck)) {
		_vm->setGameFlag(flagToSet);
		_vm->clearGameFlag(flagToCheck);

		getSharedData()->setActorUpdateFlag2(getSharedData()->getActorUpdateFlag2() + 1);

		getWorld()->getObjectById(objectToUpdate)->setNextFrame(8);
		getSound()->playSound(getWorld()->soundResourceIds[17], false, Config.sfxVolume - 10);
		getWorld()->getObjectById(objectToDisable)->disable();
	}
}

void Actor::updateStatusEnabled() {
	if (_frameCount == 0)
		error("[Actor::updateStatusEnabled] Actor has no frame!");

	_frameIndex = (_frameIndex + 1) % _frameCount;

	if (_vm->screenUpdateCount - _lastScreenUpdate > 300) {
		// All actors except Crow
		if (strcmp((char *)&_name, "Crow")) {
			if (_vm->getRandom(100) < 50
			 && (!getSpeech()->getSoundResourceId() || !getSound()->isPlaying(getSpeech()->getSoundResourceId()))
			 && isDefaultDirection(10))
				updateStatus(kActorStatus9);

			_lastScreenUpdate = _vm->screenUpdateCount;
		}
	}

	// Actor: Player
	if (_index == getScene()->getPlayerIndex()) {
		if (_vm->lastScreenUpdate && (_vm->screenUpdateCount - _vm->lastScreenUpdate) > 500) {

			if (_vm->isGameFlagNotSet(kGameFlagScriptProcessing)
			 && isVisible()
			 && !_vm->encounter()->isRunning()
			 && !getSpeech()->getSoundResourceId()) {
				if (_vm->getRandom(100) < 50) {
					if (getWorld()->chapter == kChapter13)
						getSpeech()->playPlayer(507);
					else
						getSpeech()->playIndexed(4);
				}
			}
			_lastScreenUpdate = _vm->screenUpdateCount;
			_vm->lastScreenUpdate = _vm->screenUpdateCount;
		}

		return;
	}

	// Actor:: BigCrow
	if (!strcmp(_name, "Big Crow")) {
		if (_vm->getRandom(10) < 5) {
			switch (_vm->getRandom(4)) {
			default:
				break;

			case 0:
				setPosition(10, 1350, kDirectionN, 0);
				processStatus(1460, -100, false);
				break;

			case 1:
				setPosition(300, 0, kDirectionN, 0);
				processStatus(1700, 1400, false);
				break;

			case 2:
				setPosition(1560, -100, kDirectionN, 0);
				processStatus(-300, 1470, false);
				break;

			case 3:
				setPosition(1150, 1400, kDirectionN, 0);
				processStatus(-250, 0, false);
				break;
			}
		}

		return;
	}

	// All other actors
	if (getWorld()->chapter != kChapter2 || _index != 8) {
		if (_field_944 == 4) {
			Common::Rect frameRect = GraphicResource::getFrameRect(_vm, getWorld()->backgroundImage, 0);
			processStatus(rnd(frameRect.width() + 200) - 100, rnd(frameRect.height() + 200) - 100, false);
		} else {
			// Actor: Crow
			if (rnd(1000) < 5 || !strcmp(_name, "Crow")) {
				if (_actionIdx2 != -1) {

					// Process action area
					int32 areaIndex = getWorld()->getRandomActionAreaIndexById(_actionIdx2);
					if (areaIndex != -1) {

						ActionArea *area = getWorld()->actions[areaIndex];
						PolyDefinitions *poly = &getScene()->polygons()->entries[area->polygonIndex];

						Common::Point pt(poly->boundingRect.left + rnd(poly->boundingRect.width()),
						                 poly->boundingRect.top + rnd(poly->boundingRect.height()));

						if (!getSharedData()->actorUpdateEnabledCheck) {
							if (!isInActionArea(pt, area)) {
								Common::Point *polyPoint = &poly->points[rnd(poly->count())];
								processStatus(polyPoint->x, polyPoint->y, false);
							} else {
								processStatus(pt.x, pt.y, false);
							}
						}
					}
				}
			}
		}
	} else {
		switch (getSharedData()->actorUpdateStatusEnabledCounter) {
		default:
			break;

		case 0:
			updateStatusEnabledProcessStatus(1055, 989, 1, 1088, 956);
			break;

		case 1:
			updateStatusEnabledProcessStatus(1088, 956, 2, _point1.x + _point2.x, 900);
			break;

		case 2:
			updateStatusEnabledProcessStatus(1088, 900, 3, 1018, 830);
			break;

		case 3:
			updateStatusEnabledProcessStatus(1018, 830, 4, 970, 830);
			break;

		case 4:
			updateStatusEnabledProcessStatus(970, 830, 5, 912, 936);
			break;

		case 5:
			updateStatusEnabledProcessStatus(912, 936, 0, 1055, 989);
			break;
		}
	}
}

void Actor::updateStatusEnabledProcessStatus(int32 testX, int32 testY, uint32 counter, int32 setX, int32 setY) {
	int32 xsum = _point1.x + _point2.x;
	int32 ysum = _point1.y + _point2.y;

	if (xsum != testX || ysum != testY) {
		if (rnd(1000) < 5)
			processStatus(testX, testY, false);
	} else {
		getSharedData()->actorUpdateStatusEnabledCounter = counter;

		if (rnd(1000) < 5)
			processStatus(setX, setY, false);
	}
}

void Actor::updateStatus9() {
	if (_index == getScene()->getPlayerIndex()
	 && getWorld()->chapter != kChapter9
	 && getWorld()->actorType == 0
	 && _frameIndex == 0
	 && isResourcePresent()) {
		if (!getSpeech()->getSoundResourceId() || !getSound()->isPlaying(getSpeech()->getSoundResourceId()))
			getSpeech()->playPlayer(13);
	}

	++_frameIndex;
	if (_frameIndex == _frameCount) {
		enable();
		_lastScreenUpdate = _vm->screenUpdateCount;
	}
}

void Actor::updateStatus12_Chapter2() {
	// Compute distance
	uint32 frameIndex = _frameIndex;
	if (_frameIndex >= _frameCount)
		frameIndex = 2 * _frameCount - _frameIndex - 1;

	int32 distance = getDistanceForFrame(_direction, frameIndex);

	// Face actor
	faceTarget(getScene()->getPlayerIndex(), kDirectionFromActor);

	// FIXME: another field is used (not _data!)
	//if (_data.field_10[_index + 10] > 0) {
	//	_direction = (ActorDirection)(_direction + 4);
	//	_data.field_10[_index + 10] -= 1;
	//}

	// Compute coordinates and distance
	Actor *player = getScene()->getActor();
	Common::Point sumPlayer = *player->getPoint1() + *player->getPoint2();
	Common::Point sum = _point1 + _point2;

	uint32 absX = abs(sum.x - sumPlayer.x);
	uint32 absY = abs(sum.y - sumPlayer.y);

	// Adjust distance
	if (absX <= absY)
		absX = absY;

	if (!distance)
		distance = 0; // FIXME: get proper distance value

	if (absX >= 50) {
		playSounds(_direction, abs(distance));
	} else {
		_frameIndex = 0;
		// FIXME: another field is used (not _data!)
		//_data.field_10[2 * _index + 15] = player->getPoint1()->x - _point1.x;
		//_data.field_10[2 * _index + 16] = player->getPoint1()->y - _point1.y;

		updateStatus(kActorStatus18);
	}
}

void Actor::updateStatus12_Chapter2_Actor11() {
	error("[Actor::updateStatus12_Chapter2_Actor11] not implemented!");
}

void Actor::updateStatus12_Chapter11_Actor1() {
	// Original seems to have lots of dead code here
	Actor *actor0 = getScene()->getActor(0);
	if (actor0->isVisible())
		return;

	if (_vm->isGameFlagNotSet(kGameFlag560))
		_frameIndex = (_frameIndex + 1) & _frameCount;

	if (getWorld()->tickCount1 < (int32)_vm->getTick()
	 && !_frameIndex
	 && _vm->isGameFlagNotSet(kGameFlag560)) {
		_vm->setGameFlag(kGameFlag560);
		hide();
		updateStatus(kActorStatusEnabled);
		actor0->updateStatus(kActorStatusEnabled);

		getWorld()->field_E848C = 0;
		getScript()->queueScript(getWorld()->getActionAreaById(1574)->scriptIndex, 1);
	}
}

void Actor::updateStatus12_Chapter11() {
	if (!_frameIndex)
		getSound()->playSound(getWorld()->soundResourceIds[6]);

	++_frameIndex;

	if (_frameIndex >= _frameCount) {
		_frameIndex = 0;
		updateStatus(kActorStatus14);
		getWorld()->tickValueArray[_index] = rnd(4000) + _vm->getTick();
	}

	Actor *actor0 = getScene()->getActor(0);

	getSharedData()->vector1.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector1.y = actor0->getPoint1()->y + actor0->getPoint2()->y - 5;

	getSharedData()->vector2.x = _point1.x + _point2.x;
	getSharedData()->vector2.y = _point1.y + _point2.y;

	updateCoordinates(getSharedData()->vector1, getSharedData()->vector2);
}

void Actor::updateStatus14() {
	_frameIndex = (_frameIndex + 1) % _frameCount;
	_lastScreenUpdate = _vm->screenUpdateCount;

	switch (getWorld()->chapter) {
	default:
		break;

	case kChapter2:
		if (_index == 11)
			updateStatus(kActorStatus12);
		else if (_index > 12)
			updateStatus14_Chapter2();
		break;

	case kChapter11:
		if (_index >= 10 && _index < 16)
			updateStatus14_Chapter11();
		break;
	}
}

void Actor::updateStatus14_Chapter2() {
	if (!getSharedData()->getData2(_index)) {
		updateStatus(kActorStatus12);
		return;
	}


	error("[Actor::updateStatus14_Chapter2] not implemented!");
}

void Actor::updateStatus14_Chapter11() {
	Actor *actor0 = getScene()->getActor(0);

	getSharedData()->vector1.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector1.y = actor0->getPoint1()->y + actor0->getPoint2()->y - 5;

	getSharedData()->vector2.x = _point1.x + _point2.x;
	getSharedData()->vector2.y = _point1.y + _point2.y;

	if (getWorld()->tickValueArray[_index] == -666)
		getWorld()->tickValueArray[_index] = rnd(4000) + _vm->getTick();

	faceTarget(kActorMax, kDirectionFromActor);
	updateCoordinates(getSharedData()->vector1, getSharedData()->vector2);

	if (getWorld()->tickValueArray[_index] < (int)_vm->getTick()) {
		if (distance(getSharedData()->vector1, getSharedData()->vector2) >= 75) {
			getWorld()->tickValueArray[_index] = rnd(1000) + 2000 + _vm->getTick();
		} else {
			if (actor0->getStatus() == kActorStatus12 || actor0->getStatus() == kActorStatus14 || actor0->getStatus() == kActorStatus15)
				updateStatus(kActorStatus15);

			getWorld()->tickValueArray[_index] = -666;
		}
	}
}

void Actor::updateStatus15_Chapter2() {
	error("[Actor::updateStatus15_Chapter2] not implemented!");
}

void Actor::updateStatus15_Chapter2_Helper() {
	Actor *actor39 = getScene()->getActor(39);

	actor39->getPoint1()->x = _point1.x;
	actor39->getPoint1()->y = _point1.y;

	if (_vm->isGameFlagSet(kGameFlag169))
		actor39->getPoint1()->y += 80;

	switch (getSharedData()->getData(40)) {
	default:
		break;

	case 0:
		_vm->setGameFlag(kGameFlag369);

		if (getSound()->isPlaying(getWorld()->soundResourceIds[5]))
			getSound()->stop(getWorld()->soundResourceIds[5]);

		if (!getSound()->isPlaying(getWorld()->soundResourceIds[6]))
			getSound()->playSound(getWorld()->soundResourceIds[6], true, Config.sfxVolume - 10);
		break;

	case 1:
		_vm->setGameFlag(kGameFlag370);

		if (getSound()->isPlaying(getWorld()->soundResourceIds[6]))
			getSound()->stop(getWorld()->soundResourceIds[6]);

		if (!getSound()->isPlaying(getWorld()->soundResourceIds[7]))
			getSound()->playSound(getWorld()->soundResourceIds[7], true, Config.sfxVolume - 10);
		break;

	case 2:
		if (getSound()->isPlaying(getWorld()->soundResourceIds[7]))
			getSound()->stop(getWorld()->soundResourceIds[7]);
		break;
	}

	getSharedData()->setData(40, getSharedData()->getData(40) + 1);

	switch (getSharedData()->getData(40)) {
	default:
		break;

	case 0:
		enableActorsChapter2(_vm);
		getCursor()->hide();
		break;

	case 1:
		_vm->setGameFlag(kGameFlag369);
		break;

	case 2:
		_vm->setGameFlag(kGameFlag370);
		break;
	}
}

void Actor::updateStatus15_Chapter2_Player() {
	error("[Actor::updateStatus15_Chapter2_Player] not implemented!");
}

void Actor::updateStatus15_Chapter2_Player_Helper() {
	// we are the current player
	Actor *actor11 = getScene()->getActor(11);
	Actor *actor40 = getScene()->getActor(40);

	Common::Point point(_point1.x + _point2.x, _point1.y + _point2.y);
	Common::Point point11(actor11->getPoint1()->x + actor11->getPoint2()->x, actor11->getPoint1()->y + actor11->getPoint2()->y);

	if (actor11->getStatus() == kActorStatus15 && distance(point, point11) < 100) {
		Actor *actor = getScene()->getActor(getSharedData()->getData(38));

		actor40->show();
		actor40->setFrameIndex(0);
		actor40->getPoint1()->x = actor->getPoint1()->x;
		actor40->getPoint1()->y = actor->getPoint1()->y;

		if (actor11->getFrameIndex() <= 7) {
			getSound()->playSound(getWorld()->soundResourceIds[9], false, Config.sfxVolume - 10);
		} else if (getSharedData()->getData(36) <= 6) {
			getSound()->playSound(getWorld()->soundResourceIds[9], false, Config.sfxVolume - 10);
		} else {
			getScene()->getActor(10)->updateStatus(kActorStatus17);
			getSound()->playSound(getWorld()->soundResourceIds[10], false, Config.sfxVolume - 10);
		}
	}
}

bool Actor::updateStatus15_isNoVisibleOrStatus17() {
	return (!isVisible() || _status == kActorStatus17);
}

void Actor::updateStatus15_Chapter2_Actor11() {
	error("[Actor::updateStatus15_Chapter2_Actor11] not implemented!");
}

bool Actor::updateStatus15_Chapter2_Actor11_Helper(ActorIndex actorIndex1, ActorIndex actorIndex2) {
	Actor *actor1 = getScene()->getActor(actorIndex1);
	Actor *actor2 = getScene()->getActor(actorIndex2);

	if (actor1->getField944())
		return false;

	if (actor2->getField944())
		return false;

	int16 actor2_x = actor2->getPoint1()->x + actor2->getPoint2()->x;
	int16 actor2_y = actor2->getPoint1()->y + actor2->getPoint2()->y;

	Common::Point pt1(actor2_x -     actor1->getField948() - 10, actor2_y -     actor1->getField94C() - 10);
	Common::Point pt2(actor2_x + 2 * actor1->getField948() + 10, actor2_y + 2 * actor1->getField94C() + 10);
	Common::Point pt3(actor2_x -     actor2->getField948() - 25, actor2_y -     actor2->getField94C() - 20);
	Common::Point pt4(actor2_x + 2 * actor2->getField948() + 25, actor2_y + 2 * actor2->getField94C() + 20);

	return getScene()->rectIntersect(pt1.x, pt1.y, pt2.x, pt2.y, pt3.x, pt3.y, pt4.x, pt4.y);
}

void Actor::updateStatus15_Chapter11() {
	Actor *actor0 = getScene()->getActor(0);

	// Update vectors
	getSharedData()->vector1.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector1.y = actor0->getPoint1()->y + actor0->getPoint2()->y - 5;

	getSharedData()->vector2.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector2.y = actor0->getPoint1()->y + actor0->getPoint2()->y;

	updateCoordinates(getSharedData()->vector1, getSharedData()->vector2);

	++_frameIndex;
	if (_frameIndex >= _frameCount)
		updateStatus(kActorStatus14);

	if (_frameIndex == 14) {
		if (Actor::distance(getSharedData()->vector1, getSharedData()->vector2) < 75) {

			actor0->updateStatus(kActorStatus16);
			++getWorld()->field_E848C;

			getSound()->stop(getWorld()->soundResourceIds[3]);
			getSound()->stop(getWorld()->soundResourceIds[4]);
			getSound()->stop(getWorld()->soundResourceIds[5]);

			getSpeech()->playPlayer(131);
		}
	}
}

void Actor::updateStatus15_Chapter11_Player() {
	_frameIndex++;

	if (_frameIndex == 17) {
		getSpeech()->playPlayer(130);

		if (getWorld()->field_E849C >= 666) {
			if (_vm->isGameFlagSet(kGameFlag583)) {
				_vm->setGameFlag(kGameFlag582);
				_vm->clearGameFlag(kGameFlag565);
				++getWorld()->field_E8518;
				getSound()->playSound(getWorld()->soundResourceIds[2]);
			}
		} else {
			Actor *actor2 = getScene()->getActor(getWorld()->field_E849C);

			double diffX = (actor2->getPoint1()->x + actor2->getPoint2()->x) - (_point1.x + _point2.x);
			double diffY = (actor2->getPoint1()->y + actor2->getPoint2()->y) - (_point1.y + _point2.y);

			if (sqrt(diffX * diffX + diffY * diffY) < 75.0f
			 && (actor2->getStatus() == kActorStatus14 || actor2->getStatus() == kActorStatus15)) {
				getSound()->playSound(getWorld()->soundResourceIds[2]);

				switch (getWorld()->field_E849C) {
				default:
					break;

				case 10:
					_vm->setGameFlag(kGameFlag563);
					break;

				case 11:
					_vm->setGameFlag(kGameFlag724);
					break;

				case 12:
					_vm->setGameFlag(kGameFlag727);
					break;

				case 13:
					_vm->setGameFlag(kGameFlag730);
					break;
				}

				actor2->updateStatus(kActorStatus17);
			}
		}
	}

	if (_frameIndex >= _frameCount) {
		getCursor()->show();
		getSharedData()->setFlag(kFlag1, false);
		_frameIndex = 0;
		updateStatus(kActorStatus14);
	}
}

void Actor::updateStatus16_Chapter2() {
	// We are sure to be the current player

	++_frameIndex;

	if (_frameIndex > _frameCount - 1) {
		if (getSharedData()->getData(40) <= 2) {
			_frameIndex = 0;
			updateStatus(kActorStatus14);
		} else {
			_vm->clearGameFlag(kGameFlag438);
			_vm->clearGameFlag(kGameFlag439);
			_vm->clearGameFlag(kGameFlag440);
			_vm->clearGameFlag(kGameFlag441);
			_vm->clearGameFlag(kGameFlag442);

			getSpeech()->playPlayer(53);

			_vm->setGameFlag(kGameFlag219);

			_frameIndex = 0;
			updateStatus(kActorStatus17);

			_vm->clearGameFlag(kGameFlag369);
			_vm->clearGameFlag(kGameFlag370);

			if (getSound()->isPlaying(getWorld()->soundResourceIds[5]))
				getSound()->stop(getWorld()->soundResourceIds[5]);

			if (getSound()->isPlaying(getWorld()->soundResourceIds[6]))
				getSound()->stop(getWorld()->soundResourceIds[6]);

			if (getSound()->isPlaying(getWorld()->soundResourceIds[7]))
				getSound()->stop(getWorld()->soundResourceIds[7]);

			if (_vm->isGameFlagSet(kGameFlag235)) {
				Actor::enableActorsChapter2(_vm);
				_vm->clearGameFlag(kGameFlag235);
			}
		}
	}
}

void Actor::updateStatus16_Chapter11() {
	// We are sure to be the current player
	getCursor()->show();
	getSharedData()->setFlag(kFlag1, false);

	if (_frameIndex != 5 || _vm->isGameFlagNotSet(kGameFlag570))
		++_frameIndex;

	if (_frameIndex > _frameCount - 1) {
		if (getWorld()->field_E848C >= 3) {
			_frameIndex = 0;

			getScene()->getActor(0)->updateStatus(kActorStatus17);

			_tickCount = _vm->getTick() + 2000;
		} else {
			getScene()->getActor(0)->updateStatus(kActorStatus14);
		}
	}
}

void Actor::updateStatus17_Chapter2() {
	++_frameIndex;

	if (_frameIndex >= _frameCount) {
		_frameIndex = 0;
		updateStatus(kActorStatus14);
		hide();

		if (_vm->getRandomBit() == 1) {
			_vm->setGameFlag(kGameFlag219);
			getSpeech()->playPlayer(133);
		}
	}
}

void Actor::updateStatus18_Chapter2() {
	Actor *player = getScene()->getActor();

	_point1.x = player->getPoint1()->x - getSharedData()->getData(2 * _index + 19);
	_point1.y = player->getPoint1()->y - getSharedData()->getData(2 * _index + 20);

	_frameIndex++;

	if (_frameIndex > _frameCount - 1) {
		getSharedData()->setData2(_index, true);
		updateStatus(kActorStatus14);

		_point1.y += 54;
		getSound()->playSound(getWorld()->soundResourceIds[1], false, Config.sfxVolume - 10);

		getSharedData()->setData(_index, getSharedData()->getData(_index) - 54);
	}
}

void Actor::updateStatus18_Chapter2_Actor11() {
	error("[Actor::updateStatus18_Chapter2_Actor11] not implemented!");
}

void Actor::updateStatus21() {
	if (_resourceId == getWorld()->graphicResourceIds[3] || _resourceId == getWorld()->graphicResourceIds[4] || _resourceId == getWorld()->graphicResourceIds[5]) {
		if (_frameIndex < _frameCount - 1) {
			++_frameIndex;

			if (_frameIndex == _frameCount / 2) {
				getWorld()->currentPaletteId = getWorld()->graphicResourceIds[getWorld()->nextPlayer - 1];
				getScreen()->setPalette(getWorld()->currentPaletteId);
				getScreen()->setGammaLevel(getWorld()->currentPaletteId, 0);
			}

			return;
		}
	} else {
		if (_frameIndex > 0) {
			--_frameIndex;

			if (_frameIndex == _frameCount / 2)
				getScreen()->setPalette(getWorld()->graphicResourceIds[getWorld()->nextPlayer - 1]);

			getWorld()->currentPaletteId = getWorld()->graphicResourceIds[getWorld()->nextPlayer - 1];
			getScreen()->setGammaLevel(getWorld()->currentPaletteId, 0);
			return;
		}
	}

	getScene()->changePlayer(getWorld()->nextPlayer);
	updateStatus(kActorStatusEnabled);
	getWorld()->nextPlayer = kActorInvalid;
}

void Actor::updateFinish() {
	if (_field_944 == 4 || !isVisible())
		return;

	int32 areaIndex = getScene()->findActionArea(kActionAreaType1, Common::Point((int16)(_point1.x + _point2.x), (int16)(_point1.y + _point2.y)));
	if (areaIndex == _actionIdx3 || areaIndex == -1)
		return;

	ActionArea *area = getWorld()->actions[areaIndex];
	ActionArea *actorArea = getWorld()->actions[_actionIdx3];
	if (!getScript()->isProcessingSkipped()) {
		getScript()->queueScript(actorArea->scriptIndex2, _index);
		getScript()->queueScript(area->scriptIndex, _index);
	}

	if (!area->paletteResourceId || area->paletteResourceId == actorArea->paletteResourceId || _index) {
		if (area->paletteResourceId != actorArea->paletteResourceId && !_index)
			_vm->screen()->startPaletteFade(area->paletteResourceId, 50, 3);

		_actionIdx3 = areaIndex;
	} else {
		_vm->screen()->startPaletteFade(area->paletteResourceId, 50, 3);
		_actionIdx3 = areaIndex;
	}
}

void Actor::updateCoordinates(Common::Point vec1, Common::Point vec2) {
	if (getScene()->getActor(1)->isVisible())
		return;

	uint32 diffY = abs(vec2.y - vec1.y);
	if (diffY > 5)
		diffY = 5;

	if (diffY == 0)
		return;

	ActorDirection dir = (diffY > 0) ? kDirectionS : kDirectionN;

	if (process_408B20(&vec2, dir, diffY + 3, false))
		updateCoordinatesForDirection(dir, diffY - 1, &_point);
}

void Actor::resetActors() {
	getCursor()->hide();
	getScene()->getActor(0)->hide();
	getScene()->getActor(1)->setFrameIndex(0);

	getWorld()->tickCount1 = _vm->getTick() + 3000;
}

void Actor::updateNumbers(int32 reaction, const Common::Point &point) {
	if (reaction != 1)
		return;

	_numberStringX = point.x;
	_numberStringY = point.y + 8;
	_numberStringWidth = 40;
	sprintf(_numberString01, "%d", _numberValue01);

	_numberFlag01 = 1;
}

//////////////////////////////////////////////////////////////////////////
// Path finding functions
//////////////////////////////////////////////////////////////////////////
bool Actor::processAction1(const Common::Point &source, const Common::Point &destination, Common::Array<int> *actions) {
	error("[Actor::processAction1] Not implemented");
}

bool Actor::processAction2(const Common::Point &source, const Common::Point &destination, Common::Array<int> *actions) {
	error("[Actor::processAction2] Not implemented");
}

bool Actor::processAction3(const Common::Point &source, const Common::Point &destination, Common::Array<int> *actions) {
	error("[Actor::processAction3] Not implemented");
}

bool Actor::processAction4(const Common::Point &source, const Common::Point &destination, Common::Array<int> *actions) {
	error("[Actor::processAction4] Not implemented");
}

bool Actor::checkAllActions(const Common::Point &pt, Common::Array<ActionArea *> *actions) {
	if (actions->size() == 0)
		return false;

	for (Common::Array<ActionArea *>::iterator it = actions->begin(); it != actions->end(); it++) {
		if (isInActionArea(pt, *it))
			return true;
	}

	return false;

}

bool Actor::isInActionArea(const Common::Point &pt, ActionArea *area) {
	Common::Rect sceneRect = getWorld()->sceneRects[getWorld()->sceneRectIdx];

	if (!sceneRect.contains(pt))
		return false;

	if (area->flags & 1)
		return false;

	// Check flags
	bool found = false;
	for (uint32 i = 0; i < 10; i++) {
		int32 flag = area->flagNums[i];
		bool state = (flag <= 0) ? _vm->isGameFlagNotSet((GameFlag)-flag) : _vm->isGameFlagSet((GameFlag)flag);

		if (!state) {
			found = true;
			break;
		}
	}

	if (found)
		return false;

	PolyDefinitions poly = getScene()->polygons()->entries[area->polygonIndex];
	if (!poly.contains(pt))
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////

void Actor::setVisible(bool value) {
	if (value)
		flags |= kActorFlagVisible;
	else
		flags &= ~kActorFlagVisible;

	stopSound();
}

bool Actor::isOnScreen() {
	Common::Rect scene(getWorld()->yTop, getWorld()->xLeft, getWorld()->yTop + 480, getWorld()->xLeft + 640);
	Common::Rect actor(_boundingRect);
	actor.moveTo(_point1.x, _point1.y);

	return isVisible() && scene.intersects(actor);
}

void Actor::setVolume() {
	if (!_soundResourceId || !getSound()->isPlaying(_soundResourceId))
		return;

	// Compute volume
	int32 volume = Config.voiceVolume + getSound()->calculateVolumeAdjustement(_point1 + _point2, _field_968, 0);
	if (volume < -10000)
		volume = -10000;

	getSound()->setVolume(_soundResourceId, volume);
}

//////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////

ActorDirection Actor::direction(Common::Point vec1, Common::Point vec2) {
	int32 diffX = (vec2.x - vec1.x) * 2^16;
	int32 diffY = (vec1.y - vec2.y) * 2^16;
	int32 adjust = 0;

	if (diffX < 0) {
		adjust = 2;
		diffX = -diffX;
	}

	if (diffY < 0) {
		adjust |= 1;
		diffY = -diffY;
	}

	int32 dirAngle = -1;

	if (diffX) {
		uint32 index = (diffY * 256) / diffX;

		if (index < 256)
			dirAngle = angleTable01[index];
		else if (index < 4096)
			dirAngle = angleTable02[index / 16];
		else if (index < 65536)
			dirAngle = angleTable03[index / 256];
	} else {
		dirAngle = 90;
	}

	switch (adjust) {
	default:
		break;

	case 1:
		dirAngle = 360 - dirAngle;
		break;

	case 2:
		dirAngle = 180 - dirAngle;
		break;

	case 3:
		dirAngle += 180;
		break;
	}

	if (dirAngle >= 360)
		dirAngle -= 360;

	ActorDirection dir;

	if (dirAngle < 157 || dirAngle >= 202) {
		if (dirAngle < 112 || dirAngle >= 157) {
			if (dirAngle < 67 || dirAngle >= 112) {
				if (dirAngle < 22 || dirAngle >= 67) {
					if ((dirAngle < 0 || dirAngle >= 22) && (dirAngle < 337 || dirAngle > 359)) {
						if (dirAngle < 292 || dirAngle >= 337) {
							if (dirAngle < 247 || dirAngle >= 292) {
								if (dirAngle < 202 || dirAngle >= 247) {
									error("[Actor::direction] got a bad direction angle: %d!", dirAngle);
								} else {
									dir = kDirectionSO;
								}
							} else {
								dir = kDirectionS;
							}
						} else {
							dir = kDirectionSE;
						}
					} else {
						dir = kDirectionE;
					}
				} else {
					dir = kDirectionNE;
				}
			} else {
				dir = kDirectionN;
			}
		} else {
			dir = kDirectionNO;
		}
	} else {
		dir = kDirectionO;
	}

	return dir;
}

void Actor::updateGraphicData(uint32 offset) {
	int32 index = ((_direction > kDirectionS) ? kDirection8 - _direction : _direction) + (int32)offset;
	_resourceId = _graphicResourceIds[index];
	_frameCount = GraphicResource::getFrameCount(_vm, _resourceId);
	_frameIndex = 0;
}

bool Actor::isDefaultDirection(int index) const {
	return _graphicResourceIds[index] != _graphicResourceIds[5];
}

void Actor::adjustCoordinates(Common::Point *point) {
	if (!point)
		error("[Actor::adjustCoordinates] Invalid point parameter!");

	point->x = _point1.x - getWorld()->xLeft;
	point->y = _point1.y - getWorld()->yTop;
}

DrawFlags Actor::getGraphicsFlags() {
	if (getWorld()->chapter == kChapter11) {
		int res = strcmp((char *)&_name, "Dead Sarah");

		if (res == 0)
			return kDrawFlagNone;
	}

	if (_direction < kDirectionSE)
		return kDrawFlagNone;

	return kDrawFlagMirrorLeftRight;
}

int32 Actor::getDistance() const {
	int32 index = (_frameIndex >= _frameCount) ? (2 * _frameCount) - (_frameIndex + 1) : _frameIndex;

	if (index >= 20)
		error("[Actor::getDistance] Invalid index calculation (was: %d, max: 20)", index);

	switch (_direction) {
	default:
	case kDirectionN:
	case kDirectionS:
		return 0;

	case kDirectionNO:
	case kDirectionSO:
		return -_field_8D0[index];

	case kDirectionO:
		return -_field_830[index];

	case kDirectionSE:
	case kDirectionNE:
		return _field_8D0[index];

	case kDirectionE:
		return _field_830[index];
	}
}

int32 Actor::getDistanceForFrame(ActorDirection dir, uint32 frameIndex) {
	switch (dir) {
	default:
	case kDirectionN:
	case kDirectionS:
		return _field_880[frameIndex];

	case kDirectionNO:
	case kDirectionSO:
	case kDirectionSE:
	case kDirectionNE:
		return _field_8D0[frameIndex];

	case kDirectionO:
	case kDirectionE:
		return _field_830[frameIndex];
	}
}

void Actor::updateCoordinatesForDirection(ActorDirection direction, int32 delta, Common::Point *point) {
	if (!point)
		error("[Actor::updateCoordinatesForDirection] Invalid point (NULL)!");

	switch (direction) {
	default:
		break;

	case kDirectionN:
		point->y -= delta;
		break;

	case kDirectionNO:
		point->x -= delta;
		point->y -= delta;
		break;

	case kDirectionO:
		point->x -= delta;
		break;

	case kDirectionSO:
		point->x -= delta;
		point->y += delta;
		break;

	case kDirectionS:
		point->y += delta;
		break;

	case kDirectionSE:
		point->x += delta;
		point->y += delta;
		break;

	case kDirectionE:
		point->x += delta;
		break;

	case kDirectionNE:
		point->y += delta;
		point->y -= delta;
		break;
	}
}

uint32 Actor::distance(Common::Point vec1, Common::Point vec2) {
	return sqrt(pow((double)(vec2.y - vec1.y), 2) + pow((double)(vec2.x - vec1.x), 2));
}

uint32 Actor::angle(Common::Point vec1, Common::Point vec2) {
	int32 result = ((long)(180 - acos((double)(vec2.y - vec1.y) / distance(vec1, vec2)) * 180 / M_PI)) % 360;

	if (vec1.x < vec2.x)
		return 360 - result;

	return result;
}

void Actor::rect(Common::Rect *rect, ActorDirection direction, Common::Point point) {
	if (!rect)
		error("[Actor::rect] Invalid rect (NULL)!");

	switch (direction) {
	default:
		rect->top = 0;
		rect->left = 0;
		rect->bottom = 0;
		rect->right = 0;
		return;

	case kDirectionN:
		rect->top = point.x - 9;
		rect->left = point.y - 84;
		break;

	case kDirectionNO:
		rect->top = point.x - 55;
		rect->left = point.y - 84;
		break;

	case kDirectionO:
		rect->top = point.x - 34;
		rect->left = point.y - 93;
		break;

	case kDirectionSO:
		rect->top = point.x + 27;
		rect->left = point.y - 94;
		break;

	case kDirectionS:
		rect->top = point.x + 41;
		rect->left = point.y - 9;
		break;

	case kDirectionSE:
		rect->top = point.x + 27;
		rect->left = point.y + 54;
		break;

	case kDirectionE:
		rect->top = point.x - 34;
		rect->left = point.y + 53;
		break;

	case kDirectionNE:
		rect->top = point.x - 55;
		rect->left = point.y + 44;
		break;
	}

	rect->setWidth(40);
	rect->setHeight(40);
}

bool Actor::compareAngles(Common::Point vec1, Common::Point vec2) {
	Common::Point vec3(2289, 171);

	int32 diff = angle(vec1, vec3) - angle(vec1, vec2);

	if (diff < 0)
		diff += 359;

	return (diff != 180);
}

bool Actor::compare(Common::Point vec1, Common::Point vec2, Common::Point vec) {
	if (vec.y >= vec1.y && vec.y <= vec2.y && vec.x >= vec1.x && vec.x <= vec2.x)
		return true;

	return false;
}

int32 Actor::compareX(Common::Point vec1, Common::Point vec2, Common::Point vec) {
	if (vec.y > vec2.y)
		return 3;

	if (vec.y < vec1.y)
		return 2;

	return 0;
}

int32 Actor::compareY(Common::Point vec1, Common::Point vec2, Common::Point vec) {
	if (vec.y > vec2.y)
		return 3;

	if (vec.y < vec1.y)
		return 2;

	return 0;
}


} // end of namespace Asylum
