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

#ifndef ROOMS_H
#define ROOMS_H

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
			_shown[i] = false;
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
		_shown[section] = visible;
	}
	bool isSectionVisible(uint index) const {
		return _shown[index];
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

protected:
	int _fileNumber;
	bool _shown[kMaxSection];
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
	Intro(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = -1;
		_id = INTRO;
		_shown[0] = false;

		_objectState[0] =
		    Object("Keycard", "Die Keycard fuer deine Schraenke.", KEYCARD,
		           TAKE | CARRIED | COMBINABLE, 255, 255, 0, NULLROOM, 0);
		_objectState[1] =
		    Object("Taschenmesser", "Es ist nicht mehr das sch\204rfste.", KNIFE,
		           TAKE | CARRIED | COMBINABLE, 255, 255, 0, NULLROOM, 0);
		_objectState[2] =
		    Object("Armbanduhr", Object::defaultDescription, WATCH,
		           TAKE | COMBINABLE | CARRIED, 255, 255, 8, NULLROOM, 0);
		_objectState[3] =
		    Object("Discman", "Es ist eine \"Mad Monkeys\"-CD darin.", DISCMAN,
		           TAKE | COMBINABLE, 255, 255, 0, NULLROOM, 0);
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

// Spaceship
class ShipCorridor : public Room {
public:
	ShipCorridor(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 17;
		_id = CORRIDOR;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = true;

		_objectState[0] = Object("Luke", Object::defaultDescription, HATCH1, OPENABLE | EXIT, 0, 6, 1, CABIN_L1, 15);
		_objectState[1] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | EXIT, 1, 7, 2, CABIN_L2, 10);
		_objectState[2] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | EXIT, 2, 8, 3, CABIN_L3, 5);
		_objectState[3] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | EXIT, 5, 11, 6, CABIN_R1, 19);
		_objectState[4] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | EXIT, 4, 10, 5, CABIN_R2, 14);
		_objectState[5] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | EXIT | OPENED, 9, 3, 4, CABIN_R3, 9);
		_objectState[6] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | CLOSED | EXIT, 12, 12, 0, AIRLOCK, 2);
		_objectState[7] = Object("Knopf", "Er geh\224rt zu der gro�en Luke.", BUTTON, PRESS, 13, 13, 0, NULLROOM, 0);
		_objectState[8] = Object("Leiter", Object::defaultDescription, NULLOBJECT, NULLTYPE, 14, 14, 0, NULLROOM, 0);
		_objectState[9] = Object("Ausgang", Object::defaultDescription, NULLOBJECT, EXIT, 15, 15, 0, HALL, 22);
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
		_shown[0] = true;

		_objectState[0] = Object("Luke", "Sie f\204hrt ins Cockpit.", NULLOBJECT, OPENABLE | EXIT, 4, 5, 1, COCKPIT, 10);
		_objectState[1] = Object("Luke", "Sie f\204hrt zur K\201che.", KITCHEN_HATCH, OPENABLE | EXIT, 0, 0, 0, NULLROOM, 1);
		_objectState[2] = Object("Luke", "Sie f\204hrt zu den Tiefschlafkammern.", NULLOBJECT, OPENABLE | CLOSED | EXIT, 1, 1, 2, SLEEP, 8);
		_objectState[3] = Object("Schlitz", "Es ist ein Keycard-Leser.", SLEEP_SLOT, COMBINABLE, 2, 2, 0, NULLROOM, 0);
		_objectState[4] = Object("Leiter", Object::defaultDescription, NULLOBJECT, NULLTYPE, 3, SLEEP, 0, NULLROOM, 0);
		_objectState[5] = Object("Gang", Object::defaultDescription, NULLOBJECT, EXIT, 6, 6, 0, CORRIDOR, 19);
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
		_shown[0] = true;

		_objectState[0] = Object("Luke", "Dies ist eine der Tiefschlafkammern.", CABINS, NULLTYPE, 0, 0, 0, NULLROOM, 0);
		_objectState[1] = Object("Luke", "Dies ist eine der Tiefschlafkammern.", CABIN, NULLTYPE, 1, 1, 0, NULLROOM, 0);
		_objectState[2] = Object("Computer", Object::defaultDescription, COMPUTER, NULLTYPE, 2, 2, 0, NULLROOM, 0);
		_objectState[3] = Object("Ausgang", Object::defaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);

		_codeword_DE = "ZWEIUNDVIERZIG";
		_codeword_EN = "FORTYTWO";
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	Common::String _codeword_DE;
	Common::String _codeword_EN;
	byte _color;
};

class ShipCockpit : public Room {
public:
	ShipCockpit(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 9;
		_id = COCKPIT;
		_shown[0] = true;

		_objectState[0] = Object("Instrumente", "Hmm, sieht ziemlich kompliziert aus.", INSTRUMENTS, NULLTYPE, 2, 2, 0, NULLROOM, 0);
		_objectState[1] = Object("Monitor", Object::defaultDescription, MONITOR, NULLTYPE, 0, 0, 0, NULLROOM, 0);
		_objectState[2] = Object("Monitor", "Dieser Monitor sagt dir nichts.", NULLOBJECT, TAKE, 1, 0, 0, NULLROOM, 0);
		_objectState[3] = Object("Ausgang", Object::defaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 22);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = true;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;

		_objectState[0] = Object("Bild", "Herb!", NULLOBJECT, UNNECESSARY, 5, 5, 0, NULLROOM, 0);
		_objectState[1] = Object("Bild", "Toll!", NULLOBJECT, UNNECESSARY, 6, 6, 0, NULLROOM, 0);
		_objectState[2] = Object("Bild", "Genial!", NULLOBJECT, UNNECESSARY, 7, 7, 0, NULLROOM, 0);
		_objectState[3] = Object("Magnete", "Damit werden Sachen auf|dem Tisch festgehalten.", NULLOBJECT, UNNECESSARY, 8, 8, 0, NULLROOM, 0);
		_objectState[4] = Object("Bild", "Es scheint noch nicht fertig zu sein.", NULLOBJECT, UNNECESSARY, 9, 9, 0);
		_objectState[5] = Object("Stift", "Ein Kugelschreiber.", PEN, TAKE | COMBINABLE, 10, 10, 5 | 128);
		_objectState[6] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPENABLE | OPENED | EXIT, 3, 3, 24 | 128, CORRIDOR, 9);
		_objectState[7] = Object("Schlitz", "Es ist ein Keycard-Leser.", NULLOBJECT, COMBINABLE, 0, 0, 0);
		_objectState[8] = Object("Schrank", Object::defaultDescription, NULLOBJECT, OPENABLE | CLOSED, 1, 1, 0);
		_objectState[9] = Object("Fach", Object::defaultDescription, NULLOBJECT, OPENABLE | CLOSED, 2, 2, 0);
		_objectState[10] = Object("Steckdose", Object::defaultDescription, SOCKET, COMBINABLE, 4, 4, 0);
		_objectState[11] = Object("Toilette", Object::defaultDescription, BATHROOM_DOOR, EXIT, 255, 255, 0, BATHROOM, 22);
	}
};

