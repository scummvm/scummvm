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

#ifndef CHAMBER_ENUMS_H
#define CHAMBER_ENUMS_H

namespace Chamber {

/*game items, mirrors inventory_items (except kItemNone)*/
enum Items {
	kItemNone = 0,

	kItemSkull1 = 1,
	kItemSkull2 = 2,
	kItemSkull3 = 3,
	kItemSkull4 = 4,
	kItemSkull5 = 5,

	kItemRope1 = 6,
	kItemRope2 = 7,
	kItemRope3 = 8,
	kItemRope4 = 9,
	kItemRope5 = 10,

	kItemFly1 = 11,
	kItemFly2 = 12,
	kItemFly3 = 13,
	kItemFly4 = 14,

	kItemGoblet1 = 15,
	kItemGoblet2 = 16,
	kItemGoblet3 = 17,
	kItemGoblet4 = 18,

	kItemDagger1 = 19,
	kItemDagger2 = 20,
	kItemDagger3 = 21,
	kItemDagger4 = 22,

	kItemLantern1 = 23,
	kItemLantern2 = 24,
	kItemLantern3 = 25,
	kItemLantern4 = 26,

	kItemBlueSpider1 = 27,
	kItemBlueSpider2 = 28,
	kItemBlueSpider3 = 29,
	kItemBlueSpider4 = 30,

	kItemRedSpider1 = 31,
	kItemRedSpider2 = 32,
	kItemRedSpider3 = 33,
	kItemRedSpider4 = 34,

	kItemDie1 = 35,
	kItemDie2 = 36,
	kItemDie3 = 37,
	kItemDie4 = 38,

	kItemZapstik1 = 39,
	kItemZapstik2 = 40,
	kItemZapstik3 = 41,
	kItemZapstik4 = 42,
	kItemZapstik5 = 43,
	kItemZapstik6 = 44,
	kItemZapstik7 = 45,
	kItemZapstik8 = 46,
	kItemZapstik9 = 47,
	kItemZapstik10 = 48,
	kItemZapstik11 = 49,
	kItemZapstik12 = 50,
	kItemZapstik13 = 51,
	kItemZapstik14 = 52,

	kItemMask = 53,
	kItemWhistle = 54,
	kItemEgg1 = 55,
	kItemBlade = 56,
	kItemChopper = 57,
	kItemZorq = 58,
	kItemSaura = 59,
	kItemFlask = 60,
	kItemBean = 61,
	kItemEgg2 = 62,
	kItemEgg3 = 63
};

/*game persons, mirrors pers_list*/
enum Persons {
	kPersVort = 0,

	kPersAspirant1 = 1,
	kPersAspirant2 = 2,
	kPersAspirant3 = 3,
	kPersAspirant4 = 4,

	kPersTurkey = 5,

	kPersPriestess1 = 6,
	kPersPriestess2 = 7,

	kPersMaster = 8,

	kPersProtozorq1 = 9,
	kPersProtozorq2 = 10,
	kPersProtozorq3 = 11,
	kPersProtozorq4 = 12,
	kPersProtozorq5 = 13,
	kPersProtozorq6 = 14,
	kPersProtozorq7 = 15,
	kPersProtozorq8 = 16,
	kPersProtozorq9 = 17,
	kPersProtozorq10 = 18,
	kPersProtozorq11 = 19,
	kPersProtozorq12 = 20,
	kPersProtozorq13 = 21,
	kPersProtozorq14 = 22,

	kPersPoormouth = 23,
	kPersKhele = 24,
	kPersMistress = 25,
	kPersDeilos = 26,
	kPersScifi = 27,
	kPersNormajeen = 28,
	kPersAsh = 29,
	kPersMonkey = 30,
	kPersHarssk = 31,
	kPersZorq = 32,
	kPersQriich = 33,

	kPersVort2 = 34,
	kPersVort3 = 35,

	kPersPriestess3 = 36,
	kPersPriestess4 = 37,

	kPersCadaver = 38,

	kPersTurkey1 = 39,
	kPersTurkey2 = 40
};

#define PersonOffset(p) ((p) * 5)

enum Areas {
	kAreaNone = 0,
	kAreaTheMastersEye = 1,
	kAreaTheMastersOrbit1 = 2,
	kAreaTheMastersOrbit2 = 3,
	kAreaTheMastersOrbit3 = 5,
	kAreaTheReturn = 8,
	kAreaTheRing1 = 10,
	kAreaTheRing2 = 12,
	kAreaTheRing3 = 14,
	kAreaTheRing4 = 16,
	kAreaTheRing5 = 18,
	kAreaTheRing6 = 20,
	kAreaDeProfundis = 22,
	kAreaTheWall = 24,
	kAreaTheInfidelsTomb1 = 25,
	kAreaTheInfidelsTomb2 = 26,
	kAreaVictoryOfTheFaith1 = 25,
	kAreaVictoryOfTheFaith2 = 26,
	kAreaDeadEnd = 29,
	kAreaTheWall2 = 30,
	kAreaTheInfidelsTomb3 = 31,
	kAreaTheInfidelsTomb4 = 32,
	kAreaVictoryOfTheFaith3 = 31,
	kAreaVictoryOfTheFaith4 = 32,
	kAreaDeadEnd2 = 35,
	kAreaTheNoose = 36,
	kAreaTheSource = 37,
	kAreaTheTwins = 38,
	kAreaWhoWillBeSaved = 39,
	kAreaInTheScorpionsPresence = 40,
	kAreaTheWeb = 41,
	kAreaPassage1 = 44,
	kAreaPassage2 = 46,
	kAreaPassage3 = 48,
	kAreaThePowersOfTheAbyss = 50,
	kAreaTheConcourse = 51,
	kAreaPassage4 = 52,
	kAreaPassage5 = 54,
	kAreaDreamsOfSlime = 55,
	kAreaGuardRoom = 56,
	kAreaAHiddenPlace = 57,
	kAreaAnteChamber = 58,
	kAreaPlacatingThePowers = 59,
	kAreaInThePresenceOfGod = 60,
	kAreaBirthOfADivineRace = 61,
	kAreaSaurasRepose = 62,
	kAreaTheThresholdOfTruth = 63,
	kAreaTheScriptures = 63,
	kAreaRefectory = 65,
	kAreaPassage6 = 66,
	kAreaPassage7 = 68,
	kAreaAblutions = 70,
	kAreaCell1 = 71,
	kAreaCell2 = 72,
	kAreaCell3 = 73,
	kAreaCell4 = 74,
	kAreaUpwardGallery = 75,
	kAreaGallery1 = 75,
	kAreaCavern = 77,
	kAreaGallery2 = 78,
	kAreaGallery3 = 79,
	kAreaGallery4 = 82,
	kAreaDownwardGallery = 83,
	kAreaGallery5 = 83,
	kAreaGallery6 = 85,
	kAreaPinkGallery1 = 87,
	kAreaGallery7 = 88,
	kAreaPinkGallery2 = 90,
	kAreaPinkGallery3 = 91,
	kAreaTube = 93,
	kAreaUnderwater = 94,
	kAreaObscurity = 101
};

} // End of namespace Chamber

#endif
