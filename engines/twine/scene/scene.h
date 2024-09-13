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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TWINE_SCENE_SCENE_H
#define TWINE_SCENE_SCENE_H

#include "common/scummsys.h"
#include "common/util.h"
#include "twine/scene/actor.h"
#include "twine/shared.h"

namespace TwinE {

#define NUM_SCENES_FLAGS 80

#define NUM_MAX_ACTORS 100
#define NUM_MAX_ZONES 100
#define NUM_MAX_TRACKS 200

enum class ScenePositionType {
	kNoPosition = 0,
	kZone = 1,
	kScene = 2,
	kReborn = 3
};

// ZONES

#define ZONE_INIT_ON 1
#define ZONE_ON 2
#define ZONE_ACTIVE 4
#define ZONE_OBLIGATOIRE 8

/**
 * Special actions, like change scene, climbing a ladder, ...
 */
struct ZoneStruct {
	IVec3 mins;
	IVec3 maxs;
	ZoneType type = ZoneType::kCube;
	int32 num;
	union {
		struct {
			int32 x;
			int32 y;
			int32 z;
		} ChangeScene;
		struct {
			int32 x;
			int32 y;
			int32 z;
		} CameraView;
		/** show a text (e.g. when reading a sign) */
		struct {
			int32 textColor; /*!< text color (see @c ActorStruct::talkColor) */
		} DisplayText;
		struct {
			BonusParameter typesFlag;
			int32 amount;
			/**
			 * Already used
			 */
			int32 used;
		} Bonus;
		struct {
			int32 info0;
			int32 info1;
			int32 info2;
			int32 info3;
			int32 info4;
			int32 info5;
			int32 info6;
			int32 info7;
		} generic;
	} infoData;
};

class TwinEEngine;

/**
 * scene 0: 23 actors
 *
 * scene 1: 14 actors
 * actor 1 - car
 * actor 2 - elephant
 * actor 3 - soldier at the house
 * actor 4 - patrolling soldier before gate
 * actor 5 - soldier after gate
 * actor 6 - ??
 * actor 7 - ??
 * actor 8 - left gate
 * actor 9 - ??
 * actor 10 - door after leaving truck
 * actor 11 - door subway
 * actor 12 - guy at rubbish
 * actor 13 - ??
 */
class Scene {
private:
	TwinEEngine *_engine;

	void loadModel(ActorStruct &actor, int32 modelIndex, bool lba1);

	/** Process zone extra bonus */
	void processZoneExtraBonus(ZoneStruct *zone);
	void setActorStaticFlags(ActorStruct *act, uint32 staticFlags);
	void setBonusParameterFlags(ActorStruct *act, uint16 bonusFlags);
	bool loadSceneLBA1();
	bool loadSceneLBA2();

	/** Initialize new scene */
	bool initScene(int32 index);
	/** Reset scene */
	void resetScene();

	// the first actor is the own hero
	ActorStruct _sceneActors[NUM_MAX_ACTORS]; // ListObjet
	int32 _currentSceneSize = 0;
	bool _isOutsideScene = false; // lba2

	/** Timer for the next sample ambience in scene */
	int32 _sampleAmbienceTime = 0;
	int16 _sampleAmbiance[4]{0};
	int16 _sampleRepeat[4]{0};
	int16 _sampleRound[4]{0};
	int16 _sampleFrequency[4]{0}; // lba2
	int16 _sampleVolume[4]{0}; // lba2
	int16 _sampleMinDelay = 0;
	int16 _sampleMinDelayRnd = 0;

	int16 _samplePlayed = 0;

public:
	int16 _cubeJingle = 0;
private:
	IVec3 _sceneHeroPos;
	IVec3 _zoneHeroPos;

	int32 _currentGameOverScene = 0;

	uint8 *_currentScene = nullptr;
	void dumpSceneScripts() const;
	void dumpSceneScript(const char *type, int actorIdx, const uint8* script, int size) const;
public:
	Scene(TwinEEngine *engine) : _engine(engine) {}
	~Scene();

	int32 _needChangeScene = LBA1SceneId::Citadel_Island_Prison;
	int32 _currentSceneIdx = LBA1SceneId::Citadel_Island_Prison; // NumCube
	int32 _previousSceneIdx = LBA1SceneId::Citadel_Island_Prison;

	int32 _planet = -1;

	int32 _holomapTrajectory = -1;

	TextBankId _sceneTextBank = TextBankId::None;
	int32 _alphaLight = 0;
	int32 _betaLight = 0;

	uint8 _island = 0;
	uint8 _shadowLevel = 0; // lba2
	uint8 _modeLabyrinthe = 0; // lba2
	uint8 _currentCubeX = 0; // lba2
	uint8 _currentCubeY = 0; // lba2

	IVec3 _newHeroPos;

	/** Hero Y coordinate before fall */
	int16 _startYFalling = 0;

	/** Hero type of position in scene */
	ScenePositionType _heroPositionType = ScenePositionType::kNoPosition; // twinsenPositionModeInNewCube

	// ACTORS
	int32 _nbObjets = 0;
	ActorStruct *_sceneHero = nullptr;

	/** Meca penguin actor index */
	int16 _mecaPenguinIdx = 0;

	/** Current followed actor in scene */
	int16 _currentlyFollowedActor = OWN_ACTOR_SCENE_INDEX;
	/** Current actor in zone - climbing a ladder */
	bool _flagClimbing = false;
	bool _enableEnhancements = false;
	/** Current actor manipulated in scripts */
	int16 _currentScriptValue = 0;

	int16 _talkingActor = 0;

	// TRACKS Tell the actor where to go
	int32 _sceneNumTracks = 0;
	IVec3 _sceneTracks[NUM_MAX_TRACKS];

	bool _enableGridTileRendering = true;

	uint8 _listFlagCube[NUM_SCENES_FLAGS]{0}; // ListVarCube

	int32 _sceneNumZones = 0; // NbZones
	ZoneStruct _sceneZones[NUM_MAX_ZONES]; // ListZone

	ActorStruct *getActor(int32 actorIdx); // ListObjet

	void playSceneMusic();

	void reloadCurrentScene();

	/** Change to another scene */
	void changeCube();

	/** For the buggy to get the 2D coordinates of an exterior cube in the map */
	bool loadSceneCubeXY(int sceneNum, int32 *cubeX, int32 *cubeY);

	/** Process scene environment sound */
	void processEnvironmentSound();
	void initSceneVars();

	bool isGameRunning() const;
	void stopRunningGame();

	/**
	 * Process actor zones
	 * @param actorIdx Process actor index
	 */
	void checkZoneSce(int32 actorIdx);
};

inline bool Scene::isGameRunning() const {
	return _currentScene != nullptr;
}

} // namespace TwinE

#endif