class ShipCabinL2 : public Room {
public:
	ShipCabinL2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 21;
		_id = CABIN_L2;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = true;

		_objectState[0] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_KL1,COMBINABLE,31,31,0);
		_objectState[1] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_KL2,COMBINABLE,32,32,0);
		_objectState[2] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_KL3,COMBINABLE,33,33,0);
		_objectState[3] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_KL4,COMBINABLE,45,45,0);
		_objectState[4] = Object("Schrank",Object::defaultDescription,SHELF_L1,OPENABLE | CLOSED,25,26,17);
		_objectState[5] = Object("Pistole","Es ist keine Munition drin.",PISTOL,TAKE,39,39,20);
		_objectState[6] = Object("Fach",Object::defaultDescription,SHELF_L2,OPENABLE | CLOSED,27,28,18);
		_objectState[7] = Object("B\201cher","Lauter wissenschaftliche B\201cher.",NULLOBJECT,UNNECESSARY,40,40,0);
		_objectState[8] = Object("Fach",Object::defaultDescription,SHELF_L3,OPENABLE | CLOSED,29,30,19);
		_objectState[9] = Object("Kabelrolle","Da sind mindestens zwanzig Meter drauf.", SPOOL,TAKE | COMBINABLE,41,41,21);
		_objectState[10] = Object("Fach",Object::defaultDescription,SHELF_L4,OPENABLE | CLOSED,43,44,22);
		_objectState[11] = Object("Buch",Object::defaultDescription,BOOK2,TAKE,46,46,23);
		_objectState[12] = Object("Unterw\204sche","Ich habe keine Lust, in|der Unterw\204sche des|Commanders rumzuw\201hlen.",NULLOBJECT,UNNECESSARY,34,34,0);
		_objectState[13] = Object("Unterw\204sche","Ich habe keine Lust, in|der Unterw\204sche des|Commanders rumzuw\201hlen.",NULLOBJECT,UNNECESSARY,35,35,0);
		_objectState[14] = Object("Kleider",Object::defaultDescription,NULLOBJECT,UNNECESSARY,36,36,0);
		_objectState[15] = Object("Krimskram","Es ist nichts brauchbares dabei.",NULLOBJECT,UNNECESSARY,37,37,0);
		_objectState[16] = Object("Krimskram","Es ist nichts brauchbares dabei.",NULLOBJECT,UNNECESSARY,38,38,0);
		_objectState[17] = Object("Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",NULLOBJECT,UNNECESSARY,23,23,0);
		_objectState[18] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
		_objectState[19] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,24 | 128,CORRIDOR,9);
		_objectState[20] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[21] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[22] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[23] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[24] = Object("Ordner","Darauf steht \"Dienstanweisungen|zur Mission Supernova\".|Es steht nichts wichtiges drin.",NULLOBJECT,UNNECESSARY,49,49,0);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = true;
		_shown[7] = true;
		_shown[8] = true;
		_shown[9] = true;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = true;

		_objectState[0] = Object("Poster","Ein Poster von \"Big Boss\".",NULLOBJECT,UNNECESSARY,11,11,0);
		_objectState[1] = Object("Poster","Ein Poster von \"Rock Desaster\".",NULLOBJECT,UNNECESSARY,12,12,0);
		_objectState[2] = Object("Box",Object::defaultDescription,NULLOBJECT,NULLTYPE,13,13,0);
		_objectState[3] = Object("Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",NULLOBJECT,UNNECESSARY,14,14,0);
		_objectState[4] = Object("Schallplatte","Die Platte ist von \"Big Boss\".",RECORD,TAKE | COMBINABLE,15,15,8 | 128);
		_objectState[5] = Object("Schallplattenst\204nder","Du hast jetzt keine Zeit, in|der Plattensammlung rumzust\224bern.",NULLOBJECT,UNNECESSARY,16,16,0);
		_objectState[6] = Object("Knopf",Object::defaultDescription,TURNTABLE_BUTTON,PRESS,22,22,0);
		_objectState[7] = Object("Plattenspieler","Sieht aus, als k�me|er aus dem Museum.",TURNTABLE,UNNECESSARY | COMBINABLE,17,17,0);
		_objectState[8] = Object("Leitung",Object::defaultDescription,WIRE,COMBINABLE,18,18,0);
		_objectState[9] = Object("Leitung",Object::defaultDescription,WIRE2,COMBINABLE,19,19,0);
		_objectState[10] = Object("Stecker",Object::defaultDescription,PLUG,COMBINABLE,20,20,0);
		_objectState[11] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,24 | 128,CORRIDOR,9);
		_objectState[12] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[13] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[14] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[15] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[16] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = true;

		_objectState[0] = Object("Bild","Manche Leute haben schon|einen komischen Geschmack.",NULLOBJECT,UNNECESSARY,5,5,0);
		_objectState[1] = Object("Zeichenger\204te","Auf dem Zettel sind lauter|unverst\204ndliche Skizzen und Berechnungen.|(Jedenfalls f\201r dich unverst\204ndlich.)",NULLOBJECT,UNNECESSARY,6,6,0);
		_objectState[2] = Object("Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",NULLOBJECT,UNNECESSARY,7,7,0);
		_objectState[3] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[4] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[5] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[6] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[7] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[8] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	}
};

