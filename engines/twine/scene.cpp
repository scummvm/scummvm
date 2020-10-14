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

#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/extra.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/movements.h"
#include "twine/music.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

void Scene::setActorStaticFlags(int32 actorIdx, uint16 staticFlags) {
	if (staticFlags & 0x1) {
		sceneActors[actorIdx].staticFlags.bComputeCollisionWithObj = 1;
	}
	if (staticFlags & 0x2) {
		sceneActors[actorIdx].staticFlags.bComputeCollisionWithBricks = 1;
	}
	if (staticFlags & 0x4) {
		sceneActors[actorIdx].staticFlags.bIsZonable = 1;
	}
	if (staticFlags & 0x8) {
		sceneActors[actorIdx].staticFlags.bUsesClipping = 1;
	}
	if (staticFlags & 0x10) {
		sceneActors[actorIdx].staticFlags.bCanBePushed = 1;
	}
	if (staticFlags & 0x20) {
		sceneActors[actorIdx].staticFlags.bComputeLowCollision = 1;
	}
	if (staticFlags & 0x40) {
		sceneActors[actorIdx].staticFlags.bCanDrown = 1;
	}
	if (staticFlags & 0x80) {
		sceneActors[actorIdx].staticFlags.bUnk80 = 1;
	}

	if (staticFlags & 0x100) {
		sceneActors[actorIdx].staticFlags.bUnk0100 = 1;
	}
	if (staticFlags & 0x200) {
		sceneActors[actorIdx].staticFlags.bIsHidden = 1;
	}
	if (staticFlags & 0x400) {
		sceneActors[actorIdx].staticFlags.bIsSpriteActor = 1;
	}
	if (staticFlags & 0x800) {
		sceneActors[actorIdx].staticFlags.bCanFall = 1;
	}
	if (staticFlags & 0x1000) {
		sceneActors[actorIdx].staticFlags.bDoesntCastShadow = 1;
	}
	if (staticFlags & 0x2000) {
		//sceneActors[actorIdx].staticFlags.bIsBackgrounded = 1;
	}
	if (staticFlags & 0x4000) {
		sceneActors[actorIdx].staticFlags.bIsCarrierActor = 1;
	}
	if (staticFlags & 0x8000) {
		sceneActors[actorIdx].staticFlags.bUseMiniZv = 1;
	}
}

