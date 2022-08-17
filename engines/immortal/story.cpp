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

/* [Alternate Name: Level Subsystem/Script Data]
 * --- Story File ---
 * A story file (as defined in story.h) is a set of ROM
 * data that describes the properties of a level. This includes
 * the coordinates for each room, the doors in the level,
 * the torches, objects, monsters, etc. It also included the string
 * data in the source code, but technically it was writing those
 * strings to a separate string bank, so they weren't contiguous
 * with the story files (sometimes?). These story files are read in when loading
 * a new level, and are used to construct the room object, the monster
 * objects, and everything in the rooms.
 */

/*	UNIVAT	1024,480,    1152,   464,    \-1, -1,             zip,level1Ladders,  rooma, 704/64,  544/32\
	UNIVAT	304, 448,    472+32, 500+16, \-1, -1,             zip,level12Ladders, -1,    0,       0\
	UNIVAT	600, 450,    560,    598,    \-1, r2.b+(16*r2.a), zip,level3Ladders,  r2.b,  640/64,  576/32\
	UNIVAT	120, 540,    188,    584,    \-1, -1,             zip,level4Ladders,  -1,    0,       0\
	UNIVAT	64,  128,    128,    128+32, \-1, -1,             zip,level5Ladders,  -1,    1088/64, 928/32\
	UNIVAT	768, 224,    896,    288-16, \-1, -1,             zip,level5Ladders,  -1,    1088/64, 928/32\
	UNIVAT	896, 672+64, 960,    832-16, \-1, -1,             zip,level6Ladders,  -1,    0,       0\
	UNIVAT	688, 800,    912-64, 888-32, \-1, -1,             zip,level7Ladders,  -1,    1088/64, 928/32\
	UNIVAT	64,  704,    64+96,  704+64, \-1, -1,             zip,level8Ladders,  -1,    0,       0\
*/

#include "immortal/immortal.h"

