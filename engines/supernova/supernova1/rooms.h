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

#include "common/str.h"

#include "supernova/msn_def.h"
#include "supernova/supernova1/stringid.h"
#include "supernova/room.h"

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Supernova {

class GameManager1;
class SupernovaEngine;

// Room 0
class Intro : public Room {
public:
	Intro(SupernovaEngine *vm, GameManager1 *gm);
	virtual void onEntrance();

private:
	GameManager1 *_gm;
	bool animate(int section1, int section2, int duration);
	bool animate(int section1, int section2, int duration, MessagePosition position,
				 int text);
	bool animate(int section1, int section2, int section3, int section4, int duration,
				 MessagePosition position, int text);

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
	ShipCorridor(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ShipHall: public Room {
public:
	ShipHall(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ShipSleepCabin: public Room {
public:
	ShipSleepCabin(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	GameManager1 *_gm;
	byte _color;
};

class ShipCockpit : public Room {
public:
	ShipCockpit(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	GameManager1 *_gm;
	byte _color;
};

class ShipCabinL1: public Room {
public:
	ShipCabinL1(SupernovaEngine *vm, GameManager1 *gm);

private:
	GameManager1 *_gm;
};

class ShipCabinL2 : public Room {
public:
	ShipCabinL2(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ShipCabinL3 : public Room {
public:
	ShipCabinL3(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ShipCabinR1 : public Room {
public:
	ShipCabinR1(SupernovaEngine *vm, GameManager1 *gm);

private:
	GameManager1 *_gm;
};

class ShipCabinR2 : public Room {
public:
	ShipCabinR2(SupernovaEngine *vm, GameManager1 *gm);

private:
	GameManager1 *_gm;
};

class ShipCabinR3 : public Room {
public:
	ShipCabinR3(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class ShipCabinBathroom : public Room  {
public:
	ShipCabinBathroom(SupernovaEngine *vm, GameManager1 *gm);

private:
	GameManager1 *_gm;
};

class ShipAirlock : public Room {
public:
	ShipAirlock(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class ShipHold : public Room {
public:
	ShipHold(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class ShipLandingModule : public Room {
public:
	ShipLandingModule(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ShipGenerator : public Room {
public:
	ShipGenerator(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ShipOuterSpace : public Room {
public:
	ShipOuterSpace(SupernovaEngine *vm, GameManager1 *gm);

private:
	GameManager1 *_gm;
};


// Arsano
class ArsanoRocks : public Room {
public:
	ArsanoRocks(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class ArsanoCave : public Room {
public:
	ArsanoCave(SupernovaEngine *vm, GameManager1 *gm);

private:
	GameManager1 *_gm;
};

class ArsanoMeetup : public Room {
public:
	ArsanoMeetup(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	byte _sign;
	byte _beacon;
	GameManager1 *_gm;
};

class ArsanoEntrance : public Room {
public:
	ArsanoEntrance(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();

private:
	int _dialog1[5];
	int _dialog2[5];
	int _dialog3[5];
	byte _eyewitness;
	GameManager1 *_gm;
};

class ArsanoRemaining : public Room {
public:
	ArsanoRemaining(SupernovaEngine *vm, GameManager1 *gm);

	virtual void animation();

private:
	bool _chewing;
	int _i;
	GameManager1 *_gm;
};

class ArsanoRoger : public Room {
public:
	ArsanoRoger(SupernovaEngine *vm, GameManager1 *gm);

	virtual void animation();
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	int _dialog1[4];
	byte _eyewitness;
	byte _hands;
	GameManager1 *_gm;
};

class ArsanoGlider : public Room {
public:
	ArsanoGlider(SupernovaEngine *vm, GameManager1 *gm);

	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	byte _sinus;
	GameManager1 *_gm;
};

class ArsanoMeetup2 : public Room {
public:
	ArsanoMeetup2(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

	void shipStart();

private:
	// TODO: change to 6, fix initialization
	int _dialog1[2];
	int _dialog2[2];
	int _dialog3[4];
	int _dialog4[3];
	GameManager1 *_gm;

	// FIXME: Remove following unused bool variables?
	//bool _found;
	//bool _flug;
};

class ArsanoMeetup3 : public Room {
public:
	ArsanoMeetup3(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	int _dialog2[4];
	int _dialog3[2];

	// TODO: Hack, to be move away and renamed when the other uses are found
	int _dialogsX[6];
	GameManager1 *_gm;
};


// Axacuss
class AxacussCell : public Room {
public:
	AxacussCell(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class AxacussCorridor1 : public Room {
public:
	AxacussCorridor1(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class AxacussCorridor2 : public Room {
public:
	AxacussCorridor2(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class AxacussCorridor3 : public Room {
public:
	AxacussCorridor3(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class AxacussCorridor4 : public Room {
public:
	AxacussCorridor4(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussCorridor5 : public Room {
public:
	AxacussCorridor5(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void stopInteract(int sum);
	bool handleMoneyDialog();

	// TODO: Change to 6, or change struct, and fix initialization
	int _dialog1[2];
	int _dialog2[2];
	int _dialog3[4];

	byte _rows[6];

	GameManager1 *_gm;
};

class AxacussCorridor6 : public Room {
public:
	AxacussCorridor6(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussCorridor7 : public Room {
public:
	AxacussCorridor7(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();

private:
	GameManager1 *_gm;
};

class AxacussCorridor8 : public Room {
public:
	AxacussCorridor8(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussCorridor9 : public Room {
public:
	AxacussCorridor9(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussBcorridor : public Room {
public:
	AxacussBcorridor(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussIntersection : public Room {
public:
	AxacussIntersection(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	int _dialogsX[6];
	GameManager1 *_gm;
};

class AxacussExit : public Room {
public:
	AxacussExit(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	int _dialogsX[6];
	GameManager1 *_gm;
};

class AxacussOffice1 : public Room {
public:
	AxacussOffice1(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussOffice2 : public Room {
public:
	AxacussOffice2(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussOffice3 : public Room {
public:
	AxacussOffice3(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussOffice4 : public Room {
public:
	AxacussOffice4(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussOffice5 : public Room {
public:
	AxacussOffice5(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussElevator : public Room {
public:
	AxacussElevator(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussStation : public Room {
public:
	AxacussStation(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class AxacussSign : public Room {
public:
	AxacussSign(SupernovaEngine *vm, GameManager1 *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	GameManager1 *_gm;
};

class Outro : public Room {
public:
	Outro(SupernovaEngine *vm, GameManager1 *gm);

	virtual void onEntrance();
	virtual void animation();

private:
	void animate(int filenumber, int section1, int section2, int duration);
	void animate(int filenumber, int section1, int section2, int duration, MessagePosition position,
				 const char *text);
	void animate(int filenumber, int section1, int section2, int section3, int section4, int duration,
				 MessagePosition position, const char *text);

	Common::String _outroText;
	GameManager1 *_gm;
};

}
#endif // SUPERNOVA_ROOMS_H