void Scene::loadScene() {
	int32 i;
	int32 scriptSize = 0;
	uint8 *localScene = currentScene;

	// load scene ambience properties
	_engine->_text->currentTextBank = *(localScene++);
	currentGameOverScene = *(localScene++);
	localScene += 4;

	alphaLight = *((uint16 *)localScene);
	localScene += 2;
	betaLight = *((uint16 *)localScene);
	localScene += 2;

	// FIXME: Workaround to fix lighting issue - not using proper dark light
	alphaLight = 896;
	betaLight = 950;

	sampleAmbiance[0] = *((uint16 *)localScene);
	localScene += 2;
	sampleRepeat[0] = *((uint16 *)localScene);
	localScene += 2;
	sampleRound[0] = *((uint16 *)localScene);
	localScene += 2;

	sampleAmbiance[1] = *((uint16 *)localScene);
	localScene += 2;
	sampleRepeat[1] = *((uint16 *)localScene);
	localScene += 2;
	sampleRound[1] = *((uint16 *)localScene);
	localScene += 2;

	sampleAmbiance[2] = *((uint16 *)localScene);
	localScene += 2;
	sampleRepeat[2] = *((uint16 *)localScene);
	localScene += 2;
	sampleRound[2] = *((uint16 *)localScene);
	localScene += 2;

	sampleAmbiance[3] = *((uint16 *)localScene);
	localScene += 2;
	sampleRepeat[3] = *((uint16 *)localScene);
	localScene += 2;
	sampleRound[3] = *((uint16 *)localScene);
	localScene += 2;

	sampleMinDelay = *((uint16 *)localScene);
	localScene += 2;
	sampleMinDelayRnd = *((uint16 *)localScene);
	localScene += 2;

	sceneMusic = *(localScene++);

	// load hero properties
	sceneHeroX = *((uint16 *)localScene);
	localScene += 2;
	sceneHeroY = *((uint16 *)localScene);
	localScene += 2;
	sceneHeroZ = *((uint16 *)localScene);
	localScene += 2;

	scriptSize = *((uint16 *)localScene);
	localScene += 2;
	sceneHero->moveScript = localScene;
	localScene += scriptSize;

	scriptSize = *((uint16 *)localScene);
	localScene += 2;
	sceneHero->lifeScript = localScene;
	localScene += scriptSize;

	sceneNumActors = *((uint16 *)localScene);
	localScene += 2;

	for (i = 1; i < sceneNumActors; i++) {
		uint16 staticFlags;

		_engine->_actor->resetActor(i);

		staticFlags = *((uint16 *)localScene);
		localScene += 2;
		setActorStaticFlags(i, staticFlags);

		sceneActors[i].entity = *((uint16 *)localScene);
		localScene += 2;

		if (!sceneActors[i].staticFlags.bIsSpriteActor) {
			_engine->_hqrdepack->hqrGetallocEntry(&sceneActors[i].entityDataPtr, Resources::HQR_FILE3D_FILE, sceneActors[i].entity);
		}

		sceneActors[i].body = *(localScene++);
		sceneActors[i].anim = (AnimationTypes)*(localScene++);
		sceneActors[i].sprite = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].x = *((uint16 *)localScene);
		sceneActors[i].collisionX = sceneActors[i].x;
		localScene += 2;
		sceneActors[i].y = *((uint16 *)localScene);
		sceneActors[i].collisionY = sceneActors[i].y;
		localScene += 2;
		sceneActors[i].z = *((uint16 *)localScene);
		sceneActors[i].collisionZ = sceneActors[i].z;
		localScene += 2;
		sceneActors[i].strengthOfHit = *(localScene++);
		sceneActors[i].bonusParameter = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].bonusParameter &= 0xFE;
		sceneActors[i].angle = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].speed = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].controlMode = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].info0 = *((int16 *)localScene);
		localScene += 2;
		sceneActors[i].info1 = *((int16 *)localScene);
		localScene += 2;
		sceneActors[i].info2 = *((int16 *)localScene);
		localScene += 2;
		sceneActors[i].info3 = *((int16 *)localScene);
		localScene += 2;
		sceneActors[i].followedActor = sceneActors[i].info3;
		sceneActors[i].bonusAmount = *(localScene++);
		sceneActors[i].talkColor = *(localScene++);
		sceneActors[i].armor = *(localScene++);
		sceneActors[i].life = *(localScene++);

		scriptSize = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].moveScript = localScene;
		localScene += scriptSize;

		scriptSize = *((uint16 *)localScene);
		localScene += 2;
		sceneActors[i].lifeScript = localScene;
		localScene += scriptSize;
	}

	sceneNumZones = *((uint16 *)localScene);
	localScene += 2;

	for (i = 0; i < sceneNumZones; i++) {
		sceneZones[i].bottomLeft.x = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].bottomLeft.y = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].bottomLeft.z = *((uint16 *)localScene);
		localScene += 2;

		sceneZones[i].topRight.x = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].topRight.y = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].topRight.z = *((uint16 *)localScene);
		localScene += 2;

		sceneZones[i].type = *((uint16 *)localScene);
		localScene += 2;

		sceneZones[i].infoData.generic.info0 = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].infoData.generic.info1 = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].infoData.generic.info2 = *((uint16 *)localScene);
		localScene += 2;
		sceneZones[i].infoData.generic.info3 = *((uint16 *)localScene);
		localScene += 2;

		sceneZones[i].snap = *((uint16 *)localScene);
		localScene += 2;
	}

	sceneNumTracks = *((uint16 *)localScene);
	localScene += 2;

	for (i = 0; i < sceneNumTracks; i++) {
		sceneTracks[i].x = *((uint16 *)localScene);
		localScene += 2;
		sceneTracks[i].y = *((uint16 *)localScene);
		localScene += 2;
		sceneTracks[i].z = *((uint16 *)localScene);
		localScene += 2;
	}
}

