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

#ifndef SUPERNOVA_ROOMS_H
#define SUPERNOVA_ROOMS_H

#include "common/scummsys.h"

#include "msn_def.h"

namespace Supernova {

class GameManager;
class SupernovaEngine;

class Room {
public:
	Room() {
		_seen = false;
		for (int i = 0; i < kMaxSection; ++i)
			_shown[i] = kShownFalse;
		for (int i = 0; i < kMaxDialog; ++i)
			_sentenceRemoved[i] = 0;
	}

	bool hasSeen() {
		return _seen;
	}
	void setRoomSeen(bool seen) {
		_seen = seen;
	}

	int getFileNumber() const {
		return _fileNumber;
	}
	RoomID getId() const {
		return _id;
	}

	void setSectionVisible(uint section, bool visible) {
		_shown[section] = visible ? kShownTrue : kShownFalse;
	}

	bool isSectionVisible(uint index) const {
		return _shown[index] == kShownTrue;
	}

	void removeSentence(int sentence, int number) {
		if (number > 0)
			_sentenceRemoved[number - 1] |= (1 << sentence);
	}

	void addSentence(int sentence, int number) {
		if (number > 0)
			_sentenceRemoved[number - 1] &= ~(1 << sentence);
	}

	void addAllSentences(int number) {
		if (number > 0)
			_sentenceRemoved[number - 1] = 0;
	}

	bool sentenceRemoved(int sentence, int number) {
		if (number <= 0)
			return false;
		return (_sentenceRemoved[number - 1] & (1 << sentence));
	}

	bool allSentencesRemoved(int maxSentence, int number) {
		if (number <= 0)
			return false;
		for (int i = 0, flag = 1 ; i < maxSentence ; ++i, flag <<= 1)
			if (!(_sentenceRemoved[number - 1] & flag))
				return false;
		return true;
	}

	Object *getObject(uint index) {
		return &_objectState[index];
	}

	virtual ~Room() {}
	virtual void animation() {}
	virtual void onEntrance() {}
	virtual bool interact(Action verb, Object &obj1, Object &obj2) {
		return false;
	}
	virtual bool serialize(Common::WriteStream *out);
	virtual bool deserialize(Common::ReadStream *in, int version);

protected:
	int _fileNumber;
	bool _shown[kMaxSection];
	byte _sentenceRemoved[kMaxDialog];
	Object _objectState[kMaxObject];
	RoomID _id;
	SupernovaEngine *_vm;
	GameManager *_gm;

private:
	bool _seen;
};

// Room 0
class Intro : public Room {
public:
	Intro(SupernovaEngine *vm, GameManager *gm);
	virtual void onEntrance();

private:
	bool animate(int section1, int section2, int duration);
	bool animate(int section1, int section2, int duration, MessagePosition position,
	             StringID text);
	bool animate(int section1, int section2, int section3, int section4, int duration,
	             MessagePosition position, StringID text);

	void titleScreen();
	void titleFadeIn();
	void cutscene();
	void leaveCutscene();

	bool _shouldExit;
	Common::String _introText;
};

// Spaceship
class ShipCorridor : public Room {
public:
	ShipCorridor(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 17;
		_id = CORRIDOR;
		_shown[0] = kShownTrue;
		_shown[4] = kShownTrue;

		_objectState[0] = Object(_id, kStringHatch, kStringDefaultDescription, HATCH1, OPENABLE | EXIT, 0, 6, 1, CABIN_L1, 15);
		_objectState[1] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 1, 7, 2, CABIN_L2, 10);
		_objectState[2] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 2, 8, 3, CABIN_L3, 5);
		_objectState[3] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 5, 11, 6, CABIN_R1, 19);
		_objectState[4] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT, 4, 10, 5, CABIN_R2, 14);
		_objectState[5] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | EXIT | OPENED, 9, 3, 4, CABIN_R3, 9);
		_objectState[6] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | CLOSED | EXIT, 12, 12, 0, AIRLOCK, 2);
		_objectState[7] = Object(_id, kStringButton, kStringHatchButtonDescription, BUTTON, PRESS, 13, 13, 0, NULLROOM, 0);
		_objectState[8] = Object(_id, kStringLadder, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 14, 14, 0, NULLROOM, 0);
		_objectState[9] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 15, 15, 0, HALL, 22);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipHall: public Room {
public:
	ShipHall(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 15;
		_id = HALL;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringHatch, kStringCockpitHatchDescription, NULLOBJECT, OPENABLE | EXIT, 4, 5, 1, COCKPIT, 10);
		_objectState[1] = Object(_id, kStringHatch, kStringKitchenHatchDescription, KITCHEN_HATCH, OPENABLE | EXIT, 0, 0, 0, NULLROOM, 1);
		_objectState[2] = Object(_id, kStringHatch, kStringStasisHatchDescription, NULLOBJECT, OPENABLE | CLOSED | EXIT, 1, 1, 2, SLEEP, 8);
		_objectState[3] = Object(_id, kStringSlot, kStringSlotDescription, SLEEP_SLOT, COMBINABLE, 2, 2, 0, NULLROOM, 0);
		_objectState[4] = Object(_id, kStringLadder, kStringDefaultDescription, NULLOBJECT, NULLTYPE, 3, SLEEP, 0, NULLROOM, 0);
		_objectState[5] = Object(_id, kStringCorridor, kStringDefaultDescription, NULLOBJECT, EXIT, 6, 6, 0, CORRIDOR, 19);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipSleepCabin: public Room {
public:
	ShipSleepCabin(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 33;
		_id = SLEEP;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringHatch, kStringStasisHatchDescription2, CABINS, NULLTYPE, 0, 0, 0, NULLROOM, 0);
		_objectState[1] = Object(_id, kStringHatch, kStringStasisHatchDescription2, CABIN, NULLTYPE, 1, 1, 0, NULLROOM, 0);
		_objectState[2] = Object(_id, kStringComputer, kStringDefaultDescription, COMPUTER, NULLTYPE, 2, 2, 0, NULLROOM, 0);
		_objectState[3] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	byte _color;
};

class ShipCockpit : public Room {
public:
	ShipCockpit(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 9;
		_id = COCKPIT;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringInstruments, kStringInstrumentsDescription1, INSTRUMENTS, NULLTYPE, 2, 2, 0, NULLROOM, 0);
		_objectState[1] = Object(_id, kStringMonitor, kStringDefaultDescription, MONITOR, NULLTYPE, 0, 0, 0, NULLROOM, 0);
		_objectState[2] = Object(_id, kStringMonitor, kStringMonitorDescription, NULLOBJECT, TAKE, 1, 0, 0, NULLROOM, 0);
		_objectState[3] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	byte _color;
};

class ShipCabinL1: public Room {
public:
	ShipCabinL1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 21;
		_id = CABIN_L1;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[2] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;

		_objectState[0] = Object(_id, kStringImage, kStringGenericDescription1, NULLOBJECT, UNNECESSARY, 5, 5, 0, NULLROOM, 0);
		_objectState[1] = Object(_id, kStringImage, kStringGenericDescription2, NULLOBJECT, UNNECESSARY, 6, 6, 0, NULLROOM, 0);
		_objectState[2] = Object(_id, kStringImage, kStringGenericDescription3, NULLOBJECT, UNNECESSARY, 7, 7, 0, NULLROOM, 0);
		_objectState[3] = Object(_id, kStringMagnete, kStringMagneteDescription, NULLOBJECT, UNNECESSARY, 8, 8, 0, NULLROOM, 0);
		_objectState[4] = Object(_id, kStringImage, kStringGenericDescription4, NULLOBJECT, UNNECESSARY, 9, 9, 0);
		_objectState[5] = Object(_id, kStringPen, kStringPenDescription, PEN, TAKE | COMBINABLE, 10, 10, 5 | 128);
		_objectState[6] = Object(_id, kStringHatch, kStringDefaultDescription, NULLOBJECT, OPENABLE | OPENED | EXIT, 3, 3, 24 | 128, CORRIDOR, 9);
		_objectState[7] = Object(_id, kStringSlot, kStringSlotDescription, NULLOBJECT, COMBINABLE, 0, 0, 0);
		_objectState[8] = Object(_id, kStringShelf, kStringDefaultDescription, NULLOBJECT, OPENABLE | CLOSED, 1, 1, 0);
		_objectState[9] = Object(_id, kStringCompartment, kStringDefaultDescription, NULLOBJECT, OPENABLE | CLOSED, 2, 2, 0);
		_objectState[10] = Object(_id, kStringSocket, kStringDefaultDescription, SOCKET, COMBINABLE, 4, 4, 0);
		_objectState[11] = Object(_id, kStringToilet, kStringDefaultDescription, BATHROOM_DOOR, EXIT, 255, 255, 0, BATHROOM, 22);
	}
};