class ShipCabinR2 : public Room {
public:
	ShipCabinR2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_id = CABIN_R2;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;

		_objectState[0] = Object("Schachspiel","Es macht wohl Spa\341, an|der Decke Schach zu spielen.",NULLOBJECT,UNNECESSARY,11,11,0);
		_objectState[1] = Object("Tennisschl\204ger","Fliegt Boris Becker auch mit?",NULLOBJECT,UNNECESSARY,8,8,0);
		_objectState[2] = Object("Tennisball","Toll!",NULLOBJECT,UNNECESSARY,9,9,0);
		_objectState[3] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPENABLE | OPENED | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[4] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[5] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[6] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[7] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[8] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
	}
};

class ShipCabinR3 : public Room {
public:
	ShipCabinR3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_id = CABIN_R3;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = true;
		_shown[8] = true;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = true;

		_objectState[0] = Object("Schachspiel","Dein Magnetschachspiel. Schach war|schon immer deine Leidenschaft.",CHESS,TAKE | COMBINABLE,12,12,7 | 128);
		_objectState[1] = Object("Bett","Das ist dein Bett. Toll, nicht wahr?",NULLOBJECT,NULLTYPE,13,13,0);
		_objectState[2] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_K1,COMBINABLE,27,27,0);
		_objectState[3] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_K2,COMBINABLE,28,28,0);
		_objectState[4] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_K3,COMBINABLE,29,29,0);
		_objectState[5] = Object("Schlitz","Es ist ein Keycard-Leser.",SLOT_K4,COMBINABLE,30,30,0);
		_objectState[6] = Object("Fach","Das ist eins deiner drei F\204cher.",SHELF1,OPENABLE | CLOSED,14,18,9);
		_objectState[7] = Object("Alben","Deine Briefmarkensammlung.",NULLOBJECT,UNNECESSARY,14,14,0);
		_objectState[8] = Object("Fach","Das ist eins deiner drei F\204cher.",SHELF2,OPENABLE | CLOSED,15,19,10);
		_objectState[9] = Object("Seil","Es ist ungef\204hr 10 m lang und 4 cm dick.",ROPE,TAKE | COMBINABLE,15,15,12);
		_objectState[10] = Object("Schrank","Das ist dein Schrank.",SHELF3,OPENABLE | CLOSED,16,17,11);
		_objectState[11] = Object("Krimskram","Es ist nichts brauchbares dabei.",NULLOBJECT,UNNECESSARY,20,20,0);
		_objectState[12] = Object("Kleider","Es sind Standard-Weltraum-Klamotten.",NULLOBJECT,UNNECESSARY,21,21,0);
		_objectState[13] = Object("Unterw\204sche",Object::defaultDescription,NULLOBJECT,UNNECESSARY,22,22,0);
		_objectState[14] = Object("Str\201mpfe",Object::defaultDescription,NULLOBJECT,UNNECESSARY,23,23,0);
		_objectState[15] = Object("Fach","Das ist eins deiner drei F\204cher.",SHELF4,OPENABLE | CLOSED,24,25,13);
		_objectState[16] = Object("Buch","Es ist|\"Per Anhalter durch die Galaxis\"|von Douglas Adams.",BOOK,TAKE,26,26,14);
		_objectState[17] = Object("Discman","Es ist eine \"Mad Monkeys\"-CD darin.",DISCMAN,TAKE | COMBINABLE,33,33,16);
		_objectState[18] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPENABLE | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[19] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[20] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,1,1,0);
		_objectState[21] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,2,2,0);
		_objectState[22] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[23] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
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
		_shown[0] = true;

		_objectState[0] = Object("Klo","Ein Klo mit Saugmechanismus.",TOILET,NULLTYPE,0,0,0);
		_objectState[1] = Object("Dusche",Object::defaultDescription,SHOWER,NULLTYPE,1,1,0);
		_objectState[2] = Object("Ausgang",Object::defaultDescription,BATHROOM_EXIT,EXIT,255,255,0,CABIN_R3,2);
	}
};

