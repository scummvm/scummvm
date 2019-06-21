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

#ifndef SUPERNOVA_ROOM_H
#define SUPERNOVA_ROOM_H

#include "common/str.h"

#include "supernova/msn_def.h"

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Supernova {

class GameManager1;
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
	char _shown[kMaxSection];
	byte _sentenceRemoved[kMaxDialog];
	Object _objectState[kMaxObject];
	RoomId _id;
	SupernovaEngine *_vm;

private:
	bool _seen;
};

}
#endif // SUPERNOVA_ROOM_H