class ShipCabinL2 : public Room {
public:
	ShipCabinL2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 21;
		_id = CABIN_L2;
		_shown[0] = kShownTrue;
		_shown[16] = kShownTrue;

		_objectState[0] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL1,COMBINABLE,31,31,0);
		_objectState[1] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL2,COMBINABLE,32,32,0);
		_objectState[2] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL3,COMBINABLE,33,33,0);
		_objectState[3] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_KL4,COMBINABLE,45,45,0);
		_objectState[4] = Object(_id, kStringShelf,kStringDefaultDescription,SHELF_L1,OPENABLE | CLOSED,25,26,17);
		_objectState[5] = Object(_id, kStringPistol,kStringPistolDescription,PISTOL,TAKE,39,39,20);
		_objectState[6] = Object(_id, kStringCompartment,kStringDefaultDescription,SHELF_L2,OPENABLE | CLOSED,27,28,18);
		_objectState[7] = Object(_id, kStringBooks,kStringBooksDescription,NULLOBJECT,UNNECESSARY,40,40,0);
		_objectState[8] = Object(_id, kStringCompartment,kStringDefaultDescription,SHELF_L3,OPENABLE | CLOSED,29,30,19);
		_objectState[9] = Object(_id, kStringSpool,kStringSpoolDescription, SPOOL,TAKE | COMBINABLE,41,41,21);
		_objectState[10] = Object(_id, kStringCompartment,kStringDefaultDescription,SHELF_L4,OPENABLE | CLOSED,43,44,22);
		_objectState[11] = Object(_id, kStringBook,kStringDefaultDescription,BOOK2,TAKE,46,46,23);
		_objectState[12] = Object(_id, kStringUnderwear,kStringUnderwearDescription,NULLOBJECT,UNNECESSARY,34,34,0);
		_objectState[13] = Object(_id, kStringUnderwear,kStringUnderwearDescription,NULLOBJECT,UNNECESSARY,35,35,0);
		_objectState[14] = Object(_id, kStringClothes,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,36,36,0);
		_objectState[15] = Object(_id, kStringJunk,kStringJunkDescription,NULLOBJECT,UNNECESSARY,37,37,0);
		_objectState[16] = Object(_id, kStringJunk,kStringJunkDescription,NULLOBJECT,UNNECESSARY,38,38,0);
		_objectState[17] = Object(_id, kStringMagnete,kStringMagneteDescription,NULLOBJECT,UNNECESSARY,23,23,0);
		_objectState[18] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
		_objectState[19] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,24 | 128,CORRIDOR,9);
		_objectState[20] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[21] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[22] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[23] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[24] = Object(_id, kStringFolders,kStringFoldersDescription,NULLOBJECT,UNNECESSARY,49,49,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipCabinL3 : public Room {
public:
	ShipCabinL3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 21;
		_id = CABIN_L3;
		_shown[0] = kShownTrue;
		_shown[6] = kShownTrue;
		_shown[7] = kShownTrue;
		_shown[8] = kShownTrue;
		_shown[9] = kShownTrue;
		_shown[12] = kShownTrue;

		_objectState[0] = Object(_id, kStringPoster,kStringPosterDescription1,NULLOBJECT,UNNECESSARY,11,11,0);
		_objectState[1] = Object(_id, kStringPoster,kStringPosterDescription2,NULLOBJECT,UNNECESSARY,12,12,0);
		_objectState[2] = Object(_id, kStringSpeaker,kStringDefaultDescription,NULLOBJECT,NULLTYPE,13,13,0);
		_objectState[3] = Object(_id, kStringMagnete,kStringMagneteDescription,NULLOBJECT,UNNECESSARY,14,14,0);
		_objectState[4] = Object(_id, kStringRecord,kStringRecordDescription,RECORD,TAKE | COMBINABLE,15,15,8 | 128);
		_objectState[5] = Object(_id, kStringRecordStand,kStringRecordStandDescription,NULLOBJECT,UNNECESSARY,16,16,0);
		_objectState[6] = Object(_id, kStringButton,kStringDefaultDescription,TURNTABLE_BUTTON,PRESS,22,22,0);
		_objectState[7] = Object(_id, kStringTurntable,kStringTurntableDescription,TURNTABLE,UNNECESSARY | COMBINABLE,17,17,0);
		_objectState[8] = Object(_id, kStringWire,kStringDefaultDescription,WIRE,COMBINABLE,18,18,0);
		_objectState[9] = Object(_id, kStringWire,kStringDefaultDescription,WIRE2,COMBINABLE,19,19,0);
		_objectState[10] = Object(_id, kStringPlug,kStringDefaultDescription,PLUG,COMBINABLE,20,20,0);
		_objectState[11] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,24 | 128,CORRIDOR,9);
		_objectState[12] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[13] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[14] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[15] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[16] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipCabinR1 : public Room {
public:
	ShipCabinR1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_id = CABIN_R1;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[2] = kShownTrue;

		_objectState[0] = Object(_id, kStringImage,kStringImageDescription1,NULLOBJECT,UNNECESSARY,5,5,0);
		_objectState[1] = Object(_id, kStringDrawingInstruments,kStringDrawingInstrumentsDescription,NULLOBJECT,UNNECESSARY,6,6,0);
		_objectState[2] = Object(_id, kStringMagnete,kStringMagneteDescription,NULLOBJECT,UNNECESSARY,7,7,0);
		_objectState[3] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[4] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[5] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[6] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[7] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[8] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	}
};

