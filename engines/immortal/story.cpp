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

/*	These are the UNIVAT for each Story entry
	UNIVAT	1024,480,    1152,   464,    \-1, -1,             zip,level1Ladders,  rooma, 704/64,  544/32\
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

void ImmortalEngine::initStoryStatic() {
	Common::Array<Common::String> s{"#" + Common::String(kSwordBigFrame) + "sword@",
									"You find an Elven sword of&agility. Take it?@",
									"Search the bones?%",
									"}The sword permanently endows you with Elven agility and quickness in combat.@",
									"}You notice something that looks wet and green under the pile. Search further?%",
									"#" + Common::String(kBagBigFrame) + "  dust@",
									"}You find a bag containing Dust of Complaisance.&@",
									"}Drop the bait on the ground here?%",
									"}To use this dust, you throw it in the air. Do that here?%",
									"_}Don+t bother me, I+m cutting a gem. Yes, you need it. No, you can+t have it. I wouldn+t give it to anyone, least of all you. Go away. ]]]]=",
									"_}Let me help you. Please take this gem. No, really, I insist. Take it and go with my blessings. Good luck. ]]]]=",
									"#" + Common::String(kCarpetBigFrame) + "carpet@",
									"#" + Common::String(kBombBigFrame) + " bomb@",
									"A gas bomb that goblins&use to paralyze trolls.&@",
									"Take it?<>@",
									"%",
									" other@",
									"#" + Common::String(kKeyBigFrame) + "  key@",
									"#" + Common::String(kKeyBigFrame) + "  key@",
									"A key to a chest.&@",
									"The chest is open. Examine&contents?%",
									"Put it on?%",
									"Drop it?%",
									"It+s unlocked. Open it?%",
									"It+s locked but you have&the key. Open it?%",
									"It+s locked and you don+t&have the key.@",
									"The lock, triggered by a&complicated set of latches,&is unfamiliar to you.@",
									"#" + Common::String(kGoldBigFrame) + "$0 gold@",
									"You find $0 gold pieces.&&^#" + Common::String(kPileFrame) + "@",
									"@",
									"You can+t plant them on&stone tiles.@",
									"It+s locked but you are&able to unlock it with&the key.@",
									"_}The king is not dead, but the poison is taking effect. When he sees you, he attempts to speak:[(Give me water... the fountain... I give you... information... peace...+[Give him water?%",
									"_}You dont have any water to give him. He mumbles something. Then silence... You find a key on his body.]]]]=",
									"_}He mumbles something. Then silence... You find a key on his body.]]]]=",
									"_}I+ll tell you how to... next level... past slime... three jewels... slime... rock becomes... floor... right, left, center of the... [Then silence. His hand opens, releasing a key.]]]]=",
									"You find a door key.&@",
									"You find a note.&@",
									"#" + Common::String(kNoteBigFrame) + "note@",
									"He+s dead.&Look for possessions?%",
									"You don+t have it. Check&your inventory.@",
									"Game Over&&Play again?@",
									"Congratulations!&&Play again?@",
									"You find a bag of bait.&@",
									"#" + Common::String(kBagBigFrame) + "   bait@",
									"You find a stone. @",
									"#" + Common::String(kStoneBigFrame) + " stone@",
									"You find a red gem.&@",
									"#" + Common::String(kGemBigFrame) + "  gem@",
									"You find a scroll with&fireball spells.&@",
									"#" + Common::String(kScrollBigFrame) + "$ shots@",
									"You find a map warning&you about pit traps.&@",
									"#" + Common::String(kMapBigFrame) + "  map@",
									"#" + Common::String(kVaseBigFrame) + "   oil@",
									"You apply the oil but notice&as you walk that the leather&is drying out quickly.@",
									"}You discover a scroll with a charm spell to use on will o+ the wisps.&@",
									"#" + Common::String(kScrollBigFrame) + " charm@",
									"}This charms the will o+ the wisps to follow you. Read the spell again to turn them against your enemies.@",
									"}It looks like water. Drink it?%",
									"Drink it?%",
									"}It works! You are much stronger.]]]=",
									"}It looks like it has green stuff inside. Open it?%",
									"Now this will take&effect when you press the&fire button.@",
									"You find a potion,&Magic Muscle.&@",
									"#" + Common::String(kVaseBigFrame) + "  potion@",
									"You find a bottle.&@",
									"#" + Common::String(kVaseBigFrame) + "  bottle@",
									"#" + Common::String(kRingBigFrame) + "Protean@",
									"You find a Protean Ring.&@",
									"You find a troll ritual knife,&used to declare a fight to&the death. @",
									"#" + Common::String(kKnifeBigFrame) + " knife@",
									"_}It is a fine woman+s garment. Folded inside is a ring with the words,[`To Ana, so harm will never find you. -Your loving father, Dunric.+&@",
									"You find a small, well&crafted ring. @",
									"#" + Common::String(kRingBigFrame) + "  gift@",
									"#" + Common::String(kRingBigFrame) + " Ana+s@",
									"_}She is hurt and upset when she finds you don+t have her ring or won+t give it to her. She scurries back into the hole. The hole is too small for you to follow.&@",
									"_}`Sir, can you help me,+ the girl pleads. `I was kidnapped and dragged down here. All the man would say is `Mordamir+s orders.+[I ~" + Common::String(kStrGive2),
									"escaped using a ring my father gave me, but now I+ve lost it. Did you find it?+%",
									"_}We have met before, old man. Do you remember? Because you helped me, you may pass. But I warn you, we are at war with the trolls.[Over this ladder, across the spikes, is troll territory. Very dangerous.@",
									"_}You are an impostor!]]]]=",
									"_}Old man, do you remember me? I am king of the goblins. You didn+t give me the water. You left me to die after you took the key from me. Now you will pay.]]]]=",
									"_}You quickly fall into a deep, healing sleep...[Vivid images of a beautiful enchanted city pass by. All the city people are young and glowing. Fountains fill the city, and the splash and ~" + Common::String(kStrDream1P2),
									"sparkle of water is everywhere...[Suddenly the images go black. A face appears... Mordamir!]][ ~" + Common::String(kStrDream1P3),
									"He is different from how you remember him. His gentle features are now withered. His kind eyes, now cold and sunken, seem to look through you with a dark, penetrating stare. You wake rejuvenated, but disturbed.]]]]]=",
									"_}Here, take this ring in return. [I don+t know if it will help, but I heard the unpleasant little dwarf say, (Clockwise, three rings around the triangle.+[Could that be a clue to his exit puzzle? I must go. Goodbye.]]]]=",
									"#" + Common::String(kSackBigFrame) + " spores@",
									"You find a sack of bad&smelling spores.&@",
									"Please insert play disk.@",
									"New game?%",
									"Enter certificate:&-=",
									"Invalid certificate.@",
									"End of level!&Here is your certificate:&&=",
									"&@",
									"\\   Electronic Arts presents&&       The Immortal&&&&      1990 Will Harvey|]]]]]]]]\\]=",
									"          written by&&         Will Harvey&         Ian Gooding&      Michael Marcantel&       Brett G. Durrett&        Douglas Fulton|]]]]]]]/=",
									"_}Greetings, friend! Come, I+ve got something you need. These parts are plagued with slime.[You can+t venture safely without my slime oil for boots, a bargain at only 80 gold pieces.%",
									"_}All right, 60 gold pieces for my oil. Rub it on your boots and slime won+t touch you. 60, friend.%",
									"This room doesn+t resemble&any part of the map.@",
									"This room resembles part&of the map.@"};
	_strPtrs = s;

	Common::Array<int> cyc0{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1};
	Common::Array<int> cyc1{15,16,17,18,19,20,21,22,-1};
	Common::Array<int> cyc2{0,1,2,-1};
	Common::Array<int> cyc3{3,4,5,-1};
	Common::Array<int> cyc4{6,7,8,9,10,-1};
	Common::Array<int> cyc5{11,12,13,14,15,-1};
	Common::Array<int> cyc6{16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,-1};
	Common::Array<int> cyc7{0,1,2,3,4,-1};
	Common::Array<int> cyc8{5,1 + 5,2 + 5,3 + 5,4 + 5,-1};
	Common::Array<int> cyc9{10,1 + 10,2 + 10,3 + 10,4 + 10,-1};
	Common::Array<int> cyc10{15,1 + 15,2 + 15,3 + 15,4 + 15,-1};
	Common::Array<int> cyc11{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,-1};
	Common::Array<int> cyc12{0,1,2,3,4,5,6,7,8,9,-1};
	Common::Array<int> cyc13{0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, 0,1,2,3, -1};
	Common::Array<int> cyc14{31,32,33,32, 34,35,36,35, 37,38,39,38, 40,41,42,41, 43,44,45,44, 46,47,48,47, 49,50,51,50, 52,53,54,53, -1};
	Common::Array<int> cyc15{55, -1};
	Common::Array<int> cyc16{63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66, 63,64,65,66,-1};
	Common::Array<int> cyc17{0,1,0,-1};
	Common::Array<int> cyc18{0,1,2,4,5,6,7,8,9,10,11,12,2,1,-1};
	Common::Array<int> cyc19{0,0,1,2,13,14,15,16,4,2,3,-1};
	Common::Array<int> cyc20{0,1,2,3,20,21,22,23,24,25,26,27,5,4,3,-1};
	Common::Array<int> cyc21{0,1,2,3,-1};
	Common::Array<int> cyc22{0,17,18,19,3,-1};
	Common::Array<int> cyc23{0,1,-1};
	Common::Array<int> cyc24{28,28,28,28,-1};
	Common::Array<int> cyc25{15,16,15,16,15,1 + 15,1 + 15,-1};
	Common::Array<int> cyc26{10 + 15,11+ 15,12 + 15,13 + 15,14 + 15,15 + 15,16 + 15,-1};
	Common::Array<int> cyc27{2 + 15,3 + 15,4 + 15,5 + 15,-1};
	Common::Array<int> cyc28{6 + 15,7 + 15,8 + 15,9 + 15,-1};
	Common::Array<int> cyc29{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,-1};
	Common::Array<int> cyc30{0,1,2,3,3,3,3,4,5,6,-1};
	Common::Array<int> cyc31{0,1,2,3,4,5,6,7,8,-1};

	Common::Array<SCycle> c{SCycle(kBubble,     false, cyc0),  SCycle(kBubble, 	   false, cyc1),
							SCycle(kSpark, 	    false, cyc2),  SCycle(kSpark, 	   false, cyc3),
							SCycle(kSpark, 	    false, cyc4),  SCycle(kSpark, 	   false, cyc5),  SCycle(kSpark,  false, cyc6),
							SCycle(kPipe, 	    false, cyc7),  SCycle(kPipe, 	   false, cyc8),
							SCycle(kPipe,	    false, cyc9),  SCycle(kPipe, 	   false, cyc10),
							SCycle(kAnaVanish,  false, cyc11), SCycle(kAnaGlimpse, false, cyc12),
							SCycle(kKnife, 	    true,  cyc13),
							SCycle(kSpark, 	    true,  cyc14), SCycle(kSpark, 	   true, cyc15), SCycle(kSpark,  true,  cyc16),
							SCycle(kBigBurst,   false, cyc17),
							SCycle(kFlame,      false, cyc18), SCycle(kFlame,      false, cyc19), SCycle(kFlame,  false, cyc20),
							SCycle(kFlame,      false, cyc21), SCycle(kFlame,      false, cyc22), SCycle(kFlame,  false, cyc23),
							SCycle(kFlame,      false, cyc24),
							SCycle(kCandle,     false, cyc25), SCycle(kCandle,     false, cyc26), SCycle(kCandle, false, cyc27),
							SCycle(kCandle,     false, cyc28), SCycle(kCandle,     false, cyc29),
							SCycle(kSink,       false, cyc30),
							SCycle(kNorlacDown, false, cyc31)};
	_cycPtrs = c;

	Common::Array<Motive>   m{};
	_motivePtrs = m;

	Common::Array<Damage>   d{};
	_damagePtrs = d;

	Common::Array<Use>      u{};
	_usePtrs = u;

	Common::Array<Pickup>   p{};
	_pickupPtrs = p;

	CArray2D<Motive>       pr{};
	_programPtrs = pr;

	Common::Array<ObjType>  o{};
	_objTypePtrs = o;

}

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

	const uint16 kZip = 5;

	/*
	 * ::: Level 0: Intro 1 :::
	 */

	/* Universe related properties
	 * including spawn point and entry/exit points
	 */ 
	int univRoom = 4;               // The room the player starts in when beginning this level
	uint16 univRoomX = 512;
	uint16 univRoomY = 416;

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

	/*
	 * ::: Level 0: Intro 2 :::
	 */

}

} // namespace Immortal

