class ShipAirlock : public Room {
public:
	ShipAirlock(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 34;
		_id = AIRLOCK;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = true;

		_objectState[0] = Object("Luke","Das ist eine Luke !!!",NULLOBJECT,EXIT | OPENABLE | OPENED | CLOSED,0,0,0,CORRIDOR,10);
		_objectState[1] = Object("Luke","Dies ist eine Luke !!!",NULLOBJECT,EXIT | OPENABLE | CLOSED,1,1,0,HOLD,14);
		_objectState[2] = Object("Knopf",Object::defaultDescription,BUTTON1,PRESS,2,2,0);
		_objectState[3] = Object("Knopf",Object::defaultDescription,BUTTON2,PRESS,3,3,0);
		_objectState[4] = Object("Helm","Es ist der Helm zum Raumanzug.",HELMET,TAKE,4,4,7);
		_objectState[5] = Object("Raumanzug","Der einzige Raumanzug, den die|anderen hiergelassen haben ...",SUIT,TAKE,5,5,8);
		_objectState[6] = Object("Versorgung","Es ist der Versorgungsteil zum Raumanzug.",LIFESUPPORT,TAKE,6,6,9);
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
		_shown[0] = true;

		_objectState[0] = Object("",Object::defaultDescription,HOLD_WIRE,COMBINABLE,255,255,0);
		_objectState[1] = Object("Schrott","Da ist eine L\201sterklemme dran, die|noch ganz brauchbar aussieht.|Ich nehme sie mit.",SCRAP_LK,NULLTYPE,4,4,0);
		_objectState[2] = Object("L\201sterklemme",Object::defaultDescription,TERMINALSTRIP,COMBINABLE,255,255,0);
		_objectState[3] = Object("Schrott","Junge, Junge! Die Explosion hat ein|ganz sch\224nes Durcheinander angerichtet.",NULLOBJECT,NULLTYPE,5,5,0);
		_objectState[4] = Object("Reaktor","Das war einmal der Reaktor.",NULLOBJECT,NULLTYPE,6,6,0);
		_objectState[5] = Object("D\201se",Object::defaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[6] = Object("blauer K\201rbis","Keine Ahnung, was das ist.",NULLOBJECT,NULLTYPE,8,8,0);
		_objectState[7] = Object("Luke",Object::defaultDescription,LANDINGMOD_OUTERHATCH,EXIT | OPENABLE,1,2,2,LANDINGMODULE,6);
		_objectState[8] = Object("Landef\204hre","Sie war eigentlich f\201r Bodenuntersuchungen|auf Arsano 3 gedacht.",NULLOBJECT,NULLTYPE,0,0,0);
		_objectState[9] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,AIRLOCK,22);
		_objectState[10] = Object("Luke","Sie f\201hrt nach drau\341en.",OUTERHATCH_TOP,EXIT | OPENABLE | OPENED,3,3,0,GENERATOR,8);
		_objectState[11] = Object("Generator","Er versorgt das Raumschiff mit Strom.",GENERATOR_TOP,EXIT,12,12,0,GENERATOR,8);

		_descriptionScrap = "Ein St\201ck Schrott.";
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();

private:
	Common::String _descriptionScrap;
};

class ShipLandingModule : public Room {
public:
	ShipLandingModule(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 25;
		_id = LANDINGMODULE;
		_shown[0] = true;

		_objectState[0] = Object("Steckdose",Object::defaultDescription,LANDINGMOD_SOCKET,COMBINABLE,1,1,0);
		_objectState[1] = Object("Knopf","Es ist ein Sicherheitsknopf.|Er kann nur mit einem spitzen|Gegenstand gedr\201ckt werden.",LANDINGMOD_BUTTON,PRESS | COMBINABLE,2,2,0);
		_objectState[2] = Object("Monitor",Object::defaultDescription,LANDINGMOD_MONITOR,NULLTYPE,3,3,0);
		_objectState[3] = Object("Tastatur",Object::defaultDescription,KEYBOARD,NULLTYPE,4,4,0);
		_objectState[4] = Object("",Object::defaultDescription,LANDINGMOD_WIRE,COMBINABLE,255,255,0);
		_objectState[5] = Object("Luke",Object::defaultDescription,LANDINGMOD_HATCH,EXIT | OPENABLE | OPENED | COMBINABLE, 0,0,1 | 128,HOLD,10);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = true;

		_objectState[0] = Object("langes Kabel mit Stecker",Object::defaultDescription,GENERATOR_WIRE,COMBINABLE,255,255,0);
		_objectState[1] = Object("leere Kabelrolle",Object::defaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
		_objectState[2] = Object("Keycard des Commanders","Hey, das ist die Keycard des Commanders!|Er mu\341 sie bei dem \201berst\201rzten|Aufbruch verloren haben.",KEYCARD2,COMBINABLE | TAKE,12,12,5 | 128);
		_objectState[3] = Object("Seil",Object::defaultDescription,GENERATOR_ROPE,COMBINABLE,255,255,0);
		_objectState[4] = Object("Luke","Sie f\201hrt nach drau\341en.",OUTERHATCH,EXIT | OPENABLE,1,2,1,OUTSIDE,22);
		_objectState[5] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPENABLE | CLOSED,3,3,0);
		_objectState[6] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,4,4,0);
		_objectState[7] = Object("Klappe",Object::defaultDescription,TRAP,OPENABLE,5,6,2);
		_objectState[8] = Object("Leitung",Object::defaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[9] = Object("Spannungmessger\204t",Object::defaultDescription,VOLTMETER,NULLTYPE,9,9,0,NULLROOM,0);
		_objectState[10] = Object("Klemme",Object::defaultDescription,CLIP,COMBINABLE,8,8,0);
		_objectState[11] = Object("Leitung","Sie f\201hrt vom Generator zum Spannungmessger\204t.",SHORT_WIRE,COMBINABLE,10,10,0);
		_objectState[12] = Object("Leiter",Object::defaultDescription,LADDER,EXIT,0,0,0,HOLD,1);
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
		_shown[0] = true;

		_objectState[0] = Object("Luke",Object::defaultDescription,NULLOBJECT,EXIT,0,0,0,GENERATOR,3);
		_objectState[1] = Object("Seil",Object::defaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
	}
};

// Arsano
class ArsanoRocks : public Room {
public:
	ArsanoRocks(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 12;
		_shown[0] = true;

		_objectState[0] = Object("Seil",Object::defaultDescription,NULLOBJECT,UNNECESSARY | EXIT,0,0,0,GENERATOR,12);
		_objectState[1] = Object("Stein",Object::defaultDescription,STONE,NULLTYPE,1,1,0);
		_objectState[2] = Object("Stein",Object::defaultDescription,NULLOBJECT,NULLTYPE,2,2,0);
		_objectState[3] = Object("Loch","Es scheint eine H\224hle zu sein.",NULLOBJECT,NULLTYPE,255,255,0,CAVE,1);
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
		_shown[0] = false;

		_objectState[0] = Object("Ausgang","Hier bist du gerade hergekommen.",NULLOBJECT,EXIT,255,255,0,ROCKS,22);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,MEETUP,2);
	}
};
class ArsanoMeetup : public Room {
public:
	ArsanoMeetup(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 37;
		_id = MEETUP;
		_shown[0] = true;

		_objectState[0] = Object("H\224hle",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);
		_objectState[1] = Object("Schild","Diese Schrift kannst du nicht lesen.",MEETUP_SIGN,NULLTYPE,0,0,0);
		_objectState[2] = Object("Eingang",Object::defaultDescription,DOOR,EXIT,1,1,0,ENTRANCE,7);
		_objectState[3] = Object("Stern",Object::defaultDescription,STAR,NULLTYPE,2,2,0);
		_objectState[4] = Object("Raumschiff",Object::defaultDescription,SPACESHIPS,COMBINABLE,3,3,0);
		_objectState[5] = Object("Raumschiff",Object::defaultDescription,SPACESHIP,COMBINABLE,4,4,0);
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
		_shown[0] = true;

		_objectState[0] = Object("Portier","Du siehst doch selbst, wie er aussieht.",PORTER,TALK,0,0,0);
		_objectState[1] = Object("T\201r",Object::defaultDescription,NULLOBJECT,EXIT | OPENABLE | CLOSED,1,1,0,NULLROOM,5);
		_objectState[2] = Object("Schild","Diese Schrift kannst du nicht lesen.",KITCHEN_SIGN,NULLTYPE,2,2,0);
		_objectState[3] = Object("Kaugummi",Object::defaultDescription,SCHNUCK,TAKE,255,255,10+128);
		_objectState[4] = Object("Gummib\204rchen",Object::defaultDescription,SCHNUCK,TAKE,255,255,11+128);
		_objectState[5] = Object("Schokokugel",Object::defaultDescription,SCHNUCK,TAKE,255,255,12+128);
		_objectState[6] = Object("\232berraschungsei",Object::defaultDescription,EGG,TAKE,255,255,13+128);
		_objectState[7] = Object("Lakritz",Object::defaultDescription,SCHNUCK,TAKE,255,255,14+128);
		_objectState[8] = Object("Tablette","Die Plastikh\201lle zeigt einen|Mund mit einer Sprechblase. Was|darin steht, kannst du nicht lesen.",PILL,TAKE,255,255,0);
		_objectState[9] = Object("Schlitz",Object::defaultDescription,CAR_SLOT,COMBINABLE,6,6,0);
		_objectState[10] = Object("Automat","Sieht aus wie ein Kaugummiautomat.",NULLOBJECT,NULLTYPE,5,5,0);
		_objectState[11] = Object("Toilette","Die Toiletten sind denen|auf der Erde sehr \204hnlich.",ARSANO_BATHROOM,NULLTYPE,255,255,0);
		_objectState[12] = Object("Knopf",Object::defaultDescription,BATHROOM_BUTTON,PRESS,3,3,0);
		_objectState[13] = Object("Schild","Diese Schrift kannst du nicht lesen.",BATHROOM_SIGN,NULLTYPE,4,4,0);
		_objectState[14] = Object("Treppe",Object::defaultDescription,STAIRCASE,EXIT,8,8,0,REST,3);
		_objectState[15] = Object("Ausgang",Object::defaultDescription,MEETUP_EXIT,EXIT,255,255,0,MEETUP,22);
		_objectState[16] = Object("M\201nzen","Es sind seltsame|K\224pfe darauf abgebildet.",COINS,TAKE|COMBINABLE,255,255,0);
		_objectState[17] = Object("Tablettenh\201lle","Darauf steht:\"Wenn Sie diese|Schrift jetzt lesen k\224nnen,|hat die Tablette gewirkt.\"",PILL_HULL,TAKE,255,255,0);

		_dialog1[0] = "Wieso das denn nicht?";
		_dialog1[1] = "Wo bin ich hier?";
		_dialog1[2] = "Wo soll ich die Schuhe ablegen?";
		_dialog1[3] = "Schwachsinn! Ich gehe jetzt nach oben!";
		_dialog1[4] = "|";

		_dialog2[0] = "Wo bin ich hier?";
		_dialog2[1] = "Sch\224nes Wetter heute, nicht wahr?";
		_dialog2[2] = "W\201rden Sie mich bitte durchlassen.";
		_dialog2[3] = "Hey Alter, la\341 mich durch!";
		_dialog2[4] = "|";

		_dialog3[0] = "Was haben Sie gesagt?";
		_dialog3[1] = "Sprechen Sie bitte etwas deutlicher!";

		_eyewitness = 5;
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();

private:
	Common::String _dialog1[5];
	Common::String _dialog2[5];
	Common::String _dialog3[5];
	byte _eyewitness;
};
class ArsanoRemaining : public Room {
public:
	ArsanoRemaining(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 28;
		_id = REST;
		_shown[0] = true;

		_objectState[0] = Object("Treppe",Object::defaultDescription,NULLOBJECT,EXIT,0,0,0,ENTRANCE,17);
		_objectState[1] = Object("Stuhl",Object::defaultDescription,NULLOBJECT,EXIT,1,1,0,ROGER,2);
		_objectState[2] = Object("Schuhe","Wie ist der denn mit|Schuhen hier reingekommen?",NULLOBJECT,NULLTYPE,2,2,0);

		_chewing = true;
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
		_shown[0] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,REST,19);
		_objectState[1] = Object("Froschgesicht",Object::defaultDescription,ROGER_W,TALK,0,0,0);
		_objectState[2] = Object("Gekritzel","\"Mr Spock was here\"",NULLOBJECT,NULLTYPE,3,3,0);
		_objectState[3] = Object("Brieftasche",Object::defaultDescription,WALLET,TAKE,1,1,4);
		_objectState[4] = Object("Speisekarte","\"Heute empfehlen wir:|Fonua Opra mit Ulk.\"",NULLOBJECT,UNNECESSARY,2,2,0);
		_objectState[5] = Object("Tasse","Sie enth\204lt eine gr\201nliche Fl\201ssigkeit.",CUP,UNNECESSARY,4,4,0);
		_objectState[6] = Object("Schachspiel",Object::defaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
		_objectState[7] = Object("10-Buckazoid-Schein","Nicht gerade sehr viel Geld.",NULLOBJECT,TAKE|COMBINABLE,255,255,0);
		_objectState[8] = Object("Keycard von Roger",Object::defaultDescription,KEYCARD_R,TAKE|COMBINABLE,255,255,0);

		_dialog1[0] = "K\224nnten Sie mir ein Gericht empfehlen?";
		_dialog1[1] = "Wie lange dauert es denn noch bis zur Supernova?";
		_dialog1[2] = "Sie kommen mir irgendwie bekannt vor.";
		_dialog1[3] = "|";

		_eyewitness = 5;
	}

