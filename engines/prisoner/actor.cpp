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

#include "prisoner/prisoner.h"
#include "prisoner/path.h"
#include "prisoner/resource.h"

namespace Prisoner {

int16 PrisonerEngine::addActor(Common::String &pakName, int16 pakSlot, int16 frameListIndex, int16 nodeIndex,
	int16 x, int16 y) {

	int16 actorIndex = _actors.getFreeSlot();
	Actor *actor = &_actors[actorIndex];
	ActorSprite tempActorSprite;

	actor->resourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 4);
	actor->pathResultIndex = 0;
	actor->pathResultCount = 0;
	actor->pathWalker1 = NULL;
	actor->pathWalker2 = NULL;
	actor->status = 1;
	actor->actorSprite = &_actorSprites[actorIndex];

	tempActorSprite.animationResource = _res->get<AnimationResource>(actor->resourceCacheSlot);
	tempActorSprite.frameIndex = 0;

	if (nodeIndex != -1) {
		PathNode *node = _pathSystem->getNode(nodeIndex);
		tempActorSprite.x = node->x;
		tempActorSprite.y = node->y;
		tempActorSprite.scale = node->scale;
	} else {
		tempActorSprite.x = x;
		tempActorSprite.y = y;
		tempActorSprite.scale = 100;
	}

	assignActorSprite(actorIndex, tempActorSprite);

	setActorSpriteFrameListIndex(actor->actorSprite, frameListIndex, true);

	actor->pathNodeIndex = nodeIndex;
	actor->pathEdgeIndex = -1;
	actor->pathPolyIndex = -1;
	actor->x = actor->actorSprite->x;
	actor->y = actor->actorSprite->y;
	actor->x2 = actor->actorSprite->x;
	actor->y2 = actor->actorSprite->y - 40;

	actor->textFontNumber = _textFont;
	getFontColors(_textFont, actor->fontInkColor, actor->fontOutlineColor);

	return actorIndex;
}

void PrisonerEngine::clearActor(int16 actorIndex) {
	Actor *actor = &_actors[actorIndex];
	actor->actorSprite->used = 0;
	buildActorSpriteDrawQueue();
	clearActorFrameSoundsByActorIndex(actorIndex);
	_res->unload(actor->resourceCacheSlot);
	actor->resourceCacheSlot = -1;
	clearActorZone(actorIndex);
	if (actor->pathWalker2) {
		resetPathWalker(actor->pathWalkerIndex);
		actor->pathWalker2 = NULL;
	}
	actor->pathWalkerIndex = -1;
	if (actorIndex == _mainActorIndex) {
		_mainActorIndex = -1;
		_mainActorValid = false;
	}
}

void PrisonerEngine::clearActors() {
	_cameraDeltaX = 0;
	_cameraDeltaY = 0;
	_mainActorIndex = -1;
	_mainActorValid = false;
	_actorsCleared = true;

	for (int16 actorIndex = 0; actorIndex < kMaxActors; actorIndex++) {
		Actor *actor = &_actors[actorIndex];
		actor->resourceCacheSlot = -1;
		actor->altAnimationIndex = -1;
		actor->pathWalkerIndex = -1; //???
	}

	for (int16 altActorAnimationIndex = 0; altActorAnimationIndex < kMaxAltActorAnimations; altActorAnimationIndex++) {
		AltActorAnimation *altActorAnimation = &_altActorAnimations[altActorAnimationIndex];
		altActorAnimation->resourceCacheSlot = -1;
	}

	_pathWalkers.clear();

}

void PrisonerEngine::unloadActors() {
	clearActorSprites();
	if (_actorsCleared) {
		for (int16 actorIndex = 0; actorIndex < kMaxActors; actorIndex++) {
			Actor *actor = &_actors[actorIndex];
			if (actor->resourceCacheSlot != -1) {
				clearActorFrameSoundsByActorIndex(actorIndex);
				_res->unload(actor->resourceCacheSlot);
				actor->resourceCacheSlot = -1;
				if (actor->pathWalker2) {
					resetPathWalker(actor->pathWalkerIndex);
					actor->pathWalker2 = NULL;
				}
				actor->pathWalkerIndex = -1;
			}
		}
		for (int16 altActorAnimationIndex = 0; altActorAnimationIndex < kMaxAltActorAnimations; altActorAnimationIndex++) {
			AltActorAnimation *altActorAnimation = &_altActorAnimations[altActorAnimationIndex];
			if (altActorAnimation->resourceCacheSlot != -1) {
				_res->unload(altActorAnimation->resourceCacheSlot);
				altActorAnimation->resourceCacheSlot = -1;
			}
		}
	}
}

