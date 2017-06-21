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

	void setSectionVisible(size_t section, bool visible) {
		_shown[section] = visible;
	}
	bool isSectionVisible(size_t index) const {
		return _shown[index];
	}
	Object *getObject(size_t index) {
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
	SupernovaEngine *_vm;
	GameManager *_gm;

private:
	bool _seen;
};

// Room 0
class StartingItems : public Room {
public:
	StartingItems(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = -1;

		_shown[0] = false;

		_objectState[0] =
		    Object("Keycard", "Die Keycard fuer deine Schraenke.", KEYCARD,
		           TAKE | CARRIED | COMBINABLE, 255, 255, 0, NULLROOM, 0);
		_objectState[1] =
		    Object("Taschenmesser", "Es ist nicht mehr das sch�rfste.", KNIFE,
		           TAKE | CARRIED | COMBINABLE, 255, 255, 0, NULLROOM, 0);
		_objectState[2] =
		    Object("Armbanduhr", Object::defaultDescription, WATCH,
		           TAKE | COMBINABLE | CARRIED, 255, 255, 8, NULLROOM, 0);
		_objectState[3] =
		    Object("Discman", "Es ist eine \"Mad Monkeys\"-CD darin.", DISCMAN,
		           TAKE | COMBINABLE, 255, 255, 0, NULLROOM, 0);
	}
};