class ShipCabinR2 : public Room {
public:
	ShipCabinR2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_id = CABIN_R2;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;

		_objectState[0] = Object(_id, kStringChessGame,kStringChessGameDescription1,NULLOBJECT,UNNECESSARY,11,11,0);
		_objectState[1] = Object(_id, kStringTennisRacket,kStringTennisRacketDescription,NULLOBJECT,UNNECESSARY,8,8,0);
		_objectState[2] = Object(_id, kStringTennisBall,kStringGenericDescription2,NULLOBJECT,UNNECESSARY,9,9,0);
		_objectState[3] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[4] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[5] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[6] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[7] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[8] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	}
};

class ShipCabinR3 : public Room {
public:
	ShipCabinR3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_id = CABIN_R3;
		_shown[0] = kShownTrue;
		_shown[7] = kShownTrue;
		_shown[8] = kShownTrue;
		_shown[15] = kShownTrue;

		_objectState[0] = Object(_id, kStringChessGame,kStringChessGameDescription2,CHESS,TAKE | COMBINABLE,12,12,7 | 128);
		_objectState[1] = Object(_id, kStringBed,kStringBedDescription,NULLOBJECT,NULLTYPE,13,13,0);
		_objectState[2] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K1,COMBINABLE,27,27,0);
		_objectState[3] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K2,COMBINABLE,28,28,0);
		_objectState[4] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K3,COMBINABLE,29,29,0);
		_objectState[5] = Object(_id, kStringSlot,kStringSlotDescription,SLOT_K4,COMBINABLE,30,30,0);
		_objectState[6] = Object(_id, kStringCompartment,kStringCompartmentDescription,SHELF1,OPENABLE | CLOSED,14,18,9);
		_objectState[7] = Object(_id, kStringAlbums,kStringAlbumsDescription,NULLOBJECT,UNNECESSARY,14,14,0);
		_objectState[8] = Object(_id, kStringCompartment,kStringCompartmentDescription,SHELF2,OPENABLE | CLOSED,15,19,10);
		_objectState[9] = Object(_id, kStringRope,kStringRopeDescription,ROPE,TAKE | COMBINABLE,15,15,12);
		_objectState[10] = Object(_id, kStringShelf,kStringShelfDescription,SHELF3,OPENABLE | CLOSED,16,17,11);
		_objectState[11] = Object(_id, kStringJunk,kStringJunkDescription,NULLOBJECT,UNNECESSARY,20,20,0);
		_objectState[12] = Object(_id, kStringClothes,kStringClothesDescription,NULLOBJECT,UNNECESSARY,21,21,0);
		_objectState[13] = Object(_id, kStringUnderwear,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,22,22,0);
		_objectState[14] = Object(_id, kStringSocks,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,23,23,0);
		_objectState[15] = Object(_id, kStringCompartment,kStringCompartmentDescription,SHELF4,OPENABLE | CLOSED,24,25,13);
		_objectState[16] = Object(_id, kStringBook,kStringBookHitchhiker,BOOK,TAKE,26,26,14);
		_objectState[17] = Object(_id, kStringDiscman,kStringDiscmanDescription,DISCMAN,TAKE | COMBINABLE,33,33,16);
		_objectState[18] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[19] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[20] = Object(_id, kStringShelf,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[21] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[22] = Object(_id, kStringSocket,kStringDefaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[23] = Object(_id, kStringToilet,kStringDefaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();
};

class ShipCabinBathroom : public Room  {
public:
	ShipCabinBathroom(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 23;
		_id = BATHROOM;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringBathroom,kStringBathroomDescription,TOILET,NULLTYPE,0,0,0);
		_objectState[1] = Object(_id, kStringShower,kStringDefaultDescription,SHOWER,NULLTYPE,1,1,0);
		_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,BATHROOM_EXIT,EXIT,255,255,0,CABIN_R3,2);
	}
};

class ShipAirlock : public Room {
public:
	ShipAirlock(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 34;
		_id = AIRLOCK;
		_shown[0] = kShownTrue;
		_shown[6] = kShownTrue;

		_objectState[0] = Object(_id, kStringHatch,kStringHatchDescription1,NULLOBJECT,EXIT | OPENABLE | OPENED | CLOSED,0,0,0,CORRIDOR,10);
		_objectState[1] = Object(_id, kStringHatch,kStringHatchDescription2,NULLOBJECT,EXIT | OPENABLE | CLOSED,1,1,0,HOLD,14);
		_objectState[2] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON1,PRESS,2,2,0);
		_objectState[3] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON2,PRESS,3,3,0);
		_objectState[4] = Object(_id, kStringHelmet,kStringHelmetDescription,HELMET,TAKE,4,4,7);
		_objectState[5] = Object(_id, kStringSuit,kStringSuitDescription,SUIT,TAKE,5,5,8);
		_objectState[6] = Object(_id, kStringLifeSupport,kStringLifeSupportDescription,LIFESUPPORT,TAKE,6,6,9);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();
};