	virtual void animation();
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	Common::String _dialog1[4];
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
		_shown[0] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,MEETUP,15);
		_objectState[1] = Object("Knopf",Object::defaultDescription,GLIDER_BUTTON1,PRESS,0,0,0);
		_objectState[2] = Object("Knopf",Object::defaultDescription,GLIDER_BUTTON2,PRESS,1,1,0);
		_objectState[3] = Object("Knopf",Object::defaultDescription,GLIDER_BUTTON3,PRESS,2,2,0);
		_objectState[4] = Object("Knopf",Object::defaultDescription,GLIDER_BUTTON4,PRESS,3,3,0);
		_objectState[5] = Object("Keycard von Roger",Object::defaultDescription,GLIDER_KEYCARD,TAKE|COMBINABLE,255,255,0);
		_objectState[6] = Object("Schlitz",Object::defaultDescription,GLIDER_SLOT,COMBINABLE,4,4,0);
		_objectState[7] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPENABLE,5,6,6);
		_objectState[8] = Object("Tastatur",Object::defaultDescription,GLIDER_BUTTONS,NULLTYPE,7,7,0);
		_objectState[9] = Object("Anzeige","Hmm, seltsame Anzeigen.",GLIDER_DISPLAY,NULLTYPE,8,8,0);
		_objectState[10] = Object("Instrumente","Hmm, seltsame Anzeigen.",GLIDER_INSTRUMENTS,NULLTYPE,9,9,0);
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
		_shown[0] = true;

