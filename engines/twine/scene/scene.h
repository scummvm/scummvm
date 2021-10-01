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

/**
 * Special actions, like change scene, climbing a ladder, ...
 */
struct ZoneStruct {
	IVec3 mins;
	IVec3 maxs;
	ZoneType type = ZoneType::kCube;
	int16 snap = 0;
	union {
		struct {
			int32 newSceneIdx;
			int32 x;
			int32 y;
			int32 z;
		} ChangeScene;
		struct {
			int32 dummy;
			int32 x;
			int32 y;
			int32 z;
		} CameraView;
		struct {
			int32 zoneIdx;
		} Sceneric;
		struct {
			int32 newGrid;
		} CeillingGrid;

		/** show a text (e.g. when reading a sign) */
		struct {
			TextId textIdx;   /*!< text index in the current active text bank */
			int32 textColor; /*!< text color (see @c ActorStruct::talkColor) */
		} DisplayText;
		struct {
			int32 info0;
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
	ActorStruct _sceneActors[NUM_MAX_ACTORS];
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

	int16 _sceneMusic = 0;

	IVec3 _sceneHeroPos;
	IVec3 _zoneHeroPos;

	int32 _currentGameOverScene = 0;

	uint8 *_currentScene = nullptr;

public:
	Scene(TwinEEngine *engine) : _engine(engine) {}
	~Scene();

	int32 _needChangeScene = LBA1SceneId::Citadel_Island_Prison;
	int32 _currentSceneIdx = LBA1SceneId::Citadel_Island_Prison;
	int32 _previousSceneIdx = LBA1SceneId::Citadel_Island_Prison;

	int32 _holomapTrajectory = -1;

	TextBankId _sceneTextBank = TextBankId::None;
	int32 _alphaLight = ANGLE_0;
	int32 _betaLight = ANGLE_0;

	IVec3 _newHeroPos;

	/** Hero Y coordinate before fall */
	int16 _heroYBeforeFall = 0;

	/** Hero type of position in scene */
	ScenePositionType _heroPositionType = ScenePositionType::kNoPosition; // twinsenPositionModeInNewCube

	// ACTORS
	int32 _sceneNumActors = 0;
	ActorStruct *_sceneHero = nullptr;

	/** Meca penguin actor index */
	int16 _mecaPenguinIdx = 0;

	/** Current followed actor in scene */
	int16 _currentlyFollowedActor = OWN_ACTOR_SCENE_INDEX;
	/** Current actor in zone - climbing a ladder */
	bool _currentActorInZone = false;
	bool _useScenePatches = true;
	/** Current actor manipulated in scripts */
	int16 _currentScriptValue = 0;

	int16 _talkingActor = 0;

	// TRACKS Tell the actor where to go
	int32 _sceneNumTracks = 0;
	IVec3 _sceneTracks[NUM_MAX_TRACKS];

	bool _enableGridTileRendering = true;

	uint8 _sceneFlags[NUM_SCENES_FLAGS]{0};

	int32 _sceneNumZones = 0;
	ZoneStruct _sceneZones[NUM_MAX_ZONES];

	ActorStruct *getActor(int32 actorIdx);

	void playSceneMusic();

	void reloadCurrentScene();

	/** Change to another scene */
	void changeScene();

	/** Process scene environment sound */
	void processEnvironmentSound();
	void initSceneVars();

	bool isGameRunning() const;
	void stopRunningGame();

	/**
	 * Process actor zones
	 * @param actorIdx Process actor index
	 */
	void processActorZones(int32 actorIdx);
};

inline bool Scene::isGameRunning() const {
	return _currentScene != nullptr;
}

} // namespace TwinE

#endif
