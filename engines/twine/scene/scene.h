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
#include "common/util.h"
#include "twine/scene/actor.h"
#include "twine/text.h"

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

struct ScenePoint {
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;
};

/**
 * Special actions, like change scene, climbing a ladder, ...
 */
struct ZoneStruct {
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

		/** show a text (e.g. when reading a sign) */
		struct {
			int16 textIdx;   /*!< text index in the current active text bank */
			int16 textColor; /*!< text color (see @c ActorStruct::talkColor) */
		} DisplayText;
		struct {
			int16 info0;
			BonusParameter typesFlag;
			int16 amount;
			/**
			 * Already used
			 */
			int16 used;
		} Bonus;
		struct {
			int16 info0;
			int16 info1;
			int16 info2;
			int16 info3;
		} generic;
	} infoData;
	int16 snap = 0;
};

enum ZoneType {
	kCube = 0,     // Change to another scene
	kCamera = 1,   // Binds camera view
	kSceneric = 2, // For use in Life Script
	kGrid = 3,     // Set disappearing Grid fragment
	kObject = 4,   // Give bonus
	kText = 5,     // Displays text message
	kLadder = 6    // Hero can climb on it
};

enum LBA1SceneId {
	Citadel_Island_Prison = 0,
	Citadel_Island_outside_the_citadel = 1,
	Citadel_Island_near_the_tavern = 2,
	Citadel_Island_near_the_pharmacy = 3,
	Citadel_Island_near_twinsens_house = 4,
	Citadel_Island_inside_Twinsens_house = 5,
	Citadel_Island_Harbor = 6,
	Citadel_Island_Pharmacy = 7,
	White_Leaf_Desert_Temple_of_Bu_1st_scene = 8,
	Hamalayi_Mountains_landing_place = 9,
	Principal_Island_Library = 10,
	Principal_Island_Harbor = 11,
	Principal_Island_outside_the_fortress = 12,
	Principal_Island_Old_Burg = 13,
	Citadel_Island_Tavern = 14,
	Hamalayi_Mountains_Rabbibunny_village = 15,
	Citadel_Island_inside_a_Rabbibunny_house = 16,
	Principal_Island_Ruins = 17,
	Principal_Island_outside_the_library = 18,
	Principal_Island_Militairy_camp = 19,
	Citadel_Island_Architects_house = 20,
	Citadel_Island_secret_chamber_in_the_house = 21,
	Principal_Island_Ticket_office = 22,
	Principal_Island_Prison = 23,
	Principal_Island_Port_Belooga = 24,
	Principal_Island_Peg_Leg_Street = 25,
	Principal_Island_Shop = 26,
	Principal_Island_Locksmith = 27,
	Principal_Island_inside_a_Rabbibunny_house = 28,
	Principal_Island_Astronimers_House = 29,
	Principal_Island_Tavern = 30,
	Principal_Island_Basement_of_the_Astronomer = 31,
	Principal_Island_Stables = 32,
	Citadel_Island_Cellar_of_the_Tavern = 33,
	Citadel_Island_Sewer_of_the_1st_scene = 34,
	Citadel_Island_Warehouse = 35,
	White_Leaf_Desert_outside_the_Temple_of_Bu = 36,
	Principal_Island_outside_the_water_tower = 37,
	Principal_Island_inside_the_water_tower = 38,
	White_Leaf_Desert_Militairy_camp = 39,
	White_Leaf_Desert_Temple_of_Bu_2nd_scene = 40,
	White_Leaf_Desert_Temple_of_Bu_3rd_scene = 41,
	Proxima_Island_Proxim_City = 42,
	Proxima_Island_Museum = 43,
	Proxima_Island_near_the_Inventors_house = 44,
	Proxima_Island_upper_rune_stone = 45,
	Proxima_Island_lower_rune_stone = 46,
	Proxima_Island_befor_the_upper_rune_stone = 47,
	Proxima_Island_Forgers_house = 48,
	Proxima_Island_Prison = 49,
	Proxima_Island_Shop = 50,
	Proxima_Island_Sewer = 51,
	Principal_Island_house_at_Peg_Leg_Street = 52,
	Proxima_Island_Grobo_house = 53,
	Proxima_Island_Inventors_house = 54,
	Citadel_Island_Sewer_secret = 55,
	Principal_Island_Sewer_secret = 56,
	White_Leaf_Desert_Maze = 57,
	Principal_Island_House_with_the_TV = 58,
	Rebelion_Island_Harbor = 59,
	Rebelion_Island_Rebel_camp = 60,
	Some_room_cut_out = 61,
	Hamalayi_Mountains_1st_fighting_scene = 62,
	Hamalayi_Mountains_2nd_fighting_scene = 63,
	Hamalayi_Mountains_Prison = 64,
	Hamalayi_Mountains_outside_the_transporter = 65,
	Hamalayi_Mountains_inside_the_transporter = 66,
	Hamalayi_Mountains_Mutation_centre_1st_scene = 67,
	Hamalayi_Mountains_Mutation_centre_2nd_scene = 68,
	Hamalayi_Mountains_3rd_fighting_scene = 69,
	Hamalayi_Mountains_Entrance_to_the_prison = 70,
	Hamalayi_Mountains_outside_the_prison = 71,
	Hamalayi_Mountains_Catamaran_dock = 72,
	Hamalayi_Mountains_Bunker_near_clear_water = 73,
	Tippet_Island_Village = 74,
	Tippet_Island_Secret_passage_scene_2 = 75,
	Tippet_Island_near_the_bar = 76,
	Tippet_Island_Secret_passage_scene_1 = 77,
	Tippet_Island_near_the_Dino_Fly = 78,
	Tippet_Island_Secret_passage_scene_3 = 79,
	Tippet_Island_Twinsun_Cafe = 80,
	Hamalayi_Mountains_Sacret_Carrot = 81,
	Hamalayi_Mountains_Backdoor_of_the_prison = 82,
	Fortress_Island_inside_the_fortress = 83,
	Fortress_Island_outside_the_forstress = 84,
	Fortress_Island_Secret_passage_scene_1 = 85,
	Fortress_Island_Secret_in_the_fortress = 86,
	Fortress_Island_near_Zoes_cell = 87,
	Fortress_Island_Swimming_pool = 88,
	Fortress_Island_Cloning_centre = 89,
	Fortress_Island_Rune_stone = 90,
	Hamalayi_Mountains_Behind_the_sacret_carrot = 91,
	Hamalayi_Mountains_Clear_water_lake = 92,
	Fortress_Island_outside_fortress_destroyed = 93,
	Brundle_Island_outside_the_teleportation = 94,
	Brundle_Island_inside_the_teleportation = 95,
	Hamalayi_Mountains_Ski_resort = 96,
	Brundle_Island_Docks = 97,
	Brundle_Island_Secret_room = 98,
	Brundle_Island_near_the_telepods = 99,
	Fortress_Island_Docks = 100,
	Tippet_Island_Shop = 101,
	Principal_Island_house_in_port_Belooga = 102,
	Brundle_Island_Painters_house = 103,
	Citadel_Island_Ticket_Office = 104,
	Principal_Island_inside_the_fortress = 105,
	Polar_Island_2nd_scene = 106,
	Polar_Island_3rd_scene = 107,
	Polar_Island_Before_the_rocky_peak = 108,
	Polar_Island_4th_scene = 109,
	Polar_Island_The_rocky_peak = 110,
	Polar_Island_on_the_rocky_peak = 111,
	Polar_Island_Before_the_end_room = 112,
	Polar_Island_Final_Battle = 113,
	Polar_Island_end_scene = 114,
	Polar_Island_1st_scene = 115,
	Citadel_Island_end_sequence_1 = 116,
	Citadel_Island_end_sequence_2 = 117,
	Citadel_Island_Twinsens_house_destroyed = 118,
	Credits_List_Sequence = 119,