void PrisonerEngine::restoreActorSprites() {
	if (_actorsCleared) {
		for (int16 actorIndex = 0; actorIndex < kMaxActors; actorIndex++) {
			Actor *actor = &_actors[actorIndex];
			if (actor->resourceCacheSlot != -1) {
				ActorSprite *actorSprite = actor->actorSprite;
				actorSprite->animationResource = _res->get<AnimationResource>(actor->resourceCacheSlot);
				actorSprite->frameList = actorSprite->animationResource->_anims[actorSprite->frameListIndex];
			}
		}
	}
}

void PrisonerEngine::setActorFontColors(int16 actorIndex, int16 outlineColor, int16 inkColor) {
	_actors[actorIndex].fontOutlineColor = outlineColor;
	_actors[actorIndex].fontInkColor = inkColor;
}

void PrisonerEngine::setActorAnimation(int16 actorIndex, Common::String &pakName, int16 pakSlot,
	int16 frameListIndex, int16 nodeIndex) {

	Actor *actor = &_actors[actorIndex];
	ActorSprite actorSprite;

	_res->unload(actor->resourceCacheSlot);

	actor->resourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 4);
	actorSprite = *actor->actorSprite;
	actorSprite.animationResource = _res->get<AnimationResource>(actor->resourceCacheSlot);
	actorSprite.frameIndex = 0;

	if (nodeIndex != -1) {
		PathNode *node = _pathSystem->getNode(nodeIndex);
		actorSprite.x = node->x;
		actorSprite.y = node->y;
		actor->pathNodeIndex = nodeIndex;
		actor->pathEdgeIndex = -1;
		actor->pathPolyIndex = -1;
	}

	actor->x = actorSprite.x;
	actor->y = actorSprite.y;

	assignActorSprite(actorIndex, actorSprite);
	setActorSpriteFrameListIndex(actor->actorSprite, frameListIndex, true);

	actor->status = 1;
	actor->pathResultCount = 0;

	if (actorIndex == _mainActorIndex) {
		_queuedZoneAction.used = 0;
		_queuedZoneAction.zoneActionIndex = -1;
		_queuedZoneAction.pathNodeIndex = -1;
	}

}

void PrisonerEngine::setActorAnimationAtPos(int16 actorIndex, Common::String &pakName, int16 pakSlot,
	int16 frameListIndex, int16 x, int16 y) {

	Actor *actor = &_actors[actorIndex];
	ActorSprite actorSprite;

	_res->unload(actor->resourceCacheSlot);

	actor->resourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 4);
	actorSprite = *actor->actorSprite;
	actorSprite.animationResource = _res->get<AnimationResource>(actor->resourceCacheSlot);
	actorSprite.frameIndex = 0;
	actorSprite.x = x;
	actorSprite.y = y;

	actor->pathNodeIndex = -1;
	actor->pathEdgeIndex = -1;
	actor->pathPolyIndex = -1;
	actor->x = actorSprite.x;
	actor->y = actorSprite.y;

	assignActorSprite(actorIndex, actorSprite);
	setActorSpriteFrameListIndex(actor->actorSprite, frameListIndex, true);

	actor->status = 1;
	actor->pathResultCount = 0;

	if (actorIndex == _mainActorIndex) {
		_queuedZoneAction.used = 0;
		_queuedZoneAction.zoneActionIndex = -1;
		_queuedZoneAction.pathNodeIndex = -1;
	}

}

void PrisonerEngine::setMainActor(int16 actorIndex) {
	_mainActorIndex = actorIndex;
	_mainActorValid = true;
	actorAssignPathWalker(actorIndex);
	if (_cameraFollowsActorIndex == -1) {
		_cameraFollowsActorIndex = actorIndex;
		_cameraFocusActor = true;
	}
}

void PrisonerEngine::actorAssignPathWalker(int16 actorIndex) {
	Actor *actor = &_actors[actorIndex];
	actor->pathWalkerIndex = _pathWalkers.getFreeSlot();
	_pathWalkers[actor->pathWalkerIndex].used = 1;
	actor->pathWalker1 = &_pathWalkers[actor->pathWalkerIndex].items[0];
	actor->pathWalker2 = &_pathWalkers[actor->pathWalkerIndex].items[0];
}

