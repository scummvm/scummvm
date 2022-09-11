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

#ifndef TWINE_SHARED_H
#define TWINE_SHARED_H

#include "common/scummsys.h"

#define NUM_GAME_FLAGS 255

/** Number of colors used in the game */
#define NUMOFCOLORS 256

#define NUM_LOCATIONS 150

#define NUM_INVENTORY_ITEMS 28
/**
 * This gameflag indicates that the inventory items are taken from Twinson because he went to jail
 */
#define GAMEFLAG_INVENTORY_DISABLED 70
// Hit
#define GAMEFLAG_VIDEO_BAFFE 200
// Hit, band-aid
#define GAMEFLAG_VIDEO_BAFFE2 201
// Hit, black eye
#define GAMEFLAG_VIDEO_BAFFE3 202
// Ferry #1
#define GAMEFLAG_VIDEO_BATEAU 203
// Temple of Bu
#define GAMEFLAG_VIDEO_TEMPLE 204
// White Leaf Desert, flute
#define GAMEFLAG_VIDEO_FLUTE2 205
// Hamalayi Mountains, chuttle
#define GAMEFLAG_VIDEO_NAVETTE 206
// Hamalayi Mountains, storm
#define GAMEFLAG_VIDEO_NEIGE2 207
// Hamalayi Mountains, ski lift
#define GAMEFLAG_VIDEO_SURF 208
// Ferry #2
#define GAMEFLAG_VIDEO_BATEAU2 209
// Fortress, Zoe Clone
#define GAMEFLAG_VIDEO_CAPTURE 210
// Fortress, Rune stone (cut from the game)
#define GAMEFLAG_VIDEO_VERSER 211
// Fortress, Rune stone
#define GAMEFLAG_VIDEO_VERSER2 212
// Fortress, explosion
#define GAMEFLAG_VIDEO_FORTRESS 213
// Sendel give powers to Twinsen and Zoe.
#define GAMEFLAG_VIDEO_SENDEL2 214
// Hit, reject
#define GAMEFLAG_VIDEO_BAFFE5 215
// Twinsun explosion (on top of the well)
#define GAMEFLAG_VIDEO_EXPLODE 216
// Clear water lake
#define GAMEFLAG_VIDEO_GLASS2 217
// Twinsen in Well of Sendell
#define GAMEFLAG_VIDEO_SENDEL 218
// Twinsun explosion
#define GAMEFLAG_VIDEO_EXPLODE2 219

#define OWN_ACTOR_SCENE_INDEX 0
#define IS_HERO(x) ((x) == OWN_ACTOR_SCENE_INDEX)

namespace TwinE {

#include "common/pack-start.h"
struct I16Vec3 {
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;
};
#include "common/pack-end.h"
STATIC_ASSERT(sizeof(I16Vec3) == 6, "Unexpected pointTab size");

struct IVec3 {
	constexpr IVec3() : x(0), y(0), z(0) {}
	constexpr IVec3(int32 _x, int32 _y, int32 _z) : x(_x), y(_y), z(_z) {}
	int32 x;
	int32 y;
	int32 z;