// Spaceship
class ShipCorridor : public Room {
public:
	ShipCorridor(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 17;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = true;

		_objectState[0] = Object("Luke", Object::defaultDescription, HATCH1, OPEN | EXIT, 0, 6, 1, CABIN_L1, 15);
		_objectState[1] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | EXIT, 1, 7, 2, CABIN_L2, 10);
		_objectState[2] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | EXIT, 2, 8, 3, CABIN_L3, 5);
		_objectState[3] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | EXIT, 5, 11, 6, CABIN_R1, 19);
		_objectState[4] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | EXIT, 4, 10, 5, CABIN_R2, 14);
		_objectState[5] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | EXIT | OPENED, 9, 3, 4, CABIN_R3, 9);
		_objectState[6] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | CLOSED | EXIT, 12, 12, 0, AIRLOCK, 2);
		_objectState[7] = Object("Knopf", "Er geh�rt zu der gro�en Luke.", BUTTON, PRESS, 13, 13, 0, NULLROOM, 0);
		_objectState[8] = Object("Leiter", Object::defaultDescription, NULLOBJECT, NULLTYPE, 14, 14, 0, NULLROOM, 0);
		_objectState[9] = Object("Ausgang", Object::defaultDescription, NULLOBJECT, EXIT, 15, 15, 0, HALL, 2);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipHall: public Room {
public:
	ShipHall(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 15;
		_shown[0] = true;

		_objectState[0] = Object("Luke", "Sie f�hrt ins Cockpit.", NULLOBJECT, OPEN | EXIT, 4, 5, 1, COCKPIT, 10);
		_objectState[1] = Object("Luke", "Sie f�hrt zur K�che.", KITCHEN_HATCH, OPEN | EXIT, 0, 0, 0, NULLROOM, 1);
		_objectState[2] = Object("Luke", "Sie f�hrt zu den Tiefschlafkammern.", NULLOBJECT, OPEN | CLOSED | EXIT, 1, 1, 2, SLEEP, 8);
		_objectState[3] = Object("Schlitz", "Es ist ein Keycard-Leser.", SLEEP_SLOT, COMBINABLE, 2, 2, 0, NULLROOM, 0);
		_objectState[4] = Object("Leiter", Object::defaultDescription, NULLOBJECT, NULLTYPE, 3, SLEEP, 0, NULLROOM, 0);
		_objectState[5] = Object("Gang", Object::defaultDescription, NULLOBJECT, EXIT, 6, 6, 0, CORRIDOR, 1);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipSleepCabin: public Room {
public:
	ShipSleepCabin(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 33;
		_shown[0] = true;

		_objectState[0] = Object("Luke", "Dies ist eine der Tiefschlafkammern.", CABINS, NULLTYPE, 0, 0, 0, NULLROOM, 0);
		_objectState[1] = Object("Luke", "Dies ist eine der Tiefschlafkammern.", CABIN, NULLTYPE, 1, 1, 0, NULLROOM, 0);
		_objectState[2] = Object("Computer", Object::defaultDescription, COMPUTER, NULLTYPE, 2, 2, 0, NULLROOM, 0);
		_objectState[3] = Object("Ausgang", Object::defaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 2);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();
};

class ShipCockpit : public Room {
public:
	ShipCockpit(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 9;
		_shown[0] = true;

		_objectState[0] = Object("Instrumente", "Hmm, sieht ziemlich kompliziert aus.", INSTRUMENTS, NULLTYPE, 2, 2, 0, NULLROOM, 0);
		_objectState[1] = Object("Monitor", Object::defaultDescription, MONITOR, NULLTYPE, 0, 0, 0, NULLROOM, 0);
		_objectState[2] = Object("Monitor", "Dieser Monitor sagt dir nichts.", NULLOBJECT, TAKE, 1, 0, 0, NULLROOM, 0);
		_objectState[3] = Object("Ausgang", Object::defaultDescription, NULLOBJECT, EXIT, 255, 255, 0, HALL, 2);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();
};

class ShipCabinL1: public Room {
public:
	ShipCabinL1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 21;
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
		_objectState[6] = Object("Luke", Object::defaultDescription, NULLOBJECT, OPEN | OPENED | EXIT, 3, 3, 24 | 128, CORRIDOR, 9);
		_objectState[7] = Object("Schlitz", "Es ist ein Keycard-Leser.", NULLOBJECT, COMBINABLE, 0, 0, 0);
		_objectState[8] = Object("Schrank", Object::defaultDescription, NULLOBJECT, OPEN | CLOSED, 1, 1, 0);
		_objectState[9] = Object("Fach", Object::defaultDescription, NULLOBJECT, OPEN | CLOSED, 2, 2, 0);
		_objectState[10] = Object("Steckdose", Object::defaultDescription, SOCKET, COMBINABLE, 4, 4, 0);
		_objectState[11] = Object("Toilette", Object::defaultDescription, BATHROOM_DOOR, EXIT, 255, 255, 0, BATHROOM, 2);
	}
};

class ShipCabinL2 : public Room {
public:
	ShipCabinL2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 21;
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
		_objectState[4] = Object("Schrank",Object::defaultDescription,SHELF_L1,OPEN | CLOSED,25,26,17);
		_objectState[5] = Object("Pistole","Es ist keine Munition drin.",PISTOL,TAKE,39,39,20);
		_objectState[6] = Object("Fach",Object::defaultDescription,SHELF_L2,OPEN | CLOSED,27,28,18);
		_objectState[7] = Object("B�cher","Lauter wissenschaftliche B�cher.",NULLOBJECT,UNNECESSARY,40,40,0);
		_objectState[8] = Object("Fach",Object::defaultDescription,SHELF_L3,OPEN | CLOSED,29,30,19);
		_objectState[9] = Object("Kabelrolle","Da sind mindestens zwanzig Meter drauf.", SPOOL,TAKE | COMBINABLE,41,41,21);
		_objectState[10] = Object("Fach",Object::defaultDescription,SHELF_L4,OPEN | CLOSED,43,44,22);
		_objectState[11] = Object("Buch",Object::defaultDescription,BOOK2,TAKE,46,46,23);
		_objectState[12] = Object("Unterw�sche","Ich habe keine Lust, in|der Unterw�sche des|Commanders rumzuw�hlen.",NULLOBJECT,UNNECESSARY,34,34,0);
		_objectState[13] = Object("Unterw�sche","Ich habe keine Lust, in|der Unterw�sche des|Commanders rumzuw�hlen.",NULLOBJECT,UNNECESSARY,35,35,0);
		_objectState[14] = Object("Kleider",Object::defaultDescription,NULLOBJECT,UNNECESSARY,36,36,0);
		_objectState[15] = Object("Krimskram","Es ist nichts brauchbares dabei.",NULLOBJECT,UNNECESSARY,37,37,0);
		_objectState[16] = Object("Krimskram","Es ist nichts brauchbares dabei.",NULLOBJECT,UNNECESSARY,38,38,0);
		_objectState[17] = Object("Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",NULLOBJECT,UNNECESSARY,23,23,0);
		_objectState[18] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,22);
		_objectState[19] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPEN | OPENED | EXIT,3,3,24 | 128,CORRIDOR,9);
		_objectState[20] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[21] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,1,1,0);
		_objectState[22] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,2,2,0);
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
		_objectState[5] = Object("Schallplattenst�nder","Du hast jetzt keine Zeit, in|der Plattensammlung rumzust�bern.",NULLOBJECT,UNNECESSARY,16,16,0);
		_objectState[6] = Object("Knopf",Object::defaultDescription,TURNTABLE_BUTTON,PRESS,22,22,0);
		_objectState[7] = Object("Plattenspieler","Sieht aus, als k�me|er aus dem Museum.",TURNTABLE,UNNECESSARY | COMBINABLE,17,17,0);
		_objectState[8] = Object("Leitung",Object::defaultDescription,WIRE,COMBINABLE,18,18,0);
		_objectState[9] = Object("Leitung",Object::defaultDescription,WIRE2,COMBINABLE,19,19,0);
		_objectState[10] = Object("Stecker",Object::defaultDescription,PLUG,COMBINABLE,20,20,0);
		_objectState[11] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPEN | OPENED | EXIT,3,3,24 | 128,CORRIDOR,9);
		_objectState[12] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[13] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,1,1,0);
		_objectState[14] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,2,2,0);
		_objectState[15] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[16] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,2);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipCabinR1 : public Room {
public:
	ShipCabinR1(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_shown[0] = true;
		_shown[1] = true;
		_shown[2] = true;

		_objectState[0] = Object("Bild","Manche Leute haben schon|einen komischen Geschmack.",NULLOBJECT,UNNECESSARY,5,5,0);
		_objectState[1] = Object("Zeichenger�te","Auf dem Zettel sind lauter|unverst�ndliche Skizzen und Berechnungen.|(Jedenfalls f�r dich unverst�ndlich.)",NULLOBJECT,UNNECESSARY,6,6,0);
		_objectState[2] = Object("Magnete","Damit werden Sachen auf|dem Tisch festgehalten.",NULLOBJECT,UNNECESSARY,7,7,0);
		_objectState[3] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPEN | OPENED | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[4] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[5] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,1,1,0);
		_objectState[6] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,2,2,0);
		_objectState[7] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[8] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,2);
	}
};