void PrisonerEngine::actorWalkToPoint(int16 actorIndex, int16 x, int16 y) {
	Actor *actor = &_actors[actorIndex];
	int16 direction = -1;

	if (!_mainActorValid && actorIndex == _mainActorIndex)
		return;

	actor->walkDestX = x;
	actor->walkDestY = y;

	_pathSystem->findPath(actor->actorSprite->x, actor->actorSprite->y, x, y);
	if (actor->pathResultCount > 1)
		direction = actor->pathWalker1->direction;

	actor->pathWalker1 = actor->pathWalker2;

	// TODO: Rework this
	PathResult *pathResults = _pathSystem->getPathResults();
	int16 pathResultsCount = _pathSystem->getPathResultsCount();

	if (pathResultsCount >= 2) {
		bool actorWasNotWalking = actor->pathResultCount == 0;
		actor->pathResultCount = pathResultsCount;
		actor->pathResultIndex = 1;
		actor->pathWalker1++;
		memcpy(actor->pathWalker2, pathResults, pathResultsCount * sizeof(PathResult));
		actor->pathNodeIndex = pathResults[0].nodeIndex;
		actor->pathEdgeIndex = pathResults[0].edgeIndex;
		actor->pathPolyIndex = pathResults[0].polyIndex;
		if (pathResults[0].direction + 8 != direction) {
			setActorSpriteFrameListIndex(actor->actorSprite, pathResults[0].direction + 8,
				actorWasNotWalking);
		}
		if (actor->actorSprite->frameCount == 1) {
			actor->pathResultCount = 0;
			actor->pathResultIndex = 0;
		}
	} else {
		if (pathResultsCount == 1) {
			actor->pathNodeIndex = pathResults[0].nodeIndex;
			actor->pathEdgeIndex = pathResults[0].edgeIndex;
			actor->pathPolyIndex = pathResults[0].polyIndex;
		} else {
			actor->pathNodeIndex = -1;
			actor->pathEdgeIndex = -1;
			actor->pathPolyIndex = -1;
		}

		actor->pathResultCount = 0;
		actor->pathResultIndex = 0;
		if ((actor->actorSprite->x != x || actor->actorSprite->y != y) &&
			(_queuedZoneAction.used == 0)) {
			setActorLookInDirection(actorIndex);
		}

	}

}

void PrisonerEngine::actorWalkToPathNode(int16 actorIndex, int16 nodeIndex) {
	PathNode *node = _pathSystem->getNode(nodeIndex);
	actorWalkToPoint(actorIndex, node->x, node->y);
}

void PrisonerEngine::resetActorPathWalk(int16 actorIndex) {
	Actor *actor = &_actors[actorIndex];
	actor->pathResultCount = 0;
	actor->pathResultIndex = 0;
	actor->pathNodeIndex = -1;
	actor->pathEdgeIndex = -1;
	actor->pathPolyIndex = -1;
	setActorLookInDirection(actorIndex);
}

void PrisonerEngine::actor21C78() {
	_actorPathFlag = false;
	_zoneActionItem = _queuedZoneAction;
	if (_mainActorIndex != -1 && _actors[_mainActorIndex].pathResultCount > 0) {
		Actor *actor = &_actors[_mainActorIndex];
		_actorPathFlag = true;
		_actorPathX = actor->pathWalker2[actor->pathResultCount - 1].x;
		_actorPathY = actor->pathWalker2[actor->pathResultCount - 1].y;
		_actorPathDeltaX = actor->pathWalker2[actor->pathResultIndex].x -
			actor->pathWalker2[actor->pathResultIndex - 1].x;
		_actorPathDeltaY = actor->pathWalker2[actor->pathResultIndex].y -
			actor->pathWalker2[actor->pathResultIndex - 1].y;
	}
}

void PrisonerEngine::actor21C89() {
	_queuedZoneAction = _zoneActionItem;
	if (_mainActorIndex != -1) {
		Actor *actor = &_actors[_mainActorIndex];
		if (_queuedZoneAction.pathNodeIndex != -1) {
			actorWalkToPathNode(_mainActorIndex, _queuedZoneAction.pathNodeIndex);
			if (actor->pathResultCount > 1) {
				int16 deltaX = actor->pathWalker2[1].x - actor->pathWalker2[0].x;
				int16 deltaY = actor->pathWalker2[1].y - actor->pathWalker2[0].y;
				if (_actorPathDeltaX * deltaX + _actorPathDeltaY * deltaY < 0) {
					resetActorPathWalk(_mainActorIndex);
					setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex, true);
				}
			}
		} else if (_actorPathFlag) {
			actorWalkToPoint(_mainActorIndex, _actorPathX, _actorPathY);
			if (actor->pathResultCount > 1) {
				int16 deltaX = actor->pathWalker2[1].x - actor->pathWalker2[0].x;
				int16 deltaY = actor->pathWalker2[1].y - actor->pathWalker2[0].y;
				if (_actorPathDeltaX * deltaX + _actorPathDeltaY * deltaY < 0) {
					resetActorPathWalk(_mainActorIndex);
					setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex, true);
				}
			}
		}
	}
}

// TODO: Rename this
void PrisonerEngine::setActorDirection(int16 actorIndex, int16 direction) {
	Actor *actor = &_actors[actorIndex];
	restoreActorAnimation(actorIndex);
	if (direction != -1) {
		setActorSpriteFrameListIndex(actor->actorSprite, direction, true);
	} else {
		resetActorPathWalk(actorIndex);
	}
	if (actorIndex == _mainActorIndex) {
		_queuedZoneAction.used = 0;
		_queuedZoneAction.zoneActionIndex = -1;
		_queuedZoneAction.pathNodeIndex = -1;
	}
	actor->pathResultIndex = 0;
	actor->pathResultCount = 0;
	actor->status = 1;
}