class ShipHold : public Room {
public:
	ShipHold(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 24;
		_id = HOLD;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kNoString,kStringDefaultDescription,HOLD_WIRE,COMBINABLE,255,255,0);
		_objectState[1] = Object(_id, kStringScrap,kStringScrapDescription1,SCRAP_LK,NULLTYPE,4,4,0);
		_objectState[2] = Object(_id, kStringTerminalStrip,kStringDefaultDescription,TERMINALSTRIP,COMBINABLE,255,255,0);
		_objectState[3] = Object(_id, kStringScrap,kStringScrapDescription2,NULLOBJECT,NULLTYPE,5,5,0);
		_objectState[4] = Object(_id, kStringReactor,kStringReactorDescription,NULLOBJECT,NULLTYPE,6,6,0);
		_objectState[5] = Object(_id, kStringNozzle,kStringDefaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[6] = Object(_id, kStringPumpkin,kStringPumpkinDescription,NULLOBJECT,NULLTYPE,8,8,0);
		_objectState[7] = Object(_id, kStringHatch,kStringDefaultDescription,LANDINGMOD_OUTERHATCH,EXIT | OPENABLE,1,2,2,LANDINGMODULE,6);
		_objectState[8] = Object(_id, kStringLandingModule,kStringLandingModuleDescription,NULLOBJECT,NULLTYPE,0,0,0);
		_objectState[9] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,AIRLOCK,22);
		_objectState[10] = Object(_id, kStringHatch,kStringHatchDescription3,OUTERHATCH_TOP,EXIT | OPENABLE | OPENED,3,3,0,GENERATOR,8);
		_objectState[11] = Object(_id, kStringGenerator,kStringGeneratorDescription,GENERATOR_TOP,EXIT,12,12,0,GENERATOR,8);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();
};

