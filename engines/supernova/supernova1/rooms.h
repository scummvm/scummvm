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
#include "supernova/room.h"

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Supernova {

class GameManager1;
class SupernovaEngine;

class Room1: public Room {
	public:
		Room1();
	protected:
		GameManager1 *_gm;
	int _dialogsX[6];
};

// Room 0
class Intro : public Room1 {
public:
	Intro(SupernovaEngine *vm, GameManager1 *gm);
	void onEntrance() override;

private:
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
class ShipCorridor : public Room1 {
public:
	ShipCorridor(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ShipHall: public Room1 {
public:
	ShipHall(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ShipSleepCabin: public Room1 {
public:
	ShipSleepCabin(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void animation() override;
	void onEntrance() override;

private:
	byte _color;
};

class ShipCockpit : public Room1 {
public:
	ShipCockpit(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void animation() override;
	void onEntrance() override;

private:
	byte _color;
};

class ShipCabinL1: public Room1 {
public:
	ShipCabinL1(SupernovaEngine *vm, GameManager1 *gm);

private:
};

class ShipCabinL2 : public Room1 {
public:
	ShipCabinL2(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ShipCabinL3 : public Room1 {
public:
	ShipCabinL3(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ShipCabinR1 : public Room1 {
public:
	ShipCabinR1(SupernovaEngine *vm, GameManager1 *gm);

private:
};

class ShipCabinR2 : public Room1 {
public:
	ShipCabinR2(SupernovaEngine *vm, GameManager1 *gm);

private:
};

class ShipCabinR3 : public Room1 {
public:
	ShipCabinR3(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void onEntrance() override;

private:
};

class ShipCabinBathroom : public Room1  {
public:
	ShipCabinBathroom(SupernovaEngine *vm, GameManager1 *gm);

private:
};

class ShipAirlock : public Room1 {
public:
	ShipAirlock(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void onEntrance() override;

private:
};

class ShipHold : public Room1 {
public:
	ShipHold(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void onEntrance() override;

private:
};

class ShipLandingModule : public Room1 {
public:
	ShipLandingModule(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ShipGenerator : public Room1 {
public:
	ShipGenerator(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ShipOuterSpace : public Room1 {
public:
	ShipOuterSpace(SupernovaEngine *vm, GameManager1 *gm);

private:
};


// Arsano
class ArsanoRocks : public Room1 {
public:
	ArsanoRocks(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class ArsanoCave : public Room1 {
public:
	ArsanoCave(SupernovaEngine *vm, GameManager1 *gm);

private:
};

class ArsanoMeetup : public Room1 {
public:
	ArsanoMeetup(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	void animation() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
	byte _sign;
	byte _beacon;
};

class ArsanoEntrance : public Room1 {
public:
	ArsanoEntrance(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void animation() override;

private:
	int _dialog1[5];
	int _dialog2[5];
	int _dialog3[5];
	byte _eyewitness;
};

class ArsanoRemaining : public Room1 {
public:
	ArsanoRemaining(SupernovaEngine *vm, GameManager1 *gm);

	void animation() override;

private:
	bool _chewing;
	int _i;
};

class ArsanoRoger : public Room1 {
public:
	ArsanoRoger(SupernovaEngine *vm, GameManager1 *gm);

	void animation() override;
	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
	int _dialog1[4];
	byte _eyewitness;
	byte _hands;
};

class ArsanoGlider : public Room1 {
public:
	ArsanoGlider(SupernovaEngine *vm, GameManager1 *gm);

	void animation() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
	byte _sinus;
};

class ArsanoMeetup2 : public Room1 {
public:
	ArsanoMeetup2(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

	void shipStart();

private:
	// TODO: change to 6, fix initialization
	int _dialog1[2];
	int _dialog2[2];
	int _dialog3[4];
	int _dialog4[3];
};

class ArsanoMeetup3 : public Room1 {
public:
	ArsanoMeetup3(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
	int _dialog2[4];
	int _dialog3[2];
};


// Axacuss
class AxacussCell : public Room1 {
public:
	AxacussCell(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
	void animation() override;
	void onEntrance() override;

private:
};

class AxacussCorridor1 : public Room1 {
public:
	AxacussCorridor1(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;

private:
};

class AxacussCorridor2 : public Room1 {
public:
	AxacussCorridor2(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;

private:
};

class AxacussCorridor3 : public Room1 {
public:
	AxacussCorridor3(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;

private:
};

class AxacussCorridor4 : public Room1 {
public:
	AxacussCorridor4(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	void animation() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussCorridor5 : public Room1 {
public:
	AxacussCorridor5(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
	void stopInteract(int sum);
	bool handleMoneyDialog();

	// TODO: Change to 6, or change struct, and fix initialization
	int _dialog1[2];
	int _dialog2[2];
	int _dialog3[4];

	byte _rows[6];

};

class AxacussCorridor6 : public Room1 {
public:
	AxacussCorridor6(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussCorridor7 : public Room1 {
public:
	AxacussCorridor7(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;

private:
};

class AxacussCorridor8 : public Room1 {
public:
	AxacussCorridor8(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussCorridor9 : public Room1 {
public:
	AxacussCorridor9(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussBcorridor : public Room1 {
public:
	AxacussBcorridor(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussIntersection : public Room1 {
public:
	AxacussIntersection(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
};

class AxacussExit : public Room1 {
public:
	AxacussExit(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;
};

class AxacussOffice1 : public Room1 {
public:
	AxacussOffice1(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussOffice2 : public Room1 {
public:
	AxacussOffice2(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussOffice3 : public Room1 {
public:
	AxacussOffice3(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussOffice4 : public Room1 {
public:
	AxacussOffice4(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussOffice5 : public Room1 {
public:
	AxacussOffice5(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussElevator : public Room1 {
public:
	AxacussElevator(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussStation : public Room1 {
public:
	AxacussStation(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class AxacussSign : public Room1 {
public:
	AxacussSign(SupernovaEngine *vm, GameManager1 *gm);

	bool interact(Action verb, Object &obj1, Object &obj2) override;

private:
};

class Outro : public Room1 {
public:
	Outro(SupernovaEngine *vm, GameManager1 *gm);

	void onEntrance() override;
	void animation() override;

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