class ShipCabinR2 : public Room {
public:
	ShipCabinR2(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = true;
		_shown[4] = true;
		_shown[5] = true;

		_objectState[0] = Object("Schachspiel","Es macht wohl Spa�, an|der Decke Schach zu spielen.",NULLOBJECT,UNNECESSARY,11,11,0);
		_objectState[1] = Object("Tennisschl�ger","Fliegt Boris Becker auch mit?",NULLOBJECT,UNNECESSARY,8,8,0);
		_objectState[2] = Object("Tennisball","Toll!",NULLOBJECT,UNNECESSARY,9,9,0);
		_objectState[3] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPEN | OPENED | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[4] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[5] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,1,1,0);
		_objectState[6] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,2,2,0);
		_objectState[7] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[8] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,2);
	}
};

class ShipCabinR3 : public Room {
public:
	ShipCabinR3(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 22;
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
		_objectState[6] = Object("Fach","Das ist eins deiner drei F�cher.",SHELF1,OPEN | CLOSED,14,18,9);
		_objectState[7] = Object("Alben","Deine Briefmarkensammlung.",NULLOBJECT,UNNECESSARY,14,14,0);
		_objectState[8] = Object("Fach","Das ist eins deiner drei F�cher.",SHELF2,OPEN | CLOSED,15,19,10);
		_objectState[9] = Object("Seil","Es ist ungef�hr 10 m lang und 4 cm dick.",ROPE,TAKE | COMBINABLE,15,15,12);
		_objectState[10] = Object("Schrank","Das ist dein Schrank.",SHELF3,OPEN | CLOSED,16,17,11);
		_objectState[11] = Object("Krimskram","Es ist nichts brauchbares dabei.",NULLOBJECT,UNNECESSARY,20,20,0);
		_objectState[12] = Object("Kleider","Es sind Standard-Weltraum-Klamotten.",NULLOBJECT,UNNECESSARY,21,21,0);
		_objectState[13] = Object("Unterw�sche",Object::defaultDescription,NULLOBJECT,UNNECESSARY,22,22,0);
		_objectState[14] = Object("Str�mpfe",Object::defaultDescription,NULLOBJECT,UNNECESSARY,23,23,0);
		_objectState[15] = Object("Fach","Das ist eins deiner drei F�cher.",SHELF4,OPEN | CLOSED,24,25,13);
		_objectState[16] = Object("Buch","Es ist|\"Per Anhalter durch die Galaxis\"|von Douglas Adams.",BOOK,TAKE,26,26,14);
		_objectState[17] = Object("Discman","Es ist eine \"Mad Monkeys\"-CD darin.",DISCMAN,TAKE | COMBINABLE,33,33,16);
		_objectState[18] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPEN | EXIT,3,3,15 | 128,CORRIDOR,5);
		_objectState[19] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,0,0,0);
		_objectState[20] = Object("Schrank",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,1,1,0);
		_objectState[21] = Object("Fach",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,2,2,0);
		_objectState[22] = Object("Steckdose",Object::defaultDescription,SOCKET,COMBINABLE,4,4,0);
		_objectState[23] = Object("Toilette",Object::defaultDescription,BATHROOM_DOOR,EXIT,255,255,0,BATHROOM,2);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipCabinBathroom : public Room  {
public:
	ShipCabinBathroom(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 23;
		_shown[0] = true;

		_objectState[0] = Object("Klo","Ein Klo mit Saugmechanismus.",TOILET,NULLTYPE,0,0,0);
		_objectState[1] = Object("Dusche",Object::defaultDescription,SHOWER,NULLTYPE,1,1,0);
		_objectState[2] = Object("Ausgang",Object::defaultDescription,BATHROOM_EXIT,EXIT,255,255,0,NULLROOM,2);
	}
};