namespace Immortal {

void ImmortalEngine::initStoryDynamic() {
	/* There is one major difference between the source logic and this method.
	 * It doesn't change the game logic, but it does change the logic of storing
	 * the initial rom data. In the source, because there are no language based
	 * arrays available (the array/qarray have overhead and are not designed for this),
	 * the story entries are written out dynamically to ensure everything links together
	 * (in quite a clever way, but does require a lot of untangling to see).
	 * On the game end however, this means that to populate a level with it's objects,
	 * rooms, etc. It has to look at every single entry individually, and check the 'recordop'.
	 * This tells the game what kind of entry it is, and therefor which routine to call.
	 * But, the catch is that making sure the right entry goes with the right room is tricky.
	 * In certain cases, there are references to the rooms. In most however it relies on
	 * INROOM, which is a macro that basically sets the dynamic variable keeping track of what
	 * room the current entry is using for x/y coordinates. This doesn't serve any purpose
	 * for us though, because we can use real arrays and structs for the stories, which is what
	 * I believe the source would have used (though even the DOS version did it this way so
	 * who knows). All of this to say, instead of INROOM, the equivlent here is basically
	 * checking for nullptr within arrays that are always the size of the number of rooms.
	 */

	// *NOTE* the data types Trap and Program will be in the static Story area, and referenced by an enum

	const uint8 kZip = 5;

	/*
	 * ::: Level 0: Intro 1 :::
	 */

	/* Universe related properties
	 * including spawn point and entry/exit points
	 */ 
	int univRoom = 4;               // The room the player starts in when beginning this level
	uint8 univRoomX = 512;
	uint8 univRoomY = 416;

	_stories[0]._level = 0;
	_stories[0]._part  = 1;
	_stories[0]._initialUnivX = 1024 / 8;
	_stories[0]._initialUnivY = 480 / 8;
	_stories[0]._playerPointX = (1152 - univRoomX) / 2;
	_stories[0]._playerPointY = 464 - univRoomY;

	Common::Array<int> ladders{-1, -1, kStoryNull, 2, 0, univRoom, (704 / 64),(544 / 32)};
	_stories[0]._ladders = ladders;

	/* All of the rooms
	 */
	Common::Array<SRoom> rooms{SRoom(384, 256, kRoomFlag0), SRoom(512, 64, kRoomFlag0),
							   SRoom(640, 160, kRoomFlag0), SRoom(768, 224, kRoomFlag0),
							   SRoom(univRoomX, univRoomY, kRoomFlag0), SRoom(960, 512, kRoomFlag0),
							   SRoom(1024, 352, kRoomFlag0), SRoom(896, 64, kRoomFlag0)};
	_stories[0]._rooms = rooms;

	/* All of the doors
	 */
	Common::Array<SDoor> doors{SDoor(0, 704, 224, 0, 2, false), SDoor(1, 576, 352, 4, 0, true),
					  		   SDoor(1, 704, 96,  2, 1, false), SDoor(1, 960, 128, 7, 2, false),
					  		   SDoor(1, 1088,160, 3, 7, false), SDoor(1, 1088,320, 6, 3, false),
					  		   SDoor(1, 896, 416, 4, 3, false)};
	_stories[0]._doors = doors;

	/* All of the flames
	 * Macro for flames is (x - roomx), (y - roomy), pattern number
	 */
	Common::Array<SFlame> f5{SFlame(512 - 384,   (240 + 32) - 256, kFlameOff),    SFlame(672 - 384, (240 + 32) - 256, kFlameOff)};
	Common::Array<SFlame> f7{SFlame(576 - 384,   (112 + 32) - 256, kFlameNormal), SFlame(576 - 384, (112 + 32) - 256, kFlameNormal),
							 SFlame(928 - 384,   (48 + 32) - 256,  kFlameNormal)};
	Common::Array<SFlame> f8{SFlame(800 - 640,   (144 + 32) - 160, kFlameNormal)};
	Common::Array<SFlame> f9{SFlame(768 - 768,   (304 + 32) - 224, kFlameNormal), SFlame((928 - 768), (304 + 32) - 224, kFlameNormal),
							 SFlame(1024 - 768,  (240 + 32) - 224, kFlameNormal)};
	Common::Array<SFlame> fA{SFlame(672 - 512,   (400 + 32) - 416, kFlameNormal), SFlame((800 - 64) - 512, (496 - 32) - 416, kFlameNormal),
							 SFlame(576 - 512,   (528 + 32) - 416, kFlameNormal)};
	Common::Array<SFlame> fD{SFlame(1024 - 960,  (496 + 32) - 512, kFlameNormal)};
	Common::Array<SFlame> fE{SFlame(1184 - 1024,  432 - 352, 	   kFlameCandle)};
	Common::Array<SFlame> fF{SFlame(1024 - 896,  (144 + 32) - 64,  kFlameNormal)};
	CArray2D<SFlame> flames{f5, f7, f8, f9, fA, fD, fE, fF};
	_stories[0]._flames = flames;

	/* All of the objects
	 * Macro for traps is arrowType,freq,#sinkTraps,#1(going toward 5),#3,#5,#7,#trapdoors
	 */
	Common::Array<uint8> noTraps{};
	Common::Array<uint8> o5Traps{0,0x80,0,0,0,0,0,5};
	Common::Array<uint8> o7Traps{0,0x80,15,5,3,0,0,0};
	Common::Array<uint8> o8Traps{0,0x80,0,0,0,0,0,3};

	Common::Array<SObj> noObj{};
	Common::Array<SObj> o5{SObj(kZip, kZip, kTypeTrap,     kNoFrame,         kObjIsRunning + kObjIsInvisible, o5Traps),
				  		   SObj(459,  379,  kTypeCoin,     kRingFrame,       kObjNone,                        noTraps),
				  		   SObj(446,  327,  kTypeWowCharm, kScrollFrame,     kObjNone,                        noTraps)};
	Common::Array<SObj> o7{SObj(145,  138,  kTypeTrap,     kNoFrame,         kObjIsRunning + kObjIsInvisible, o7Traps)};
	Common::Array<SObj> o8{SObj(kZip, kZip, kTypeTrap,     kNoFrame,         kObjIsRunning + kObjIsInvisible, o8Traps)};
	Common::Array<SObj> o9{SObj(1052, 309,  kTypeDead,     kDeadGoblinFrame, kObjIsChest + kObjIsOnGround,    noTraps),
						   SObj(kZip, kZip, kTypeFireBall, kScrollFrame,     kObjUsesFireButton,              noTraps),
						   SObj(128,  464,  kTypeDunRing,  kRingFrame,       0,                               noTraps),
						   SObj(837,  421,  kTypeChest,    kChest0Frame,     kObjIsChest,                     noTraps),
					       SObj(kZip, kZip, kTypeDeathMap, kScrollFrame,     0,                               noTraps),
					       SObj(597,  457,  kTypeWater,    kVaseFrame,       0,                               noTraps),
					       SObj(kZip, kZip, kTypeSpores,   kSporesFrame,     0,                               noTraps),
					       SObj(kZip, kZip, kTypeWormFood, kNoFrame,         0,                               noTraps),
					       SObj(205,  158,  kTypeChestKey, kKeyFrame,        0,                               noTraps)};
	Common::Array<SObj> oE{SObj(1184, 426,  kTypePhant,    kAltarFrame,      0,                               noTraps),
				  		   SObj(145,  138,  kTypeGold,     kNoFrame,         kObjIsRunning,                   noTraps),
				  		   SObj(671,  461,  kTypeHay,      kNoFrame,         kObjIsRunning + kObjIsInvisible, noTraps),
				  		   SObj(780,  508,  kTypeBeam,     kNoFrame,         kObjIsRunning + kObjIsInvisible, noTraps)};
	CArray2D<SObj> objects{o5, o7, o8, o9, noObj, noObj, oE, noObj};
	_stories[0]._objects = objects;

	/* All of the monsters
	 * A 'Program' is just an array of pointers to 'Motives'
	 */
	Common::Array<Motive> progShade{kMotiveRoomCombat, kMotiveShadeFind, kMotiveShadeLoose, kMotiveEngage, kMotiveUpdateGoal, kMotiveFollow, kMotiveShadeHesitate};
	Common::Array<Motive> progEasy{kMotiveEasyRoomCombat, kMotiveFind8, kMotiveLoose4, kMotiveEngage, kMotiveUpdateGoal, kMotiveFollow};
	Common::Array<Motive> progUlindor{kMotiveDefensiveCombat, kMotiveEngage, kMotiveUlinTalk, kMotiveGive, kMotiveUseUpMonster};
	Common::Array<Motive> progGoblin5{kMotiveAliveRoomCombat, kMotiveFindAlways, kMotiveLoose4, kMotiveEngage, kMotiveUpdateGoal, kMotiveFollow};
	Common::Array<Motive> progPlayer{kMotivePlayerCombat, kMotiveJoystick, kMotivePlayerDoor};
	Common::Array<Motive> progWill2{kMotiveRoomCombat, kMotivewaittalk2, kMotiveFindAlways, kMotiveGetDisturbed, kMotiveLoose32, kMotiveUpdateGoal, kMotiveIfNot1Skip1, kMotiveFollow, kMotiveEngage};

	Common::Array<SMonster> noMonst{};
	Common::Array<SMonster> m5{SMonster(448,  344, 12, kMonstPlayer,  kMonstA + kMonstIsEngage + kMonstIsTough, progShade,   kShadow),
					  		   SMonster(590,  381, 12, kMonstPlayer,  kMonstA + kMonstIsEngage + kMonstIsTough, progShade,   kShadow)};
	Common::Array<SMonster> m9{SMonster(1106, 258, 3,  kMonstPlayer,  kMonstA + kMonstIsEngage,                 progEasy,    kGoblin0),
							   SMonster(832,  364, 10, kMonstA,       kMonstB + kMonstIsPoss,                   progUlindor, kUlindor3),
					  		   SMonster(838,  370, 15, kMonstPlayer,  kMonstA + kMonstIsEngage,                 progGoblin5, kGoblin7)};
	Common::Array<SMonster> mE{SMonster(1136, 464, 15, kMonstMonster, kMonstPlayer + kMonstIsEngage,            progPlayer,  kWizard0)};
	Common::Array<SMonster> mF{SMonster(1182, 116, 5,  kMonstPlayer,  kMonstA + kMonstIsEngage,                 progWill2,   kGoblin5)};
	CArray2D<SMonster> monsters{m5, noMonst, noMonst, m9, noMonst, noMonst, mE, mF};
	_stories[0]._monsters = monsters;

}

} // namespace Immortal

