void PrisonerEngine::setActorLookInDirection(int16 actorIndex) {
	Actor *actor = &_actors[actorIndex];
	int16 direction = -1;

	if (actorIndex == _mainActorIndex && (_queuedZoneAction.pathNodeIndex != -1 || _queuedZoneAction.used != 0)) {
		direction = -1;
	} else {
		direction = calcDirection(actor->actorSprite->x, actor->actorSprite->y,
			actor->walkDestX, actor->walkDestY);
	}

	if (direction == -1) {
		direction = actor->actorSprite->frameListIndex;
		if (direction > 7)
			direction -= 8;
	}

	setActorSpriteFrameListIndex(actor->actorSprite, direction, true);

}

int16 PrisonerEngine::getActorX(int16 actorIndex) {
	return _actors[actorIndex].actorSprite->x;
}

int16 PrisonerEngine::getActorY(int16 actorIndex) {
	return _actors[actorIndex].actorSprite->y;
}

void PrisonerEngine::setActorX(int16 actorIndex, int16 x) {
	// TODO; Unused?
}

void PrisonerEngine::setActorY(int16 actorIndex, int16 y) {
	// TODO; Unused?
}

void PrisonerEngine::actorPutAtPos(int16 actorIndex, int16 x, int16 y) {
	Actor *actor = &_actors[actorIndex];
	actor->actorSprite->x = x;
	actor->actorSprite->y = y;
	actor->pathNodeIndex = -1;
	setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex, false);
}

void PrisonerEngine::actorPutAtPathNode(int16 actorIndex, int16 nodeIndex) {
	Actor *actor = &_actors[actorIndex];
	PathNode *node = _pathSystem->getNode(nodeIndex);
	actor->actorSprite->x = node->x;
	actor->actorSprite->y = node->y;
	actor->actorSprite->scale = node->scale;
	actor->pathNodeIndex = nodeIndex;
	setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex, false);
}

void PrisonerEngine::actorAnimation218A1(int16 actorIndex, Common::String &pakName, int16 pakSlot,
	int16 firstFrameListIndex, int16 lastFrameListIndex, int16 minTicks, int16 maxTicks) {

	Actor *actor = &_actors[actorIndex];

	actor->firstFrameListIndex = firstFrameListIndex;
	actor->lastFrameListIndex = lastFrameListIndex;
	actor->minTicks = minTicks;
	actor->maxTicks = maxTicks;

	restoreActorAnimation(actorIndex);
	backupActorAnimation(actorIndex);

	setActorAnimation(actorIndex, pakName, pakSlot, firstFrameListIndex, -1);
	setActorRandomFrameListIndex(actor);

}

void PrisonerEngine::setActorRandomFrameListIndex(Actor *actor) {
	int16 frameListIndex = _rnd->getRandomNumberRng(actor->firstFrameListIndex, actor->lastFrameListIndex);
	setActorSpriteFrameListIndex(actor->actorSprite, frameListIndex, true);
	actor->status = 3;
	actor->ticksFlag = 1;
	actor->ticks = 0;
}

void PrisonerEngine::setActorSpriteFrameListIndexIfIdle(int16 actorIndex, int16 frameListIndex) {
	Actor *actor = &_actors[actorIndex];
	if (actor->status == 0) {
		setActorSpriteFrameListIndex(actor->actorSprite, frameListIndex, true);
		actor->actorSprite->x = actor->x;
		actor->actorSprite->y = actor->y;
	}
	actor->status = 2;
}