class ShipAirlock : public Room {
public:
	ShipAirlock(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 34;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = false;
		_shown[6] = true;

		_objectState[0] = Object("Luke","Das ist eine Luke !!!",NULLOBJECT,EXIT | OPEN | OPENED | CLOSED,0,0,0,CORRIDOR,10);
		_objectState[1] = Object("Luke","Dies ist eine Luke !!!",NULLOBJECT,EXIT | OPEN | CLOSED,1,1,0,HOLD,14);
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
		_shown[0] = true;

		_objectState[0] = Object("",Object::defaultDescription,HOLD_WIRE,COMBINABLE,255,255,0);
		_objectState[1] = Object("Schrott","Da ist eine L�sterklemme dran, die|noch ganz brauchbar aussieht.|Ich nehme sie mit.",SCRAP_LK,NULLTYPE,4,4,0);
		_objectState[2] = Object("L�sterklemme",Object::defaultDescription,TERMINALSTRIP,COMBINABLE,255,255,0);
		_objectState[3] = Object("Schrott","Junge, Junge! Die Explosion hat ein|ganz sch�nes Durcheinander angerichtet.",NULLOBJECT,NULLTYPE,5,5,0);
		_objectState[4] = Object("Reaktor","Das war einmal der Reaktor.",NULLOBJECT,NULLTYPE,6,6,0);
		_objectState[5] = Object("D�se",Object::defaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[6] = Object("blauer K�rbis","Keine Ahnung, was das ist.",NULLOBJECT,NULLTYPE,8,8,0);
		_objectState[7] = Object("Luke",Object::defaultDescription,LANDINGMOD_OUTERHATCH,EXIT | OPEN,1,2,2,LANDINGMODULE,6);
		_objectState[8] = Object("Landef�hre","Sie war eigentlich f�r Bodenuntersuchungen|auf Arsano 3 gedacht.",NULLOBJECT,NULLTYPE,0,0,0);
		_objectState[9] = Object("Ausgang",Object::defaultDescription,NULLOBJECT,EXIT,255,255,0,AIRLOCK,22);
		_objectState[10] = Object("Luke","Sie f�hrt nach drau�en.",OUTERHATCH_TOP,EXIT | OPEN | OPENED,3,3,0,GENERATOR,8);
		_objectState[11] = Object("Generator","Er versorgt das Raumschiff mit Strom.",GENERATOR_TOP,EXIT,12,12,0,GENERATOR,8);
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
		_shown[0] = true;

		_objectState[0] = Object("Steckdose",Object::defaultDescription,LANDINGMOD_SOCKET,COMBINABLE,1,1,0);
		_objectState[1] = Object("Knopf","Es ist ein Sicherheitsknopf.|Er kann nur mit einem spitzen|Gegenstand gedr�ckt werden.",LANDINGMOD_BUTTON,PRESS | COMBINABLE,2,2,0);
		_objectState[2] = Object("Monitor",Object::defaultDescription,LANDINGMOD_MONITOR,NULLTYPE,3,3,0);
		_objectState[3] = Object("Tastatur",Object::defaultDescription,KEYBOARD,NULLTYPE,4,4,0);
		_objectState[4] = Object("",Object::defaultDescription,LANDINGMOD_WIRE,COMBINABLE,255,255,0);
		_objectState[5] = Object("Luke",Object::defaultDescription,LANDINGMOD_HATCH,EXIT | OPEN | OPENED | COMBINABLE, 0,0,1 | 128,HOLD,10);
	}

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipGenerator : public Room {
public:
	ShipGenerator(SupernovaEngine *vm, GameManager *gm) {
		_vm = vm;
		_gm = gm;

		_fileNumber = 18;
		_shown[0] = true;
		_shown[1] = false;
		_shown[2] = false;
		_shown[3] = false;
		_shown[4] = false;
		_shown[5] = true;

		_objectState[0] = Object("langes Kabel mit Stecker",Object::defaultDescription,GENERATOR_WIRE,COMBINABLE,255,255,0);
		_objectState[1] = Object("leere Kabelrolle",Object::defaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
		_objectState[2] = Object("Keycard","Hey, das ist die Keycard des Commanders!|Er mu� sie bei dem �berst�rzten|Aufbruch verloren haben.",KEYCARD2,COMBINABLE | TAKE,12,12,5 | 128);
		_objectState[3] = Object("Seil",Object::defaultDescription,GENERATOR_ROPE,COMBINABLE,255,255,0);
		_objectState[4] = Object("Luke","Sie f�hrt nach drau�en.",OUTERHATCH,EXIT | OPEN,1,2,1,OUTSIDE,22);
		_objectState[5] = Object("Luke",Object::defaultDescription,NULLOBJECT,OPEN | CLOSED,3,3,0);
		_objectState[6] = Object("Schlitz","Es ist ein Keycard-Leser.",NULLOBJECT,COMBINABLE,4,4,0);
		_objectState[7] = Object("Klappe",Object::defaultDescription,TRAP,OPEN,5,6,2);
		_objectState[8] = Object("Leitung",Object::defaultDescription,NULLOBJECT,NULLTYPE,7,7,0);
		_objectState[9] = Object("Spannungmessger�t",Object::defaultDescription,VOLTMETER,NULLTYPE,9,9,0,NULLROOM,0);
		_objectState[10] = Object("Klemme",Object::defaultDescription,CLIP,COMBINABLE,8,8,0);
		_objectState[11] = Object("Leitung","Sie f�hrt vom Generator zum Spannungmessger�t.",SHORT_WIRE,COMBINABLE,10,10,0);
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
		_shown[0] = true;

		_objectState[0] = Object("Luke",Object::defaultDescription,NULLOBJECT,EXIT,0,0,0,GENERATOR,3);
		_objectState[1] = Object("Seil",Object::defaultDescription,NULLOBJECT,UNNECESSARY,255,255,0);
	}
};

// Arsano

// Axacuss


}
#endif // ROOMS_H
