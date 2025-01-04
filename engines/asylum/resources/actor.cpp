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

#include "asylum/resources/actor.h"

#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/polygons.h"
#include "asylum/resources/special.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"

#include "asylum/asylum.h"
#include "asylum/staticres.h"


namespace Asylum {

#define DIR(val) (ActorDirection)((val) & 7)

Actor::Actor(AsylumEngine *engine, ActorIndex index) : _vm(engine), _index(index), inventory(engine, _numberValue01) {
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
 	_walkingSound1 = 0;
 	_walkingSound2 = 0;
 	_walkingSound3 = 0;
 	_walkingSound4 = 0;
 	_field_64C = 0;
 	_field_650 = 0;
 	memset(_graphicResourceIds, 0 , sizeof(_graphicResourceIds));
 	memset(&_name, 0, sizeof(_name));
 	memset(&_distancesEO, 0, sizeof(_distancesEO));
 	memset(&_distancesNS, 0, sizeof(_distancesNS));
 	memset(&_distancesNSEO, 0, sizeof(_distancesNSEO));
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
 	memset(&_numberString01, 0, 8);
 	_field_968 = 0;
 	_transparency = 0;
 	_processNewDirection = false;
 	_invertPriority = false;
 	_nextDirection = kDirectionN;
 	_nextActionIndex = 0;
 	_nextActorIndex = kActorMax;
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

/////////////////////////////////////////////////////////////////////////
// Loading
//////////////////////////////////////////////////////////////////////////
void Actor::load(Common::SeekableReadStream *stream) {
	if (!stream)
		error("[Actor::load] invalid stream");

	_point.x              = (int16)stream->readSint32LE();
	_point.y              = (int16)stream->readSint32LE();
	_resourceId           = (ResourceId)stream->readSint32LE();
	_objectIndex          = stream->readSint32LE();
	_frameIndex           = stream->readUint32LE();
	_frameCount           = stream->readUint32LE();
	_point1.x             = (int16)stream->readSint32LE();
	_point1.y             = (int16)stream->readSint32LE();
	_point2.x             = (int16)stream->readSint32LE();
	_point2.y             = (int16)stream->readSint32LE();

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

	inventory.load(stream);
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
		_distancesEO[i] = stream->readSint32LE();

	for (int32 i = 0; i < 20; i++)
		_distancesNS[i] = stream->readSint32LE();

	for (int32 i = 0; i < 20; i++)
		_distancesNSEO[i] = stream->readSint32LE();

	if (_vm->checkGameVersion("Demo"))
		return;

	_actionIdx2           = stream->readSint32LE();
	_field_924            = stream->readSint32LE();
	_lastScreenUpdate     = stream->readUint32LE();
	_scriptIndex          = stream->readSint32LE();
	actionType            = stream->readSint32LE();
	_field_934            = stream->readSint32LE();
	_field_938            = stream->readSint32LE();
	_soundResourceId      = (ResourceId)stream->readSint32LE();
	_numberValue01        = stream->readSint32LE();
	_field_944            = stream->readSint32LE();
	_field_948            = stream->readSint32LE();
	_field_94C            = stream->readSint32LE();
	_numberFlag01         = stream->readSint32LE();
	_numberStringWidth    = (int16)stream->readSint32LE();
	_numberPoint.x        = (int16)stream->readSint32LE();
	_numberPoint.y        = (int16)stream->readSint32LE();
	stream->read(_numberString01, sizeof(_numberString01));
	_field_968            = stream->readSint32LE();
	_transparency         = stream->readSint32LE();
	_processNewDirection  = (bool)stream->readSint32LE();
	_invertPriority       = (bool)stream->readSint32LE();
	_nextDirection        = (ActorDirection)stream->readSint32LE();
	_nextActionIndex      = stream->readSint32LE();
	_nextActorIndex       = stream->readSint32LE();
	_nextPositionOffset.x = (int16)stream->readSint32LE();
	_nextPositionOffset.y = (int16)stream->readSint32LE();
	_nextPosition.x       = (int16)stream->readSint32LE();
	_nextPosition.y       = (int16)stream->readSint32LE();
	_field_994            = stream->readSint32LE();
	_field_998            = stream->readSint32LE();
	_field_99C            = stream->readSint32LE();
	_field_9A0            = stream->readSint32LE();
}

void Actor::saveLoadWithSerializer(Common::Serializer &s) {
	s.syncAsSint32LE(_point.x);
	s.syncAsSint32LE(_point.y);
	s.syncAsSint32LE(_resourceId);
	s.syncAsSint32LE(_objectIndex);
	s.syncAsUint32LE(_frameIndex);
	s.syncAsUint32LE(_frameCount);
	s.syncAsSint32LE(_point1.x);
	s.syncAsSint32LE(_point1.y);
	s.syncAsSint32LE(_point2.x);
	s.syncAsSint32LE(_point2.y);

	s.syncAsSint32LE(_boundingRect.left);
	s.syncAsSint32LE(_boundingRect.top);
	s.syncAsSint32LE(_boundingRect.right);
	s.syncAsSint32LE(_boundingRect.bottom);

	s.syncAsSint32LE(_direction);
	s.syncAsSint32LE(_field_3C);
	s.syncAsSint32LE(_status);
	s.syncAsSint32LE(_field_44);
	s.syncAsSint32LE(_priority);
	s.syncAsSint32LE(flags);
	s.syncAsSint32LE(_field_50);
	s.syncAsSint32LE(_field_54);
	s.syncAsSint32LE(_field_58);
	s.syncAsSint32LE(_field_5C);
	s.syncAsSint32LE(_field_60);
	s.syncAsSint32LE(_actionIdx3);

	// TODO skip field_68 till field_617
	s.skip(0x5B0);

	inventory.saveLoadWithSerializer(s);

	s.syncAsSint32LE(_walkingSound1);
	s.syncAsSint32LE(_walkingSound2);
	s.syncAsSint32LE(_walkingSound3);
	s.syncAsSint32LE(_walkingSound4);
	s.syncAsUint32LE(_field_64C);
	s.syncAsUint32LE(_field_650);

	for (int32 i = 0; i < ARRAYSIZE(_graphicResourceIds); i++)
		s.syncAsSint32LE(_graphicResourceIds[i]);

	s.syncBytes((byte *)&_name, sizeof(_name));

	for (int32 i = 0; i < 20; i++)
		s.syncAsSint32LE(_distancesEO[i]);

	for (int32 i = 0; i < 20; i++)
		s.syncAsSint32LE(_distancesNS[i]);

	for (int32 i = 0; i < 20; i++)
		s.syncAsSint32LE(_distancesNSEO[i]);

	s.syncAsSint32LE(_actionIdx2);
	s.syncAsSint32LE(_field_924);
	s.syncAsSint32LE(_lastScreenUpdate);
	s.syncAsSint32LE(_scriptIndex);
	s.syncAsSint32LE(actionType);
	s.syncAsSint32LE(_field_934);
	s.syncAsSint32LE(_field_938);
	s.syncAsSint32LE(_soundResourceId);
	s.syncAsSint32LE(_numberValue01);
	s.syncAsSint32LE(_field_944);
	s.syncAsSint32LE(_field_948);
	s.syncAsSint32LE(_field_94C);
	s.syncAsSint32LE(_numberFlag01);
	s.syncAsSint32LE(_numberStringWidth);
	s.syncAsSint32LE(_numberPoint.x);
	s.syncAsSint32LE(_numberPoint.y);
	s.syncBytes((byte *)&_numberString01, sizeof(_numberString01));
	s.syncAsSint32LE(_field_968);
	s.syncAsSint32LE(_transparency);
	s.syncAsSint32LE(_processNewDirection);
	s.syncAsSint32LE(_invertPriority);
	s.syncAsSint32LE(_nextDirection);
	s.syncAsSint32LE(_nextActionIndex);
	s.syncAsSint32LE(_nextActorIndex);
	s.syncAsSint32LE(_nextPositionOffset.x);
	s.syncAsSint32LE(_nextPositionOffset.y);
	s.syncAsSint32LE(_nextPosition.x);
	s.syncAsSint32LE(_nextPosition.y);
	s.syncAsSint32LE(_field_994);
	s.syncAsSint32LE(_field_998);
	s.syncAsSint32LE(_field_99C);
	s.syncAsSint32LE(_field_9A0);
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
		getScreen()->addGraphicToQueue(_resourceId, frameIndex, point, getGraphicsFlags(), _transparency, _priority);
	}
}

void Actor::drawNumber() {
	if (!_numberFlag01)
		return;

	getText()->loadFont(getWorld()->font1);
	getText()->drawCentered(_numberPoint, _numberStringWidth, (char *)&_numberString01);
}

void Actor::update() {
	if (!isVisible())
		return;

	switch (_status) {
	default:
		break;

	case kActorStatusGettingHurt:
		if (getWorld()->chapter == kChapter2) {
			MaxGetsSome();
		} else if (getWorld()->chapter == kChapter11 && _index == getSharedData()->getPlayerIndex()) {
			SarahGetsSome();
		}
		break;

	case kActorStatusRestarting:
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
					if (_updateCounter == 0) {
						++_updateCounter;
					} else {
						_updateCounter = 0;
						++_frameIndex;
					}
				} else {
					if (_vm->isGameFlagSet(kGameFlag556)) {
						Actor *player = getScene()->getActor();

						getSpeech()->playPlayer(453);
						getScene()->getActor(11)->hide();

						player->changeStatus(kActorStatusInteracting);
						player->setResourceId(player->getResourcesId(35));
						player->setDirection(kDirectionS);
						player->setFrameCount(GraphicResource::getFrameCount(_vm, player->getResourceId()));

						getCursor()->hide();
						getScene()->getActor(0)->changeDirection(kDirectionS);

						// Queue script
						getScript()->queueScript(getWorld()->getActionAreaById(2696)->scriptIndex, getSharedData()->getPlayerIndex());

						_vm->setGameFlag(kGameFlag279);
						_vm->clearGameFlag(kGameFlag368);

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
						getScene()->getActor(11)->updateGraphicData(25);
						_vm->setGameFlag(kGameFlag556);
					}
				}
			}

