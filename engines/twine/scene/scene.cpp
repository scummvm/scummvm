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

#include "twine/scene/scene.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/util.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/debugger/debug_grid.h"
#include "twine/debugger/debug_scene.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/extra.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

Scene::~Scene() {
	free(currentScene);
}

void Scene::setActorStaticFlags(ActorStruct *act, uint16 staticFlags) {
	if (staticFlags & 0x1) {
		act->staticFlags.bComputeCollisionWithObj = 1;
	}
	if (staticFlags & 0x2) {
		act->staticFlags.bComputeCollisionWithBricks = 1;
	}
	if (staticFlags & 0x4) {
		act->staticFlags.bIsZonable = 1;
	}
	if (staticFlags & 0x8) {
		act->staticFlags.bUsesClipping = 1;
	}
	if (staticFlags & 0x10) {
		act->staticFlags.bCanBePushed = 1;
	}
	if (staticFlags & 0x20) {
		act->staticFlags.bComputeLowCollision = 1;
	}
	if (staticFlags & 0x40) {
		act->staticFlags.bCanDrown = 1;
	}
	if (staticFlags & 0x80) {
		act->staticFlags.bComputeCollisionWithFloor = 1;
	}

	if (staticFlags & 0x100) {
		act->staticFlags.bUnk0100 = 1;
	}
	if (staticFlags & 0x200) {
		act->staticFlags.bIsHidden = 1;
	}
	if (staticFlags & 0x400) {
		act->staticFlags.bIsSpriteActor = 1;
	}
	if (staticFlags & 0x800) {
		act->staticFlags.bCanFall = 1;
	}
	if (staticFlags & 0x1000) {
		act->staticFlags.bDoesntCastShadow = 1;
	}
	if (staticFlags & 0x2000) {
		//sceneActors[actorIdx].staticFlags.bIsBackgrounded = 1;
	}
	if (staticFlags & 0x4000) {
		act->staticFlags.bIsCarrierActor = 1;
	}
	if (staticFlags & 0x8000) {
		act->staticFlags.bUseMiniZv = 1;
	}
}

void Scene::setBonusParameterFlags(ActorStruct *act, uint16 bonusFlags) {
	if (bonusFlags & 0x1) {
		act->bonusParameter.unk1 = 1;
	}
	if (bonusFlags & 0x2) {
		act->bonusParameter.unk2 = 1;
	}
	if (bonusFlags & 0x4) {
		act->bonusParameter.unk3 = 1;
	}
	if (bonusFlags & 0x8) {
		act->bonusParameter.unk4 = 1;
	}
	if (bonusFlags & 0x10) {
		act->bonusParameter.kashes = 1;
	}
	if (bonusFlags & 0x20) {
		act->bonusParameter.lifepoints = 1;
	}
	if (bonusFlags & 0x40) {
		act->bonusParameter.magicpoints = 1;
	}
	if (bonusFlags & 0x80) {
		act->bonusParameter.key = 1;
	}
	if (bonusFlags & 0x100) {
		act->bonusParameter.cloverleaf = 1;
	}
}

