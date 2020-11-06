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

#include "twine/scene.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/util.h"
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
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

void Scene::setActorStaticFlags(int32 actorIdx, uint16 staticFlags) {
	if (staticFlags & 0x1) {
		_sceneActors[actorIdx].staticFlags.bComputeCollisionWithObj = 1;
	}
	if (staticFlags & 0x2) {
		_sceneActors[actorIdx].staticFlags.bComputeCollisionWithBricks = 1;
	}
	if (staticFlags & 0x4) {
		_sceneActors[actorIdx].staticFlags.bIsZonable = 1;
	}
	if (staticFlags & 0x8) {
		_sceneActors[actorIdx].staticFlags.bUsesClipping = 1;
	}
	if (staticFlags & 0x10) {
		_sceneActors[actorIdx].staticFlags.bCanBePushed = 1;
	}
	if (staticFlags & 0x20) {
		_sceneActors[actorIdx].staticFlags.bComputeLowCollision = 1;
	}
	if (staticFlags & 0x40) {
		_sceneActors[actorIdx].staticFlags.bCanDrown = 1;
	}
	if (staticFlags & 0x80) {
		_sceneActors[actorIdx].staticFlags.bUnk80 = 1;
	}

	if (staticFlags & 0x100) {
		_sceneActors[actorIdx].staticFlags.bUnk0100 = 1;
	}
	if (staticFlags & 0x200) {
		_sceneActors[actorIdx].staticFlags.bIsHidden = 1;
	}
	if (staticFlags & 0x400) {
		_sceneActors[actorIdx].staticFlags.bIsSpriteActor = 1;
	}
	if (staticFlags & 0x800) {
		_sceneActors[actorIdx].staticFlags.bCanFall = 1;
	}
	if (staticFlags & 0x1000) {
		_sceneActors[actorIdx].staticFlags.bDoesntCastShadow = 1;
	}
	if (staticFlags & 0x2000) {
		//sceneActors[actorIdx].staticFlags.bIsBackgrounded = 1;
	}
	if (staticFlags & 0x4000) {
		_sceneActors[actorIdx].staticFlags.bIsCarrierActor = 1;
	}
	if (staticFlags & 0x8000) {
		_sceneActors[actorIdx].staticFlags.bUseMiniZv = 1;
	}
}

