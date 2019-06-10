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

#ifndef SUPERNOVA2_MS2_DEF_H
#define SUPERNOVA2_MS2_DEF_H

#include "common/scummsys.h"

namespace Supernova2 {

const int kTextSpeed[] = {19, 14, 10, 7, 4};
const int kMsecPerTick = 55;

const int kMaxSection = 40;
const int kMaxDialog = 2;
const int kMaxObject = 30;
const int kMaxCarry = 30;

const int kSleepAutosaveSlot = 999;

const byte kShownFalse = 0;
const byte kShownTrue = 1;

enum MessagePosition {
	kMessageNormal,
	kMessageLeft,
	kMessageRight,
	kMessageCenter,
	kMessageTop
};

enum ObjectType {
	NULLTYPE    =      0,
	TAKE        =      1,
	OPENABLE    =      2,
	OPENED      =      4,
	CLOSED      =      8,
	EXIT        =     16,
	PRESS       =     32,
	COMBINABLE  =     64,
	CARRIED     =    128,
	UNNECESSARY =    256,
	WORN        =    512,
	TALK        =   1024,
	OCCUPIED    =   2048,
	CAUGHT      =   4096
};
typedef uint16 ObjectTypes;

enum Action {
	ACTION_WALK,
	ACTION_LOOK,
	ACTION_TAKE,
	ACTION_OPEN,
	ACTION_CLOSE,
	ACTION_PRESS,
	ACTION_PULL,
	ACTION_USE,
	ACTION_TALK,
	ACTION_GIVE
};

enum RoomId {
	INTRO,AIRPORT,TAXISTAND,STREET,GAMES,CABIN,KIOSK,
	CULTURE_PALACE,CHECKOUT,CITY1,CITY2,ELEVATOR,APARTMENT,SHIP,

	PYRAMID,PYR_ENTRANCE,UPSTAIRS1,DOWNSTAIRS1,
	BOTTOM_RIGHT_DOOR,BOTTOM_LEFT_DOOR,UPSTAIRS2,DOWNSTAIRS2,
	UPPER_DOOR,PUZZLE_FRONT,PUZZLE_BEHIND,
	FORMULA1_F,FORMULA1_N,FORMULA2_F,FORMULA2_N,TOMATO_F,TOMATO_N,
	MONSTER_F,MONSTER1_N,MONSTER2_N,UPSTAIRS3,DOWNSTAIRS3,
	LGANG1,LGANG2,HOLE_ROOM,IN_HOLE,FLOORDOOR,FLOORDOOR_U,
	BST_DOOR,HALL,COFFIN_ROOM,MASK,