bool Scene::loadSceneLBA1() {
	Common::MemoryReadStream stream(currentScene, _currentSceneSize);

	// load scene ambience properties
	sceneTextBank = stream.readByte();
	_currentGameOverScene = stream.readByte();
	stream.skip(4);

	alphaLight = stream.readUint16LE();
	betaLight = stream.readUint16LE();

	// FIXME: Workaround to fix lighting issue - not using proper dark light
	alphaLight = 896;
	betaLight = 950;

	_sampleAmbiance[0] = stream.readUint16LE();
	_sampleRepeat[0] = stream.readUint16LE();
	_sampleRound[0] = stream.readUint16LE();

	_sampleAmbiance[1] = stream.readUint16LE();
	_sampleRepeat[1] = stream.readUint16LE();
	_sampleRound[1] = stream.readUint16LE();

	_sampleAmbiance[2] = stream.readUint16LE();
	_sampleRepeat[2] = stream.readUint16LE();
	_sampleRound[2] = stream.readUint16LE();

	_sampleAmbiance[3] = stream.readUint16LE();
	_sampleRepeat[3] = stream.readUint16LE();
	_sampleRound[3] = stream.readUint16LE();

	_sampleMinDelay = stream.readUint16LE();
	_sampleMinDelayRnd = stream.readUint16LE();

	_sceneMusic = stream.readByte();

	// load hero properties
	_sceneHeroX = stream.readUint16LE();
	_sceneHeroY = stream.readUint16LE();
	_sceneHeroZ = stream.readUint16LE();

	sceneHero->moveScriptSize = stream.readUint16LE();
	sceneHero->moveScript = currentScene + stream.pos();
	stream.skip(sceneHero->moveScriptSize);

	sceneHero->lifeScriptSize = stream.readUint16LE();
	sceneHero->lifeScript = currentScene + stream.pos();
	stream.skip(sceneHero->lifeScriptSize);

	sceneNumActors = stream.readUint16LE();
	int cnt = 1;
	for (int32 i = 1; i < sceneNumActors; i++, cnt++) {
		_engine->_actor->resetActor(i);

		ActorStruct *act = &_sceneActors[i];
		setActorStaticFlags(act, stream.readUint16LE());

		act->loadModel(stream.readUint16LE());

		act->body = stream.readByte();
		act->anim = (AnimationTypes)stream.readByte();
		act->sprite = stream.readUint16LE();
		act->x = stream.readUint16LE();
		act->collisionX = act->x;
		act->y = stream.readUint16LE();
		act->collisionY = act->y;
		act->z = stream.readUint16LE();
		act->collisionZ = act->z;
		act->strengthOfHit = stream.readByte();
		setBonusParameterFlags(act, stream.readUint16LE());
		act->angle = stream.readUint16LE();
		act->speed = stream.readUint16LE();
		act->controlMode = (ControlMode)stream.readUint16LE();
		act->cropLeft = stream.readSint16LE();
		act->delayInMillis = act->cropLeft; // TODO: this might not be needed
		act->cropTop = stream.readSint16LE();
		act->cropRight = stream.readSint16LE();
		act->cropBottom = stream.readSint16LE();
		act->followedActor = act->cropBottom; // TODO: is this needed? and valid?
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

		if (_engine->_debugScene->onlyLoadActor != -1 && _engine->_debugScene->onlyLoadActor != cnt) {
			sceneNumActors--;
			i--;
		}
	}

	sceneNumZones = stream.readUint16LE();
	for (int32 i = 0; i < sceneNumZones; i++) {
		ZoneStruct *zone = &sceneZones[i];
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
		ScenePoint *point = &sceneTracks[i];
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

	_engine->_actor->clearBodyTable();
	_engine->_screens->useAlternatePalette = false;
}

void Scene::changeScene() {
	// change twinsen house destroyed hard-coded
	if (needChangeScene == LBA1SceneId::Citadel_Island_near_twinsens_house && _engine->_gameState->hasOpenedFunfrocksSafe()) {
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

	_engine->_text->initSceneTextBank();
	_engine->_grid->initGrid(needChangeScene);

	if (heroPositionType == ScenePositionType::kZone) {
		newHeroX = _zoneHeroX;
		newHeroY = _zoneHeroY;
		newHeroZ = _zoneHeroZ;
	}

	if (heroPositionType == ScenePositionType::kScene || heroPositionType == ScenePositionType::kNoPosition) {
		newHeroX = _sceneHeroX;
		newHeroY = _sceneHeroY;
		newHeroZ = _sceneHeroZ;
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
	_sampleAmbienceTime = 0;

	ActorStruct *followedActor = getActor(currentlyFollowedActor);
	_engine->_grid->newCameraX = followedActor->x / BRICK_SIZE;
	_engine->_grid->newCameraY = followedActor->y / BRICK_HEIGHT;
	_engine->_grid->newCameraZ = followedActor->z / BRICK_SIZE;

	_engine->_gameState->magicBallIdx = -1;
	_engine->_movements->heroMoved = true;
	_engine->_grid->useCellingGrid = -1;
	_engine->_grid->cellingGridIdx = -1;
	_engine->_redraw->reqBgRedraw = true;
	_engine->_screens->lockPalette = false;

	needChangeScene = -1;
	changeRoomVar10 = true;

	_engine->_renderer->setLightVector(alphaLight, betaLight, 0);

	if (_sceneMusic != -1) {
		_engine->_music->playTrackMusic(_sceneMusic);
	}
}

ActorStruct *Scene::getActor(int32 actorIdx) {
	assert(actorIdx >= 0);
	assert(actorIdx < NUM_MAX_ACTORS);
	return &_sceneActors[actorIdx];
}

void Scene::initSceneVars() {
	_sampleAmbiance[0] = -1;
	_sampleAmbiance[1] = -1;
	_sampleAmbiance[2] = -1;
	_sampleAmbiance[3] = -1;

	_sampleRepeat[0] = 0;
	_sampleRepeat[1] = 0;
	_sampleRepeat[2] = 0;
	_sampleRepeat[3] = 0;

	_sampleRound[0] = 0;
	_sampleRound[1] = 0;
	_sampleRound[2] = 0;
	_sampleRound[3] = 0;

	sceneNumActors = 0;
	sceneNumZones = 0;
	sceneNumTracks = 0;
}

void Scene::processEnvironmentSound() {
	if (_engine->lbaTime >= _sampleAmbienceTime) {
		int16 currentAmb = _engine->getRandomNumber(4); // random ambiance

		for (int32 s = 0; s < 4; s++) {
			if (!(_samplePlayed & (1 << currentAmb))) { // if not already played
				_samplePlayed |= (1 << currentAmb);     // make sample played

				if (_samplePlayed == 15) { // reset if all samples played
					_samplePlayed = 0;
				}

				const int16 sampleIdx = _sampleAmbiance[currentAmb];
				if (sampleIdx != -1) {
					/*int16 decal = _sampleRound[currentAmb];*/
					int16 repeat = _sampleRepeat[currentAmb];

					_engine->_sound->playSample(sampleIdx, repeat, 110, -1, 110);
					break;
				}
			}

			currentAmb++;    // try next ambiance
			currentAmb &= 3; // loop in all 4 ambiances
		}

		// compute next ambiance timer
		_sampleAmbienceTime = _engine->lbaTime + (_engine->getRandomNumber(_sampleMinDelayRnd) + _sampleMinDelay) * 50;
	}
}

void Scene::processZoneExtraBonus(ZoneStruct *zone) {
	if (zone->infoData.Bonus.used) {
		return;
	}

	const int bonusSprite = _engine->_extra->getBonusSprite(zone->infoData.Bonus.typesFlag);
	if (bonusSprite == -1) {
		return;
	}

	const int16 amount = zone->infoData.Bonus.amount;
	const int32 angle = _engine->_movements->getAngleAndSetTargetActorDistance(ABS(zone->topRight.x + zone->bottomLeft.x) / 2, ABS(zone->topRight.z + zone->bottomLeft.z) / 2, sceneHero->x, sceneHero->z);
	const int32 index = _engine->_extra->addExtraBonus(ABS(zone->topRight.x + zone->bottomLeft.x) / 2, zone->topRight.y, ABS(zone->topRight.z + zone->bottomLeft.z) / 2, ANGLE_63, angle, bonusSprite, amount);

	if (index != -1) {
		_engine->_extra->extraList[index].type |= 0x400;
		zone->infoData.Bonus.used = 1; // set as used
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
					_zoneHeroX = actor->x - zone->bottomLeft.x + zone->infoData.ChangeScene.x;
					_zoneHeroY = actor->y - zone->bottomLeft.y + zone->infoData.ChangeScene.y;
					_zoneHeroZ = actor->z - zone->bottomLeft.z + zone->infoData.ChangeScene.z;
					heroPositionType = ScenePositionType::kZone;
				}
				break;
			case kCamera:
				if (currentlyFollowedActor == actorIdx && !_engine->_debugGrid->useFreeCamera) {
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
				if (IS_HERO(actorIdx) && _engine->_movements->shouldTriggerZoneAction()) {
					_engine->_animations->initAnim(AnimationTypes::kAction, 1, AnimationTypes::kStanding, 0);
					processZoneExtraBonus(zone);
				}
				break;
			case kText:
				if (IS_HERO(actorIdx) && _engine->_movements->shouldTriggerZoneAction()) {
					_engine->freezeTime();
					_engine->_text->setFontCrossColor(zone->infoData.DisplayText.textColor);
					talkingActor = actorIdx;
					_engine->_text->drawTextFullscreen(zone->infoData.DisplayText.textIdx);
					_engine->unfreezeTime();
					_engine->_redraw->redrawEngineActions(true);
				}
				break;
			case kLadder:
				if (IS_HERO(actorIdx) && _engine->_actor->heroBehaviour != HeroBehaviourType::kProtoPack && (actor->anim == AnimationTypes::kForward || actor->anim == AnimationTypes::kTopLadder || actor->anim == AnimationTypes::kClimbLadder)) {
					_engine->_movements->rotateActor(actor->boudingBox.x.bottomLeft, actor->boudingBox.z.bottomLeft, actor->angle + ANGLE_360 + ANGLE_135);
					_engine->_renderer->destX += _engine->_movements->processActorX;
					_engine->_renderer->destZ += _engine->_movements->processActorZ;

					if (_engine->_renderer->destX >= 0 && _engine->_renderer->destZ >= 0 && _engine->_renderer->destX <= 0x7E00 && _engine->_renderer->destZ <= 0x7E00) {
						if (_engine->_grid->getBrickShape(_engine->_renderer->destX, actor->y + ANGLE_90, _engine->_renderer->destZ) != ShapeType::kNone) {
							currentActorInZone = true;
							if (actor->y >= ABS(zone->bottomLeft.y + zone->topRight.y) / 2) {
								_engine->_animations->initAnim(AnimationTypes::kTopLadder, 2, AnimationTypes::kStanding, actorIdx); // reached end of ladder
							} else {
								_engine->_animations->initAnim(AnimationTypes::kClimbLadder, 0, AnimationTypes::kAnimInvalid, actorIdx); // go up in ladder
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

void Scene::stopRunningGame() {
	free(currentScene);
	currentScene = nullptr;
}

} // namespace TwinE