void PrisonerEngine::updateActors() {

	// TODO: Keyboard handling

	for (int16 actorIndex = 0; actorIndex < kMaxActors; actorIndex++) {
		Actor *actor = &_actors[actorIndex];
		if (actor->resourceCacheSlot == -1)
			continue;

		//debug(8, "actorIndex = %d; actor->status = %d; actor->pathNodeIndex = %d", actorIndex, actor->status, actor->pathNodeIndex);

		if (actor->pathResultIndex > 0) {
			// Walking actor

			PathResult *currPathResult = actor->pathWalker1;

			if (actor->pathResultCount > 1) {
				PathResult *prevPathResult = actor->pathWalker1 - 1;
				int16 newX = currPathResult->x;
				int16 newY = currPathResult->y;
				int16 prevX = prevPathResult->x;
				int16 prevY = prevPathResult->y;
				if (!updateActorSpriteWalking(actorIndex, prevX, prevY, newX, newY)) {
					actor->actorSprite->scale = currPathResult->scale;
					if (actor->pathResultIndex + 1 < actor->pathResultCount) {
						/*
						if (actorIndex == _mainActorIndex && _mainActorValid && _inputFlag) {//TODO:key
							// Key input cancels walking here or something like that
							_inputFlag = false;
							if (_pathEdges[currPathResult->edgeIndex].polyCount > 0) {
								setActorLookInDirection(actorIndex);
							} else {
								setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex - 8,
									true);
							}
							actor->pathResultIndex = 0;
							actor->pathResultCount = 0;
							actor->pathWalker1 = actor->pathWalker2;
						} else */{
							//loc_3BF0C
							PathResult *nextPathResult = actor->pathWalker1 + 1;
							if (currPathResult->x - nextPathResult->x < 0) {
								actor->actorSprite->x += actor->actorSprite->xadd;
							}
							if (currPathResult->y - nextPathResult->y < 0) {
								actor->actorSprite->y += actor->actorSprite->yadd;
							}
							actor->pathNodeIndex = currPathResult->nodeIndex;
							actor->pathEdgeIndex = currPathResult->edgeIndex;
							actor->pathPolyIndex = currPathResult->polyIndex;
							if (currPathResult->direction != -1 && currPathResult->direction != prevPathResult->direction) {
								setActorSpriteFrameListIndex(actor->actorSprite, currPathResult->direction + 8,
									false);
							}
							actor->pathResultIndex++;
							actor->pathWalker1++;
						}
					} else {
						//loc_3BF8B
						actor->pathNodeIndex = currPathResult->nodeIndex;
						actor->pathEdgeIndex = currPathResult->edgeIndex;
						actor->pathPolyIndex = currPathResult->polyIndex;
						// TODO: Key input
						if (actorIndex == _mainActorIndex)
							_inputFlag = false;
						if (_pathSystem->getEdge(currPathResult->edgeIndex)->polyCount > 0) {
							setActorLookInDirection(actorIndex);
						} else {
							setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex - 8,
								true);
						}
						actor->pathResultIndex = 0;
						actor->pathResultCount = 0;
						actor->pathWalker1 = actor->pathWalker2;
					}
				} else {
					//loc_3C13E
					if (currPathResult->scale == prevPathResult->scale) {
						actor->actorSprite->scale = currPathResult->scale;
					} else if (currPathResult->y != prevPathResult->y) {
						actor->actorSprite->scale = currPathResult->scale - ((currPathResult->y - actor->actorSprite->y) *
							(currPathResult->scale - prevPathResult->scale) / (currPathResult->y - prevPathResult->y));
					} else {
						actor->actorSprite->scale = currPathResult->scale - ((currPathResult->x - actor->actorSprite->x) *
							(currPathResult->scale - prevPathResult->scale) / (currPathResult->x - prevPathResult->x));
					}
				}
			} else {
				//cseg02:0003C1BA
				if (actorIndex == _mainActorIndex) {
					if (_inputFlag) {
						if (_pathSystem->getEdge(currPathResult->edgeIndex)->polyCount > 0) {
							setActorLookInDirection(actorIndex);
						} else {
							setActorSpriteFrameListIndex(actor->actorSprite, actor->actorSprite->frameListIndex - 8,
								true);
						}
					}
					_inputFlag = false;
				}
				actor->pathResultIndex = 0;
				actor->pathResultCount = 0;
				actor->pathWalker1 = actor->pathWalker2;
				actor->actorSprite->xsub = 0;
				actor->actorSprite->ysub = 0;
			}

		} else {
			if (actorIndex == _mainActorIndex)
				_inputFlag = false;
			if (actor->status == 3) {
				if (actor->ticksFlag == 0) {
					if (actor->ticks <= 0) {
						setActorRandomFrameListIndex(actor);
					} else {
						actor->ticks -= _frameTicks;
					}
				} else if (!updateActorSpriteAnimation(actorIndex)) {
					actor->actorSprite->frameIndex = 0;
					actor->ticks = _rnd->getRandomNumberRng(actor->minTicks, actor->maxTicks);
					actor->ticksFlag = 0;
				}
			} else if (actor->status != 0) {
				ActorSprite *actorSprite = actor->actorSprite;
				actorSprite->ysub = 0;
				actorSprite->xsub = 0;
				if (!updateActorSpriteAnimation(actorIndex)) {
					if (actor->status == 1) {
						actor->status = 0;
					} else {
						actorSprite->x = actor->x;
						actorSprite->y = actor->y;
						actorSprite->frameIndex = 0;
					}
				}
			}
		}

	}

}

void PrisonerEngine::resetPathWalker(int16 pathWalkerIndex) {
	_pathWalkers[pathWalkerIndex].used = 0;
}