	MUSEUM,MUS_EING,MUS1,MUS2,MUS3,MUS4,MUS5,MUS6,MUS7,MUS8,
	MUS9,MUS10,MUS11,MUS_RUND,
	MUS12,MUS13,MUS14,MUS15,MUS16,MUS17,MUS18,MUS19,MUS20,MUS21,MUS22,
	NUMROOMS,NULLROOM
};

enum ObjectId {
	INVALIDOBJECT = -1,
	NULLOBJECT = 0,
	TAXI=1,WALLET,TRANSMITTER,KNIFE,ROD,OCCUPIED_CABIN,MONEY,
	SLOT1,CHAIR,CORRIDOR,G_RIGHT,G_LEFT,PYRA_ENTRANCE,DOOR,BUTTON,
	PART0,PART1,PART2,PART3,PART4,PART5,PART6,PART7,
	PART8,PART9,PART10,PART11,PART12,PART13,PART14,PART15,
	TKNIFE,ROPE,NOTE,MOUTH,
	HOLE1,HOLE2,HOLE3,HOLE4,HOLE5,HOLE6,HOLE7,HOLE8,HOLE9,HOLE10,
	HOLE11,HOLE12,HOLE13,HOLE14,HOLE15,HOLE16,HOLE17,HOLE18,HOLE19,HOLE20,
	HOLE21,HOLE22,HOLE23,HOLE24,HOLE25,SIGN,
	SLOT,HOLE,STONES,
	BST1,BST2,BST3,BST4,BST5,BST6,BST7,BST8,
	BST9,BST10,BST11,BST12,BST13,BST14,BST15,BST16,
	COFFIN,SUN,MONSTER,EYE,EYE1,EYE2,L_BALL,R_BALL,
	PRIZE,REAR_STREET,
	BMASK,BOTTLE,PLAYER,TOOTHBRUSH,BOOKS,LEXICON,PLANT,SNAKE,
	CUP,JOYSTICK,BOTTLE1,BOTTLE2,BOTTLE3,BOTTLE4,BOX,FACES,
	SELLER,POSTER,AXACUSSER,KP_ENTRANCE,CHIP,CARD,
	SCRIBBLE1,SCRIBBLE2,BELL,KEYPAD,DOOR_L,DOOR_R,ID_CARD,
	MAGNET,UNDER_BED,KEY,HATCH,CABINET,DISCMAN,
	SMALL_DOOR,BACK_MONEY,WALL,SUCTION_CUP,IMITATION,SP_KEYCARD,ALARM_CRACKER,
	ENCRYPTED_DOOR,ALARM_ANLAGE,MUS_STREET,BIG_DOOR,MUSIC_SYSTEM,
	HANDLE,SWITCH,DOOR_SWITCH,SUIT,CABLE,RCABLE,
	MUSCARD,SKOPF, DISPLAY
};

enum StringId {
	kNoString = -1,
kStringCommandGo, kStringCommandLook, kStringCommandTake, kStringCommandOpen, kStringCommandClose,
kStringCommandPress, kStringCommandPull, kStringCommandUse, kStringCommandTalk, kStringCommandGive,
kStringConversationEnd, kStringStatusCommandGo, kStringStatusCommandLook, kStringStatusCommandTake, kStringStatusCommandOpen,
kStringStatusCommandClose, kStringStatusCommandPress, kStringStatusCommandPull, kStringStatusCommandUse, kStringStatusCommandTalk,
kStringStatusCommandGive, kPhrasalVerbParticleGiveTo, kPhrasalVerbParticleUseWith, kString23, kString24,
kString25, kString26, kString27, kStringTextSpeed, kString29,
kString30, kString31, kStringGenericInteract1, kStringGenericInteract2, kStringGenericInteract3,
kStringGenericInteract4, kStringGenericInteract5, kStringGenericInteract6, kStringGenericInteract7, kStringGenericInteract8,
kStringGenericInteract9, kStringGenericInteract10, kStringGenericInteract11, kStringGenericInteract12, kStringGenericInteract13,
kStringIntro1, kStringIntro2, kStringIntro3, kStringIntro4, kStringIntro5,
kString50, kString51, kString52, kString53, kString54,
kString55, kString56, kString57, kString58, kString59,
kString60, kString61, kString62, kString63, kString64,
kString65, kString66, kString67, kString68, kString69,
kString70, kString71, kString72, kString73, kString74,
kString75, kString76, kString77, kString78, kString79,
kString80, kString81, kString82, kString83, kString84,
kString85, kString86, kString87, kString88, kString89,
kString90, kString91, kString92, kString93, kString94,
kStringEntrance, kStringDoor, kString97, kString98, kString99,
kString100, kStringCorridor, kString102, kStringDinosaurHead, kString104,
kString105, kStringSuctionCup, kString107, kStringOpening, kString109,
kStringMassive, kStringInscriptionDescription, kStringPyramid0, kStringPyramid1, kStringPyramid2,
kStringPyramid3, kStringPyramid4, kStringPiece, kStringPyramid5, kStringPyramid6,
kString120, kString121, kString122, kString123, kString124,
kString125, kString126, kString127, kString128, kString129,
kString130, kString131, kStringDirection1, kStringDirection2, kStringDirection3,
kStringDirection4, kStringRope, kStringSign, kStringSignDescription, kStringEntrance1Description,
kStringPyramid, kStringPyramidDescription, kStringSun, kStringSunDescription, kStringSign5Description,
kStringRight, kStringLeft, kStringButton, kStringInscription, kStringTomato,
kStringFunnyDescription, kStringKnife1, kStringKnife1Description, kStringMonster, kStringRopeDescription,
kStringEyes, kStringMouth, kStringMonster1Description, kStringNote, kStringNoteDescription,
kString160, kString161, kString162, kStringSlot, kString164,
kString165, kString166, kString167, kString168, kStringExit,
kString170, kStringToothbrush, kString172, kString173, kString174,
kString175, kString176, kString177, kStringDefaultDescription, kStringLooksMetal,
kStringTaxiArrives, kStringNothingHappens, kStringEmpty, kStringWalletOpen, kStringAttachMagnet,
kStringPoleMagnet, kStringCunning, kStringMustBuyFirst, kStringInsertChip, kStringTransferCD,
kStringCDNotInserted, kStringRemoveChip, kStringChipNotInserted, kStringWhatFor, kStringMMCD,
kStringChipEmpty, kStringListeningToCD, kStringNoChip, kStringTipsy, kString199,
kStringAirportEntrance, kStringAirport, kStringDowntown, kStringCulturePalace, kStringEarth,
kStringPrivateApartment, kStringLeaveTaxi, kStringPay, kStringAddress, kStringCheater,
kStringNotEnoughMoney, kStringTaxiAccelerating, kString5MinutesLater, kStringAlreadyHavePole, kStringSawPole,
kStringOnlyShop, kStringCabinOccupiedSay, kStringTakeMoney, kStringAlreadyPaid, kStringNoMoney,
kStringPay10Xa, kStringWillPassOut, kStringRest, kStringCypher, kStringWillTakeIt,
kStringTooExpensive, kStringWouldBuy, kStringMeHorstHummel, kStringHaveMusicChip, kStringGreatMask,
kStringThreeYears, kStringStrongDrink, kStringMusicDevice, kStringArtusToothbrush, kStringSellInBulk,
kStringRarityBooks, kStringEncyclopedia, kStringLargestDictionary, kStringOver400Words, kStringNotSale,
kStringGaveOne, kStringExcited, kStringFromGame, kStringRobust, kStringCheapSwill,
kStringStickers, kStringDishes, kStringUgly, kStringSellsWell, kStringThatCosts,
kStringTakeALook, kStringNonsense, kStringImSorry, kStringGoodEvening, kStringHello,
kStringScaredMe, kStringHowSo, kStringDisguise, kStringWhatDisguise, kStringStopPretending,
kStringYouDisguised, kStringIAmHorstHummel, kStringGiveItUp, kStringGestures, kStringMovesDifferently,
kStringHeIsRobot, kStringYouAreCrazy, kStringYouIdiot, kStringShutUp, kStringKnife,
kStringKnifeDescription, kStringMoney, kStringDiscman, kStringDiscmanDescription, kStringSuctionCupDescription,
kStringSpecialCard, kStringSpecialCardDescription, kStringAlarmCracker, kStringAlarmCrackerDescription, kStringKeycard,
kStringSpaceship, kStringSpaceshipDescription, kStringVehicles, kStringVehiclesDescription, kStringVehicle,
kStringVehicleDescription, kStringEntranceDescription, kStringWallet, kStringWalletDescription, kStringDevice,
kStringDeviceDescription, kStringIdCard, kStringIdCardDescription, kStringStaircase, kStringStaircaseDescription,
kStringBusinessStreet, kStringBusinessStreetDescription, kStringRod, kStringPost, kStringRailing,
kStringPoster, kStringPosterDescription, kStringCabin, kStringCabinFree, kStringCabinOccupied,
kStringFeet, kStringFeetDescription, kStringHood, kStringHoodDescription, kString400Xa,
kString10Xa, kStringSlotDescription1, kStringSlotDescription2, kStringChair, kStringChairDescription,
kStringScribble, kStringFace, kStringFaceDescription, kStringBooks, kStringDictionary,
kStringPlant, kStringMask, kStringSnake, kStringCup, kStringJoystick,
kStringToothbrushDescription, kStringMusic, kStringMusicDescription, kStringBottle, kStringBottleDescription,
kStringBox, kStringSeller, kStringWhat, kStringNotInformed, kStringHorstHummel,
kStringNiceWeather, kStringTellTicket, kStringHereIsXa, kString500Xa, kString1000Xa,
kString5000Xa, kString10000Xa, kStringThankYou, kStringWhatYouOffer, kStringHello2,
kStringWhatYouWant, kStringWhoAreYou, kStringHorstHummel2, kStringNeverHeard, kStringYouDontKnow,
kStringImOnTV, kStringIDontKnow, kStringFunny, kStringAha, kStringICan,
kStringFromWhom, kStringCost, kStringAsYouSay, kStringGetCard, kStringOnlyParticipation,
kStringWhatForIt, kStringMakeOffer, kStringGoodOffer, kStringGiveCard, kStringIdiot,
kStringCheckout1, kStringCheckout2, kStringCheckout3, kStringYes, kStringNo,
kStringCheckout4, kStringCheckout5, kStringCheckout6, kStringCheckout7, kStringCheckout8,
kStringCheckout9, kStringCheckout10, kStringCheckout11, kStringCheckout12, kStringCheckout13,
kStringCheckout14, kStringCheckout15, kStringCheckout16, kStringCheckout17, kStringCheckout18,
kStringCheckout19, kStringCheckout20, kStringCheckout21, kStringCheckout22, kStringCheckout23,
kStringCheckout24, kStringCheckout25, kStringCheckout26, kStringCheckout27, kStringCheckout28,
kStringCheckout29, kStringCheckout30, kStringCheckout31, kStringCheckout32, kStringCheckout33,
kStringCheckout34, kStringCheckout35, kStringCheckout36, kStringCheckout37, kStringCheckout38,
kStringCheckout39, kStringCheckout40, kStringCheckout41, kStringCheckout42, kStringCheckout43,
kStringCheckout44, kStringCheckout45, kStringCheckout46, kStringCheckout47, kStringCheckout48,
kStringCheckout49, kStringAtMusicContest, kStringNoImitation, kStringGoodJoke, kStringCommon,
kStringIWillProof, kStringIWillPerform, kString422, kStringAppearance1, kStringAppearance2,
kStringAppearance3, kStringAppearance4, kStringAppearance5, kStringAppearance6, kStringAppearance7,
kStringAppearance8, kStringAppearance9, kStringAppearance10, kStringAppearance11, kStringAppearance12,
kStringAppearance13, kStringAppearance14, kStringAppearance15, kStringAppearance16, kStringAppearance17,
kStringAppearance18, kStringAppearance19, kStringAppearance20, kStringAppearance21, kStringAppearance22,
kStringAppearance23, kStringAppearance24, kStringAppearance25, kStringAppearance26, kStringAppearance27,
kStringAppearance28, kStringAppearance29, kStringAppearance30, kStringAppearance31, kStringElevator1,
kStringElevator2, kStringElevator3, kStringElevator4, kStringElevator5, kStringElevator6,
kStringElevator7, kStringElevator8, kStringElevator9, kStringElevator10, kStringElevator11,
kStringElevator12, kStringElevator13, kStringElevator14, kStringElevator15, kStringElevator16,
kStringElevator17, kStringElevator18, kStringElevator19, kStringElevator20, kStringElevator21,
kStringElevator22, kStringElevator23, kStringElevator24, kStringElevator25, kStringElevator26,
kStringElevator27, kStringElevator28, kStringElevator29, kStringElevator30, kStringElevator31,
kStringElevator32, kStringElevator33, kStringElevator34, kStringElevator35, kStringElevator36,
kStringElevator37, kStringElevator38, kStringElevator39, kStringElevator40, kStringElevator41,
kStringElevator42, kStringElevator43, kStringElevator44, kStringElevator45, kStringElevator46,
kStringElevator47, kStringElevator48, kStringElevator49, kStringElevator50, kStringElevator51,
kStringElevator52, kStringElevator53, kStringElevator54, kStringElevator55, kStringElevator56,
kStringElevator57, kStringElevator58, kStringElevator59, kStringElevator60, kStringElevator61,
kStringElevator62, kStringElevator63, kStringElevator64, kStringElevator65, kStringApartment1,
kStringApartment2, kStringApartment3, kStringApartment4, kStringApartment5, kStringApartment6,
kStringApartment7, kStringApartment8, kStringShip0, kStringShip1, kStringShip2,
kStringShip3, kStringShip4, kStringShip5, kStringShip6, kStringShip7,
kStringShip8, kStringShip9, kStringShip10, kStringShip11, kStringShip12,
kStringShip13, kStringShip14, kStringShip15, kStringShip16, kStringShip17,
kStringShip18, kStringShip19, kStringFascinating, kStringTaxis, kStringTaxisDescription,
kStringAxacussan, kStringParticipationCard, kStringAxacussian, kStringSign1Description, kStringSign2Description,
kStringSign3Description, kStringSign4Description, kStringBell, kStringDisplay, kStringKeypad,
kStringKeypadDescription, kStringChip, kStringChipDescription, kStringHatch, kStringHatchDescription,
kStringMusicSystem, kStringMusicSystemDescription, kStringSpeakers, kStringSpeakersDescription, kStringPencils,
kStringPencilsDescription, kStringMetalBlocks, kStringMetalBlocksDescription, kStringImage, kStringImageDescription,
kStringCabinet, kStringCabinetDescription, kStringElevator, kStringUnderBed, kStringUnderBedDescription,
kStringKey, kStringKeyDescription, kStringSwitch, kStringHandle, kStringHatch2,
kStringSpaceSuit, kStringSpaceSuitDescription, kStringCable, kStringCableDescription1, kStringCableDescription2,
kStringIntro6, kStringIntro7, kStringIntro8, kStringIntroTV1, kStringIntroTV2,
kStringIntroTV3, kStringIntroTV4, kStringIntroTV5, kStringIntroTV6, kStringIntroTV7,
kStringIntroTV8, kStringIntroTV9, kStringIntroTV10, kStringIntroTV11, kStringIntroTV12,
kStringIntroTV13, kStringIntroTV14, kStringIntroTV15, kStringIntroTV16, kStringIntro9,
kStringIntro10, kStringIntro11, kStringIntro12, kStringIntro13, kStringIntro14, 
kStringMonsterDescription, kStringDialogSeparator
};

ObjectType operator|(ObjectType a, ObjectType b);
ObjectType operator&(ObjectType a, ObjectType b);
ObjectType operator^(ObjectType a, ObjectType b);
ObjectType &operator|=(ObjectType &a, ObjectType b);
ObjectType &operator&=(ObjectType &a, ObjectType b);
ObjectType &operator^=(ObjectType &a, ObjectType b);

struct Object {
	Object()
		: _name(kNoString)
		, _description(kStringDefaultDescription)
		, _id(INVALIDOBJECT)
		, _roomId(NULLROOM)
		, _type(NULLTYPE)
		, _click(0)
		, _click2(0)
		, _section(0)
		, _exitRoom(NULLROOM)
		, _direction(0)
	{}
	Object(byte roomId, StringId name, StringId description, ObjectId id, ObjectType type,
		   byte click, byte click2, byte section = 0, RoomId exitRoom = NULLROOM, byte direction = 0)
		: _name(name)
		, _description(description)
		, _id(id)
		, _roomId(roomId)
		, _type(type)
		, _click(click)
		, _click2(click2)
		, _section(section)
		, _exitRoom(exitRoom)
		, _direction(direction)
	{}

	void resetProperty(ObjectType type = NULLTYPE) {
		_type = type;
	}

	void setProperty(ObjectType type) {
		_type |= type;
	}

	void disableProperty(ObjectType type) {
		_type &= ~type;
	}

	bool hasProperty(ObjectType type) const {
		return _type & type;
	}

	static bool combine(Object &obj1, Object &obj2, ObjectId id1, ObjectId id2) {
		if (obj1.hasProperty(COMBINABLE))
			return (((obj1._id == id1) && (obj2._id == id2)) ||
					((obj1._id == id2) && (obj2._id == id1)));
		else
			return false;
	}

	byte _roomId;
	StringId _name;
	StringId _description;
	ObjectId _id;
	ObjectTypes _type;
	byte _click;
	byte _click2;
	byte _section;
	RoomId _exitRoom;
	byte _direction;
};

#define ticksToMsec(x) (x * kMsecPerTick)

}

#endif // SUPERNOVA2_MS2_DEF_H