	SceneIdMax = 120
};

#define OWN_ACTOR_SCENE_INDEX 0
#define IS_HERO(x) (x) == OWN_ACTOR_SCENE_INDEX

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
	void setActorStaticFlags(ActorStruct *act, uint16 staticFlags);
	void setBonusParameterFlags(ActorStruct *act, uint16 bonusFlags);
	bool loadSceneLBA1();
	/** Initialize new scene */
	bool initScene(int32 index);
	/** Reset scene */
	void resetScene();

	// the first actor is the own hero
	ActorStruct _sceneActors[NUM_MAX_ACTORS];
	int32 _currentSceneSize = 0;

	/** Timer for the next sample ambience in scene */
	int32 _sampleAmbienceTime = 0;
	int16 _sampleAmbiance[4]{0};
	int16 _sampleRepeat[4]{0};
	int16 _sampleRound[4]{0};
	int16 _sampleMinDelay = 0;
	int16 _sampleMinDelayRnd = 0;

	int16 _samplePlayed = 0;

	int16 _sceneMusic = 0;

	int16 _sceneHeroX = 0; // newTwinsenXByScene
	int16 _sceneHeroY = 0; // newTwinsenYByScene
	int16 _sceneHeroZ = 0; // newTwinsenZByScene

	int16 _zoneHeroX = 0; // newTwinsenXByZone
	int16 _zoneHeroY = 0; // newTwinsenYByZone
	int16 _zoneHeroZ = 0; // newTwinsenZByZone
	int32 _currentGameOverScene = 0;

public:
	Scene(TwinEEngine *engine) : _engine(engine) {}
	~Scene();

	uint8 *currentScene = nullptr;

	int32 needChangeScene = LBA1SceneId::Citadel_Island_Prison;
	int32 currentSceneIdx = LBA1SceneId::Citadel_Island_Prison;
	int32 previousSceneIdx = LBA1SceneId::Citadel_Island_Prison;

	int32 sceneTextBank = TextBankId::None;
	int32 alphaLight = 0;
	int32 betaLight = 0;

	int16 newHeroX = 0; // newTwinsenX
	int16 newHeroY = 0; // newTwinsenY
	int16 newHeroZ = 0; // newTwinsenZ

	/** Hero Y coordinate before fall */
	int16 heroYBeforeFall = 0;

	/** Hero type of position in scene */
	ScenePositionType heroPositionType = ScenePositionType::kNoPosition; // twinsenPositionModeInNewCube

	// ACTORS
	int32 sceneNumActors = 0;
	ActorStruct *sceneHero = nullptr;
	ActorStruct *getActor(int32 actorIdx);

	/** Meca pinguin actor index */
	int16 mecaPinguinIdx = 0; // currentPingouin

	/** Current followed actor in scene */
	int16 currentlyFollowedActor = OWN_ACTOR_SCENE_INDEX;
	/** Current actor in zone - climbing a ladder */
	bool currentActorInZone = false;
	/** Current actor manipulated in scripts */
	int16 currentScriptValue = 0; // manipActorResult

	int16 talkingActor = 0;

	// TRACKS Tell the actor where to go

	int32 sceneNumTracks = 0;
	ScenePoint sceneTracks[NUM_MAX_TRACKS];

	// TODO: check what is this - disables rendering of the grid tiles - used for credits only?
	bool changeRoomVar10 = false;

	uint8 sceneFlags[NUM_SCENES_FLAGS]{0}; // cubeFlags

	int32 sceneNumZones = 0;
	ZoneStruct sceneZones[NUM_MAX_ZONES];

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
	return currentScene != nullptr;
}

} // namespace TwinE

#endif