bool Scene::loadSceneLBA1() {
	Common::MemoryReadStream stream(currentScene, _currentSceneSize);

	// load scene ambience properties
	sceneTextBank = stream.readByte();
	currentGameOverScene = stream.readByte();
	stream.skip(4);

	alphaLight = stream.readUint16LE();
	betaLight = stream.readUint16LE();

	// FIXME: Workaround to fix lighting issue - not using proper dark light
	alphaLight = 896;
	betaLight = 950;

	sampleAmbiance[0] = stream.readUint16LE();
	sampleRepeat[0] = stream.readUint16LE();
	sampleRound[0] = stream.readUint16LE();

	sampleAmbiance[1] = stream.readUint16LE();
	sampleRepeat[1] = stream.readUint16LE();
	sampleRound[1] = stream.readUint16LE();

	sampleAmbiance[2] = stream.readUint16LE();
	sampleRepeat[2] = stream.readUint16LE();
	sampleRound[2] = stream.readUint16LE();

	sampleAmbiance[3] = stream.readUint16LE();
	sampleRepeat[3] = stream.readUint16LE();
	sampleRound[3] = stream.readUint16LE();

	sampleMinDelay = stream.readUint16LE();
	sampleMinDelayRnd = stream.readUint16LE();

	sceneMusic = stream.readByte();

	// load hero properties
	sceneHeroX = stream.readUint16LE();
	sceneHeroY = stream.readUint16LE();
	sceneHeroZ = stream.readUint16LE();

	sceneHero->moveScriptSize = stream.readUint16LE();
	sceneHero->moveScript = currentScene + stream.pos();
	stream.skip(sceneHero->moveScriptSize);

	sceneHero->lifeScriptSize = stream.readUint16LE();
	sceneHero->lifeScript = currentScene + stream.pos();
	stream.skip(sceneHero->lifeScriptSize);

	sceneNumActors = stream.readUint16LE();
	for (int32 i = 1; i < sceneNumActors; i++) {
		_engine->_actor->resetActor(i);

		const uint16 staticFlags = stream.readUint16LE();
		setActorStaticFlags(i, staticFlags);

		ActorStruct* act = &_sceneActors[i];
		act->entity = stream.readUint16LE();

		if (!act->staticFlags.bIsSpriteActor) {
			act->entityDataSize = HQR::getAllocEntry(&act->entityDataPtr, Resources::HQR_FILE3D_FILE, act->entity);
		}

		act->body = stream.readByte();
		act->anim = (AnimationTypes) stream.readByte();
		act->sprite = stream.readUint16LE();
		act->x = stream.readUint16LE();
		act->collisionX = act->x;
		act->y = stream.readUint16LE();
		act->collisionY = act->y;
		act->z = stream.readUint16LE();
		act->collisionZ = act->z;
		act->strengthOfHit = stream.readByte();
		act->bonusParameter = stream.readUint16LE();
		act->bonusParameter &= 0xFE;
		act->angle = stream.readUint16LE();
		act->speed = stream.readUint16LE();
		act->controlMode = (ControlMode)stream.readUint16LE();
		act->info0 = stream.readUint16LE();
		act->info1 = stream.readUint16LE();
		act->info2 = stream.readUint16LE();
		act->info3 = stream.readUint16LE();
		act->followedActor = act->info3;
		act->bonusAmount = stream.readByte();
		act->talkColor = stream.readByte();
		act->armor = stream.readByte();
		act->life = stream.readByte();

		act->moveScriptSize = stream.readUint16LE();
		act->moveScript = currentScene + stream.pos();
		stream.skip(act->moveScriptSize);

		act->lifeScriptSize = stream.readUint16LE();
		act->lifeScript = currentScene + stream.pos();
		stream.skip(act->lifeScriptSize);
	}

	sceneNumZones = stream.readUint16LE();
	for (int32 i = 0; i < sceneNumZones; i++) {
		ZoneStruct* zone = &sceneZones[i];
		zone->bottomLeft.x = stream.readUint16LE();
		zone->bottomLeft.y = stream.readUint16LE();
		zone->bottomLeft.z = stream.readUint16LE();

		zone->topRight.x = stream.readUint16LE();
		zone->topRight.y = stream.readUint16LE();
		zone->topRight.z = stream.readUint16LE();

		zone->type = stream.readUint16LE();

		zone->infoData.generic.info0 = stream.readUint16LE();
		zone->infoData.generic.info1 = stream.readUint16LE();
		zone->infoData.generic.info2 = stream.readUint16LE();
		zone->infoData.generic.info3 = stream.readUint16LE();

		zone->snap = stream.readUint16LE();
	}

	sceneNumTracks = stream.readUint16LE();
	for (int32 i = 0; i < sceneNumTracks; i++) {
		ScenePoint* point = &sceneTracks[i];
		point->x = stream.readUint16LE();
		point->y = stream.readUint16LE();
		point->z = stream.readUint16LE();
	}

	return true;
}

bool Scene::initScene(int32 index) {
	// load scene from file
	_currentSceneSize = HQR::getAllocEntry(&currentScene, Resources::HQR_SCENE_FILE, index);
	if (_currentSceneSize == 0) {
		return false;
	}

	if (_engine->isLBA1()) {
		return loadSceneLBA1();
	}

	return false;
}

void Scene::resetScene() {
	_engine->_extra->resetExtras();

	for (int32 i = 0; i < ARRAYSIZE(sceneFlags); i++) {
		sceneFlags[i] = 0;
	}

	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		_engine->_redraw->overlayList[i].info0 = -1;
	}

	_engine->_actor->currentPositionInBodyPtrTab = 0;
	_engine->_screens->useAlternatePalette = false;
}

