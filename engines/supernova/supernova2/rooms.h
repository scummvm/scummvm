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


class Intro2 : public Room {
public:
	Intro2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();

private:
	GameManager2 *_gm;
	void titleScreen();
	bool tvSay(int mod1, int mod2, int rest, MessagePosition pos, int id);
	bool tvRest(int mod1, int mod2, int rest);
	bool displayThoughtMessage(int id);
	bool thoughts1();
	bool tvDialogue();
	bool thoughts2();

	Common::String _introText;
};

class Airport : public Room {
public:
	Airport(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();

private:
	GameManager2 *_gm;
};

class TaxiStand : public Room {
public:
	TaxiStand(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Street : public Room {
public:
	Street(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Games : public Room {
public:
	Games(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Cabin2 : public Room {
public:
	Cabin2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
	bool _paid;
};

class Kiosk : public Room {
public:
	Kiosk(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class CulturePalace : public Room {
public:
	CulturePalace(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
	void notEnoughMoney();
};

class Checkout : public Room {
public:
	Checkout(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
	void appearance();
	void shouting();
};

class City1 : public Room {
public:
	City1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class City2 : public Room {
public:
	City2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Elevator2 : public Room {
public:
	Elevator2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
	void jobDescription();
};

class Apartment : public Room {
public:
	Apartment(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Ship : public Room {
public:
	Ship(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	Common::String _outroText;
	Common::String _outroText2;

private:
	GameManager2 *_gm;
	void kill();
	void outro();
};

class Pyramid : public Room {
public:
	Pyramid(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class PyrEntrance : public Room {
public:
	PyrEntrance(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
	uint32 _waitTime;
};

class Upstairs1 : public Room {
public:
	Upstairs1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Downstairs1 : public Room {
public:
	Downstairs1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class BottomRightDoor : public Room {
public:
	BottomRightDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class BottomLeftDoor : public Room {
public:
	BottomLeftDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Upstairs2 : public Room {
public:
	Upstairs2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Downstairs2 : public Room {
public:
	Downstairs2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class UpperDoor : public Room {
public:
	UpperDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class PuzzleFront : public Room {
public:
	PuzzleFront(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class PuzzleBehind : public Room {
public:
	PuzzleBehind(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Formula1F : public Room {
public:
	Formula1F(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Formula1N : public Room {
public:
	Formula1N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Formula2F : public Room {
public:
	Formula2F(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Formula2N : public Room {
public:
	Formula2N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class TomatoF : public Room {
public:
	TomatoF(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class TomatoN : public Room {
public:
	TomatoN(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class MonsterF : public Room {
public:
	MonsterF(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Monster1N : public Room {
public:
	Monster1N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Monster2N : public Room {
public:
	Monster2N(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Upstairs3 : public Room {
public:
	Upstairs3(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Downstairs3 : public Room {
public:
	Downstairs3(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class LCorridor1 : public Room {
public:
	LCorridor1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class LCorridor2 : public Room {
public:
	LCorridor2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class HoleRoom : public Room {
public:
	HoleRoom(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class InHole : public Room {
public:
	InHole(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Floordoor : public Room {
public:
	Floordoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class FloordoorU : public Room {
public:
	FloordoorU(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class BstDoor : public Room {
public:
	BstDoor(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
	char _password[16];
};

class Hall2 : public Room {
public:
	Hall2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class CoffinRoom : public Room {
public:
	CoffinRoom(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mask : public Room {
public:
	Mask(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Museum : public Room {
public:
	Museum(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class MusEntrance : public Room {
public:
	MusEntrance(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus1 : public Room {
public:
	Mus1(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus2 : public Room {
public:
	Mus2(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus3 : public Room {
public:
	Mus3(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus4 : public Room {
public:
	Mus4(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus5 : public Room {
public:
	Mus5(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus6 : public Room {
public:
	Mus6(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus7 : public Room {
public:
	Mus7(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus8 : public Room {
public:
	Mus8(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus9 : public Room {
public:
	Mus9(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus10 : public Room {
public:
	Mus10(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus11 : public Room {
public:
	Mus11(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class MusRound : public Room {
public:
	MusRound(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus12 : public Room {
public:
	Mus12(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus13 : public Room {
public:
	Mus13(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus14 : public Room {
public:
	Mus14(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus15 : public Room {
public:
	Mus15(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus16 : public Room {
public:
	Mus16(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus17 : public Room {
public:
	Mus17(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus18 : public Room {
public:
	Mus18(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus19 : public Room {
public:
	Mus19(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus20 : public Room {
public:
	Mus20(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus21 : public Room {
public:
	Mus21(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

class Mus22 : public Room {
public:
	Mus22(SupernovaEngine *vm, GameManager2 *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager2 *_gm;
};

}
#endif // SUPERNOVA2_ROOMS_H