class ShipLandingModule : public Room {
public:
	ShipLandingModule(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 25;
		_id = LANDINGMODULE;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringSocket,kStringDefaultDescription,LANDINGMOD_SOCKET,COMBINABLE,1,1,0);
		_objectState[1] = Object(_id, kStringButton,kSafetyButtonDescription,LANDINGMOD_BUTTON,PRESS | COMBINABLE,2,2,0);
		_objectState[2] = Object(_id, kStringMonitor,kStringDefaultDescription,LANDINGMOD_MONITOR,NULLTYPE,3,3,0);
		_objectState[3] = Object(_id, kStringKeyboard,kStringDefaultDescription,KEYBOARD,NULLTYPE,4,4,0);
		_objectState[4] = Object(_id, kNoString,kStringDefaultDescription,LANDINGMOD_WIRE,COMBINABLE,255,255,0);
		_objectState[5] = Object(_id, kStringHatch,kStringDefaultDescription,LANDINGMOD_HATCH,EXIT | OPENABLE | OPENED | COMBINABLE, 0,0,1 | 128,HOLD,10);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipGenerator : public Room {
public:
	ShipGenerator(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 18;
		_id = GENERATOR;
		_shown[0] = kShownTrue;
		_shown[5] = kShownTrue;

		_objectState[0] = Object(_id, kStringGeneratorWire,kStringDefaultDescription,GENERATOR_WIRE,COMBINABLE,255,255,0);
		_objectState[1] = Object(_id, kStringEmptySpool,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
		_objectState[2] = Object(_id, kStringKeycard2,kStringKeycard2Description,KEYCARD2,COMBINABLE | TAKE,12,12,5 | 128);
		_objectState[3] = Object(_id, kStringRope,kStringDefaultDescription,GENERATOR_ROPE,COMBINABLE,255,255,0);
		_objectState[4] = Object(_id, kStringHatch,kStringHatchDescription3,OUTERHATCH,EXIT | OPENABLE,1,2,1,OUTSIDE,22);
		_objectState[5] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,OPENABLE | CLOSED,3,3,0);
		_objectState[6] = Object(_id, kStringSlot,kStringSlotDescription,NULLOBJECT,COMBINABLE,4,4,0);
		_objectState[7] = Object(_id, kStringTrap,kStringDefaultDescription,TRAP,OPENABLE,5,6,2);
		_objectState[8] = Object(_id, kStringWire,kStringDefaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[9] = Object(_id, kStringVoltmeter,kStringDefaultDescription,VOLTMETER,NULLTYPE,9,9,0,NULLROOM,0);
		_objectState[10] = Object(_id, kStringClip,kStringDefaultDescription,CLIP,COMBINABLE,8,8,0);
		_objectState[11] = Object(_id, kStringWire,kStringWireDescription,SHORT_WIRE,COMBINABLE,10,10,0);
		_objectState[12] = Object(_id, kStringLadder,kStringDefaultDescription,LADDER,EXIT,0,0,0,HOLD,1);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipOuterSpace : public Room {
public:
	ShipOuterSpace(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 4;
		_id = OUTSIDE;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringHatch,kStringDefaultDescription,NULLOBJECT,EXIT,0,0,0,GENERATOR,3);
		_objectState[1] = Object(_id, kStringRope,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
	}
};

// Arsano
class ArsanoRocks : public Room {
public:
	ArsanoRocks(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 12;
		_id = OUTSIDE;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringRope,kStringDefaultDescription,NULLOBJECT,UNNECESSARY | EXIT,0,0,0,GENERATOR,12);
		_objectState[1] = Object(_id, kStringStone,kStringDefaultDescription,STONE,NULLTYPE,1,1,0);
		_objectState[2] = Object(_id, kStringStone,kStringDefaultDescription,NULLOBJECT,NULLTYPE,2,2,0);
		_objectState[3] = Object(_id, kStringCaveOpening,kStringCaveOpeningDescription,NULLOBJECT,NULLTYPE,255,255,0,CAVE,1);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class ArsanoCave : public Room {
public:
	ArsanoCave(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 12;
		_id = CAVE;

		_objectState[0] = Object(_id, kStringExit,kStringExitDescription,NULLOBJECT,EXIT,255,255,0,ROCKS,22);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,MEETUP,2);
	}
};
class ArsanoMeetup : public Room {
public:
	ArsanoMeetup(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 37;
		_id = MEETUP;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringCave,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);
		_objectState[1] = Object(_id, kStringSign,kStringSignDescription,MEETUP_SIGN,NULLTYPE,0,0,0);
		_objectState[2] = Object(_id, kStringEntrance,kStringDefaultDescription,DOOR,EXIT,1,1,0,ENTRANCE,7);
		_objectState[3] = Object(_id, kStringStar,kStringDefaultDescription,STAR,NULLTYPE,2,2,0);
		_objectState[4] = Object(_id, kStringSpaceshift,kStringDefaultDescription,SPACESHIPS,COMBINABLE,3,3,0);
		_objectState[5] = Object(_id, kStringSpaceshift,kStringDefaultDescription,SPACESHIP,COMBINABLE,4,4,0);
	}

	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	byte _sign;
	byte _beacon;
};
class ArsanoEntrance : public Room {
public:
	ArsanoEntrance(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 10;
		_id = ENTRANCE;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringPorter,kStringPorterDescription,PORTER,TALK,0,0,0);
		_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,NULLOBJECT,EXIT | OPENABLE | CLOSED,1,1,0,NULLROOM,5);
		_objectState[2] = Object(_id, kStringSign,kStringSignDescription,KITCHEN_SIGN,NULLTYPE,2,2,0);
		_objectState[3] = Object(_id, kStringChewingGum,kStringDefaultDescription,SCHNUCK,TAKE,255,255,10+128);
		_objectState[4] = Object(_id, kStringGummyBears,kStringDefaultDescription,SCHNUCK,TAKE,255,255,11+128);
		_objectState[5] = Object(_id, kStringChocolateBall,kStringDefaultDescription,SCHNUCK,TAKE,255,255,12+128);
		_objectState[6] = Object(_id, kStringEgg,kStringDefaultDescription,EGG,TAKE,255,255,13+128);
		_objectState[7] = Object(_id, kStringLiquorice,kStringDefaultDescription,SCHNUCK,TAKE,255,255,14+128);
		_objectState[8] = Object(_id, kStringPill,kStringPillDescription,PILL,TAKE,255,255,0);
		_objectState[9] = Object(_id, kStringSlot,kStringDefaultDescription,CAR_SLOT,COMBINABLE,6,6,0);
		_objectState[10] = Object(_id, kStringVendingMachine,kStringVendingMachineDescription,NULLOBJECT,NULLTYPE,5,5,0);
		_objectState[11] = Object(_id, kStringToilet,kStringToiletDescription,ARSANO_BATHROOM,NULLTYPE,255,255,0);
		_objectState[12] = Object(_id, kStringButton,kStringDefaultDescription,BATHROOM_BUTTON,PRESS,3,3,0);
		_objectState[13] = Object(_id, kStringSign,kStringSignDescription,BATHROOM_SIGN,NULLTYPE,4,4,0);
		_objectState[14] = Object(_id, kStringStaircase,kStringDefaultDescription,STAIRCASE,EXIT,8,8,0,REST,3);
		_objectState[15] = Object(_id, kStringExit,kStringDefaultDescription,MEETUP_EXIT,EXIT,255,255,0,MEETUP,22);
		_objectState[16] = Object(_id, kStringCoins,kStringCoinsDescription,COINS,TAKE|COMBINABLE,255,255,0);
		_objectState[17] = Object(_id, kStringTabletPackage,kStringTabletPackageDescription,PILL_HULL,TAKE,255,255,0);

		_dialog1[0] = kStringArsanoDialog7;
		_dialog1[1] = kStringArsanoDialog1;
		_dialog1[2] = kStringArsanoDialog8;
		_dialog1[3] = kStringArsanoDialog9;
		_dialog1[4] = kStringDialogSeparator;

		_dialog2[0] = kStringArsanoDialog1;
		_dialog2[1] = kStringArsanoDialog2;
		_dialog2[2] = kStringArsanoDialog3;
		_dialog2[3] = kStringArsanoDialog4;
		_dialog2[4] = kStringDialogSeparator;

		_dialog3[0] = kStringArsanoDialog5;
		_dialog3[1] = kStringArsanoDialog6;

		_eyewitness = 5;
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();

private:
	StringID _dialog1[5];
	StringID _dialog2[5];
	StringID _dialog3[5];
	byte _eyewitness;
};
class ArsanoRemaining : public Room {
public:
	ArsanoRemaining(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 28;
		_id = REST;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringStaircase,kStringDefaultDescription,NULLOBJECT,EXIT,0,0,0,ENTRANCE,17);
		_objectState[1] = Object(_id, kStringChair,kStringDefaultDescription,NULLOBJECT,EXIT,1,1,0,ROGER,2);
		_objectState[2] = Object(_id, kStringShoes,kStringShoesDescription,NULLOBJECT,NULLTYPE,2,2,0);

		_chewing = kShownTrue;
	}

	virtual void animation();

private:
	bool _chewing;
	int _i;
};
class ArsanoRoger : public Room {
public:
	ArsanoRoger(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 29;
		_id = ROGER;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,REST,19);
		_objectState[1] = Object(_id, kStringFrogFace,kStringDefaultDescription,ROGER_W,TALK,0,0,0);
		_objectState[2] = Object(_id, kStringScrible,kStringScribleDescription,NULLOBJECT,NULLTYPE,3,3,0);
		_objectState[3] = Object(_id, kStringWallet,kStringDefaultDescription,WALLET,TAKE,1,1,4);
		_objectState[4] = Object(_id, kStringMenu,kStringMenuDescription,NULLOBJECT,UNNECESSARY,2,2,0);
		_objectState[5] = Object(_id, kStringCup,kStringCupDescription,CUP,UNNECESSARY,4,4,0);
		_objectState[6] = Object(_id, kStringChessGame,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
		_objectState[7] = Object(_id, kStringBill,kStringBillDescription,NULLOBJECT,TAKE|COMBINABLE,255,255,0);
		_objectState[8] = Object(_id, kStringKeycard3,kStringDefaultDescription,KEYCARD_R,TAKE|COMBINABLE,255,255,0);

		_dialog1[0] = kStringDialogArsanoRoger1;
		_dialog1[1] = kStringDialogArsanoRoger2;
		_dialog1[2] = kStringDialogArsanoRoger3;
		_dialog1[3] = kStringDialogSeparator;

		_eyewitness = 5;
	}

