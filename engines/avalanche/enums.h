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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

#ifndef AVALANCHE_ENUMS_H
#define AVALANCHE_ENUMS_H

namespace Avalanche {
enum Color {
	kColorBlack,      kColorBlue,      kColorGreen,     kColorCyan,         kColorRed,
	kColorMagenta,    kColorBrown,     kColorLightgray, kColorDarkgray,     kColorLightblue,
	kColorLightgreen, kColorLightcyan, kColorLightred,  kColorLightmagenta, kColorYellow,
	kColorWhite
};

// CHECKME: kRoomBossKey is a guess
enum Room {
	kRoomNowhere = 0,       kRoomYours = 1,        kRoomOutsideYours = 2,          kRoomOutsideSpludwicks = 3,
	kRoomYourHall = 5,      kRoomMusicRoom = 7,    kRoomOutsideArgentPub = 9,      kRoomArgentRoad = 10,
	kRoomWiseWomans = 11,   kRoomSpludwicks = 12,  kRoomInsideAbbey = 13,          kRoomOutsideAbbey = 14,
	kRoomAvvysGarden = 15,  kRoomAylesOffice = 16, kRoomArgentPub = 19,            kRoomBrummieRoad = 20,
	kRoomBridge = 21,       kRoomLusties = 22,     kRoomLustiesRoom = 23,          kRoomWestHall = 25,
	kRoomEastHall = 26,     kRoomOubliette = 27,   kRoomGeidas = 28,               kRoomCatacombs = 29,
	kRoomEntranceHall = 40, kRoomRobins = 42,      kRoomOutsideNottsPub = 46,      kRoomNottsPub = 47,
	kRoomOutsideDucks = 50, kRoomDucks = 51,       kRoomOutsideCardiffCastle = 70, kRoomInsideCardiffCastle = 71,
	kRoomBossKey = 98,      kRoomMap = 99,         kRoomDummy = 177 // Dummy room
};

// Objects you can hold:
enum Object {
	kObjectWine = 1, kObjectMoney,    kObjectBodkin,       kObjectPotion, kObjectChastity,
	kObjectBolt,     kObjectCrossbow, kObjectLute,         kObjectBadge,  kObjectMushroom,
	kObjectKey,      kObjectBell,     kObjectPrescription, kObjectPen,    kObjectInk,
	kObjectClothes,  kObjectHabit,    kObjectOnion,        kObjectDummy = 177
};

// People who hang around this game.
enum People {
	// Boys:
	kPeopleAvalot = 150,     kPeopleSpludwick = 151, kPeopleCrapulus = 152,  kPeopleDrDuck = 153,
	kPeopleMalagauche = 154, kPeopleFriarTuck = 155, kPeopleRobinHood = 156, kPeopleCwytalot = 157,
	kPeopleDuLustie = 158,   kPeopleDuke = 159,      kPeopleDogfood = 160,   kPeopleTrader = 161,
	kPeopleIbythneth = 162,  kPeopleAyles = 163,     kPeoplePort = 164,      kPeopleSpurge = 165,
	kPeopleJacques = 166,
	// Girls:
	kPeopleArkata = 175,     kPeopleGeida = 176,     kPeopleInvisible = 177, kPeopleWisewoman = 178,
	//
	kPeoplePardon = 254,     kPeopleNone = 0
};

enum VerbCode {
	kVerbCodeExam = 1,     kVerbCodeOpen = 2,    kVerbCodePause = 3,   kVerbCodeGet = 4,       kVerbCodeDrop = 5,
	kVerbCodeInv = 6,      kVerbCodeTalk = 7,    kVerbCodeGive = 8,    kVerbCodeDrink = 9,     kVerbCodeLoad = 10,
	kVerbCodeSave = 11,    kVerbCodePay = 12,    kVerbCodeLook = 13,   kVerbCodeBreak = 14,    kVerbCodeQuit = 15,
	kVerbCodeSit = 16,     kVerbCodeStand = 17,  kVerbCodeGo = 18,     kVerbCodeInfo = 19,     kVerbCodeUndress = 20,
	kVerbCodeWear = 21,    kVerbCodePlay = 22,   kVerbCodeRing = 23,   kVerbCodeHelp = 24,     kVerbCodeLarrypass = 25,
	kVerbCodePhaon = 26,   kVerbCodeBoss = 27,   kVerbCodePee = 28,    kVerbCodeCheat = 29,    kVerbCodeMagic = 30,
	kVerbCodeRestart = 31, kVerbCodeEat = 32,    kVerbCodeListen = 33, kVerbCodeBuy = 34,      kVerbCodeAttack = 35,
	kVerbCodePasswd = 36,  kVerbCodeDir = 37,    kVerbCodeDie = 38,    kVerbCodeScore = 39,    kVerbCodePut = 40,
	kVerbCodeKiss = 41,    kVerbCodeClimb = 42,  kVerbCodeJump = 43,   kVerbCodeHiscores = 44, kVerbCodeWake = 45,
	kVerbCodeHello = 46,   kVerbCodeThanks = 47, 
	kVerbCodeSmartAlec = 249, kVerbCodeExpletive = 253, kVerbCodePardon = 254
};

} // End of namespace Avalanche

#endif // AVALANCHE_ENUMS_H