		_objectState[0] = Object("Roger W.",Object::defaultDescription,ROGER_W,TALK,255,255,0);
		_objectState[1] = Object("Raumschiff",Object::defaultDescription,SPACESHIP,COMBINABLE,255,255,0);
		_objectState[2] = Object("Höhle",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);

		_dialog1[0] = "Ach, Ihnen geh\224rt die. Ich habe sie eben im Sand gefunden.";
		_dialog1[1] = "Nein, tut mir leid.";
		_dialog2[0] = "Nein, danke. Ich bleibe lieber hier.";
		_dialog2[1] = "Ja, das w\204re gut.";
		_dialog3[0] = "Zur Erde.";
		_dialog3[1] = "Zum Pr\204sident der Galaxis.";
		_dialog3[2] = "Nach Xenon.";
		_dialog3[3] = "Mir egal, setzen Sie mich irgendwo ab!";
		_dialog4[0] = "Ich habe gerade Ihre Brieftasche gefunden!";
		_dialog4[1] = "Sie lag da dr\201ben hinter einem Felsen.";
		_dialog4[2] = "Ich wollte nur wissen, ob Sie die Brieftasche wiederhaben.";
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	Common::String _dialog1[2];
	Common::String _dialog2[2];
	Common::String _dialog3[4];
	Common::String _dialog4[3];
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
		_shown[0] = true;

		_objectState[0] = Object("Ufo","Der Eingang scheint offen zu sein.",UFO,EXIT,0,0,0,NULLROOM,3);
		_objectState[1] = Object("Stern",Object::defaultDescription,STAR,NULLTYPE,1,1,0);
		_objectState[2] = Object("H\224hle",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,CAVE,22);

		_dialog2[0] = "Wo bin ich hier?";
		_dialog2[1] = "Was wollen Sie von mir?";
		_dialog2[2] = "Hilfe!!";
		_dialog2[3] = "Warum sprechen Sie meine Sprache?";
		_dialog3[0] = "Ja, ich bin einverstanden.";
		_dialog3[1] = "Nein, lieber bleibe ich hier, als mit Ihnen zu fliegen.";
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	Common::String _dialog2[4];
	Common::String _dialog3[2];
};