			if (_index == getSharedData()->getPlayerIndex()) {
				if (_frameIndex <= _frameCount - 1) {
					++_frameIndex;
				} else {
					_vm->clearGameFlag(kGameFlag239);
					getScene()->getActor(10)->changeStatus(kActorStatusEnabled2);
					hide();
					_vm->setGameFlag(kGameFlag238);

					// Queue script
					getScript()->queueScript(getWorld()->getActionAreaById(1000)->scriptIndex, getSharedData()->getPlayerIndex());
				}
			}

		} else if (getWorld()->chapter == kChapter11) {
			if (_index == getSharedData()->getPlayerIndex()) {
				if (_frameIndex <= _frameCount - 1)
					++_frameIndex;
				else
					SarahDies();
			}

			if (_index >= 10)
				TentacleDies();
		}
		break;

	case kActorStatusAttacking:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12)
				CrowDives();

			if (_index == getSharedData()->getPlayerIndex())
				MaxAttacks();

			if (_index == 11)
				ScareCrowAttacks();

		} else if (getWorld()->chapter == kChapter11) {
			if (_index >= 10 && _index < 16)
				TentacleWhips();

			if (_index == getSharedData()->getPlayerIndex())
				SarahAttacks();
		}
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12)
				CrowSwoops();

			if (_index == 11)
				ScareCrowRetreats();
		}
		break;

	case kActorStatusDisabled:
		_frameIndex = (_frameIndex + 1) % _frameCount;

		if (_vm->screenUpdateCount > _lastScreenUpdate + 300) {
			if (_vm->getRandom(100) < 50) {
				if (!getSpeech()->getSoundResourceId() || !getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
					if (canChangeStatus(10))
						changeStatus(kActorStatusFidget);
				}
			}
			_lastScreenUpdate = _vm->screenUpdateCount;
		}
		break;

	case kActorStatusWalking2:
		if (getWorld()->chapter == kChapter2) {
			if (_index > 12) {
				CrowClosesIn();
				return;
			}

			if (_index == 11) {
				ScareCrowClosesIn();
				return;
			}
		} else if (getWorld()->chapter == kChapter11) {
			switch (_index) {
			default:
				break;

			case 1:
				getSpecial()->run(nullptr, _index);
				return;

			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				TentacleRises();
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
		// fallthrough

	case kActorStatusWalking: {
		uint32 index = (_frameIndex >= _frameCount) ? 2 * _frameCount - (_frameIndex + 1) : _frameIndex;

		uint32 dist = (uint32)abs((double)getWalkIncrement(_direction, index));
		Common::Point point = _point1 + _point2;

		if (canMove(&point, _direction, dist, false)) {
			move(_direction, dist);
		} else if (canMove(&point, DIR(_direction + 1), dist, false)) {
			move(DIR(_direction + 1), dist);
		} else if (canMove(&point, DIR(_direction + 7), dist, false)) {
			move(DIR(_direction + 7), dist);
		} else if (canMove(&point, DIR(_direction + 2), dist, false)) {
			move(DIR(_direction + 2), dist);
		} else if (canMove(&point, DIR(_direction + 6), dist, false)) {
			move(DIR(_direction + 6), dist);
		}

		}
		break;

	case kActorStatusWalkingTo:
	case kActorStatusWalkingTo2: {
		uint32 index = (_frameIndex >= _frameCount) ? 2 * _frameCount - (_frameIndex + 1) : _frameIndex;

		int32 dist = (int32)abs((double)getWalkIncrement(_direction, index));

		Common::Point point = _point1 + _point2;
		Common::Point current = _data.points[_data.current];

		if (point.x < (int16)(current.x - (dist + 1))
		 || point.x > (int16)(current.x + (dist + 1))
		 || point.y < (int16)(current.y - (dist + 1))
		 || point.y > (int16)(current.y + (dist + 1))) {
			if (canMove(&point, _direction, (uint32)dist, false)) {
				move(_direction, (uint32)dist);
			} else {
				stopWalking();
			}
		} else {
			int32 area = getScene()->findActionArea(kActionAreaType1, current);
			if (_field_944 == 1 || _field_944 == 4)
				area = 1;

			if (area == -1 && _data.current >= (int32)(_data.count - 1)) {
				stopWalking();
			} else {
				_frameIndex = (_frameIndex + 1) % _frameCount;

				if (testActorCollision(&current, _direction)) {
					_point1.x = current.x - _point2.x; 
					_point1.y = current.y - _point2.y;

					if (_data.current < (int32)(_data.count - 1)) {
						_data.current++;

						changeDirection(_data.directions[_data.current]);
					} else {
						stopWalking();
					}
				} else {
					stopWalking();
				}
			}
		}

		}
		break;

	case kActorStatusInteracting:
	case kActorStatusHittingPumpkin:
		updateStatusInteracting();
		break;

	case kActorStatusStoppedInteracting:
		if (getSharedData()->actorEnableForStatus7) {
			getSharedData()->actorEnableForStatus7 = false;
			enable();
		}
		break;

	case kActorStatusEnabled:
		if (_field_944 != 5)
			updateStatusEnabled();
		break;

	case kActorStatusEnabled2:
		updateStatusEnabled2();
		break;

	case kActorStatusMorphingInto:
		updateStatusMorphing();
		break;

	case kActorStatusFidget:
		updateStatusBored();
		break;

	case kActorStatusShowingInventory:
	case kActorStatus10:
		_frameIndex = (_frameIndex + 1) % _frameCount;
		break;

	case kActorStatus8:
		if (getSharedData()->getFlag(kFlagIsEncounterRunning)
		 || !_soundResourceId
		 || getSound()->isPlaying(_soundResourceId)) {
			_frameIndex = (_frameIndex + 1) % _frameCount;
		} else {
			enable();
			_soundResourceId = kResourceNone;
		}
		break;
	}

	// Finish
	if (_soundResourceId && getSound()->isPlaying(_soundResourceId))
		setVolume();

	if (_index != getSharedData()->getPlayerIndex() && getWorld()->chapter != kChapter9)
		getSpecial()->run(nullptr, _index);

	updateReflectionData();

	if (_field_944 != 5)
		actionAreaCheck();
}

void Actor::changeStatus(ActorStatus actorStatus) {
	debugC(kDebugLevelActor, "[changeStatus] %d point1(%d:%d) point2(%d:%d)", actorStatus, _point1.x, _point1.y, _point2.x, _point2.y);

	switch (actorStatus) {
	default:
		break;

	case kActorStatusWalking:
	case kActorStatusWalking2:
		if ((getWorld()->chapter == kChapter2
		 && _index == getSharedData()->getPlayerIndex() && (_status == kActorStatus18 || _status == kActorStatusGettingHurt || _status == kActorStatusRestarting))
		 || (_status != kActorStatusEnabled && _status != kActorStatusFidget && _status != kActorStatusEnabled2 && _status != kActorStatusAttacking && _status != kActorStatus18))
			return;

		updateGraphicData(0);

		// Force status in some cases
		if (_status == kActorStatusEnabled2 || _status == kActorStatusAttacking || _status == kActorStatus18) {
			_status = kActorStatusWalking2;
			return;
		}
		break;

	case kActorStatusWalkingTo:
	case kActorStatusWalkingTo2:
		updateGraphicData(0);
		break;

	case kActorStatusInteracting:
	case kActorStatusHittingPumpkin:
		if (!strcmp(_name, "Big Crow"))
			_status = kActorStatusEnabled;
		break;

	case kActorStatusEnabled:
	case kActorStatusShowingInventory:
	case kActorStatusEnabled2:
		updateGraphicData(5);
		break;

	case kActorStatusDisabled:
		updateGraphicData(5);
		_resourceId = _graphicResourceIds[(_direction > kDirectionS ? kDirection8 - _direction : _direction) + 5];

		getScreen()->setFlag(-1);
		break;

	case kActorStatusStoppedInteracting:
		if (getWorld()->chapter == kChapter2 && _index == 10 && _vm->isGameFlagSet(kGameFlag279)) {
			Actor *actor = getScene()->getActor(0);
			actor->getPoint1()->x = _point2.x + _point1.x - actor->getPoint2()->x;
			actor->getPoint1()->y = _point2.y + _point1.y - actor->getPoint2()->y;
			actor->setDirection(kDirectionS);

			getSharedData()->setPlayerIndex(0);

			// Hide this actor and the show the other one
			hide();
			actor->show();

			_vm->clearGameFlag(kGameFlag279);

			getCursor()->show();
		}
		break;

	case kActorStatus8:
	case kActorStatus10:
	case kActorStatusRestarting:
		updateGraphicData(20);
		break;

	case kActorStatusFidget:
		if (getSharedData()->getFlag(kFlagIsEncounterRunning))
			return;

		if (_vm->getRandomBit() == 1 && canChangeStatus(15))
			updateGraphicData(15);
		else
			updateGraphicData(10);
		break;

	case kActorStatusAttacking:
	case kActorStatusGettingHurt:
		updateGraphicData(actorStatus == kActorStatusAttacking ? 10 : 15);
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2) {
			GraphicResource *resource = new GraphicResource(_vm);
			_frameIndex = 0;

			if (_index > 12)
				_resourceId = _graphicResourceIds[_direction + 30];

			if (getSharedData()->getPlayerIndex() == _index) {
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

void Actor::updateReflectionData() {
	if (!_processNewDirection)
		return;

	Common::Point sum = _point1 + _point2;
	if (_nextActionIndex != -1 && !getScene()->polygons()->get(getWorld()->actions[_nextActionIndex]->polygonIndex).contains(sum))
		return;

	ActorDirection direction = _direction;
	Common::Point position = sum;
	ResourceId resourceId;
	switch (_nextDirection) {
	default:
		// position is unchanged
		break;

	case kDirectionN:
	case kDirectionS:
		position.x = _nextPosition.x + sum.x;
		position.y = _nextPosition.y + sum.y;
		position.y += (int16)((_nextDirection == kDirectionN ? -1 : 1) * 2 * abs(sum.y - _nextPositionOffset.y));

		switch (_direction) {
		default:
			break;

		case kDirectionN:
			direction = kDirectionS;
			break;

		case kDirectionS:
			direction = kDirectionN;
			break;

		case kDirectionNE:
			direction = kDirectionSE;
			break;

		case kDirectionSE:
			direction = kDirectionNE;
			break;

		case kDirectionNW:
			direction = kDirectionSW;
			break;

		case kDirectionSW:
			direction = kDirectionNW;
			break;
		}
		break;

	case kDirectionW:
	case kDirectionE:
		position.x = _nextPosition.x + sum.x;
		position.x += (int16)((_nextDirection == kDirectionW ? -1 : 1) * 2 * abs(sum.x - _nextPositionOffset.x));
		position.y = _nextPosition.y + sum.y;

		switch (_direction) {
		default:
			break;

		case kDirectionW:
			direction = kDirectionE;
			break;

		case kDirectionE:
			direction = kDirectionW;
			break;

		case kDirectionSW:
			direction = kDirectionSE;
			break;

		case kDirectionNW:
			direction = kDirectionNE;
			break;

		case kDirectionSE:
			direction = kDirectionSW;
			break;

		case kDirectionNE:
			direction = kDirectionNW;
			break;
		}
		break;

	case kDirectionNW:
	case kDirectionSE:
		position.x = _nextPosition.x + _nextPositionOffset.x;
		position.y = _nextPosition.y + _nextPositionOffset.y;
		position.x += (int16)((_nextDirection == kDirectionNW ? -1 : 1) * abs(sum.y - _nextPositionOffset.y));
		position.y += (int16)((_nextDirection == kDirectionNW ? -1 : 1) * abs(sum.x - _nextPositionOffset.x));

		switch (_direction) {
		default:
			break;

		case kDirectionN:
			direction = kDirectionE;
			break;

		case kDirectionS:
			direction = kDirectionW;
			break;

		case kDirectionW:
			direction = kDirectionS;
			break;

		case kDirectionE:
			direction = kDirectionN;
			break;

		case kDirectionNW:
			direction = kDirectionSE;
			break;

		case kDirectionNE:
			direction = kDirectionNE;
			break;

		case kDirectionSW:
			direction = kDirectionSW;
			break;

		case kDirectionSE:
			direction = kDirectionNW;
			break;
		}
		break;

	case kDirectionSW:
	case kDirectionNE:
		if (_nextDirection == kDirectionSW) {
			position.x = (int16)(_nextPosition.x + _nextPositionOffset.x - abs(sum.y - _nextPositionOffset.y));
			position.y = (int16)(_nextPosition.y + _nextPositionOffset.y + abs(sum.x - _nextPositionOffset.x));
		} else {
			double deltaX = sum.x * -0.56666666;
			double deltaY = (419 - (sum.y + deltaX)) * 0.87613291;
			position.x = (int16)(sum.x + 2 * (_nextPositionOffset.x - deltaY));
			position.y = (int16)(sum.y + 2 * (_nextPositionOffset.y - (sum.y + deltaX + (deltaY * 0.56666666))));
		}

		switch (_direction) {
		default:
			break;

		case kDirectionS:
			direction = kDirectionE;
			break;

		case kDirectionN:
			direction = kDirectionW;
			break;

		case kDirectionE:
			direction = kDirectionS;
			break;

		case kDirectionW:
			direction = kDirectionN;
			break;

		case kDirectionSE:
			direction = kDirectionSE;
			break;

		case kDirectionSW:
			direction = kDirectionNE;
			break;

		case kDirectionNE:
			direction = kDirectionSW;
			break;

		case kDirectionNW:
			direction = kDirectionNW;
			break;
		}
		break;

	case kDirection8:
		position = _nextPosition + sum;
		break;
	}

	// Get the next resource index offset
	uint32 index = 0;
	while (_graphicResourceIds[index] != _resourceId) {
		index++;

		if (index >= 55)
			error("[Actor::updateReflectionData] Invalid resource id index");
	}

	// Compute resource id and adjust frame count
	Actor *nextActor = getScene()->getActor(_nextActorIndex);

	resourceId = nextActor->getResourcesId((index - index % 5) + (direction > kDirectionS ? 8 - direction : direction));
	nextActor->setFrameCount(GraphicResource::getFrameCount(_vm, resourceId));

	// Adjust graphic resource and position
	uint32 frameIndex = _frameIndex % nextActor->getFrameCount();
	nextActor->setPosition(position.x, position.y, direction, frameIndex);
	nextActor->setFrameIndex(frameIndex);
	nextActor->setResourceId(resourceId);
}

void Actor::changeDirection(ActorDirection actorDirection) {
	_direction = actorDirection;

	if (_field_944 == 5)
		return;

	switch (_status) {
	default:
		break;

	case kActorStatusDisabled:
	case kActorStatusEnabled:
	case kActorStatusEnabled2:
		_resourceId = _graphicResourceIds[(actorDirection > kDirectionS ? kDirection8 - actorDirection : actorDirection) + 5];
		_frameCount = GraphicResource::getFrameCount(_vm, _resourceId);
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2)
			if (_index == 11) // we are actor 11
				_resourceId = _graphicResourceIds[(actorDirection > kDirectionS) ? kDirection8 - actorDirection : actorDirection];
		break;

	case kActorStatusWalking:
	case kActorStatusWalkingTo:
	case kActorStatusWalking2:
		_resourceId = _graphicResourceIds[(actorDirection > kDirectionS ? kDirection8 - actorDirection : actorDirection)];
		_frameCount = GraphicResource::getFrameCount(_vm, _resourceId);
		break;

	case kActorStatus8:
		_resourceId = _graphicResourceIds[(actorDirection > kDirectionS ? kDirection8 - actorDirection : actorDirection) + 20];
		break;
	}
}

void Actor::faceTarget(uint32 target, DirectionFrom from) {
	debugC(kDebugLevelActor, "[Actor] Facing target %d using direction from %d", target, from);

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

		point.x = (int16)Common::Rational(frameRect.width(), 2).toInt()  + object->x;
		point.y = (int16)Common::Rational(frameRect.height(), 2).toInt() + object->y;
		}
		break;

	case kDirectionFromPolygons: {
		int32 actionIndex = getWorld()->getActionAreaIndexById(target);
		if (actionIndex == -1) {
			warning("[Actor::faceTarget] No ActionArea found for id %d", target);
			return;
		}

		Polygon polygon = getScene()->polygons()->get(getWorld()->actions[actionIndex]->polygonIndex);

		point.x = polygon.boundingRect.left + (polygon.boundingRect.right  - polygon.boundingRect.left) / 2;
		point.y = polygon.boundingRect.top  + (polygon.boundingRect.bottom - polygon.boundingRect.top)  / 2;
		}
		break;

	case kDirectionFromActor: {
		Actor *actor = getScene()->getActor((ActorIndex)target);
		point = *actor->getPoint1() + *actor->getPoint2();
		}
		break;

	case kDirectionFromParameters:
		point.x = point.y = (int16)target;
		break;
	}

	Common::Point mid(_point1.x + _point2.x, _point1.y + _point2.y);
	changeDirection(getAngle(mid, point));
}

