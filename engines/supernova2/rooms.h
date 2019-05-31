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

	int _restTime;
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

}
#endif // SUPERNOVA2_ROOMS_H