int32 Scene::initScene(int32 index) {
	// load scene from file
	_engine->_hqrdepack->hqrGetallocEntry(&currentScene, Resources::HQR_SCENE_FILE, index);

	loadScene();

	return 1;
}

void Scene::resetScene() {
	int32 i;

	_engine->_extra->resetExtras();

	for (i = 0; i < NUM_SCENES_FLAGS; i++) {
		sceneFlags[i] = 0;
	}

	for (i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		_engine->_redraw->overlayList[i].info0 = -1;
	}

	_engine->_actor->currentPositionInBodyPtrTab = 0;
	_engine->_screens->useAlternatePalette = 0;
}

void Scene::changeScene() {
	int32 a;

	// change twinsen house destroyed hard-coded
	if (needChangeScene == 4 && _engine->_gameState->gameFlags[30] != 0)
		needChangeScene = 118;

	// local backup previous scene
	previousSceneIdx = currentSceneIdx;
	currentSceneIdx = needChangeScene;

	_engine->_sound->stopSamples();

	resetScene();
	_engine->_actor->loadHeroEntities();

	sceneHero->controlMode = 1;
	sceneHero->zone = -1;
	sceneHero->positionInLifeScript = 0;
	sceneHero->positionInMoveScript = -1;
	sceneHero->labelIdx = -1;

	initScene(needChangeScene);

	//TODO: treat holomap trajectories

	if (needChangeScene == 116 || needChangeScene == 117)
		_engine->_text->currentTextBank = 10;

	_engine->_text->initTextBank(_engine->_text->currentTextBank + 3);
	_engine->_grid->initGrid(needChangeScene);

	if (heroPositionType == kZone) {
		newHeroX = zoneHeroX;
		newHeroY = zoneHeroY;
		newHeroZ = zoneHeroZ;
	}

	if (heroPositionType == kScene || heroPositionType == kNoPosition) {
		newHeroX = sceneHeroX;
		newHeroY = sceneHeroY;
		newHeroZ = sceneHeroZ;
	}

	sceneHero->x = newHeroX;
	sceneHero->y = heroYBeforeFall = newHeroY;
	sceneHero->z = newHeroZ;

	_engine->_renderer->setLightVector(alphaLight, betaLight, 0);

	if (previousSceneIdx != needChangeScene) {
		_engine->_actor->previousHeroBehaviour = _engine->_actor->heroBehaviour;
		_engine->_actor->previousHeroAngle = sceneHero->angle;
		_engine->_gameState->saveGame();
	}

	_engine->_actor->restartHeroScene();

	for (a = 1; a < sceneNumActors; a++) {
		_engine->_actor->initActor(a);
	}

	_engine->_gameState->inventoryNumKeys = 0;
	_engine->disableScreenRecenter = 0;
	heroPositionType = kNoPosition;
	sampleAmbienceTime = 0;

	_engine->_grid->newCameraX = sceneActors[currentlyFollowedActor].x >> 9;
	_engine->_grid->newCameraY = sceneActors[currentlyFollowedActor].y >> 8;
	_engine->_grid->newCameraZ = sceneActors[currentlyFollowedActor].z >> 9;

	_engine->_gameState->magicBallIdx = -1;
	_engine->_movements->heroMoved = 1;
	_engine->_grid->useCellingGrid = -1;
	_engine->_grid->cellingGridIdx = -1;
	_engine->_redraw->reqBgRedraw = 1;
	_engine->_screens->lockPalette = 0;

	needChangeScene = -1;
	changeRoomVar10 = 1;
	changeRoomVar11 = 14;

	_engine->_renderer->setLightVector(alphaLight, betaLight, 0);

	if (sceneMusic != -1) {
		_engine->_music->playMidiMusic(sceneMusic, 0); // TODO this should play midi or cd tracks
	}
}