// Axacuss
class AxacussCell : public Room {
public:
	AxacussCell(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 43;
		_id = CELL;
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = false;
		_shown[21] = false;
		_shown[22] = false;
		_shown[23] = false;
		_shown[24] = false;
		_shown[25] = false;
		_shown[26] = false;
		_shown[27] = false;
		_shown[28] = false;
		_shown[29] = false;
		_shown[30] = false;
		_shown[31] = true;

		_objectState[0] = Object("Knopf",Object::defaultDescription,CELL_BUTTON,PRESS,1,1,0);
		_objectState[1] = Object("T\201r",Object::defaultDescription,CELL_DOOR,EXIT|OPENABLE|CLOSED,0,0,31+128,CORRIDOR4,1);
		_objectState[2] = Object("Tablett","Es ist irgendein Fra\341 und|etwas zu Trinken darauf.",TRAY,UNNECESSARY,255,255,0);
		_objectState[3] = Object("Stange","Es scheint eine Lampe zu sein.",NULLOBJECT,COMBINABLE,3,3,0);
		_objectState[4] = Object("Augen","Es ist nur ein Bild.",NULLOBJECT,NULLTYPE,4,4,0);
		_objectState[5] = Object("Leitung",Object::defaultDescription,CELL_WIRE,COMBINABLE|TAKE,6,6,0);
		_objectState[6] = Object("Steckdose","Sieht etwas anders aus als auf der Erde.",SOCKET,COMBINABLE,5,5,0);
		_objectState[7] = Object("Metallblock","Er ist ziemlich schwer.",MAGNET,TAKE|COMBINABLE,255,255,30);
		_objectState[8] = Object("Roboter","Den hast du erledigt.",NULLOBJECT,NULLTYPE,255,255,0);
		_objectState[9] = Object("Tisch","Ein kleiner Metalltisch.",CELL_TABLE,COMBINABLE,2,2,0);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = true;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = false;
		_shown[21] = true;
		_shown[22] = false;
		_shown[23] = true;
		_shown[24] = false;
		_shown[25] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,2,2,0,GUARD3,2);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR2,22);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = true;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = true;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = false;
		_shown[21] = true;
		_shown[22] = false;
		_shown[23] = false;
		_shown[24] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR1,2);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR3,22);
		_objectState[2] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,1,1,0,CORRIDOR4,14);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = false;
		_shown[5] = true;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = true;
		_shown[20] = false;
		_shown[21] = false;
		_shown[22] = false;
		_shown[23] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR2,2);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = true;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = true;
		_shown[9] = true;
		_shown[10] = false;
		_shown[11] = true;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = true;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = true;
		_shown[19] = false;
		_shown[20] = true;
		_shown[21] = false;
		_shown[22] = false;
		_shown[23] = false;
		_shown[24] = false;
		_shown[25] = false;
		_shown[26] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,0,0,0,CORRIDOR2,10);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,1,1,0,GUARD,14);
		_objectState[2] = Object("Zellent\201r","Hier warst du eingesperrt.",DOOR,EXIT|OPENABLE|OPENED|CLOSED,7,7,0,CELL,16);
		_objectState[3] = Object("Laptop",Object::defaultDescription,NEWSPAPER,TAKE,6,6,8);
		_objectState[4] = Object("Armbanduhr",Object::defaultDescription,WATCH,TAKE|COMBINABLE,255,255,8);
		_objectState[5] = Object("Tisch",Object::defaultDescription,TABLE,COMBINABLE,5,5,0);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = true;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = false;
		_shown[21] = false;
		_shown[22] = true;
		_shown[23] = true;
		_shown[24] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,DOOR,EXIT,2,2,0,NULLROOM,2);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR6,22);

		_dialog1[0] = "\216h ... nein, mein Name ist M\201ller.";
		_dialog1[1] = "Oh, ich habe mich im Gang vertan.";
		_dialog2[0] = "W\201rden Sie mich bitte zum Fahrstuhl lassen?";
		_dialog2[1] = "Ich gehe wieder.";
		_dialog3[0] = "Dann gehe ich eben wieder.";
		_dialog3[1] = "Ach, halten Sie's Maul, ich gehe trotzdem!";
		_dialog3[2] = "Wenn Sie mich durchlassen gebe ich Ihnen ";
		_dialog3[3] = "Wenn Sie mich durchlassen gebe ich Ihnen ";
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	Common::String _dialog1[2];
	Common::String _dialog2[2];
	Common::String _dialog3[4];
};
class AxacussCorridor6 : public Room {
public:
	AxacussCorridor6(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 16;
		_id = CORRIDOR6;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;
		_shown[6] = true;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = false;
		_shown[21] = false;
		_shown[22] = true;
		_shown[23] = false;
		_shown[24] = true;
		_shown[25] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR5,2);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR7,22);
		_objectState[2] = Object("T\201r",Object::defaultDescription,DOOR,OPENABLE|CLOSED,255,255,0,CORRIDOR8,13);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = true;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = false;
		_shown[21] = true;
		_shown[22] = false;
		_shown[23] = false;
		_shown[24] = true;
		_shown[25] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,2,2,0,CORRIDOR6,2);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,GUARD,22);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = true;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = true;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = false;
		_shown[20] = true;
		_shown[21] = false;
		_shown[22] = true;
		_shown[23] = false;
		_shown[24] = false;
		_shown[25] = false;
		_shown[26] = false;
		_shown[27] = false;
		_shown[28] = true;

		_objectState[0] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE,0,0,0,CORRIDOR6,10);
		_objectState[1] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,BCORRIDOR,22);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = true;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = false;
		_shown[18] = false;
		_shown[19] = true;
		_shown[20] = false;
		_shown[21] = false;
		_shown[22] = false;
		_shown[23] = true;
		_shown[24] = false;
		_shown[25] = false;
		_shown[26] = false;
		_shown[27] = false;
		_shown[28] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,2,2,0,BCORRIDOR,2);
		_objectState[1] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE,0,0,0,GUARD,10);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;

		_objectState[0] = Object("S\204ule",Object::defaultDescription,PILLAR1,NULLTYPE,4,4,0);
		_objectState[1] = Object("S\204ule",Object::defaultDescription,PILLAR2,NULLTYPE,5,5,0);
		_objectState[2] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,6,6,0,CORRIDOR8,2);
		_objectState[3] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,7,7,0,CORRIDOR9,22);
		_objectState[4] = Object("T\201r","Auf einem Schild an der T\201r steht \"Dr. Alab Hansi\".",DOOR1,EXIT|OPENABLE|CLOSED|OCCUPIED,0,0,1,OFFICE_L1,6);
		_objectState[5] = Object("T\201r","Auf einem Schild an der T\201r steht \"Saval Lun\".",DOOR2,EXIT|OPENABLE|CLOSED|OCCUPIED,1,1,2,OFFICE_L2,16);
		_objectState[6] = Object("T\201r","Auf einem Schild an der T\201r steht \"Prof. Dr. Ugnul Tschabb\".",DOOR3,EXIT|OPENABLE|OPENED,2,2,3,OFFICE_R1,8);
		_objectState[7] = Object("T\201r","Auf einem Schild an der T\201r steht \"Alga Hurz Li\".",DOOR4,EXIT|OPENABLE|CLOSED|OCCUPIED,3,3,4,OFFICE_R2,18);

		_dontEnter = "Diese T\201r w\201rde ich lieber|nicht \224ffnen. Nach dem Schild zu|urteilen, ist jemand in dem Raum.";
	}

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	Common::String _dontEnter;
	byte _nameSeen;
};
class AxacussIntersection : public Room {
public:
	AxacussIntersection(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 40;
		_id = GUARD;
		_shown[0] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,CORRIDOR4,21);
		_objectState[1] = Object("Gang",Object::defaultDescription,NULLOBJECT,EXIT,3,3,0,CORRIDOR7,5);
		_objectState[2] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE,1,1,6,CORRIDOR9,3);
		// Originally GUARD was ObjectID..
		_objectState[3] = Object("Axacussaner","Du m\201\341test ihn irgendwie ablenken.",INSTRUMENTS,TALK,0,0,0);
		_objectState[4] = Object("Bild","Komisches Bild.",NULLOBJECT,NULLTYPE,2,2,0);
		_objectState[5] = Object("Karte","Darauf steht: \"Generalkarte\".",MASTERKEYCARD,TAKE|COMBINABLE,255,255,1);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussExit : public Room {
public:
	AxacussExit(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 42;
		_id = GUARD3;
		_shown[0] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,CORRIDOR1,22);
		_objectState[1] = Object("T\201r",Object::defaultDescription,NULLOBJECT,EXIT|OPENABLE|CLOSED,0,0,0,NULLROOM,20);
		_objectState[2] = Object("T\201r",Object::defaultDescription,NULLOBJECT,EXIT|OPENABLE|CLOSED,1,1,0,NULLROOM,15);
		_objectState[3] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE,2,2,11,OFFICE_L,0);
		_objectState[4] = Object("Lampe",Object::defaultDescription,LAMP,COMBINABLE,3,3,0);
		// Originally GUARD was ObjectID..
		_objectState[5] = Object("Axacussaner",Object::defaultDescription,INSTRUMENTS,TALK,5,5,0);
		_objectState[6] = Object("Bild","Seltsam!",NULLOBJECT,NULLTYPE,4,4,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};
