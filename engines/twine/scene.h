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

#ifndef TWINE_SCENE_H
#define TWINE_SCENE_H

#include "common/scummsys.h"
#include "twine/actor.h"

namespace TwinE {

#define NUM_SCENES_FLAGS 80

#define NUM_SCENES_ENTRIES 120
#define NUM_SCENES NUM_SCENES_ENTRIES - 1

#define NUM_MAX_ACTORS 100
#define NUM_MAX_ZONES 100
#define NUM_MAX_TRACKS 200

enum ScenePositionType {
	kNoPosition = 0,
	kZone = 1,
	kScene = 2,
	kReborn = 3
};

// ZONES

typedef struct ScenePoint {
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;
} ScenePoint;

typedef struct ZoneStruct {
	ScenePoint bottomLeft;
	ScenePoint topRight;
	int16 type = 0;
	union {
		struct {
			int16 newSceneIdx;
			int16 x;
			int16 y;
			int16 z;
		} ChangeScene;
		struct {
			int16 dummy;
			int16 x;
			int16 y;
			int16 z;
		} CameraView;
		struct {
			int16 zoneIdx;
		} Sceneric;
		struct {
			int16 newGrid;
		} CeillingGrid;
		struct {
			int16 textIdx;
			int16 textColor;
		} DisplayText;
		struct {
			int16 info0;
			int16 info1;
			int16 info2;
			int16 info3;
		} generic;
	} infoData;
	int16 snap = 0;
} ZoneStruct;

enum ZoneType {
	kCube = 0,     // Change to another scene
	kCamera = 1,   // Binds camera view
	kSceneric = 2, // For use in Life Script
	kGrid = 3,     // Set disappearing Grid fragment
	kObject = 4,   // Give bonus
	kText = 5,     // Displays text message
	kLadder = 6    // Hero can climb on it
};

class TwinEEngine;
class Scene {
private:
	TwinEEngine *_engine;

/** Process zone extra bonus */
	void processZoneExtraBonus(ZoneStruct *zone);
	void setActorStaticFlags(int32 actorIdx, uint16 staticFlags);
	void loadScene();
	/** Initialize new scene */
	int32 initScene(int32 index);
	/** Reset scene */
	void resetScene();

public:
	Scene(TwinEEngine *engine) : _engine(engine) {}

	uint8 *currentScene = nullptr;

	int32 needChangeScene = 0;
	int32 currentSceneIdx = 0;
	int32 previousSceneIdx = 0;

	uint8 *spriteShadowPtr = nullptr;
	uint8 *spriteBoundingBoxPtr = nullptr;

	int32 currentGameOverScene = 0;
	int32 alphaLight = 0;
	int32 betaLight = 0;

	/** Timer for the next sample ambience in scene */
	int32 sampleAmbienceTime = 0;

	int16 sampleAmbiance[4] {0};
	int16 sampleRepeat[4] {0};
	int16 sampleRound[4] {0};
	int16 sampleMinDelay = 0;
	int16 sampleMinDelayRnd = 0;

	int16 samplePlayed = 0;

	int16 sceneMusic = 0;

	int16 sceneHeroX = 0; // newTwinsenXByScene
	int16 sceneHeroY = 0; // newTwinsenYByScene
	int16 sceneHeroZ = 0; // newTwinsenZByScene

	int16 newHeroX = 0; // newTwinsenX
	int16 newHeroY = 0; // newTwinsenY
	int16 newHeroZ = 0; // newTwinsenZ

	int16 zoneHeroX = 0; // newTwinsenXByZone
	int16 zoneHeroY = 0; // newTwinsenYByZone
	int16 zoneHeroZ = 0; // newTwinsenZByZone

	/** Hero Y coordinate before fall */
	int16 heroYBeforeFall = 0;

	/** Hero type of position in scene */
	int16 heroPositionType = 0; // twinsenPositionModeInNewCube

	// ACTORS
	int32 sceneNumActors = 0;
	// the first actor is the own hero
	ActorStruct sceneActors[NUM_MAX_ACTORS];
	ActorStruct *sceneHero = nullptr;

	/** Meca pinguin actor index */
	int16 mecaPinguinIdx = 0; // currentPingouin

	/** Current followed actor in scene */
	int16 currentlyFollowedActor = 0;
	/** Current actor in zone */
	int16 currentActorInZone = 0; // currentActorInZoneProcess
	/** Current actor manipulated in scripts */
	int16 currentScriptValue = 0; // manipActorResult

	int16 talkingActor = 0;

	// TRACKS

	int32 sceneNumTracks = 0;
	ScenePoint sceneTracks[NUM_MAX_TRACKS];

	// TODO: check what is this
	int16 changeRoomVar10 = 0;
	int16 changeRoomVar11 = 0;

	uint8 sceneFlags[80] {0}; // cubeFlags

	int32 sceneNumZones = 0;
	ZoneStruct sceneZones[NUM_MAX_ZONES];

	/** Change to another scene */
	void changeScene();

	/** Process scene environment sound */
	void processEnvironmentSound();

	/**
	 * Process actor zones
	 * @param actorIdx Process actor index
	 */
	void processActorZones(int32 actorIdx);
};

} // namespace TwinE

#endif