bool PrisonerEngine::updateActorSpriteWalking(int16 actorIndex, int16 prevX, int16 prevY, int16 newX, int16 newY) {
	ActorSprite *actorSprite = &_actorSprites[actorIndex];
	bool result;

	if (actorSprite->frameIndex != actorSprite->prevFrameIndex) {
		actorSprite->ticks -= _animationFrameTicks;
	}

	if (actorSprite->ticks > 0)
		return true;

	if (actorSprite->flag == 0) {
		actorSprite->flag = 1;
		result = true;
	} else {
		result = updateActorSpriteWalkingPosition(actorSprite, prevX, prevY, newX, newY);
	}

	if (!result)
		return false;

	actorSprite->prevFrameIndex = actorSprite->frameIndex;

	resetActorSpriteAnimationTicks(actorSprite);

	actorSprite->elementIndex = actorSprite->frameList->frames[actorSprite->frameIndex]->elementIndex;

	if (actorSprite->frameIndex + 1 < actorSprite->frameCount)
		actorSprite->frameIndex++;
	else
		actorSprite->frameIndex = 0;

	calcAnimationFrameBounds(actorSprite->animationResource, actorSprite->elementIndex,
		actorSprite->boundsX1, actorSprite->boundsY1,
		actorSprite->boundsX2, actorSprite->boundsY2);

	actorSprite->zoneX1 = actorSprite->x + actorSprite->boundsX1;
	actorSprite->zoneY1 = actorSprite->y + actorSprite->boundsY1;
	actorSprite->zoneX2 = actorSprite->x + actorSprite->boundsX2;
	actorSprite->zoneY2 = actorSprite->y + actorSprite->boundsY2;

	return result;

}

void PrisonerEngine::backupActorAnimation(int16 actorIndex) {
	Actor *actor = &_actors[actorIndex];
	_res->getSlotInfo(actor->resourceCacheSlot, actor->pakName, actor->pakSlot);
	actor->frameListIndex = actor->actorSprite->frameListIndex;
}

void PrisonerEngine::restoreActorAnimation(int16 actorIndex) {
	Actor *actor = &_actors[actorIndex];
	if (actor->status == 3) {
		setActorAnimation(actorIndex, actor->pakName, actor->pakSlot, actor->frameListIndex, -1);
	}
}

/* Actor sprites */

void PrisonerEngine::assignActorSprite(int16 actorIndex, ActorSprite &actorSprite) {
	actorSprite.used = 1;
	actorSprite.prevFrameIndex = actorSprite.frameIndex;
	actorSprite.frameListCount = actorSprite.animationResource->_anims.size();
	actorSprite.yadd = 0;
	actorSprite.xadd = 0;
	actorSprite.xsub = 0;
	actorSprite.ysub = 0;
	actorSprite.flag = 0;
	actorSprite.actorIndex = actorIndex;
	_actorSprites[actorIndex] = actorSprite;
}

void PrisonerEngine::setActorSpriteFrameListIndex(ActorSprite *actorSprite, int16 frameListIndex, bool firstFrame) {

	actorSprite->frameListIndex = frameListIndex;
	actorSprite->frameList = actorSprite->animationResource->_anims[frameListIndex];
	actorSprite->frameCount = actorSprite->frameList->frames.size();
	actorSprite->flag = 0;

	if (firstFrame || actorSprite->frameIndex >= actorSprite->frameCount || actorSprite->frameIndex < 0)
		actorSprite->frameIndex = 0;

	if (actorSprite->frameCount <= 1)
		actorSprite->prevFrameIndex = actorSprite->frameIndex;
	else
		actorSprite->prevFrameIndex = -1;

	actorSprite->xoffs = actorSprite->frameList->frames[actorSprite->frameIndex]->xOffs;
	actorSprite->yoffs = actorSprite->frameList->frames[actorSprite->frameIndex]->yOffs;
	actorSprite->xsub = 0;
	actorSprite->ysub = 0;

	resetActorSpriteAnimationTicks(actorSprite);

	actorSprite->elementIndex = actorSprite->frameList->frames[actorSprite->frameIndex]->elementIndex;

	calcAnimationFrameBounds(actorSprite->animationResource, actorSprite->elementIndex,
		actorSprite->boundsX1, actorSprite->boundsY1,
		actorSprite->boundsX2, actorSprite->boundsY2);

	actorSprite->zoneX1 = actorSprite->x + actorSprite->boundsX1;
	actorSprite->zoneY1 = actorSprite->y + actorSprite->boundsY1;
	actorSprite->zoneX2 = actorSprite->x + actorSprite->boundsX2;
	actorSprite->zoneY2 = actorSprite->y + actorSprite->boundsY2;

}

void PrisonerEngine::calcAnimationFrameBounds(AnimationResource *animationResource, int16 elementIndex,
	int16 &boundsX1, int16 &boundsY1, int16 &boundsX2, int16 &boundsY2) {

	AnimationCommand *animationCommand = animationResource->_elements[elementIndex]->commands[0];

	if (animationCommand->cmd == 1) {
		int16 celIndex = animationCommand->argAsInt16();
		int16 offsX = animationCommand->points[0].x;
		int16 offsY = animationCommand->points[0].y;
		int16 celWidth = animationResource->_cels[celIndex]->width;
		int16 celHeight = animationResource->_cels[celIndex]->height;
		boundsX1 = offsX;
		boundsY1 = offsY - celHeight;
		boundsX2 = offsX + celWidth;
		boundsY2 = offsY;
	} else {
		boundsX1 = 0;
		boundsY1 = 0;
		boundsX2 = 0;
		boundsY2 = 0;
	}

}