class AxacussOffice1 : public Room {
public:
	AxacussOffice1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 7;
		_id = OFFICE_L1;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = true;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = true;
		_shown[8] = false;
		_shown[9] = true;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = true;

		_objectState[0] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,9,BCORRIDOR,9);
		_objectState[1] = Object("Computer",Object::defaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object("Geld","Es sind 500 Xa.",TICKETS,TAKE,255,255,0);
		_objectState[3] = Object("Schließfach","Es hat ein elektronisches Zahlenschlo\341.",LOCKER,OPENABLE|CLOSED,5,5,0);
		_objectState[4] = Object("Brief",Object::defaultDescription,LETTER,UNNECESSARY,3,3,0);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = true;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = true;

		_objectState[0] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,9,BCORRIDOR,9);
		_objectState[1] = Object("Computer",Object::defaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object("W\201rfel","Sonderbar!",NULLOBJECT,NULLTYPE,0,0,0);
		_objectState[3] = Object("Bild","Affenstark!",NULLOBJECT,NULLTYPE,1,1,0);
		_objectState[4] = Object("Komisches Ding","Wundersam!",NULLOBJECT,UNNECESSARY,2,2,0);
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
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = false;
		_shown[3] = true;

		_objectState[0] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE|OPENED,0,0,3,BCORRIDOR,5);
		_objectState[1] = Object("Computer",Object::defaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object("Bild","Es ist ein Axacussanerkopf auf dem Bild.",NULLOBJECT,UNNECESSARY,1,1,0);
		_objectState[3] = Object("Bild","Es ist ein Axacussanerkopf auf dem Bild.",PAINTING,UNNECESSARY,2,2,0);
		_objectState[4] = Object("Pflanze",Object::defaultDescription,NULLOBJECT,UNNECESSARY,3,3,0);
		_objectState[5] = Object("",Object::defaultDescription,MONEY,TAKE|COMBINABLE,255,255,0);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = true;
		_shown[3] = true;

		_objectState[0] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE|OPENED,0,0,3,BCORRIDOR,5);
		_objectState[1] = Object("Computer",Object::defaultDescription,COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object("Figur","Stark!",NULLOBJECT,UNNECESSARY,6,6,0);
		_objectState[3] = Object("Pflanze","Sie ist den Pflanzen auf der Erde sehr ähnlich.",NULLOBJECT,UNNECESSARY,5,5,0);
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
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = false;
		_shown[5] = true;
		_shown[6] = false;
		_shown[7] = false;
		_shown[8] = false;
		_shown[9] = false;
		_shown[10] = false;
		_shown[11] = false;
		_shown[12] = false;
		_shown[13] = false;
		_shown[14] = false;
		_shown[15] = false;
		_shown[16] = false;
		_shown[17] = true;

		_objectState[0] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE|OPENED,6,6,17,GUARD3,9);
		_objectState[1] = Object("Computer","Er funktioniert nicht.",COMPUTER,COMBINABLE,4,4,0);
		_objectState[2] = Object("Graffiti","Seltsamer B\201roschmuck!",NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[3] = Object("Geld","Es sind 350 Xa.",TICKETS,TAKE,8,8,0);
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
		_shown[0] = true;

		_objectState[0] = Object("Knopf",Object::defaultDescription,BUTTON1,PRESS,0,0,0);
		_objectState[1] = Object("Knopf",Object::defaultDescription,BUTTON2,PRESS,1,1,0);
		_objectState[2] = Object("Ausgang",Object::defaultDescription,DOOR,EXIT,255,255,0,NULLROOM,22);
		_objectState[3] = Object("Dschungel","Lauter B\204ume.",JUNGLE,NULLTYPE,255,255,0,STATION,2);
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
		_shown[0] = true;
		_objectState[0] = Object("Schild",Object::defaultDescription,STATION_SIGN,NULLTYPE,0,0,0);
		_objectState[1] = Object("T\201r",Object::defaultDescription,DOOR,EXIT|OPENABLE|CLOSED,1,1,0,NULLROOM,7);
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
		_shown[0] = true;
		_shown[1] = true;

		_objectState[0] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,STATION,22);
		_objectState[1] = Object("Schlitz",Object::defaultDescription,STATION_SLOT,COMBINABLE,0,0,0);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

}
#endif // ROOMS_H