void Actor::setPosition(int16 newX, int16 newY, ActorDirection newDirection, uint32 frame) {
	_point1.x = newX - _point2.x;
	_point1.y = newY - _point2.y;

	if (_direction != kDirection8)
		changeDirection(newDirection);

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

void Actor::clearReflectionData() {
	_processNewDirection = false;
	_invertPriority = false;
	_nextDirection = kDirectionN;
	_nextActionIndex = 0;
	_nextActorIndex = 0;
	_nextPositionOffset = Common::Point(0, 0);
	_nextPosition = Common::Point(0, 0);
	_field_994 = 0;
	_field_998 = 0;
	_field_99C = 0;
	_field_9A0 = 0;
}

bool Actor::checkBoredStatus() const {
	if (_status != kActorStatusFidget)
		return false;

	int index;
	for (index = 10; index < 20; index++) {
		if (_graphicResourceIds[index] == _resourceId)
			break;
	}

	return (index >= 15);
}

bool Actor::canReach(const Common::Point &point) {
	// Compute point and delta
	Common::Point sum(_point1.x + _point2.x, _point1.y + _point2.y);
	Common::Point delta = point - sum;

	// Compute modifiers
	int16 a1 = 0;
	int16 a2 = 0;
	int16 a3 = 0;

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
		if (canMove(&sum, a3 >= 2 ? kDirectionS : kDirectionN, (uint32)abs(delta.y), false)) {
			_data.points[0] = point;
			_data.current   = 0;
			_data.count     = 1;

			return true;
		}
	}

	if (point.x == sum.x) {
		ActorDirection actorDir = a3 >= 2 ? kDirectionS : kDirectionN;
		if (canMove(&sum, actorDir, (uint32)abs(delta.y), false)) {
			_data.points[0] = point;
			_data.current   = 0;
			_data.count     = 1;

			changeDirection(actorDir);

			return true;
		}

		return false;
	}

	if (point.y == sum.y) {
		ActorDirection actorDir = (a3 != 0 && a3 != 3) ? kDirectionE : kDirectionW;

		if (canMove(&sum, actorDir, (uint32)abs(delta.x), true)) {
			_data.points[0] = point;
			_data.current   = 0;
			_data.count     = 1;

			changeDirection(actorDir);

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
			point1 = Common::Point((int16)(sum.x + abs(delta.x) * a1), (int16)(sum.y + abs(delta.x) * a2));
			point2 = Common::Point(sum.x                             , (int16)(sum.y + abs(abs(delta.x) - abs(delta.y)) * a2));
			count1 = (uint32)abs(point1.x - sum.x);
			count2 = (uint32)abs(point1.y - point.y);

			switch (a3) {
			default:
				error("[Actor::canReach] Invalid value for a3");
				break;

			case 0:
				direction1 = kDirectionNW;
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
				direction1 = kDirectionSW;
				direction2 = kDirectionS;
				break;
			}
		} else {
			point1 = Common::Point((int16)(sum.x + abs(delta.y) * a1),                     (int16)(sum.y + abs(delta.y) * a2));
			point2 = Common::Point((int16)(sum.x + abs(abs(delta.y) - abs(delta.x)) * a1), sum.y);
			count1 = (uint32)abs(abs(delta.y) * a2);
			count2 = (uint32)abs(point1.x - point.x);

			switch (a3) {
			default:
				error("[Actor::canReach] Invalid value for a3");
				break;

			case 0:
				direction1 = kDirectionNW;
				direction2 = kDirectionW;
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
				direction1 = kDirectionSW;
				direction2 = kDirectionW;
				break;
			}
		}

		// Check scene rects
		if (getWorld()->chapter != kChapter2 || strcmp(_name, "Big Crow")) {
			Common::Rect currentRect = getWorld()->sceneRects[getWorld()->sceneRectIdx];

			if (!currentRect.contains(point1) || !currentRect.contains(point2))
				return false;
		}

		if (canMove(&sum,    direction1, count1, true)
		 && canMove(&point1, direction2, count2, true)) {
			_data.points[0] = point1;
			_data.points[1] = point;
			_data.current = 0;
			_data.count = 2;

			changeDirection(direction1);

			_data.directions[1] = direction2;

			return true;
		}

		if (canMove(&sum,    direction2, count2, true)
		 && canMove(&point2, direction1, count1, true)) {
			_data.points[0] = point2;
			_data.points[1] = point;
			_data.current = 0;
			_data.count = 2;

			changeDirection(direction2);

			_data.directions[1] = direction1;

			return true;
		}

		// Compute actor actions
		int actorActions[5];
		ActionArea *actorArea = getWorld()->actions[_actionIdx3];
		for (uint32 i = 0; i < 5; i++) {
			if (actorArea->paths[i])
				actorActions[i] = actorArea->paths[i];
			else
				actorActions[i] = -1;
		}

		if (actorActions[0] == -1)
			actions.push_back(_actionIdx3);

		// Process all scene actions
		for (uint32 i = 0; i < getWorld()->actions.size(); i++) {
			// Check each area action against each actor action
			ActionArea *area = getWorld()->actions[i];
			for (uint32 j = 0; j < 5; j++) {
				for (uint32 k = 0; k < 5; k++) {
					if (area->paths[j] == actorActions[k])
						actions.push_back(i);
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Process actions
		_frameNumber = 0;

		if (abs(sum.x - point.x) > abs(sum.y - point.y)) {
			if (sum.x <= point.x) {
				if (!findLeftPath(sum, point, &actions))
					return false;
			} else {
				if (!findRightPath(sum, point, &actions))
					return false;
			}

			changeDirection(_data.directions[0]);

			return true;
		}

		if (sum.y > point.y) {
			if (!findUpPath(sum, point, &actions))
				return false;

			changeDirection(_data.directions[0]);

			return true;
		}

		// last case: sum.y < point.y
		if (!findDownPath(sum, point, &actions))
			return false;

		changeDirection(_data.directions[0]);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Last case: abs(delta.x) == abs(delta.y)

	// Compute direction
	ActorDirection actorDir = kDirectionSW;
	switch (a3) {
	default:
		break;

	case 0:
		actorDir = kDirectionNW;
		break;

	case 1:
		actorDir = kDirectionNE;
		break;

	case 2:
		actorDir = kDirectionSE;
		break;
	}

	if (!canMove(&sum, actorDir, (uint32)abs(delta.y), true))
		return false;

	// Update actor data
	_data.points[0] = point;
	_data.current   = 0;
	_data.count     = 1;

	// Update actor from direction
	changeDirection(actorDir);

	return true;
}

void Actor::forceTo(int16 actorX, int16 actorY, bool doSpeech) {
	if (canReach(Common::Point(actorX, actorY))) {
		if (_status <= kActorStatus11)
			changeStatus(kActorStatusWalkingTo);
		else
			changeStatus(kActorStatusWalkingTo2);
	} else if (doSpeech) {
		getSpeech()->playIndexed(1);
	}
}

void Actor::setupReflectionData(ActorIndex nextActor, int32 actionAreaId, ActorDirection nextDirection, const Common::Point &nextPosition, bool invertPriority, const Common::Point &nextPositionOffset) {
	_nextActorIndex = nextActor;
	_nextActionIndex = (actionAreaId != -1) ? getWorld()->getActionAreaIndexById(actionAreaId) : -1;
	_nextDirection = nextDirection;
	_nextPosition = nextPosition;
	_invertPriority = invertPriority;

	Common::Point offset;
	if (actionAreaId != -1) {
		if (nextPositionOffset.x) {
			offset = nextPositionOffset;
		} else {
			Polygon polygon = getScene()->polygons()->get((uint32)_nextActionIndex + 1);

			offset = polygon.points[0];
			// Iterate through points
			if (polygon.count() > 1) {
				for (uint i = 1; i < polygon.count(); i++) {
					Common::Point point = polygon.points[i];

					switch (nextDirection) {
					default:
						break;

					case kDirectionN:
						if (offset.y > point.y)
							offset.y = point.y;
						break;

					case kDirectionNW:
						if (offset.y > point.y)
							offset.y = point.y;

						if (offset.x > point.x)
							offset.x = point.x;
						break;

					case kDirectionW:
						if (offset.x > point.x)
							offset.x = point.x;
						break;

					case kDirectionSW:
						if (offset.y < point.y)
							offset.y = point.y;

						if (offset.x > point.x)
							offset.x = point.x;
						break;

					case kDirectionS:
						if (offset.y < point.y)
							offset.y = point.y;
						break;

					case kDirectionSE:
						if (offset.y < point.y)
							offset.y = point.y;

						if (offset.x < point.x)
							offset.x = point.x;
						break;

					case kDirectionE:
						if (offset.x < point.x)
							offset.x = point.x;
						break;

					case kDirectionNE:
						if (offset.y > point.y)
							offset.y = point.y;

						if (offset.x < point.x)
							offset.x = point.x;
						break;

					case kDirection8:
						offset = Common::Point(0, 0);
						break;
					}
				}
			}
		}
	}

	_nextPositionOffset = offset;

	int16 cosValue = (int16)(cos(0.523598775) * 1000.0);
	int16 sinValue = (int16)(sin(0.523598775) * 1000.0);

	_field_994 = offset.x - cosValue;
	_field_998 = offset.y + sinValue;
	_field_99C = offset.x + cosValue;
	_field_9A0 = offset.y - sinValue;

	_processNewDirection = true;

	updateReflectionData();
}

bool Actor::aNicePlaceToTalk(Common::Point *point, int32 *param) {
	Actor *player = getScene()->getActor();

	int16 offset = 65;
	if (getWorld()->chapter != kChapter2 || _index != 8)
		offset = 40;

	int32 parameter;
	Common::Point actorPoint = _point1 + _point2, pt = actorPoint;

	if (getWorld()->chapter != kChapter2 || _index != 1) {
		Common::Point diff = *player->getPoint1() - _point1;

		if (abs(diff.y) <= abs(diff.x)) {

			if (diff.x < 0) {

				parameter = 2;
				pt.x -= offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 3;
				pt = actorPoint;
				pt.x -= offset;
				pt.y += offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 1;
				pt = actorPoint;
				pt.x -= offset;
				pt.y -= offset;

			} else {

				parameter = 6;
				pt.x += offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 5;
				pt = actorPoint;
				pt.x += offset;
				pt.y += offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 7;
				pt = actorPoint;
				pt.x += offset;
				pt.y -= offset;
			}

		} else {

			if (diff.y >= 0) {

				parameter = 4;
				pt.y += offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 3;
				pt = actorPoint;
				pt.x -= offset;
				pt.y += offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 5;
				pt = actorPoint;
				pt.x += offset;
				pt.y += offset;

			} else {

				parameter = 0;
				pt.y -= offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 1;
				pt = actorPoint;
				pt.x -= offset;
				pt.y -= offset;
				if (getScene()->findActionArea(kActionAreaType2, pt) != -1)
					goto processActor;

				parameter = 7;
				pt = actorPoint;
				pt.x += offset;
				pt.y -= offset;
			}
		}
	} else {
		parameter = 5;
		pt.x += offset;
		pt.y += offset;
	}

	if (getScene()->findActionArea(kActionAreaType2, pt) == -1)
		return false;

processActor:
	if (!player->canReach(pt))
		return false;

	*point = pt;
	*param = abs(parameter + 4) & 7;

	return true;
}

bool Actor::canMove(Common::Point *point, ActorDirection direction, uint32 distance, bool hasDelta) {
	if (_field_944 == 1 || _field_944 == 4)
		return true;

	int16 x = (hasDelta ? point->x : point->x + deltaPointsArray[direction][0]);
	int16 y = (hasDelta ? point->y : point->y + deltaPointsArray[direction][1]);

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

		if (!testActorCollision(point, direction))
			return false;
	}

	if (distance > 0) {
		uint32 allowed = 0;

		while (getScene()->findActionArea(kActionAreaType1, Common::Point(x, y), true) != -1) {
			x += deltaPointsArray[direction][0];
			y += deltaPointsArray[direction][1];

			++allowed;

			if (allowed >= distance)
				return true;
		}

		return false;
	}

	return true;
}

void Actor::move(ActorDirection actorDir, uint32 dist) {
	if (_frameCount == 0)
		error("[Actor::move] Invalid frame count (cannot be 0)");

	_lastScreenUpdate = _vm->screenUpdateCount;

	Common::Point sum(_point1.x + _point2.x, _point1.y + _point2.y);
	int32 panning = getSound()->calculatePanningAtPoint(sum);

	switch (_status) {
	default:
		break;

	case kActorStatusWalking:
	case kActorStatusWalkingTo:
	case kActorStatusWalking2:
	case kActorStatusWalkingTo2:
		incPosition(actorDir, (int16)dist, &_point1);

		_frameIndex = (_frameIndex + 1) % _frameCount;

		if (_walkingSound1 != kResourceNone) {

			// Compute volume
			int32 vol = (int32)sqrt((double)-Config.sfxVolume);
			if (_index != getSharedData()->getPlayerIndex())
				vol += (int32)sqrt((double)abs(getSound()->calculateVolumeAdjustement(sum, 10, 0)));

			int32 volume = (Config.sfxVolume + vol) * (Config.sfxVolume + vol);
			if (volume > 10000)
				volume = 10000;

			if (_field_944 != 1 && _field_944 != 4) {
				// Compute resource Id
				ResourceId resourceId = kResourceNone;
				if (getWorld()->actions[_actionIdx3]->soundResourceIdFrame != kResourceNone && strcmp((char *)&_name, "Crow") && strcmp((char *)&_name, "Big Crow")) {
					if (_frameIndex == _field_64C)
						resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceIdFrame + (int)rnd(2));
					else if (_frameIndex == _field_650)
						resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceId + (int)rnd(2));
				} else {
					if (_frameIndex == _field_64C)
						resourceId = (ResourceId)(_walkingSound1 + (int)rnd(2));
					else if (_frameIndex == _field_650)
						resourceId = (ResourceId)(_walkingSound3 + (int)rnd(2));
				}

				// Play sound
				getSound()->playSound(resourceId, false, -volume, panning);
			}
		}
		break;

	case kActorStatus18:
		if (getWorld()->chapter == kChapter2) {
			incPosition(actorDir, (int16)dist, &_point1);

			if (_walkingSound1 == kResourceNone)
				break;

			// Compute volume
			int32 vol = getWorld()->actions[_actionIdx3]->volume;
			if (_index != getSharedData()->getPlayerIndex())
				vol += (int32)sqrt((double)abs(getSound()->calculateVolumeAdjustement(sum, 10, 0)));

			int32 volume = (Config.sfxVolume + vol) * (Config.sfxVolume + vol);
			if (volume > 10000)
				volume = 10000;

			// Compute resource Id
			ResourceId resourceId = kResourceNone;
			if (getWorld()->actions[_actionIdx3]->soundResourceIdFrame != kResourceNone && strcmp((char *)&_name, "Crow") && strcmp((char *)&_name, "Big Crow")) {
				if (_frameIndex == _field_64C)
					resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceIdFrame + (int)rnd(2));
				else if (_frameIndex == _field_650)
					resourceId = (ResourceId)(getWorld()->actions[_actionIdx3]->soundResourceId + (int)rnd(2));
			} else {
				if (_frameIndex == _field_64C)
					resourceId = (ResourceId)(_walkingSound1 + (int)rnd(2));
				else if (_frameIndex == _field_650)
					resourceId = (ResourceId)(_walkingSound3 + (int)rnd(2));
			}

			// Play sound
			getSound()->playSound(resourceId, false, -volume, panning);
		}
		break;
	}
}