void PrisonerEngine::clearActorSprites() {
	for (int16 i = 0; i < kMaxActors; i++) {
		ActorSprite *actorSprite = &_actorSprites[i];
		actorSprite->used = 0;
		actorSprite->actorIndex = i;
	}
	_actorSpriteDrawQueue.clear();
}

bool PrisonerEngine::updateActorSpriteAnimation(int16 actorIndex) {
	ActorSprite *actorSprite = &_actorSprites[actorIndex];

	if (actorSprite->frameCount == 1)
		return false;

	if (actorSprite->frameIndex != actorSprite->prevFrameIndex)
		actorSprite->ticks -= _animationFrameTicks;

	if (actorSprite->ticks > 0)
		return true;

	resetActorSpriteAnimationTicks(actorSprite);

	AnimationFrame *frame = actorSprite->frameList->frames[actorSprite->frameIndex];
	actorSprite->x += frame->xOffs;
	actorSprite->y += frame->yOffs;
	actorSprite->elementIndex = frame->elementIndex;
	actorSprite->prevFrameIndex = actorSprite->frameIndex;

	if (actorSprite->frameIndex + 1 >= actorSprite->frameCount /*frameCount?!*/) {
		actorSprite->frameIndex = -1;
		return false;
	}

	actorSprite->frameIndex++;

	calcAnimationFrameBounds(actorSprite->animationResource, actorSprite->elementIndex,
		actorSprite->boundsX1, actorSprite->boundsY1,
		actorSprite->boundsX2, actorSprite->boundsY2);

	actorSprite->zoneX1 = actorSprite->x + actorSprite->boundsX1;
	actorSprite->zoneY1 = actorSprite->y + actorSprite->boundsY1;
	actorSprite->zoneX2 = actorSprite->x + actorSprite->boundsX2;
	actorSprite->zoneY2 = actorSprite->y + actorSprite->boundsY2;

	return true;
}

bool PrisonerEngine::updateActorSpriteWalkingPosition(ActorSprite *actorSprite, int16 prevX, int16 prevY, int16 x, int16 y) {
	bool result = false, flag = false;
	int16 newX, newY;

	if (actorSprite->frameIndex < actorSprite->frameCount) {
		AnimationFrame *frame = actorSprite->frameList->frames[actorSprite->frameIndex];
		actorSprite->xoffs = frame->xOffs;
		actorSprite->yoffs = frame->yOffs;
	} else {
		AnimationFrame *frame = actorSprite->frameList->frames[0];
		actorSprite->xoffs = frame->xOffs;
		actorSprite->yoffs = frame->yOffs;
	}

	newX = actorSprite->x + actorSprite->xoffs;
	newY = actorSprite->y + actorSprite->yoffs;

	actorSprite->xsub = actorSprite->x;
	actorSprite->ysub = actorSprite->y;

	switch (actorSprite->frameListIndex - 8) {
	case 0:
		flag = newY <= y;
		break;
	case 1:
		flag = newY <= y && x <= newX;
		break;
	case 2:
		flag = x <= newX;
		break;
	case 3:
		flag = x <= newX && newY >= y;
		break;
	case 4:
		flag = newY >= y;
		break;
	case 5:
		flag = newY >= y && x >= newX;
		break;
	case 6:
		flag = x >= newX;
		break;
	case 7:
		flag = newY <= y && x >= newX;
		break;
	}

	if (flag) {
		actorSprite->xsub -= x;
		actorSprite->ysub -= y;
		_pathSystem->path252C1(prevX, prevY, newX, newY, x, y);
		actorSprite->xadd = newX - x;
		actorSprite->yadd = newY - y;
		actorSprite->x = x;
		actorSprite->y = y;
		result = false;
	} else {
		actorSprite->x = newX;
		actorSprite->y = newY;
		_pathSystem->path252C1(prevX, prevY, actorSprite->x, actorSprite->y, x, y);
		actorSprite->xsub -= actorSprite->x;
		actorSprite->ysub -= actorSprite->y;
		actorSprite->yadd = 0;
		actorSprite->xadd = 0;
		result = true;
	}

	return result;
}

void PrisonerEngine::resetActorSpriteAnimationTicks(ActorSprite *actorSprite) {
	if (_animationSpeed == 0) {
		actorSprite->ticks = 0;
	} else if (_animationSpeed == 100) {
		actorSprite->ticks = actorSprite->frameList->frames[actorSprite->frameIndex]->ticks;
	} else {
		actorSprite->ticks = actorSprite->frameList->frames[actorSprite->frameIndex]->ticks * _animationSpeed / 100;
	}
}