	virtual void animation();
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringID _dialog1[4];
	byte _eyewitness;
	byte _hands;
};
class ArsanoGlider : public Room {
public:
	ArsanoGlider(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 19;
		_id = GLIDER;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,MEETUP,15);
		_objectState[1] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON1,PRESS,0,0,0);
		_objectState[2] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON2,PRESS,1,1,0);
		_objectState[3] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON3,PRESS,2,2,0);
		_objectState[4] = Object(_id, kStringButton,kStringDefaultDescription,GLIDER_BUTTON4,PRESS,3,3,0);
		_objectState[5] = Object(_id, kStringKeycard,kStringDefaultDescription,GLIDER_KEYCARD,TAKE|COMBINABLE,255,255,0);
		_objectState[6] = Object(_id, kStringSlot,kStringDefaultDescription,GLIDER_SLOT,COMBINABLE,4,4,0);
		_objectState[7] = Object(_id, kStringCompartment,kStringDefaultDescription,NULLOBJECT,OPENABLE,5,6,6);
		_objectState[8] = Object(_id, kStringKeyboard,kStringDefaultDescription,GLIDER_BUTTONS,NULLTYPE,7,7,0);
		_objectState[9] = Object(_id, kStringAnnouncement,kStringAnnouncementDescription,GLIDER_DISPLAY,NULLTYPE,8,8,0);
		_objectState[10] = Object(_id, kStringInstruments,kStringAnnouncementDescription,GLIDER_INSTRUMENTS,NULLTYPE,9,9,0);
	}

	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	byte _sinus;
};
class ArsanoMeetup2 : public Room {
public:
	ArsanoMeetup2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 38;
		_id = MEETUP2;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringRoger, kStringDefaultDescription, ROGER_W, TALK, 255, 255, 0);
		_objectState[1] = Object(_id, kStringSpaceshift, kStringDefaultDescription, SPACESHIP, COMBINABLE, 255, 255, 0);
		_objectState[2] = Object(_id, kStringCave, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, CAVE, 22);

		_dialog1[0] = kStringDialogArsanoMeetup2_1;
		_dialog1[1] = kStringDialogArsanoMeetup2_2;
		_dialog2[0] = kStringDialogArsanoMeetup2_3;
		_dialog2[1] = kStringDialogArsanoMeetup2_4;
		_dialog3[0] = kStringDialogArsanoMeetup2_5;
		_dialog3[1] = kStringDialogArsanoMeetup2_6;
		_dialog3[2] = kStringDialogArsanoMeetup2_7;
		_dialog3[3] = kStringDialogArsanoMeetup2_8;
		_dialog4[0] = kStringDialogArsanoMeetup2_9;
		_dialog4[1] = kStringDialogArsanoMeetup2_10;
		_dialog4[2] = kStringDialogArsanoMeetup2_11;
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

	void shipStart();

private:
	// TODO: change to 6, fix initialization
	StringID _dialog1[2];
	StringID _dialog2[2];
	StringID _dialog3[4];
	StringID _dialog4[3];

	bool _found;
	bool _flug;
};
class ArsanoMeetup3 : public Room {
public:
	ArsanoMeetup3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 39;
		_id = MEETUP3;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringUfo,kStringUfoDescription,UFO,EXIT,0,0,0,NULLROOM,3);
		_objectState[1] = Object(_id, kStringStar,kStringDefaultDescription,STAR,NULLTYPE,1,1,0);
		_objectState[2] = Object(_id, kStringCave,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);

		_dialog2[0] = kStringArsanoDialog1;
		_dialog2[1] = kStringDialogArsanoMeetup3_1;
		_dialog2[2] = kStringDialogArsanoMeetup3_2;
		_dialog2[3] = kStringDialogArsanoMeetup3_3;
		_dialog3[0] = kStringDialogArsanoMeetup3_4;
		_dialog3[1] = kStringDialogArsanoMeetup3_5;

		_dialogsX[0] = kStringDialogX1;
		_dialogsX[1] = kStringDialogX2;
		_dialogsX[2] = kStringDialogX3;
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringID _dialog2[4];
	StringID _dialog3[2];

	// TODO: Hack, to be move away and renamed when the other uses are found
	StringID _dialogsX[6];
	//
};