bool Actor::testActorCollision(Common::Point *point, ActorDirection dir) {
	int32 dist = getStride(dir, (_frameIndex >= _frameCount) ? 2 * _frameCount - (_frameIndex + 1) : _frameIndex);

	int32 x = point->x + deltaPointsArray[dir][0] * dist - (_field_948 + 10);
	int32 y = point->y + deltaPointsArray[dir][1] * dist - (_field_94C + 10);
	int32 x1 = x + 2 * _field_948 + 20;
	int32 y1 = y + 2 * _field_94C + 20;

	for (int32 i = 0; i < (int32)getWorld()->actors.size(); i++) {
		if (i == _index)
			continue;

		Actor *actor = getScene()->getActor(i);

		if (!actor->isOnScreen())
			continue;

		if (actor->_field_944)
			continue;

		int32 x2 = actor->getPoint1()->x + actor->getPoint2()->x -    (actor->getField948() + 15);
		int32 y2 = actor->getPoint1()->y + actor->getPoint2()->y -    (actor->getField94C() + 10);
		int32 x3 = actor->getPoint1()->x + actor->getPoint2()->x + 2 * actor->getField948() + 15;
		int32 y3 = actor->getPoint1()->y + actor->getPoint2()->y + 2 * actor->getField94C() + 10;

		if (i == getSharedData()->getPlayerIndex() && getWorld()->chapter != kChapter11) {
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

			switch (_direction) {
			default:
				break;

			case kDirectionNW:
				if (x4 >= x)
					break;
				// fallthrough

			case kDirectionN:
				if (y4 >= y)
					break;

				return false;

			case kDirectionW:
				if (x4 < x)
					return false;

				break;

			case kDirectionSW:
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

void Actor::drawInventory() {
	Actor *player = getScene()->getActor();
	Common::Point mouse = getCursor()->position();
	bool keepField = false;

	uint count = inventory.find();
	for (uint32 i = 0; i < count; i++) {
		// Compute points
		Common::Point coords;
		adjustCoordinates(&coords);

		Common::Point ringPoint = Inventory::getInventoryRingPoint(_vm, count, i);
		Common::Point point = coords + Common::Point(player->getPoint2()->x + ringPoint.x, player->getPoint2()->y / 2 - ringPoint.y);

		if (mouse.x < point.x || mouse.x > (point.x + 40) || mouse.y < point.y || mouse.y > (point.y + 40)) {
			getScreen()->addGraphicToQueue(getWorld()->inventoryIconsNormal[inventory[i] - 1],
			                               0,
			                               point,
			                               kDrawFlagNone,
			                               0,
			                               1);
		} else {
			if (getWorld()->field_120 != (int32)(i + 1)) {
				getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 3));
				Inventory::describe(_vm, inventory[i] - 1);
			}

			getWorld()->field_120 = i + 1;
			keepField = true;

			getScreen()->addGraphicToQueue(getWorld()->inventoryIconsActive[inventory[i] - 1],
			                               0,
			                               point,
			                               kDrawFlagNone,
			                               0,
			                               1);
		}

		if (getWorld()->chapter == kChapter4)
			updateNumbers(inventory[i] - 1, point);
	}

	if (!keepField)
		getWorld()->field_120 = 0;
}

void Actor::stopWalking() {
	changeStatus(_status <= 11 ? kActorStatusEnabled : kActorStatusEnabled2);
	_data.current = 0;
}

//////////////////////////////////////////////////////////////////////////
// Static update methods
//////////////////////////////////////////////////////////////////////////
void Actor::crowsReturn(AsylumEngine *engine) {
	engine->clearGameFlag(kGameFlag438);
	engine->clearGameFlag(kGameFlag439);
	engine->clearGameFlag(kGameFlag440);
	engine->clearGameFlag(kGameFlag441);
	engine->clearGameFlag(kGameFlag442);

	// Reset shared data
	engine->data()->resetChapter2Data();

	engine->scene()->getActor(13)->enable();
	engine->scene()->getActor(13)->forceTo(2300, 71, false);

	engine->scene()->getActor(14)->enable();
	engine->scene()->getActor(14)->forceTo(2600, 1300, false);

	engine->scene()->getActor(15)->enable();
	engine->scene()->getActor(15)->forceTo(2742, 615, false);

	engine->scene()->getActor(16)->enable();
	engine->scene()->getActor(16)->forceTo(2700, 1200, false);

	engine->scene()->getActor(17)->enable();
	engine->scene()->getActor(17)->forceTo(2751, 347, false);

	engine->scene()->getActor(18)->enable();
	engine->scene()->getActor(18)->forceTo(2420, 284, false);

	engine->scene()->getActor(19)->enable();
	engine->scene()->getActor(19)->forceTo(2800, 370, false);

	engine->scene()->getActor(20)->enable();
	engine->scene()->getActor(20)->forceTo(1973, 1, false);

	engine->scene()->getActor(21)->enable();
	engine->scene()->getActor(21)->forceTo(2541, 40, false);
}

void Actor::morphInto(AsylumEngine *engine, int nextPlayer) {
	WorldStats *world = engine->scene()->worldstats();
	if (world->chapter != kChapter9)
		return;

	Actor *player = engine->scene()->getActor();
	world->nextPlayer = nextPlayer;

	switch (engine->data()->getPlayerIndex()) {
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

	player->changeStatus(kActorStatusMorphingInto);
}

//////////////////////////////////////////////////////////////////////////
// Update methods
//////////////////////////////////////////////////////////////////////////

void Actor::updateStatusInteracting() {
	if (getWorld()->chapter != kChapter2 || _frameIndex != 6 || _status == kActorStatusInteracting) { /* Original check: _status  <= kActorStatus11 */
		if (_frameIndex < _frameCount - 1) {
			++_frameIndex;
		} else {
			if (_status == kActorStatusInteracting)
				changeStatus(kActorStatusStoppedInteracting);
			else
				changeStatus(kActorStatusStoppedHitting);
		}
	} else {
		if (_index == getSharedData()->getPlayerIndex())
			checkPumpkinDeath();

		++_frameIndex;
	}
}

void Actor::checkPumpkinDeath() {
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

		getSharedData()->setChapter2Counter(5, getSharedData()->getChapter2Counter(5) + 1);

		getWorld()->getObjectById(objectToUpdate)->setNextFrame(8);
		getSound()->playSound(getWorld()->soundResourceIds[17], false, Config.sfxVolume - 10);
		getWorld()->getObjectById(objectToDisable)->disable();
	}
}

void Actor::updateStatusEnabled() {
	if (_frameCount == 0)
		error("[Actor::updateStatusEnabled] Actor has no frame!");

	_frameIndex = (_frameIndex + 1) % _frameCount;

	if (_vm->screenUpdateCount > _lastScreenUpdate + 300) {
		// All actors except Crow and Armed Max
		if (strcmp((char *)&_name, "Crow") && strcmp((char *)_name, "Armed Max")) {
			if (_vm->getRandom(100) < 50
			 && (!getSpeech()->getSoundResourceId() || !getSound()->isPlaying(getSpeech()->getSoundResourceId()))
			 && canChangeStatus(10))
				changeStatus(kActorStatusFidget);

			_lastScreenUpdate = _vm->screenUpdateCount;
		}
	}

	// Actor: Player
	if (_index == getSharedData()->getPlayerIndex()) {
		if (_vm->lastScreenUpdate && (_vm->screenUpdateCount > _vm->lastScreenUpdate + 500)) {

			if (_vm->isGameFlagNotSet(kGameFlagScriptProcessing)
			 && isVisible()
			 && !getSharedData()->getFlag(kFlagIsEncounterRunning)
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
				forceTo(1460, -100, false);
				break;

			case 1:
				setPosition(300, 0, kDirectionN, 0);
				forceTo(1700, 1400, false);
				break;

			case 2:
				setPosition(1560, -100, kDirectionN, 0);
				forceTo(-300, 1470, false);
				break;

			case 3:
				setPosition(1150, 1400, kDirectionN, 0);
				forceTo(-250, 0, false);
				break;
			}
		}

		return;
	}

	// All other actors
	if (getWorld()->chapter != kChapter2 || _index != 8) {
		if (_field_944 == 4) {
			Common::Rect frameRect = GraphicResource::getFrameRect(_vm, getWorld()->backgroundImage, 0);
			forceTo((int16)rnd(frameRect.width() + 200) - 100, (int16)rnd(frameRect.height() + 200) - 100, false);
		} else {
			// Actor: Crow
			if (rnd(1000) < 5 || !strcmp(_name, "Crow")) {
				if (_actionIdx2 != -1) {

					// Process action area
					int32 areaIndex = getWorld()->getRandomActionAreaIndexById(_actionIdx2);
					if (areaIndex != -1) {

						ActionArea *area = getWorld()->actions[areaIndex];
						Polygon poly = getScene()->polygons()->get(area->polygonIndex);

						Common::Point pt(poly.boundingRect.left + (int16)rnd((uint16)poly.boundingRect.width()),
						                 poly.boundingRect.top  + (int16)rnd((uint16)poly.boundingRect.height()));

						if (!getSharedData()->getFlag(kFlagActorUpdateEnabledCheck)) {
							if (!isInActionArea(pt, area)) {
								Common::Point polyPoint = poly.points[rnd(poly.count())];
								forceTo(polyPoint.x, polyPoint.y, false);
							} else {
								forceTo(pt.x, pt.y, false);
							}
						}
					}
				}
			}
		}
	} else {
		switch (getSharedData()->getActorUpdateStatusEnabledCounter()) {
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

void Actor::updateStatusEnabledProcessStatus(int16 testX, int16 testY, uint32 counter, int16 setX, int16 setY) {
	int32 xsum = _point1.x + _point2.x;
	int32 ysum = _point1.y + _point2.y;

	if (xsum != testX || ysum != testY) {
		if (rnd(1000) < 5)
			forceTo(testX, testY, false);
	} else {
		getSharedData()->setActorUpdateStatusEnabledCounter(counter);

		if (rnd(1000) < 5)
			forceTo(setX, setY, false);
	}
}

void Actor::updateStatusBored() {
	if (_index == getSharedData()->getPlayerIndex()
	 && getWorld()->chapter != kChapter9
	 && getWorld()->actorType == kActorMax
	 && _frameIndex == 0
	 && checkBoredStatus()) {
		if (!getSpeech()->getSoundResourceId() || !getSound()->isPlaying(getSpeech()->getSoundResourceId()))
			getSpeech()->playPlayer(13);
	}

	++_frameIndex;
	if (_frameIndex == _frameCount) {
		enable();
		_lastScreenUpdate = _vm->screenUpdateCount;
	}
}

void Actor::CrowClosesIn() {
	// Compute distance
	uint32 frameIndex = _frameIndex;
	if (_frameIndex >= _frameCount)
		frameIndex = 2 * _frameCount - (_frameIndex + 1);

	uint32 distance = (uint32)abs((double)getWalkIncrement(_direction, frameIndex));

	// Face actor
	faceTarget((uint32)getSharedData()->getPlayerIndex(), kDirectionFromActor);

	int32 data = getSharedData()->crowsData[_index + 25];
	if (data > 0) {
		_direction = DIR(_direction + 4);
		getSharedData()->crowsData[_index + 25] = data - 1;
	}

	// Compute coordinates and distance
	Actor *player = getScene()->getActor();
	Common::Point sumPlayer = *player->getPoint1() + *player->getPoint2();
	Common::Point sum = _point1 + _point2;

	uint32 absX = (uint32)abs(sum.x - sumPlayer.x);
	uint32 absY = (uint32)abs(sum.y - sumPlayer.y);

	// Adjust distance
	if (absX <= absY)
		absX = absY;

	if (sum.y < sumPlayer.y)
		absX += 20;

	if (absX >= 50) {
		move(_direction, distance);
	} else {
		_frameIndex = 0;

		getSharedData()->crowsData[2 * _index + 30] = player->getPoint1()->x - _point1.x;
		getSharedData()->crowsData[2 * _index + 31] = player->getPoint1()->y - _point1.y;

		changeStatus(kActorStatus18);
	}
}

void Actor::ScareCrowClosesIn() {
	bool processEnd = true;
	Actor *player = getScene()->getActor();
	Common::Point sum = _point1 + _point2;
	Common::Point sumPlayer = *player->getPoint1() + *player->getPoint2();

	// Compute distance
	uint32 frameIndex = _frameIndex;
	if (_frameIndex >= _frameCount)
		frameIndex = 2 * _frameCount - (_frameIndex + 1);

	uint32 distance = (uint32)abs((double)getWalkIncrement(_direction, frameIndex));

	// Update status
	if (player->getStatus() == kActorStatusRestarting || !getScene()->getActor(10)->isVisible()) {
		changeStatus(kActorStatusEnabled);
		getSharedData()->crowsData[_index - 11] = 160;
	}

	// Face player
	faceTarget((uint32)getSharedData()->getPlayerIndex(), kDirectionFromActor);

	// Compute coordinates
	Common::Point delta  = Common::Point((sumPlayer.x + sum.x) / 2,       (sumPlayer.y + sum.y) / 2);
	Common::Point point1 = Common::Point((delta.x     + sum.x) / 2,       (delta.y     + sum.y) / 2);
	Common::Point point2 = Common::Point((delta.x     + sumPlayer.x) / 2, (delta.y     + sumPlayer.y) / 2);

	if (getScene()->findActionArea(kActionAreaType1, delta) == -1
	 || getScene()->findActionArea(kActionAreaType1, point1) == -1
	 || getScene()->findActionArea(kActionAreaType1, point2) == -1) {
		processEnd = false;

		if (determineLeftOrRight(sum, sumPlayer) == false) {
			if (canMove(&sum, DIR(_direction + 1), distance, false)) {
				move(DIR(_direction + 1), distance);
			} else if (canMove(&sum, DIR(_direction + 2), distance, false)) {
				move(DIR(_direction + 2), distance);
			} else if (canMove(&sum, DIR(_direction + 3), distance, false)) {
				move(DIR(_direction + 3), distance);
			} else if (canMove(&sum, DIR(_direction + 4), distance, false)) {
				move(DIR(_direction + 4), distance);
			}
		} else {
			if (canMove(&sum, DIR(_direction + 7), distance, false)) {
				move(DIR(_direction + 7), distance);
			} else if (canMove(&sum, DIR(_direction + 6), distance, false)) {
				move(DIR(_direction + 6), distance);
			} else if (canMove(&sum, DIR(_direction + 5), distance, false)) {
				move(DIR(_direction + 5), distance);
			} else if (canMove(&sum, DIR(_direction + 4), distance, false)) {
				move(DIR(_direction + 4), distance);
			}
		}
	} else {
		if (canMove(&sum, DIR(_direction + 1), distance, false)) {
			move(DIR(_direction + 1), distance);
		} else if (canMove(&sum, DIR(_direction + 2), distance, false)) {
			move(DIR(_direction + 2), distance);
		} else if (canMove(&sum, DIR(_direction + 7), distance, false)) {
			move(DIR(_direction + 7), distance);
		} else if (canMove(&sum, DIR(_direction + 6), distance, false)) {
			move(DIR(_direction + 6), distance);
		}
	}

	if (processEnd) {
		if (player->getStatus() != kActorStatusRestarting && player->getStatus() != kActorStatusGettingHurt) {
			if (sqrt((double)((sum.y - sumPlayer.y) * (sum.y - sumPlayer.y) + (sum.x - sumPlayer.x) * (sum.x - sumPlayer.x))) < 80.0) {
				_frameIndex = 0;
				faceTarget((uint32)getSharedData()->getPlayerIndex(), kDirectionFromActor);
				changeStatus(kActorStatusAttacking);
			}
		}
	}
}

void Actor::TentacleRises() {
	if (!_frameIndex)
		getSound()->playSound(getWorld()->soundResourceIds[6]);

	++_frameIndex;

	if (_frameIndex >= _frameCount) {
		_frameIndex = 0;
		changeStatus(kActorStatusEnabled2);
		getWorld()->tickValueArray[_index] = rnd(4000) + _vm->getTick();
	}

	Actor *actor0 = getScene()->getActor(0);

	getSharedData()->vector1.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector1.y = actor0->getPoint1()->y + actor0->getPoint2()->y - 5;

	getSharedData()->vector2.x = _point1.x + _point2.x;
	getSharedData()->vector2.y = _point1.y + _point2.y;

	TentacleBlocksSarah(getSharedData()->vector1, getSharedData()->vector2);
}

void Actor::updateStatusEnabled2() {
	if (_frameCount == 0)
		error("[Actor::updateStatusEnabled2] Invalid frame count (cannot be 0)");

	_frameIndex = (_frameIndex + 1) % _frameCount;
	_lastScreenUpdate = _vm->screenUpdateCount;

	switch (getWorld()->chapter) {
	default:
		break;

	case kChapter2:
		if (_index == 11)
			changeStatus(kActorStatusWalking2);
		else if (_index > 12)
			CrowStatusQuo();
		break;

	case kChapter11:
		if (_index >= 10 && _index < 16)
			TentacleWigglesForSarah();
		break;
	}
}

void Actor::CrowStatusQuo() {
	if (getSharedData()->crowsData[_index + 61])
		CrowHoveringBeforeKill();
	else
		changeStatus(kActorStatusWalking2);
}

void Actor::CrowHoveringBeforeKill() {
	// Original calls getWalkIncrement but does not seem to do anything with the results

	Actor *player = getScene()->getActor();
	ActorStatus playerStatus = player->getStatus();

	if (playerStatus == kActorStatusRestarting || !getScene()->getActor(10)->isVisible()) {
		changeStatus(kActorStatusEnabled);
		getSharedData()->crowsData[_index - 11] = 160;
	}

	if (playerStatus != kActorStatusGettingHurt) {
		_point1.x = player->getPoint1()->x - (int16)getSharedData()->crowsData[2 * _index + 30];
		_point1.y = player->getPoint1()->y - (int16)getSharedData()->crowsData[2 * _index + 31] + 54;
	}

	if (_frameIndex == _frameCount - 1) {
		_frameIndex = 0;
		if (getSharedData()->crowsData[_index + 29] <= 1 || playerStatus == kActorStatusGettingHurt || playerStatus == kActorStatusRestarting) {
			getSharedData()->crowsData[_index + 29]++;
		} else {
			changeStatus(kActorStatusAttacking);
			_point1.y -= 54;
			getSharedData()->crowsData[_index + 29] = 0;
			getSharedData()->crowsData[_index - 2] += 54;
		}
	}

	if (playerStatus == kActorStatusRestarting && getSharedData()->crowsData[_index + 17] < 100) {
		_point1.y -= 6;
		getSharedData()->crowsData[_index + 29] = 100;
		getSharedData()->crowsData[_index - 2] += 6;
	}

	if (getSharedData()->crowsData[_index + 17] > 99) {
		_point1.y -= 6;
		getSharedData()->crowsData[_index + 17]++;
		getSharedData()->crowsData[_index - 2] += 6;

		if (getSharedData()->crowsData[_index + 17] > 108) {
			getSharedData()->crowsData[_index + 29] = 0;

			changeStatus(kActorStatusEnabled);

			switch (_index) {
			default:
				error("Invalid actor index (was: %d)", _index);
				break;

			case 13:
				forceTo(2300, 671, false);
				break;

			case 14:
				forceTo(2600, 1300, false);
				break;

			case 15:
				forceTo(2742, 615, false);
				break;

			case 16:
				forceTo(2700, 1400, false);
				break;

			case 17:
				forceTo(2751, 347, false);
				break;

			case 18:
				forceTo(2420, 284, false);
				break;

			case 19:
				forceTo(2800, 370, false);
				break;

			case 20:
				forceTo(1973, 1, false);
				break;
			}
		}
	}
}

void Actor::TentacleWigglesForSarah() {
	Actor *actor0 = getScene()->getActor(0);

	getSharedData()->vector1.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector1.y = actor0->getPoint1()->y + actor0->getPoint2()->y - 5;

	getSharedData()->vector2.x = _point1.x + _point2.x;
	getSharedData()->vector2.y = _point1.y + _point2.y;

	if (getWorld()->tickValueArray[_index] == -666)
		getWorld()->tickValueArray[_index] = rnd(4000) + _vm->getTick();

	faceTarget(kActorMax, kDirectionFromActor);
	TentacleBlocksSarah(getSharedData()->vector1, getSharedData()->vector2);

	if (getWorld()->tickValueArray[_index] < (int)_vm->getTick()) {
		if (euclidianDistance(getSharedData()->vector1, getSharedData()->vector2) >= 75) {
			getWorld()->tickValueArray[_index] = rnd(1000) + 2000 + _vm->getTick();
		} else {
			if (actor0->getStatus() == kActorStatusWalking2 || actor0->getStatus() == kActorStatusEnabled2 || actor0->getStatus() == kActorStatusAttacking)
				changeStatus(kActorStatusAttacking);

			getWorld()->tickValueArray[_index] = -666;
		}
	}
}

void Actor::CrowDives() {
	Actor *player = getScene()->getActor();

	Common::Point sum = _point1 + _point2;
	Common::Point sumPlayer = *player->getPoint1() + *player->getPoint2();

	if (getScene()->getActor(10)->getStatus() == kActorStatusRestarting || !getScene()->getActor(10)->isVisible()) {
		changeStatus(kActorStatusEnabled);
		getSharedData()->crowsData[_index - 11] = 160;
	}

	if (_frameIndex == 1)
		getSound()->playSound(getWorld()->soundResourceIds[1], false, Config.sfxVolume - 10);

	if (player->getStatus() == kActorStatusRestarting && _frameIndex < 6)
		changeStatus(kActorStatusEnabled);

	uint32 dist = euclidianDistance(sumPlayer, sum);
	int16 offset = (dist <= 10) ? 7 : 12;
	if (dist > 20) {
		faceTarget((uint32)getSharedData()->getPlayerIndex(), kDirectionFromActor);
		getScene()->getActor(_index + 9)->setDirection(_direction);
	}

	if (_frameIndex < 5 || !getSharedData()->crowsData[_index + 61])
		_frameIndex++;

	if (sumPlayer.x > sum.x)
		_point1.x += offset;
	else if (sumPlayer.x < sum.x)
		_point1.x -= offset;

	if (sumPlayer.y > sum.y)
		_point1.y += offset;
	else if (sumPlayer.y < sum.y)
		_point1.y -= offset;

	if ((int32)dist < (offset + 1)) {
		if (player->getStatus() != kActorStatusGettingHurt && player->getStatus() != kActorStatusRestarting && player->getFrameIndex() < 6) {
			_point1 = sumPlayer - _point2;

			MaxGetsHit();
			getSpeech()->playPlayer(51);
			_vm->setGameFlag(kGameFlag219);

			player->changeDirection(DIR(_direction + 4));
			player->changeStatus(kActorStatusGettingHurt);

			getSharedData()->crowsData[_index + 61] = 0;
		}
	}

	if (_frameIndex > _frameCount - 1) {
		switch (rnd(4)) {
		default:
		case 0:
			sum.y -= 200;
			break;

		case 1:
			sum.y += 200;
			break;

		case 2:
			sum.x -= 200;
			break;

		case 3:
			sum.x += 200;
			break;
		}

		_frameIndex = 0;

		if (getSharedData()->getChapter2Counter(6) <= 2)
			forceTo(sum.x, sum.y, false);
		else
			changeStatus(kActorStatusEnabled);

		getSharedData()->crowsData[_index + 61] = 0;
	}
}

void Actor::MaxGetsHit() {
	Actor *actor39 = getScene()->getActor(39);

	actor39->setFrameIndex(0);
	*actor39->getPoint1() = *getScene()->getActor()->getPoint1();

	if (_vm->isGameFlagSet(kGameFlag169))
		actor39->getPoint1()->y += 80;

	switch (getSharedData()->getChapter2Counter(6)) {
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

	getSharedData()->setChapter2Counter(6, getSharedData()->getChapter2Counter(6) + 1);

	switch (getSharedData()->getChapter2Counter(6)) {
	default:
		crowsReturn(_vm);
		getCursor()->hide();
		break;

	case 0:
		break;

	case 1:
		_vm->setGameFlag(kGameFlag369);
		break;

	case 2:
		_vm->setGameFlag(kGameFlag370);
		break;
	}
}

void Actor::MaxAttacks() {
	if (_index != getSharedData()->getPlayerIndex())
		error("[Actor::MaxAttacks] Function is only available for the current player");

	// Update frame index and process
	_frameIndex++;

	if (_frameIndex == 1)
		getSound()->playSound(getWorld()->soundResourceIds[3], false, Config.sfxVolume - 10);

	ActorIndex actorIndex = getSharedData()->getChapter2ActorIndex();
	if (_frameIndex == 3) {

		if (actorIndex > 12) {

			Actor *otherActor = getScene()->getActor(actorIndex);

			if (otherActor->getStatus() == kActorStatusEnabled2) {
				// FIXME: this is a bit strange, but it looks like the original does exactly that
				// this might be dead code (the actor 38 never exists and thus setting values has no effect)
				Actor *actor38 = getScene()->getActor(38);
				actor38->setFrameIndex(0);
				*actor38->getPoint1() = *otherActor->getPoint1();

				switch (actorIndex) {
				default:
					break;

				case 13:
					_vm->setGameFlag(kGameFlag319);
					_vm->clearGameFlag(kGameFlag235);
					break;

				case 14:
					_vm->setGameFlag(kGameFlag320);
					_vm->clearGameFlag(kGameFlag235);
					break;

				case 15:
					if (getScene()->getActor(16)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag321);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;

				case 16:
					if (getScene()->getActor(15)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag321);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;

				case 17:
					if (getScene()->getActor(21)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag322);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;

				case 18:
					if (getScene()->getActor(19)->checkCrowDeath() && getScene()->getActor(20)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag323);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;

				case 19:
					if (getScene()->getActor(18)->checkCrowDeath() && getScene()->getActor(20)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag323);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;

				case 20:
					if (getScene()->getActor(19)->checkCrowDeath() && getScene()->getActor(18)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag323);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;

				case 21:
					if (getScene()->getActor(17)->checkCrowDeath()) {
						_vm->setGameFlag(kGameFlag322);
						_vm->clearGameFlag(kGameFlag235);
					}
					break;
				}

				otherActor->changeStatus(kActorStatusRestarting);
				getSound()->playSound(getWorld()->soundResourceIds[2], false, Config.sfxVolume - 10);
			}
		}

		if (actorIndex == 11)
			checkScareCrowDeath();
	}

	if (_frameIndex >= _frameCount) {
		_frameIndex = 0;
		changeStatus(kActorStatusEnabled2);
	}
}

void Actor::checkScareCrowDeath() {
	// we are the current player
	Actor *actor11 = getScene()->getActor(11);
	Actor *actor40 = getScene()->getActor(40);

	Common::Point point(_point1.x + _point2.x, _point1.y + _point2.y);
	Common::Point point11(actor11->getPoint1()->x + actor11->getPoint2()->x, actor11->getPoint1()->y + actor11->getPoint2()->y);

	if (actor11->getStatus() == kActorStatusAttacking && euclidianDistance(point, point11) < 100) {
		Actor *actor = getScene()->getActor(getSharedData()->getChapter2ActorIndex());

		actor40->show();
		actor40->setFrameIndex(0);
		actor40->getPoint1()->x = actor->getPoint1()->x;
		actor40->getPoint1()->y = actor->getPoint1()->y;

		if (actor11->getFrameIndex() <= 7) {
			getSound()->playSound(getWorld()->soundResourceIds[9], false, Config.sfxVolume - 10);
		} else if (getSharedData()->getChapter2Counter(5) <= 6) {
			getSound()->playSound(getWorld()->soundResourceIds[9], false, Config.sfxVolume - 10);
		} else {
			getScene()->getActor(11)->changeStatus(kActorStatusRestarting);
			getSound()->playSound(getWorld()->soundResourceIds[10], false, Config.sfxVolume - 10);
		}
	}
}

bool Actor::checkCrowDeath() {
	return (!isVisible() || _status == kActorStatusRestarting);
}

void Actor::ScareCrowAttacks() {
	Actor *player = getScene()->getActor();

	Common::Point sum = _point1 + _point2;
	Common::Point sumPlayer = *player->getPoint1() + *player->getPoint2();

	Common::Rect rect;
	getCrowStrikeZone(&rect, _direction, sum);

	switch (_frameIndex) {
	default:
		break;

	case 1:
		getSound()->playSound(getWorld()->soundResourceIds[4], false, Config.sfxVolume - 10);
		break;

	case 9:
		getSharedData()->setChapter2FrameIndexOffset(1);
		getSharedData()->setChapter2Counter(8, getSharedData()->getChapter2Counter(8) + 1);
		break;

	case 11:
		if (getSharedData()->getChapter2Counter(8) >= 3)
			getSharedData()->setChapter2Counter(8, 0);
		else
			getSharedData()->setChapter2FrameIndexOffset(-1);
		break;
	}

	_frameIndex += getSharedData()->getChapter2FrameIndexOffset();

	Common::Point actionPoint = sum;
	actionPoint.x += pointInRectXAdjust(rect, sumPlayer);
	actionPoint.y += pointInRectYAdjust(rect, sumPlayer);

	if (getScene()->getActor(11)->getFrameIndex() < 8
	 && getScene()->findActionArea(kActionAreaType2, actionPoint) != -1
	 && !actorsIntersect(10, 11))
		 _point1 = actionPoint - _point2;

	if (_frameIndex != 8 || _status == kActorStatusGettingHurt) { /* FIXME the status test seems useless */
		if (_frameIndex > _frameCount - 1) {
			_frameIndex = 0;

			if (!getSharedData()->getFlag(kFlagActorUpdateStatus15Check)) {
				changeStatus(kActorStatusWalking2);
			} else {
				getSharedData()->setFlag(kFlagActorUpdateStatus15Check, false);
				getScene()->getActor(11)->changeStatus(kActorStatus18);
			}
		}
	} else {
		if (rect.contains(sumPlayer)) {

			_vm->clearGameFlag(kGameFlag263);
			_vm->clearGameFlag(kGameFlag264);
			_vm->clearGameFlag(kGameFlag265);
			_vm->clearGameFlag(kGameFlag266);
			_vm->clearGameFlag(kGameFlag267);
			_vm->clearGameFlag(kGameFlag268);
			_vm->clearGameFlag(kGameFlag269);

			player->stopWalking();
			player->changeStatus(kActorStatusGettingHurt);
			MaxGetsHit();

			getSpeech()->playPlayer(52);

			_vm->setGameFlag(kGameFlag219);

		} else {
			if ((abs(sum.y - sumPlayer.y) + abs(sum.x - sumPlayer.x)) < 100) {
				getSharedData()->setChapter2Counter(7, 5);
				getSharedData()->setFlag(kFlagActorUpdateStatus15Check, true);
			}
		}
	}
}

bool Actor::actorsIntersect(ActorIndex actorIndex1, ActorIndex actorIndex2) {
	Actor *actor1 = getScene()->getActor(actorIndex1);
	Actor *actor2 = getScene()->getActor(actorIndex2);

	if (actor1->getField944())
		return false;

	if (actor2->getField944())
		return false;

	int16 actor2_x = actor2->getPoint1()->x + actor2->getPoint2()->x;
	int16 actor2_y = actor2->getPoint1()->y + actor2->getPoint2()->y;

	Common::Point pt1((int16)(actor2_x -    (actor1->getField948() + 10)), (int16)(actor2_y -    (actor1->getField94C() + 10)));
	Common::Point pt2((int16)(actor2_x +     actor1->getField948() + 10),  (int16)(actor2_y +     actor1->getField94C() + 10));
	Common::Point pt3((int16)(actor2_x -    (actor2->getField948() + 25)), (int16)(actor2_y -    (actor2->getField94C() + 20)));
	Common::Point pt4((int16)(actor2_x + 2 * actor2->getField948() + 25),  (int16)(actor2_y + 2 * actor2->getField94C() + 20));

	return getScene()->rectIntersect(pt1.x, pt1.y, pt2.x, pt2.y, pt3.x, pt3.y, pt4.x, pt4.y);
}

void Actor::TentacleWhips() {
	Actor *actor0 = getScene()->getActor(0);

	// Update vectors
	getSharedData()->vector1.x = actor0->getPoint1()->x + actor0->getPoint2()->x;
	getSharedData()->vector1.y = actor0->getPoint1()->y + actor0->getPoint2()->y - 5;

	getSharedData()->vector2.x = getPoint1()->x + getPoint2()->x;
	getSharedData()->vector2.y = getPoint1()->y + getPoint2()->y;

	TentacleBlocksSarah(getSharedData()->vector1, getSharedData()->vector2);

	++_frameIndex;
	if (_frameIndex >= _frameCount)
		changeStatus(kActorStatusEnabled2);

	if (_frameIndex == 14) {
		if (Actor::euclidianDistance(getSharedData()->vector1, getSharedData()->vector2) < 75) {

			actor0->changeStatus(kActorStatusGettingHurt);
			++getWorld()->field_E848C;

			getSound()->stop(getWorld()->soundResourceIds[3]);
			getSound()->stop(getWorld()->soundResourceIds[4]);
			getSound()->stop(getWorld()->soundResourceIds[5]);

			getSpeech()->playPlayer(131);
		}
	}
}

void Actor::SarahAttacks() {
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
			 && (actor2->getStatus() == kActorStatusEnabled2 || actor2->getStatus() == kActorStatusAttacking)) {
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

				actor2->changeStatus(kActorStatusRestarting);
			}
		}
	}

	if (_frameIndex >= _frameCount) {
		getCursor()->show();
		getSharedData()->setFlag(kFlag1, false);
		_frameIndex = 0;
		changeStatus(kActorStatusEnabled2);
	}
}

void Actor::MaxGetsSome() {
	Actor *player = getScene()->getActor();

	player->setFrameIndex(player->getFrameIndex() + 1);

	if (player->getFrameIndex() > (player->getFrameCount() - 1)) {
		if (getSharedData()->getChapter2Counter(6) <= 2) {
			player->setFrameIndex(0);
			player->changeStatus(kActorStatusEnabled2);
		} else {
			_vm->clearGameFlag(kGameFlag438);
			_vm->clearGameFlag(kGameFlag439);
			_vm->clearGameFlag(kGameFlag440);
			_vm->clearGameFlag(kGameFlag441);
			_vm->clearGameFlag(kGameFlag442);

			getSpeech()->playPlayer(53);

			_vm->setGameFlag(kGameFlag219);

			player->setFrameIndex(0);
			player->changeStatus(kActorStatusRestarting);

			_vm->clearGameFlag(kGameFlag369);
			_vm->clearGameFlag(kGameFlag370);

			if (getSound()->isPlaying(getWorld()->soundResourceIds[5]))
				getSound()->stop(getWorld()->soundResourceIds[5]);

			if (getSound()->isPlaying(getWorld()->soundResourceIds[6]))
				getSound()->stop(getWorld()->soundResourceIds[6]);

			if (getSound()->isPlaying(getWorld()->soundResourceIds[7]))
				getSound()->stop(getWorld()->soundResourceIds[7]);

			if (_vm->isGameFlagSet(kGameFlag235)) {
				Actor::crowsReturn(_vm);
				_vm->clearGameFlag(kGameFlag235);
			}
		}
	}
}

void Actor::SarahGetsSome() {
	// We are sure to be the current player
	getCursor()->show();
	getSharedData()->setFlag(kFlag1, false);

	if (_frameIndex != 5 || _vm->isGameFlagNotSet(kGameFlag570))
		++_frameIndex;

	if (_frameIndex > _frameCount - 1) {
		if (getWorld()->field_E848C >= 3) {
			_frameIndex = 0;

			getScene()->getActor(0)->changeStatus(kActorStatusRestarting);
			getScene()->getActor(1)->setTickCount(_vm->getTick() + 2000);
		} else {
			getScene()->getActor(0)->changeStatus(kActorStatusEnabled2);
		}
	}
}

void Actor::TentacleDies() {
	++_frameIndex;

	if (_frameIndex >= _frameCount) {
		_frameIndex = 0;
		changeStatus(kActorStatusEnabled2);
		hide();

		if (_vm->getRandomBit() == 1) {
			_vm->setGameFlag(kGameFlag219);
			getSpeech()->playPlayer(133);
		}
	}
}

void Actor::CrowSwoops() {
	Actor *player = getScene()->getActor();

	_point1.x = player->getPoint1()->x - (int16)getSharedData()->crowsData[2 * _index + 30];
	_point1.y = player->getPoint1()->y - (int16)getSharedData()->crowsData[2 * _index + 31];

	_frameIndex++;

	if (_frameIndex > _frameCount - 1) {
		getSharedData()->crowsData[_index + 61] = 1;
		changeStatus(kActorStatusEnabled2);

		_point1.y += 54;
		getSound()->playSound(getWorld()->soundResourceIds[1], false, Config.sfxVolume - 10);

		getSharedData()->crowsData[_index - 2] -= 54;
	}
}

void Actor::ScareCrowRetreats() {
	int32 frameIndex = (int32)_frameIndex;
	uint32 distance = (uint32)abs((double)getWalkIncrement(_direction, (_frameIndex < _frameCount) ? _frameIndex : 2 * _frameCount - (_frameIndex + 1)));

	getSharedData()->setChapter2Counter(7, getSharedData()->getChapter2Counter(7) + 1);
	if (getSharedData()->getChapter2Counter(7) > 14) {
		getSharedData()->setChapter2Counter(7, 0);
		changeStatus(kActorStatusWalking2);
	}

	faceTarget((uint32)getSharedData()->getPlayerIndex(), kDirectionFromActor);

	Common::Point sum = _point1 + _point2;
	if (canMove(&sum, DIR(_direction + 4), distance, false)) {
		move(DIR(_direction + 4), distance);
		--frameIndex;
	} else if (canMove(&sum, DIR(_direction + 5), distance, false)) {
		move(DIR(_direction + 5), distance);
		--frameIndex;
	} else if (canMove(&sum, DIR(_direction + 3), distance, false)) {
		move(DIR(_direction + 3), distance);
		--frameIndex;
	} else if (canMove(&sum, DIR(_direction + 6), distance, false)) {
		move(DIR(_direction + 6), distance);
		--frameIndex;
	} else if (canMove(&sum, DIR(_direction + 2), distance, false)) {
		move(DIR(_direction + 2), distance);
		--frameIndex;
	}

	if (frameIndex < 0)
		_frameIndex = _frameCount - 1;
	else
		_frameIndex = (uint32)frameIndex;
}

void Actor::updateStatusMorphing() {
	if (_resourceId == getWorld()->graphicResourceIds[3] || _resourceId == getWorld()->graphicResourceIds[4] || _resourceId == getWorld()->graphicResourceIds[5]) {
		if (_frameIndex < _frameCount - 1) {
			++_frameIndex;

			if (_frameIndex == _frameCount / 2) {
				getWorld()->currentPaletteId = getWorld()->graphicResourceIds[getWorld()->nextPlayer - 1];
				getScreen()->setPalette(getWorld()->currentPaletteId);
				getScreen()->setGammaLevel(getWorld()->currentPaletteId);
			}

			return;
		}
	} else {
		if (_frameIndex > 0) {
			--_frameIndex;

			if (_frameIndex == _frameCount / 2)
				getScreen()->setPalette(getWorld()->graphicResourceIds[getWorld()->nextPlayer - 1]);

			getWorld()->currentPaletteId = getWorld()->graphicResourceIds[getWorld()->nextPlayer - 1];
			getScreen()->setGammaLevel(getWorld()->currentPaletteId);
			return;
		}
	}

	getScene()->changePlayer(getWorld()->nextPlayer);
	changeStatus(kActorStatusEnabled);
	getWorld()->nextPlayer = kActorInvalid;
}

void Actor::actionAreaCheck() {
	if (_field_944 == 4 || !isVisible())
		return;

	int32 areaIndex = getScene()->findActionArea(kActionAreaType1, Common::Point((int16)(_point1.x + _point2.x), (int16)(_point1.y + _point2.y)));
	if (areaIndex == _actionIdx3 || areaIndex == -1)
		return;

	ActionArea *area = getWorld()->actions[areaIndex];
	ActionArea *actorArea = getWorld()->actions[_actionIdx3];

	if (!(area->flags & 1))
		return;

	if (!getSharedData()->getFlag(kFlagSkipScriptProcessing)) {
		debugC(kDebugLevelScripts, "[Script] Entered ActionArea (idx: %d, name: %s)", areaIndex, area->name);
		debugC(kDebugLevelScripts, "[Script] Queuing Script #1 (idx: %d) for Actor (idx: %d)", actorArea->scriptIndex2, _index);
		getScript()->queueScript(actorArea->scriptIndex2, _index);
		debugC(kDebugLevelScripts, "[Script] Queuing Script #2 (idx: %d) for Actor (idx: %d)", area->scriptIndex, _index);
		getScript()->queueScript(area->scriptIndex, _index);
	}

	if (!area->paletteResourceId || area->paletteResourceId == actorArea->paletteResourceId || _index) {
		if (area->paletteResourceId != actorArea->paletteResourceId && !_index)
			_vm->screen()->queuePaletteFade(getWorld()->currentPaletteId, 100, 3);

		_actionIdx3 = areaIndex;
	} else {
		_vm->screen()->queuePaletteFade(area->paletteResourceId, 50, 3);
		_actionIdx3 = areaIndex;
	}
}

void Actor::TentacleBlocksSarah(const Common::Point &vec1, Common::Point vec2) {
	if (getScene()->getActor(1)->isVisible())
		return;

	uint32 diffY = (uint32)abs(vec2.y - vec1.y);
	if (diffY > 5)
		diffY = 5;

	if (diffY == 0)
		return;

	ActorDirection dir = (vec1.y > vec2.y) ? kDirectionS : kDirectionN;

	if (canMove(&vec2, dir, diffY + 3, false))
		incPosition(dir, (int16)(diffY - 1), &_point1);
}

void Actor::SarahDies() {
	getCursor()->hide();
	getScene()->getActor(0)->hide();
	getScene()->getActor(1)->setFrameIndex(0);

	getWorld()->tickCount1 = _vm->getTick() + 3000;
}

void Actor::updateNumbers(uint item, const Common::Point &point) {
	if (item != 1)
		return;

	_numberPoint.x = point.x;
	_numberPoint.y = point.y + 8;
	_numberStringWidth = 40;
	snprintf(_numberString01, sizeof(_numberString01), "%d", _numberValue01);

	_numberFlag01 = 1;
}

//////////////////////////////////////////////////////////////////////////
// Path finding functions
//////////////////////////////////////////////////////////////////////////
bool Actor::findLeftPath(Common::Point source, const Common::Point &destination, Common::Array<int> *actions) {
	// Reset pathfinding data
	_data.count = 0;
	_data.current = 0;

	bool flag = false;
	Common::Point src = source;

	for (uint32 i = 0; i < 60; i++) {

		// Note: this is handled differently from other tryDirection functions
		// as we break instead of checking the other actors
		if (!tryDirection(source, actions, &src, kDirectionE,  destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionNE, destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionSE, destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionN,  destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionS,  destination, &flag))
			break;

		// Update source point after all processing
		source = src;

		if (flag)
			return true;
	}

	return false;
}

bool Actor::findRightPath(Common::Point source, const Common::Point &destination, Common::Array<int> *actions) {
	// Reset pathfinding data
	_data.count = 0;
	_data.current = 0;

	bool flag = false;
	Common::Point src = source;

	for (uint32 i = 0; i < 60; i++) {
		if (!tryDirection(source, actions, &src, kDirectionW,  destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionNW, destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionSW, destination, &flag)) {
			if (src.y <= destination.y) {
				if (!tryDirection(source, actions, &src, kDirectionS,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionN,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionSE, destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionE,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionNE, destination, &flag))
					continue;
			} else {
				if (!tryDirection(source, actions, &src, kDirectionN,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionS,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionNE, destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionE,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionSE, destination, &flag))
					continue;
			}
		}

		// Update source point after all processing
		source = src;

		if (flag)
			return true;
	}

	return false;
}

bool Actor::findUpPath(Common::Point source, const Common::Point &destination, Common::Array<int> *actions) {
	// Reset pathfinding data
	_data.count = 0;
	_data.current = 0;

	bool flag = false;
	Common::Point src = source;

	for (uint32 i = 0; i < 60; i++) {
		if (!tryDirection(source, actions, &src, kDirectionN, destination, &flag)) {
			if (src.x >= destination.x) {
				if (!tryDirection(source, actions, &src, kDirectionNW, destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionW,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionNE, destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionE,  destination, &flag))
					continue;
			} else {
				if (!tryDirection(source, actions, &src, kDirectionNE, destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionE,  destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionNW, destination, &flag)
				 && !tryDirection(source, actions, &src, kDirectionW,  destination, &flag))
					continue;
			}
		}

		// Update source point after all processing
		source = src;

		if (flag)
			return true;
	}

	return false;
}

bool Actor::findDownPath(Common::Point source, const Common::Point &destination, Common::Array<int> *actions) {
	// Reset pathfinding data
	_data.count = 0;
	_data.current = 0;

	bool flag = false;
	Common::Point src = source;

	for (uint32 i = 0; i < 60; i++) {
		if (!tryDirection(source, actions, &src, kDirectionS,  destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionSE, destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionSW, destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionE,  destination, &flag)
		 && !tryDirection(source, actions, &src, kDirectionW,  destination, &flag))
			continue;

		// Update source point after all processing
		source = src;

		if (flag)
			return true;
	}

	return false;
}

bool Actor::tryDirection(const Common::Point &source, Common::Array<int> *actions, Common::Point *point, ActorDirection direction, const Common::Point &destination, bool *flag) {
	Common::Point sign;
	Common::Point src = source;
	uint32 frameNumber = _frameNumber;

	switch (direction) {
	default:
		return false;

	case kDirectionN:
		sign.y = -1;
		break;

	case kDirectionNW:
		sign.x = -1;
		sign.y = -1;
		break;

	case kDirectionW:
		sign.x = -1;
		break;

	case kDirectionSW:
		sign.x = -1;
		sign.y = 1;
		break;

	case kDirectionS:
		sign.y = 1;
		break;

	case kDirectionSE:
		sign.x = 1;
		sign.y = 1;
		break;

	case kDirectionE:
		sign.x = 1;
		break;

	case kDirectionNE:
		sign.x = 1;
		sign.y = -1;
		break;
	}

	for (int i = 0; i < 10; i++) {
		if (!testPolyInLink(src, actions))
			break;

		int32 dist = getStride(direction, frameNumber);
		src.x += (int16)(sign.x * dist);
		src.y += (int16)(sign.y * dist);

		if (abs(src.x - destination.x) >= getStride(kDirectionW, frameNumber)) {
			if (abs(src.y - destination.y) < getStride(kDirectionN, frameNumber)) {

				if (src.x >= destination.x) {
					if (canGetToDest(actions, src, kDirectionW, src.x - destination.x)) {
						*flag = true;
						*point = src;

						_data.points[_data.count]     = src;
						_data.directions[_data.count] = direction;
						_data.count++;

						_data.points[_data.count]     = destination;
						_data.directions[_data.count] = kDirectionW;
						_data.count++;

						return true;
					}
				} else {
					if (canGetToDest(actions, src, kDirectionE, destination.x - src.x)) {
						*flag = true;
						*point = src;

						_data.points[_data.count]     = src;
						_data.directions[_data.count] = direction;
						_data.count++;

						_data.points[_data.count]     = destination;
						_data.directions[_data.count] = kDirectionE;
						_data.count++;

						return true;
					}
				}
			}
		} else {
			if (src.y >= destination.y) {
				if (canGetToDest(actions, src, kDirectionN, src.y - destination.y)) {
					*flag = true;
					*point = src;

					_data.points[_data.count]     = src;
					_data.directions[_data.count] = direction;
					_data.count++;

					_data.points[_data.count]     = destination;
					_data.directions[_data.count] = kDirectionN;
					_data.count++;

					return true;
				}
			} else {
				if (canGetToDest(actions, src, kDirectionS, destination.y - src.y)) {
					*flag = true;
					*point = src;

					_data.points[_data.count]     = src;
					_data.directions[_data.count] = direction;
					_data.count++;

					_data.points[_data.count]     = destination;
					_data.directions[_data.count] = kDirectionS;
					_data.count++;

					return true;
				}
			}
		}

		// Proceed to next frame
		++frameNumber;

		if (frameNumber >= _frameCount)
			frameNumber = 0;
	}

	// Check if we need to process
	int32 distance = getStride(direction, frameNumber);
	if (source.x == (src.x - sign.x * distance) && source.y == (src.y - sign.y * distance))
		return false;

	if (!getWorld()->sceneRects[getWorld()->sceneRectIdx].contains(src))
		return false;

	// Update frame and setup pathfinding
	_frameNumber = frameNumber;

	if (_frameNumber == 0)
		distance = getStride(direction, _frameCount - 1);
	else
		distance = getStride(direction, _frameNumber - 1);

	src.x -= (int16)(sign.x * distance);
	src.y -= (int16)(sign.y * distance);

	*point = src;
	_data.points[_data.count]     = src;
	_data.directions[_data.count] = direction;
	_data.count++;

	return true;
}

bool Actor::canGetToDest(Common::Array<int> *actions, const Common::Point &point, ActorDirection direction, int16 loopcount) {
	if (loopcount <= 1)
		return true;

	// Initialize base coordinates
	Common::Point delta     = Common::Point(deltaPointsArray[direction][0], deltaPointsArray[direction][1]);
	Common::Point basePoint = delta + point;
	Common::Rect  rect      = getWorld()->sceneRects[getWorld()->sceneRectIdx];

	for (int16 i = 1; i < loopcount; i++) {
		if (!testPolyInLink(basePoint, actions))
			return false;

		if (!rect.contains(basePoint))
			return false;

		basePoint += delta;
	}

	return true;
}

bool Actor::testPolyInLink(const Common::Point &pt, Common::Array<int> *actions) {
	if (actions->size() == 0)
		return false;

	for (Common::Array<int>::iterator it = actions->begin(); it != actions->end(); it++) {
		if (isInActionArea(pt, getWorld()->actions[*it]))
			return true;
	}

	return false;

}

bool Actor::isInActionArea(const Common::Point &pt, ActionArea *area) {
	Common::Rect sceneRect = getWorld()->sceneRects[getWorld()->sceneRectIdx];

	if (!sceneRect.contains(pt))
		return false;

	if (!(area->flags & 1))
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

	Polygon poly = getScene()->polygons()->get(area->polygonIndex);
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
	Common::Rect scene(getWorld()->xLeft, getWorld()->yTop, getWorld()->xLeft + 640, getWorld()->yTop + 480);
	Common::Rect actor(_boundingRect);
	actor.translate(_point1.x, _point1.y);

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

ActorDirection Actor::getAngle(const Common::Point &vec1, const Common::Point &vec2) {
	uint32 diffX, diffY;
	byte adjust = 0;

	if (vec1.x > vec2.x) {
		adjust |= 2;
		diffX = (vec1.x - vec2.x) << 16;
	} else {
		diffX = (vec2.x - vec1.x) << 16;
	}

	if (vec2.y > vec1.y) {
		adjust |= 1;
		diffY = (vec2.y - vec1.y) << 16;
	} else {
		diffY = (vec1.y - vec2.y) << 16;
	}

	// dirAngle will be initially calculated as a value between 0 and 90, ie. in [0, 90]
	int32 dirAngle = -1;

	if (diffX) {
		uint32 index = (uint32)((diffY * 256) / diffX);

		if (index < 256)
			dirAngle = angleTable01[index];
		else if (index < 4096)
			dirAngle = angleTable02[index / 16];
		else if (index < 65536)
			dirAngle = angleTable03[index / 256];
		else
			dirAngle = 90;
	} else {
		dirAngle = 90;
	}

	// The adjust variable value is used to properly calculate the dirAngle within the [0, 360] range
	// value: 0 is first quadrant, 2 is second quadrant, 3 is third quadrant, 1 is fourth quadrant
	switch (adjust) {
	default:
		// first quadrant
		break;

	case 1:
		// fourth quadrant
		dirAngle = 360 - dirAngle;
		break;

	case 2:
		// second quadrant
		dirAngle = 180 - dirAngle;
		break;

	case 3:
		// third quadrant
		dirAngle += 180;
		break;
	}

	if (dirAngle >= 360)
		dirAngle -= 360;

	ActorDirection dir;

	if (dirAngle < 22 || dirAngle >= 337) {
		dir = kDirectionE;
	} else if (dirAngle < 67) {
		dir = kDirectionNE;
	} else if (dirAngle < 112) {
		dir = kDirectionN;
	} else if (dirAngle < 157) {
		dir = kDirectionNW;
	} else if (dirAngle < 202) {
		dir = kDirectionW;
	} else if (dirAngle < 247) {
		dir = kDirectionSW;
	} else if (dirAngle < 292) {
		dir = kDirectionS;
	} else {
		dir = kDirectionSE;
	}

	return dir;
}

void Actor::updateGraphicData(uint32 offset) {
	int32 index = ((_direction > kDirectionS) ? kDirection8 - _direction : _direction) + (int32)offset;
	_resourceId = _graphicResourceIds[index];
	_frameCount = GraphicResource::getFrameCount(_vm, _resourceId);
	_frameIndex = 0;
}

bool Actor::canChangeStatus(int index) const {
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

int32 Actor::getStride(ActorDirection dir, uint32 frameIndex) const {
	// WORKAROUND: It seems that the original allows frameIndex to be out of range
	uint32 index = MIN<uint32>(frameIndex, 19);
	if (frameIndex >= ARRAYSIZE(_distancesNS))
		debugC(kDebugLevelMain, "[Actor::getStride] Invalid frame index %d for actor '%s' with direction %d", frameIndex, _name, dir);

	switch (dir) {
	default:
		error("[Actor::getStride] Invalid direction");

	case kDirectionN:
	case kDirectionS:
		return _distancesNS[index];

	case kDirectionNW:
	case kDirectionSW:
	case kDirectionSE:
	case kDirectionNE:
		return _distancesNSEO[index];

	case kDirectionW:
	case kDirectionE:
		return _distancesEO[index];
	}
}

int32 Actor::getWalkIncrement(ActorDirection dir, uint32 frameIndex) const {
	if (frameIndex >= ARRAYSIZE(_distancesNS))
		error("[Actor::getWalkIncrement] Invalid frame index (was: %d, max: %d)", _frameIndex, ARRAYSIZE(_distancesNS) - 1);

	switch (dir) {
	default:
		error("[Actor::getWalkIncrement] Invalid direction %d", dir);

	case kDirectionN:
		return -_distancesNS[frameIndex];

	case kDirectionS:
		return _distancesNS[frameIndex];

	case kDirectionNW:
		return -_distancesNSEO[frameIndex];

	case kDirectionNE:
		return -_distancesNSEO[frameIndex];

	case kDirectionSW:
		return _distancesNSEO[frameIndex];

	case kDirectionSE:
		return _distancesNSEO[frameIndex];

	case kDirectionW:
		return -_distancesEO[frameIndex];

	case kDirectionE:
		return _distancesEO[frameIndex];
	}
}

void Actor::incPosition(ActorDirection direction, int16 delta, Common::Point *point) {
	if (!point)
		error("[Actor::incPosition] Invalid point (NULL)!");

	switch (direction) {
	default:
		break;

	case kDirectionN:
		point->y -= delta;
		break;

	case kDirectionNW:
		point->x -= delta;
		point->y -= delta;
		break;

	case kDirectionW:
		point->x -= delta;
		break;

	case kDirectionSW:
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
		point->x += delta;
		point->y -= delta;
		break;
	}
}

uint32 Actor::euclidianDistance(const Common::Point &point1, const Common::Point &point2) {
	return (uint32)sqrt(pow((double)(point2.y - point1.y), 2) + pow((double)(point2.x - point1.x), 2));
}

int32 Actor::getAngleOfVector(const Common::Point &vec1, const Common::Point &vec2) {
	int32 result = (int32)(((long)(180 - acos((double)(vec2.y - vec1.y) / euclidianDistance(vec1, vec2)) * 180 / -M_PI)) % 360);

	if (vec1.x < vec2.x)
		return 360 - result;

	return result;
}

void Actor::getCrowStrikeZone(Common::Rect *rect, ActorDirection direction, const Common::Point &point) {
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
		rect->top = point.y - 84;
		rect->left = point.x - 9;
		break;

	case kDirectionNW:
		rect->top = point.y - 55;
		rect->left = point.x - 84;
		break;

	case kDirectionW:
		rect->top = point.y - 34;
		rect->left = point.x - 93;
		break;

	case kDirectionSW:
		rect->top = point.y + 27;
		rect->left = point.x - 94;
		break;

	case kDirectionS:
		rect->top = point.y + 41;
		rect->left = point.x - 9;
		break;

	case kDirectionSE:
		rect->top = point.y + 27;
		rect->left = point.x + 54;
		break;

	case kDirectionE:
		rect->top = point.y - 34;
		rect->left = point.x + 53;
		break;

	case kDirectionNE:
		rect->top = point.y - 55;
		rect->left = point.x + 44;
		break;
	}

	rect->setWidth(40);
	rect->setHeight(40);
}

bool Actor::determineLeftOrRight(const Common::Point &vec1, const Common::Point &vec2) {
	Common::Point vec3(2289, 171);

	int32 diff = getAngleOfVector(vec1, vec3) - getAngleOfVector(vec1, vec2);

	if (diff < 0)
		diff += 359;

	return (diff > 180);
}

int16 Actor::pointInRectXAdjust(const Common::Rect &rect, const Common::Point &point) {
	if (point.x > rect.right)
		return 3;

	if (point.x < rect.left)
		return -3;
	else
		return 0;
}

int16 Actor::pointInRectYAdjust(const Common::Rect &rect, const Common::Point &point) {
	if (point.y > rect.bottom)
		return 3;

	if (point.y < rect.top)
		return -3;
	else
		return 0;
}


} // end of namespace Asylum
