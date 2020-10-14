/** @file scene.h
	@brief
	This file contains main scenario routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SCENE_H
#define SCENE_H

#include "sys.h"
#include "actor.h"

#define NUM_SCENES_FLAGS	80

#define NUM_SCENES_ENTRIES	120
#define NUM_SCENES	NUM_SCENES_ENTRIES-1

#define NUM_MAX_ACTORS		100
#define NUM_MAX_ZONES		100
#define NUM_MAX_TRACKS		200

enum ScenePositionType {
	kNoPosition	= 0,
	kZone		= 1,
	kScene		= 2,
	kReborn		= 3
};


int32 needChangeScene;
int32 currentSceneIdx;
int32 previousSceneIdx;

uint8 *spriteShadowPtr;
uint8 *spriteBoundingBoxPtr;

int32 currentGameOverScene;
int32 alphaLight;
int32 betaLight;

/** Timer for the next sample ambience in scene */
int32 sampleAmbienceTime;

int16 sampleAmbiance[4];
int16 sampleRepeat[4];
int16 sampleRound[4];
int16 sampleMinDelay;
int16 sampleMinDelayRnd;

int16 samplePlayed;

int16 sceneMusic;

int16 sceneHeroX; // newTwinsenXByScene
int16 sceneHeroY; // newTwinsenYByScene
int16 sceneHeroZ; // newTwinsenZByScene

int16 newHeroX; // newTwinsenX
int16 newHeroY; // newTwinsenY
int16 newHeroZ; // newTwinsenZ

int16 zoneHeroX; // newTwinsenXByZone
int16 zoneHeroY; // newTwinsenYByZone
int16 zoneHeroZ; // newTwinsenZByZone

/** Hero Y coordinate before fall */
int16 heroYBeforeFall;

/** Hero type of position in scene */
int16 heroPositionType; // twinsenPositionModeInNewCube

// ACTORS
int32 sceneNumActors;
ActorStruct sceneActors[NUM_MAX_ACTORS];
ActorStruct *sceneHero;

/** Meca pinguin actor index */
int16 mecaPinguinIdx; // currentPingouin

/** Current followed actor in scene */
int16 currentlyFollowedActor;
/** Current actor in zone */
int16 currentActorInZone; // currentActorInZoneProcess
/** Current actor manipulated in scripts */
int16 currentScriptValue; // manipActorResult

int16 talkingActor;

// ZONES

typedef struct ScenePoint {
	int16 X;
	int16 Y;
	int16 Z;
} ScenePoint;

typedef struct ZoneStruct {
	ScenePoint bottomLeft;
	ScenePoint topRight;
	int16 type;
	union {
		struct {
			int16 newSceneIdx;
			int16 X;
			int16 Y;
			int16 Z;
		} ChangeScene;
		struct {
			int16 dummy;
			int16 X;
			int16 Y;
			int16 Z;
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
	int16 snap;
} ZoneStruct;

int32 sceneNumZones;
ZoneStruct sceneZones[NUM_MAX_ZONES];

enum ZoneType {
	kCube		= 0, // Change to another scene
	kCamera		= 1, // Binds camera view
	kSceneric	= 2, // For use in Life Script
	kGrid		= 3, // Set disappearing Grid fragment
	kObject		= 4, // Give bonus
	kText		= 5, // Displays text message
	kLadder		= 6  // Hero can climb on it
};


// TRACKS

int32 sceneNumTracks;
ScenePoint sceneTracks[NUM_MAX_TRACKS];

// TODO: check what is this
int16 changeRoomVar10;
int16 changeRoomVar11;

uint8 sceneFlags[80]; // cubeFlags

/** Change to another scene */
void changeScene();

/** Process scene environment sound */
void processEnvironmentSound();

/** Process actor zones
	@param actorIdx Process actor index */
void processActorZones(int32 actorIdx);

#endif