// Axacuss
class AxacussCell : public Room {
public:
	AxacussCell(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 43;
		_id = CELL;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[31] = kShownTrue;

		_objectState[0] = Object(_id, kStringButton,kStringDefaultDescription,CELL_BUTTON,PRESS,1,1,0);
		_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,CELL_DOOR,EXIT|OPENABLE|CLOSED,0,0,31+128,CORRIDOR4,1);
		_objectState[2] = Object(_id, kStringTray,kStringTrayDescription,TRAY,UNNECESSARY,255,255,0);
		_objectState[3] = Object(_id, kStringLamp,kStringLampDescription,NULLOBJECT,COMBINABLE,3,3,0);
		_objectState[4] = Object(_id, kStringEyes,kStringEyesDescription,NULLOBJECT,NULLTYPE,4,4,0);
		_objectState[5] = Object(_id, kStringWire,kStringDefaultDescription,CELL_WIRE,COMBINABLE|TAKE,6,6,0);
		_objectState[6] = Object(_id, kStringSocket,kStringSocketDescription,SOCKET,COMBINABLE,5,5,0);
		_objectState[7] = Object(_id, kStringMetalBlock,kStringMetalBlockDescription,MAGNET,TAKE|COMBINABLE,255,255,30);
		_objectState[8] = Object(_id, kStringRobot,kStringRobotDescription,NULLOBJECT,NULLTYPE,255,255,0);
		_objectState[9] = Object(_id, kStringTable,kStringTableDescription,CELL_TABLE,COMBINABLE,2,2,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();
};
class AxacussCorridor1 : public Room {
public:
	AxacussCorridor1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR1;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[13] = kShownTrue;
		_shown[21] = kShownTrue;
		_shown[23] = kShownTrue;
		_shown[25] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,GUARD3,2);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR2,22);
	}

	virtual void onEntrance();
};
class AxacussCorridor2 : public Room {
public:
	AxacussCorridor2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR2;
		_shown[0] = kShownTrue;
		_shown[2] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[17] = kShownTrue;
		_shown[21] = kShownTrue;
		_shown[24] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR1,2);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR3,22);
		_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,1,1,0,CORRIDOR4,14);
	}

	virtual void onEntrance();
};
class AxacussCorridor3 : public Room {
public:
	AxacussCorridor3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR3;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[19] = kShownTrue;
		_shown[23] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR2,2);
	}

	virtual void onEntrance();
};
class AxacussCorridor4 : public Room {
public:
	AxacussCorridor4(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR4;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[2] = kShownTrue;
		_shown[8] = kShownTrue;
		_shown[9] = kShownTrue;
		_shown[11] = kShownTrue;
		_shown[15] = kShownTrue;
		_shown[18] = kShownTrue;
		_shown[20] = kShownTrue;
		_shown[26] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,0,0,0,CORRIDOR2,10);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,1,1,0,GUARD,14);
		_objectState[2] = Object(_id, kStringCellDoor,kStringCellDoorDescription,DOOR,EXIT|OPENABLE|OPENED|CLOSED,7,7,0,CELL,16);
		_objectState[3] = Object(_id, kStringLaptop,kStringDefaultDescription,NEWSPAPER,TAKE,6,6,8);
		_objectState[4] = Object(_id, kStringWristwatch,kStringDefaultDescription,WATCH,TAKE|COMBINABLE,255,255,8);
		_objectState[5] = Object(_id, kStringTable,kStringDefaultDescription,TABLE,COMBINABLE,5,5,0);
	}

	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussCorridor5 : public Room {
public:
	AxacussCorridor5(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR5;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[12] = kShownTrue;
		_shown[22] = kShownTrue;
		_shown[23] = kShownTrue;
		_shown[24] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,DOOR,EXIT,2,2,0,NULLROOM,2);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR6,22);

		_dialog1[0] = kStringDialogAxacussCorridor5_1;
		_dialog1[1] = kStringDialogAxacussCorridor5_2;
		_dialog2[0] = kStringDialogAxacussCorridor5_3;
		_dialog2[1] = kStringDialogAxacussCorridor5_4;
		_dialog3[0] = kStringDialogAxacussCorridor5_5;
		_dialog3[1] = kStringDialogAxacussCorridor5_6;
		_dialog3[2] = kStringDialogAxacussCorridor5_7;
		_dialog3[3] = kStringDialogAxacussCorridor5_7;

		_rows[0] = 1;
		_rows[1] = 1;
		_rows[2] = 1;
		_rows[3] = 1;
		_rows[4] = 0;
		_rows[5] = 0;
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void stopInteract(int sum);
	bool handleMoneyDialog();

	// TODO: Change to 6, or change struct, and fix initialization
	StringID _dialog1[2];
	StringID _dialog2[2];
	StringID _dialog3[4];

	byte _rows[6];
};

class AxacussCorridor6 : public Room {
public:
	AxacussCorridor6(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR6;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[6] = kShownTrue;
		_shown[22] = kShownTrue;
		_shown[24] = kShownTrue;
		_shown[25] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR5,2);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR7,22);
		_objectState[2] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,OPENABLE|CLOSED,255,255,0,CORRIDOR8,13);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussCorridor7 : public Room {
public:
	AxacussCorridor7(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR7;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[10] = kShownTrue;
		_shown[21] = kShownTrue;
		_shown[24] = kShownTrue;
		_shown[25] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR6,2);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,GUARD,22);
	}

	virtual void onEntrance();
};
class AxacussCorridor8 : public Room {
public:
	AxacussCorridor8(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR8;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[4] = kShownTrue;
		_shown[15] = kShownTrue;
		_shown[20] = kShownTrue;
		_shown[22] = kShownTrue;
		_shown[28] = kShownTrue;

		_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE,0,0,0,CORRIDOR6,10);
		_objectState[1] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,3,3,0,BCORRIDOR,22);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussCorridor9 : public Room {
public:
	AxacussCorridor9(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR9;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[14] = kShownTrue;
		_shown[19] = kShownTrue;
		_shown[23] = kShownTrue;
		_shown[28] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,2,2,0,BCORRIDOR,2);
		_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE,0,0,0,GUARD,10);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussBcorridor : public Room {
public:
	AxacussBcorridor(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 6;
		_id = BCORRIDOR;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;

		_objectState[0] = Object(_id, kStringPillar,kStringDefaultDescription,PILLAR1,NULLTYPE,4,4,0);
		_objectState[1] = Object(_id, kStringPillar,kStringDefaultDescription,PILLAR2,NULLTYPE,5,5,0);
		_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,6,6,0,CORRIDOR8,2);
		_objectState[3] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,7,7,0,CORRIDOR9,22);
		_objectState[4] = Object(_id, kStringDoor,kStringDoorDescription1,DOOR1,EXIT|OPENABLE|CLOSED|OCCUPIED,0,0,1,OFFICE_L1,6);
		_objectState[5] = Object(_id, kStringDoor,kStringDoorDescription2,DOOR2,EXIT|OPENABLE|CLOSED|OCCUPIED,1,1,2,OFFICE_L2,16);
		_objectState[6] = Object(_id, kStringDoor,kStringDoorDescription3,DOOR3,EXIT|OPENABLE|OPENED,2,2,3,OFFICE_R1,8);
		_objectState[7] = Object(_id, kStringDoor,kStringDoorDescription4,DOOR4,EXIT|OPENABLE|CLOSED|OCCUPIED,3,3,4,OFFICE_R2,18);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussIntersection : public Room {
public:
	AxacussIntersection(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 40;
		_id = GUARD;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit, kStringDefaultDescription, NULLOBJECT, EXIT, 255, 255, 0, CORRIDOR4, 21);
		_objectState[1] = Object(_id, kStringCorridor, kStringDefaultDescription, NULLOBJECT, EXIT, 3, 3, 0, CORRIDOR7, 5);
		_objectState[2] = Object(_id, kStringDoor, kStringDefaultDescription, DOOR, EXIT | OPENABLE, 1, 1, 6, CORRIDOR9, 3);
		_objectState[3] = Object(_id, kStringAxacussan, kStringAxacussanDescription, GUARDIAN, TALK, 0, 0, 0);
		_objectState[4] = Object(_id, kStringImage, kStringImageDescription2, NULLOBJECT, NULLTYPE, 2, 2, 0);
		_objectState[5] = Object(_id, kStringMastercard, kStringMastercardDescription, MASTERKEYCARD, TAKE | COMBINABLE, 255, 255, 1);

		_dialogsX[0] = kStringDialogX1;
		_dialogsX[1] = kStringDialogX2;
		_dialogsX[2] = kStringDialogX3;
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringID _dialogsX[6];
};

