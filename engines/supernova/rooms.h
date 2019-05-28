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

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Supernova {

class GameManager;
class SupernovaEngine;

class Room {
public:
	Room();

	bool hasSeen();
	void setRoomSeen(bool seen);
	int getFileNumber() const;
	RoomId getId() const;
	void setSectionVisible(uint section, bool visible);
	bool isSectionVisible(uint index) const;
	void removeSentence(int sentence, int number);
	void addSentence(int sentence, int number);
	void addAllSentences(int number);
	bool sentenceRemoved(int sentence, int number);
	bool allSentencesRemoved(int maxSentence, int number);
	Object *getObject(uint index);

	virtual ~Room();
	virtual void animation();
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual bool serialize(Common::WriteStream *out);
	virtual bool deserialize(Common::ReadStream *in, int version);

protected:
	int _fileNumber;
	bool _shown[kMaxSection];
	byte _sentenceRemoved[kMaxDialog];
	Object _objectState[kMaxObject];
	RoomId _id;
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
				 StringId text);
	bool animate(int section1, int section2, int section3, int section4, int duration,
				 MessagePosition position, StringId text);

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
	ShipCorridor(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipHall: public Room {
public:
	ShipHall(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipSleepCabin: public Room {
public:
	ShipSleepCabin(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	byte _color;
};

class ShipCockpit : public Room {
public:
	ShipCockpit(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();

private:
	byte _color;
};

class ShipCabinL1: public Room {
public:
	ShipCabinL1(SupernovaEngine *vm, GameManager *gm);
};

class ShipCabinL2 : public Room {
public:
	ShipCabinL2(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipCabinL3 : public Room {
public:
	ShipCabinL3(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipCabinR1 : public Room {
public:
	ShipCabinR1(SupernovaEngine *vm, GameManager *gm);
};

class ShipCabinR2 : public Room {
public:
	ShipCabinR2(SupernovaEngine *vm, GameManager *gm);
};

class ShipCabinR3 : public Room {
public:
	ShipCabinR3(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();
};

class ShipCabinBathroom : public Room  {
public:
	ShipCabinBathroom(SupernovaEngine *vm, GameManager *gm);
};

class ShipAirlock : public Room {
public:
	ShipAirlock(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();
};

class ShipHold : public Room {
public:
	ShipHold(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void onEntrance();
};

class ShipLandingModule : public Room {
public:
	ShipLandingModule(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipGenerator : public Room {
public:
	ShipGenerator(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ShipOuterSpace : public Room {
public:
	ShipOuterSpace(SupernovaEngine *vm, GameManager *gm);
};


// Arsano
class ArsanoRocks : public Room {
public:
	ArsanoRocks(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class ArsanoCave : public Room {
public:
	ArsanoCave(SupernovaEngine *vm, GameManager *gm);
};

class ArsanoMeetup : public Room {
public:
	ArsanoMeetup(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	byte _sign;
	byte _beacon;
};

class ArsanoEntrance : public Room {
public:
	ArsanoEntrance(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();

private:
	StringId _dialog1[5];
	StringId _dialog2[5];
	StringId _dialog3[5];
	byte _eyewitness;
};

class ArsanoRemaining : public Room {
public:
	ArsanoRemaining(SupernovaEngine *vm, GameManager *gm);

	virtual void animation();

private:
	bool _chewing;
	int _i;
};

class ArsanoRoger : public Room {
public:
	ArsanoRoger(SupernovaEngine *vm, GameManager *gm);

	virtual void animation();
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringId _dialog1[4];
	byte _eyewitness;
	byte _hands;
};

class ArsanoGlider : public Room {
public:
	ArsanoGlider(SupernovaEngine *vm, GameManager *gm);

	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	byte _sinus;
};

class ArsanoMeetup2 : public Room {
public:
	ArsanoMeetup2(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

	void shipStart();

private:
	// TODO: change to 6, fix initialization
	StringId _dialog1[2];
	StringId _dialog2[2];
	StringId _dialog3[4];
	StringId _dialog4[3];

	// FIXME: Remove following unused bool variables?
	//bool _found;
	//bool _flug;
};

class ArsanoMeetup3 : public Room {
public:
	ArsanoMeetup3(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringId _dialog2[4];
	StringId _dialog3[2];

	// TODO: Hack, to be move away and renamed when the other uses are found
	StringId _dialogsX[6];
};


// Axacuss
class AxacussCell : public Room {
public:
	AxacussCell(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
	virtual void animation();
	virtual void onEntrance();
};

class AxacussCorridor1 : public Room {
public:
	AxacussCorridor1(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
};

class AxacussCorridor2 : public Room {
public:
	AxacussCorridor2(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
};

class AxacussCorridor3 : public Room {
public:
	AxacussCorridor3(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
};

class AxacussCorridor4 : public Room {
public:
	AxacussCorridor4(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussCorridor5 : public Room {
public:
	AxacussCorridor5(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void stopInteract(int sum);
	bool handleMoneyDialog();

	// TODO: Change to 6, or change struct, and fix initialization
	StringId _dialog1[2];
	StringId _dialog2[2];
	StringId _dialog3[4];

	byte _rows[6];
};

class AxacussCorridor6 : public Room {
public:
	AxacussCorridor6(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussCorridor7 : public Room {
public:
	AxacussCorridor7(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
};

class AxacussCorridor8 : public Room {
public:
	AxacussCorridor8(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussCorridor9 : public Room {
public:
	AxacussCorridor9(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussBcorridor : public Room {
public:
	AxacussBcorridor(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussIntersection : public Room {
public:
	AxacussIntersection(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringId _dialogsX[6];
};

class AxacussExit : public Room {
public:
	AxacussExit(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	StringId _dialogsX[6];
};

class AxacussOffice1 : public Room {
public:
	AxacussOffice1(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussOffice2 : public Room {
public:
	AxacussOffice2(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussOffice3 : public Room {
public:
	AxacussOffice3(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussOffice4 : public Room {
public:
	AxacussOffice4(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussOffice5 : public Room {
public:
	AxacussOffice5(SupernovaEngine *vm, GameManager *gm);

	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussElevator : public Room {
public:
	AxacussElevator(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussStation : public Room {
public:
	AxacussStation(SupernovaEngine *vm, GameManager *gm);

	virtual bool interact(Action verb, Object &obj1, Object &obj2);
};

class AxacussSign : public Room {
public:
	AxacussSign(SupernovaEngine *vm, GameManager *gm);

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