void Scene::processEnvironmentSound() {
	int16 s, currentAmb, decal, repeat;
	int16 sampleIdx = -1;

	if (_engine->lbaTime >= sampleAmbienceTime) {
		currentAmb = _engine->getRandomNumber(4); // random ambiance

		for (s = 0; s < 4; s++) {
			if (!(samplePlayed & (1 << currentAmb))) { // if not already played
				samplePlayed |= (1 << currentAmb);     // make sample played

				if (samplePlayed == 15) { // reset if all samples played
					samplePlayed = 0;
				}

				sampleIdx = sampleAmbiance[currentAmb];
				if (sampleIdx != -1) {
					decal = sampleRound[currentAmb];
					repeat = sampleRepeat[currentAmb];

					_engine->_sound->playSample(sampleIdx, (0x1000 + _engine->getRandomNumber(decal) - (decal / 2)), repeat, 110, -1, 110, -1);
					break;
				}
			}

			currentAmb++;    // try next ambiance
			currentAmb &= 3; // loop in all 4 ambiances
		}

		// compute next ambiance timer
		sampleAmbienceTime = _engine->lbaTime + (_engine->getRandomNumber(sampleMinDelayRnd) + sampleMinDelay) * 50;
	}
}

void Scene::processZoneExtraBonus(ZoneStruct *zone) {
	int32 a, numBonus;
	int8 bonusTable[8], currentBonus;

	numBonus = 0;

	// bonus not used yet
	if (!zone->infoData.generic.info3) {
		for (a = 0; a < 5; a++) {
			if (zone->infoData.generic.info1 & (1 << (a + 4))) {
				bonusTable[numBonus++] = a;
			}
		}

		if (numBonus) {
			int32 angle, index;
			currentBonus = bonusTable[_engine->getRandomNumber(numBonus)];

			// if bonus is magic an no magic level yet, then give life points
			if (!_engine->_gameState->magicLevelIdx && currentBonus == 2) {
				currentBonus = 1;
			}

			angle = _engine->_movements->getAngleAndSetTargetActorDistance(ABS(zone->topRight.x + zone->bottomLeft.x) / 2, ABS(zone->topRight.z + zone->bottomLeft.z) / 2, sceneHero->x, sceneHero->z);
			index = _engine->_extra->addExtraBonus(ABS(zone->topRight.x + zone->bottomLeft.x) / 2, zone->topRight.y, ABS(zone->topRight.z + zone->bottomLeft.z) / 2, 180, angle, currentBonus + 3, zone->infoData.generic.info2);

			if (index != -1) {
				_engine->_extra->extraList[index].type |= 0x400;
				zone->infoData.generic.info3 = 1; // set as used
			}
		}
	}
}