void Scene::changeScene() {
	// change twinsen house destroyed hard-coded
	if (needChangeScene == LBA1SceneId::Citadel_Island_near_twinsens_house && _engine->_gameState->gameFlags[30] != 0) {
		needChangeScene = LBA1SceneId::Citadel_Island_Twinsens_house_destroyed;
	}

	// local backup previous scene
	previousSceneIdx = currentSceneIdx;
	currentSceneIdx = needChangeScene;

	_engine->_sound->stopSamples();

	resetScene();
	_engine->_actor->loadHeroEntities();

	sceneHero->controlMode = ControlMode::kManual;
	sceneHero->zone = -1;
	sceneHero->positionInLifeScript = 0;
	sceneHero->positionInMoveScript = -1;
	sceneHero->labelIdx = -1;

	initScene(needChangeScene);

	// TODO: treat holomap trajectories

	if (needChangeScene == LBA1SceneId::Citadel_Island_end_sequence_1 || needChangeScene == LBA1SceneId::Citadel_Island_end_sequence_2) {
		sceneTextBank = TextBankId::Tippet_Island;
	}

	_engine->_text->initTextBank(sceneTextBank + 3);
	_engine->_grid->initGrid(needChangeScene);

	if (heroPositionType == ScenePositionType::kZone) {
		newHeroX = zoneHeroX;
		newHeroY = zoneHeroY;
		newHeroZ = zoneHeroZ;
	}

	if (heroPositionType == ScenePositionType::kScene || heroPositionType == ScenePositionType::kNoPosition) {
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
		_engine->autoSave();
	}

	_engine->_actor->restartHeroScene();

	for (int32 a = 1; a < sceneNumActors; a++) {
		_engine->_actor->initActor(a);
	}

	_engine->_gameState->inventoryNumKeys = 0;
	_engine->disableScreenRecenter = false;
	heroPositionType = ScenePositionType::kNoPosition;
	sampleAmbienceTime = 0;

	_engine->_grid->newCameraX = _sceneActors[currentlyFollowedActor].x >> 9;
	_engine->_grid->newCameraY = _sceneActors[currentlyFollowedActor].y >> 8;
	_engine->_grid->newCameraZ = _sceneActors[currentlyFollowedActor].z >> 9;

	_engine->_gameState->magicBallIdx = -1;
	_engine->_movements->heroMoved = true;
	_engine->_grid->useCellingGrid = -1;
	_engine->_grid->cellingGridIdx = -1;
	_engine->_redraw->reqBgRedraw = true;
	_engine->_screens->lockPalette = false;

	needChangeScene = -1;
	changeRoomVar10 = 1;
	changeRoomVar11 = 14;

	_engine->_renderer->setLightVector(alphaLight, betaLight, 0);

	if (sceneMusic != -1) {
		_engine->_music->playTrackMusic(sceneMusic);
	}
}

ActorStruct *Scene::getActor(int32 actorIdx) {
	assert(actorIdx >= 0);
	assert(actorIdx < NUM_MAX_ACTORS);
	return &_sceneActors[actorIdx];
}

void Scene::initSceneVars() {
	sampleAmbiance[0] = -1;
	sampleAmbiance[1] = -1;
	sampleAmbiance[2] = -1;
	sampleAmbiance[3] = -1;

	sampleRepeat[0] = 0;
	sampleRepeat[1] = 0;
	sampleRepeat[2] = 0;
	sampleRepeat[3] = 0;

	sampleRound[0] = 0;
	sampleRound[1] = 0;
	sampleRound[2] = 0;
	sampleRound[3] = 0;

	sceneNumActors = 0;
	sceneNumZones = 0;
	sceneNumTracks = 0;
}

