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

#include "supernova2/ms2_def.h"

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Supernova2 {

class GameManager;
class Supernova2Engine;

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
	void removeSentenceByMask(int mask, int number);
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
	Supernova2Engine *_vm;
	GameManager *_gm;

private:
	bool _seen;
};

class Intro : public Room {
public:
	Intro(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();

private:
	void titleScreen();
	bool tvSay(int mod1, int mod2, int rest, MessagePosition pos, StringId id);
	bool tvRest(int mod1, int mod2, int rest);
	bool displayThoughtMessage(StringId id);
	bool thoughts1();
	bool tvDialogue();
	bool thoughts2();

	Common::String _introText;
};

class Airport : public Room {
public:
	Airport(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();

private:
};

class TaxiStand : public Room {
public:
	TaxiStand(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Street : public Room {
public:
	Street(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Games : public Room {
public:
	Games(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Cabin : public Room {
public:
	Cabin(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	bool _paid;
	bool _var2; // Not sure what this variable means yet
};

class Kiosk : public Room {
public:
	Kiosk(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class CulturePalace : public Room {
public:
	CulturePalace(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void notEnoughMoney();
	bool _var1;
	bool _var2;
	bool _var3;
};

class Checkout : public Room {
public:
	Checkout(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
	void appearance();
	bool _var1;
	int _var2;
	bool _var3;
	bool _var4;
	bool _var5;
};

class City1 : public Room {
public:
	City1(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class City2 : public Room {
public:
	City2(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Elevator : public Room {
public:
	Elevator(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Apartment : public Room {
public:
	Apartment(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Ship : public Room {
public:
	Ship(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Pyramid : public Room {
public:
	Pyramid(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class PyrEntrance : public Room {
public:
	PyrEntrance(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Upstairs1 : public Room {
public:
	Upstairs1(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Downstairs1 : public Room {
public:
	Downstairs1(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class BottomRightDoor : public Room {
public:
	BottomRightDoor(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class BottomLeftDoor : public Room {
public:
	BottomLeftDoor(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Upstairs2 : public Room {
public:
	Upstairs2(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Downstairs2 : public Room {
public:
	Downstairs2(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class UpperDoor : public Room {
public:
	UpperDoor(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class PuzzleFront : public Room {
public:
	PuzzleFront(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class PuzzleBehind : public Room {
public:
	PuzzleBehind(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Formula1F : public Room {
public:
	Formula1F(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Formula1N : public Room {
public:
	Formula1N(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Formula2F : public Room {
public:
	Formula2F(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Formula2N : public Room {
public:
	Formula2N(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class TomatoF : public Room {
public:
	TomatoF(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class TomatoN : public Room {
public:
	TomatoN(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class MonsterF : public Room {
public:
	MonsterF(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Monster1N : public Room {
public:
	Monster1N(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Monster2N : public Room {
public:
	Monster2N(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Upstairs3 : public Room {
public:
	Upstairs3(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Downstairs3 : public Room {
public:
	Downstairs3(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class LGang1 : public Room {
public:
	LGang1(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class LGang2 : public Room {
public:
	LGang2(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class HoleRoom : public Room {
public:
	HoleRoom(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class InHole : public Room {
public:
	InHole(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Bodentuer : public Room {
public:
	Bodentuer(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class BodentuerU : public Room {
public:
	BodentuerU(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class BstDoor : public Room {
public:
	BstDoor(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Hall : public Room {
public:
	Hall(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class CoffinRoom : public Room {
public:
	CoffinRoom(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mask : public Room {
public:
	Mask(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Museum : public Room {
public:
	Museum(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class MusEing : public Room {
public:
	MusEing(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus1 : public Room {
public:
	Mus1(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus2 : public Room {
public:
	Mus2(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus3 : public Room {
public:
	Mus3(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus4 : public Room {
public:
	Mus4(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus5 : public Room {
public:
	Mus5(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus6 : public Room {
public:
	Mus6(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus7 : public Room {
public:
	Mus7(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus8 : public Room {
public:
	Mus8(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus9 : public Room {
public:
	Mus9(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus10 : public Room {
public:
	Mus10(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus11 : public Room {
public:
	Mus11(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class MusRund : public Room {
public:
	MusRund(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus12 : public Room {
public:
	Mus12(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus13 : public Room {
public:
	Mus13(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus14 : public Room {
public:
	Mus14(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus15 : public Room {
public:
	Mus15(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus16 : public Room {
public:
	Mus16(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus17 : public Room {
public:
	Mus17(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus18 : public Room {
public:
	Mus18(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus19 : public Room {
public:
	Mus19(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus20 : public Room {
public:
	Mus20(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus21 : public Room {
public:
	Mus21(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

class Mus22 : public Room {
public:
	Mus22(Supernova2Engine *vm, GameManager *gm);
	virtual void onEntrance();
	virtual void animation();
	virtual bool interact(Action verb, Object &obj1, Object &obj2);

private:
};

}
#endif // SUPERNOVA2_ROOMS_H