void Scene::processActorZones(int32 actorIdx) {
	int32 currentX, currentY, currentZ, z, tmpCellingGrid;
	ActorStruct *actor;

	actor = &sceneActors[actorIdx];

	currentX = actor->x;
	currentY = actor->y;
	currentZ = actor->z;

	actor->zone = -1;
	tmpCellingGrid = 0;

	if (!actorIdx) {
		currentActorInZone = actorIdx;
	}

	for (z = 0; z < sceneNumZones; z++) {
		ZoneStruct *zone = &sceneZones[z];

		// check if actor is in zone
		if ((currentX >= zone->bottomLeft.x && currentX <= zone->topRight.x) &&
		    (currentY >= zone->bottomLeft.y && currentY <= zone->topRight.y) &&
		    (currentZ >= zone->bottomLeft.z && currentZ <= zone->topRight.z)) {
			switch (zone->type) {
			case kCube:
				if (!actorIdx && actor->life > 0) {
					needChangeScene = zone->infoData.ChangeScene.newSceneIdx;
					zoneHeroX = actor->x - zone->bottomLeft.x + zone->infoData.ChangeScene.x;
					zoneHeroY = actor->y - zone->bottomLeft.y + zone->infoData.ChangeScene.y;
					zoneHeroZ = actor->z - zone->bottomLeft.z + zone->infoData.ChangeScene.z;
					heroPositionType = kZone;
				}
				break;
			case kCamera:
				if (currentlyFollowedActor == actorIdx) {
					_engine->disableScreenRecenter = 1;
					if (_engine->_grid->newCameraX != zone->infoData.CameraView.x || _engine->_grid->newCameraY != zone->infoData.CameraView.y || _engine->_grid->newCameraZ != zone->infoData.CameraView.z) {
						_engine->_grid->newCameraX = zone->infoData.CameraView.x;
						_engine->_grid->newCameraY = zone->infoData.CameraView.y;
						_engine->_grid->newCameraZ = zone->infoData.CameraView.z;
						_engine->_redraw->reqBgRedraw = 1;
					}
				}
				break;
			case kSceneric:
				actor->zone = zone->infoData.Sceneric.zoneIdx;
				break;
			case kGrid:
				if (currentlyFollowedActor == actorIdx) {
					tmpCellingGrid = 1;
					if (_engine->_grid->useCellingGrid != zone->infoData.CeillingGrid.newGrid) {
						if (zone->infoData.CeillingGrid.newGrid != -1) {
							_engine->_grid->createGridMap();
						}

						_engine->_grid->useCellingGrid = zone->infoData.CeillingGrid.newGrid;
						_engine->_grid->cellingGridIdx = z;
						_engine->freezeTime();
						_engine->_grid->initCellingGrid(_engine->_grid->useCellingGrid);
						_engine->unfreezeTime();
					}
				}
				break;
			case kObject:
				if (!actorIdx && _engine->_movements->heroAction != 0) {
					_engine->_animations->initAnim(kAction, 1, 0, 0);
					processZoneExtraBonus(zone);
				}
				break;
			case kText:
				if (!actorIdx && _engine->_movements->heroAction != 0) {
					_engine->freezeTime();
					_engine->_text->setFontCrossColor(zone->infoData.DisplayText.textColor);
					talkingActor = actorIdx;
					_engine->_text->drawTextFullscreen(zone->infoData.DisplayText.textIdx);
					_engine->unfreezeTime();
					_engine->_redraw->redrawEngineActions(1);
				}
				break;
			case kLadder:
				if (!actorIdx && _engine->_actor->heroBehaviour != kProtoPack && (actor->anim == kForward || actor->anim == kTopLadder || actor->anim == kClimbLadder)) {
					_engine->_movements->rotateActor(actor->boudingBox.x.bottomLeft, actor->boudingBox.z.bottomLeft, actor->angle + 0x580);
					_engine->_renderer->destX += _engine->_movements->processActorX;
					_engine->_renderer->destZ += _engine->_movements->processActorZ;

					if (_engine->_renderer->destX >= 0 && _engine->_renderer->destZ >= 0 && _engine->_renderer->destX <= 0x7E00 && _engine->_renderer->destZ <= 0x7E00) {
						if (_engine->_grid->getBrickShape(_engine->_renderer->destX, actor->y + 0x100, _engine->_renderer->destZ)) {
							currentActorInZone = 1;
							if (actor->y >= ABS(zone->bottomLeft.y + zone->topRight.y) / 2) {
								_engine->_animations->initAnim(kTopLadder, 2, 0, actorIdx); // reached end of ladder
							} else {
								_engine->_animations->initAnim(kClimbLadder, 0, 255, actorIdx); // go up in ladder
							}
						}
					}
				}
				break;
			}
		}
	}

	if (!tmpCellingGrid && actorIdx == currentlyFollowedActor && _engine->_grid->useCellingGrid != -1) {
		_engine->_grid->useCellingGrid = -1;
		_engine->_grid->cellingGridIdx = -1;
		_engine->_grid->createGridMap();
		_engine->_redraw->reqBgRedraw = 1;
	}
}

} // namespace TwinE