/* Actor alt animations */

int16 PrisonerEngine::addActorAltAnimation(Common::String &pakName, int16 pakSlot) {
	int16 index = _altActorAnimations.getFreeSlot();
	AltActorAnimation *altActorAnimation = &_altActorAnimations[index];
	altActorAnimation->resourceCacheSlot = _res->load<AnimationResource>(pakName, pakSlot, 4);
	altActorAnimation->value = 0;
	return index;
}

void PrisonerEngine::unloadActorAltAnimation(int16 altActorAnimationIndex) {
	AltActorAnimation *altActorAnimation = &_altActorAnimations[altActorAnimationIndex];
	clearActorFrameSoundsByActorIndex(altActorAnimationIndex);
	_res->unload(altActorAnimation->resourceCacheSlot);
	altActorAnimation->resourceCacheSlot = -1;
}

void PrisonerEngine::setActorAltAnimationAtPos(int16 actorIndex, int16 altAnimationIndex, int16 frameListIndex, int16 x, int16 y) {
	Actor *actor = &_actors[actorIndex];
	AltActorAnimation *altActorAnimation = &_altActorAnimations[altAnimationIndex];
	ActorSprite tempActorSprite;

	actor->altAnimationIndex = altAnimationIndex;
	SWAP(actor->resourceCacheSlot, altActorAnimation->resourceCacheSlot);

	tempActorSprite = *actor->actorSprite;
	tempActorSprite.animationResource = _res->get<AnimationResource>(actor->resourceCacheSlot);
	tempActorSprite.frameIndex = 0;
	tempActorSprite.x = x;
	tempActorSprite.y = y;

	actor->pathNodeIndex = -1;
	actor->pathPolyIndex = -1;
	actor->pathEdgeIndex = -1;
	actor->x = x;
	actor->y = y;

	assignActorSprite(actorIndex, tempActorSprite);

	setActorSpriteFrameListIndex(actor->actorSprite, frameListIndex, true);

	actor->status = 1;
	actor->pathResultCount = 0;

	if (actorIndex == _mainActorIndex) {
		_queuedZoneAction.used = 0;
		_queuedZoneAction.zoneActionIndex = -1;
		_queuedZoneAction.pathNodeIndex = -1;
	}

}

/* ActorFrameSounds */

void PrisonerEngine::updateActorFrameSounds() {
	// TODO
}

int16 PrisonerEngine::addActorFrameSound(int16 actorIndex, int16 soundIndex, int16 volume, int16 frameNum, int16 unk1) {

	int16 actorFrameSoundIndex = _actorFrameSounds.getFreeSlot();
	ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];

	actorFrameSound->actorIndex = actorIndex;
	actorFrameSound->soundIndex = soundIndex;
	actorFrameSound->frameNum = frameNum;
	actorFrameSound->unk1 = unk1;
	actorFrameSound->volume = volume;
	actorFrameSound->unk2 = 0;

	_actorFrameSoundItemsCount++;

	return actorFrameSoundIndex;
}

void PrisonerEngine::removeActorFrameSound(int16 actorFrameSoundIndex) {
	_actorFrameSounds[actorFrameSoundIndex].actorIndex = -1;
	_actorFrameSoundItemsCount--;
}

void PrisonerEngine::clearActorFrameSoundsBySoundIndex(int16 soundIndex) {
	for (int16 actorFrameSoundIndex = 0; actorFrameSoundIndex < kMaxActorFrameSounds; actorFrameSoundIndex++) {
		ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];
		if (actorFrameSound->soundIndex == soundIndex) {
			actorFrameSound->actorIndex = -1;
		}
	}
}

void PrisonerEngine::clearActorFrameSoundsByActorIndex(int16 actorIndex) {
	for (int16 actorFrameSoundIndex = 0; actorFrameSoundIndex < kMaxActorFrameSounds; actorFrameSoundIndex++) {
		ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];
		if (actorFrameSound->actorIndex == actorIndex) {
			actorFrameSound->actorIndex = -1;
		}
	}
}

void PrisonerEngine::setActorFrameSound(int16 actorFrameSoundIndex, int16 soundIndex, int16 volume) {
	ActorFrameSound *actorFrameSound = &_actorFrameSounds[actorFrameSoundIndex];
	if (soundIndex != -1)
		actorFrameSound->soundIndex = soundIndex;
	if (volume != -1)
		actorFrameSound->volume = volume;
}

void PrisonerEngine::clearActorFrameSounds() {
	_actorFrameSoundItemsCount = 0;
	for (int16 actorFrameSoundIndex = 0; actorFrameSoundIndex < kMaxActorFrameSounds; actorFrameSoundIndex++) {
		_actorFrameSounds[actorFrameSoundIndex].actorIndex = -1;
	}
}

} // End of namespace Prisoner
