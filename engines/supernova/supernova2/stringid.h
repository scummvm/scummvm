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

#ifndef SUPERNOVA2_STRINGID_H
#define SUPERNOVA2_STRINGID_H

#include "common/scummsys.h"

namespace Supernova {
enum StringId2 {
	// 36
	kString23 = 36, kString24, kString25, kString26,
	kString27, kStringGenericInteract1, kStringGenericInteract2, kStringGenericInteract3, kStringGenericInteract4,
	kStringGenericInteract5, kStringGenericInteract6, kStringGenericInteract7, kStringGenericInteract8, kStringGenericInteract9,
	// 50
	kStringGenericInteract10, kStringGenericInteract11, kStringGenericInteract12, kStringGenericInteract13, kStringIntro1,
	kStringIntro2, kStringIntro3, kStringIntro4, kStringIntro5, kStringOutro1,
	kStringOutro2, kStringOutro3, kStringOutro4, kStringOutro5, kStringShout1,
	kStringShout2, kStringShout3, kStringShout4, kStringShout5, kStringShout6,
	kStringShout7, kStringShout8, kStringShout9, kStringShout10, kStringShout11,
	// 75
	kStringShout12, kStringShout13, kStringShout14, kStringShout15, kStringMuseum8,
	kStringMuseum9, kStringMuseum7, kStringMuseum6, kStringMuseum15, kStringMuseum16,
	kStringMuseum10, kStringMuseum12, kStringMuseum13, kStringMuseum14, kStringMuseum1,
	kStringMuseum2, kStringMuseum3, kStringMuseum4, kStringMuseum5, kStringMuseum23,
	kStringMuseum24, kStringMuseum17, kStringMuseum18, kStringMuseum19, kStringMuseum20,
	// 100
	kStringMuseum21, kStringMuseum22, kStringDinosaur, kStringDinosaurDescription, kStringEntrance,
	kStringDoor, kStringRoad, kStringCamera, kStringCameraDescription, kStringMainEntrance,
	kStringCorridor, kStringDinosaurDescription2, kStringDinosaurHead, kStringDinosaurHeadDescription, kStringAlarmSystem,
	kStringSuctionCup, kStringWall, kStringOpening, kStringLetter, kStringMassive,
	kStringInscriptionDescription, kStringPyramid0, kStringPyramid1, kStringPyramid2, kStringPyramid3,
	// 125
	kStringPyramid4, kStringPiece, kStringPyramid5, kStringPyramid6, kStringPyramid7,
	kStringPyramid8, kStringPyramid9, kStringPyramid10, kStringPyramid11, kStringPyramid12,
	kStringPyramid13, kStringPyramid14, kStringPyramid15, kStringPyramid17, kStringPyramid18,
	kStringPyramid19, kStringDirection1, kStringDirection2, kStringDirection3, kStringDirection4,
	kStringRope, kStringSign, kStringSignDescription, kStringEntrance1Description, kStringPyramid,
	// 150
	kStringPyramidDescription, kStringSun, kStringSunDescription, kStringSign5Description, kStringRight,
	kStringLeft, kStringButton, kStringInscription, kStringTomato, kStringFunnyDescription,
	kStringKnife1, kStringKnife1Description, kStringMonster, kStringRopeDescription, kStringEyes,
	kStringMouth, kStringMonster1Description, kStringNote, kStringNoteDescription, kStringOpeningDescription1,
	kStringOpeningDescription2, kStringNoteDescription1, kStringSlot, kStringSlotDescription3, kStringOpeningDescription3,
	// 175
	kStringStones, kStringPlate, kStringCoffin, kStringExit, kStringCreepy,
	kStringToothbrush, kStringToothbrushDescription1, kStringToothpaste, kStringBall, kStringBallDescription,
	kStringEye, kStringEyeDescription, kStringLooksMetal, kStringTaxiArrives, kStringNothingHappens,
	kStringEmpty, kStringWalletOpen, kStringAttachMagnet, kStringPoleMagnet, kStringCunning,
	kStringMustBuyFirst, kStringInsertChip, kStringTransferCD, kStringCDNotInserted, kStringRemoveChip,
	// 200
	kStringChipNotInserted, kStringWhatFor, kStringMMCD, kStringChipEmpty, kStringListeningToCD,
	kStringNoChip, kStringTipsy, kStringXa, kStringAirportEntrance, kStringAirport,
	kStringDowntown, kStringCulturePalace, kStringEarth, kStringPrivateApartment, kStringLeaveTaxi,
	kStringPay, kStringAddress, kStringCheater, kStringNotEnoughMoney, kStringTaxiAccelerating,
	kString5MinutesLater, kStringAlreadyHavePole, kStringSawPole, kStringOnlyShop, kStringCabinOccupiedSay,
	// 225
	kStringTakeMoney, kStringAlreadyPaid, kStringNoMoney, kStringPay10Xa, kStringWillPassOut,
	kStringRest, kStringCypher, kStringWillTakeIt, kStringTooExpensive, kStringWouldBuy,
	kStringMeHorstHummel, kStringHaveMusicChip, kStringGreatMask, kStringThreeYears, kStringStrongDrink,
	kStringMusicDevice, kStringArtusToothbrush, kStringSellInBulk, kStringRarityBooks, kStringEncyclopedia,
	kStringLargestDictionary, kStringOver400Words, kStringNotSale, kStringGaveOne, kStringExcited,
	// 250
	kStringFromGame, kStringRobust, kStringCheapSwill, kStringStickers, kStringDishes,
	kStringUgly, kStringSellsWell, kStringThatCosts, kStringTakeALook, kStringNonsense,
	kStringImSorry, kStringGoodEvening, kStringHello, kStringScaredMe, kStringHowSo,
	kStringDisguise, kStringWhatDisguise, kStringStopPretending, kStringYouDisguised, kStringIAmHorstHummel,
	kStringGiveItUp, kStringGestures, kStringMovesDifferently, kStringHeIsRobot, kStringYouAreCrazy,
	// 275
	kStringYouIdiot, kStringShutUp, kStringKnife, kStringKnifeDescription, kStringMoney,
	kStringDiscman, kStringDiscmanDescription, kStringSuctionCupDescription, kStringSpecialCard, kStringSpecialCardDescription,
	kStringAlarmCracker, kStringAlarmCrackerDescription, kStringKeycard, kStringSpaceship, kStringSpaceshipDescription,
	kStringVehicles, kStringVehiclesDescription, kStringVehicle, kStringVehicleDescription, kStringEntranceDescription,
	kStringWallet, kStringWalletDescription, kStringDevice, kStringDeviceDescription, kStringIdCard,
	// 300
	kStringIdCardDescription, kStringStaircase, kStringStaircaseDescription, kStringBusinessStreet, kStringBusinessStreetDescription,
	kStringRod, kStringPost, kStringRailing, kStringPoster, kStringPosterDescription,
	kStringCabin, kStringCabinFree, kStringCabinOccupied, kStringFeet, kStringFeetDescription,
	kStringHood, kStringHoodDescription, kString400Xa, kString10Xa, kStringSlotDescription1,
	kStringSlotDescription2, kStringChair, kStringChairDescription, kStringScribble, kStringFace,
	// 325
	kStringFaceDescription, kStringBooks, kStringDictionary, kStringPlant, kStringMask,
	kStringSnake, kStringCup, kStringJoystick, kStringToothbrushDescription, kStringMusic,
	kStringMusicDescription, kStringBottle, kStringBottleDescription, kStringBox, kStringSeller,
	kStringWhat, kStringNotInformed, kStringHorstHummel, kStringNiceWeather, kStringTellTicket,
	kStringHereIsXa, kString500Xa, kString1000Xa, kString5000Xa, kString10000Xa,
	// 350
	kStringThankYou, kStringWhatYouOffer, kStringHello2, kStringWhatYouWant, kStringWhoAreYou,
	kStringHorstHummel2, kStringNeverHeard, kStringYouDontKnow, kStringImOnTV, kStringIDontKnow,
	kStringFunny, kStringAha, kStringICan, kStringFromWhom, kStringCost,
	kStringAsYouSay, kStringGetCard, kStringOnlyParticipation, kStringWhatForIt, kStringMakeOffer,
	kStringGoodOffer, kStringGiveCard, kStringIdiot, kStringCheckout1, kStringCheckout2,
	// 375
	kStringCheckout3, kStringYes2, kStringNo2, kStringCheckout4, kStringCheckout5,
	kStringCheckout6, kStringCheckout7, kStringCheckout8, kStringCheckout9, kStringCheckout10,
	kStringCheckout11, kStringCheckout12, kStringCheckout13, kStringCheckout14, kStringCheckout15,
	kStringCheckout16, kStringCheckout17, kStringCheckout18, kStringCheckout19, kStringCheckout20,
	kStringCheckout21, kStringCheckout22, kStringCheckout23, kStringCheckout24, kStringCheckout25,
	// 400
	kStringCheckout26, kStringCheckout27, kStringCheckout28, kStringCheckout29, kStringCheckout30,
	kStringCheckout31, kStringCheckout32, kStringCheckout33, kStringCheckout34, kStringCheckout35,
	kStringCheckout36, kStringCheckout37, kStringCheckout38, kStringCheckout39, kStringCheckout40,
	kStringCheckout41, kStringCheckout42, kStringCheckout43, kStringCheckout44, kStringCheckout45,
	kStringCheckout46, kStringCheckout47, kStringCheckout48, kStringCheckout49, kStringAtMusicContest,
	// 425
	kStringNoImitation, kStringGoodJoke, kStringCommon, kStringIWillProof, kStringIWillPerform,
	kStringAppearance32, kStringAppearance1, kStringAppearance2, kStringAppearance3, kStringAppearance4,
	kStringAppearance5, kStringAppearance6, kStringAppearance7, kStringAppearance8, kStringAppearance9,
	kStringAppearance10, kStringAppearance11, kStringAppearance12, kStringAppearance13, kStringAppearance14,
	kStringAppearance15, kStringAppearance16, kStringAppearance17, kStringAppearance18, kStringAppearance19,
	// 450
	kStringAppearance20, kStringAppearance21, kStringAppearance22, kStringAppearance23, kStringAppearance24,
	kStringAppearance25, kStringAppearance26, kStringAppearance27, kStringAppearance28, kStringAppearance29,
	kStringAppearance30, kStringAppearance31, kStringElevator1, kStringElevator2, kStringElevator3,
	kStringElevator4, kStringElevator5, kStringElevator6, kStringElevator7, kStringElevator8,
	kStringElevator9, kStringElevator10, kStringElevator11, kStringElevator12, kStringElevator13,
	// 475
	kStringElevator14, kStringElevator15, kStringElevator16, kStringElevator17, kStringElevator18,
	kStringElevator19, kStringElevator20, kStringElevator21, kStringElevator22, kStringElevator23,
	kStringElevator24, kStringElevator25, kStringElevator26, kStringElevator27, kStringElevator28,
	kStringElevator29, kStringElevator30, kStringElevator31, kStringElevator32, kStringElevator33,
	kStringElevator34, kStringElevator35, kStringElevator36, kStringElevator37, kStringElevator38,
	// 500
	kStringElevator39, kStringElevator40, kStringElevator41, kStringElevator42, kStringElevator43,
	kStringElevator44, kStringElevator45, kStringElevator46, kStringElevator47, kStringElevator48,
	kStringElevator49, kStringElevator50, kStringElevator51, kStringElevator52, kStringElevator53,
	kStringElevator54, kStringElevator55, kStringElevator56, kStringElevator57, kStringElevator58,
	kStringElevator59, kStringElevator60, kStringElevator61, kStringElevator62, kStringElevator63,
	// 525
	kStringElevator64, kStringElevator65, kStringApartment1, kStringApartment2, kStringApartment3,
	kStringApartment4, kStringApartment5, kStringApartment6, kStringApartment7, kStringApartment8,
	kStringShip0, kStringShip1, kStringShip2, kStringShip3, kStringShip4,
	kStringShip5, kStringShip6, kStringShip7, kStringShip8, kStringShip9,
	kStringShip10, kStringShip11, kStringShip12, kStringShip13, kStringShip14,
	// 550
	kStringShip15, kStringShip16, kStringShip17, kStringShip18, kStringShip19,
	kStringFascinating, kStringTaxis, kStringTaxisDescription, kStringAxacussan, kStringParticipationCard,
	kStringAxacussian, kStringSign1Description, kStringSign2Description, kStringSign3Description, kStringSign4Description,
	kStringBell, kStringDisplay, kStringKeypad, kStringKeypadDescription, kStringChip,
	kStringChipDescription, kStringHatch, kStringHatchDescription, kStringMusicSystem, kStringMusicSystemDescription,
	// 575
	kStringSpeakers, kStringSpeakersDescription, kStringPencils, kStringPencilsDescription, kStringMetalBlocks,
	kStringMetalBlocksDescription, kStringImage, kStringImageDescription, kStringCabinet, kStringCabinetDescription,
	kStringElevator, kStringUnderBed, kStringUnderBedDescription, kStringKey, kStringKeyDescription,
	kStringSwitch, kStringHandle, kStringHatch2, kStringSpaceSuit, kStringSpaceSuitDescription,
	kStringCable, kStringCableDescription1, kStringCableDescription2, kStringIntro6, kStringIntro7,
	// 600
	kStringIntro8, kStringIntroTV1, kStringIntroTV2, kStringIntroTV3, kStringIntroTV4,
	kStringIntroTV5, kStringIntroTV6, kStringIntroTV7, kStringIntroTV8, kStringIntroTV9,
	kStringIntroTV10, kStringIntroTV11, kStringIntroTV12, kStringIntroTV13, kStringIntroTV14,
	kStringIntroTV15, kStringIntroTV16, kStringIntro9, kStringIntro10, kStringIntro11,
	kStringIntro12, kStringIntro13, kStringIntro14, kStringMonsterDescription, kStringPyramid16,
	// 625
	kStringMuseum11
};
}

#endif // SUPERNOVA2_STRINGID_H
