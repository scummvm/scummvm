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

#ifndef SUPERNOVA2_ROOMS_H
#define SUPERNOVA2_ROOMS_H

#include "common/str.h"

#include "supernova/msn_def.h"
#include "supernova/room.h"

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Supernova {

class GameManager2;
class SupernovaEngine;

struct RoomEntry {
	int _e;
	int _s;
	int _z;
	int _r;
	RoomId _exitRoom;
};

class Room2 : public Room {
	protected:
		GameManager2 *_gm;
};

class Intro2 : public Room2 {
public:
	Intro2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();

private:
	void titleScreen();
	bool tvSay(int mod1, int mod2, int rest, MessagePosition pos, int id);
	bool tvRest(int mod1, int mod2, int rest);
	bool displayThoughtMessage(int id);
	bool thoughts1();
	bool tvDialogue();
	bool thoughts2();

	Common::String _introText;
};

class Airport : public Room2 {
public:
	Airport(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();

};

class TaxiStand : public Room2 {
public:
	TaxiStand(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Street : public Room2 {
public:
	Street(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Games : public Room2 {
public:
	Games(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Cabin2 : public Room2 {
public:
	Cabin2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class Kiosk : public Room2 {
public:
	Kiosk(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class CulturePalace : public Room2 {
public:
	CulturePalace(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void notEnoughMoney();
};

class Checkout : public Room2 {
public:
	Checkout(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void appearance();
	void shouting();
};

class City1 : public Room2 {
public:
	City1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class City2 : public Room2 {
public:
	City2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Elevator2 : public Room2 {
public:
	Elevator2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void jobDescription();
};

class Apartment : public Room2 {
public:
	Apartment(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Ship : public Room2 {
public:
	Ship(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	Common::String _outroText;
	Common::String _outroText2;

private:
	void kill();
	void outro();
};

class Pyramid : public Room2 {
public:
	Pyramid(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class PyrEntrance : public Room2 {
public:
	PyrEntrance(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	uint32 _waitTime;
};

class Upstairs1 : public Room2 {
public:
	Upstairs1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Downstairs1 : public Room2 {
public:
	Downstairs1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class BottomRightDoor : public Room2 {
public:
	BottomRightDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class BottomLeftDoor : public Room2 {
public:
	BottomLeftDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Upstairs2 : public Room2 {
public:
	Upstairs2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Downstairs2 : public Room2 {
public:
	Downstairs2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class UpperDoor : public Room2 {
public:
	UpperDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class PuzzleFront : public Room2 {
public:
	PuzzleFront(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class PuzzleBehind : public Room2 {
public:
	PuzzleBehind(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Formula1F : public Room2 {
public:
	Formula1F(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Formula1N : public Room2 {
public:
	Formula1N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Formula2F : public Room2 {
public:
	Formula2F(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Formula2N : public Room2 {
public:
	Formula2N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class TomatoF : public Room2 {
public:
	TomatoF(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class TomatoN : public Room2 {
public:
	TomatoN(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class MonsterF : public Room2 {
public:
	MonsterF(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Monster1N : public Room2 {
public:
	Monster1N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Monster2N : public Room2 {
public:
	Monster2N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Upstairs3 : public Room2 {
public:
	Upstairs3(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Downstairs3 : public Room2 {
public:
	Downstairs3(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class LCorridor1 : public Room2 {
public:
	LCorridor1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class LCorridor2 : public Room2 {
public:
	LCorridor2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class HoleRoom : public Room2 {
public:
	HoleRoom(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class InHole : public Room2 {
public:
	InHole(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Floordoor : public Room2 {
public:
	Floordoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class FloordoorU : public Room2 {
public:
	FloordoorU(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class BstDoor : public Room2 {
public:
	BstDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	bool _password[16];
};

class Hall2 : public Room2 {
public:
	Hall2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class CoffinRoom : public Room2 {
public:
	CoffinRoom(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mask : public Room2 {
public:
	Mask(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Museum : public Room2 {
public:
	Museum(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class MusEntrance : public Room2 {
public:
	MusEntrance(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus1 : public Room2 {
public:
	Mus1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus2 : public Room2 {
public:
	Mus2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus3 : public Room2 {
public:
	Mus3(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus4 : public Room2 {
public:
	Mus4(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus5 : public Room2 {
public:
	Mus5(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus6 : public Room2 {
public:
	Mus6(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus7 : public Room2 {
public:
	Mus7(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus8 : public Room2 {
public:
	Mus8(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus9 : public Room2 {
public:
	Mus9(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus10 : public Room2 {
public:
	Mus10(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus11 : public Room2 {
public:
	Mus11(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class MusRound : public Room2 {
public:
	MusRound(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus12 : public Room2 {
public:
	Mus12(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus13 : public Room2 {
public:
	Mus13(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus14 : public Room2 {
public:
	Mus14(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus15 : public Room2 {
public:
	Mus15(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus16 : public Room2 {
public:
	Mus16(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus17 : public Room2 {
public:
	Mus17(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus18 : public Room2 {
public:
	Mus18(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus19 : public Room2 {
public:
	Mus19(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus20 : public Room2 {
public:
	Mus20(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus21 : public Room2 {
public:
	Mus21(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

class Mus22 : public Room2 {
public:
	Mus22(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

};

}
#endif // SUPERNOVA2_ROOMS_H