void Scene::processEnvironmentSound() {
	if (_engine->lbaTime >= sampleAmbienceTime) {
		int16 currentAmb = _engine->getRandomNumber(4); // random ambiance

		for (int32 s = 0; s < 4; s++) {
			if (!(samplePlayed & (1 << currentAmb))) { // if not already played
				samplePlayed |= (1 << currentAmb);     // make sample played

				if (samplePlayed == 15) { // reset if all samples played
					samplePlayed = 0;
				}

				int16 sampleIdx = sampleAmbiance[currentAmb];
				if (sampleIdx != -1) {
					int16 decal = sampleRound[currentAmb];
					int16 repeat = sampleRepeat[currentAmb];

					_engine->_sound->playSample(sampleIdx, (4096 + _engine->getRandomNumber(decal) - (decal / 2)), repeat, 110, -1, 110);
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
	// bonus not used yet
	if (zone->infoData.Bonus.used) {
		return;
	}
	int8 bonusTable[8];
	int32 numBonus = 0;
	for (int32 a = 0; a < 5; a++) {
		if (zone->infoData.Bonus.typesFlag & (1 << (a + 4))) {
			bonusTable[numBonus++] = a;
		}
	}

	if (numBonus) {
		int8 currentBonus = bonusTable[_engine->getRandomNumber(numBonus)];

		// if bonus is magic an no magic level yet, then give life points
		if (!_engine->_gameState->magicLevelIdx && currentBonus == 2) {
			currentBonus = 1;
		}

		const int16 amount = zone->infoData.Bonus.amount;
		const int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(ABS(zone->topRight.x + zone->bottomLeft.x) / 2, ABS(zone->topRight.z + zone->bottomLeft.z) / 2, sceneHero->x, sceneHero->z);
		const int32 index = _engine->_extra->addExtraBonus(ABS(zone->topRight.x + zone->bottomLeft.x) / 2, zone->topRight.y, ABS(zone->topRight.z + zone->bottomLeft.z) / 2, 180, angle, currentBonus + 3, amount);

		if (index != -1) {
			_engine->_extra->extraList[index].type |= 0x400;
			zone->infoData.Bonus.used = 1; // set as used
		}
	}
}

void Scene::processActorZones(int32 actorIdx) {
	ActorStruct *actor = &_sceneActors[actorIdx];

	int32 currentX = actor->x;
	int32 currentY = actor->y;
	int32 currentZ = actor->z;

	actor->zone = -1;
	int32 tmpCellingGrid = 0;

	if (IS_HERO(actorIdx)) {
		currentActorInZone = false;
	}

	for (int32 z = 0; z < sceneNumZones; z++) {
		ZoneStruct *zone = &sceneZones[z];

		// check if actor is in zone
		if ((currentX >= zone->bottomLeft.x && currentX <= zone->topRight.x) &&
		    (currentY >= zone->bottomLeft.y && currentY <= zone->topRight.y) &&
		    (currentZ >= zone->bottomLeft.z && currentZ <= zone->topRight.z)) {
			switch (zone->type) {
			case kCube:
				if (IS_HERO(actorIdx) && actor->life > 0) {
					needChangeScene = zone->infoData.ChangeScene.newSceneIdx;
					zoneHeroX = actor->x - zone->bottomLeft.x + zone->infoData.ChangeScene.x;
					zoneHeroY = actor->y - zone->bottomLeft.y + zone->infoData.ChangeScene.y;
					zoneHeroZ = actor->z - zone->bottomLeft.z + zone->infoData.ChangeScene.z;
					heroPositionType = ScenePositionType::kZone;
				}
				break;
			case kCamera:
				if (currentlyFollowedActor == actorIdx) {
					_engine->disableScreenRecenter = true;
					if (_engine->_grid->newCameraX != zone->infoData.CameraView.x || _engine->_grid->newCameraY != zone->infoData.CameraView.y || _engine->_grid->newCameraZ != zone->infoData.CameraView.z) {
						_engine->_grid->newCameraX = zone->infoData.CameraView.x;
						_engine->_grid->newCameraY = zone->infoData.CameraView.y;
						_engine->_grid->newCameraZ = zone->infoData.CameraView.z;
						_engine->_redraw->reqBgRedraw = true;
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
						ScopedEngineFreeze freeze(_engine);
						_engine->_grid->initCellingGrid(_engine->_grid->useCellingGrid);
					}
				}
				break;
			case kObject:
				if (IS_HERO(actorIdx) && _engine->_movements->heroAction) {
					_engine->_animations->initAnim(kAction, 1, 0, 0);
					processZoneExtraBonus(zone);
				}
				break;
			case kText:
				if (IS_HERO(actorIdx) && _engine->_movements->heroAction) {
					_engine->freezeTime();
					_engine->_text->setFontCrossColor(zone->infoData.DisplayText.textColor);
					talkingActor = actorIdx;
					_engine->_text->drawTextFullscreen(zone->infoData.DisplayText.textIdx);
					_engine->unfreezeTime();
					_engine->_redraw->redrawEngineActions(1);
				}
				break;
			case kLadder:
				if (IS_HERO(actorIdx) && _engine->_actor->heroBehaviour != kProtoPack && (actor->anim == kForward || actor->anim == kTopLadder || actor->anim == kClimbLadder)) {
					_engine->_movements->rotateActor(actor->boudingBox.x.bottomLeft, actor->boudingBox.z.bottomLeft, actor->angle + 0x580);
					_engine->_renderer->destX += _engine->_movements->processActorX;
					_engine->_renderer->destZ += _engine->_movements->processActorZ;

					if (_engine->_renderer->destX >= 0 && _engine->_renderer->destZ >= 0 && _engine->_renderer->destX <= 0x7E00 && _engine->_renderer->destZ <= 0x7E00) {
						if (_engine->_grid->getBrickShape(_engine->_renderer->destX, actor->y + 0x100, _engine->_renderer->destZ)) {
							currentActorInZone = true;
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
		_engine->_redraw->reqBgRedraw = true;
	}
}

} // namespace TwinE