class AxacussExit : public Room {
public:
	AxacussExit(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 42;
		_id = GUARD3;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,CORRIDOR1,22);
		_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,NULLOBJECT,EXIT|OPENABLE|CLOSED,0,0,0,NULLROOM,20);
		_objectState[2] = Object(_id, kStringDoor,kStringDefaultDescription,NULLOBJECT,EXIT|OPENABLE|CLOSED,1,1,0,NULLROOM,15);
		_objectState[3] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE,2,2,11,OFFICE_L,0);
		_objectState[4] = Object(_id, kStringLamp2,kStringDefaultDescription,LAMP,COMBINABLE,3,3,0);
		_objectState[5] = Object(_id, kStringAxacussan,kStringDefaultDescription,GUARDIAN,TALK,5,5,0);
		_objectState[6] = Object(_id, kStringImage,kStringGenericDescription5,NULLOBJECT,NULLTYPE,4,4,0);

		_dialogsX[0] = kStringDialogX1;
		_dialogsX[1] = kStringDialogX2;
		_dialogsX[2] = kStringDialogX3;
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringID _dialogsX[6];
};
class AxacussOffice1 : public Room {
public:
	AxacussOffice1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 7;
		_id = OFFICE_L1;
		_shown[0] = kShownTrue;
		_shown[2] = kShownTrue;
		_shown[7] = kShownTrue;
		_shown[9] = kShownTrue;
		_shown[16] = kShownTrue;

		_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,9,BCORRIDOR,9);
		_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object(_id, kStringMoney,kStringMoneyDescription1,MONEY,TAKE,255,255,0);
		_objectState[3] = Object(_id, kStringLocker,kStringLockerDescription,LOCKER,OPENABLE|CLOSED,5,5,0);
		_objectState[4] = Object(_id, kStringLetter,kStringDefaultDescription,LETTER,UNNECESSARY,3,3,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussOffice2 : public Room {
public:
	AxacussOffice2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 7;
		_id = OFFICE_L2;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[9] = kShownTrue;
		_shown[16] = kShownTrue;

		_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,9,BCORRIDOR,9);
		_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object(_id, kStringCube,kStringGenericDescription6,NULLOBJECT,NULLTYPE,0,0,0);
		_objectState[3] = Object(_id, kStringImage,kStringGenericDescription7,NULLOBJECT,NULLTYPE,1,1,0);
		_objectState[4] = Object(_id, kStringStrangeThing,kStringGenericDescription8,NULLOBJECT,UNNECESSARY,2,2,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussOffice3 : public Room {
public:
	AxacussOffice3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 8;
		_id = OFFICE_R1;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;
		_shown[3] = kShownTrue;

		_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,0,0,3,BCORRIDOR,5);
		_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object(_id, kStringImage,kStringImageDescription2,NULLOBJECT,UNNECESSARY,1,1,0);
		_objectState[3] = Object(_id, kStringImage,kStringImageDescription2,PAINTING,UNNECESSARY,2,2,0);
		_objectState[4] = Object(_id, kStringPlant,kStringDefaultDescription,NULLOBJECT,UNNECESSARY,3,3,0);
		_objectState[5] = Object(_id, kNoString,kStringDefaultDescription,MONEY,TAKE|COMBINABLE,255,255,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussOffice4 : public Room {
public:
	AxacussOffice4(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 8;
		_id = OFFICE_R2;
		_shown[0] = kShownTrue;
		_shown[2] = kShownTrue;
		_shown[3] = kShownTrue;

		_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,0,0,3,BCORRIDOR,5);
		_objectState[1] = Object(_id, kStringComputer,kStringDefaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object(_id, kStringStatue,kStringStatueDescription,NULLOBJECT,UNNECESSARY,6,6,0);
		_objectState[3] = Object(_id, kStringPlant,kStringPlantDescription,NULLOBJECT,UNNECESSARY,5,5,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussOffice5 : public Room {
public:
	AxacussOffice5(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 7;
		_id = OFFICE_L;
		_shown[0] = kShownTrue;
		_shown[3] = kShownTrue;
		_shown[5] = kShownTrue;
		_shown[17] = kShownTrue;

		_objectState[0] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,17,GUARD3,9);
		_objectState[1] = Object(_id, kStringComputer,kStringComputerDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object(_id, kStringGraffiti,kStringGraffitiDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[3] = Object(_id, kStringMoney,kStringMoneyDescription2,MONEY,TAKE,8,8,0);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussElevator : public Room {
public:
	AxacussElevator(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 3;
		_id = ELEVATOR;
		_shown[0] = kShownTrue;

		_objectState[0] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON1,PRESS,0,0,0);
		_objectState[1] = Object(_id, kStringButton,kStringDefaultDescription,BUTTON2,PRESS,1,1,0);
		_objectState[2] = Object(_id, kStringExit,kStringDefaultDescription,DOOR,EXIT,255,255,0,NULLROOM,22);
		_objectState[3] = Object(_id, kStringJungle,kStringJungleDescription,JUNGLE,NULLTYPE,255,255,0,STATION,2);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussStation : public Room {
public:
	AxacussStation(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 5;
		_id = STATION;
		_shown[0] = kShownTrue;
		_objectState[0] = Object(_id, kStringSign,kStringDefaultDescription,STATION_SIGN,NULLTYPE,0,0,0);
		_objectState[1] = Object(_id, kStringDoor,kStringDefaultDescription,DOOR,EXIT|OPENABLE|CLOSED,1,1,0,NULLROOM,7);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussSign : public Room {
public:
	AxacussSign(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 32;
		_id = SIGN;
		_shown[0] = kShownTrue;
		_shown[1] = kShownTrue;

		_objectState[0] = Object(_id, kStringExit,kStringDefaultDescription,NULLOBJECT,EXIT,255,255,0,STATION,22);
		_objectState[1] = Object(_id, kStringSlot,kStringDefaultDescription,STATION_SLOT,COMBINABLE,0,0,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class Outro : public Room {
public:
	Outro(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual void animation();

private:
	void animate(int filenumber, int section1, int section2, int duration);
	void animate(int filenumber, int section1, int section2, int duration, MessagePosition position,
	             const char *text);
	void animate(int filenumber, int section1, int section2, int section3, int section4, int duration,
	             MessagePosition position, const char *text);

	Common::String _outroText;
};

}
#endif // SUPERNOVA_ROOMS_H
