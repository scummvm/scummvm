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

#ifndef SUPERNOVA_STRINGID_H
#define SUPERNOVA_STRINGID_H

#include "common/scummsys.h"

namespace Supernova {

enum StringId1 {
	// 36
	kStringTitleVersion = 36, kStringTitle1, kStringTitle2, kStringTitle3,
	kStringIntro1, kStringIntro2, kStringIntro3, kStringIntro4, kStringIntro5,
	kStringIntro6, kStringIntro7, kStringIntro8, kStringIntro9, kStringIntro10,
	// 50
	kStringIntro11, kStringIntro12, kStringIntro13, kStringBroken, kStringTakeMessage,
	kStringKeycard, kStringKeycardDescription, kStringKnife, kStringKnifeDescription, kStringWatch,
	kStringDiscman, kStringDiscmanDescription, kStringHatch, kStringButton, kStringHatchButtonDescription,
	kStringLadder, kStringExit, kStringCockpitHatchDescription, kStringKitchenHatchDescription, kStringStasisHatchDescription,
	kStringStasisHatchDescription2, kStringSlot, kStringSlotDescription, kStringCorridor, kStringComputer,
	// 75
	kStringComputerPassword, kStringInstruments, kStringInstrumentsDescription1, kStringMonitor, kStringMonitorDescription,
	kStringImage, kStringGenericDescription1, kStringGenericDescription2, kStringGenericDescription3, kStringGenericDescription4,
	kStringMagnete, kStringMagneteDescription, kStringPen, kStringPenDescription, kStringShelf,
	kStringCompartment, kStringSocket, kStringToilet, kStringPistol, kStringPistolDescription,
	kStringBooks, kStringBooksDescription, kStringSpool, kStringSpoolDescription, kStringBook,
	// 100
	kStringUnderwear, kStringUnderwearDescription, kStringClothes, kStringJunk, kStringJunkDescription,
	kStringFolders, kStringFoldersDescription, kStringPoster, kStringPosterDescription1, kStringPosterDescription2,
	kStringSpeaker, kStringRecord, kStringRecordDescription, kStringRecordStand, kStringRecordStandDescription,
	kStringTurntable, kStringTurntableDescription, kStringWire, kStringPlug, kStringImageDescription1,
	kStringDrawingInstruments, kStringDrawingInstrumentsDescription, kStringChessGame, kStringChessGameDescription1, kStringTennisRacket,
	// 125
	kStringTennisRacketDescription, kStringTennisBall, kStringChessGameDescription2, kStringBed, kStringBedDescription,
	kStringCompartmentDescription, kStringAlbums, kStringAlbumsDescription, kStringRope, kStringRopeDescription,
	kStringShelfDescription, kStringClothesDescription, kStringSocks, kStringBookHitchhiker, kStringBathroom,
	kStringBathroomDescription, kStringShower, kStringHatchDescription1, kStringHatchDescription2, kStringHelmet,
	kStringHelmetDescription, kStringSuit, kStringSuitDescription, kStringLifeSupport, kStringLifeSupportDescription,
	// 150
	kStringScrap, kStringScrapDescription1, kStringTerminalStrip, kStringScrapDescription2, kStringReactor,
	kStringReactorDescription, kStringNozzle, kStringPumpkin, kStringPumpkinDescription, kStringLandingModule,
	kStringLandingModuleDescription, kStringHatchDescription3, kStringGenerator, kStringGeneratorDescription, kStringScrapDescription3,
	kSafetyButtonDescription, kStringKeyboard, kStringGeneratorWire, kStringEmptySpool, kStringKeycard2,
	kStringKeycard2Description, kStringTrap, kStringVoltmeter, kStringClip, kStringWireDescription,
	// 175
	kStringStone, kStringCaveOpening, kStringCaveOpeningDescription, kStringExitDescription, kStringCave,
	kStringSign, kStringSignDescription, kStringEntrance, kStringStar, kStringSpaceshift,
	kStringPorter, kStringPorterDescription, kStringDoor, kStringChewingGum, kStringGummyBears,
	kStringChocolateBall, kStringEgg, kStringLiquorice, kStringPill, kStringPillDescription,
	kStringVendingMachine, kStringVendingMachineDescription, kStringToiletDescription, kStringStaircase, kStringCoins,
	// 200
	kStringCoinsDescription, kStringTabletPackage, kStringTabletPackageDescription, kStringChair, kStringShoes,
	kStringShoesDescription, kStringFrogFace, kStringScrible, kStringScribleDescription, kStringWallet,
	kStringMenu, kStringMenuDescription, kStringCup, kStringCupDescription, kStringBill,
	kStringBillDescription, kStringKeycard3, kStringAnnouncement, kStringAnnouncementDescription, kStringRoger,
	kStringUfo, kStringUfoDescription, kStringTray, kStringTrayDescription, kStringLamp,
	// 225
	kStringLampDescription, kStringEyes, kStringEyesDescription, kStringSocketDescription, kStringMetalBlock,
	kStringMetalBlockDescription, kStringRobot, kStringRobotDescription, kStringTable, kStringTableDescription,
	kStringCellDoor, kStringCellDoorDescription, kStringLaptop, kStringWristwatch, kStringPillar,
	kStringDoorDescription1, kStringDoorDescription2, kStringDoorDescription3, kStringDoorDescription4, kStringDontEnter,
	kStringAxacussan, kStringAxacussanDescription, kStringImageDescription2, kStringMastercard, kStringMastercardDescription,
	// 250
	kStringLamp2, kStringGenericDescription5, kStringMoney, kStringMoneyDescription1, kStringLocker,
	kStringLockerDescription, kStringLetter, kStringCube, kStringGenericDescription6, kStringGenericDescription7,
	kStringStrangeThing, kStringGenericDescription8, kStringImageDescription3, kStringPlant, kStringStatue,
	kStringStatueDescription, kStringPlantDescription, kStringComputerDescription, kStringGraffiti, kStringGraffitiDescription,
	kStringMoneyDescription2, kStringJungle, kStringJungleDescription, kStringOutro1, kStringOutro2,
	// 275
	kStringOutro3, kStringOutro4, kStringOutro5, kStringOutro6, kStringOutro7,
	kStringOutro8, kStringOutro9, kStringOutro10, kStringOutro11, kStringOutro12,
	kStringOutro13, kStringOutro14, kStringWireAndPlug, kStringWireAndClip, kStringWireAndPlug2,
	kStringSignDescription2, kStringCoin, kStringDoorDescription5, kStringDoorDescription6, kStringKeycard2Description2,
	kSringSpoolAndClip, kStringIntroCutscene1, kStringIntroCutscene2, kStringIntroCutscene3, kStringIntroCutscene4,
	// 300
	kStringIntroCutscene5, kStringIntroCutscene6, kStringIntroCutscene7, kStringIntroCutscene8, kStringIntroCutscene9,
	kStringIntroCutscene10, kStringIntroCutscene11, kStringIntroCutscene12, kStringIntroCutscene13, kStringIntroCutscene14,
	kStringIntroCutscene15, kStringIntroCutscene16, kStringIntroCutscene17, kStringIntroCutscene18, kStringIntroCutscene19,
	kStringIntroCutscene20, kStringIntroCutscene21, kStringIntroCutscene22, kStringIntroCutscene23, kStringIntroCutscene24,
	kStringIntroCutscene25, kStringIntroCutscene26, kStringIntroCutscene27, kStringIntroCutscene28, kStringIntroCutscene29,
	// 325
	kStringIntroCutscene30, kStringIntroCutscene31, kStringIntroCutscene32, kStringIntroCutscene33, kStringIntroCutscene34,
	kStringIntroCutscene35, kStringIntroCutscene36, kStringIntroCutscene37, kStringIntroCutscene38, kStringIntroCutscene39,
	kStringIntroCutscene40, kStringIntroCutscene41, kStringIntroCutscene42, kStringShipHall1, kStringShipSleepCabin1,
	kStringShipSleepCabin2,  kStringShipSleepCabin3,  kStringShipSleepCabin4,  kStringShipSleepCabin5,  kStringShipSleepCabin6,
	kStringShipSleepCabin7,  kStringShipSleepCabin8,  kStringShipSleepCabin9,  kStringShipSleepCabin10, kStringShipSleepCabin11,
	// 350
	kStringShipSleepCabin12, kStringShipSleepCabin13, kStringShipSleepCabin14, kStringShipSleepCabin15, kStringShipSleepCabin16,
	kStringShipCockpit1, kStringShipCockpit2, kStringShipCockpit3, kStringShipCockpit4, kStringShipCockpit5,
	kStringShipCockpit6, kStringShipCockpit7, kStringShipCockpit8, kStringShipCockpit9, kStringShipCockpit10,
	kStringShipCockpit11, kStringShipCockpit12, kStringShipCockpit13, kStringShipCabinL3_1, kStringShipCabinL3_2,
	kStringShipCabinL3_3, kStringShipCabinL3_4, kStringShipCabinL3_5, kStringShipAirlock1, kStringShipAirlock2,
	// 375
	kStringShipAirlock3, kStringShipAirlock4, kStringShipHold1, kStringCable1, kStringCable2,
	kStringCable3, kStringCable4, kStringShipHold2, kStringShipHold3, kStringShipHold4,
	kStringShipHold5, kStringShipHold6, kStringShipHold7, kStringShipHold8, kStringShipHold9,
	kStringShipHold10, kStringShipHold11, kStringShipHold12, kStringShipHold13, kStringShipHold14,
	kStringShipHold15, kStringShipHold16, kStringArsanoMeetup1, kStringArsanoMeetup2, kStringArsanoMeetup3,
	// 400
	kStringArsanoEntrance1, kStringArsanoEntrance2, kStringArsanoEntrance3, kStringArsanoEntrance4, kStringArsanoEntrance5,
	kStringArsanoEntrance6, kStringArsanoEntrance7, kStringArsanoEntrance8, kStringArsanoEntrance9, kStringArsanoEntrance10,
	kStringArsanoEntrance11, kStringArsanoEntrance12, kStringArsanoEntrance13, kStringArsanoEntrance14, kStringArsanoEntrance15,
	kStringArsanoEntrance16, kStringArsanoEntrance17, kStringArsanoEntrance18, kStringArsanoEntrance19, kStringArsanoEntrance20,
	kStringArsanoEntrance21, kStringArsanoEntrance22, kStringArsanoEntrance23, kStringArsanoEntrance24, kStringArsanoEntrance25,
	// 425
	kStringArsanoEntrance26, kStringArsanoEntrance27, kStringArsanoDialog1, kStringArsanoDialog2, kStringArsanoDialog3,
	kStringArsanoDialog4, kStringArsanoDialog5, kStringArsanoDialog6, kStringArsanoDialog7, kStringArsanoDialog8,
	kStringArsanoDialog9, kStringDialogArsanoRoger1, kStringDialogArsanoRoger2, kStringDialogArsanoRoger3, kStringDialogArsanoMeetup3_1,
	kStringDialogArsanoMeetup3_2, kStringDialogArsanoMeetup3_3, kStringDialogArsanoMeetup3_4, kStringDialogArsanoMeetup3_5, kStringArsanoRoger1,
	kStringArsanoRoger2, kStringArsanoRoger3, kStringArsanoRoger4, kStringArsanoRoger5, kStringArsanoRoger6,
	// 450
	kStringArsanoRoger7, kStringArsanoRoger8, kStringArsanoRoger9, kStringArsanoRoger10, kStringArsanoRoger11,
	kStringArsanoRoger12, kStringArsanoRoger13, kStringArsanoRoger14, kStringArsanoRoger15, kStringArsanoRoger16,
	kStringArsanoRoger17, kStringArsanoRoger18, kStringArsanoRoger19, kStringArsanoRoger20, kStringArsanoRoger21,
	kStringArsanoRoger22, kStringArsanoRoger23, kStringArsanoRoger24, kStringArsanoRoger25, kStringArsanoRoger26,
	kStringArsanoRoger27, kStringArsanoRoger28, kStringArsanoRoger29, kStringArsanoRoger30, kStringArsanoRoger31,
	// 475
	kStringArsanoRoger32, kStringArsanoRoger33, kStringArsanoRoger34, kStringArsanoRoger35, kStringArsanoRoger36,
	kStringArsanoRoger37, kStringArsanoRoger38, kStringArsanoRoger39, kStringArsanoRoger40, kStringArsanoGlider1,
	kStringArsanoMeetup2_1, kStringArsanoMeetup2_2, kStringArsanoMeetup2_3, kStringArsanoMeetup2_4, kStringArsanoMeetup2_5,
	kStringArsanoMeetup2_6, kStringArsanoMeetup2_7, kStringArsanoMeetup2_8, kStringArsanoMeetup2_9, kStringArsanoMeetup2_10,
	kStringArsanoMeetup2_11, kStringArsanoMeetup2_12, kStringArsanoMeetup2_13, kStringArsanoMeetup3_1, kStringArsanoMeetup3_2,
	// 500
	kStringArsanoMeetup3_3, kStringArsanoMeetup3_4, kStringArsanoMeetup3_5, kStringArsanoMeetup3_6, kStringArsanoMeetup3_7,
	kStringArsanoMeetup3_8, kStringArsanoMeetup3_9, kStringArsanoMeetup3_10, kStringArsanoMeetup3_11, kStringArsanoMeetup3_12,
	kStringArsanoMeetup3_13, kStringArsanoMeetup3_14, kStringArsanoMeetup3_15, kStringArsanoMeetup3_16, kStringArsanoMeetup3_17,
	kStringArsanoMeetup3_18, kStringArsanoMeetup3_19, kStringArsanoMeetup3_20, kStringArsanoMeetup3_21, kStringArsanoMeetup3_22,
	kStringArsanoMeetup3_23, kStringArsanoMeetup3_24, kStringArsanoMeetup3_25, kStringArsanoMeetup3_26, kStringArsanoMeetup3_27,
	// 525
	kStringArsanoMeetup3_28, kStringAxacussCell_1, kStringAxacussCell_2, kStringAxacussCell_3, kStringAxacussCell_4,
	kStringAxacussCell_5, kStringOk, kStringDialogArsanoMeetup2_1, kStringDialogArsanoMeetup2_2, kStringDialogArsanoMeetup2_3,
	kStringDialogArsanoMeetup2_4, kStringDialogArsanoMeetup2_5, kStringDialogArsanoMeetup2_6, kStringDialogArsanoMeetup2_7, kStringDialogArsanoMeetup2_8,
	kStringDialogArsanoMeetup2_9, kStringDialogArsanoMeetup2_10, kStringDialogArsanoMeetup2_11, kStringDialogAxacussCorridor5_1, kStringDialogAxacussCorridor5_2,
	kStringDialogAxacussCorridor5_3, kStringDialogAxacussCorridor5_4, kStringDialogAxacussCorridor5_5, kStringDialogAxacussCorridor5_6, kStringDialogAxacussCorridor5_7,
	// 550
	kStringDialogX1, kStringDialogX2, kStringDialogX3, kStringAxacussCorridor5_1, kStringAxacussCorridor5_2,
	kStringAxacussCorridor5_3, kStringAxacussCorridor5_4, kStringAxacussCorridor5_5, kStringAxacussCorridor5_6, kStringAxacussCorridor5_7,
	kStringAxacussBcorridor_1, kStringAxacussOffice1_1, kStringAxacussOffice1_2, kStringAxacussOffice1_3, kStringAxacussOffice1_4,
	kStringAxacussOffice1_5, kStringAxacussOffice1_6, kStringAxacussOffice1_7, kStringAxacussOffice1_8, kStringAxacussOffice1_9,
	kStringAxacussOffice1_10, kStringAxacussOffice1_11, kStringAxacussOffice1_12, kStringAxacussOffice1_13, kStringAxacussOffice1_14,
	// 575
	kStringAxacussOffice1_15, kStringAxacussOffice1_16, kStringAxacussOffice3_1, kStringAxacussElevator_1, kStringAxacussElevator_2,
	kStringAxacussElevator_3, kStringShock, kStringShot, kStringCloseLocker_1, kStringIsHelmetOff_1,
	kStringGenericInteract_1, kStringGenericInteract_2, kStringGenericInteract_3, kStringGenericInteract_4, kStringGenericInteract_5,
	kStringGenericInteract_6, kStringGenericInteract_7, kStringGenericInteract_8, kStringGenericInteract_9, kStringGenericInteract_10,
	kStringGenericInteract_11, kStringGenericInteract_12, kStringGenericInteract_13, kStringGenericInteract_14, kStringGenericInteract_15,
	// 600
	kStringGenericInteract_16, kStringGenericInteract_17, kStringGenericInteract_18, kStringGenericInteract_19, kStringGenericInteract_20,
	kStringGenericInteract_21, kStringGenericInteract_22, kStringGenericInteract_23, kStringGenericInteract_24, kStringGenericInteract_25,
	kStringGenericInteract_26, kStringGenericInteract_27, kStringGenericInteract_28, kStringGenericInteract_29, kStringGenericInteract_30,
	kStringGenericInteract_31, kStringGenericInteract_32, kStringGenericInteract_33, kStringGenericInteract_34, kStringGenericInteract_35,
	kStringGenericInteract_36, kStringGenericInteract_37, kStringGenericInteract_38, kStringGenericInteract_39, kStringGenericInteract_40,
	// 625
	kStringGenericInteract_41, kStringGenericInteract_42, kStringGenericInteract_43, kStringSupernova1, kStringSupernova2,
	kStringSupernova3, kStringSupernova4, kStringSupernova5, kStringSupernova6, kStringSupernova7,
	kStringSupernova8, kStringGuardNoticed1, kStringGuardNoticed2, kStringTelomat1, kStringTelomat2,
	kStringTelomat3, kStringTelomat4, kStringTelomat5, kStringTelomat6, kStringTelomat7,
	kStringTelomat8, kStringTelomat9, kStringTelomat10, kStringTelomat11, kStringTelomat12,
	// 650
	kStringTelomat13, kStringTelomat14, kStringTelomat15, kStringTelomat16, kStringTelomat17,
	kStringTelomat18, kStringTelomat19, kStringTelomat20, kStringTelomat21, kStringAlarm,
	// Add two placeholder strings at the end for variable text
	kStringPlaceholder1, kStringPlaceholder2,

	// String for money in inventory
	kStringInventoryMoney
};
}
#endif // SUPERNOVA_STRINGID_H