	inline IVec3 &operator=(const I16Vec3& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	inline IVec3& operator+=(const IVec3 &other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	inline IVec3& operator-=(const IVec3 &other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
};

inline constexpr IVec3 operator+(const IVec3 &lhs, const IVec3 &rhs) {
	return IVec3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline constexpr IVec3 operator-(const IVec3 &lhs, const IVec3 &rhs) {
	return IVec3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

inline constexpr IVec3 operator-(const IVec3 &v) {
	return IVec3{-v.x, -v.y, -v.z};
}

/**
 * Get distance value in 2D
 * @param x1 Actor 1 X coordinate
 * @param z1 Actor 1 Z coordinate
 * @param x2 Actor 2 X coordinate
 * @param z2 Actor 2 Z coordinate
 */
int32 getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2);
int32 getDistance2D(const IVec3 &v1, const IVec3 &v2);

/**
 * Get distance value in 3D
 * @param x1 Actor 1 X coordinate
 * @param y1 Actor 1 Y coordinate
 * @param z1 Actor 1 Z coordinate
 * @param x2 Actor 2 X coordinate
 * @param y2 Actor 2 Y coordinate
 * @param z2 Actor 2 Z coordinate
 */
int32 getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2);
int32 getDistance3D(const IVec3 &v1, const IVec3 &v2);

/**
 * @brief Axis aligned bounding box
 */
struct BoundingBox {
	IVec3 mins;
	IVec3 maxs;
};

struct ActorBoundingBox {
	BoundingBox bbox;
	bool hasBoundingBox = false;
};

enum class ActionType : uint8 {
	ACTION_NOP = 0,
	ACTION_BODY = 1,
	ACTION_BODP = 2,
	ACTION_ANIM = 3,
	ACTION_ANIP = 4,
	ACTION_HITTING = 5,
	ACTION_SAMPLE = 6,
	ACTION_SAMPLE_FREQ = 7,
	ACTION_THROW_EXTRA_BONUS = 8,
	ACTION_THROW_MAGIC_BALL = 9,
	ACTION_SAMPLE_REPEAT = 10,
	ACTION_THROW_SEARCH = 11,
	ACTION_THROW_ALPHA = 12,
	ACTION_SAMPLE_STOP = 13,
	ACTION_ZV = 14,
	ACTION_LEFT_STEP = 15,
	ACTION_RIGHT_STEP = 16,
	ACTION_HERO_HITTING = 17,
	ACTION_THROW_3D = 18,
	ACTION_THROW_3D_ALPHA = 19,
	ACTION_THROW_3D_SEARCH = 20,
	ACTION_THROW_3D_MAGIC = 21,
	ACTION_LAST
};

enum class ShapeType {
	kNone = 0,
	kSolid = 1,
	kStairsTopLeft = 2,
	kStairsTopRight = 3,
	kStairsBottomLeft = 4,
	kStairsBottomRight = 5,
	kDoubleSideStairsTop1 = 6,
	kDoubleSideStairsBottom1 = 7,
	kDoubleSideStairsLeft1 = 8,
	kDoubleSideStairsRight1 = 9,
	kDoubleSideStairsTop2 = 10,
	kDoubleSideStairsBottom2 = 11,
	kDoubleSideStairsLeft2 = 12,
	kDoubleSideStairsRight2 = 13,
	kFlatBottom1 = 14,
	kFlatBottom2 = 15
};

/** Control mode types */
enum class ControlMode {
	kNoMove = 0,
	kManual = 1,
	kFollow = 2,
	kTrack = 3,
	kFollow2 = 4,
	kTrackAttack = 5,
	kSameXZ = 6,
	kRandom = 7
};

enum class AnimationTypes {
	kAnimNone = -1,
	kStanding = 0,
	kForward = 1,
	kBackward = 2,
	kTurnLeft = 3,
	kTurnRight = 4,
	kHit = 5,
	kBigHit = 6,
	kFall = 7,
	kLanding = 8,
	kLandingHit = 9,
	kLandDeath = 10,
	kAction = 11,
	kClimbLadder = 12,
	kTopLadder = 13,
	kJump = 14,
	kThrowBall = 15,
	kHide = 16,
	kKick = 17,
	kRightPunch = 18,
	kLeftPunch = 19,
	kFoundItem = 20,
	kDrawn = 21,
	kHit2 = 22,
	kSabreAttack = 23,
	kSabreUnknown = 24,
	kCarStarting = 303,
	kCarDriving = 304,
	kCarDrivingBackwards = 305,
	kCarStopping = 306,
	kCarFrozen = 307,
	kAnimInvalid = 255
};

enum class AnimType {
	kAnimationTypeLoop = 0,
	kAnimationThen = 1,
	// play animation and let animExtra follow as next animation
	// if there is already a next animation set - replace the value
	kAnimationAllThen = 2,
	// replace animation and let the current animation follow
	kAnimationInsert = 3,
	// play animation and let animExtra follow as next animation
	// but don't take the current state in account
	kAnimationSet = 4
};

/** Hero behaviour
 * <li> NORMAL: Talk / Read / Search / Use
 * <li> ATHLETIC: Jump
 * <li> AGGRESSIVE:
 * Auto mode   : Fight
 * Manual mode : While holding the spacebar down
 * 			UP / RIGHT / LEFT will manually select
 * 			different punch/kick options
 * <li> DISCREET: Kneel down to hide
 *
 * @note The values must match the @c TextId indices
 */
enum class HeroBehaviourType {
	kNormal = 0,
	kAthletic = 1,
	kAggressive = 2,
	kDiscrete = 3,
	kProtoPack = 4
};

/**
 * 0: tunic + medallion
 * 1: tunic
 * 2: tunic + medallion + sword
 * 3: prison suit
 * 4: nurse outfit
 * 5: tunic + medallion + horn
 * 6: snowboard (WARNING, this can crash the game when you change behavior)
 */
enum class BodyType {
	btNone = -1,
	btNormal = 0,
	btTunic = 1,
	btSabre = 2,
	btPrisonSuit = 3,
	btNurseSuit = 4,
	btHorn = 5,
	btSnowboard = 6
};

enum class ExtraSpecialType {
	kHitStars = 0,
	kExplodeCloud = 1
};

enum class ZoneType {
	kCube = 0,     // Change to another scene
	kCamera = 1,   // Binds camera view
	kSceneric = 2, // For use in Life Script
	kGrid = 3,     // Set disappearing Grid fragment
	kObject = 4,   // Give bonus
	kText = 5,     // Displays text message
	kLadder = 6    // Hero can climb on it
};

#define SCENE_CEILING_GRID_FADE_1 (-1)
#define SCENE_CEILING_GRID_FADE_2 (-2)

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

// lba
enum class TextBankId : int16 {
	None = -1,
	Options_and_menus = 0,
	Credits = 1,
	Inventory_Intro_and_Holomap = 2,
	Citadel_Island = 3,
	Principal_Island = 4,
	White_Leaf_Desert = 5,
	Proxima_Island = 6,
	Rebellion_Island = 7,
	Hamalayi_mountains_southern_range = 8,
	Hamalayi_mountains_northern_range = 9,
	Tippet_Island = 10,
	Brundle_Island = 11,
	Fortress_Island = 12,
	Polar_Island = 13
};

/** menu text ids */
enum class TextId : int16 {
	kNone = -1,
	kBehaviourNormal = 0,
	kBehaviourSporty = 1,
	kBehaviourAggressiveManual = 2,
	kBehaviourHiding = 3,
	kBehaviourAggressiveAuto = 4,
	kUseProtopack = 5,
	kSendell = 6,
	kMusicVolume = 10,
	kSoundVolume = 11,
	kCDVolume = 12,
	kSpeechVolume = 13,
	kMasterVolume = 14,
	kReturnGame = 15,
	kSaveSettings = 16,
	kNewGame = 20,
	kNewGamePlus = 255,
	kContinueGame = 21,
	kQuit = 22,
	kOptions = 23,
	kDelete = 24,
	kReturnMenu = 26,
	kGiveUp = 27,
	kContinue = 28,
	kVolumeSettings = 30,
	kDetailsPolygonsHigh = 31,
	kDetailsShadowHigh = 32,
	//kSceneryZoomOn = 33, // duplicate with 133 - TODO check if this is the same in all languages
	kCreateNewPlayer = 40,
	kCreateSaveGame = 41,
	kEnterYourName = 42,
	kPlayerAlreadyExists = 43,
	kEnterYourNewName = 44,
	kDeleteSaveGame = 45,
	kSaveManage = 46,
	kAdvanced = 47,
	kDelete2 = 48, // difference between 24 and 48?
	kTransferVoices = 49,
	kPleaseWaitWhileVoicesAreSaved = 50,
	kRemoveProtoPack = 105,
	kDetailsPolygonsMiddle = 131,
	kShadowsFigures = 132,
	kSceneryZoomOn = 133,
	kIntroText1 = 150,
	kIntroText2 = 151,
	kIntroText3 = 152,
	kBookOfBu = 161,
	kBonusList = 162,
	kStarWarsFanBoy = 226,
	kDetailsPolygonsLow = 231,
	kShadowsDisabled = 232,
	kNoSceneryZoom = 233,

	// custom strings (not originally included in the game)
	kCustomHighResOptionOn = -2,
	kCustomHighResOptionOff = -3,
	kCustomWallCollisionOn = -4,
	kCustomWallCollisionOff = -5,

	// ------ lba2

	toContinueGame = 70,
	toNewGame = 71,
	toLoadGame = 72,
	toSauver = 73,
	toOptions = 74,
	toQuit = 75
};

enum InventoryItems {
	kiHolomap = 0,
	kiMagicBall = 1,
	kiUseSabre = 2,
	kiGawleysHorn = 3,
	kiTunic = 4,
	kiBookOfBu = 5,
	kSendellsMedallion = 6,
	kFlaskOfClearWater = 7,
	kRedCard = 8,
	kBlueCard = 9,
	kIDCard = 10,
	kMrMiesPass = 11,
	kiProtoPack = 12,
	kSnowboard = 13,
	kiPenguin = 14,
	kGasItem = 15,
	kPirateFlag = 16,
	kMagicFlute = 17,
	kSpaceGuitar = 18,
	kHairDryer = 19,
	kAncesteralKey = 20,
	kBottleOfSyrup = 21,
	kEmptyBottle = 22,
	kFerryTicket = 23,
	kKeypad = 24,
	kCoffeeCan = 25,
	kiBonusList = 26,
	kiCloverLeaf = 27,
	MaxInventoryItems = 28
};

struct TwineResource {
	const char *hqr;
	const int32 index;

	constexpr TwineResource(const char *_hqr, int32 _index) : hqr(_hqr), index(_index) {
	}
};

struct TwineImage {
	TwineResource image;
	TwineResource palette;

	constexpr TwineImage(const char *hqr, int32 index, int32 paletteIndex = -1) : image(hqr, index), palette(hqr, paletteIndex) {
	}
};

// lba2 does from 0 to 0x1000
// lba1 angles
// TODO: wrap in a class to be able to handle lba1 and lba2
#define ANGLE_360 1024
#define ANGLE_351 1000
#define ANGLE_334 950
#define ANGLE_315 896
#define ANGLE_270 768
#define ANGLE_225 640
#define ANGLE_210 600
#define ANGLE_180 512
#define ANGLE_140 400
#define ANGLE_135 384
#define ANGLE_90 256
#define ANGLE_70 200
#define ANGLE_63 180
#define ANGLE_45 128
#define ANGLE_22_5 64
#define ANGLE_17 50
#define ANGLE_11_25 32
#define ANGLE_2 8 // 1.67
#define ANGLE_1 5 // 1.75
#define ANGLE_0 0

#define VIEW_X0 (-50)
#define VIEW_Y0 (-30)
#define VIEW_X1(engine) ((engine)->width() - 40)
#define VIEW_Y1(engine) ((engine)->height() + 100)

inline int32 NormalizeAngle(int32 angle) {
	if (angle < -ANGLE_180) {
		angle += ANGLE_360;
	} else if (angle > ANGLE_180) {
		angle -= ANGLE_360;
	}
	return angle;
}

/**
 * @param[in] angle The angle as input from game data
 * @return The value as it is used at runtime
 */
inline constexpr int32 ToAngle(int32 angle) {
	return angle;
}

/**
 * @param[in] angle The angle as used at runtime
 * @return The value as it should be used for storing in game data
 */
inline constexpr int32 FromAngle(int32 angle) {
	return angle;
}

inline constexpr double AngleToRadians(int32 angle) {
	return 2.0 * M_PI * angle / (double)ANGLE_360;
}

inline constexpr int32 ClampAngle(int32 angle) {
	return angle & (ANGLE_360 - 1);
}

template<typename T>
inline constexpr T bits(T value, uint8 offset, uint8 bits) {
	return (((1 << bits) - 1) & (value >> offset));
}

#define COLOR_BLACK 0
#define COLOR_BRIGHT_BLUE 4
#define COLOR_9 9
#define COLOR_14 14
// color 1 = yellow
// color 2 - 15 = white
// color 16 - 19 = brown
// color 20 - 24 = orange to yellow
// color 25 orange
// color 26 - 30 = bright gray or white
#define COlOR_31 31 // green dark
#define COlOR_47 47 // green bright
#define COLOR_48 48 // brown dark
#define COLOR_63 63 // brown bright
#define COLOR_64 64 // blue dark
#define COLOR_68 68 // blue
#define COLOR_73 73 // blue
#define COLOR_75 75
#define COLOR_79 79 // blue bright
#define COLOR_80 80
#define COLOR_91 91
#define COLOR_BRIGHT_BLUE2 69
#define COLOR_WHITE 15
#define COLOR_GOLD 155
#define COLOR_158 158

enum kDebugLevels {
	kDebugScripts =   1 << 0
};

}

#endif
